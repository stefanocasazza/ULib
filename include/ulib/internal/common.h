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

// Manage which file headers to include

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
#  if defined(__clang__)
#     define istream    std::istream
#     define ostream    std::ostream
#     define streambuf  std::streambuf
#     define streamsize std::streamsize
#     if !defined(__OSX__)
#        define ios     std::ios
#     endif
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

#include <ulib/internal/macro.h>
#include <ulib/internal/objectIO.h>
#include <ulib/internal/memory_pool.h>

struct null {}; // Special type to bind a NULL value to column using operator,() - syntactic sugar

using namespace std;

class UString;
class UStringRep;

union uustring    { ustring*    p1; UString*    p2; };
union uustringrep { ustringrep* p1; UStringRep* p2; };

class U_EXPORT ULib {
public:
    ULib(const char* mempool) { init(U_NULLPTR, mempool); }
   ~ULib()                    { end(); }

   static void end();
   static void init(char** argv, const char* mempool = U_NULLPTR);

   private:
   static uustring uustringnull;
   static uustring uustringubuffer;
   static uustringrep uustringrepnull;

   friend class UString;
   friend class UStringRep;

   U_DISALLOW_COPY_AND_ASSIGN(ULib)
};

extern U_EXPORT const double u_pow10[309]; /* 1e-0...1e308: 309 * 8 bytes = 2472 bytes */

#ifndef HAVE_CXX11
static inline           uint16_t u_dd(uint8_t i) { return U_MULTICHAR_CONSTANT16(u_ctn2s[i], u_ctn2s[i+1]); }
#else
static inline constexpr uint16_t u_dd(uint8_t i)
{
   return U_MULTICHAR_CONSTANT16('0' + ((i%2)     ? ((i/2)%10)     : ((i/2)/10)),
                                 '0' + (((i+1)%2) ? (((i+1)/2)%10) : (((i+1)/2)/10)));
}

static_assert( u_dd(0) == U_MULTICHAR_CONSTANT16('0','0'), "should be U_MULTICHAR_CONSTANT16('0','0')" );
static_assert( u_dd(1) == U_MULTICHAR_CONSTANT16('0','0'), "should be U_MULTICHAR_CONSTANT16('0','0')" );
static_assert( u_dd(2) == U_MULTICHAR_CONSTANT16('0','1'), "should be U_MULTICHAR_CONSTANT16('0','1')" );
static_assert( u_dd(3) == U_MULTICHAR_CONSTANT16('1','0'), "should be U_MULTICHAR_CONSTANT16('1','0')" );
static_assert( u_dd(4) == U_MULTICHAR_CONSTANT16('0','2'), "should be U_MULTICHAR_CONSTANT16('0','2')" );

template<typename T> static constexpr T pow10(size_t x) { return x ? 10*pow10<T>(x-1) : 1; }

# ifndef __INTEL_COMPILER 
static_assert( pow10<double>(29)   == 1e+29,                   "should be 1e+29" ); // NB: fail for exponent >= 30
# endif
static_assert( pow10<uint64_t>(19) == 10000000000000000000ULL, "should be 1e+19" );

static_assert( U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE) == 9, "should be 9" );
#endif

// Init library

#define U_ULIB_INIT(argv) U_SET_LOCATION_INFO, ULib::init(argv, U_NULLPTR)

#endif
