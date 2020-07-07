// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    string_ext.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/file.h>
#include <ulib/tokenizer.h>
#include <ulib/base/miniz/miniz.h>
#include <ulib/utility/string_ext.h>

#ifdef USE_LIBPCRE
#  include <ulib/pcre/pcre.h>
#endif
#ifdef USE_LIBZ
#  include <ulib/base/coder/gzio.h>
#elif !defined(Z_OK)
#  define Z_OK 0
#endif
#ifdef USE_LIBZOPFLI
#  include <zopfli.h>
#endif
#ifdef USE_LIBEXPAT
#  include <ulib/xml/expat/xml2txt.h>
#endif

#ifndef _MSWINDOWS_
#  include <pwd.h>
#endif

#ifndef U_LOG_DISABLE
const char* UStringExt::deflate_agent = "gzip";
#endif

#ifdef USE_LIBSSL
UString UStringExt::BIOtoString(BIO* bio)
{
   U_TRACE(1, "UStringExt::BIOtoString(%p)", bio)

   char* buffer;
   long len = BIO_get_mem_data(bio, &buffer);

   if (len > 0)
      {
      UString result((const void*)buffer, len);

      // only bio needs to be freed :)

      (void) BIO_set_close(bio, BIO_CLOSE); // So BIO_free() free BUF_MEM

      (void) U_SYSCALL(BIO_free, "%p", bio);

      U_RETURN_STRING(result);
      }

   return UString::getStringNull();
}

UString UStringExt::ASN1TimetoString(ASN1_GENERALIZEDTIME* t)
{
   U_TRACE(1, "UStringExt::ASN1TimetoString(%p)", t)

   UString result;

   if (t)
      {
      BIO* bio = (BIO*) U_SYSCALL(BIO_new, "%p", BIO_s_mem());

      (void) U_SYSCALL(ASN1_GENERALIZEDTIME_print, "%p,%p", bio, t);

      result = BIOtoString(bio);
      }

   U_RETURN_STRING(result);
}
#endif

// Replace parts of a string using regular expressions. This method is the counterpart of the perl s// operator.
// It replaces the substrings which matched the given regular expression with the supplied string

#ifdef USE_LIBPCRE
UString UStringExt::pregReplace(const UString& pattern, const UString& replacement, const UString& subject)
{
   U_TRACE(0, "UStringExt::pregReplace(%V,%V,%V)", pattern.rep, replacement.rep, subject.rep)

   return UPCRE(pattern, PCRE_FOR_REPLACE).replace(subject, replacement);
}

UString UStringExt::sanitize(const UString& input)
{
   U_TRACE(0, "UStringExt::sanitize(%V)", input.rep)

   /*
   static UPCRE* strip1;
   static UPCRE* strip2;
   static UPCRE* strip3;
   static UPCRE* strip4;

   if (strip1 == 0)
      {
      U_NEW_ULIB_OBJECT(UPCRE, strip1, UPCRE(U_STRING_FROM_CONSTANT("@<script[^>]*?>.*?</script>@si"), 0U)); // Strip out javascript
      U_NEW_ULIB_OBJECT(UPCRE, strip2, UPCRE(U_STRING_FROM_CONSTANT("@<[/!]*?[^<>]*?>@si"),            0U)); // Strip out HTML tags
      U_NEW_ULIB_OBJECT(UPCRE, strip3, UPCRE(U_STRING_FROM_CONSTANT("@<style[^>]*?>.*?</style>@siU"),  0U)); // Strip style tags properly
      U_NEW_ULIB_OBJECT(UPCRE, strip4, UPCRE(U_STRING_FROM_CONSTANT("@<![sS]*?--[ \t\n\r]*>@"),        0U)); // Strip multi-line comments

      strip1->study();
      strip2->study();
      strip3->study();
      strip4->study();
      }

   UString result = strip1->replace(input,  UString::getStringNull());
           result = strip2->replace(result, UString::getStringNull());
           result = strip3->replace(result, UString::getStringNull());
           result = strip4->replace(result, UString::getStringNull());

   U_RETURN_STRING(result);
   */

   U_RETURN_STRING(input);
}
#endif

#ifdef USE_LIBEXPAT
UString UStringExt::stripTags(const UString& html, UString* list_tags_allowed)
{
   U_TRACE(0, "UStringExt::stripTags(%V,%p)", html.rep, list_tags_allowed)

   UString tag_list, result;

   if (list_tags_allowed) tag_list = *list_tags_allowed;

   UXml2Txt converter(tag_list, false, true);

   if (converter.parse(html)) result = converter.getText();

   U_RETURN_STRING(result);
}
#endif

UString UStringExt::numberToString(uint64_t n)
{
   U_TRACE(0, "UStringExt::numberToString(%llu)", n)

   UString x(32U);
   unsigned int i;
   uint64_t u = 1024ULL;

   for (i = 0; i < U_CONSTANT_SIZE("bKMGTPEZY"); ++i)
      {
      if ((n / u) == 0) break;

      u *= 1024ULL;
      }

   if (i == 0)
      {
      x.setFromNumber64(n);

      x.push_back('b');
      }
   else
      {
      float fsize = (float)((double)n/(u/1024ULL));

      x.snprintf(U_CONSTANT_TO_PARAM("%.1f%c"), fsize, "bKMGTPEZY"[i]);
      }

   U_RETURN_STRING(x);
}

UString UStringExt::expandTab(const char* s, uint32_t n, int tab)
{
   U_TRACE(1, "UStringExt::expandTab(%.*S,%u,%d)", n, s, n, tab)

   void* p;
   UString x(U_CAPACITY);
   uint32_t start = 0, len;

   while ((p = (void*)memchr(s + start, '\t', n - start)))
      {
      uint32_t _end = (const char*)p - s;

      if (_end > start)
         {
         len = _end - start;

         (void) x.reserve(len + tab);

         if (len)
            {
            U_MEMCPY(x.pend(), s + start, len);

            x.rep->_length += len;
            }
         }

      uint32_t num = tab - (x.rep->_length % tab);

      U_INTERNAL_DUMP("start = %u _end = %u num = %u", start, _end, num)

      char* r = x.rep->data();

      while (num--) r[x.rep->_length++] = ' ';

      start = _end + 1;
      }

   len = n - start;

   if (len) (void) x.append(s + start, len);

   (void) x.shrink(true);

   U_RETURN_STRING(x);
}

