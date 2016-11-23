// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    common.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================
 
#include <ulib/file.h>
#include <ulib/json/value.h>
#include <ulib/application.h>
#include <ulib/utility/interrupt.h>

#ifndef HAVE_POLL_H
#  include <ulib/notifier.h>
#  include <ulib/event/event_time.h>
#endif

#ifdef U_STDCPP_ENABLE
# if defined(HAVE_CXX14) && GCC_VERSION_NUM > 60100 && defined(HAVE_ARCH64)
#  include "./itoa.h"
# endif
#else
U_EXPORT bool __cxa_guard_acquire() { return 1; }
U_EXPORT bool __cxa_guard_release() { return 1; }

U_EXPORT void* operator new(  size_t n)   { return malloc(n); }
U_EXPORT void* operator new[](size_t n)   { return malloc(n); }
U_EXPORT void  operator delete(  void* p) { free(p); }
U_EXPORT void  operator delete[](void* p) { free(p); }
# ifdef __MINGW32__
U_EXPORT void  operator delete(  void* p,      unsigned int) { free(p); }
U_EXPORT void  operator delete[](void* p,      unsigned int) { free(p); }
# else
U_EXPORT void  operator delete(  void* p, long unsigned int) { free(p); }
U_EXPORT void  operator delete[](void* p, long unsigned int) { free(p); }
# endif
#endif

const double u_pow10[309] = { // 1e-0...1e308: 309 * 8 bytes = 2472 bytes
  1,
  1e+1,  1e+2,  1e+3,  1e+4,  1e+5,  1e+6,  1e+7,  1e+8,  1e+9,  1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15, 1e+16, 1e+17, 1e+18, 1e+19, 1e+20,
  1e+21, 1e+22, 1e+23, 1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31, 1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39, 1e+40,
  1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47, 1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55, 1e+56, 1e+57, 1e+58, 1e+59, 1e+60,
  1e+61, 1e+62, 1e+63, 1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71, 1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79, 1e+80,
  1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87, 1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95, 1e+96, 1e+97, 1e+98, 1e+99, 1e+100,
  1e+101,1e+102,1e+103,1e+104,1e+105,1e+106,1e+107,1e+108,1e+109,1e+110,1e+111,1e+112,1e+113,1e+114,1e+115,1e+116,1e+117,1e+118,1e+119,1e+120,
  1e+121,1e+122,1e+123,1e+124,1e+125,1e+126,1e+127,1e+128,1e+129,1e+130,1e+131,1e+132,1e+133,1e+134,1e+135,1e+136,1e+137,1e+138,1e+139,1e+140,
  1e+141,1e+142,1e+143,1e+144,1e+145,1e+146,1e+147,1e+148,1e+149,1e+150,1e+151,1e+152,1e+153,1e+154,1e+155,1e+156,1e+157,1e+158,1e+159,1e+160,
  1e+161,1e+162,1e+163,1e+164,1e+165,1e+166,1e+167,1e+168,1e+169,1e+170,1e+171,1e+172,1e+173,1e+174,1e+175,1e+176,1e+177,1e+178,1e+179,1e+180,
  1e+181,1e+182,1e+183,1e+184,1e+185,1e+186,1e+187,1e+188,1e+189,1e+190,1e+191,1e+192,1e+193,1e+194,1e+195,1e+196,1e+197,1e+198,1e+199,1e+200,
  1e+201,1e+202,1e+203,1e+204,1e+205,1e+206,1e+207,1e+208,1e+209,1e+210,1e+211,1e+212,1e+213,1e+214,1e+215,1e+216,1e+217,1e+218,1e+219,1e+220,
  1e+221,1e+222,1e+223,1e+224,1e+225,1e+226,1e+227,1e+228,1e+229,1e+230,1e+231,1e+232,1e+233,1e+234,1e+235,1e+236,1e+237,1e+238,1e+239,1e+240,
  1e+241,1e+242,1e+243,1e+244,1e+245,1e+246,1e+247,1e+248,1e+249,1e+250,1e+251,1e+252,1e+253,1e+254,1e+255,1e+256,1e+257,1e+258,1e+259,1e+260,
  1e+261,1e+262,1e+263,1e+264,1e+265,1e+266,1e+267,1e+268,1e+269,1e+270,1e+271,1e+272,1e+273,1e+274,1e+275,1e+276,1e+277,1e+278,1e+279,1e+280,
  1e+281,1e+282,1e+283,1e+284,1e+285,1e+286,1e+287,1e+288,1e+289,1e+290,1e+291,1e+292,1e+293,1e+294,1e+295,1e+296,1e+297,1e+298,1e+299,1e+300,
  1e+301,1e+302,1e+303,1e+304,1e+305,1e+306,1e+307,1e+308
};

