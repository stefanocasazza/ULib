// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    string.h - Components for manipulating sequences of characters
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_USTRING_H
#define ULIB_USTRING_H 1

#include <ulib/base/hash.h>
#include <ulib/internal/common.h>

#include <ulib/base/utility.h>

enum StringAllocationType {
   STR_ALLOCATE_SOAP         = 0x00000001,
   STR_ALLOCATE_IMAP         = 0x00000002,
   STR_ALLOCATE_SSI          = 0x00000004,
   STR_ALLOCATE_NOCAT        = 0x00000008,
   STR_ALLOCATE_HTTP         = 0x00000010,
   STR_ALLOCATE_QUERY_PARSER = 0x00000020,
   STR_ALLOCATE_ORM          = 0x00000040,
   STR_ALLOCATE_HTTP2        = 0x00000080
};

enum StringAllocationIndex {
   STR_ALLOCATE_INDEX_SOAP         = 21,
   STR_ALLOCATE_INDEX_IMAP         = STR_ALLOCATE_INDEX_SOAP+14,
   STR_ALLOCATE_INDEX_SSI          = STR_ALLOCATE_INDEX_IMAP+4,
   STR_ALLOCATE_INDEX_NOCAT        = STR_ALLOCATE_INDEX_SSI+2,
   STR_ALLOCATE_INDEX_HTTP         = STR_ALLOCATE_INDEX_NOCAT+2,
   STR_ALLOCATE_INDEX_QUERY_PARSER = STR_ALLOCATE_INDEX_HTTP+11,
   STR_ALLOCATE_INDEX_ORM          = STR_ALLOCATE_INDEX_QUERY_PARSER+5,
   STR_ALLOCATE_INDEX_HTTP2        = STR_ALLOCATE_INDEX_ORM+15
};

// macro for constant string like "xxx"

#define U_STRING_RFIND(x,str)                  (x).rfind(str,U_NOT_FOUND,U_CONSTANT_SIZE(str))    // string constant
#define U_STRING_FIND(x,start,str)             (x).find(str,(start),U_CONSTANT_SIZE(str))         // string constant
#define U_STRING_FIND_EXT(x,start,str,n)       (x).find(str,(start),U_CONSTANT_SIZE(str),n)       // string constant
#define U_STRING_FINDNOCASE_EXT(x,start,str,n) (x).findnocase(str,(start),U_CONSTANT_SIZE(str),n) // string constant

#define U_STRING_COPY(str)          UString((void*)(str).data(),(str).size())
#define U_STRING_FROM_CONSTANT(str) UString(str,U_CONSTANT_SIZE(str))                          // string constant
#define U_ENDS_WITH(x,str)          u_endsWith((x).data(),(x).size(),U_CONSTANT_TO_PARAM(str)) // string constant

// UString content and size

#ifdef DEBUG_DEBUG
#  define U_STRING_TO_TRACE(str)  U_min(128,(str).size()),(str).data()
#else
#  define U_STRING_TO_TRACE(str)            (str).size(), (str).data()
#endif
#define   U_STRING_TO_PARAM(str)            (str).data(), (str).size()
#define   U_STRING_TO_RANGE(str)            (str).data(), (str).pend()

/**
 * UStringRep: string representation
 * ---------------------------------------------------------------------------------------------------------
 * The string object requires only one allocation. The allocation function which gets a block of raw bytes
 * and with room enough and constructs a UStringRep object at the front. Invariants:
 * ---------------------------------------------------------------------------------------------------------
 * 1. string really contains length+1 characters; last is set to '\0' for sure on call to c_str()
 * 2. capacity >= length - allocated memory is always capacity+1
 * 3. references has two states:
 *       0: one reference
 *     n>0: n+1 references
 * 4. all fields == 0 is an empty string, given the extra storage beyond-the-end for a null terminator;
 *    thus, the shared empty string representation needs no constructor
 * ---------------------------------------------------------------------------------------------------------
 * Note that the UStringRep object is a POD so that you can have a static "empty string" UStringRep object
 * already "constructed" before static constructors have run. The reference-count encoding is chosen so that
 * a 0 indicates 1 reference, so you never try to destroy the empty-string UStringRep object
 */

#ifdef DEBUG
#  define U_STRINGREP_FROM_CONSTANT(cstr) (void*)U_CHECK_MEMORY_SENTINEL, U_NULLPTR, 0, U_CONSTANT_SIZE(cstr), 0, 0, cstr
#elif defined(U_SUBSTR_INC_REF)
#  define U_STRINGREP_FROM_CONSTANT(cstr)                                 U_NULLPTR,    U_CONSTANT_SIZE(cstr), 0, 0, cstr
#else
#  define U_STRINGREP_FROM_CONSTANT(cstr)                                               U_CONSTANT_SIZE(cstr), 0, 0, cstr
#endif

class Url;
class UCDB;
class URDB;
class UTDB;
class UFile;
class UDES3;
class UHTTP;
class UHTTP2;
class UValue;
class UCache;
class UValue;
class UString;
class UBase64;
class UEscape;
class UHexDump;
class UOptions;
class UTimeDate;
class USSEThread;
class UStringExt;
class USocketExt;
class UOrmDriver;
class UXMLEscape;
class UMimeHeader;
class UPop3Client;
class UHttpPlugIn;
class UProxyPlugIn;
class Application;
class UFlatBuffer;
class UServer_Base;
class UHashMapNode;
class UHashTableNode;
class UMongoDBClient;
class URDBClient_Base;
class UQuotedPrintable;
class UClientImage_Base;
class UREDISClient_Base;

template <class T> class UVector;
template <class T> class UHashMap;
template <class T> class UHashTable;
template <class T> class UJsonTypeHandler;

typedef void (*vPFprpv)(UStringRep*,void*);
typedef bool (*bPFprpv)(UStringRep*,void*);

class U_EXPORT UStringRep {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // --------------------------
   // references has two states:
   // --------------------------
   //   0: one reference
   // n>0: n+1 references
   // --------------------------

   bool uniq() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::uniq()")

      U_CHECK_MEMORY

      if (references == 0) U_RETURN(true); // NB: 0 -> one reference