UString UStringExt::substitute(const char* s, uint32_t n, const char* a, uint32_t n1, const char* b, uint32_t n2)
{
   U_TRACE(1, "UStringExt::substitute(%.*S,%u,%.*S,%u,%.*S,%u)", n, s, n, n1, a, n1, n2, b, n2)

   U_INTERNAL_ASSERT_MAJOR(n, 0)
   U_INTERNAL_ASSERT_MAJOR(n1, 0)

   uint32_t capacity;
   bool breserve = false;

   if (n2 <= n1) capacity = n;
   else
      {
      capacity = n2*((n+n1)/n1);

      if (capacity > (256U * 1024U * 1024U)) capacity = (breserve = true, (256U * 1024U * 1024U)); // worst case... 
      }

   uint32_t len1;
   const char* p1;
   UString x(capacity);
   char* p2 = x.data();

   while ((p1 = (const char*)u_find(s, n, a, n1)))
      {
      len1 = (p1-s);

      U_INTERNAL_DUMP("len1 = %u", len1)

      if (breserve)
         {
         uint32_t len2 = len1 + n2;

         x.rep->_length = x.distance(p2);

         if (x.space() < len2)
            {
            UString::_reserve(x, x.rep->_length + len2);

            p2 = x.pend();
            }
         }

      if (len1)
         {
         U_MEMCPY(p2, s, len1);
                  p2 +=  len1;

         s  = p1;
         n -= len1;
         }

      n -= n1;
      s += n1;

      if (n2)
         {
         U_MEMCPY(p2, b, n2);
                  p2 +=  n2;
         }
      }

   x.rep->_length = x.distance(p2);

   U_INTERNAL_DUMP("n = %u", n)

   if (n)
      {
      if (breserve &&
          x.space() < n)
         {
         UString::_reserve(x, x.rep->_length + n);

         p2 = x.pend();
         }

      U_MEMCPY(p2, s, n);

      x.rep->_length += n;
      }

   U_INTERNAL_ASSERT(x.invariant())

   U_RETURN_STRING(x);
}

UString UStringExt::substitute(const char* s, uint32_t len, UVector<UString>& vec)
{
   U_TRACE(1, "UStringExt::substitute(%.*S,%u,%p)", len, s, len, &vec)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

   uint32_t n = vec.size();

   U_INTERNAL_ASSERT_EQUALS(n & 1, 0)

   if (n == 2) return substitute(s, len, U_STRING_TO_PARAM(vec[0]), U_STRING_TO_PARAM(vec[1]));

   char c;
   int32_t i;
   UString item;
   bool breserve = false, bdigit = false, bspace = false, bdollar;
   uint32_t n1 = 0, n2 = 0, capacity, mask_lower = 0, mask_upper = 0, dollar = 0;

   uint32_t maskFirstChar[] = { 0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010,
                                0x00000020, 0x00000040, 0x00000080, 0x00000100, 0x00000200,
                                0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000,
                                0x00008000, 0x00010000, 0x00020000, 0x00040000, 0x00080000,
                                0x00100000, 0x00200000, 0x00400000, 0x00800000, 0x01000000,
                                0x02000000 };

   for (i = 0; i < (int32_t)n; i += 2)
      {
      item = vec[i];

      n1 +=     item.size();
      n2 += vec[i+1].size();

      c = item.first_char();

      if (c == '$')
         {
         ++dollar;

         continue;
         }

           if (u__isdigit(c)) bdigit = true;
      else if (u__isspace(c)) bspace = true;
      else if (u__islower(c)) mask_lower |= maskFirstChar[c-'a'];
      else if (u__isupper(c)) mask_upper |= maskFirstChar[c-'A'];
      }

   U_INTERNAL_DUMP("n1 = %u n2 = %u", n1, n2)

   if (n2 <= n1) capacity = len;
   else
      {
      capacity = n2*((len+n1)/n1);

      if (capacity > (256U * 1024U * 1024U)) capacity = (breserve = true, (256U * 1024U * 1024U)); // worst case... 
      }

   uint32_t len1;
   const char* p1;
   UString x(capacity);
   char* p2 = x.data();
   const char* end = s + len;

   bdollar = (dollar == n);

   U_INTERNAL_DUMP("mask_lower = %B mask_upper = %B bdigit = %b bspace = %b dollar = %u bdollar = %b",
                    mask_lower,     mask_upper,     bdigit,     bspace,     dollar,     bdollar)

loop:
   for (p1 = s; p1 < end; ++p1)
      {
      c = *p1;

      U_INTERNAL_DUMP("c = %C p1 = %.10S", c, p1)

      if (c == '$')
         {
         if (dollar) goto chk;

         continue;
         }

      if (bdollar) continue;

      if ((u__isdigit(c) && bdigit == false)                          ||
          (u__isspace(c) && bspace == false)                          ||
          (u__islower(c) && (mask_lower & maskFirstChar[c-'a']) == 0) ||
          (u__isupper(c) && (mask_upper & maskFirstChar[c-'A']) == 0))
         {
         continue;
         }

chk:  for (i = 0; i < (int32_t)n; i += 2)
         {
         item = vec[i];

         if (memcmp(p1, U_STRING_TO_PARAM(item)) == 0)
            {
            n1 =  item.size();
            n2 = (item = vec[i+1]).size();

            U_INTERNAL_DUMP("n1 = %u item(%u) = %V", n1, n2, item.rep)

            goto found;
            }
         }
      }

   if (p1 < end)
      {
found:
      len1 = (p1-s);

      U_INTERNAL_DUMP("len1 = %u", len1)

      if (breserve)
         {
         uint32_t len2 = len1 + n2;

         x.rep->_length = x.distance(p2);

         if (x.space() < len2)
            {
            UString::_reserve(x, x.rep->_length + len2);

            p2 = x.pend();
            }
         }

      if (len1)
         {
         U_MEMCPY(p2, s, len1);
                  p2 +=  len1;

           s  = p1;
         len -= len1;
         }

      len -= n1;
        s += n1;

      if (n2)
         {
         U_MEMCPY(p2, item.data(), n2);
                  p2 +=            n2;
         }

      goto loop;
      }

   x.rep->_length = x.distance(p2);

   U_INTERNAL_DUMP("len = %u", len)

   if (len)
      {
      if (breserve &&
          x.space() < len)
         {
         UString::_reserve(x, x.rep->_length + len);

         p2 = x.pend();
         }

      U_MEMCPY(p2, s, len);

      x.rep->_length += len;
      }

   U_INTERNAL_ASSERT(x.invariant())

   U_RETURN_STRING(x);
}

// The format s uses positional identifiers indicated by a dollar sign ($) and single digit
// positional ids to indicate which substitution arguments to use at that location within the format s

