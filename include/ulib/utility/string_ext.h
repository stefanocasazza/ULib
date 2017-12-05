// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    string_ext.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_STRING_EXT_H
#define ULIB_STRING_EXT_H 1

#include <ulib/container/vector.h>

#ifdef USE_LIBSSL
#  include <openssl/pem.h>
#endif
#ifdef USE_LIBBROTLI
#  include <brotli/encode.h>
#  include <brotli/decode.h>
#endif

class U_EXPORT UStringExt {
public:
#ifdef USE_LIBSSL
   static UString BIOtoString(BIO* bio);
   static UString ASN1TimetoString(ASN1_GENERALIZEDTIME* t);
#endif

#ifdef USE_LIBPCRE // Searches subject for matches to pattern and replaces them with replacement
   static UString sanitize(const UString& input);
   static UString pregReplace(const UString& pattern, const UString& replacement, const UString& subject);
#endif

#ifdef USE_LIBEXPAT // to strip out the HTML tags
   static UString stripTags(const UString& html, UString* list_tags_allowed = U_NULLPTR);
#endif

#ifndef U_LOG_DISABLE
   static const char* deflate_agent;
#endif

   static bool isDelimited(const UString& s, const char* delimiter = "()")
      {
      U_TRACE(0, "UStringExt::isDelimited(%V,%S)", s.rep, delimiter)

      U_INTERNAL_ASSERT_EQUALS(u__strlen(delimiter, __PRETTY_FUNCTION__), 2)

      if (s.first_char() != delimiter[0] ||
          s.last_char()  != delimiter[1] ||
          u_strpend(U_STRING_TO_PARAM(s), delimiter, 2, 0) != s.c_pointer(s.size()-1))
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   // COMPRESS method

   static bool isCompress(const char* s)       { return (u_get_unalignedp32(s) == U_MULTICHAR_CONSTANT32('\x89','M','N','Z')); } // U_MINIZ_COMPRESS
   static bool isCompress(const UString& s)    { return isCompress(s.data()); }

   static UString   compress(const char* s, uint32_t n);
   static UString decompress(const char* s, uint32_t n);

   static UString   compress(const UString& s) { return   compress(U_STRING_TO_PARAM(s)); }
   static UString decompress(const UString& s) { return decompress(U_STRING_TO_PARAM(s)); }

   static uint32_t ratio, ratio_threshold;

   static bool isGzip(const char* s)    { return (u_get_unalignedp16(s) == U_MULTICHAR_CONSTANT16('\x1F','\x8B')); }
   static bool isGzip(const UString& s) { return isGzip(s.data()); }

   static UString gunzip(const char* s, uint32_t n, uint32_t sz_orig); // .gz uncompress
   static UString gunzip(const UString& s, uint32_t sz_orig = 0) { return  gunzip(U_STRING_TO_PARAM(s), sz_orig); }

#ifdef USE_LIBZ
   static UString deflate(const char* s, uint32_t n, uint32_t quality = 0); // .gz compress (0 => zopfli)
   static UString deflate(const UString& s, uint32_t quality = 0) { return deflate(U_STRING_TO_PARAM(s), quality); }
#endif

   static bool isBrotli(const char* s)    { return false; } // (u_get_unalignedp32(s) == U_MULTICHAR_CONSTANT32('\xCE','\xB2','\xCF','\x81'))
   static bool isBrotli(const UString& s) { return isBrotli(s.data()); }

   static UString unbrotli(const char* s, uint32_t n);
   static UString unbrotli(const UString& s) { return unbrotli(U_STRING_TO_PARAM(s)); }

#ifdef USE_LIBBROTLI
   static UString brotli(const char* s, uint32_t n, uint32_t quality = BROTLI_MAX_QUALITY, uint32_t mode = BROTLI_MODE_TEXT, uint32_t lgwin = BROTLI_DEFAULT_WINDOW);
   static UString brotli(const UString& s,          uint32_t quality = BROTLI_MAX_QUALITY, uint32_t mode = BROTLI_MODE_TEXT, uint32_t lgwin = BROTLI_DEFAULT_WINDOW)
      { return brotli(U_STRING_TO_PARAM(s), quality, mode, lgwin); }
#endif

   // Convert numeric to string

   static UString printSize(off_t n)
      {
      U_TRACE(0, "UStringExt::printSize(%I)", n)

      UString x(32U);

      x.rep->_length = u_printSize(x.data(), n);

      U_RETURN_STRING(x);
      }

   static UString numberToString(uint32_t n)
      {
      U_TRACE(0, "UStringExt::numberToString(%u)", n)

      UString x(22U);

      x.setFromNumber32(n);

      U_RETURN_STRING(x);
      }

   static UString numberToString(uint64_t n);

   static UString numberToString(double n)
      {
      U_TRACE(0, "UStringExt::numberToString(%f)", n)

      UString x(32U);
      char* ptr = x.data();

      ptr[(x.rep->_length = (u_dtoa(n, ptr) - ptr))] = '\0';

      U_RETURN_STRING(x);
      }

   static UString stringFromNumber(long n)
      {
      U_TRACE(0, "UStringExt::stringFromNumber(%ld)", n)

      UString x(22U);

#  if SIZEOF_LONG == 4
      x.setFromNumber32s(n);
#  else
      x.setFromNumber64s(n);
#  endif

      U_RETURN_STRING(x);
      }

   static void appendNumber32( UString& s, uint32_t number) { s.appendNumber32( number); }
   static void appendNumber32s(UString& s,  int32_t number) { s.appendNumber32s(number); }
   static void appendNumber64( UString& s, uint64_t number) { s.appendNumber64( number); }
   static void appendNumber64s(UString& s,  int64_t number) { s.appendNumber64s(number); }

   // convert letter to upper or lower case

   static UString tolower(const char* s, uint32_t n)
      {
      U_TRACE(0, "UStringExt::tolower(%.*S,%u)", n, s, n)

      UString r(n);

            char* ptr =      r.rep->data();
      const char* end = s + (r.rep->_length = n);

      while (s < end) *ptr++ = u__tolower(*s++);

      *ptr = '\0';

      U_RETURN_STRING(r);
      }

   static UString toupper(const char* s, uint32_t n)
      {
      U_TRACE(0, "UStringExt::toupper(%.*S,%u)", n, s, n)

      UString r(n);

            char* ptr =      r.rep->data();
      const char* end = s + (r.rep->_length = n);

      while (s < end) *ptr++ = u__toupper(*s++);

      *ptr = '\0';

      U_RETURN_STRING(r);
      }

   static UString tolower(const UString& s) { return tolower(U_STRING_TO_PARAM(s)); }
   static UString toupper(const UString& s) { return toupper(U_STRING_TO_PARAM(s)); }

   // manage pathname

   static UString  dirname(const char* s, uint32_t n);
   static UString basename(const char* s, uint32_t n)
      {
      U_TRACE(0, "UStringExt::basename(%.*S,%u)", n, s, n)

      const char* ptr = u_basename(s, n);

      return UString(ptr, n-(ptr-s));
      }

   static UString  dirname(const UString& s) { return  dirname(U_STRING_TO_PARAM(s)); }
   static UString basename(const UString& s) { return basename(U_STRING_TO_PARAM(s)); }

   static uint32_t getBaseNameLen(const char* s, uint32_t n)
      {
      U_TRACE(0, "UStringExt::getBaseNameLen(%.*S,%u)", n, s, n)

      return n-(u_basename(s, n)-s);
      }

   static uint32_t getBaseNameLen(const UString& s) { return getBaseNameLen(U_STRING_TO_PARAM(s)); }

   // check if string s1 start with string s2

   static bool startsWith(const char* s1, uint32_t n1, const char* s2, uint32_t n2)
      {
      U_TRACE(0, "UStringExt::startsWith(%.*S,%u,%.*S,%u)", n1, s1, n1, n2, s2, n2)

      if (n1 >= n2 && u_startsWith(s1, n1, s2, n2)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool startsWith(const UString& s1, const UString& s2)           { return startsWith(U_STRING_TO_PARAM(s1), U_STRING_TO_PARAM(s2)); }
   static bool startsWith(const UString& s1, const char* s2, uint32_t n2) { return startsWith(U_STRING_TO_PARAM(s1), s2, n2); }

   // check if string s1 terminate with string s2

   static bool endsWith(const char* s1, uint32_t n1, const char* s2, uint32_t n2)
      {
      U_TRACE(0, "UStringExt::endsWith(%.*S,%u,%.*S,%u)", n1, s1, n1, n2, s2, n2)

      if (n1 >= n2 && u_endsWith(s1, n1, s2, n2)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool endsWith(const UString& s1,           const UString& s2)           { return endsWith(U_STRING_TO_PARAM(s1), U_STRING_TO_PARAM(s2)); }
   static bool endsWith(const UString& s1,           const char* s2, uint32_t n2) { return endsWith(U_STRING_TO_PARAM(s1), s2, n2); }
   static bool endsWith(const char* s1, uint32_t n1, const UString& s2)           { return endsWith(s1, n1,                U_STRING_TO_PARAM(s2)); }

   // SUBSTITUTE: replace all occurrences of 'a' with 'b'

   static UString substitute(const char* s, uint32_t n, const char* a, uint32_t n1, const char* b, uint32_t n2);
   static UString substitute(const UString& s,                char  a,                    char  b)              { return substitute(U_STRING_TO_PARAM(s), &a,  1, &b,  1); }
   static UString substitute(const char* s, uint32_t n,       char  a,              const char* b, uint32_t n2) { return substitute(s, n,                 &a,  1,  b, n2); }
   static UString substitute(const UString& s,                char  a,              const char* b, uint32_t n2) { return substitute(U_STRING_TO_PARAM(s), &a,  1,  b, n2); }
   static UString substitute(const UString& s,          const char* a, uint32_t n1,       char  b)              { return substitute(U_STRING_TO_PARAM(s),  a, n1, &b,  1); }
   static UString substitute(const UString& s,          const char* a, uint32_t n1, const char* b, uint32_t n2) { return substitute(U_STRING_TO_PARAM(s),  a, n1,  b, n2); }

   static UString substitute(const char* s, uint32_t n, const UString& a, const UString& b) { return substitute(s, n,                 U_STRING_TO_PARAM(a), U_STRING_TO_PARAM(b)); }
   static UString substitute(const UString& s,          const UString& a, const UString& b) { return substitute(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(a), U_STRING_TO_PARAM(b)); }

   static UString substitute(const char* s, uint32_t n, UVector<UString>& vec);
   static UString substitute(const UString& s,          UVector<UString>& vec) { return substitute(U_STRING_TO_PARAM(s), vec); }

   static UString substituteIds(const char* s, uint32_t n, UVector<UString>& vec);
   static UString substituteIds(const UString& s,          UVector<UString>& vec) { return substituteIds(U_STRING_TO_PARAM(s), vec); }

   // ERASE

   static UString erase(const UString& s, char a)                     { return substitute(U_STRING_TO_PARAM(s), &a, 1,                U_NULLPTR, 0); } 
   static UString erase(const UString& s, const UString& a)           { return substitute(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(a), U_NULLPTR, 0); }
   static UString erase(const UString& s, const char* a, uint32_t n1) { return substitute(U_STRING_TO_PARAM(s), a, n1,                U_NULLPTR, 0); }

   // dos2unix: '\n' <=> '\r\n' convertor

   static UString dos2unix(const UString& s, bool unix2dos = false);

   // convert tabs to spaces

   static UString expandTab(const char* s, uint32_t n, int tab = 3);
   static UString expandTab(const UString& s,          int tab = 3) { return expandTab(U_STRING_TO_PARAM(s), tab); }

   // expand path (~/... and ~user/... plus $var and $var/...)

   static UString expandPath(const char* s, uint32_t n, const UString* environment);
   static UString expandPath(const UString& s,          const UString* environment) { return expandPath(U_STRING_TO_PARAM(s), environment); }

   // prepare for environment variables (check if some of them need quoting...)

   static UString prepareForEnvironmentVar(const char* s, uint32_t n);

   static UString prepareForEnvironmentVar(const UString& env) { return prepareForEnvironmentVar(U_STRING_TO_PARAM(env)); }

   // expand environment variables

   static UString getEnvironmentVar(const char* s, uint32_t n, const UString* env);
   static UString getEnvironmentVar(const UString& name,       const UString* env) { return getEnvironmentVar(U_STRING_TO_PARAM(name), env); }

   // recursively expand environment variables if needed

   static UString expandEnvironmentVar(const char* s, uint32_t n, const UString* env);
   static UString expandEnvironmentVar(const UString& s,          const UString* env) { return expandEnvironmentVar(U_STRING_TO_PARAM(s), env); }

   static UString getPidProcess()
      {
      U_TRACE_NO_PARAM(0, "UStringExt::getPidProcess()")

      UString value(22U);

      U_MEMCPY(value.data(), u_pid_str, u_pid_str_len);

      value.size_adjust(u_pid_str_len);

      U_RETURN_STRING(value);
      }

   static UString evalExpression(const UString& expr, const UString& environment);

   // Within a string we can count number of occurrence of another string by using substr_count function.
   // This function takes the main string and the search string as inputs and returns number of time search string is found inside the main string

   static __pure uint32_t substr_count(const char* s, uint32_t n, const char* a, uint32_t n1)
      {
      U_TRACE(0, "UStringExt::substr_count(%.*S,%u,%.*S,%u)", n, s, n, n1, a, n1)

      uint32_t num    = 0;
      const char* ptr = s;
      const char* end = s + n;

      while (true)
         {
         ptr = (const char*) u_find(ptr, end - ptr, a, n1);

         if (ptr == U_NULLPTR) U_RETURN(num);

         ++num;

         ptr += n1;
         }
      }

   static uint32_t substr_count(const UString& s,       char  a)             { return substr_count(U_STRING_TO_PARAM(s), &a, 1); }
   static uint32_t substr_count(const UString& s, const char* a, uint32_t n) { return substr_count(U_STRING_TO_PARAM(s),  a, n); }
   static uint32_t substr_count(const UString& s, const UString& a)          { return substr_count(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(a)); }

   // manage escaping for delimiter character

   static UString removeEscape(const char* s, uint32_t n);
   static UString insertEscape(const char* s, uint32_t n, char delimiter = '"');

   static UString removeEscape(const UString& s)                       { return removeEscape(U_STRING_TO_PARAM(s)); }
   static UString insertEscape(const UString& s, char delimiter = '"') { return insertEscape(U_STRING_TO_PARAM(s), delimiter); }

   // Returns a string that has whitespace removed from the start and the end (leading and trailing)

   static UString trim(const char* s, uint32_t n);

   static UString trim(const UString& s) { return trim(U_STRING_TO_PARAM(s)); }

   // Returns a string that has any printable character which is not a space or
   // an alphanumeric character removed from the start and the end (leading and trailing)

   static UString trimPunctuation(const char* s, uint32_t n);

   static UString trimPunctuation(const UString& s) { return trimPunctuation(U_STRING_TO_PARAM(s)); }

   // returns a string that has whitespace removed from the start and the end, and
   // which has each sequence of internal whitespace replaced with a single space

   static UString simplifyWhiteSpace(const char* s, uint32_t n);

   static UString simplifyWhiteSpace(const UString& s) { return simplifyWhiteSpace(U_STRING_TO_PARAM(s)); }

   // returns a string that has suppressed all whitespace 

   static UString removeWhiteSpace(const char* s, uint32_t n);

   static UString removeWhiteSpace(const UString& s) { return removeWhiteSpace(U_STRING_TO_PARAM(s)); }

   // returns a string that has suppressed repeated empty lines

   static UString removeEmptyLine(const char* s, uint32_t n);

   static UString removeEmptyLine(const UString& s) { return removeEmptyLine(U_STRING_TO_PARAM(s)); }

   // Minifies CSS/JS by removing comments and whitespaces

   static UString minifyCssJs(const char* s, uint32_t n);

   static UString minifyCssJs(const UString& s) { return minifyCssJs(U_STRING_TO_PARAM(s)); }

   // ----------------------------------------------------------------------------------------
   // Sort two version numbers, comparing equivalently seperated strings of digits numerically
   // ----------------------------------------------------------------------------------------
   // Returns a positive number if (a  > b)
   // Returns a negative number if (a  < b)
   // Returns zero              if (a == b)
   // ----------------------------------------------------------------------------------------
   static int compareversion(const char* a, uint32_t n1, const char* b, uint32_t n2) __pure;

   static int compareversion(const UString& s, const UString& a)          { return compareversion(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(a)); }
   static int compareversion(const UString& s, const char* a, uint32_t n) { return compareversion(U_STRING_TO_PARAM(s), a, n); }

   static int qscompver(const void* p, const void* q)
      {
      U_TRACE(0, "UStringExt::qscompver(%p,%p)", p, q)

#  ifndef U_STDCPP_ENABLE
      return compareversion(U_STRING_TO_PARAM(**(UStringRep**)p), U_STRING_TO_PARAM(**(UStringRep**)q));
#  else
      return (compareversion(U_STRING_TO_PARAM(*(UStringRep*)p), U_STRING_TO_PARAM(*(UStringRep*)q)) < 0);
#  endif
      }

   // Verifies that the passed string is actually an e-mail address

   static bool isEmailAddress(const UString& s)
      {
      U_TRACE(0, "UStringExt::isEmailAddress(%V)", s.rep)

      if (u_validate_email_address(U_STRING_TO_PARAM(s))) U_RETURN(true);

      U_RETURN(false);
      }

   // Gived the name retrieve pointer on value elements from headers "name1:value1\nname2:value2\n"...

   static const char* getValueFromName(const UString& request, uint32_t pos, uint32_t len, const UString& name, bool nocase)
      { return getValueFromName(request, pos, len, U_STRING_TO_PARAM(name), nocase); }

   static const char* getValueFromName(const UString& buffer, uint32_t pos, uint32_t len, const char* name, uint32_t name_len, bool nocase) __pure;

   // Retrieve information on form elements as couple <name1>=<value1>&<name2>=<value2>&...

   static uint32_t getNameValueFromData(const UString& content, UVector<UString>& name_value, const char* delim, uint32_t dlen);

#  define U_TOKEN_NM 4U
#  define U_TOKEN_LN (U_TOKEN_NM + 8U)

   // -----------------------------------------------------------------------------------------------------------------------
   // Very simple RPC-like layer
   //
   // Requests and responses are build of little packets each containing a U_TOKEN_NM-byte ascii token,
   // an 8-byte hex value or length, and optionally data corresponding to the length
   // -----------------------------------------------------------------------------------------------------------------------

   // built token name (U_TOKEN_NM characters) and value (32-bit int, as 8 hex characters)

   static void buildTokenInt(const char* token, uint32_t value, UString& buffer)
      {
      U_TRACE(0, "UStringExt::buildTokenInt(%S,%u,%V)", token, value, buffer.rep)

      U_INTERNAL_ASSERT_POINTER(token)
      U_INTERNAL_ASSERT(u__strlen(token, __PRETTY_FUNCTION__) == U_TOKEN_NM)

      uint32_t start = buffer.size();

      char* ptr = buffer.c_pointer(start);

      U_MEMCPY(ptr, token, U_TOKEN_NM);

      u_int2hex(ptr + U_TOKEN_NM, value);

      buffer.size_adjust(start + U_TOKEN_LN);
      }

   static void buildTokenString(const char* token, const UString& value, UString& buffer)
      {
      U_TRACE(0, "UStringExt::buildTokenString(%S,%V,%p)", token, value.rep, &buffer)

      uint32_t sz = value.size();

      buildTokenInt(token, sz, buffer);

      (void) buffer.append(value.data(), sz);
      }

   static void buildTokenVector(const char* token, UVector<UString>& vec, UString& buffer)
      {
      U_TRACE(0, "UStringExt::buildTokenVector(%S,%p,%p)", token, &vec, &buffer)

      uint32_t argc = vec.size();

      buildTokenInt(token, argc, buffer);

      for (uint32_t i = 0; i < argc; ++i) buildTokenString("ARGV", vec[i], buffer);
      }
};

#endif
