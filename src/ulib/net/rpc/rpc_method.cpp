// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_method.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc_method.h>

URPCFault*   URPCMethod::pFault;
URPCEncoder* URPCMethod::encoder;

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCMethod::dump(bool reset) const
{
   *UObjectIO::os << "pFault      (URPCFault   " << (void*)pFault        << ")\n"
                  << "encoder     (URPCEncoder " << (void*)encoder       << ")\n"
                  << "ns          (UString     " << (void*)&ns           << ")\n"
                  << "method_name (UString     " << (void*)&method_name  << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
