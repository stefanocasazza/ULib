// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    bison.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/flex/bison.h>

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UBison::dump(bool _reset) const
{
   UFlexer::dump(false);

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