#if defined(USE_LIBSSL) && OPENSSL_VERSION_NUMBER < 0x10100000L
#  include <openssl/ssl.h>
#  include <openssl/rand.h>
#  include <openssl/conf.h>
#endif

#ifndef HAVE_OLD_IOSTREAM
#  include "./dtoa.h"
#endif

static struct ustringrep u_empty_string_rep_storage = {
# ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL, /* memory_error (_this) */
# endif
# if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   0, /* parent - substring increment reference of source string */
#  ifdef DEBUG
   0, /* child  - substring capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'... */
#  endif
# endif
   0, /* _length */
   0, /* _capacity */
   0, /* references */
  ""  /* str - NB: we need an address (see c_str() or isNullTerminated()) and must be null terminated... */
};

static struct ustring u_empty_string_storage = { &u_empty_string_rep_storage };

uustring    ULib::uustringnull    = { &u_empty_string_storage };
uustringrep ULib::uustringrepnull = { &u_empty_string_rep_storage };

void ULib::init(const char* mempool, char** argv)
{
   u_init_ulib(argv);

   U_TRACE(1, "ULib::init(%S,%p)", mempool, argv)

   // conversion number => string

#ifndef HAVE_OLD_IOSTREAM
   u_dbl2str = dtoa_rapidjson;
#endif
#ifdef DEBUG
   char buffer[32];

   UMemoryPool::obj_class = "";
   UMemoryPool::func_call = __PRETTY_FUNCTION__;

   U_INTERNAL_ASSERT_EQUALS(u_dbl2str(1234567890, buffer)-buffer, 12)
   U_INTERNAL_ASSERT_EQUALS(memcmp(buffer, "1234567890.0", 12), 0)

   U_INTERNAL_ASSERT_EQUALS(u_num2str64(1234567890, buffer)-buffer, 10)
   U_INTERNAL_ASSERT_EQUALS(memcmp(buffer, "1234567890", 10), 0)
#endif

#if defined(HAVE_CXX14) && GCC_VERSION_NUM > 60100 && defined(HAVE_ARCH64)
   u_num2str32 = itoa_fwd;
   u_num2str64 = itoa_fwd;

   U_INTERNAL_ASSERT_EQUALS(u_num2str64(1234567890, buffer)-buffer, 10)
   U_INTERNAL_DUMP("buffer = %.10S", buffer)
   U_INTERNAL_ASSERT_EQUALS(memcmp(buffer, "1234567890", 10), 0)
#endif

   // setting from u_init_ulib(char** argv)

   U_INTERNAL_DUMP("u_progname(%u) = %.*S u_cwd(%u) = %.*S", u_progname_len, u_progname_len, u_progname, u_cwd_len, u_cwd_len, u_cwd)

   // allocation from memory pool

#if defined(ENABLE_MEMPOOL) // check if we want some preallocation for memory pool
   const char* ptr = (mempool ? (UValue::jsonParseFlags = 2, mempool) : U_SYSCALL(getenv, "%S", "UMEMPOOL")); // start from 1... (Ex: 768,768,0,1536,2085,0,0,0,121)

   // coverity[tainted_scalar]
   if (           ptr &&
      u__isdigit(*ptr))
      {
      UMemoryPool::allocateMemoryBlocks(ptr);
      }

   U_INTERNAL_ASSERT_EQUALS(U_BUFFER_SIZE, U_MAX_SIZE_PREALLOCATE * 2)

   ptr      = (char*) UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE));
   u_buffer = (char*) UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE));

   if (ptr < u_buffer) u_buffer = (char*)ptr;

   u_err_buffer = (char*) UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(256));

   U_INTERNAL_DUMP("ptr = %p u_buffer = %p diff = %ld", ptr, u_buffer, ptr - u_buffer)

# ifdef DEBUG
   UMemoryError::pbuffer = (char*) UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE));
# endif
#else
   u_buffer     = (char*) U_SYSCALL(malloc, "%u", U_BUFFER_SIZE);
   u_err_buffer = (char*) U_SYSCALL(malloc, "%u", 256);

# ifdef DEBUG
   UMemoryError::pbuffer = (char*) U_SYSCALL(malloc, "%u", U_MAX_SIZE_PREALLOCATE);
# endif
#endif

   UString::ptrbuf =
   UString::appbuf = (char*)UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(1024));
   UFile::cwd_save = (char*)UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(1024));

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
# ifdef DEBUG
   UMemoryPool::obj_class = UMemoryPool::func_call = 0;