UString UStringExt::substituteIds(const char* s, uint32_t len, UVector<UString>& vec)
{
   U_TRACE(1, "UStringExt::substituteIds(%.*S,%u,%p)", len, s, len, &vec)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

   UString item;
   uint32_t len1;
   const char* p1;
   bool breserve = false;
   const char* end = s + len;
   uint32_t i, n = vec.size(), n1 = n*2, n2 = 0, capacity, ids, old_ids = U_NOT_FOUND;

   for (i = 0; i < n; ++i) n2 += vec[i].size();

   if (n2 <= n1) capacity = len;
   else
      {
      capacity = n2*((len+n1)/n1);

      if (capacity > (256U * 1024U * 1024U)) capacity = (breserve = true, (256U * 1024U * 1024U)); // worst case... 
      }

   U_INTERNAL_DUMP("n1 = %u n2 = %u breserve = %b", n1, n2, breserve)

   UString x(capacity);
   char* p2 = x.data();

loop:
   for (p1 = s; p1 < end; ++p1)
      {
      U_INTERNAL_DUMP("p1 = %.10S", p1)

      if (*p1 != '$') continue;

      if (u__isdigit(p1[2]) == false) ids = (n1 = 2, p1[1] - '0');
      else
         {
         for (i = 3; u__isdigit(p1[i]); ++i) {}

         ids = (n1 = i, u__strtoul(p1+1, n1-1));
         }

      U_INTERNAL_DUMP("n1 = %u ids = %u old_ids = %u", n1, ids, old_ids)

      if (ids != old_ids)
         {
         old_ids = ids;

         n2 = (item = vec.at(ids)).size();

         U_INTERNAL_DUMP("item(%u) = %V", n2, item.rep)

         U_INTERNAL_ASSERT_MAJOR(n2, 0)
         }

      len1 = (p1-s);

      U_INTERNAL_DUMP("len1 = %u", len1)

      if (breserve)
         {
         uint32_t len2 = len1 + n2;

         x.rep->_length = x.distance(p2);

         if (x.space() < len2)
            {
            UString::_reserve(x, x.rep->_length + len2);

            p2 = x.pend();
            }
         }

      if (len1)
         {
         U_MEMCPY(p2, s, len1);
                  p2 +=  len1;

           s  = p1;
         len -= len1;
         }

      len -= n1;
        s += n1;

      U_MEMCPY(p2, item.data(), n2);
               p2 +=            n2;

      goto loop;
      }

   x.rep->_length = x.distance(p2);

   U_INTERNAL_DUMP("len = %u", len)

   if (len)
      {
      if (breserve &&
          x.space() < len)
         {
         UString::_reserve(x, x.rep->_length + len);

         p2 = x.pend();
         }

      U_MEMCPY(p2, s, len);

      x.rep->_length += len;
      }

   U_INTERNAL_ASSERT(x.invariant())

   U_RETURN_STRING(x);
}

UString UStringExt::eraseIds(const char* s, uint32_t len)
{
   U_TRACE(0, "UStringExt::eraseIds(%.*S,%u)", len, s, len)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

   const char* p1;
   const char* end = s + len;

   UString x(len);
   char* p2 = x.data();

loop:
   for (p1 = s; p1 < end; ++p1)
      {
      U_INTERNAL_DUMP("p1 = %.10S", p1)

      if (*p1 != '$') continue;

      len = (p1-s);

      U_INTERNAL_DUMP("len = %u", len)

      if (len)
         {
         U_MEMCPY(p2, s, len);
                  p2 +=  len;
         }

      s = p1+1;

      while (u__isname(*s)) { ++s; }

      goto loop;
      }

   len = (end-s);

   U_INTERNAL_DUMP("len = %u", len)

   if (len)
      {
      U_MEMCPY(p2, s, len);
               p2 +=  len;
      }

   x.rep->_length = x.distance(p2);

   U_INTERNAL_ASSERT(x.invariant())

   U_RETURN_STRING(x);
}

// dos2unix: '\n' <=> '\r\n' convertor

UString UStringExt::dos2unix(const UString& s, bool unix2dos)
{
   U_TRACE(0, "UStringExt::dos2unix(%V,%b)", s.rep, unix2dos)

   UString result(s.size() * 2);

   const char* ptr   = s.data();
   const char* _end  = s.pend();
         char* str   = result.data();
         char* start = str;

   while (ptr < _end)
      {
      char c = *ptr++;

      if (c == '\r') continue;
      if (c == '\n')
         {
         if (unix2dos) *str++ = '\r';

         *str++ = '\n';

         continue;
         }

      *str++ = c;
      }

   result.size_adjust(str - start);

   U_RETURN_STRING(result);
}

UString UStringExt::expandPath(const char* s, uint32_t n, const UString* environment)
{
   U_TRACE(0, "UStringExt::expandPath(%.*S,%u,%p)", n, s, n, environment)

   UString x(n+100);

   char c = *s;

   if (c == '~' ||
       c == '$')
      {
      UString value;
      uint32_t _end = 1;

      while (_end < n && s[_end] != '/') ++_end;

      U_INTERNAL_DUMP("_end = %u", _end)

      if (_end == 1)
         {
         if (c == '$') goto end;

         // expand ~/...

         value = getEnvironmentVar(U_CONSTANT_TO_PARAM("HOME"), environment);
         }
      else if (c == '$')
         {
         // expand $var... and $var/...

         value = getEnvironmentVar(s + 1, _end - 1, environment);
         }
      else
         {
         // expand ~user/...

         char buffer[128];

         U_INTERNAL_ASSERT_MINOR(_end, sizeof(buffer))

         U_MEMCPY(buffer, s + 1, _end - 1);

         buffer[_end-1] = '\0';

         struct passwd* pw = (struct passwd*) U_SYSCALL(getpwnam, "%S", buffer);

         if (pw && pw->pw_dir) (void) value.assign(pw->pw_dir);
         }

      s += _end;
      n -= _end;

      (void) x.append(value);
      }

end:
   if (n) (void) x.append(s, n);

   U_RETURN_STRING(x);
}

// prepare for environment variables (check if some of them need quoting...)

UString UStringExt::prepareForEnvironmentVar(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::prepareForEnvironmentVar(%.*S,%u)", n, s, n)

   bool quoted;
   const char* p;
   const char* ptr;
   const char* ptr1;
   uint32_t len, sz = 0;
   UString result(n + (n / 4));
   char c = 0, delimiter = (memchr(s, '\n', n) ? '\n' : ' ');

   char* str = result.data();
   const char* _end = s + n - 1;

   while (s < _end)
      {
      if (u__isspace(*s))
         {
         ++s;

         continue;
         }

      U_INTERNAL_DUMP("s = %.*S", 10, s)

      if (*s == '#') // skip line comment
         {
         s = (const char* restrict) memchr(s, delimiter, _end - s + 1);

         if (s == U_NULLPTR) goto end;

         continue;
         }

      p = s;
      s = (const char* restrict) memchr(s, '=', _end - s + 1);

      if (s == U_NULLPTR) goto end;

      U_INTERNAL_DUMP("name = %.*S", s - p, p)

      ++s;
      quoted = false;

      if (*p == '\'')
         {
         s = (const char* restrict) memchr(s, '\'', _end - s + 1);

         if (s == U_NULLPTR) goto end;

         len = (++s - p);

         U_INTERNAL_DUMP("copy = %.*S", len, p)
         }
      else
         {
         s = (const char* restrict) memchr(s, delimiter, _end - s + 1);

         if (s == U_NULLPTR) s = _end;

         ptr = s;

         U_INTERNAL_DUMP("*ptr = %C", *ptr)

         for (c = *ptr; u__isspace(c) && --ptr > p; c = *ptr) {}

         len = (ptr - p) + 1;

         U_INTERNAL_ASSERT_MAJOR(len, 0)

         ptr1 = p;

         while (++ptr1 < ptr)
            {
            c = *ptr1;

            if (c == '=')
               {
               U_INTERNAL_DUMP("name = %.*S value = %.*S", ptr1 - p, p, ptr - ptr1, ptr1+1)

               while (++ptr1 < ptr)
                  {
                  c = *ptr1;

                  if (c == ' ' ||
                      c == '"')
                     {
                     quoted    = true;
                     str[sz++] = '\'';

                     break;
                     }
                  }

               break;
               }

            U_INTERNAL_ASSERT(u__isname(c))
            }
         }

      U_MEMCPY(str + sz, p, len);

      sz += len;

      if (quoted) str[sz++] = '\'';

      str[sz++] = '\n';
      }

end:
   result.size_adjust(sz);

   U_INTERNAL_DUMP("result(%d) = %#V", sz, result.rep)

   U_RETURN_STRING(result);
}

