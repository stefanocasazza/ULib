/* apex_memmove.h */

#ifndef __APEX_MEMMOVE_H__
#define __APEX_MEMMOVE_H__

#include <ulib/base/base.h>

#if defined(__MINGW32__) || !defined(HAVE_ARCH64)
extern U_EXPORT pvPFpvpvs apex_memcpy;
extern U_EXPORT pvPFpvpvs apex_memmove;
#else
/**
 * apex_memmove written by Trevor Herselman in 2014
 *
 * FORCE `CDECL` calling convention on 32-bit builds on our function pointers, because we need it to match the original `std::memmove` definition;
 * in-case the user specified a different default function calling convention! (I specified __fastcall as my default calling convention and got errors!
 * So I needed to add this!)
 */

U_EXPORT void apex_memmove_dispatcher(void);

#if !defined(__x86_64__) && !defined(_M_X64) && (defined(__i386) || defined(_M_IX86)) && (defined(_MSC_VER) || defined(__GNUC__))
   #if defined(_MSC_VER)
      #define APEXCALL __cdecl /* 32-bit on Visual Studio */
   #else
      #define APEXCALL __attribute__((__cdecl__))  /* 32-bit on GCC / LLVM (Clang) */
   #endif
#else
   #define APEXCALL  /* 64-bit - __fastcall is default on 64-bit! */
#endif

#include <stddef.h>  /* ANSI/ISO C  -  for `size_t` */

extern U_EXPORT void* (APEXCALL *apex_memcpy)( void *dst, const void *src, size_t size);
extern U_EXPORT void* (APEXCALL *apex_memmove)(void *dst, const void *src, size_t size);
#endif

#endif /* __APEX_MEMMOVE_H__ */
