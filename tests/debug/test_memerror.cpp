// test_memerror.cpp

#include <ulib/debug/trace.h>
#include <ulib/debug/common.h>
#include <ulib/debug/error_memory.h>

class UInt {
public:
   // Check for memory error
   U_MEMORY_TEST

   // COSTRUTTORI

   UInt()      { a = 0; }
   UInt(int i) { a = i; }

   // ASSEGNAZIONI

   UInt(const UInt& i) : a(i.a)   {           U_MEMORY_TEST_COPY(i) }
   UInt& operator=(const UInt& i) { a = i.a;  U_MEMORY_TEST_COPY(i) return *this; }

   // CONVERSIONI

   operator int() const
      {
      U_CHECK_MEMORY

      return a;
      }

   // OPERATORI

   bool operator< (const UInt& i) const { return (a <  i.a); }
   bool operator==(const UInt& i) const { return (a == i.a); }

protected:
   int a;
};

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d,%p)", argc, argv)

   UInt a(10);

   U_CHECK_MEMORY_OBJECT(&a)

   UInt* b = new UInt(a);

   delete b;

/*
#if defined(__has_feature) && __has_feature(address_sanitizer)
=================================================================
==24665==ERROR: AddressSanitizer: heap-use-after-free on address 0x60200000efb0 at pc 0x40151e bp 0x7fff84092b90 sp 0x7fff84092b80
READ of size 8 at 0x60200000efb0 thread T0
    #0 0x40151d in UMemoryError::invariant() const ../../include/ulib/debug/error_memory.h:42
    #1 0x40151d in UInt::operator int() const /usr/src/ULib-1.2.0/tests/debug/test_memerror.cpp:26
    #2 0x400fe0 in main /usr/src/ULib-1.2.0/tests/debug/test_memerror.cpp:54
    #3 0x7fc7d1f42c44 in __libc_start_main (/lib64/libc.so.6+0x24c44)
    #4 0x401164 (/usr/src/ULib-1.2.0/tests/debug/.libs/test_memerror+0x401164)

0x60200000efb0 is located 0 bytes inside of 16-byte region [0x60200000efb0,0x60200000efc0)
freed by thread T0 here:
    #0 0x7fc7d2633243 in operator delete(void*) (/usr/lib/gcc/x86_64-pc-linux-gnu/4.9.0/libasan.so.1+0x58243)
    #1 0x400fd1 in main /usr/src/ULib-1.2.0/tests/debug/test_memerror.cpp:52
    #2 0x7fc7d1f42c44 in __libc_start_main (/lib64/libc.so.6+0x24c44)

previously allocated by thread T0 here:
    #0 0x7fc7d2632d81 in operator new(unsigned long) (/usr/lib/gcc/x86_64-pc-linux-gnu/4.9.0/libasan.so.1+0x57d81)
    #1 0x400f45 in main /usr/src/ULib-1.2.0/tests/debug/test_memerror.cpp:50
    #2 0x7fc7d1f42c44 in __libc_start_main (/lib64/libc.so.6+0x24c44)

SUMMARY: AddressSanitizer: heap-use-after-free ../../include/ulib/debug/error_memory.h:42 UMemoryError::invariant() const
Shadow bytes around the buggy address:
  0x0c047fff9da0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9db0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9dc0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9dd0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9de0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>0x0c047fff9df0: fa fa fa fa fa fa[fd]fd fa fa 00 07 fa fa fd fd
  0x0c047fff9e00: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9e10: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9e20: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9e30: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c047fff9e40: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Heap right redzone:      fb
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack partial redzone:   f4
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Contiguous container OOB:fc
  ASan internal:           fe
==24665==ABORTING
#else
test_memerror: ERROR ON MEMORY
-------------------------------------
 pid: 10713
 file: test_memerror.cpp
 line: 26
 function: UInt::operator int () const
 assertion: "((memory).invariant())" [this = 0x8194138 _this = (nil) - FMR]
-------------------------------------
#endif
*/

   int c = *b;

/*
test_memerror: ERROR ON MEMORY
-------------------------------------
 pid: 10713
 file: ../../include/ulib/debug/error_memory.h
 line: 30
 function: UMemoryError::~UMemoryError ()
 assertion: "((*this).invariant())" [this = 0xbffff2c0 _this = 0xff - ABW]
-------------------------------------
*/

   *(int*)&a = 0x00ff;

   U_RETURN(0);
}