// recursively expand environment variables if needed

UString UStringExt::expandEnvironmentVar(const char* s, uint32_t n, const UString* environment)
{
   U_TRACE(0, "UStringExt::expandEnvironmentVar(%.*S,%u,%p)", n, s, n, environment)

   char* new_ptr;
   const char* p;
   UString value, result(n+500U);
   const char* var_ptr = U_NULLPTR;
   uint32_t var_size, new_size = 0;

   while ((p = (const char*) memchr(s, '$', n)))
      {
      U_INTERNAL_DUMP("p = %.*S", 10, p)

      uint32_t len = p - s;

      n  -= len;

      // read name=$var
      //          =>...

      uint32_t _end = 1;

      while (_end < n &&
             u__isname(p[_end]))
         {
         U_INTERNAL_ASSERT_DIFFERS(p[_end], '$')
         U_INTERNAL_ASSERT_EQUALS(u__isspace(p[_end]), false)

         ++_end;
         }

      U_INTERNAL_DUMP("len = %u n = %u _end = %u", len, n, _end)

      if (_end == 1) var_size = 0;
      else
         {
         var_ptr  = p + 1;
         var_size = _end - 1;

         U_INTERNAL_DUMP("var = %.*S", var_size, var_ptr)

         value = getEnvironmentVar(var_ptr, var_size, environment);

         if (new_size &&
             value.find('$', 0U) != U_NOT_FOUND)
            {
            value = getEnvironmentVar(var_ptr, var_size, &result);
            }

         var_ptr  = value.data();
         var_size = value.size();
         }

      (void) result.reserve(new_size + len + var_size);

      new_ptr = result.c_pointer(new_size);

      if (len) U_MEMCPY(new_ptr, s, len);

      if (var_size)
         {
         U_MEMCPY(new_ptr + len, var_ptr, var_size);

         new_size += var_size;
         }

      new_size += len;

      result.size_adjust(new_size);

      s  = p + _end;
      n -=     _end;
      }

   if (n) (void) result.append(s, n);

   U_RETURN_STRING(result);
}

UString UStringExt::getEnvironmentVar(const char* s, uint32_t n, const UString* environment)
{
   U_TRACE(1, "UStringExt::getEnvironmentVar(%.*S,%u,%p)", n, s, n, environment)

   UString value(300U);

   if (environment)
      {
      char c, c1;
      const char* end;
      uint32_t start = 0;
      bool quoted, bexpand;

      // NB: check if param 's' is a environment-var
loop:
      start = environment->find(s, start, n);

      if (start == U_NOT_FOUND) goto next;

      c = '\0';

      if (start)
         {
         c = environment->c_char(start-1);

         U_INTERNAL_DUMP("c = %C", c)

         if (u__isname(c) ||
             c == '#') // NB: check if commented...
            {
            start += n;

            goto loop;
            }
         }

      start += n;

      c1 = environment->c_char(start);

      U_INTERNAL_DUMP("c1 = %C", c1)

      if (c1 != '=') goto loop;

      quoted  = u__isquote(c);
      bexpand = false;

      U_INTERNAL_DUMP("quoted = %b", quoted)

      s   = environment->c_pointer(++start);
      end = environment->pend();

      U_INTERNAL_DUMP("end - s = %ld", end - s)

      if (s < end)
         {
         const char* ptr = s;

         do {
            if ((c1 = *ptr) == '$') bexpand = true;

            if (quoted)
               {
               if (c1 != c ||
                   ptr[-1] == '\\')
                  {
                  continue;
                  }
               }
            else
               {
               if (u__isspace(c1) == false) continue;
               }

            U_INTERNAL_DUMP("ptr - s = %ld", ptr - s)

            if (ptr == s) goto end; // NB: name=<empty>...

            n = ptr - s;

            goto assign;
            }
         while (++ptr < end);

         n = end - s;

assign:  U_INTERNAL_DUMP("n = %u", n)

         U_INTERNAL_ASSERT_MAJOR(n, 0)

         if (bexpand) value = expandEnvironmentVar(s, n, environment);
         else  (void) value.assign(s, n);
         }
      }
   else
      {
next: char buffer[128];

      U_INTERNAL_ASSERT_MINOR(n, sizeof(buffer))

      U_MEMCPY(buffer, s,  n);

      buffer[n] = '\0';

      const char* ptr = U_SYSCALL(getenv, "%S", buffer);

      if (ptr) (void) value.assign(ptr);
      }
end:
   U_RETURN_STRING(value);
}

extern void* expressionParserAlloc(void* (*mallocProc)(size_t));
extern void  expressionParserFree(void* p, void (*freeProc)(void*));
extern void  expressionParserTrace(FILE* stream, char* zPrefix);
extern void  expressionParser(void* yyp, int yymajor, UString* yyminor, UString* result);

UString UStringExt::evalExpression(const UString& expr, const UString& environment)
{
   U_TRACE(0, "UStringExt::evalExpression(%V,%V)", expr.rep, environment.rep)

   int token_id;
   void* pParser;
   UString* ptoken;
   UTokenizer t(expr);
   UString token, result = *UString::str_true;

#ifdef USE_LIBMIMALLOC
   pParser = expressionParserAlloc(mi_malloc);
#else
   pParser = expressionParserAlloc(malloc);
#endif

   /*
#ifdef U_DEBUG
   (void) fprintf(stderr, "start parsing expr: \"%.*s\"\n", U_STRING_TO_TRACE(expr));
    expressionParserTrace(stderr, (char*)"parser: ");
#endif
   */

   while ((token_id = t.getTokenId(&token)) > 0)
      {
      if (token_id == U_TK_NAME)
         {
         token    = UStringExt::getEnvironmentVar(token, &environment);
         token_id = U_TK_VALUE;
         }
      else if (token_id == U_TK_PID)
         {
         token    = UStringExt::getPidProcess();
         token_id = U_TK_VALUE;
         }

      U_NEW_STRING(ptoken, UString(token));

      expressionParser(pParser, token_id, ptoken, &result);

      U_INTERNAL_DUMP("result = %V", result.rep)

      if (result.empty()) break;
      }

   expressionParser(pParser, 0, U_NULLPTR, &result);

#ifdef USE_LIBMIMALLOC
   expressionParserFree(pParser, mi_free);
#else
   expressionParserFree(pParser, free);
#endif

   /*
#if defined(U_DEBUG) && !defined(U_SUBSTR_INC_REF)
   (void) fprintf(stderr, "ended parsing expr: \"%.*s\"\n", U_STRING_TO_TRACE(expr));
   U_INTERNAL_DUMP("token.rep->parent->child = %d", token.rep->parent->child)
#endif
   */

   U_RETURN_STRING(result);
}