      U_RETURN(false);
      }

   void hold()
      {
      U_TRACE_NO_PARAM(0, "UStringRep::hold()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("this = %p parent = %p references = %d child = %d", this, parent, references, child)

      ++references;
      }

   void release() // NB: we don't use delete (dtor) because add a deallocation to the destroy object process...
      {
      U_TRACE_NO_PARAM(0, "UStringRep::release()")

      U_INTERNAL_DUMP("this = %p parent = %p references = %u child = %d", this, parent, references, child)

#  ifdef DEBUG
      if (memory.invariant() == false)
         {
         U_ERROR("UStringRep::release() %s - this = %p parent = %p references = %u child = %d _capacity = %u str(%u) = %.*S",
                  memory.getErrorType(this), this, parent, references, child, _capacity, _length, _length, str);
         }
#  endif

      if (references) --references;
      else _release();
      }

   // Size and Capacity

   uint32_t size() const   { return _length; }
   uint32_t length() const { return _length; }

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::empty()")

      U_CHECK_MEMORY

      U_RETURN(_length == 0);
      }

   uint32_t capacity() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::capacity()")

      U_CHECK_MEMORY

      U_RETURN(_capacity);
      }

   bool writeable() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::writeable()")

      U_CHECK_MEMORY

      U_RETURN(_capacity != 0); // mode: 0 -> const
      }

   bool  isNullTerminated() const { return (str [_length] == '\0'); }
   void setNullTerminated() const { ((char*)str)[_length]  = '\0'; }

   uint32_t space() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::space()")

      U_CHECK_MEMORY

      if ((int32_t)_capacity > 0) U_RETURN(_capacity - _length);

      U_RETURN(0);
      }

   ptrdiff_t remain(const char* ptr) const
      {
      U_TRACE(0, "UStringRep::remain(%p)", ptr)

      U_CHECK_MEMORY

      U_RETURN((str + _length) - ptr);
      }

   uint32_t distance(const char* ptr) const
      {
      U_TRACE(0, "UStringRep::distance(%p)", ptr)

      U_CHECK_MEMORY

      U_RETURN(ptr - str);
      }

   uint32_t fold(uint32_t pos, uint32_t off) const { return fold(pos, off, _length); }

   // C-Style String

   char* data() const { return (char*)str; }

   uint32_t copy(char* s, uint32_t n = U_NOT_FOUND, uint32_t pos = 0) const;

   // ELEMENT ACCESS

   char at(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::at(%u)", pos)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MINOR(pos, _length)

      U_RETURN(str[pos]);
      }

   char first_char() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::first_char()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_length, 0)

      U_RETURN(str[0]);
      }

   char last_char() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::last_char()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_length, 0)

      U_RETURN(str[_length - 1]);
      }

   void setLastChar(char c)
      {
      U_TRACE(0, "UStringRep::setLastChar(%C)", c)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_length, 0)

      ((char*)str)[_length - 1] = c;
      }

   char operator[](uint32_t pos) const { return str[pos]; }

   const char* c_pointer(uint32_t pos) const { return (str + pos); }

   char* pend() const { return (char*)(str + _length); }

   // Compare

   int compare(const char* s, uint32_t n) const
      {
      U_TRACE(0, "UStringRep::compare(%.*S,%u)", n, s, n)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("str(%u) = %.*S", _length, U_min(_length, n), str)

      U_INTERNAL_ASSERT_MAJOR(n, 0)

      int r = *(const unsigned char*)str - *(const unsigned char*)s;

      if (r) U_RETURN(r);

      uint32_t len = U_min(_length, n);

      if (len > 1)
         {
         r = memcmp(str+1, s+1, len-1);

         if (r) U_RETURN(r);
         }

      r = _length < n ? -1 : _length > n; // _length - n;

      U_RETURN(r);
      }

   int compare(const UStringRep* rep) const { return compare(rep->str, rep->_length); }

   int compare(uint32_t pos, uint32_t n1, const char* s, uint32_t n2) const
      {
      U_TRACE(0, "UStringRep::compare(%u,%u,%.*S,%u)", pos, n1, n2, s, n2)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT((pos + n1) <= _length)

      int r = memcmp(str + pos, s, U_min(n1, n2));

      if (r == 0) r = (n1 - n2);

      U_RETURN(r);
      }

   int compare(const UStringRep* rep, uint32_t depth) const __pure;

   // Compare with ignore case

   int comparenocase(const char* s, uint32_t n) const
      {
      U_TRACE(0, "UStringRep::comparenocase(%.*S,%u)", n, s, n)

      U_CHECK_MEMORY

      int r = u__strncasecmp(str, s, U_min(_length, n));

      U_INTERNAL_DUMP("str = %.*S", U_min(_length, n), str)

      if (r == 0) r = (_length - n);

      U_RETURN(r);
      }

   int comparenocase(const UStringRep* rep) const { return comparenocase(rep->str, rep->_length); }

   // Equal

   bool equal(char c) const
      {
      U_TRACE(0, "UStringRep::equal(%C)", c)

      U_CHECK_MEMORY

      if (*str == c &&
          _length == 1)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool equal(const char* s, uint32_t n) const
      {
      U_TRACE(0, "UStringRep::equal(%#.*S,%u)", n, s, n)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(s)

      if (_length == n &&
          memcmp(str, s, n) == 0)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool equal(const char* s) const         { return equal(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   bool equal(const UStringRep* rep) const { return equal(rep->str, rep->_length); }

   // Equal with ignore case

   bool equalnocase(char c) const
      {
      U_TRACE(0, "UStringRep::equalnocase(%C)", c)

      U_CHECK_MEMORY

      if (_length == 1 &&
          u__tolower(*str) == u__tolower(c))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool equalnocase(const char* s, uint32_t n) const
      {
      U_TRACE(0, "UStringRep::equalnocase(%.*S,%u)", n, s, n)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(s)

      if (_length == n &&
          u__strncasecmp(str, s, n) == 0)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool equalnocase(const UStringRep* rep) const { return equalnocase(rep->str, rep->_length); }

   // Substring

   bool isSubStringOf(UStringRep* rep) const __pure;

   UStringRep* substr(const char* t, uint32_t tlen) const;

   UStringRep* substr(uint32_t pos, uint32_t n)  const { return substr(str + pos, n); }

   // Assignment

   void size_adjust()
      {
      U_TRACE_NO_PARAM(0+256, "UStringRep::size_adjust()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

#  ifdef DEBUG
      if (references)
         {
         string_rep_share = this;

         U_DUMP_OBJECT_WITH_CHECK("shared with this", checkIfReferences)

         U_INTERNAL_ASSERT_MSG(false, "CANNOT ADJUST SIZE OF A REFERENCED STRING...")
         }
#  endif

      _length = u__strlen(str, __PRETTY_FUNCTION__);

      U_INTERNAL_ASSERT(invariant())
      }

   void size_adjust(uint32_t value)
      {
      U_TRACE(0+256, "UStringRep::size_adjust(%u)", value)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR((int32_t)_capacity, 0)

#  ifdef DEBUG
      if (references)
         {
         string_rep_share = this;

         U_DUMP_OBJECT_WITH_CHECK("shared with this", checkIfReferences)

         U_INTERNAL_ASSERT_MSG(false, "CANNOT ADJUST SIZE OF A REFERENCED STRING...")
         }
#  endif

      U_INTERNAL_ASSERT(value <= _capacity)

      _length = value;

      U_INTERNAL_ASSERT(invariant())
      }

   void size_adjust_force()
      {
      U_TRACE_NO_PARAM(0, "UStringRep::size_adjust_force()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR((int32_t)_capacity, 0)

      _length = u__strlen(str, __PRETTY_FUNCTION__);

      U_INTERNAL_ASSERT(invariant())
      }

   void size_adjust_force(uint32_t value)
      {
      U_TRACE(0, "UStringRep::size_adjust_force(%u)", value)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(value <= _capacity)
      U_INTERNAL_ASSERT_MAJOR((int32_t)_capacity, 0)

      _length = value;

      U_INTERNAL_ASSERT(invariant())
      }

   void size_adjust(      const char* ptr) { size_adjust(      distance(ptr)); }
   void size_adjust_force(const char* ptr) { size_adjust_force(distance(ptr)); }

   void size_adjust_constant(uint32_t sz)     { _length = sz; }
   void size_adjust_constant(const char* ptr) { size_adjust_constant(distance(ptr)); }

   void replace(const char* s, uint32_t n)
      {
      U_TRACE(0, "UStringRep::replace(%S,%u)", s, n)

      U_INTERNAL_ASSERT_MAJOR(n, 0)
      U_INTERNAL_ASSERT(_capacity >= n)

      (void) memcpy((char*)str, s, _length = n);
      }

#ifdef DEBUG
   bool invariant() const;
   const char* dump(bool _reset) const;
#endif

   // EXTENSION

   bool isNumber(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isNumber(%u)", pos)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_isNumber(str + pos, _length - pos)) U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isBinary(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isBinary(%u)", pos)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MINOR(pos, _length)

      if (u_isBinary((const unsigned char*)(str + pos), _length - pos)) U_RETURN(true);

      U_RETURN(false);
      }

   bool isBase64(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isBase64(%u)", pos)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_isBase64(str + pos, _length - pos)) U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isBase64Url(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isBase64Url(%u)", pos)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_isBase64Url(str + pos, _length - pos)) U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isPrintable(uint32_t pos, bool bline = false) const
      {
      U_TRACE(0, "UStringRep::isPrintable(%u,%b)", pos, bline)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_isPrintable(str + pos, _length - pos, bline)) U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isWhiteSpace(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isWhiteSpace(%u)", pos)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_isWhiteSpace(str + pos, _length - pos) == false) U_RETURN(false);
         }

      U_RETURN(true);
      }

   bool isText(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isText(%u)", pos)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_isText((const unsigned char*)(str + pos), _length - pos)) U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isUTF8(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isUTF8(%u)", pos)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MINOR(pos, _length)

      if (u_isUTF8((const unsigned char*)(str + pos), _length - pos)) U_RETURN(true);

      U_RETURN(false);
      }

   bool isUTF16(uint32_t pos) const
      {
      U_TRACE(0, "UStringRep::isUTF16(%u)", pos)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MINOR(pos, _length)

      if (u_isUTF16((const unsigned char*)(str + pos), _length - pos)) U_RETURN(true);

      U_RETURN(false);
      }

   bool isIPv4Addr() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::isIPv4Addr()")

      U_CHECK_MEMORY

      if (u_isIPv4Addr(str, _length)) U_RETURN(true);

      U_RETURN(false);
      }

   bool isMacAddr() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::isMacAddr()")

      U_CHECK_MEMORY

      if (u_isMacAddr(str, _length)) U_RETURN(true);

      U_RETURN(false);
      }

   bool isXMacAddr() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::isXMacAddr()")

      U_CHECK_MEMORY

      if (u_isXMacAddr(str, _length)) U_RETURN(true);

      U_RETURN(false);
      }

   bool   isEndHeader(uint32_t pos) const __pure;
   bool findEndHeader(uint32_t pos) const __pure
      {
      U_TRACE(0, "UStringRep::findEndHeader(%u)", pos)

      U_CHECK_MEMORY

      if (_length)
         {
         U_INTERNAL_ASSERT_MINOR(pos, _length)

         if (u_findEndHeader1(str + pos, _length - pos) != U_NOT_FOUND) U_RETURN(true); // find sequence of U_CRLF2
         }

      U_RETURN(false);
      }

   uint32_t findWhiteSpace(uint32_t pos) const __pure
      {
      U_TRACE(0, "UStringRep::findWhiteSpace(%u)", pos)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(pos <= _length)

      for (; pos < _length; ++pos)
         {
         if (u__isspace(str[pos])) U_RETURN(pos);
         }

      U_RETURN(U_NOT_FOUND);
      }

#ifdef HAVE_STRTOF
   float strtof() const;
#endif
#ifdef HAVE_STRTOLD
   long double strtold() const; // long double
#endif

   bool strtob() const __pure;

    int64_t strtoll( bool check_for_suffix = false) const __pure;
   uint64_t strtoull(bool check_for_suffix = false) const __pure;

            long strtol( bool check_for_suffix = false) const __pure;
   unsigned long strtoul(bool check_for_suffix = false) const __pure;

   uint32_t hash() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::hash()")

      U_CHECK_MEMORY

      uint32_t result = u_hash((const unsigned char*)str, _length);

      U_INTERNAL_ASSERT_MAJOR(result, 0)

      U_RETURN(result);
      }

   uint32_t hashIgnoreCase() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::hashIgnoreCase()")

      U_CHECK_MEMORY

      uint32_t result = u_hash_ignore_case((const unsigned char*)str, _length);

      U_INTERNAL_ASSERT_MAJOR(result, 0)

      U_RETURN(result);
      }

   // for constant string

   void trim()
      {
      U_TRACE_NO_PARAM(0, "UStringRep::trim()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(_capacity, 0)

      (void) u_trim(&str, &_length);
      }

   // if the string is quoted...

   bool isQuoted(const unsigned char c) const
      {
      U_TRACE(0, "UStringRep::isQuoted(%C)", c)

      U_CHECK_MEMORY

      if (str[0]         == c &&
          str[_length-1] == c)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // ...unquote it

   void unQuote()
      {
      U_TRACE_NO_PARAM(0, "UStringRep::unQuote()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_length, 2)
      U_INTERNAL_ASSERT_EQUALS((int32_t)_capacity, 0)

      ++str;
      _length -= 2;
      }

   static bool needQuote(const char* s, uint32_t n)
      {
      U_TRACE(0, "UStringRep::needQuote(%S,%u)", s, n)

      U_INTERNAL_ASSERT_MAJOR(n, 0)

      for (const char* _end = s + n; s < _end; ++s)
         {
         char c = *s;

         if (c == '"'  ||
             c == '\\' ||
             u__isspace(c))
            {
            U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   bool needQuote() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::needQuote()")

      U_CHECK_MEMORY

      if (_length == 0) U_RETURN(true);

      return needQuote(str, _length);
      }

   uint32_t getSpaceToDump() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::getSpaceToDump()")

      U_CHECK_MEMORY

      if (needQuote() == false) U_RETURN(_length);

      uint32_t sz = _length + U_CONSTANT_SIZE("");

      U_RETURN(sz);
      }

   static uint32_t fold(uint32_t pos, uint32_t off, uint32_t sz)
      {
      U_TRACE(0, "UStringRep::fold(%u,%u,%u)", pos, off, sz)

      uint32_t dist   = (sz - pos),
               newoff = (off < dist ? off : dist);

      U_RETURN(newoff);
      }

   // UTF8 <--> ISO Latin 1

   static UStringRep*   toUTF8(const unsigned char* t, uint32_t tlen);
   static UStringRep* fromUTF8(const unsigned char* t, uint32_t tlen);

   static uint32_t max_size() { return U_STRING_MAX_SIZE; } // The maximum number of individual char elements of an individual string is determined by max_size()

   static UStringRep* create(uint32_t length, uint32_t capacity, const char* ptr); // NB: we don't use new (ctor) because we want an allocation with more space for string data...

   static UStringRep* string_rep_null; // This storage is init'd to 0 by the linker, resulting (carefully) in an empty string with one(=>0) reference...

#if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   UStringRep* parent; // manage substring to increment reference of source string
# ifdef DEBUG
   int32_t child;      // manage substring to capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...
   static bool check_dead_of_source_string_with_child_alive;
# endif
#endif

   // STREAM

#ifdef U_STDCPP_ENABLE
   void write(ostream& os) const;

   friend ostream& operator<<(ostream& os, const UStringRep& r) { r.write(os); return os; }
#endif

protected:
   uint32_t _length,
            _capacity,  // [0 const | -1 mmap]...
            references; // NB: must be here, see string_rep_null...
   const char* str;
   // ----------------> maybe unnamed array of char...

#ifdef DEBUG
   static int32_t max_child;
   static UStringRep* parent_destroy;
   static UStringRep* string_rep_share;

   static bool checkIfChild(     const char* name_class, void* ptr_object);
   static bool checkIfReferences(const char* name_class, void* ptr_object);
#endif

private:
   void set(uint32_t __length, uint32_t __capacity, const char* ptr)
      {
      U_TRACE(0, "UStringRep::set(%u,%u,%p)", __length, __capacity, ptr)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ptr)

#  if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
      parent = U_NULLPTR;
#    ifdef DEBUG
      child  = 0;
#    endif
#  endif

      _length    = __length;
      _capacity  = __capacity; // [0 const | -1 mmap | -2 we must call free()]...
      references = 0;
      str        = ptr;
      }

   explicit UStringRep(const char* t, uint32_t tlen) // NB: to use only with new(UStringRep(t,tlen))...
      {
      U_TRACE_CTOR(0, UStringRep, "%p,%u", t, tlen)

      U_INTERNAL_ASSERT_POINTER(t)
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      set(tlen, 0U, t);
      }

   explicit UStringRep(uint32_t tlen, const char* t) // NB: to use only with new(UStringRep(tlen,t))...
      {
      U_TRACE_CTOR(0, UStringRep, "%u,%p", tlen, t)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(t)
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

#  if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
      parent = U_NULLPTR;
#    ifdef DEBUG
      child  = 0;
#    endif
#  endif

      _length    = 0U;
      _capacity  = tlen;
      references = 1; // NB: this object cannot be destroyed...
      str        = t;
      }

   ~UStringRep();

   void _release();

   void shift(ptrdiff_t diff)
      {
      U_TRACE(0, "UStringRep::shift(%p)", diff)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(diff, 0)
      U_INTERNAL_ASSERT_EQUALS(_capacity, 0) // mode: 0 -> const

      U_INTERNAL_DUMP("this = %V", this)

      str += diff;

      U_INTERNAL_DUMP("this = %V", this)

      U_INTERNAL_ASSERT_DIFFERS(str[0], 0)

      U_INTERNAL_ASSERT(invariant())
      }

   UStringRep* duplicate() const
      {
      U_TRACE_NO_PARAM(0, "UStringRep::duplicate()")

      U_INTERNAL_ASSERT_MAJOR(_length, 0)
      U_INTERNAL_ASSERT_EQUALS(_capacity, 0) // mode: 0 -> const

      return UStringRep::create(_length, _length, str);
      }

   // equal lookup use case

   static bool equal_lookup(UStringRep* key1, const char* s2, uint32_t n2, bool ignore_case)
      {
      U_TRACE(0, "UStringRep::equal_lookup(%V,%.*S,%u,%b)", key1, n2, s2, n2, ignore_case)

      U_INTERNAL_ASSERT_POINTER(s2)
      U_INTERNAL_ASSERT_MAJOR(n2, 0)
      U_INTERNAL_ASSERT_POINTER(key1)

      const char* s1;
      uint32_t n1 = key1->size();

      U_INTERNAL_ASSERT_MAJOR(n1, 0)

      if (n1 == n2                                          &&
          ((s1 = key1->data(),     memcmp(s1, s2, n1) == 0) ||
           (ignore_case && u__strncasecmp(s1, s2, n1) == 0)))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool equal_lookup(const UStringRep* key1, const UStringRep* key2, bool ignore_case)
      {
      U_TRACE(0, "UStringRep::equal_lookup(%V,%V,%b)", key1, key2, ignore_case)

      U_INTERNAL_ASSERT_POINTER(key1)
      U_INTERNAL_ASSERT_POINTER(key2)

      const char* s1;
      const char* s2;
      uint32_t n1 = key1->size(),
               n2 = key2->size();

      U_INTERNAL_ASSERT_MAJOR(n1, 0)
      U_INTERNAL_ASSERT_MAJOR(n2, 0)

      if (   n1 ==   n2                                     &&
          (key1 == key2                                     ||
           (s1 = key1->data(),
            s2 = key2->data(),     memcmp(s1, s2, n1) == 0) ||
           (ignore_case && u__strncasecmp(s1, s2, n1) == 0)))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   U_DISALLOW_COPY_AND_ASSIGN(UStringRep)

   friend class Url;
   friend class ULib;
   friend class UCDB;
   friend class URDB;
   friend class UTDB;
   friend class UDES3;
   friend class UHTTP;
   friend class UHTTP2;
   friend class UCache;
   friend class UValue;
   friend class UString;
   friend class UBase64;
   friend class UEscape;
   friend class UHexDump;
   friend class UOptions;
   friend class UTimeDate;
   friend class UStringExt;
   friend class USocketExt;
   friend class UOrmDriver;
   friend class UXMLEscape;
   friend class UPop3Client;
   friend class UMimeHeader;
   friend class UHttpPlugIn;
   friend class Application;
   friend class UFlatBuffer;
   friend class UProxyPlugIn;
   friend class UHashMapNode;
   friend class UServer_Base;
   friend class UHashTableNode;
   friend class UMongoDBClient;
   friend class URDBClient_Base;
   friend class UQuotedPrintable;
   friend class UClientImage_Base;
   friend class UREDISClient_Base;

   friend struct UObjectIO;

   template <class T> friend class UVector;
   template <class T> friend class UHashMap;
   template <class T> friend class UHashTable;
   template <class T> friend class UJsonTypeHandler;
   template <class T> friend void u_construct(const T*, uint32_t);
};

class U_EXPORT UString {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static const UString* str_host;
   static const UString* str_chunked;
   static const UString* str_without_mac;
   static const UString* str_localhost;
   static const UString* str_http;
   static const UString* str_msg_rfc;
   static const UString* str_txt_plain;
   static const UString* str_address;
   static const UString* str_CLIENT_QUEUE_DIR;
   static const UString* str_point;
   static const UString* str_true;
   static const UString* str_false;
   static const UString* str_response;
   static const UString* str_zero;
   static const UString* str_one;
   static const UString* str_nostat;
   static const UString* str_tsa;
   static const UString* str_soap;
   static const UString* str_path_root;
   static const UString* str_asterisk;
   // SOAP
   static const UString* str_ns;
   static const UString* str_boolean;
   static const UString* str_byte;
   static const UString* str_unsignedByte;
   static const UString* str_short;
   static const UString* str_unsignedShort;
   static const UString* str_int;
   static const UString* str_unsignedInt;
   static const UString* str_long;
   static const UString* str_unsignedLong;
   static const UString* str_float;
   static const UString* str_double;
   static const UString* str_string;
   static const UString* str_base64Binary;
   // IMAP
   static const UString* str_recent;
   static const UString* str_unseen;
   static const UString* str_uidnext;
   static const UString* str_uidvalidity;
   // PROXY SERVICE
   static const UString* str_FOLLOW_REDIRECTS;
   static const UString* str_CLIENT_CERTIFICATE;
   static const UString* str_REMOTE_ADDRESS_IP;
   static const UString* str_WEBSOCKET;
   // NOCAT
   static const UString* str_without_label;
   static const UString* str_allowed_members_default;
   // SSI
   static const UString* str_cgi;
   static const UString* str_var;
   // HTTP
   static const UString* str_origin;
   static const UString* str_ctype_tsa;
   static const UString* str_ctype_txt;
   static const UString* str_ctype_html;
   static const UString* str_ctype_soap;
   static const UString* str_ulib_header;
   static const UString* str_storage_keyid;
   static const UString* str_websocket_key;
   static const UString* str_websocket_prot;
   static const UString* str_htdigest;
   static const UString* str_htpasswd;
   // QUERY PARSER
   static const UString* str_p1;
   static const UString* str_p2;
   static const UString* str_or;
   static const UString* str_and;
   static const UString* str_not;
   // ORM
   static const UString* str_port;
   static const UString* str_root;
   static const UString* str_UTF8;
   static const UString* str_UTF16;
   static const UString* str_dbname;
   static const UString* str_timeout;
   static const UString* str_compress;
   static const UString* str_character_set;
   static const UString* str_auto_reconnect;
   // ORM PGSQL
   static const UString* str_pgsql_name;
   static const UString* str_memory;
   // ORM MYSQL
   static const UString* str_mysql_name;
   static const UString* str_secure_auth;
   // ORM SQLITE
   static const UString* str_sqlite_name;
   static const UString* str_dbdir;
#ifndef U_HTTP2_DISABLE
   static const UString* str_authority;
   static const UString* str_method;
   static const UString* str_method_get;
   static const UString* str_method_post;
   static const UString* str_path;
   static const UString* str_path_index;
   static const UString* str_scheme;
   static const UString* str_scheme_https;
   static const UString* str_status;
   static const UString* str_status_200;
   static const UString* str_status_204;
   static const UString* str_status_206;
   static const UString* str_status_304;
   static const UString* str_status_400;
   static const UString* str_status_404;
   static const UString* str_status_500;
   static const UString* str_accept_charset;
   static const UString* str_accept_encoding;
   static const UString* str_accept_encoding_value;
   static const UString* str_accept_language;
   static const UString* str_accept_ranges;
   static const UString* str_accept;
   static const UString* str_access_control_allow_origin;
   static const UString* str_age;
   static const UString* str_allow;
   static const UString* str_authorization;
   static const UString* str_cache_control;
   static const UString* str_content_disposition;
   static const UString* str_content_encoding;
   static const UString* str_content_language;
   static const UString* str_content_length;
   static const UString* str_content_location;
   static const UString* str_content_range;
   static const UString* str_content_type;
   static const UString* str_cookie;
   static const UString* str_date;
   static const UString* str_etag;
   static const UString* str_expect;
   static const UString* str_expires;
   static const UString* str_from;
   static const UString* str_if_match;
   static const UString* str_if_modified_since;
   static const UString* str_if_none_match;
   static const UString* str_if_range;
   static const UString* str_if_unmodified_since;
   static const UString* str_last_modified;
   static const UString* str_link;
   static const UString* str_location;
   static const UString* str_max_forwards;
   static const UString* str_proxy_authenticate;
   static const UString* str_proxy_authorization;
   static const UString* str_range;
   static const UString* str_referer;
   static const UString* str_refresh;
   static const UString* str_retry_after;
   static const UString* str_server;
   static const UString* str_set_cookie;
   static const UString* str_strict_transport_security;
   static const UString* str_transfer_encoding;
   static const UString* str_user_agent;
   static const UString* str_vary;
   static const UString* str_via;
   static const UString* str_www_authenticate;
   static const UString* str_ULib;
#endif

   static void str_allocate(int which);

   // u_buffer string (for container, etc...)

   static UString& getUBuffer()
      {
      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return *string_u_buffer;
      }

   // null string (for container, etc...)

   static UString& getStringNull()
      {
      U_INTERNAL_ASSERT_EQUALS((bool)*string_null, false)

      return *string_null;
      }

protected:
   static UStringRep* pkey;
   static UString* string_null;
   static UString* string_u_buffer;

   friend class ULib;
   friend class UFile;
   friend class UHTTP2;
   friend class UValue;
   friend class UServices;
   friend class UStringExt;
   friend class USocketExt;
   friend class UClientImage_Base;
   friend class UREDISClient_Base;

   template <class T> friend class UVector;
   template <class T> friend class UHashMap;
   template <class T> friend class UHashTable;

   explicit UString(UStringRep** pr) : rep(*pr) // NB: for toUTF8() and fromUTF8()...
      {
      U_TRACE_CTOR(0, UString, "%V", *pr)
      }

   explicit UString(uint32_t len, uint32_t sz, char* ptr);

   explicit UString(unsigned char* t, uint32_t tlen, uint32_t need) // NB: for UHTTP2::CONTINUATION...
      {
      U_TRACE_CTOR(0, UString, "%.*S,%u,%u", tlen, t, tlen, need)

      U_INTERNAL_ASSERT_POINTER(t)
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)
      U_INTERNAL_ASSERT_MAJOR(need, tlen)

      rep = UStringRep::create(tlen, need, (const char*)t);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(uint32_t tlen, const char* t) // NB: for USSEThread::run()...
      {
      U_TRACE_CTOR(0, UString, "%u,%p", tlen, t)

      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      U_NEW(UStringRep, rep, UStringRep(tlen, t));

      U_INTERNAL_ASSERT(invariant())
      }

   void setFromData(const char** ptr, uint32_t sz, unsigned char delim);

   // NB: for UStringExt::deflate()...

   void   setConstant(uint32_t sz);
   void checkConstant(uint32_t sz);

public:
   // mutable
   UStringRep* rep;

   // SERVICES

   char* data() const         { return rep->data(); }
   bool empty() const         { return rep->empty(); }

   operator bool() const      { return (rep->_length != 0); }

   uint32_t size() const      { return rep->size(); }
   uint32_t space() const     { return rep->space(); }
   uint32_t length() const    { return rep->length(); }
   uint32_t capacity() const  { return rep->capacity(); }

   static uint32_t max_size() { return U_STRING_MAX_SIZE; }

protected:
   void _set(UStringRep* r) // in 'memory reference' distinction is made between set, copy, e assign...
      {
      U_TRACE(0, "UString::_set(%V)", r)

      U_INTERNAL_ASSERT_POINTER(r)
      U_INTERNAL_ASSERT_DIFFERS(rep, r)

      rep->release(); // 1. release existing resource
      rep = r;        // 2. bind copy to self

      U_CHECK_MEMORY_OBJECT(rep)
      }

   void _copy(UStringRep* r)
      {
      U_TRACE(0, "UString::_copy(%V)", r)

      U_INTERNAL_ASSERT_POINTER(r)

      rep = r;  // bind copy to self
      rep->hold();

      U_CHECK_MEMORY_OBJECT(rep)
      }

   // SUBSTRING

   UString(const UStringRep* _rep, const char* t, uint32_t tlen)
      {
      U_TRACE_CTOR(0, UString, "%V,%p,%u", _rep, t, tlen)

      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      rep = _rep->substr(t, tlen);

      U_INTERNAL_ASSERT(invariant())
      }

   UString(const UStringRep* _rep, uint32_t pos, uint32_t n = U_NOT_FOUND)
      {
      U_TRACE_CTOR(0, UString, "%V,%u,%u", _rep, pos, n)

      rep = _rep->substr(pos, _rep->fold(pos, n));

      U_INTERNAL_ASSERT(invariant())
      }

public:
   static uint32_t _getReserveNeed(uint32_t need = U_CAPACITY * 2)
      {
      U_TRACE(0, "UString::_getReserveNeed(%u)", need)

           if (need < U_CAPACITY) need = U_CAPACITY;
      else if (need > U_CAPACITY)
         {
         if (need < 2*1024*1024) need = (need * 2) + (PAGESIZE * 2);

         need += PAGESIZE; // NB: to avoid duplication on realloc...
         }

      U_RETURN(need);
      }

   uint32_t getReserveNeed(uint32_t n) { return _getReserveNeed(rep->_length + n); }

   void _assign(UStringRep* r)
      {
      U_TRACE(0, "UString::_assign(%V)", r)

      U_INTERNAL_ASSERT_POINTER(r)

      // NB: it works also in the case of (rep == r)...

        r->hold();    // 1. take a copy of new resource
      rep->release(); // 2. release existing resource
      rep = r;        // 3. bind copy to self

      U_CHECK_MEMORY_OBJECT(rep)
      }

   // constructors

   UString() : rep(UStringRep::string_rep_null)
      {
      U_TRACE_CTOR(0, UString, "")

      rep->hold();

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(const UStringRep* r) : rep((UStringRep*)r)
      {
      U_TRACE_CTOR(0, UString, "%V", r)

      rep->hold();

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(ustringrep* r)
      {
      U_TRACE_CTOR(0, UString, "%p", r)

#  ifdef DEBUG
      r->_this = (void*)U_CHECK_MEMORY_SENTINEL;
#  endif

      uustringrep u = { r };

      _copy(u.p2);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(const char* t, uint32_t tlen)
      {
      U_TRACE_CTOR(0, UString, "%.*S,%u", tlen, t, tlen)

      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      U_NEW(UStringRep, rep, UStringRep(t, tlen));

      U_INTERNAL_ASSERT(invariant())
      }

   // NB: Avoid constructors with a single integer argument! Use the explicit keyword if you can't avoid them...

   explicit UString(uint32_t n)
      {
      U_TRACE_CTOR(0, UString, "%u", n)

      rep = UStringRep::create(0U, n, U_NULLPTR);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(const char* t)
      {
      U_TRACE_CTOR(0, UString, "%S", t)

      uint32_t len = (t ? u__strlen(t, __PRETTY_FUNCTION__) : 0);

      if (len) U_NEW(UStringRep, rep, UStringRep(t, len))
      else     _copy(UStringRep::string_rep_null);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(const void* t, uint32_t tlen)
      {
      U_TRACE_CTOR(0, UString, "%.*S,%u", tlen, (char*)t, tlen)

      U_INTERNAL_ASSERT_POINTER(t)
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      rep = UStringRep::create(tlen, tlen, (const char*)t);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(uint32_t n, unsigned char c)
      {
      U_TRACE_CTOR(0, UString, "%u,%C", n, c)

      rep = UStringRep::create(n, n, U_NULLPTR);

      (void) memset((void*)rep->str, c, n);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(uint32_t sz, const char* format, uint32_t fmt_size, ...) // ctor with var arg
      {
      U_TRACE_CTOR(0, UString, "%u,%.*S,%u", sz, fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)

      va_list argp;
      va_start(argp, fmt_size);

      rep = UStringRep::create(0U, sz, U_NULLPTR);

      rep->_length = u__vsnprintf(rep->data(), rep->_capacity+1, format, fmt_size, argp);

      va_end(argp);

      U_INTERNAL_ASSERT(invariant())
      }

   explicit UString(const UString& str, uint32_t pos, uint32_t n = U_NOT_FOUND)
      {
      U_TRACE_CTOR(0, UString, "%p,%u,%u", &str, pos, n)

      U_INTERNAL_ASSERT(pos <= str.size())

      uint32_t sz = str.rep->fold(pos, n);

      if (sz) rep = UStringRep::create(sz, sz, str.rep->str + pos);
      else    _copy(UStringRep::string_rep_null);

      U_INTERNAL_ASSERT(invariant())
      }

   // SUBSTRING

   UString substr(const char* t, uint32_t tlen) const
      {
      U_TRACE(0, "UString::substr(%.*S,%u)", tlen, t, tlen)

      if (tlen == 0) return *string_null;

      UString result(rep, t, tlen);

      U_RETURN_STRING(result);
      }

   UString substr(uint32_t pos, uint32_t n = U_NOT_FOUND) const
      {
      U_TRACE(0, "UString::substr(%u,%u)", pos, n)

      U_INTERNAL_ASSERT(pos <= rep->_length)

      return substr(rep->str + pos, rep->fold(pos, n));
      }

   UString substrTrim(const char* t, uint32_t tlen) const
      {
      U_TRACE(0, "UString::substrTrim(%.*S,%u)", tlen, t, tlen)

      if (tlen == 0 ||
          (u_trim(&t, &tlen) && tlen == 0))
         {
         return *string_null;
         }

      UString result(rep, t, tlen);

      U_RETURN_STRING(result);
      }

   UString substrTrim(uint32_t pos, uint32_t n = U_NOT_FOUND) const
      {
      U_TRACE(0, "UString::substrTrim(%u,%u)", pos, n)

      U_INTERNAL_ASSERT(pos <= rep->_length)

      return substrTrim(rep->str + pos, rep->fold(pos, n));
      }

   bool isSubStringOf(const UString& str) const { return rep->isSubStringOf(str.rep); }

   // destructor

   ~UString()
      {
      U_TRACE_DTOR(0, UString)

      U_INTERNAL_ASSERT_POINTER(rep)

      U_CHECK_MEMORY_OBJECT(rep)

      U_INTERNAL_ASSERT_DIFFERS(this, string_u_buffer)

      rep->release();
      }

   // ASSIGNMENT

   UString(const UString& str) : rep(str.rep)
      {
      U_TRACE_CTOR(0, UString, "%p", &str)

      rep->hold();

      U_INTERNAL_ASSERT(invariant())
      }

   UString& operator=(const UString& str)
      {
      U_TRACE(0, "UString::operator=(%p)", &str)

      _assign(str.rep);

      return *this;
      }

   // swap

   void swap(UString& str)
      {
      U_TRACE(0, "UString::swap(%p)", &str)

      UStringRep* tmp = rep;

          rep = str.rep;
      str.rep = tmp;
      }

   static void swap(UString& lhs, UString& rhs) { lhs.swap(rhs); }

#ifdef U_COMPILER_RVALUE_REFS
   UString& operator=(UString && str)
      {
      U_TRACE_NO_PARAM(0, "UString::operator=(move)")

      swap(str);

      return *this;
      }

# if (!defined(__GNUC__) || GCC_VERSION_NUM > 50300) // GCC has problems dealing with move constructor, so turn the feature on for 5.3.1 and above, only
   UString(UString && str)
      {
      U_TRACE_NO_PARAM(0, "UString::UString(move)")

      // Move the value (including the allocated memory) from the first object to the second one, and leave the first one empty.
      // It will not need the value anyway, because the only operation that will be executed on it is the destruction. We must
      // leave it in the state where destructor can be safely called without causing any problems such as releasing the resources
      // that were stolen and now owned by another object

          rep = str.rep;
      str.rep = UStringRep::string_rep_null;
                UStringRep::string_rep_null->hold();

      U_INTERNAL_DUMP("rep = %p", rep)

      U_INTERNAL_ASSERT(invariant())
      }
# endif
#endif

   // Replace

   UString& replace(uint32_t pos, uint32_t n1, uint32_t n2, char c) // NB: unsigned char conflict with a uint32_t at the same parameter position...
      {
      U_TRACE(0, "UString::replace(%u,%u,%u,%C)", pos, n1, n2, c)

      char* ptr = __replace(pos, n1, n2);

      if (ptr && n2) (void) U_SYSCALL(memset, "%p,%d,%u", ptr, c, n2);

      U_INTERNAL_ASSERT(invariant())

      return *this;
      }

   UString& replace(uint32_t pos, uint32_t n, char c)
      {
      U_TRACE(0, "UString::replace(%u,%u,%C)", pos, n, c)

      char* ptr = __replace(pos, n, 1);

      if (ptr) *ptr = c;

      U_INTERNAL_ASSERT(invariant())

      return *this;
      }

   UString& replace(uint32_t pos, uint32_t n1, const char* s, uint32_t n2)
      {
      U_TRACE(0, "UString::replace(%u,%u,%S,%u)", pos, n1, s, n2)

      char* ptr = __replace(pos, n1, n2);

      if (ptr && n2) U_MEMCPY(ptr, s, n2);

      U_INTERNAL_ASSERT(invariant())

      return *this;
      }

   UString& replace(uint32_t pos1, uint32_t n1, const UString& str, uint32_t pos2, uint32_t n2 = U_NOT_FOUND)
      {
      U_TRACE(0, "UString::replace(%u,%u,%p,%u,%u)", pos1, n1, &str, pos2, n2)

      U_INTERNAL_ASSERT(pos2 <= str.size())

      return replace(pos1, n1, str.data() + pos2, str.rep->fold(pos2, n2));
      }

   UString& replace(const char* s)                                { return replace(0U, size(), s, u__strlen(s, __PRETTY_FUNCTION__)); }
   UString& replace(unsigned char c)                              { return replace(0U, size(), 1U, c); }
   UString& replace(const UString& str)                           { return replace(0U, size(), U_STRING_TO_PARAM(str)); }
   UString& replace(const char* s, uint32_t n)                    { return replace(0U, size(), s, n); }
   UString& replace(uint32_t pos, uint32_t n, const char* s)      { return replace(pos,     n, s, u__strlen(s, __PRETTY_FUNCTION__)); }
   UString& replace(uint32_t pos, uint32_t n, const UString& str) { return replace(pos,     n, U_STRING_TO_PARAM(str)); }

   // Assignment - NB: assign() DOES NOT WARRANT PROPERTIES OF STRING, replace() YES...

   UString& assign(const char* s, uint32_t n);

   UString& assign(const UString& str, uint32_t pos, uint32_t n = U_NOT_FOUND)
      {
      U_TRACE(0, "UString::assign(%p,%u,%u)", &str, pos, n)

      U_INTERNAL_ASSERT(pos <= str.size())

      return assign(str.data() + pos, str.rep->fold(pos, n));
      }

   UString& assign(const char* s)               { return assign(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   UString& assign(const UString& str)          { return assign(U_STRING_TO_PARAM(str)); }
   UString& assign(uint32_t n, unsigned char c) { return replace(0U, size(), n, c); }

   UString& operator=(const char* s)   { return assign(s); }
   UString& operator=(unsigned char c) { return assign(1U, c); }

   void shift(ptrdiff_t diff) { rep->shift(diff); }

   // Make room for a total of n element

   bool reserve(uint32_t n)
      {
      U_TRACE(0, "UString::reserve(%u)", n)

      if (rep->space() < n)
         {
         _reserve(*this, rep->_length + n);

         U_RETURN(true); // return true if it has changed rep...
         }

      U_RETURN(false);
      }

   static void _reserve(UString& buffer, uint32_t n)
      {
      U_TRACE(0, "UString::_reserve(%V,%u)", buffer.rep, n)

      UStringRep* rep = buffer.rep;

      U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_length = %u rep->_capacity = %u",
                       rep,     rep->parent,     rep->references,     rep->child,     rep->_length,     rep->_capacity)

      U_ASSERT(rep->space() < n)
      U_INTERNAL_ASSERT(n <= max_size())
      U_INTERNAL_ASSERT_MAJOR(n, rep->_length)

      buffer._set(UStringRep::create(rep->_length, n, rep->str));

      U_INTERNAL_ASSERT(buffer.invariant())
      }

   // Element access

   char         at(uint32_t pos) const { return rep->at(pos); }
   char operator[](uint32_t pos) const { return rep->operator[](pos); }

   char* pend() const { return rep->pend(); }

   // operator const char *() const { return rep->data(); }
   // operator       char *()       { return rep->data(); }

   // Modifiers

   void push_back(unsigned char c) { (void) append(1U, c); }

   UString& append(uint32_t n, char c); // NB: unsigned char conflict with a uint32_t at the same parameter position...

   UString& append(const char* s, uint32_t n)
      {
      U_TRACE(0, "UString::append(%.*S,%u)", n, s, n)

      if (n)
         {
         char* ptr = __append(n);

         U_MEMCPY(ptr, s, n);
         }

      U_INTERNAL_ASSERT(invariant())

      return *this;
      }

   UString& append(const UString& str, uint32_t pos, uint32_t n = U_NOT_FOUND)
      {
      U_TRACE(0, "UString::append(%p,%u,%u)", &str, pos, n)

      U_INTERNAL_ASSERT(pos <= str.size())

      return append(str.data() + pos, str.rep->fold(pos, n));
      }

   UString& append(const char* s)          { return append(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   UString& append(UStringRep* _rep)       { return append(_rep->str, _rep->_length); }
   UString& append(const UString& str)     { return append(str.data(), str.size()); }

   UString& operator+=(const char* s)      { return append(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   UString& operator+=(unsigned char c)    { return append(1U, c); }
   UString& operator+=(const UString& str) { return append(str.data(), str.size()); }

   // OPTMIZE APPEND (BUFFERED)

   static char* ptrbuf;
   static char* appbuf;

   void _append(unsigned char c)
      {
      U_TRACE(0, "UString::_append(%C)", c)

      U_INTERNAL_ASSERT_POINTER(ptrbuf)
      U_INTERNAL_ASSERT_POINTER(appbuf)

      if ((ptrbuf - appbuf) == 1024)
         {
         (void) append(appbuf, 1024);

         ptrbuf = appbuf;
         }

      *ptrbuf++ = c;
      }

   void _append()
      {
      U_TRACE_NO_PARAM(0, "UString::_append()")

      U_INTERNAL_ASSERT_POINTER(ptrbuf)
      U_INTERNAL_ASSERT_POINTER(appbuf)

      if (ptrbuf > appbuf)
         {
         (void) append(appbuf, ptrbuf - appbuf);

         ptrbuf = appbuf;
         }
      }

   // operator +

   friend UString operator+(const UString& lhs, const char*    rhs);
   friend UString operator+(const UString& lhs,       char     rhs);
   friend UString operator+(const UString& lhs, const UString& rhs);
   friend UString operator+(char           lhs, const UString& rhs);
   friend UString operator+(const char*    lhs, const UString& rhs);

   UString& insert(uint32_t pos, const UString& str) { return replace(pos, 0, U_STRING_TO_PARAM(str)); }

   UString& insert(uint32_t pos1, const UString& str, uint32_t pos2, uint32_t n = U_NOT_FOUND)
      {
      U_TRACE(0, "UString::insert(%u,%p,%u,%u)", pos1, &str, pos2, n)

      U_INTERNAL_ASSERT(pos2 <= str.size())

      return replace(pos1, 0, str.data() + pos2, str.rep->fold(pos2, n));
      }

   UString& insert(uint32_t pos,             char c) { return replace(pos, 0, 1, c); }
   UString& insert(uint32_t pos, uint32_t n, char c) { return replace(pos, 0, n, c); } // NB: uchar conflict with a uint32_t at the same parameter position

   UString& insert(uint32_t pos, const char* s)             { return replace(pos, 0, s, u__strlen(s, __PRETTY_FUNCTION__)); }
   UString& insert(uint32_t pos, const char* s, uint32_t n) { return replace(pos, 0, s, n); }

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UString::clear()")

      if (rep != UStringRep::string_rep_null) _assign(UStringRep::string_rep_null);

      U_INTERNAL_ASSERT(invariant())
      }

   void resize(uint32_t n, unsigned char c);

   // it can shrink the space used (capacity)...

   bool     shrink();
   UString& erase(uint32_t pos = 0, uint32_t n = U_NOT_FOUND) { return replace(pos, rep->fold(pos, n), "", 0); }

   // C-Style String

   void setNullTerminated() const
      {
      U_TRACE_NO_PARAM(0, "UString::setNullTerminated()")

      U_INTERNAL_ASSERT_MAJOR(rep->_length, 0)

      if (writeable()) rep->setNullTerminated();
      else             ((UString*)this)->duplicate();

      U_ASSERT_EQUALS(u__strlen(rep->str, __PRETTY_FUNCTION__), rep->_length)
      }

   char* c_str() const
      {
      U_TRACE_NO_PARAM(0, "UString::c_str()")

      if (rep                                &&
          rep != UStringRep::string_rep_null &&
          rep->str[rep->_length] != '\0')
         {
         U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_length = %u rep->_capacity = %u",
                          rep,     rep->parent,     rep->references,     rep->child,     rep->_length,     rep->_capacity)

         ((char*)rep->str)[rep->_length] = '\0';
         }

      return (char*)rep->str;
      }

   char* c_strdup() const                                            { return strndup(rep->str, rep->_length); }
   char* c_strndup(uint32_t pos = 0, uint32_t n = U_NOT_FOUND) const { return strndup(rep->str+pos, rep->fold(pos, n)); }

   UString copy() const
      {
      U_TRACE_NO_PARAM(0, "UString::copy()")

      if (rep->_length)
         {
         U_INTERNAL_ASSERT_EQUALS(rep->_capacity, 0) // mode: 0 -> const

         UString copia((void*)rep->str, rep->_length);

         U_RETURN_STRING(copia);
         }

      return getStringNull();
      }

   uint32_t copy(char* s, uint32_t n = U_NOT_FOUND, uint32_t pos = 0) const { return rep->copy(s, n, pos); }

   // STRING OPERATIONS

   // The `find' function searches string for a specified string (possibly a single character) and returns
   // its starting position. You can supply the parameter pos to specify the position where search must begin

   uint32_t find(unsigned char c,    uint32_t pos = 0,             uint32_t how_much = U_NOT_FOUND) const __pure;
   uint32_t find(const char* s,      uint32_t pos, uint32_t s_len, uint32_t how_much = U_NOT_FOUND) const __pure;
   uint32_t find(const UString& str, uint32_t pos = 0,             uint32_t how_much = U_NOT_FOUND) const { return find(str.data(), pos, str.size(), how_much); }

   // The `rfind' function searches from end to beginning string for a specified string (possibly a single character)
   // and returns its starting position. You can supply the parameter pos to specify the position where search must begin

   uint32_t rfind(const char* s,      uint32_t pos, uint32_t n) const __pure;
   uint32_t rfind(unsigned char c,    uint32_t pos = U_NOT_FOUND) const __pure;
   uint32_t rfind(const UString& str, uint32_t pos = U_NOT_FOUND) const { return rfind(str.data(), pos, str.size()); }

   // The `find_first_of' function searches string for the first match of any character stored in s and returns its position

   uint32_t find_first_of(const char* s,      uint32_t pos, uint32_t n) const __pure;
   uint32_t find_first_of(unsigned char c,    uint32_t pos = 0) const { return find(c, pos); }
   uint32_t find_first_of(const UString& str, uint32_t pos = 0) const { return find_first_of(str.data(), pos, str.size()); }

   // The `find_last_of' function searches string for the last  match of any character stored in s and returns its position

   uint32_t find_last_of(const char* s,       uint32_t pos, uint32_t n) const __pure;
   uint32_t find_last_of(unsigned char c,     uint32_t pos = U_NOT_FOUND) const { return rfind(c, pos); }
   uint32_t find_last_of(const UString& str,  uint32_t pos = U_NOT_FOUND) const { return find_last_of(str.data(), pos, str.size()); }

   // The `find_first_not_of' function searches the first element of string that doesn't match any character stored in s and returns its position

   uint32_t find_first_not_of(const char* s,      uint32_t pos, uint32_t n) const __pure;
   uint32_t find_first_not_of(unsigned char c,    uint32_t pos = 0) const __pure;
   uint32_t find_first_not_of(const UString& str, uint32_t pos = 0) const { return find_first_not_of(str.data(), pos, str.size()); }

   // The `find_last_not_of' function searches the last element of string that doesn't match any character stored in s and returns its position

   uint32_t find_last_not_of(const char* s,      uint32_t pos, uint32_t n) const __pure;
   uint32_t find_last_not_of(unsigned char c,    uint32_t pos = U_NOT_FOUND) const __pure;
   uint32_t find_last_not_of(const UString& str, uint32_t pos = U_NOT_FOUND) const { return find_last_not_of(str.data(), pos, str.size()); }

   // Find with ignore case

   uint32_t findnocase(const char* s,      uint32_t pos, uint32_t s_len, uint32_t how_much = U_NOT_FOUND) const __pure;
   uint32_t findnocase(const UString& str, uint32_t pos = 0,             uint32_t how_much = U_NOT_FOUND) const { return findnocase(str.data(), pos, str.size(), how_much); }

   uint32_t findWhiteSpace(uint32_t pos = 0) const __pure { return rep->findWhiteSpace(pos); }

   // Compare

   int compare(const char* s) const             { return rep->compare(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   int compare(const char* s, uint32_t n) const { return rep->compare(s, n); }

   int compare(UStringRep* _rep) const          { return rep->compare(_rep); }
   int compare(const UString& str) const        { return rep->compare(str.rep); }

   int compare(uint32_t pos, const char* s) const
      { return rep->compare(pos, u__strlen(s, __PRETTY_FUNCTION__), s, u__strlen(s, __PRETTY_FUNCTION__)); }

   int compare(uint32_t pos, uint32_t n1, const char* s, uint32_t n2) const
      { return rep->compare(pos, U_min(size() - pos, n1), s, U_min(u__strlen(s, __PRETTY_FUNCTION__), n2)); }

   int compare(uint32_t pos, uint32_t n, const char* s) const
      { return rep->compare(pos, U_min(size() - pos, n), s, u__strlen(s, __PRETTY_FUNCTION__)); }

   int compare(uint32_t pos, uint32_t n, const UString& str) const
      { return rep->compare(pos, U_min(size() - pos, n), U_STRING_TO_PARAM(str)); }

   int compare(uint32_t pos1, uint32_t n1, const UString& str, uint32_t pos2, uint32_t n2) const
      { return rep->compare(pos1, U_min(size() - pos1, n1), str.data() + pos2, U_min(str.size() - pos2, n2)); }

   // Compare with ignore case

   int comparenocase(const char* s, uint32_t n) const { return rep->comparenocase(s, n); }
   int comparenocase(const char* s) const             { return rep->comparenocase(s, u__strlen(s, __PRETTY_FUNCTION__)); }

   int comparenocase(UStringRep* _rep) const          { return rep->comparenocase(_rep); }
   int comparenocase(const UString& str) const        { return rep->comparenocase(str.rep); }

   // Same string representation

   bool same(UStringRep* _rep) const   { return (rep == _rep); }
   bool same(const UString& str) const { return same(str.rep); }

   // Equal

   bool equal(char c) const                    { return rep->equal(c); } 
   bool equal(const char* s) const             { return rep->equal(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   bool equal(const char* s, uint32_t n) const { return rep->equal(s, n); }
   bool equal(UStringRep* _rep) const          { return same(_rep) || rep->equal(_rep); }
   bool equal(const UString& str) const        { return equal(str.rep); }

   // Equal with ignore case

   bool equalnocase(char c) const                    { return rep->equalnocase(c); }
   bool equalnocase(const char* s) const             { return rep->equalnocase(s, u__strlen(s, __PRETTY_FUNCTION__)); }
   bool equalnocase(const char* s, uint32_t n) const { return rep->equalnocase(s, n); }
   bool equalnocase(UStringRep* _rep) const          { return same(_rep) || rep->equalnocase(_rep); }
   bool equalnocase(const UString& str) const        { return equalnocase(str.rep); }

   // STREAM

#ifdef U_STDCPP_ENABLE
   istream& getline(istream& is, unsigned char delim = '\n');

   friend U_EXPORT istream& operator>>(istream& is,       UString& str);
   friend U_EXPORT ostream& operator<<(ostream& os, const UString& str);
   // --------------------------------------------------------------
   // STREAM - EXTENSION TO STDLIBC++
   // --------------------------------------------------------------
   void   get(istream& is);
   void write(ostream& os) const { rep->write(os); }
   // --------------------------------------------------------------
#endif

#ifdef DEBUG
   bool invariant() const;
# ifdef U_STDCPP_ENABLE
   const char* dump(bool _reset) const;
# endif
#endif

   // -----------------------------------------------------------------------------------------------------------------------
   // START EXTENSION TO STDLIBC++
   // -----------------------------------------------------------------------------------------------------------------------

   bool isNull() const                                          { return (rep == UStringRep::string_rep_null); }
   bool isNullTerminated() const                                { return rep->isNullTerminated(); }
   bool isMacAddr() const                                       { return rep->isMacAddr(); }
   bool isIPv4Addr() const                                      { return rep->isIPv4Addr(); }
   bool isXMacAddr() const                                      { return rep->isXMacAddr(); }
   bool isText(uint32_t pos = 0) const                          { return rep->isText(pos); }
   bool isUTF8(uint32_t pos = 0) const                          { return rep->isUTF8(pos); }
   bool isUTF16(uint32_t pos = 0) const                         { return rep->isUTF16(pos); }
   bool isNumber(uint32_t pos = 0) const                        { return rep->isNumber(pos); }
   bool isBinary(uint32_t pos = 0) const                        { return rep->isBinary(pos); }
   bool isBase64(uint32_t pos = 0) const                        { return rep->isBase64(pos); }
   bool isBase64Url(uint32_t pos = 0) const                     { return rep->isBase64Url(pos); }
   bool isWhiteSpace(uint32_t pos = 0) const                    { return rep->isWhiteSpace(pos); }
   bool isPrintable(uint32_t pos = 0, bool bline = false) const { return rep->isPrintable(pos, bline); }

   bool   isEndHeader(uint32_t pos = 0) const                   { return rep->isEndHeader(pos); }
   bool findEndHeader(uint32_t pos = 0) const                   { return rep->findEndHeader(pos); }

   char  last_char() const { return rep->last_char(); }
   char first_char() const { return rep->first_char(); }

   void setLastChar(char c) { rep->setLastChar(c); }

   char  c_char(uint32_t pos) const    { return rep->at(pos); }
   char* c_pointer(uint32_t pos) const { return (char*)rep->c_pointer(pos); }

   ptrdiff_t  remain(const char* ptr) const { return rep->remain(ptr); }
   uint32_t distance(const char* ptr) const { return rep->distance(ptr); }

   void setFromInode(uint64_t* p)  { (void) replace((const char*)p, sizeof(uint64_t)); }

   uint32_t hash() const           { return rep->hash(); }
   uint32_t hashIgnoreCase() const { return rep->hashIgnoreCase(); }

   // references

   void     hold()    const   {        rep->hold(); }
   void     release() const   {        rep->release(); }
   uint32_t reference() const { return rep->references; }

   // for constant string

   void trim() const { rep->trim(); }

   // check if the string is quoted...

   bool isQuoted(unsigned char c = '"') const { return rep->isQuoted(c); }

   // ...unquote it

   void   unQuote();
   bool needQuote() const { return rep->needQuote(); }

   UString getUnQuoted()
      {
      U_TRACE_NO_PARAM(0, "UString::getUnQuoted()")

      const char* ptr = rep->str;

      if (ptr[0] == '"')
         {
         uint32_t sz = rep->_length;

         U_INTERNAL_ASSERT_EQUALS(ptr[sz-1], '"')

         UString copia(ptr+1, sz-2);

         U_RETURN_STRING(copia);
         }

      return *this;
      }

   uint32_t getSpaceToDump() const { return rep->getSpaceToDump(); }

   // set uniq

   void duplicate()
      {
      U_TRACE_NO_PARAM(0, "UString::duplicate()")

      uint32_t sz = size();

      U_INTERNAL_ASSERT_MAJOR(sz, 0)
      U_INTERNAL_ASSERT_EQUALS(rep->_capacity, 0) // mode: 0 -> const

      _set(UStringRep::create(sz, sz, rep->str));

      U_INTERNAL_ASSERT(invariant())
      }

   bool uniq() const      { return rep->uniq(); }
   bool writeable() const { return rep->writeable(); }

   // manage UString as constant string...

   bool isConstant() const
      {
      U_TRACE_NO_PARAM(0, "UString::isConstant()")

      if (rep->writeable()) U_RETURN(false);

      U_RETURN(true);
      }

   void setConstant(const char* t, uint32_t tlen)
      {
      U_TRACE(0, "UString::setConstant(%.*S,%u)", tlen, t, tlen)
 
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      UStringRep* r;

      U_NEW(UStringRep, r, UStringRep(t, tlen))

      _set(r);

      U_INTERNAL_ASSERT(invariant())

      U_INTERNAL_ASSERT_EQUALS(data(), t)
      }

   // manage UString as memory mapped area...

   bool isMmap() const
      {
      U_TRACE_NO_PARAM(0, "UString::isMmap()")

      if (rep->_capacity == U_NOT_FOUND) U_RETURN(true);

      U_RETURN(false);
      }

   void mmap(const char* map, uint32_t len);

   // manage UString as buffer...

   void setEmpty()
      {
      U_TRACE_NO_PARAM(0, "UString::setEmpty()")

      U_INTERNAL_ASSERT_DIFFERS(rep->_capacity, 0) // mode: 0 -> const

      rep->size_adjust(0U);
      }

   void setEmptyForce()
      {
      U_TRACE_NO_PARAM(0, "UString::setEmptyForce()")

      U_INTERNAL_ASSERT_DIFFERS(rep->_capacity, 0) // mode: 0 -> const

      rep->size_adjust_force(0U);
      }

   void setBuffer(uint32_t n)
      {
      U_TRACE(0, "UString::setBuffer(%u)", n)

      if (rep->references ||
          n > rep->_capacity)
         {
         resize(n);
         }
      else
         {
         rep->_length = 0;
         }
      }

   void setBufferForce(uint32_t n)
      {
      U_TRACE(0, "UString::setBufferForce(%u)", n)

      if (n > rep->_capacity)
         {
         resize(n);
         }
      else
         {
         rep->_length = 0;
         }
      }

   void moveToBeginDataInBuffer(uint32_t n)
      {
      U_TRACE(1, "UString::moveToBeginDataInBuffer(%u)", n)

      U_INTERNAL_ASSERT_MAJOR(rep->_length, n)
      U_INTERNAL_ASSERT_RANGE(1, n, max_size())
      U_INTERNAL_ASSERT_MAJOR(rep->_capacity, n)

#  if defined(DEBUG) && !defined(U_SUBSTR_INC_REF)
      U_INTERNAL_ASSERT(rep->references == 0)
#  endif

      rep->_length -= n;

      (void) U_SYSCALL(memmove, "%p,%p,%u", (void*)rep->str, rep->str + n, rep->_length);

      U_INTERNAL_ASSERT(invariant())
      }

   static vpFpcu printValueToBuffer;

   void printKeyValue(const char* key, uint32_t keylen, const char* data, uint32_t datalen);

   void snprintf(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UString::snprintf(%.*S,%u)", fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)

      va_list argp;
      va_start(argp, fmt_size);

      UString::vsnprintf(format, fmt_size, argp); 

      va_end(argp);
      }

   void snprintf_add(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UString::snprintf_add(%.*S,%u)", fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)

      va_list argp;
      va_start(argp, fmt_size);

      UString::vsnprintf_add(format, fmt_size, argp); 

      va_end(argp);
      }

   void snprintf_pos(uint32_t pos, const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UString::snprintf_pos(%u,%.*S,%u)", pos, fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)
      U_INTERNAL_ASSERT(pos <= rep->_length)

      va_list argp;
      va_start(argp, fmt_size);

      rep->_length = pos + u__vsnprintf(c_pointer(pos), rep->_capacity+1-pos, format, fmt_size, argp); // NB: +1 because we want space for null-terminator...

      U_INTERNAL_DUMP("ret = %u buffer_size = %u", rep->_length, rep->_capacity+1-pos)

      U_INTERNAL_ASSERT(invariant())

      va_end(argp);
      }

   void size_adjust()                      { rep->size_adjust(); }
   void size_adjust(uint32_t value)        { rep->size_adjust(value); }
   void size_adjust(const char* ptr)       { rep->size_adjust(ptr); }

   void size_adjust_force()                { rep->size_adjust_force(); }
   void size_adjust_force(uint32_t value)  { rep->size_adjust_force(value); }
   void size_adjust_force(const char* ptr) { rep->size_adjust_force(ptr); }

   void size_adjust_constant(uint32_t sz)     { rep->size_adjust_constant(sz); }
   void size_adjust_constant(const char* ptr) { rep->size_adjust_constant(ptr); }

   void setUpTime()
      {
      U_TRACE_NO_PARAM(0, "UString::setUpTime()")

#  ifdef DEBUG
      vsnprintf_check(U_CONSTANT_TO_PARAM("1234567890"));

      U_ASSERT(uniq())
#  endif

      char* ptr = (char*)rep->str;

      ptr[(rep->_length = u_set_uptime(ptr))] = '\0';

      U_INTERNAL_ASSERT(invariant())
      }

   void setFromNumber32(uint32_t number)
      {
      U_TRACE(0, "UString::setFromNumber32(%u)", number)

#  ifdef DEBUG
      vsnprintf_check(U_CONSTANT_TO_PARAM("1234567890"));

      U_ASSERT(uniq())
#  endif

      char* ptr = (char*)rep->str;

      ptr[(rep->_length = u_num2str32(number, ptr) - ptr)] = '\0';

      U_INTERNAL_ASSERT(invariant())
      }

   void setFromNumber32s(int32_t number)
      {
      U_TRACE(0, "UString::setFromNumber32s(%d)", number)

#  ifdef DEBUG
      vsnprintf_check(U_CONSTANT_TO_PARAM("1234567890"));

      U_ASSERT(uniq())
#  endif

      char* ptr = (char*)rep->str;

      ptr[(rep->_length = u_num2str32s(number, ptr) - ptr)] = '\0';

      U_INTERNAL_ASSERT(invariant())
      }

#ifdef DEBUG
   void vsnprintf_check(const char* format, uint32_t fmt_size) const;
#endif

   void setFromNumber64(uint64_t number)
      {
      U_TRACE(0, "UString::setFromNumber64(%llu)", number)

#  ifdef DEBUG
      vsnprintf_check(U_CONSTANT_TO_PARAM("18446744073709551615"));

      U_ASSERT(uniq())
#  endif

      char* ptr = (char*)rep->str;

      ptr[(rep->_length = u_num2str64(number, ptr) - ptr)] = '\0';

      U_INTERNAL_ASSERT(invariant())
      }

   void setFromNumber64s(int64_t number)
      {
      U_TRACE(0, "UString::setFromNumber64s(%lld)", number)

#  ifdef DEBUG
      vsnprintf_check(U_CONSTANT_TO_PARAM("18446744073709551615"));

      U_ASSERT(uniq())
#  endif

      char* ptr = (char*)rep->str;

      ptr[(rep->_length = u_num2str64s(number, ptr) - ptr)] = '\0';

      U_INTERNAL_ASSERT(invariant())
      }

   void appendNumber32(uint32_t number)
      {
      U_TRACE(0, "UString::appendNumber32(%u)", number)

      U_ASSERT_MAJOR(space(), 12)

      char* ptr = pend();

      rep->_length += u_num2str32(number, ptr) - ptr;

      U_INTERNAL_ASSERT(invariant())
      }

   void appendNumber32s(int32_t number)
      {
      U_TRACE(0, "UString::appendNumber32s(%d)", number)

      U_ASSERT_MAJOR(space(), 12)

      char* ptr = pend();

      rep->_length += u_num2str32s(number, ptr) - ptr;

      U_INTERNAL_ASSERT(invariant())
      }

   void appendNumber64(uint64_t number)
      {
      U_TRACE(0, "UString::appendNumber64(%llu)", number)

      U_ASSERT_MAJOR(space(), 22)

      char* ptr = pend();

      rep->_length += u_num2str64(number, ptr) - ptr;

      U_INTERNAL_ASSERT(invariant())
      }

   void appendNumber64s(int64_t number)
      {
      U_TRACE(0, "UString::appendNumber64s(%lld)", number)

      U_ASSERT_MAJOR(space(), 22)

      char* ptr = pend();

      rep->_length += u_num2str64s(number, ptr) - ptr;

      U_INTERNAL_ASSERT(invariant())
      }

   void appendNumberDouble(double number)
      {
      U_TRACE(0, "UString::appendNumberDouble(%g)", number)

      U_ASSERT_MAJOR(space(), 22)

      char* ptr = pend();

      rep->_length += u_dtoa(number, ptr) - ptr;

      U_INTERNAL_ASSERT(invariant())
      }

   void appendData(const char* t, uint32_t tlen)
      {
      U_TRACE(0, "UString::appendData(%.*S,%u)", tlen, t, tlen)

      U_ASSERT_MAJOR(space(), tlen)
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)

      char* ptr = pend();

      U_MEMCPY(ptr, t, tlen);

      rep->_length += tlen;

      U_INTERNAL_ASSERT(invariant())
      }

   void appendDataQuoted(const char* t, uint32_t tlen)
      {
      U_TRACE(0, "UString::appendDataQuoted(%.*S,%u)", tlen, t, tlen)

      U_ASSERT_MAJOR(space(), tlen+2)
      U_INTERNAL_ASSERT_EQUALS(u_is_quoted(t, tlen), false)

      char* ptr = pend();

      *ptr++ = '"';

      if (tlen) U_MEMCPY(ptr, t, tlen);

      ptr[tlen] = '"';

      rep->_length += tlen+2;

      U_INTERNAL_ASSERT(invariant())
      }

   void vsnprintf(const char* format, uint32_t fmt_size, va_list argp)
      {
      U_TRACE(0, "UString::vsnprintf(%.*S,%u)", fmt_size, format, fmt_size)

#  ifdef DEBUG
      vsnprintf_check(format, fmt_size);
#  endif

      rep->_length = u__vsnprintf(rep->data(), rep->_capacity+1, format, fmt_size, argp); // NB: +1 because we want space for null-terminator...

      U_INTERNAL_DUMP("ret = %u buffer_size = %u", rep->_length, rep->_capacity+1)

      U_INTERNAL_ASSERT(invariant())
      }

   void vsnprintf_add(const char* format, uint32_t fmt_size, va_list argp)
      {
      U_TRACE(0, "UString::vsnprintf_add(%.*S,%u)", fmt_size, format, fmt_size)

#  ifdef DEBUG
      vsnprintf_check(format, fmt_size);
#  endif

      uint32_t ret = u__vsnprintf(c_pointer(rep->_length), rep->space()+1, format, fmt_size, argp); // NB: +1 because we want space for null-terminator...

      U_INTERNAL_DUMP("ret = %u buffer_size = %u", ret, rep->space()+1)

      rep->_length += ret; 

      U_INTERNAL_ASSERT(invariant())
      }

#ifdef HAVE_STRTOF
   float strtof() const { return rep->strtof(); }
#endif
#ifdef HAVE_STRTOLD
   long double strtold() const { return rep->strtold(); }
#endif

     bool strtob() const { return rep->strtob(); }
   double strtod() const;

    int64_t strtoll( bool check_for_suffix = false) const { return rep->strtoll( check_for_suffix); } 
   uint64_t strtoull(bool check_for_suffix = false) const { return rep->strtoull(check_for_suffix); }

            long strtol( bool check_for_suffix = false) const  { return rep->strtol( check_for_suffix); }
   unsigned long strtoul(bool check_for_suffix = false) const  { return rep->strtoul(check_for_suffix); }   

   // UTF8 <--> ISO Latin 1

   static UString toUTF8(const unsigned char* t, uint32_t tlen)
      {
      U_TRACE(0, "UString::toUTF8(%.*S,%u)", tlen, t, tlen)

      if (tlen == 0) return *string_null; 

      UStringRep* rep = UStringRep::toUTF8(t, tlen);

      UString utf8(&rep);

      U_RETURN_STRING(utf8);
      }

   static UString fromUTF8(const unsigned char* t, uint32_t tlen)
      {
      U_TRACE(0, "UString::fromUTF8(%.*S,%u)", tlen, t, tlen)

      if (tlen == 0) return *string_null; 

      UStringRep* rep = UStringRep::fromUTF8(t, tlen);

      UString isolat1(&rep);

      U_RETURN_STRING(isolat1);
      }

   template <typename T> void toJSON(const char* name, uint32_t sz, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UString::toJSON<T>(%.*S,%u,%p)", sz, name, sz, &member)

      U_ASSERT_MAJOR(space(), sz+6)
      U_INTERNAL_ASSERT_MAJOR(sz, 0)
      U_INTERNAL_ASSERT(u_is_quoted(name, sz))

      char* ptr = pend();

      U_MEMCPY(ptr, name, sz);

      ptr[sz] = ':';

      rep->_length += sz+1;

      member.toJSON(*this);

      __push(',');
      }

   template <typename T> void toJSON(const UString& name, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UString::toJSON<T>(%V,%p)", name.rep, &member)

      uint32_t tlen = name.size();
      const char* t = name.data();

      U_ASSERT_MAJOR(space(), tlen+6)
      U_INTERNAL_ASSERT_MAJOR(tlen, 0)
      U_INTERNAL_ASSERT_EQUALS(u_is_quoted(t, tlen), false)

      char* ptr = pend();

      *ptr++ = '"';

      U_MEMCPY(ptr, t, tlen);
               ptr  += tlen;

      u_put_unalignedp16(ptr, U_MULTICHAR_CONSTANT16('"',':'));

      rep->_length += tlen+3;

      member.toJSON(*this);

      __push(',');
      }

   // -----------------------------------------------------------------------------------------------------------------------
   // END EXTENSION TO STDLIBC++
   // -----------------------------------------------------------------------------------------------------------------------

private:
   char* __append(uint32_t n);
   char* __replace(uint32_t pos, uint32_t n1, uint32_t n2);

   void resize(uint32_t n)
      {
      U_TRACE(0, "UString::resize(%u)", n)

      U_INTERNAL_ASSERT_RANGE(1, n, max_size())

      U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_capacity = %u",
                       rep,     rep->parent,     rep->references,     rep->child,     rep->_capacity)

      _set(UStringRep::create(0U, (n < U_CAPACITY ? U_CAPACITY : n), U_NULLPTR));

      U_INTERNAL_ASSERT(invariant())
      }

   void __push(uint8_t c)
      {
      U_TRACE(0, "UString::__push(%u)", c)

      U_ASSERT_MAJOR(space(), 1)

      uint8_t* ptr = (uint8_t*)(rep->str + rep->_length++);

      *ptr = c;

      U_INTERNAL_ASSERT(invariant())
      }

   friend class UHTTP;
   friend class USSEThread;
   friend class URDBClient_Base;

   template <class T> friend class UJsonTypeHandler;
};

// operator ==

inline bool operator==(const UStringRep& lhs, const UStringRep& rhs) { return lhs.equal(&rhs); }
inline bool operator==(const UString& lhs,    const UString& rhs)    { return lhs.equal(rhs); }
inline bool operator==(const char* lhs,       const UString& rhs)    { return rhs.equal(lhs); }
inline bool operator==(const UString& lhs,    const char* rhs)       { return lhs.equal(rhs); }

// operator !=

inline bool operator!=(const UStringRep& lhs, const UStringRep& rhs) { return lhs.equal(&rhs) == false; }
inline bool operator!=(const UString& lhs,    const UString& rhs)    { return lhs.equal(rhs)  == false; }
inline bool operator!=(const char* lhs,       const UString& rhs)    { return rhs.equal(lhs)  == false; }
inline bool operator!=(const UString& lhs,    const char* rhs)       { return lhs.equal(rhs)  == false; }

// operator <

inline bool operator<(const UStringRep& lhs, const UStringRep& rhs)  { return lhs.compare(&rhs) < 0; }
inline bool operator<(const UString& lhs,    const UString& rhs)     { return lhs.compare(rhs)  < 0; }
inline bool operator<(const char* lhs,       const UString& rhs)     { return rhs.compare(lhs)  > 0; }
inline bool operator<(const UString& lhs,    const char* rhs)        { return lhs.compare(rhs)  < 0; }

// operator >

inline bool operator>(const UStringRep& lhs, const UStringRep& rhs)  { return lhs.compare(&rhs) > 0; }
inline bool operator>(const UString& lhs,    const UString& rhs)     { return lhs.compare(rhs)  > 0; }
inline bool operator>(const char* lhs,       const UString& rhs)     { return rhs.compare(lhs)  < 0; }
inline bool operator>(const UString& lhs,    const char* rhs)        { return lhs.compare(rhs)  > 0; }

// operator <=

inline bool operator<=(const UStringRep& lhs, const UStringRep& rhs) { return lhs.compare(&rhs) <= 0; }
inline bool operator<=(const UString& lhs,    const UString& rhs)    { return lhs.compare(rhs)  <= 0; }
inline bool operator<=(const char* lhs,       const UString& rhs)    { return rhs.compare(lhs)  >= 0; }
inline bool operator<=(const UString& lhs,    const char* rhs)       { return lhs.compare(rhs)  <= 0; }

// operator >=

inline bool operator>=(const UStringRep& lhs, const UStringRep& rhs) { return lhs.compare(&rhs) >= 0; }
inline bool operator>=(const UString& lhs,    const UString& rhs)    { return lhs.compare(rhs)  >= 0; }
inline bool operator>=(const char* lhs,       const UString& rhs)    { return rhs.compare(lhs)  <= 0; }
inline bool operator>=(const UString& lhs,    const char* rhs)       { return lhs.compare(rhs)  >= 0; }

// template specialization for UString2Object() and UObject2String()

template <> inline void UString2Object<UString>(const char* t, uint32_t tlen, UString& object)
{
   U_TRACE(0, "UString2Object<UString>(%.*S,%u,%V)", tlen, t, tlen, object.rep)

   object.setConstant(t, tlen);
}

template <> inline char* UObject2String<UString>(UString& object)
{
   U_TRACE(0, "UObject2String<UString>(%V)", object.rep)

   U_INTERNAL_ASSERT(object.isNullTerminated())

   return object.data();
}

template <> inline uint32_t UObject2String<UString>(UString& object, char* pbuffer, uint32_t buffer_size)
{
   U_TRACE(0, "UObject2String<UString>(%V,%p,%u)", object.rep, pbuffer, buffer_size)

   uint32_t sz = object.size();

   U_INTERNAL_ASSERT_MINOR(sz, buffer_size)

   if (sz)
      {
      U_MEMCPY(pbuffer, object.data(), sz);

      return sz;
      }

   return 0;
}

// by Victor Stewart

#if defined(U_STDCPP_ENABLE)
#  if defined(HAVE_CXX11)
namespace std {
   template<> struct hash<UString> {
      std::size_t operator()(const UString& str) const noexcept { return str.hash(); }
   };
}
#  endif
#  if defined(HAVE_CXX17) && defined(U_LINUX)
#     include <utility> // std::index_sequence
template <char... Chars>
class UCompileTimeStringView {
private:

   template <size_t Offset, size_t... Indexs> constexpr
   auto substr(std::index_sequence<Indexs...>) const
   {
      return UCompileTimeStringView<string[Indexs + Offset]...>{};
   }

public:

   static constexpr char string[sizeof...(Chars)+1] = {Chars..., '\0'};
   static constexpr size_t length = sizeof...(Chars);

   constexpr char operator[](size_t index) const { return string[index]; }

   static constexpr UCompileTimeStringView instance = {};

   // implicit cast to zero-terminated C-string
   // constexpr operator const char *() const { return string; }

   static constexpr char const * cbegin() noexcept { return &string[0]; }
    
   static constexpr char const * cend() noexcept { return &string[length]; }

   template <char... OChars> constexpr
   UCompileTimeStringView<Chars..., OChars...> operator+(UCompileTimeStringView<OChars...> const &) const { return {}; }

   template <size_t From, size_t To> constexpr
   auto substr() const
   {
      if constexpr (From == To)  return UCompileTimeStringView<>();
      else                       return substr<From>(std::make_index_sequence<To - From>{});
   }
};

template <class Char, Char... Chars>
constexpr auto operator""_ctv() // compile time view
{
   return UCompileTimeStringView<static_cast<char>(Chars)...>{};
}
#  endif
#endif
#endif
