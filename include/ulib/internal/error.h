// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    error.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_ERROR_H
#define ULIB_ERROR_H 1

struct U_EXPORT UError {

   static void stackDump();

   static vPF callerDataDump;
};

#endif
