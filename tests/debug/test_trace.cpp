// test_trace.cpp

#include <ulib/debug/trace.h>
#include <ulib/debug/common.h>

static int routine2(int a, int b) // masked
{
   U_TRACE(5, "routine2(%d,%d)", a, b)

   int c = a * b;

   U_RETURN(c);
}

static int routine1(int a, int b)
{
   U_TRACE(5+256, "routine1(%d,%d)", a, b)

   int c = routine2(a,b);

   U_RETURN(c);
}

static int fd;
static char buffer[4096];

static int test_stat(const char* file)
{
   U_TRACE(5, "test_stat(%S)", file)

   struct stat buf;
   int result = U_SYSCALL(stat, "%S,%p", file, &buf);

   // for display bandwith

   (void) U_SYSCALL(read,  "%d,%S,%d", fd, buffer, sizeof(buffer));
   (void) U_SYSCALL(write, "%d,%S,%d", fd, buffer, sizeof(buffer));

   U_RETURN(result);
}

static RETSIGTYPE manage_sigpipe(int signo)
{
   U_TRACE(5, "manage_sigpipe(%d)", signo)

   (void) U_SYSCALL(open, "%S,%d,%d",
      "/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp"
      "/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp"
      "/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/tmp/prova", O_RDWR, 0666);

   fd = U_SYSCALL(open, "%S,%d,%d", "tmp/prova", O_RDWR | O_CREAT, 0666);
}

static struct itimerval timeval = { { 0, 5000 }, { 0, 5000 } };

static RETSIGTYPE manage_alarm(int signo)
{
   U_TRACE(5, "[SIGALRM} manage_alarm(%d)", signo)

   int result = U_SYSCALL(setitimer, "%d,%p,%p", ITIMER_REAL, &timeval, 0);

   U_INTERNAL_DUMP("result setitimer() = %d", result)
}

int U_EXPORT main(int argc, char* argv[])
{
   if (argc >= 2) (void) putenv((char*)"USIMERR=error.sim");

   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   if (argc < 2)
      {
      raise(SIGUSR2); // trace on/off

      u_trace_handlerSignal();
      }

   int c = routine1(2, 3);

   U_INTERNAL_DUMP("c = %d", c)

   (void) U_SYSCALL(signal, "%d,%p", SIGILL, (sighandler_t)&manage_sigpipe);

   int result = U_SYSCALL(raise, "%d", SIGILL);

   U_INTERNAL_DUMP("result raise() = %d", result)

   // test for simulation error

   if (argc >= 2)
      {
      int iteration = 5;

      if (argc == 3)
         {
         iteration = atoi(argv[2]);

         (void) U_SYSCALL(signal, "%d,%p", SIGALRM, (sighandler_t)&manage_alarm);

         (void) U_SYSCALL(setitimer, "%d,%p,%p", ITIMER_REAL, &timeval, 0);
         }

      for (int i = 0; i < iteration; ++i)
         {
         if (argc == 3)
            {
            result = test_stat("error.sim");

            U_INTERNAL_DUMP("test_stat() = %d", result)
            }
         else
            {
            U_DUMP("test_stat() = %d", test_stat("error.sim"))
            }
         }

      U_DUMP("malloc() = %p", U_SYSCALL(malloc, "%u", 1000))
      }

   U_RETURN(0);
}
