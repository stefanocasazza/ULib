// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    pcre.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/pcre/pcre.h>
#include <ulib/utility/string_ext.h>

/**
 * see: http://www.codeproject.com/Articles/638287/Short-but-very-usueful-regex-lookbehind-lazy-group
 * 
 * Short but very usueful regex lookbehind, lazy, group, and backreference - By morzel, 19 Aug 2013
 * 
 * Recently, I wanted to extract calls to external system from log files and do some LINQ to XML processing
 * on obtained data. Here a sample log line (simplified, real log was way more complicated but it doesn't matter for this post):
 * 
 * Call:<getName seqNo="56789"><id>123</id></getName> Result:<getName seqNo="56789">John Smith</getName>
 * 
 * I was interested in XML data of the call:
 * 
 * <getName seqNo="56789">
 *   <id>123</id>
 * </getName>
 * 
 * When it comes to log, some things were certain:
 *  call XML will be logged after Call: text on the beginning of line
 *  call root element name will contain only alphanumerical chars or underscore
 *  there will be no line brakes in call data
 *  call root element name may also appear in the Result section
 * 
 * Getting to the proper information was quite easy thanks to Regex class:
 * 
 * Regex regex = new Regex(@"(?<=^Call:)<(\w+).*?</\1>");
 * string call = regex.Match(logLine).Value;
 * 
 * This short regular expressions has a couple of interesting parts. It may not be perfect but proved really
 * helpful in log analysis. If this regex is not entirely clear to you - read on, you will need to use something similar sooner or later. 
 * 
 * Here the same regex with comments (RegexOptions.IgnorePatternWhitespace is required to process expression commented this way):
 * 
 * string pattern = @"(?<=^Call:) # Positive lookbehind for call marker
 *                    <(\w+)      # Capturing group for opening tag name
 *                    .*?         # Lazy wildcard (everything in between)
 *                    </\1>       # Backreference to opening tag name";   
 * Regex regex = new Regex(pattern, RegexOptions.IgnorePatternWhitespace);
 * string call = regex.Match(logLine).Value;
 * 
 * Positive lookbehind
 * 
 * (?<=Call:) is a lookaround or more precisely positive lookbehind. It's a zero-width assertion that lets us check whether some
 * text is preceded by another text. Here Call: is the preceding text we are looking for. (?<=something) denotes positive lookbehind.
 * There is also negative lookbehind expressed by (?<!something).  With negative lookbehind we can match text that doesn't have particular
 * string before it. Lookaround checks fragment of the text but doesn't became part of the match value. So the result of this:
 * 
 * Regex.Match("X123", @"(?<=X)\d*").Value
 * 
 * Will be "123" rather than "X123".
 * 
 * Note: In some cases (like in our log examination example) instead of using positive lookaround we may use non-capturing group...
 * 
 * Capturing group
 * 
 * <(\w+) will match less-than sign followed by one or more characters from \w class (letters, digits or underscores). \w+ part is
 * surrounded with parenthesis to create a group containing XML root name (getName for sample log line). We later use this group to
 * find closing tag with the use of backreference. (\w+) is capturing group, which means that results of this group existence are
 * added to Groups collection of Match object. If you want to put part of the expression into a group but you don't want to push
 * results into Groups collection you may use non-capturing group by adding a question mark and colon after opening parenthesis,
 * like this: (?:something)
 * 
 * Lazy wildcard
 * 
 * .*? matches all characters except newline (because we are not using RegexOptions.Singleline) in lazy (or non-greedy) mode thanks
 * to question mark after asterisk. By default * quantifier is greedy, which means that regex engine will try to match as much text
 * as possible. In our case, default mode will result in too long text being matched:
 * 
 * <getName seqNo="56789"><id>123</id></getName> Result:<getName seqNo="56789">John Smith</getName>
 * 
 * Backreference
 * 
 * </\1> matches XML close tag where element's name is provided with \1 backreference. Remember the (\w+) group? This group has number
 * 1 and by using \1 syntax we are referencing the text matched by this group. So for our sample log, </\1> gives us </getName>.
 * If regex is complex it may be a good idea to ditch numbered references and use named references instead. You can name a group by
 * <name> or name syntax and reference it by using k<name> or kname. So your expression could look like this:
 * 
 * @"(?<=^Call:)<(?<tag>\w+).*?</\k<tag>>"
 * 
 * or like this:
 * 
 * @"(?<=^Call:)<(?'tag'\w+).*?</\k'tag'>"
 * 
 * The latter version is better for our purpose. Using < > signs while matching XML is confusing. In this case regex engine will
 * do just fine with < > version but keep in mind that source code is written for humans
 */

