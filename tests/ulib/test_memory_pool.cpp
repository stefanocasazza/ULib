// test_memory_pool.cpp

#include <ulib/debug/crono.h>
#include <ulib/string.h>
#include <ulib/utility/interrupt.h>
#include <sys/time.h>

//#define PRINT_SIZE

#ifdef PRINT_SIZE 
#  include <ulib/all.h>

static void print_size()
{
   U_TRACE(5, "print_size()")

   U_PRINT_SIZEOF(UApplication);
// U_PRINT_SIZEOF(UBison);
   U_PRINT_SIZEOF(UCDB);
// U_PRINT_SIZEOF(UCURL);
   U_PRINT_SIZEOF(UCache);
   U_PRINT_SIZEOF(UCertificate);
// U_PRINT_SIZEOF(UCgiInput);
// U_PRINT_SIZEOF(UClassSlot);
   U_PRINT_SIZEOF(UClientImage<UTCPSocket>);
   U_PRINT_SIZEOF(UCommand);
// U_PRINT_SIZEOF(UConnection);
// U_PRINT_SIZEOF(UConstMethodSlot);
#ifdef HAVE_CURL
   U_PRINT_SIZEOF(UCrl);
#endif
   U_PRINT_SIZEOF(UTimeDate);
   U_PRINT_SIZEOF(UDialog);
// U_PRINT_SIZEOF(UFCgi);
   U_PRINT_SIZEOF(UFile);
   U_PRINT_SIZEOF(UFileConfig);
// U_PRINT_SIZEOF(UFlexer);
   U_PRINT_SIZEOF(UFtpClient);
// U_PRINT_SIZEOF(UFuncSlot);
   U_PRINT_SIZEOF(UHashMap<UString>);
   U_PRINT_SIZEOF(UHttpClient<UTCPSocket>);
   U_PRINT_SIZEOF(UIPAddress);
#ifdef HAVE_LDAP
   U_PRINT_SIZEOF(ULDAP);
   U_PRINT_SIZEOF(ULDAPEntry);
#endif
   U_PRINT_SIZEOF(ULock);
   U_PRINT_SIZEOF(ULog);
#ifdef USE_LIBMAGIC
   U_PRINT_SIZEOF(UMagic);
#endif
// U_PRINT_SIZEOF(UMethodSlot);
   U_PRINT_SIZEOF(UHTTP::UFileCacheData);
   U_PRINT_SIZEOF(UMimeEntity);
   U_PRINT_SIZEOF(UMimeHeader);
   U_PRINT_SIZEOF(UMimeMessage);
   U_PRINT_SIZEOF(UMimeMultipart);
   U_PRINT_SIZEOF(UMimeMultipartMsg);
   U_PRINT_SIZEOF(UMimePKCS7);
   U_PRINT_SIZEOF(UNotifier);
   U_PRINT_SIZEOF(UOptions);
#ifdef USE_LIBPCRE
   U_PRINT_SIZEOF(UPCRE);
#endif
   U_PRINT_SIZEOF(UPlugIn<void*>);
   U_PRINT_SIZEOF(UProcess);
   U_PRINT_SIZEOF(UQueryNode);
   U_PRINT_SIZEOF(UQueryParser);
   U_PRINT_SIZEOF(URDB);
   U_PRINT_SIZEOF(URDBClient<UTCPSocket>);
   U_PRINT_SIZEOF(URDBServer);
// U_PRINT_SIZEOF(URUBY);
   U_PRINT_SIZEOF(USOAPClient<UTCPSocket>);
   U_PRINT_SIZEOF(USOAPEncoder);
   U_PRINT_SIZEOF(USOAPFault);
   U_PRINT_SIZEOF(USOAPGenericMethod);
   U_PRINT_SIZEOF(USOAPObject);
   U_PRINT_SIZEOF(USOAPParser);
#ifdef HAVE_SSH
   U_PRINT_SIZEOF(USSHSocket);
#endif
#ifdef USE_LIBSSL
   U_PRINT_SIZEOF(UCrl);
   U_PRINT_SIZEOF(UPKCS7);
   U_PRINT_SIZEOF(UPKCS10);
   U_PRINT_SIZEOF(USSLSocket);
#endif
   U_PRINT_SIZEOF(USemaphore);
   U_PRINT_SIZEOF(UServer<UTCPSocket>);
   U_PRINT_SIZEOF(USmtpClient);
   U_PRINT_SIZEOF(USocket);
   U_PRINT_SIZEOF(UString);
   U_PRINT_SIZEOF(UStringRep);
   U_PRINT_SIZEOF(UTCPSocket);
// U_PRINT_SIZEOF(UTimeStamp);
   U_PRINT_SIZEOF(UTimeVal);
   U_PRINT_SIZEOF(UTimer);
   U_PRINT_SIZEOF(UTokenizer);
#ifdef DEBUG
   U_PRINT_SIZEOF(UTrace);
#endif
   U_PRINT_SIZEOF(UTree<UString>);
   U_PRINT_SIZEOF(UUDPSocket);
   U_PRINT_SIZEOF(UVector<UString>);
#ifdef HAVE_EXPAT
   U_PRINT_SIZEOF(UXMLAttribute);
   U_PRINT_SIZEOF(UXMLElement);
   U_PRINT_SIZEOF(UXMLParser);
#endif
#ifdef USE_LIBZ
   U_PRINT_SIZEOF(UZIP);
#endif
   U_PRINT_SIZEOF(Url);
}
#endif

