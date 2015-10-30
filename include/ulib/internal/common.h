// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    common.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_INTERNAL_COMMON_H
#define ULIB_INTERNAL_COMMON_H 1

// Manage file to include

#include <ulib/base/base.h>

#ifdef U_STDCPP_ENABLE
#  include <iostream>
#  ifdef HAVE_STRSTREAM_H
#     include <streambuf.h>
#     include <strstream.h>
#  else
#     include <streambuf>
#     include <ulib/replace/strstream.h>
#  endif
#  ifdef HAVE_OLD_IOSTREAM
#     define U_openmode ios::out
#  else
#     define U_openmode std::ios_base::out
#  endif
#  ifdef __clang__
#     define ios        std::ios
#     define istream    std::istream
#     define ostream    std::ostream
#     define streambuf  std::streambuf
#     define streamsize std::streamsize
#  endif
#else
typedef char istream;
typedef char ostream;
typedef char ostrstream;
typedef char istrstream;

extern U_EXPORT bool __cxa_guard_acquire();
extern U_EXPORT bool __cxa_guard_release();

extern U_EXPORT void* operator new(size_t);
extern U_EXPORT void* operator new[](size_t);
extern U_EXPORT void  operator delete(void*);
extern U_EXPORT void  operator delete[](void*);
#endif

// C++11 keywords and expressions

#ifdef U_COMPILER_NULLPTR
# define U_NULLPTR nullptr
#else
# define U_NULLPTR 0
#endif

#ifdef U_COMPILER_DEFAULT_MEMBERS
#  define U_DECL_EQ_DEFAULT = default
#else
#  define U_DECL_EQ_DEFAULT
#endif

#ifdef U_COMPILER_CONSTEXPR
# define U_DECL_CONSTEXPR constexpr
#else
# define U_DECL_CONSTEXPR
#endif

#ifdef U_COMPILER_EXPLICIT_OVERRIDES
# define U_DECL_FINAL final
# define U_DECL_OVERRIDE override
#else
# ifndef U_DECL_FINAL
# define U_DECL_FINAL
# endif
# ifndef U_DECL_OVERRIDE
# define U_DECL_OVERRIDE
# endif
#endif

#ifdef U_COMPILER_NOEXCEPT
# define U_DECL_NOEXCEPT noexcept
# define U_DECL_NOEXCEPT_EXPR(x) noexcept(x)
# ifdef U_DECL_NOTHROW
# undef U_DECL_NOTHROW // override with C++11 noexcept if available
# endif
#else
# define U_DECL_NOEXCEPT
# define U_DECL_NOEXCEPT_EXPR(x)
#endif
#ifndef U_DECL_NOTHROW
#define U_DECL_NOTHROW U_DECL_NOEXCEPT
#endif

#include <ulib/debug/macro.h>

#ifdef DEBUG
#  include <ulib/debug/trace.h>
#  include <ulib/debug/common.h>
#  include <ulib/debug/error_memory.h>
#  ifdef U_STDCPP_ENABLE
#     include <ulib/debug/objectDB.h>
#  endif
#endif

#include <ulib/internal/objectIO.h>
#include <ulib/internal/memory_pool.h>
#include <ulib/internal/macro.h>

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
   STR_ALLOCATE_INDEX_SOAP         = 14,
   STR_ALLOCATE_INDEX_IMAP         = STR_ALLOCATE_INDEX_SOAP+14,
   STR_ALLOCATE_INDEX_SSI          = STR_ALLOCATE_INDEX_IMAP+4,
   STR_ALLOCATE_INDEX_NOCAT        = STR_ALLOCATE_INDEX_SSI+2,
   STR_ALLOCATE_INDEX_HTTP         = STR_ALLOCATE_INDEX_NOCAT+2,
   STR_ALLOCATE_INDEX_QUERY_PARSER = STR_ALLOCATE_INDEX_HTTP+10,
   STR_ALLOCATE_INDEX_ORM          = STR_ALLOCATE_INDEX_QUERY_PARSER+5,
   STR_ALLOCATE_INDEX_HTTP2        = STR_ALLOCATE_INDEX_ORM+15
};

// json value representation

union anyvalue {
   bool bool_;
            char char_;
   unsigned char uchar_;
            short short_;
   unsigned short ushort_;
            int int_;
   unsigned int uint_;
            long long_;
   unsigned long ulong_;
            long long llong_;
   unsigned long long ullong_;
   float float_;
   void* ptr_;
        double real_;
   long double lreal_;
};

struct null {}; // Special type to bind a NULL value to column using operator,() - syntactic sugar

// Type of the value held by a UValue object

typedef enum ValueType {
      NULL_VALUE =  0, // null value
   BOOLEAN_VALUE =  1, // bool value
      CHAR_VALUE =  2, //   signed char value
     UCHAR_VALUE =  3, // unsigned char value
     SHORT_VALUE =  4, //   signed short integer value
    USHORT_VALUE =  5, // unsigned short integer value
       INT_VALUE =  6, //   signed integer value
      UINT_VALUE =  7, // unsigned integer value
      LONG_VALUE =  8, //   signed long value
     ULONG_VALUE =  9, // unsigned long value
     LLONG_VALUE = 10, //   signed long long value
    ULLONG_VALUE = 11, // unsigned long long value
     FLOAT_VALUE = 12, // float value
      REAL_VALUE = 13, // double value
     LREAL_VALUE = 14, // long double value
    STRING_VALUE = 15, // UTF-8 string value
     ARRAY_VALUE = 16, // array value (ordered list)
    OBJECT_VALUE = 17, // object value (collection of name/value pairs)
    NUMBER_VALUE = 18  // generic number value (may be -ve) int or float
} ValueType;

using namespace std;

// Init library

U_EXPORT void ULib_init();
#ifdef USE_LIBSSL
U_EXPORT void ULib_init_openssl();
#endif

#define U_ULIB_INIT(argv) U_SET_LOCATION_INFO, u_init_ulib(argv), ULib_init()

#endif