UPCRE* UPCRE::dollar;
UPCRE* UPCRE::xml_mask;
UPCRE* UPCRE::url_mask;
UPCRE* UPCRE::username_mask;

U_NO_EXPORT bool UPCRE::checkBrackets()
{
   U_TRACE_NO_PARAM(1, "UPCRE::checkBrackets()")

   /**
    * certainly we need an anchor, we want to check if the whole arg is in brackets
    *
    * braces("^[^\\\\]\\(.*[^\\\\]\\)$");
    * perlish: [^\\]\(.*[^\\]\)
    *
    * There's no reason, not to add brackets in general.
    * It's more comfortable, cause we wants to start with $1 at all, also if we set the whole arg in brackets!
    */

   if (UStringExt::isDelimited(_expression, "()") == false)
      {
      _expression = '(' + _expression + ')';

      /* recreate the p_pcre* objects to avoid memory leaks */

      if (p_pcre)
         {
         U_SYSCALL_VOID(pcre_free, "%p", p_pcre);

         p_pcre = 0;
         }

      if (p_pcre_extra)
         {
         U_SYSCALL_VOID(pcre_free, "%p", p_pcre_extra);

         p_pcre_extra = 0;
         }

      compile(0);

      U_RETURN(true);
      }

   U_RETURN(false);
}

U_NO_EXPORT void UPCRE::zero(uint32_t flags)
{
   U_TRACE(0, "UPCRE::zero(%u)", flags)

   p_pcre       = 0;
   sub_vec      = 0;
   resultset    = 0;
   stringlist   = 0;
   p_pcre_extra = 0;

   sub_len = num_matches = 0;
   did_match = false;

   if (flags == 0) replace_t = global_t = false;
   else
      {
       global_t = ((flags & PCRE_GLOBAL)      != 0);
      replace_t = ((flags & PCRE_FOR_REPLACE) != 0);

      /* remove internal flag before feeding _flags to pcre */

      if ( global_t) flags -= PCRE_GLOBAL;
      if (replace_t) flags -= PCRE_FOR_REPLACE;
      }

   _flags = flags;
}

UPCRE::UPCRE()
{
   U_TRACE_REGISTER_OBJECT(0, UPCRE, "")

   zero(0);
}

void UPCRE::set(const UString& expression, uint32_t flags)
{
   U_TRACE(0, "UPCRE::set(%V,%u)", expression.rep, flags)

   U_INTERNAL_ASSERT(expression)

   _expression = expression;

   zero(flags);

   if ((replace_t && checkBrackets()) == false) compile(0);
}

void UPCRE::set(const UString& expression, const char* flags)
{
   U_TRACE(0, "UPCRE::set(%V,%S)", expression.rep, flags)

   U_INTERNAL_ASSERT(expression)

   _expression = expression;

   zero(0);

   for (uint32_t i = 0; flags[i] != '\0'; ++i)
      {
      switch (flags[i])
         {
         case 'i':   _flags |= PCRE_CASELESS;  break;
         case 'm':   _flags |= PCRE_MULTILINE; break;
         case 's':   _flags |= PCRE_DOTALL;    break;
         case 'x':   _flags |= PCRE_EXTENDED;  break;
         case 'g':  global_t = true;           break;
         case 'r': replace_t = true;           break;
         }
      }

   compile(0);
}

void UPCRE::clean()
{
   U_TRACE_NO_PARAM(0, "UPCRE::clean()")

   if (sub_vec)
      {
      UMemoryPool::_free(sub_vec, sub_len, sizeof(int));
                         sub_vec = 0;
      }

   if (stringlist)
      {
      U_SYSCALL_VOID(pcre_free_substring_list, "%p", stringlist);
                                                     stringlist = 0;

      U_INTERNAL_ASSERT_POINTER(resultset)

      resultset->clear();

      delete resultset;

      resultset = 0;
      }
}