static void check_size()
{
   U_TRACE(5, "check_size()")

   /*
   uint32_t stack_index;

   for (uint32_t sz = 1; sz <= U_MAX_SIZE_PREALLOCATE; ++sz)
      {
      stack_index = U_SIZE_TO_STACK_INDEX(sz);

      printf("%4u %2u\n", sz, stack_index);
      }
   */

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_0 - 1) ==  0 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_0 - 0) ==  0 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_0 + 1) ==  1 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_1 - 1) ==  1 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_1 - 0) ==  1 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_1 + 1) ==  2 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_2 - 1) ==  2 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_2 - 0) ==  2 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_2 + 1) ==  3 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_3 - 1) ==  3 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_3 - 0) ==  3 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_3 + 1) ==  4 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_4 - 1) ==  4 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_4 - 0) ==  4 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_4 + 1) ==  5 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_5 - 1) ==  5 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_5 - 0) ==  5 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_5 + 1) ==  6 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_6 - 1) ==  6 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_6 - 0) ==  6 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_6 + 1) ==  7 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_7 - 1) ==  7 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_7 - 0) ==  7 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_7 + 1) ==  8 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_8 - 1) ==  8 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_8 - 0) ==  8 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_8 + 1) ==  9 )

   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_9 - 1) ==  9 )
   U_ASSERT( U_SIZE_TO_STACK_INDEX(U_STACK_TYPE_9 - 0) ==  9 )
}

static struct itimerval timeval = { { 0, 2000 }, { 0, 2000 } };

static RETSIGTYPE
manage_alarm(int signo)
{
   U_TRACE(5,"[SIGALRM} manage_alarm(%d)",signo)

   static char* ptr;
   static size_t sz = 255;

   if (ptr) delete[] ptr;

   ptr = new char[sz];

   (void) U_SYSCALL(setitimer,"%d,%p,%p",ITIMER_REAL,&timeval,0);
}

int
U_EXPORT main(int argc, char** argv)
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   if (argc > 3)
      {
      UInterrupt::setHandlerForSignal(SIGALRM, (sighandler_t)&manage_alarm);

      (void) U_SYSCALL(setitimer,"%d,%p,%p",ITIMER_REAL,&timeval,0);
      }

   check_size();

#  define U_NUM_ENTRY_MEM_BLOCK 32

   int i, j, k;
   int n = U_NUM_ENTRY_MEM_BLOCK * (argc > 1 ? u_atoi(argv[1]) : 1);

   char* vptr[n];
   UString* obj[n];

   UCrono crono;

   crono.start();

   for (i = 0; i < 10; ++i)
      {
      for (j = 0; j <= U_NUM_STACK_TYPE; ++j)
         {
         for (k = 0; k < n; ++k)
            {
            vptr[k] = new char[3 << j];

            U_NEW_STRING(obj[k], UString(U_CONSTANT_TO_PARAM("allocated")));
            }

         for (k = 0; k < n; ++k)
            {
            delete[] vptr[k];

            delete obj[k];
            }
         }
      }

   crono.stop();

   if (argc > 2) printf("Time Consumed with U_NUM_ENTRY_MEM_BLOCK(%d) = %ld ms\n", n, crono.getTimeElapsed());

#ifdef DEBUG
   UMemoryPool::printInfo(cout);
#endif

   // RISULTATI

#ifdef NDEBUG
#  ifdef ENABLE_MEMPOOL
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(32)   =     3 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(320)  =    44 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(3200) =   670 ms
#  else
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(32)   =     6 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(320)  =   260 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(3200) =  3680 ms
#  endif
#else
#  ifdef ENABLE_MEMPOOL
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(32)   =    60 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(320)  =  1463 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(3200) = 15500 ms
#  else
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(32)   =    51 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(320)  =   760 ms
   // Time Consumed with U_NUM_ENTRY_MEM_BLOCK(3200) =  8400 ms
#  endif
#endif

#ifdef PRINT_SIZE 
   print_size();
#endif
}