// Returns a string that has the delimiter escaped

UString UStringExt::insertEscape(const char* s, uint32_t n, char delimiter)
{
   U_TRACE(0, "UStringExt::insertEscape(%.*S,%u,%C)", n, s, n, delimiter)

   char* p;
   uint32_t sz, sz1 = 0;
   UString result(n * 2);
   char* str = result.data();
   const char* _end = s + n;

   while (s < _end)
      {
      p = (char*) memchr(s, delimiter, _end - s);

      if (p)
         {
         sz = p - s;

         U_MEMCPY(str, s, sz);

         s    = p + 1;
         str += sz;

         *str++ = '\\';
         *str++ = delimiter;

         sz1 += sz + 2;
         }
      else
         {
         sz = _end - s;

         U_MEMCPY(str, s, sz);

         sz1 += sz;

         break;
         }
      }

   result.size_adjust(sz1);

   U_RETURN_STRING(result);
}

// manage escaping for delimiter character

UString UStringExt::removeEscape(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::removeEscape(%.*S,%u,%C)", n, s, n)

   char* p;
   UString result(n);
   uint32_t sz, sz1 = 0;
   char* str = result.data();
   const char* _end = s + n;

   while (s < _end)
      {
      p = (char*) memchr(s, '\\', _end - s);

      if (p)
         {
         sz = p - s;

         U_MEMCPY(str, s, sz);

         s    = p + 1;
         str += sz;

         sz1 += sz;
         }
      else
         {
         sz = _end - s;

         U_MEMCPY(str, s, sz);

         sz1 += sz;

         break;
         }
      }

   result.size_adjust(sz1);

   U_RETURN_STRING(result);
}

// Returns a string that has whitespace removed from the start and the end

UString UStringExt::trim(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::trim(%.*S,%u)", n, s, n)

   if (n == 0) return UString::getStringNull();

   int32_t i = 0;
   UString result(n);

   // skip white space from start

   while (i < (int32_t)n && u__isspace(s[i])) ++i;

   U_INTERNAL_DUMP("i = %d", i)

   if (i < (int32_t)n) // not only white space
      {
      while (u__isspace(s[--n])) {} // skip white space from end

      n += 1 - i;

      U_MEMCPY(result.data(), s+i, n);

      result.size_adjust(n);
      }

   U_RETURN_STRING(result);
}

// Returns a string that has any printable character which is not a space or
// an alphanumeric character removed from the start and the end (leading and trailing)

UString UStringExt::trimPunctuation(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::trimPunctuation(%.*S,%u)", n, s, n)

   int32_t i = 0;
   UString result(n);

   // skip punctuation character from start

   while (i < (int32_t)n && u__ispunct(s[i])) ++i;

   U_INTERNAL_DUMP("i = %d", i)

   if (i < (int32_t)n) // not only punctuation character
      {
      while (u__ispunct(s[--n])) {} // skip punctuation character from end

      n += 1 - i;

      U_MEMCPY(result.data(), s+i, n);

      result.size_adjust(n);
      }

   U_RETURN_STRING(result);
}

// returns a string that has whitespace removed from the start and the end,
// and which has each sequence of internal whitespace replaced with a single space

UString UStringExt::simplifyWhiteSpace(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::simplifyWhiteSpace(%.*S,%u)", n, s, n)

   UString result(n);
   uint32_t sz1, sz = 0;
   char* str = result.data();

   const char* p;
   const char* _end = s + n;

   while (s < _end)
      {
      if (u__isspace(*s))
         {
         ++s;

         continue;
         }

      p = s++;

      while (s < _end &&
             u__isspace(*s) == false)
         {
         ++s;
         }

      sz1 = (s - p);

      U_MEMCPY(str + sz, p, sz1); // result.append(p, sz1);

      sz += sz1;

      if (++s < _end) str[sz++] = ' ';
      }

   if (sz && u__isspace(str[sz-1])) --sz;

   result.size_adjust(sz);

   U_RETURN_STRING(result);
}

// returns a string that has suppressed all whitespace

UString UStringExt::removeWhiteSpace(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::removeWhiteSpace(%.*S,%u)", n, s, n)

   UString result(n);
   uint32_t sz1, sz = 0;
   char* str = result.data();

   const char* p;
   const char* _end = s + n;

   while (s < _end)
      {
      if (u__isspace(*s))
         {
         ++s;

         continue;
         }

      p = s++;

      while (s < _end &&
             u__isspace(*s) == false)
         {
         ++s;
         }

      sz1 = (s - p);

      U_MEMCPY(str + sz, p, sz1); // result.append(p, sz1);

      sz += sz1;
      }

   if (sz && u__isspace(str[sz-1])) --sz;

   result.size_adjust(sz);

   U_RETURN_STRING(result);
}

// returns a string that has suppressed repeated empty lines

UString UStringExt::removeEmptyLine(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::removeEmptyLine(%.*S,%u)", n, s, n)

   UString result(n);
   uint32_t sz1, sz = 0;
   char* str = result.data();

   const char* p;
   const char* _end = s + n;

   while (s < _end)
      {
      if (u__islterm(*s))
         {
         ++s;

         continue;
         }

      p = s++;

      while (s < _end &&
             u__islterm(*s) == false)
         {
         ++s;
         }

      sz1 = (s - p);

      U_MEMCPY(str + sz, p, sz1); // result.append(p, sz1);

      sz += sz1;

      if (++s < _end) str[sz++] = '\n';
      }

   if (sz && u__islterm(str[sz-1])) --sz;

   result.size_adjust(sz);

   U_RETURN_STRING(result);
}

UString UStringExt::dirname(const char* path, uint32_t n)
{
   U_TRACE(0, "UStringExt::dirname(%.*S,%u)", n, path, n)

   const char* runp;
   const char* last_slash = (const char*) memrchr(path, '/', n); // Find last '/'

   if (last_slash         &&
       last_slash != path &&
       (n - (last_slash - path)) == 1)
      {
      // Determine whether all remaining characters are slashes

      for (runp = last_slash; runp != path; --runp) if (runp[-1] != '/') break;

      // The '/' is the last character, we have to look further

      if (runp != path) last_slash = (const char*) memrchr(path, '/', runp - path);
      }

   if (last_slash == U_NULLPTR)
      {
      // This assignment is ill-designed but the XPG specs require to
      // return a string containing "." in any case no directory part is
      // found and so a static and constant string is required

      U_RETURN_STRING(*UString::str_point);
      }

   // Determine whether all remaining characters are slashes

   for (runp = last_slash; runp != path; --runp) if (runp[-1] != '/') break;

   // Terminate the path

   if (runp != path) last_slash = runp;
   else
      {
      // The last slash is the first character in the string. We have to return "/".
      // As a special case we have to return "//" if there are exactly two slashes at the beginning of the string.
      // See XBD 4.10 Path Name Resolution for more information

      if (last_slash == path + 1) ++last_slash;
      else                          last_slash = path + 1;
      }

   UString result(path, (uint32_t)(last_slash - path));

   U_RETURN_STRING(result);
}

