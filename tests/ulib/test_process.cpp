// test_process.cpp

#include <ulib/timeval.h>
#include <ulib/process.h>
#include <ulib/notifier.h>

int
U_EXPORT main (int argc, char* argv[], char* _environ[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UProcess x;
   int exit_value;
   const char* info;
   char buffer[128];

   if (x.fork())
      {
      if (x.parent())
         {
         x.wait();

         exit_value = x.exitValue();
         info       = x.exitInfo(buffer);

         U_INTERNAL_ASSERT_EQUALS( exit_value, -1536 )
         U_INTERNAL_ASSERT_EQUALS( memcmp(info, U_CONSTANT_TO_PARAM("Signal SIGABRT (6, Abort")), 0 )
         }
      else
         {
         U_ABORT("abort program for testing...", 0);
         }
      }
}
