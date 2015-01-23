// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    macro.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_INTERNAL_MACRO_H
#define ULIB_INTERNAL_MACRO_H 1

#define U_SINGLE_READ     0U
#define U_TIMEOUT_MS     (30L * 1000L) // 30 second connection/read timeout
#define U_SSL_TIMEOUT_MS (10L * 1000L) // 10 second handshake       timeout

// NB: for avoid mis-aligned we use 4 bytes...
#define U_LZOP_COMPRESS "\x89LZO" // "\211LZO" "\x89\x4c\x5a\x4f"

#define U_PATH_MAX (1024U - (1 + sizeof(ustringrep)))
// -------------------------------------------------------------------------------------------------------------------
// NB: the value must be a stack type boundary, see UStringRep::checkIfMReserve()...
// -------------------------------------------------------------------------------------------------------------------
#define U_CAPACITY (U_MAX_SIZE_PREALLOCATE - (1 + sizeof(ustringrep))) // UStringRep::max_size(U_MAX_SIZE_PREALLOCATE)
// -------------------------------------------------------------------------------------------------------------------

#define U_STRING_MAX_SIZE (((U_NOT_FOUND-sizeof(ustringrep))/sizeof(char))-4096)

// NB: Optimization if it is enough a resolution of one second
#undef U_gettimeofday

#ifdef ENABLE_THREAD
#  define U_gettimeofday { if (u_pthread_time == 0) (void) U_SYSCALL(gettimeofday, "%p,%p", u_now, 0); }
#else
#  define U_gettimeofday                            (void) U_SYSCALL(gettimeofday, "%p,%p", u_now, 0);
#endif

#endif