void UPCRE::clear()
{
   U_TRACE_NO_PARAM(0, "UPCRE::clear()")

   /* avoid deleting of uninitialized pointers */

   if (p_pcre)
      {
      U_SYSCALL_VOID(pcre_free, "%p", p_pcre);
                                      p_pcre = 0;
      }

   if (p_pcre_extra)
      {
      U_SYSCALL_VOID(pcre_free, "%p", p_pcre_extra);
                                      p_pcre_extra = 0;
      }

   clean();

   _expression.clear();
}

U_NO_EXPORT void UPCRE::setStatus(int num)
{
   U_TRACE(0, "UPCRE::setStatus(%d)", num)

   const char* descr;

   switch (num)
      {
      case  0: descr = "PCRE_OK";                 break;
      case -1: descr = "PCRE_ERROR_NOMATCH";      break;
      case -2: descr = "PCRE_ERROR_NULL";         break;
      case -3: descr = "PCRE_ERROR_BADOPTION";    break;
      case -4: descr = "PCRE_ERROR_BADMAGIC";     break;
      case -5: descr = "PCRE_ERROR_UNKNOWN_NODE"; break;
      case -6: descr = "PCRE_ERROR_NOMEMORY";     break;
      case -7: descr = "PCRE_ERROR_NOSUBSTRING";  break;

      default: descr = "Code unknown";            break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("(%d, %s)"), num, descr);
}

/* compile the expression */

void UPCRE::compile(const unsigned char* tables) /* locale tables */
{
   U_TRACE(1, "UPCRE::compile(%p)", tables)

   int erroffset;
   const char* err_str;
   char* ptr = (char*) _expression.c_str();

   U_INTERNAL_ASSERT_POINTER(ptr)

   p_pcre = (pcre*) U_SYSCALL(pcre_compile, "%S,%u,%p,%p,%S", ptr, _flags, &err_str, &erroffset, tables);

#ifdef DEBUG
   if (p_pcre == 0) U_INTERNAL_DUMP("pcre_compile() failed: %S at: %S", err_str, ptr + erroffset)
#endif

   U_INTERNAL_ASSERT_POINTER(p_pcre)

   /* calculate the number of substrings we are willing to catch */

   int where;

#ifdef DEBUG
   int info = U_SYSCALL(pcre_fullinfo, "%p,%p,%d,%p", p_pcre, p_pcre_extra, PCRE_INFO_CAPTURECOUNT, &where);

   setStatus(info);

   U_INTERNAL_DUMP("status(%d) = %.*S", info, u_buffer_len, u_buffer)

   u_buffer_len = 0;

   U_INTERNAL_ASSERT_EQUALS(info, 0)
#else
   (void) U_SYSCALL(pcre_fullinfo, "%p,%p,%d,%p", p_pcre, p_pcre_extra, PCRE_INFO_CAPTURECOUNT, &where);
#endif

   sub_len = (where+2) * 3; /* see "man pcre" for the exact formula */

   U_INTERNAL_DUMP("sub_len = %d", sub_len)

   reset();
}

void UPCRE::study(int options)
{
   U_TRACE(1, "UPCRE::study(%d)", options)

   const char* err_str;

   p_pcre_extra = (pcre_extra*) U_SYSCALL(pcre_study, "%p,%d,%p", p_pcre, options, &err_str);

#ifdef DEBUG
   if (p_pcre_extra == 0 && err_str) U_INTERNAL_DUMP("pcre_study() failed: %S", err_str)
#endif
}