/**
 * Sort two version numbers, comparing equivalently seperated strings of digits numerically.
 *
 * Returns a positive number if (a > b)
 * Returns a negative number if (a < b)
 * Returns zero if (a == b)
 */

__pure int UStringExt::compareversion(const char* a, uint32_t alen, const char* b, uint32_t blen)
{
   U_TRACE(0, "UStringExt::compareversion(%.*S,%u,%.*S,%u)", alen, a, alen, blen, b, blen)

   if (a == b) U_RETURN(0);

   bool isnum;
   uint32_t apos2 = 0, bpos2 = 0;

   while (apos2 < alen &&
          bpos2 < blen)
      {
      uint32_t apos1 = apos2,
               bpos1 = bpos2;

      if (u__isdigit(a[apos2]))
         {
         isnum = true;

         while (apos2 < alen && u__isdigit(a[apos2])) apos2++;
         while (bpos2 < blen && u__isdigit(b[bpos2])) bpos2++;
         }
      else
         {
         isnum = false;

         while (apos2 < alen && !u__isdigit(a[apos2])) apos2++;
         while (bpos2 < blen && !u__isdigit(b[bpos2])) bpos2++;
         }

      U_INTERNAL_ASSERT_DIFFERS(apos1,apos2) 

      /**
       * isdigit(a[0]) != isdigit(b[0])
       *
       * arbitrarily sort the non-digit first
       */

      if (bpos1 == bpos2) U_RETURN(isnum ? 1 : -1);

      if (isnum)
         {
         /* skip numeric leading zeros */
         while (apos1 < alen && a[apos1] == '0') apos1++;
         while (bpos1 < blen && b[bpos1] == '0') bpos1++;

         /* if one number has more digits, it is greater */
         if (apos2-apos1 > bpos2-bpos1) U_RETURN(1);
         if (apos2-apos1 < bpos2-bpos1) U_RETURN(-1);
         }

      /* do an ordinary lexicographic string comparison */

      uint32_t n1 = apos2-apos1,
               n2 = bpos2-bpos1;

      int cval = memcmp(a+apos1, b+bpos1, U_min(n1, n2));

      if (cval) U_RETURN(cval < 1 ? -1 : 1);
      }

   /* ran out of characters in one string, without finding a difference */

   /* maybe they were the same version, but with different leading zeros */
   if (apos2 == alen && bpos2 == blen) U_RETURN(0);

   /* the version with a suffix remaining is greater */
   U_RETURN(apos2 < alen ? 1 : -1);
}

UString UStringExt::compress(const char* s, uint32_t sz)
{
   U_TRACE(0, "UStringExt::compress(%.*S,%u)", sz, s, sz)

   UString out(U_CONSTANT_SIZE(U_MINIZ_COMPRESS) + sizeof(uint32_t) + sz + 32);

   mz_ulong out_len   = out.capacity() - U_CONSTANT_SIZE(U_MINIZ_COMPRESS) + sizeof(uint32_t);
   unsigned char* ptr = (unsigned char*)out.data();

   // copy magic byte

   u_put_unalignedp32(ptr, U_MULTICHAR_CONSTANT32('\x89','M','N','Z')); // U_MINIZ_COMPRESS
                      ptr += 4;

   // copy original size

#if __BYTE_ORDER == __LITTLE_ENDIAN
   uint32_t size_original = sz;
#else
   uint32_t size_original = u_invert32(*(uint32_t*)&sz);
#endif

   *(int32_t*)ptr = *(int32_t*)&size_original;

#ifdef DEBUG
   int r =
#endif
   U_SYSCALL(mz_compress, "%p,%p,%p,%u", ptr + sizeof(uint32_t), &out_len, (const unsigned char*)s, sz);

   U_INTERNAL_ASSERT_EQUALS(r, Z_OK)

   out.rep->_length = U_CONSTANT_SIZE(U_MINIZ_COMPRESS) + sizeof(uint32_t) + out_len;

   U_INTERNAL_DUMP("compressed %u bytes into %lu bytes (%u%%)", sz, out_len, 100 - (out_len * 100 / sz))

   U_INTERNAL_ASSERT(UStringExt::isCompress(out))

   U_RETURN_STRING(out);
}

UString UStringExt::decompress(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringExt::decompress(%.*S,%u)", n, s, n)

   // check magic byte

   U_INTERNAL_ASSERT(UStringExt::isCompress(s))

   // read original size

   const char* ptr = (char*)s + U_CONSTANT_SIZE(U_MINIZ_COMPRESS);

#if __BYTE_ORDER == __LITTLE_ENDIAN
   uint32_t sz =            *(uint32_t*)ptr;
#else
   uint32_t sz = u_invert32(*(uint32_t*)ptr);
#endif

   U_INTERNAL_DUMP("sz = %u", sz)

   UString out(sz + 32);
   mz_ulong out_len = out.capacity();

#ifdef DEBUG
   int r =
#endif
   U_SYSCALL(mz_uncompress, "%p,%p,%p,%u", (unsigned char*)out.rep->data(), &out_len,
                                     (const unsigned char*)ptr + sizeof(uint32_t), n - U_CONSTANT_SIZE(U_MINIZ_COMPRESS) - sizeof(uint32_t));

   U_INTERNAL_ASSERT_EQUALS(r, Z_OK)

   U_INTERNAL_DUMP("decompressed %u bytes back into %lu bytes", n - U_CONSTANT_SIZE(U_MINIZ_COMPRESS) - sizeof(uint32_t), out_len)

   out.rep->_length = out_len;

   U_RETURN_STRING(out);
}

uint32_t UStringExt::ratio;
uint32_t UStringExt::ratio_threshold = 90; // NB: we accept compressed data only if ratio compression is better than 10%...

#ifdef USE_LIBBROTLI
UString UStringExt::brotli(const char* s, uint32_t len, uint32_t quality, uint32_t mode, uint32_t lgwin) // .br compress
{
   U_TRACE(1, "UStringExt::brotli(%.*S,%u,%u,%u,%u)", len, s, len, quality, mode, lgwin)

   size_t sz = U_SYSCALL(BrotliEncoderMaxCompressedSize, "%u", len); /* Get an estimation about the output buffer... */

   if (sz == 0) return UString::getStringNull();

   UString result;

   result.setConstant(sz);

   int rc = U_SYSCALL(BrotliEncoderCompress, "%u,%u,%u,%u,%p,%p,%p", quality, lgwin, (BrotliEncoderMode)mode, (size_t)len, (uint8_t*)s, &sz, (uint8_t*)result.data());

   ratio = (sz * 100U) / len;

   U_INTERNAL_DUMP("BrotliEncoderCompress() = %d ratio = %u (%u%%)", rc, ratio, 100-ratio)

   if (rc == 0 ||
       ratio > ratio_threshold)
      {
      return UString::getStringNull();
      }

   result.checkConstant(sz);

// U_INTERNAL_ASSERT(isBrotli(result)) // check magic byte

   U_RETURN_STRING(result);
}
#endif

