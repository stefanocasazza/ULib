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
#include <ulib/net/server/server.h>

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

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
void UApplication::printMemUsage()
{
   U_TRACE_NO_PARAM(0, "UApplication::printMemUsage()")

   if (U_SYSCALL(getenv, "%S", "UMEMUSAGE"))
      {
      uint32_t len;
      char buffer[4096];
      unsigned long vsz, rss;

      u_get_memusage(&vsz, &rss);

      len = u__snprintf(buffer, sizeof(buffer),
                        U_CONSTANT_TO_PARAM("address space usage: %.2f MBytes - "
                        "rss usage: %.2f MBytes\n"
                        "%v\nmax_nfd_ready = %u max_depth = %u again:read = (%u/%u - %u%%) wakeup_for_nothing = %u bepollet_threshold = (%u/10)\n"),
                        UServer_Base::getStats().rep,
                        (double)vsz / (1024.0 * 1024.0),
                        (double)rss / (1024.0 * 1024.0), UNotifier::max_nfd_ready, UServer_Base::max_depth, UServer_Base::nread_again, UServer_Base::nread,
                        (UServer_Base::nread_again * 100) / UServer_Base::nread, UServer_Base::wakeup_for_nothing, UNotifier::bepollet_threshold);

      ostrstream os(buffer + len, sizeof(buffer) - len);

      UMemoryPool::printInfo(os);

      len += os.pcount();

      U_INTERNAL_ASSERT_MINOR(len, sizeof(buffer))

      (void) UFile::writeToTmp(buffer, len, O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("%N.memusage.%P"), 0);
      }
}
#endif

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
