// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_envelope.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc_envelope.h>

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCEnvelope::dump(bool reset) const
{
   *UObjectIO::os << "mustUnderstand                              " << (void*)mustUnderstand << '\n'
                  << "arg                (UVector                 " << (void*)arg            << ")\n"
                  << "nsName             (UString                 " << (void*)&nsName        << ")\n"
                  << "methodName         (UString                 " << (void*)&methodName    << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
