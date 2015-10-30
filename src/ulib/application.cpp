// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    application.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/application.h>
#include <ulib/utility/services.h>

int      UApplication::exit_value;
bool     UApplication::is_options;
uint32_t UApplication::num_args;

UApplication::UApplication() : opt(126)
{
   U_TRACE_NO_PARAM(0, "UApplication::UApplication()")
}

UApplication::~UApplication()
{
   U_TRACE_NO_PARAM(0+256, "UApplication::~UApplication()")

   // AT EXIT

   U_INTERNAL_DUMP("u_fns_index = %d", u_fns_index)

#ifdef DEBUG
   for (int i = 0; i < u_fns_index; ++i) { U_INTERNAL_DUMP("u_fns[%2u] = %p", i, u_fns[i]) }

#  ifdef USE_LIBSSL
   if (UServices::CApath) 
      {
      delete UServices::CApath;
             UServices::CApath = 0;
      }
#  endif
#endif
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UApplication::dump(bool reset) const
{
   *UObjectIO::os << "num_args                       " << num_args    << '\n'
                  << "exit_value                     " << exit_value  << '\n'
                  << "is_options                     " << is_options  << '\n'
                  << "str (UString                   " << (void*)&str << ")\n"
                  << "opt (UOptions                  " << (void*)&opt << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
