// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tokenizer.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/tokenizer.h>
#include <ulib/utility/services.h>
#include <ulib/container/vector.h>

bool        UTokenizer::group_skip;
bool        UTokenizer::avoid_punctuation;
uint32_t    UTokenizer::group_len;
uint32_t    UTokenizer::group_len_div_2;
const char* UTokenizer::group;

void UTokenizer::setData(const UString& data)
{
   U_TRACE(0, "UTokenizer::setData(%V)", data.rep)

   str = data;
   end = (s = data.data()) + data.size();
}

bool UTokenizer::next(UString& token, bPFi func)
{
   U_TRACE(0, "UTokenizer::next(%p,%p)", &token, func)

   const char* p;

   while (s < end)
      {  
      // skip char with function

      if (func(*s))
         {
         ++s;

         continue;
         }

      p = s;

      while (s < end &&
             func(*s) == false)
         {
         ++s;
         }

      token = substr(p);

      ++s;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UTokenizer::next(UString& token, char c)
{
   U_TRACE(0, "UTokenizer::next(%p,%C)", &token, c)

   const char* p;

   while (s < end)
      {  
      // skip char delimiter

      if (*s == c)
         {
         ++s;

         continue;
         }

      // delimit token with char delimiter

      p = s;
      s = (const char*) memchr(s, c, end - s);

      if (s == 0) s = end;

      token = substr(p);

      ++s;

      U_RETURN(true);
      }

   U_RETURN(false);
}

// extend the actual token to the next char 'c'... (see PEC_report.cpp)

bool UTokenizer::extend(UString& token, char c)
{
   U_TRACE(0, "UTokenizer::extend(%p,%C)", &token, c)

   const char* p;

   while (s < end)
      {  
      // skip char delimiter

      if (*s == c)
         {
         ++s;

         continue;
         }

      // delimit token with char delimiter

      p = token.data();
      s = (const char*) memchr(s, c, end - s);

      if (s == 0) s = end;

      token = substr(p);

      ++s;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UTokenizer::next(UString& token, bool* bgroup)
{
   U_TRACE(0, "UTokenizer::next(%p,%p)", &token, bgroup)

   const char* p  = s;
   uint32_t shift = 1, n;

   if (bgroup) *bgroup = false;

   while (s < end)
      {
loop:
      if (delim)
         {
         s = u_delimit_token(s, &p, end, delim, 0);

         if (p) goto tok;

         U_RETURN(false);
         }

      s = u_skip(s, end, 0, 0);

      if (s == end) break;

      if (group)
         {
         if (memcmp(s, group, group_len_div_2) == 0)
            {
            p = s + group_len_div_2 - 1;
            s = u_strpend(p, end - p, group, group_len, '\0');

            ++p;

            if (s == 0) s = end;

            U_INTERNAL_DUMP("p = %.*S s = %.*S", s - p, p, end - s, s)

            if (group_skip)
               {
               s += group_len_div_2;

               continue;
               }

            if (bgroup) *bgroup = true;

            shift = group_len_div_2;

            goto tok;
            }
         else if (group_skip)
            {
            // -------------------------------------------------------------------
            // examples:
            // -------------------------------------------------------------------
            // <date>03/11/2005 10:17:46</date>
            // <description>description_556adfbc-0107-5000-ede4-d208</description>
            // -------------------------------------------------------------------

            s = u_delimit_token(s, &p, end, 0, 0);

            if (s < end)
               {
               const char* x = (char*) memchr(p, group[0], s - p);

               if (x && (memcmp(x, group, group_len_div_2) == 0))
                  {
                  s     = x;
                  shift = 0;
                  }
               }

            goto tok;
            }
         }

      s = u_delimit_token(s, &p, end, 0, 0);

tok:
      n = s - p;

      if (avoid_punctuation)
         {
         while (u__ispunct(*p))
            {
            --n;
            ++p;

            if (p == s) goto loop;
            }

         while (u__ispunct(p[n-1]))
            {
            --n;

            if (n == 0) goto loop;
            }
         }

      token = str.substr(p, n);

      s += shift;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UTokenizer::tokenSeen(const UString* x)
{
   U_TRACE(0, "UTokenizer::tokenSeen(%V)", x->rep)

   U_INTERNAL_DUMP("s = %.*S", end - s, s)

   skipSpaces();

   if (s < end)
      {
      uint32_t sz = x->size();

      if (memcmp(s, x->data(), sz) == 0)
         {
         s += sz;

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

bool UTokenizer::skipToken(const char* token, uint32_t sz)
{
   U_TRACE(0, "UTokenizer::skipToken(%.*S,%u)", sz, token, sz)

   if (str.distance(s) >= sz &&
       memcmp(s, token, sz) == 0)
      {
      s += sz;

      U_RETURN(true);
      }

   U_RETURN(false);
}

int UTokenizer::getTypeNumber()
{
   U_TRACE_NO_PARAM(0, "UTokenizer::getTypeNumber()")

   U_INTERNAL_ASSERT_EQUALS(u__issign(*s), false)
   U_INTERNAL_ASSERT(u__issign(*(s-1)) || u__isdigit(*(s-1)))

   int type_num = u__isdigit(*(s-1));

   for (const char* start = s; s < end; ++s)
      {
      char c = *s;

      U_INTERNAL_DUMP("c = %C type_num = %d", c, type_num)

      if (u__isdigit(c))
         {
         if (type_num == 0) type_num = 1;

         continue;
         }

      if (u__issign(c))
         {
         if (type_num == INT_MIN) continue;

         U_RETURN(0);
         }

      if (c == '.' &&
          type_num <= 1)
         {
         int pos = (s-start);

         if (pos == 0) type_num = INT_MIN+1;
         else
            {
            if (u__isdigit(*(start-1))) ++pos;

            type_num = -pos;
            }

         continue;
         }

      if (u__toupper(c) == 'E') // scientific notation (Ex: 1.45e-10)
         {
         type_num = INT_MIN;

         continue;
         }

      break;
      }

   U_RETURN(type_num);
}

UString UTokenizer::getTokenQueryParser()
{
   U_TRACE_NO_PARAM(0, "UTokenizer::getTokenQueryParser()")

   skipSpaces();

   const char* p = s++;

   if (*p == '"')
      {
      while (s < end && *s++ != '"') {}
      }
   else
      {
      while (s < end &&  // u__isname(*s)
             (u__isspace(*s) == false &&
              *s != '('               &&
              *s != ')'))
         {
         ++s;
         }
      }

   return substr(p);
}

/**
 * Expression is tokenized as:
 *
 * precedence: ( )
 * logical: && || !
 * compare: = == != < <= > =>
 * Additive operators: +, -
 * Multiplicative operators: *, /, %
 * unquoted strings: string
 * quoted strings: 'string with a dollar: $FOO'
 * variable substitution: $REMOTE_ADDR ${REMOTE_ADDR} $$(pid)
 * function call with optional params: FN_CALL([p1,p2,...,pn])
 *
 * contains:    ^
 * ends_with:   =~
 * starts_with: ~=
 */

int UTokenizer::getTokenId(UString* ptoken)
{
   U_TRACE(0, "UTokenizer::getTokenId(%p)", ptoken)

   static const int dispatch_table[] = {
      (int)((char*)&&case_exclamation-(char*)&&cvalue),/* '!' */
      0,/* '"' */
      0,/* '#' */
      (int)((char*)&&case_dollar-(char*)&&cvalue),/* '$' */
      (int)((char*)&&case_percent-(char*)&&cvalue),/* '%' */
      (int)((char*)&&case_ampersand-(char*)&&cvalue),/* '&' */
      (int)((char*)&&case_quote-(char*)&&cvalue),/* '\'' */
      (int)((char*)&&case_opening_parenthesis-(char*)&&cvalue),/* '(' */
      (int)((char*)&&case_closing_parenthesis-(char*)&&cvalue),/* ')' */
      (int)((char*)&&case_asterisk-(char*)&&cvalue),/* '*' */
      (int)((char*)&&case_plus-(char*)&&cvalue),/* '+' */
      (int)((char*)&&case_comma-(char*)&&cvalue),/* ',' */
      (int)((char*)&&case_minus-(char*)&&cvalue),/* '-' */
      0,/* '.' */
      (int)((char*)&&case_slash-(char*)&&cvalue),/* '/' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '0' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '1' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '2' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '3' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '4' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '5' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '6' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '7' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '8' */
      (int)((char*)&&case_digit-(char*)&&cvalue),/* '9' */
      0,/* ':' */
      0,/* ';' */
      (int)((char*)&&case_less-(char*)&&cvalue),/* '<' */
      (int)((char*)&&case_equal-(char*)&&cvalue),/* '=' */
      (int)((char*)&&case_major-(char*)&&cvalue),/* '>' */
      0,/* '?' */
      0,/* '@' */
      0,/* 'A' */
      0,/* 'B' */
      0,/* 'C' */
      0,/* 'D' */
      0,/* 'E' */
      0,/* 'F' */
      0,/* 'G' */
      0,/* 'H' */
      0,/* 'I' */
      0,/* 'J' */
      0,/* 'K' */
      0,/* 'L' */
      0,/* 'M' */
      0,/* 'N' */
      0,/* 'O' */
      0,/* 'P' */
      0,/* 'Q' */
      0,/* 'R' */
      0,/* 'S' */
      0,/* 'T' */
      0,/* 'U' */
      0,/* 'V' */
      0,/* 'W' */
      0,/* 'X' */
      0,/* 'Y' */
      0,/* 'Z' */
      0,/* '[' */
      0,/* '\' */
      0,/* ']' */
      (int)((char*)&&case_xor-(char*)&&cvalue),/* '^' */
      0,/* '_' */
      0,/* '`' */
      0,/* 'a' */
      0,/* 'b' */
      0,/* 'c' */
      0,/* 'd' */
      0,/* 'e' */
      (int)((char*)&&case_bool-(char*)&&cvalue),/* 'f' */
      0,/* 'g' */
      0,/* 'h' */
      0,/* 'i' */
      0,/* 'j' */
      0,/* 'k' */
      0,/* 'l' */
      0,/* 'm' */
      0,/* 'n' */
      0,/* 'o' */
      0,/* 'p' */
      0,/* 'q' */
      0,/* 'r' */
      0,/* 's' */
      (int)((char*)&&case_bool-(char*)&&cvalue),/* 't' */
      0,/* 'u' */
      0,/* 'v' */
      0,/* 'w' */
      0,/* 'x' */
      0,/* 'y' */
      0,/* 'z' */
      0,/* '{' */
      (int)((char*)&&case_pipe-(char*)&&cvalue),/* '|' */
      0,/* '}' */
      (int)((char*)&&case_tilde-(char*)&&cvalue)/* '~' */
   };

   char c;
   long sz;
   int tid = 0;
   const char* p1;
   const char* p2;

   U_INTERNAL_DUMP("s = %.*S", 20, s)

loop:
   p1 = p2 = s;

   if (s >= end) goto end;

   c = *s++;

   if (u__isspace(c)) goto loop;

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&cvalue = %p", c-'!', dispatch_table[c-'!'], &&cvalue)

   goto *((char*)&&cvalue + dispatch_table[c-'!']);

case_exclamation: tid = (*s == '=' ? (++s, U_TK_NE) : U_TK_NOT); p2 = s; goto end; /* '!' */

case_dollar: /* '$' */
   if (*s == '=')
      {
      p2  = ++s;
      tid = U_TK_ENDS_WITH;
      }
   else if (*s == '$')
      {
      p2  = ++s;
      tid = U_TK_PID;
      }
   else
      {
      tid = U_TK_NAME;

      if (*s == '{')
         {
         p1 = ++s;

         while (s < end && *s != '}') ++s;

         p2 = s++;
         }
      else
         {
         p1 = s;

         while (s < end && u__isname(*s)) ++s;

         p2 = s;
         }
      }

   goto end;

case_percent:   tid = U_TK_MOD;                                   p2 = s; goto end; /* '%' */
case_ampersand: tid = (*s == '&' ? (++s, U_TK_AND) : U_TK_ERROR); p2 = s; goto end; /* '&' */

case_quote: /* '\'' */
   tid = U_TK_VALUE;

   p1 = s;

   while (s < end && *s != '\'') ++s;

   p2 = s++;

   goto end;

case_opening_parenthesis: tid = U_TK_LPAREN; p2 = s; goto end; /* '(' */
case_closing_parenthesis: tid = U_TK_RPAREN; p2 = s; goto end; /* ')' */

case_asterisk: tid = (*s == '=' ? (++s, U_TK_CONTAINS) : U_TK_MULT); p2 = s; goto end; /* '*' */
case_plus:     tid = U_TK_PLUS;                                      p2 = s; goto end; /* '+' */
case_comma:    tid = U_TK_COMMA;                                     p2 = s; goto end; /* ',' */
case_minus:    tid = U_TK_MINUS;                                     p2 = s; goto end; /* '-' */

case_slash: /* '/' */
   c = *s;

   if (u__isdigit(c) ||
       u__isspace(c))
      {
      p2  = s;
      tid = U_TK_DIV;

      goto end;
      }

   goto cvalue;

case_digit: /* '0' ... '9' */
   tid = U_TK_VALUE;

   while (s < end && u__isdigit(*s)) ++s;

   p2 = s;

   goto end;

// foo  = "bar" - Un elemento il cui attributo "foo" è uguale a "bar"
// foo ~= "bar" - Un elemento il cui attributo "foo" ha per valore un elenco di valori separati da spazio, uno dei quali uguale a "bar"
// foo ^= "bar" - Un elemento il cui attributo "foo" ha un valore che inizia per "bar"
// foo $= "bar" - Un elemento il cui attributo "foo" ha un valore che finisce per "bar"
// foo *= "bar" - Un elemento il cui attributo "foo" ha un valore che contiene la sottostringa "bar"

case_less:  tid = (*s == '=' ? (++s, U_TK_LE)          : U_TK_LT);    p2 = s; goto end; /* '<' */
case_equal: tid = (*s == '=' ? (++s, U_TK_EQ)          : U_TK_EQ);    p2 = s; goto end; /* '=' */
case_major: tid = (*s == '=' ? (++s, U_TK_GE)          : U_TK_GT);    p2 = s; goto end; /* '>' */
case_xor:   tid = (*s == '=' ? (++s, U_TK_STARTS_WITH) : U_TK_ERROR); p2 = s; goto end; /* '^' */

case_bool: /* 'f' 't' */
   if (c == 't' ? skipToken(U_CONSTANT_TO_PARAM("rue"))
                : skipToken(U_CONSTANT_TO_PARAM("alse")))
      {
      tid = U_TK_VALUE;

      if (c == 't') p2 = s;

      goto end;
      }

   goto cvalue;

case_pipe:  tid = (*s == '|' ? (++s, U_TK_OR)          : U_TK_ERROR); p2 = s; goto end; /* '|' */
case_tilde: tid = (*s == '=' ? (++s, U_TK_IS_PRESENT)  : U_TK_ERROR); p2 = s; goto end; /* '~' */

cvalue:
   while (s < end)
      {
      c = *s;

      if (c == '(' ||
          c == ')' ||
          c == ',' ||
          u__isgraph(c) == false)
         {         
         break;
         }

      ++s;
      }

   p2  = s;
   tid = (c == '(' ? U_TK_FN_CALL : U_TK_VALUE);

end:
   if (ptoken)
      {
      sz = (p2 -p1);

      if (sz)
         {
         *ptoken = str.substr(p1, sz);

         U_INTERNAL_DUMP("token = %V", ptoken->rep)
         }
      }

   U_RETURN(tid);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UTokenizer::dump(bool reset) const
{
   *UObjectIO::os << "s                           " << (void*)s   << '\n'
                  << "end                         " << (void*)end << '\n'
                  << "group                       ";

   char buffer[32];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%S"), group));

   *UObjectIO::os << '\n'
                  << "delim                       ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%S"), delim));

   *UObjectIO::os << '\n'
                  << "group_skip                  " << group_skip  << '\n'
                  << "str       (UString          " << (void*)&str << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