bool UPCRE::search(const char* stuff, uint32_t stuff_len, int offset, int options, bool bresultset)
{
   U_TRACE(1, "UPCRE::search(%.*S,%u,%d,%d,%b)", stuff_len, stuff, stuff_len, offset, options, bresultset)

   U_INTERNAL_ASSERT_POINTER(p_pcre)

   reset();

   if (sub_vec)         UMemoryPool::_free(sub_vec,      sub_len, sizeof(int));
       sub_vec = (int*) UMemoryPool::_malloc((uint32_t*)&sub_len, sizeof(int));

   if (stuff_len == 0) stuff_len = u__strlen(stuff, __PRETTY_FUNCTION__);

   int num = U_SYSCALL(pcre_exec, "%p,%p,%S,%u,%d,%d,%p,%d", p_pcre, p_pcre_extra, stuff, stuff_len, offset, options, sub_vec, sub_len);

   /**
    * <  0 no match at all;
    * == 0 vector too small, there were too many substrings in stuff
    */

   if (num <= 0) U_RETURN(false);

   did_match = true;

        if (num == 1) num_matches = 0; /* we had a match, but without substrings */
   else if (num >  1)                  /* we had matching substrings */
      {
      int i;
#ifdef DEBUG
      int res;
#endif

      num_matches = num - 1;

      if (stringlist)
         {
         U_SYSCALL_VOID(pcre_free_substring_list, "%p", stringlist);
                                                        stringlist = 0;

         U_INTERNAL_ASSERT_POINTER(resultset)

         resultset->clear();

         delete resultset;

         resultset = 0;
         }

      if (bresultset == false) goto end;

#ifdef DEBUG
      res =
#endif
      U_SYSCALL(pcre_get_substring_list, "%S,%p,%d,%p", stuff, sub_vec, num, &stringlist);

#ifdef DEBUG
      setStatus(res);

      U_INTERNAL_DUMP("status(%d) = %.*S", res, u_buffer_len, u_buffer)

      u_buffer_len = 0;

      U_INTERNAL_ASSERT_EQUALS(res, 0)
#endif

      U_DUMP("getMatchStart() = %u getMatchEnd() = %u", getMatchStart(), getMatchEnd())

      U_INTERNAL_ASSERT_EQUALS(resultset, 0)

      U_NEW(UVector<UString>, resultset, UVector<UString>);

      for (i = 1; i < num; ++i)
         {
         const char* p = stringlist[i];

         UString str(p, u__strlen(p, __PRETTY_FUNCTION__));

         resultset->push_back(str);

         U_INTERNAL_DUMP("resultset[%d] = (%u) %V", i-1, str.size(), str.rep)

         U_DUMP("getMatchStart(%d) = %u getMatchEnd(%d) = %u getMatchLength(%d) = %u", i-1, getMatchStart(i-1), i-1, getMatchEnd(i-1), i-1, getMatchLength(i-1))

         U_ASSERT_EQUALS(str.size(), getMatchLength(i-1))
         }
      }

end:
   U_INTERNAL_DUMP("num_matches = %d", num_matches)

   U_RETURN(true);
}

uint32_t UPCRE::split(UVector<UString>& vec, const UString& piece, int limit, int start_offset, int end_offset)
{
   U_TRACE(0, "UPCRE::split(%p,%V,%d,%d,%d)", &vec, piece.rep, limit, start_offset, end_offset)

   int num_pieces, piece_start, piece_end;
   uint32_t r, pos, sz, n = vec.size(), length = piece.size();

   if (_expression.size() == 1) /* _expression will be used as delimiter */
      {
      /* use the plain c++ way, ignore the pre-compiled p_pcre */

      char z;
      UString buffer(100U), _delimiter, _piece;

      if (_flags & PCRE_CASELESS)
         {
         z = u__toupper(_expression.first_char());

         for (pos = 0; pos < length; ++pos) _piece.push_back(u__toupper(piece.c_char(pos)));
         }
      else
         {
         z      = _expression.first_char();
         _piece = piece;
         }

      for (pos = 0; pos < length; ++pos)
         {
         if (_piece.c_char(pos) == z)
            {
            vec.push_back(buffer);

            buffer.setEmpty();
            buffer.duplicate();
            }
         else
            {
            buffer.push_back(piece.c_char(pos));
            }
         }

      if (buffer) vec.push_back(buffer);

      goto end;
      }

   /* use the regex way */

   if (replace_t == false) (void) checkBrackets();

   pos        = 0;
   num_pieces = 0;

   while (search(piece, pos, 0, false) &&
          matches() > 0) /* we had matching substrings */
      {
      ++num_pieces;

      piece_start = pos;
      piece_end   = getMatchStart(0);
      sz          = piece_end - piece_start;

      // NB: it's ok a null string... (see split in replaceVars())

      U_INTERNAL_DUMP("num_pieces = %d piece_start = %d piece_end = %d sz = %d", num_pieces, piece_start, piece_end, sz)

      if ((       limit == 0 || num_pieces <         limit) &&
          (start_offset == 0 || num_pieces >= start_offset) &&
          (  end_offset == 0 || num_pieces <=   end_offset))
         {
         /* we are within the allowed range, so just add the grab */

         UString junk(piece, piece_start, sz);

         vec.push_back(junk);
         }

      pos = piece_end + getMatchLength(0);

      U_INTERNAL_DUMP("pos = %u", pos)
      }

   /* the rest of the string, there are no more delimiters */

   ++num_pieces;

   if ((       limit == 0 || num_pieces <         limit) &&
       (start_offset == 0 || num_pieces >= start_offset) &&
       (  end_offset == 0 || num_pieces <=   end_offset))
      {
      /* we are within the allowed range, so just add the grab */

      UString junk(piece, pos, length - pos);

      vec.push_back(junk);
      }

end:
   r = vec.size() - n;

   U_RETURN(r);
}

