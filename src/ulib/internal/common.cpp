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
#include <ulib/utility/interrupt.h>

#ifndef HAVE_POLL_H
#  include <ulib/notifier.h>
#  include <ulib/event/event_time.h>
#endif

#ifdef U_STDCPP_ENABLE
#  if defined(HAVE_CXX14) && GCC_VERSION_NUM > 60100
#     include "./itoa.cpp"
static uint32_t itoa32(uint32_t num, char* restrict cp) { return itoa_fwd(num, cp) - cp; }
static uint32_t itoa64(uint64_t num, char* restrict cp) { return itoa_fwd(num, cp) - cp; }
#  endif
#else
U_EXPORT bool __cxa_guard_acquire() { return 1; }
U_EXPORT bool __cxa_guard_release() { return 1; }

U_EXPORT void* operator new(  size_t n)   { return malloc(n); }
U_EXPORT void* operator new[](size_t n)   { return malloc(n); }
U_EXPORT void  operator delete(  void* p) { free(p); }
U_EXPORT void  operator delete[](void* p) { free(p); }

#  ifdef __MINGW32__
U_EXPORT void  operator delete(  void* p,      unsigned int) { free(p); }
U_EXPORT void  operator delete[](void* p,      unsigned int) { free(p); }
#  else
U_EXPORT void  operator delete(  void* p, long unsigned int) { free(p); }
U_EXPORT void  operator delete[](void* p, long unsigned int) { free(p); }
#  endif
#endif

#ifdef USE_LIBSSL
#  include <openssl/ssl.h>
#  include <openssl/rand.h>
#  include <openssl/conf.h>

void ULib_init_openssl()
{
   U_TRACE_NO_PARAM(1, "ULib_init_openssl()")

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
   U_SYSCALL_VOID(srand, "%ld", u_start_time); // seed with time

   while (RAND_status() == 0) // Seed PRNG only if needed
      {
      // PRNG may need lots of seed data

      int tmp = U_SYSCALL_NO_PARAM(rand);

      RAND_seed(&tmp, sizeof(int));
      }
# endif
}
#endif

void ULib_init()
{
   U_TRACE_NO_PARAM(1, "ULib_init()")

#if defined(HAVE_CXX14) && GCC_VERSION_NUM > 60100
   u_num2str32 = itoa32;
   u_num2str64 = itoa64;
#endif

#ifdef DEBUG
   static bool init;

   U_INTERNAL_ASSERT_EQUALS(init, false)

   init = true;

   UMemoryPool::obj_class = "";
   UMemoryPool::func_call = __PRETTY_FUNCTION__;
#endif

   // setting from u_init_ulib(char** argv)

   U_INTERNAL_DUMP("u_progname(%u) = %.*S u_cwd(%u) = %.*S", u_progname_len, u_progname_len, u_progname, u_cwd_len, u_cwd_len, u_cwd)

   // allocation from memory pool

#if defined(ENABLE_MEMPOOL) // check if we want some preallocation for memory pool
   char* ptr = U_SYSCALL(getenv, "%S", "UMEMPOOL"); // start from 1... (Ex: 768,768,0,1536,2085,0,0,0,121)

   // coverity[tainted_scalar]
   if (           ptr &&
      u__isdigit(*ptr))
      {
      UMemoryPool::allocateMemoryBlocks(ptr);
      }

   U_INTERNAL_ASSERT_EQUALS(U_BUFFER_SIZE, U_MAX_SIZE_PREALLOCATE * 2)

   ptr      = (char*) UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE));
   u_buffer = (char*) UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_MAX_SIZE_PREALLOCATE));

   if (ptr < u_buffer) u_buffer = ptr;

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

   U_INTERNAL_DUMP("u_is_tty = %b UStringRep::string_rep_null = %p UString::string_null = %p", u_is_tty, UStringRep::string_rep_null, UString::string_null)

   U_INTERNAL_DUMP("sizeof(off_t) = %u SIZEOF_OFF_T = %u", sizeof(off_t), SIZEOF_OFF_T)

   U_INTERNAL_DUMP("u_dosmatch = %p u_dosmatch_with_OR = %p u_pfn_match = %p u_pfn_flags = %u", u_dosmatch, u_dosmatch_with_OR, u_pfn_match, u_pfn_flags)

/**
 * NB: there are to many exceptions...
 *
 * #if defined(_LARGEFILE_SOURCE) && !defined(_MSWINDOWS_)
 * U_INTERNAL_ASSERT_EQUALS(sizeof(off_t), SIZEOF_OFF_T)
 * #endif
 */

   UString::str_allocate(0);

#ifdef USE_LIBSSL
   ULib_init_openssl();
#endif

#ifndef HAVE_POLL_H
   U_INTERNAL_ASSERT_EQUALS(UNotifier::time_obj, 0)

   U_NEW_ULIB_OBJECT(UEventTime, UNotifier::time_obj, UEventTime);
#endif
}
