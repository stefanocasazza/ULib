// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    error_simulation.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_ERROR_SIMULATION_H
#define ULIB_ERROR_SIMULATION_H 1

#include <ulib/base/base.h>

// CLASS FOR MANAGE SIMULATION OF ERRORS

union uuvararg {
   int         i;
   long        l;
   float       f;
   void*       p;
   double      d;
   long long   ll;
   long double ld;
};

struct U_EXPORT USimulationError {

   static bool           flag_init;
   static char*          file_mem;
   static uint32_t       file_size;
   static union uuvararg var_arg; // buffer for return value (simulation of error)

   static void  init();
   static void* checkForMatch(const char* call_name);
};

#endif