/* replace method */

UString UPCRE::replace(const UString& piece, const UString& with)
{
   U_TRACE(0, "UPCRE::replace(%V,%V)", piece.rep, with.rep)

   if (dollar == 0)
      {
      U_NEW_ULIB_OBJECT(UPCRE, dollar, UPCRE(U_STRING_FROM_CONSTANT("\\${?([0-9]+)}?"), 0U));

      dollar->study();
      }

   int iReplaced = -1;
   bool bReplaced = false;
   UString replaced(piece), use_with;

   if (replace_t == false) (void) checkBrackets();

   if (search(piece, 0, 0, true))
      {
      // we found at least one match

      U_INTERNAL_DUMP("global_t = %b", global_t)

      if (global_t == false)
         {
         // here we can resolve vars if option g is not set

         use_with = replaceVars(with);

         if (matched() &&
             matches() >= 1)
            {
            int len = getMatchEnd() - getMatchStart() + 1;

            replaced.replace(getMatchStart(0), len, use_with);

            bReplaced = true;
            iReplaced = 0;
            }
         }
      else
         {
         // here we need to resolve the vars certainly for every hit. Could be different content sometimes!

         int match_pos;

         do {
            use_with = replaceVars(with);
                                   
            int len = getMatchEnd() - getMatchStart() + 1;

            replaced.replace(getMatchStart(0), len, use_with);
                                   
            // next run should begin after the last char of the stuff we put in the text

            match_pos = (use_with.length() - len) + getMatchEnd() + 1;

              bReplaced = true;
            ++iReplaced;
            }
         while (search(replaced, match_pos, 0, true));
         }
      }

   did_match   = bReplaced;
   num_matches = iReplaced;

   // clean dollar

   U_INTERNAL_ASSERT_POINTER(dollar)

   dollar->clean();

   U_RETURN_STRING(replaced);
}

U_NO_EXPORT UString UPCRE::replaceVars(const UString& piece)
{
   U_TRACE(0, "UPCRE::replaceVars(%V)", piece.rep)

   UPCRE subsplit;
   UVector<UString> splitted;
   uint32_t pos, last = resultset->size();
   UString cstr(U_CAPACITY), first, replaced, sBracketContent, with = piece;

   U_INTERNAL_ASSERT_POINTER(dollar)

   while (dollar->search(with, 0, 0, true))
      {
      // let's do some conversion first

      first = dollar->resultset->at(0);

      uint32_t iBracketIndex = first.strtoul();

      // NB: we need this check...

      sBracketContent = resultset->at(iBracketIndex < last ? iBracketIndex : last-1);

      U_INTERNAL_DUMP("sBracket[%d] = %V", iBracketIndex, sBracketContent.rep)

      // now we can split the stuff

      subsplit.clear();

      cstr.snprintf(U_CONSTANT_TO_PARAM("(\\${?%v}?)"), first.rep);

      subsplit.set(cstr, PCRE_FOR_REPLACE);

      // normally 2 (or more) parts, the one in front of and the other one after "$..."

      replaced.clear();
      splitted.clear();

      uint32_t size = subsplit.split(splitted, with);

      for (pos = 0; pos < size; ++pos)
         {
         if (pos == (size - 1)) replaced += splitted[pos];
         else                   replaced += splitted[pos] + sBracketContent;
         }

      with = replaced; // well, one part is done
      }

   U_RETURN_STRING(with);
}

