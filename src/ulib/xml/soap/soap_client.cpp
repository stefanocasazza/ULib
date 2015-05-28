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

bool USOAPClient_Base::readResponse()
{
   U_TRACE(0, "USOAPClient_Base::readResponse()")

   if (UClient_Base::readHTTPResponse()) U_RETURN(true);

   U_RETURN(false);
}

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