UString UStringExt::unbrotli(const char* ptr, uint32_t sz) // .br uncompress
{
   U_TRACE(0, "UStringExt::unbrotli(%.*S,%u)", sz, ptr, sz)

#ifdef USE_LIBBROTLI
   UString r(sz);
   const uint8_t* next_out;
   BrotliDecoderResult result;
   size_t input_len = sz, available_out;
   const uint8_t* input = (const uint8_t*)ptr;

   BrotliDecoderState* state  = (BrotliDecoderState*) U_SYSCALL(BrotliDecoderCreateInstance, "%p,%p,%p", U_NULLPTR, U_NULLPTR, U_NULLPTR);

   do {
      available_out = 0;

      result = (BrotliDecoderResult) U_SYSCALL(BrotliDecoderDecompressStream, "%p,%p,%p,%p,%p,%p", state, &input_len, &input, &available_out, U_NULLPTR, U_NULLPTR);

      next_out = BrotliDecoderTakeOutput(state, &available_out);

      if (available_out) (void) r.append((const char*)next_out, available_out);
      }
   while (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT);

   if (result != BROTLI_DECODER_RESULT_SUCCESS)
      {
      BrotliDecoderErrorCode code = (BrotliDecoderErrorCode) U_SYSCALL(BrotliDecoderGetErrorCode, "%p", state);

      U_WARNING("brotli decoder fail, error %S", BrotliDecoderErrorString(code));
      }

   U_SYSCALL_VOID(BrotliDecoderDestroyInstance, "%p", state);

   U_RETURN_STRING(r);
#else
   return UString::getStringNull();
#endif
}

#ifdef USE_LIBZ
UString UStringExt::deflate(const char* s, uint32_t len, uint32_t quality) // .gz compress
{
   U_TRACE(1, "UStringExt::deflate(%.*S,%u,%u)", len, s, len, quality)

#ifdef USE_LIBZOPFLI
   if (quality == 0)
      {
      size_t outsize = 0;
      ZopfliOptions options;
      unsigned char* out = U_NULLPTR;

#  ifndef U_LOG_DISABLE
      deflate_agent = "zopfli";
#  endif

      U_SYSCALL_VOID(ZopfliInitOptions, "%p", &options);

      U_SYSCALL_VOID(ZopfliCompress, "%p,%d,%p,%u,%p,%p", &options, ZOPFLI_FORMAT_GZIP, (unsigned char*)s, (size_t)len, &out, &outsize);

      ratio = (outsize * 100U) / len;

      U_INTERNAL_DUMP("ZopfliCompress(%u) = %u ratio = %u (%u%%)", len, outsize, ratio, 100-ratio)

      if (ratio > ratio_threshold)
         {
         U_SYSCALL_VOID(free, "%p", out);

         return UString::getStringNull();
         }

      UString str((const char*)out, outsize);

      str.setToFree();

      U_RETURN_STRING(str);
      }
#endif

   UString result;
   uint32_t sz = (len + (len / 10) + 12U); // The zlib documentation states that destination buffer size must be at least 0.1% larger than avail_in plus 12 bytes 

   result.setConstant(sz);

#ifndef U_LOG_DISABLE
   deflate_agent = "gzip";
#endif

   U_INTERNAL_ASSERT(u_gz_deflate_header)

   ratio = ((sz = u_gz_deflate(s, len, result.data(), (quality ? quality : Z_BEST_COMPRESSION))) * 100U) / len;

   U_INTERNAL_DUMP("u_gz_deflate(%u) = %u ratio = %u (%u%%)", len, sz, ratio, 100-ratio)

   if (ratio > ratio_threshold) return UString::getStringNull();

   result.checkConstant(sz);

#ifdef DEBUG
   uint32_t* psize_original = (uint32_t*)result.c_pointer(sz - 4);
# if __BYTE_ORDER == __LITTLE_ENDIAN
   U_INTERNAL_DUMP("size original = %u (LE)",            *psize_original)
# else
   U_INTERNAL_DUMP("size original = %u (BE)", u_invert32(*psize_original))
# endif
#endif

   U_INTERNAL_ASSERT(isGzip(result)) // check magic byte

   U_RETURN_STRING(result);
}
#endif

UString UStringExt::gunzip(const char* ptr, uint32_t sz, uint32_t space) // .gz uncompress
{
   U_TRACE(0, "UStringExt::gunzip(%.*S,%u,%u)", sz, ptr, sz, space)

#ifdef USE_LIBZ
   if (space == 0)
      {
      if (isGzip(ptr)) // check magic byte
         {
         uint32_t* psize_original = (uint32_t*)(ptr + sz - 4); // read original size

#     if __BYTE_ORDER == __LITTLE_ENDIAN
         space =            u_get_unalignedp32(psize_original);
#     else
         space = u_invert32(u_get_unalignedp32(psize_original));
#     endif

         U_INTERNAL_DUMP("space = %u", space)
         }

      if (space == 0) space = sz * 4;
      }

   UString result(space);

   result.rep->_length = u_gz_inflate(ptr, sz, result.rep->data());

   U_INTERNAL_DUMP("u_gz_inflate() = %d", result.rep->_length)

   U_RETURN_STRING(result);
#else
   return UString::getStringNull();
#endif
}

// gived the name retrieve pointer on value element from headers "name1:value1\nname2:value2\n"...

__pure const char* UStringExt::getValueFromName(const UString& buffer, uint32_t pos, uint32_t len, const char* name, uint32_t name_len, bool nocase)
{
   U_TRACE(0, "UStringExt::getValueFromName(%V,%u,%u,%.*S,%u,%b)", buffer.rep, pos, len, name_len, name, name_len, nocase)

   U_INTERNAL_ASSERT(buffer)
   U_INTERNAL_ASSERT_MAJOR(len, 0)
   U_ASSERT_EQUALS(memchr(name, ':', name_len), U_NULLPTR)

   const char* ptr_header_value;
   uint32_t header_line, end = pos + len;

loop:
   header_line = buffer.find(name, pos, name_len, len);

   if (header_line == U_NOT_FOUND)
      {
      if (nocase)
         {
         header_line = buffer.findnocase(name, pos, name_len, len); 

         if (header_line != U_NOT_FOUND) goto next;
         }

      U_RETURN((const char*)U_NULLPTR);
      }

next:
   U_INTERNAL_DUMP("header_line = %.*S", 20, buffer.c_pointer(header_line))

   ptr_header_value = buffer.c_pointer(header_line + name_len);

   while (u__isspace(*ptr_header_value)) ++ptr_header_value;

   if (*ptr_header_value != ':')
      {
      pos = buffer.distance(ptr_header_value);
      len = end - pos;

      goto loop;
      }

   do { ++ptr_header_value; } while (u__isspace(*ptr_header_value));

   U_INTERNAL_DUMP("ptr_header_value = %.20S", ptr_header_value)

   return ptr_header_value;
}

// retrieve information on form elements as couple <name1>=<value1>&<name2>=<value2>&...

