/* daemon.c */

#include <ulib/base/base.h>

#include <unistd.h>
#include <stdio.h>

/**
 * This function is for programs wishing to detach themselves from the controlling terminal and run in the background as system daemons
 *
 * Unless the argument nochdir is nonzero, daemon() changes the current working directory to the root ("/").
 * Unless the argument noclose is nonzero, daemon() will redirect standard input, standard output and standard error to /dev/null
 */

extern U_EXPORT int daemon(int nochdir, int noclose);
       U_EXPORT int daemon(int nochdir, int noclose)
{
#ifndef _MSWINDOWS_
   int fd;

   switch (fork())
      {
      case -1: return -1;
      case  0: break;
      default: _exit(0);
      }

   if (setsid() == -1) return (-1);

   if (!nochdir) (void) chdir("/");

   if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1)
      {
      (void)dup2(fd, STDIN_FILENO);
      (void)dup2(fd, STDOUT_FILENO);
      (void)dup2(fd, STDERR_FILENO);

      if (fd > STDERR_FILENO) (void)close(fd);
      }
#endif

   return 0;
}