# endif
   UObjectIO::init((char*)UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE)), U_MAX_SIZE_PREALLOCATE);
#endif

   UInterrupt::init();

#ifdef _MSWINDOWS_
   WSADATA wsaData;
   WORD version_requested = MAKEWORD(2, 2); // version_high, version_low
   int err = U_SYSCALL(WSAStartup, "%d.%p", version_requested, &wsaData);

   // Confirm that the WinSock DLL supports 2.2. Note that if the DLL supports versions greater than 2.2 in addition to 2.2,
   // it will still return 2.2 in wVersion since that is the version we requested

   if (err                             ||
       LOBYTE( wsaData.wVersion ) != 2 ||
       HIBYTE( wsaData.wVersion ) != 2)
      {
      WSACleanup();

      // Tell the user that we could not find a usable WinSock DLL

      U_ERROR("Couldn't find useable Winsock DLL. Must be at least 2.2");
      }

# ifdef HAVE_ATEXIT
   (void) U_SYSCALL(atexit, "%p", (vPF)&WSACleanup);
# endif
#endif

#if defined(SOLARIS) && (defined(SPARC) || defined(sparc)) && !defined(HAVE_ARCH64)
   asm("ta 6"); // make this if there are pointer misalligned (because pointers must be always a multiple of 4 (when running 32 bit applications))
#endif

#if defined(DEBUG) && defined(__GNUC__) && defined(U_ENABLE_ALIGNMENT_CHECKING)
# ifdef __i386__
   __asm__("pushf\norl $0x40000,(%esp)\npopf"); // Enable Alignment Checking on x86
# elif defined(__x86_64__)
   __asm__("pushf\norl $0x40000,(%rsp)\npopf"); // Enable Alignment Checking on x86_64
# endif
#endif

   U_INTERNAL_ASSERT_EQUALS(sizeof(UStringRep), sizeof(ustringrep))

#if defined(U_STATIC_ONLY)
   if (UStringRep::string_rep_null == 0)
      {
      UString::string_null        = uustringnull.p2;
      UStringRep::string_rep_null = uustringrepnull.p2;
      }
#endif

   UString::str_allocate(0);

   U_INTERNAL_DUMP("u_is_tty = %b UStringRep::string_rep_null = %p UString::string_null = %p", u_is_tty, UStringRep::string_rep_null, UString::string_null)

   U_INTERNAL_DUMP("sizeof(off_t) = %u SIZEOF_OFF_T = %u", sizeof(off_t), SIZEOF_OFF_T)

   /**
   * NB: there are to many exceptions...
   *
   * #if defined(_LARGEFILE_SOURCE) && !defined(_MSWINDOWS_)
   * U_INTERNAL_ASSERT_EQUALS(sizeof(off_t), SIZEOF_OFF_T)
   * #endif
   */

#if defined(USE_LIBSSL) && OPENSSL_VERSION_NUMBER < 0x10100000L
   // A typical TLS/SSL application will start with the library initialization,
   // will provide readable error messages and will seed the PRNG (Pseudo Random Number Generator).
   // The environment variable OPENSSL_CONFIG can be set to specify the location of the configuration file

   U_SYSCALL_VOID_NO_PARAM(SSL_load_error_strings);
   U_SYSCALL_VOID_NO_PARAM(SSL_library_init);

# ifdef HAVE_OPENSSL_97
   U_SYSCALL_VOID(OPENSSL_config, "%S", 0);
# endif
   U_SYSCALL_VOID_NO_PARAM(OpenSSL_add_all_ciphers);
   U_SYSCALL_VOID_NO_PARAM(OpenSSL_add_all_digests);

   // OpenSSL makes sure that the PRNG state is unique for each thread. On systems that provide "/dev/urandom",
   // the randomness device is used to seed the PRNG transparently. However, on all other systems, the application
   // is responsible for seeding the PRNG by calling RAND_add()

# ifdef _MSWINDOWS_
   U_SYSCALL_VOID(srand, "%ld", u_seed_hash);

   while (RAND_status() == 0) // Seed PRNG only if needed
      {
      // PRNG may need lots of seed data

      int tmp = U_SYSCALL_NO_PARAM(rand);

      RAND_seed(&tmp, sizeof(int));
      }
# endif
#endif

#ifndef HAVE_POLL_H
   U_INTERNAL_ASSERT_EQUALS(UNotifier::time_obj, 0)

   U_NEW_ULIB_OBJECT(UEventTime, UNotifier::time_obj, UEventTime);
#endif
}

void ULib::end()
{
   
#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   UApplication::printMemUsage();
#endif
}