uint32_t UStringExt::getNameValueFromData(const UString& content, UVector<UString>& name_value, const char* delim, uint32_t dlen)
{
   U_TRACE(0, "UStringExt::getNameValueFromData(%V,%p,%.*S,%u)", content.rep, &name_value, dlen, delim, dlen)

   U_INTERNAL_ASSERT(content)
   U_INTERNAL_ASSERT_POINTER(delim)

   // Parse the data in one fell swoop for efficiency

   uint32_t n       = content.size();
   const char* s    = content.data();
   const char* p    = s;
   const char* _end = s + n;

   bool bform = (dlen == 1 && *delim == '&'),
        burl  = (bform ? u_isUrlEncoded(s, n, true) : false);

   UString x;
   uint32_t old_size = name_value.size(), oldPos = 0, pos = 0, len, result;

   U_INTERNAL_DUMP("bform = %b burl = %b", bform, burl)

   while (s < _end)
      {
      // Find the '=' separating the name from its value

      if (*s != '=')
         {
         ++s;

         continue;
         }

      len = s - p;

      if (len)
         {
         U_INTERNAL_DUMP("oldPos = %u p(%u) = %.*S", oldPos, len, len, p)

         U_INTERNAL_ASSERT_EQUALS(p, content.c_pointer(oldPos))

         if (burl                          == false ||
             u_isUrlEncoded(p, len, false) == false)
            {
            name_value.push_back(content.substr(oldPos, len));
            }
         else
            {
            // name is URL encoded...

            x.setBuffer(len);

            Url::decode(p, len, x);

            name_value.push_back(x);
            }

         oldPos += len + 1;
         }
      else
         {
         name_value.push_back(UString::getStringNull());

         ++oldPos;
         }

      p = ++s;

      // Find the delimitator separating subsequent name/value pairs

      if (bform)
         {
         while ( s < _end &&
                *s != '&')
            {
            ++s;
            }

         len = s - p;
         }
      else
         {
         // check if string is quoted...

         if (*s == '"') s = u_find_char(++s, _end, '"'); // find char '"' not quoted

         pos = content.find_first_of(delim, content.distance(s), dlen);

         // Even if an delimitator wasn't found the rest of the string is a value and value is already decoded...

         len = (pos == U_NOT_FOUND ? n : pos) - oldPos;
         }

      if (len)
         {
         U_INTERNAL_DUMP("oldPos = %u p(%u) = %.*S", oldPos, len, len, p)

         U_INTERNAL_ASSERT_EQUALS(p, content.c_pointer(oldPos))

         if (bform)
            {
            if (burl                          == false ||
                u_isUrlEncoded(p, len, false) == false)
               {
               name_value.push_back(content.substr(oldPos, len));
               }
            else
               {
               // value is URL encoded...

               x.setBuffer(len);

               Url::decode(p, len, x);

               name_value.push_back(x);
               }
            }
         else
            {
            x = content.substr(oldPos, len);

            if (x.isQuoted()) x.unQuote();

            name_value.push_back(x);
            }

         oldPos += len + 1;
         }
      else
         {
         name_value.push_back(UString::getStringNull());

         ++oldPos;
         }

      // Update parse position

      if (bform) p = ++s;
      else
         {
         if (pos == U_NOT_FOUND) break;

         s = content.c_pointer(pos);

         while (++s < _end && memchr(delim, *s, dlen)) {}

         oldPos = content.distance((p = s));
         }
      }

   result = (name_value.size() - old_size);

   U_RETURN(result);
}

// Minifies CSS/JS by removing comments and whitespaces

static inline bool unextendable(char c)
{
   U_TRACE(0, "::unextendable(%C)", c)

   // return true for any character that never needs to be separated from other characters via whitespace

   switch (c)
      {
      case '[':
      case ']':
      case '{':
      case '}':
      case '/':
      case ';':
      case ':': U_RETURN(true);
      default:  U_RETURN(false);
      }
}

// return true for any character that must separated from other "extendable"
// characters by whitespace on the _right_ in order keep tokens separate

static inline bool isExtendableOnRight(char c)
{
   U_TRACE(0, "::isExtendableOnRight(%C)", c)

   // NB: left paren only here -- see http://code.google.com/p/page-speed/issues/detail?id=339

   bool result = ((unextendable(c) || c == '(') == false);

   U_RETURN(result);
}

// return true for any character that must separated from other "extendable"
// characters by whitespace on the _left_ in order keep tokens separate

static inline bool isExtendableOnLeft(char c)
{
   U_TRACE(0, "::isExtendableOnLeft(%C)", c)

   // NB: right paren only here

   if ((unextendable(c) || c == ')') == false) U_RETURN(true);

   U_RETURN(false);
}

UString UStringExt::minifyCssJs(const char* s, uint32_t n)
{
   U_TRACE(0+256, "UStringExt::minifyCssJs(%.*S,%u)", n, s, n)

   char quote;
   UString r(n);
   const char* start;
   char* str = r.data();
   uint32_t sz1, sz = 0;
   const char* begin = s;
   const char* _end  = s + n;

   // we have these tokens: comment, whitespace, single/double-quoted string, and other

   while (s < _end)
      {
      if ( *s      == '/' &&
          *(s + 1) == '*' &&
           (s + 1) < _end)
         {
         // comment: scan to end of comment

         for (s += 2; s < _end; ++s)
            {
            if (*s      == '*' &&
               *(s + 1) == '/' &&
                (s + 1) < _end)
               {
               s += 2;

               break;
               }
            }
         }
      else if (*s       == '/' &&
               *(s + 1) == '/' &&
                (s + 1) < _end)
         {
         // comment: scan to end of comment

         for (s += 2; s < _end && *s != '\n'; ++s) {}
         }
      else if (u__isspace(*s))
         {
         // whitespace: scan to end of whitespace; put a single space into the
         // consumer if necessary to separate tokens, otherwise put nothing

         start = s;

         do { ++s; } while (s < _end && u__isspace(*s));

         if (s < _end                          &&
             start > begin                     &&
             isExtendableOnRight(*(start - 1)) &&
             isExtendableOnLeft(*s))
            {
            str[sz++] = ' ';
            }
         }
      else if (u__isquote(*s))
         {
         // single/double-quoted string: scan to end of string (first unescaped quote of the
         // same kind used to open the string), and put the whole string into the consumer

         start =  s;
         quote = *s++;

         while (s < _end)
            {
            if (*s == quote)
               {
               ++s;

               break;
               }
            else if (*s == '\\' && (s + 1) < _end)
               {
               s += 2;
               }
            else
               {
               ++s;
               }
            }

         sz1 = (s - start);

         U_MEMCPY(str + sz, start, sz1);  // result.append(start, sz1);

         sz += sz1;
         }
      else
         {
         // other: just copy the character over

         str[sz++] = *s;

         if (*s == '}')
            {
            // add a newline after each closing brace to prevent output lines from being too long

            str[sz++] = '\n';
            }

         ++s;
         }

      if (sz >= n) goto end;
      }

   U_INTERNAL_ASSERT(sz <= n)

end:
   r.size_adjust(sz);

   U_RETURN_STRING(r);
}
