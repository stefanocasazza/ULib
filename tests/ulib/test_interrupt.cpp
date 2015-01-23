// test_interrupt.cpp

#include <ulib/base/utility.h>
#include <ulib/utility/interrupt.h>

#ifdef __MINGW32__
#  include <process.h>
#else
#  include <sys/mman.h>
#endif

static RETSIGTYPE handlerForAlarm(int signo)
{
   U_TRACE(5,"handlerForAlarm(%d)", signo)

   UInterrupt::sendSignal(SIGUSR1, getpid());
}

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

#ifndef DEBUG
   UInterrupt::init();
#endif
   UInterrupt::setHandlerForSignal(SIGALRM, (sighandler_t)handlerForAlarm);

   alarm(1);

   UInterrupt::waitForSignal(SIGUSR1);

   /*
   char* ptr = (char*)0x013;
   *ptr = '\0'; // SIGSEGV

   int fd = open("tmp",O_CREAT|O_RDWR,0666);
   write(fd,string_and_size("aaaaaaaaaaaaaaaaaaaaaaaa"));
   ptr = (char*) mmap(NULL, 24, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
   ftruncate(fd, 0);
   close(fd);
   ptr[20] = '\0'; // SIGBUS
   */

// putenv("EXEC_ON_EXIT=/utility/stack_extend.pl");

/*
   char Buffer[] = "is this a violation?";
   U_MEMCPY(&Buffer[5], &Buffer[10], 10U);

#if defined(__has_feature) && __has_feature(address_sanitizer)
=================================================================
==25793==ERROR: AddressSanitizer: memcpy-param-overlap: memory ranges [0x7fff09ee0855,0x7fff09ee085f) and [0x7fff09ee085a, 0x7fff09ee0864) overlap
    #0 0x7f913248d208 (/usr/lib/gcc/x86_64-pc-linux-gnu/4.9.0/libasan.so.1+0x33208)
    #1 0x7f91336bc0f3 in memmove /usr/include/bits/string3.h:57
    #2 0x7f91336bc0f3 in u__memcpy base/utility.c:139
    #3 0x401545 in main /usr/src/ULib-1.2.0/tests/ulib/test_interrupt.cpp:51
    #4 0x7f9131dc1c44 in __libc_start_main (/lib64/libc.so.6+0x24c44)
    #5 0x401794 (/usr/src/ULib-1.2.0/tests/ulib/.libs/test_interrupt+0x401794)

Address 0x7fff09ee0855 is located in stack of thread T0 at offset 37 in frame
    #0 0x40127f in main /usr/src/ULib-1.2.0/tests/ulib/test_interrupt.cpp:21

  This frame has 3 object(s):
    [32, 53) 'Buffer' <== Memory access at offset 37 is inside this variable
    [96, 2148) 'utr'
    [2208, 4260) 'utr'
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
Address 0x7fff09ee085a is located in stack of thread T0 at offset 42 in frame
    #0 0x40127f in main /usr/src/ULib-1.2.0/tests/ulib/test_interrupt.cpp:21

  This frame has 3 object(s):
    [32, 53) 'Buffer' <== Memory access at offset 42 is inside this variable
    [96, 2148) 'utr'
    [2208, 4260) 'utr'
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: memcpy-param-overlap ??:0 ??
==25793==ABORTING
#else
test_interrupt: WARNING: *** Source and Destination OVERLAP in memcpy *** - int main(int, char**)
test_interrupt: WARNING: ::u__memcpy(0x7fff733319f5,0x7fff733319fa,10,"int main(int, char**)") = (nil)
#endif
*/
   U_WARNING("%s", "test for SIGSEGV from user");

   UInterrupt::sendSignal(SIGSEGV, u_pid);

   U_RETURN(0);
}