// ---------------------------------------------------------------------------------------------
// regular expressions that I have found the most useful for day-to-day web programming tasks...
//
// http://immike.net/blog/2007/04/06/5-regular-expressions-every-web-programmer-should-know/
// ---------------------------------------------------------------------------------------------

bool UPCRE::validateUsername(const UString& username)
{
   U_TRACE(0, "UPCRE::validateUsername(%V)", username.rep)

   if (username_mask == 0)
      {
      U_NEW_ULIB_OBJECT(UPCRE, username_mask, UPCRE(U_STRING_FROM_CONSTANT("(/^[a-zA-Z0-9_]{3,16}$/)"), PCRE_FOR_REPLACE));

      username_mask->study();
      }

   if (username_mask->search(username, 0, 0, false)) U_RETURN(true);

   U_RETURN(false);
}

// Matching an XHTML/XML tag with a certain attribute value
// The function tags an attribute, value, input text, and an optional tag name as arguments.
// If no tag name is specified it will match any tag with the specified attribute and attribute value

uint32_t UPCRE::getTag(UVector<UString>& vec, const UString& xml, const char* attr, const char* value, const char* tag)
{
   U_TRACE(0, "UPCRE::getTag(%p,%V,%S,%S,%S)", &vec, xml.rep, attr, value, tag)

   if (xml_mask == 0)
      {
      UString cstr(U_CAPACITY);

      cstr.snprintf(U_CONSTANT_TO_PARAM("(<(%s)[^>]*%s\\s*=\\s*([\\'\\\"])%s\\\\2[^>]*>(.*?)<\\/\\\\1>)"), tag, attr, value);

      (void) cstr.shrink();

      U_NEW_ULIB_OBJECT(UPCRE, xml_mask, UPCRE(cstr, PCRE_FOR_REPLACE));

      xml_mask->study();
      }

   uint32_t result = xml_mask->split(vec, xml, 0, 0, 0);

   U_RETURN(result);
}

/**
 * RFC 2396 gives the following regular expression for parsing URLs:
 *
 * ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?
 * 12            3  4          5       6  7        8 9
 *
 * scheme    = $2
 * authority = $4
 * path      = $5
 * query     = $7
 * fragment  = $9
 *
 * query and fragment can be considered optional,
 * scheme and authority could also be optional in the presence of a base url
 */

#define U_REGEX_URL "(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"

bool UPCRE::isValidURL(const UString& url)
{
   U_TRACE(0, "UPCRE::isValidURL(%V)", url.rep)

   if (url_mask == 0)
      {
      U_NEW_ULIB_OBJECT(UPCRE, url_mask, UPCRE(U_STRING_FROM_CONSTANT(U_REGEX_URL), PCRE_FOR_REPLACE));

      url_mask->study();
      }

   bool result = url_mask->search(url, 0, 0, false);

   U_RETURN(result);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UPCRE::dump(bool _reset) const
{
   *UObjectIO::os << "p_pcre                        " << (void*)p_pcre       << '\n'
                  << "_flags                        " << _flags              << '\n'
                  << "sub_len                       " << sub_len             << '\n'
                  << "sub_vec                       " << (void*)sub_vec      << '\n'
                  << "global_t                      " << global_t            << '\n'
                  << "replace_t                     " << replace_t           << '\n'
                  << "did_match                     " << did_match           << '\n'
                  << "stringlist                    " << (void*)stringlist   << '\n'
                  << "num_matches                   " << num_matches         << '\n'
                  << "p_pcre_extra                  " << (void*)p_pcre_extra << '\n'
                  << "_expression (UString          " << (void*)&_expression << ")\n"
                  << "resultset   (UVector<UString> " << (void*)resultset    << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
