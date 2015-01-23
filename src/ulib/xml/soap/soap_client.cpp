// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_client.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/soap/soap_client.h>

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USOAPClient_Base::dump(bool _reset) const
{
   URPCClient_Base::dump(false);

   *UObjectIO::os << '\n'
                  << "request        (UString             " << (void*)&request << ")\n"
                  << "parser         (USOAPParser         " << (void*)&parser  << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
