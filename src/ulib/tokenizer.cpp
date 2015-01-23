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
   U_TRACE(0, "UTokenizer::setData(%.*S)", U_STRING_TO_TRACE(data))

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

      token = str.substr(p, s - p);

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

      token = str.substr(p, s - p);

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

      token = str.substr(p, s - p);

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
   U_TRACE(0, "UTokenizer::tokenSeen(%.*S)", U_STRING_TO_TRACE(*x))

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

bool UTokenizer::skipNumber(bool& isReal)
{
   U_TRACE(0, "UTokenizer::skipNumber(%p)", &isReal)

   isReal = false;

   for (char c; s < end; ++s)
      {
      c = *s;

      if (u__isnumber(c)) continue;

      if (u__isreal(c) ||
          u__toupper(c) == 'E')
         {
         isReal = true;

         continue;
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString UTokenizer::getTokenQueryParser()
{
   U_TRACE(0, "UTokenizer::getTokenQueryParser()")

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

   UString token = str.substr(p, s - p);

   U_RETURN_STRING(token);
}

/*
Expression is tokenized as:

logical: && || !
compare: = == != < <= > =>
Multiplicative operators: *, /, %
Additive operators: +, -
precedence: ( )
quoted strings: 'string with a dollar: $FOO'
unquoted strings: string
variable substitution: $REMOTE_ADDR ${REMOTE_ADDR} $$(pid)
function call with optional params: FN_CALL([p1,p2,...,pn])

contains:    ^
ends_with:   =~
starts_with: ~=
*/

int UTokenizer::getTokenId(UString& token)
{
   U_TRACE(0, "UTokenizer::getTokenId(%p)", &token)

   char c;
   int tid = 0;
   const char* p1;
   const char* p2;

   U_INTERNAL_DUMP("s = %.*S", 20, s)

loop:
   p1 = p2 = s;

   if (s >= end) goto end;

   c = *s++;

   if (u__isspace(c)) goto loop;

   switch (c)
      {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         {
         tid = U_TK_VALUE;

         while (s < end && u__isdigit(*s)) ++s;

         p2 = s;
         }
      break;

      case '\'':
         {
         tid = U_TK_VALUE;

         p1 = s;

         while (s < end && *s != '\'') ++s;

         p2 = s++;
         }
      break;

      case '+': tid = U_TK_PLUS;                                          p2 = s; break;
      case '-': tid = U_TK_MINUS;                                         p2 = s; break;
      case '%': tid = U_TK_MOD;                                           p2 = s; break;
      case '(': tid = U_TK_LPAREN;                                        p2 = s; break;
      case ')': tid = U_TK_RPAREN;                                        p2 = s; break;
      case ',': tid = U_TK_COMMA;                                         p2 = s; break;

      case '|': tid = (*s == '|' ? (++s, U_TK_OR)          : U_TK_ERROR); p2 = s; break;
      case '&': tid = (*s == '&' ? (++s, U_TK_AND)         : U_TK_ERROR); p2 = s; break;

      case '=': tid = (*s == '=' ? (++s, U_TK_EQ)          : U_TK_EQ);    p2 = s; break;
      case '>': tid = (*s == '=' ? (++s, U_TK_GE)          : U_TK_GT);    p2 = s; break;
      case '<': tid = (*s == '=' ? (++s, U_TK_LE)          : U_TK_LT);    p2 = s; break;
      case '!': tid = (*s == '=' ? (++s, U_TK_NE)          : U_TK_NOT);   p2 = s; break;

      // foo  = "bar" - Un elemento il cui attributo "foo" è uguale a "bar"
      // foo ~= "bar" - Un elemento il cui attributo "foo" ha per valore un elenco di valori separati da spazio, uno dei quali uguale a "bar"
      // foo ^= "bar" - Un elemento il cui attributo "foo" ha un valore che inizia per "bar"
      // foo $= "bar" - Un elemento il cui attributo "foo" ha un valore che finisce per "bar"
      // foo *= "bar" - Un elemento il cui attributo "foo" ha un valore che contiene la sottostringa "bar"

      case '~': tid = (*s == '=' ? (++s, U_TK_IS_PRESENT)  : U_TK_ERROR); p2 = s; break;
      case '^': tid = (*s == '=' ? (++s, U_TK_STARTS_WITH) : U_TK_ERROR); p2 = s; break;
      case '*': tid = (*s == '=' ? (++s, U_TK_CONTAINS)    : U_TK_MULT);  p2 = s; break;

      case '$':
         {
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
         }
      break;

      case 't':
      case 'f':
         {
         if (c == 't' ? skipToken(U_CONSTANT_TO_PARAM("rue"))
                      : skipToken(U_CONSTANT_TO_PARAM("alse")))
            {
            tid = U_TK_VALUE;

            if (c == 't') p2 = s;

            break;
            }

         goto value;
         }
   // break;

      case '/':
         {
         c = *s;

         if (u__isdigit(c) ||
             u__isspace(c))
            {
            p2  = s;
            tid = U_TK_DIV;

            break;
            }

         goto value;
         }
   // break;

      default:
         {
value:
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
         }
      break;
      }

end:
   token = str.substr(p1, p2 - p1);

   U_INTERNAL_DUMP("token = %.*S", U_STRING_TO_TRACE(token))

   U_RETURN(tid);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UTokenizer::dump(bool reset) const
{
   *UObjectIO::os << "s                           " << (void*)s   << '\n'
                  << "end                         " << (void*)end << '\n'
                  << "group                       ";

   char buffer[32];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", group));

   *UObjectIO::os << '\n'
                  << "delim                       ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", delim));

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
