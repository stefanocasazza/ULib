// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    server_rdb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/rdb.h>
#include <ulib/net/rpc/rpc.h>
#include <ulib/net/server/server_rdb.h>
#include <ulib/net/server/client_image_rdb.h>

URDB* URDBServer::rdb;

URDBServer::URDBServer(UFileConfig* cfg, bool ignore_case) : UServer<UTCPSocket>(cfg)
{
   U_TRACE_REGISTER_OBJECT(0, URDBServer, "%p,%b", cfg, ignore_case)

   U_NEW(URDB, rdb, URDB(ignore_case));

   URPC::allocate();
}

URDBServer::~URDBServer()
{
   U_TRACE_UNREGISTER_OBJECT(0, URDBServer)

   delete rdb;
   delete URPC::rpc_info;
}

// Open a reliable database

bool URDBServer::open(const UString& pathdb, uint32_t log_size)
{
   U_TRACE(0, "URDBServer::open(%V,%u)", pathdb.rep, log_size)

   URDBClientImage::rdb = rdb;

   if (rdb->open(pathdb, log_size)) U_RETURN(true);

   U_RETURN(false);
}

// method VIRTUAL to redefine

void URDBServer::preallocate()
{
   U_TRACE_NO_PARAM(0+256, "URDBServer::preallocate()")

   UServer_Base::vClientImage = new URDBClientImage[UNotifier::max_connection];
}

#ifdef DEBUG
void URDBServer::deallocate()
{
   U_TRACE_NO_PARAM(0+256, "URDBServer::deallocate()")

   // NB: array are not pointers (virtual table can shift the address of this)...

   delete[] (URDBClientImage*)UServer_Base::vClientImage;
}

bool URDBServer::check_memory() { return u_check_memory_vector<URDBClientImage>((URDBClientImage*)UServer_Base::vClientImage, UNotifier::max_connection); }
#endif

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URDBServer::dump(bool reset) const
{
   UServer<UTCPSocket>::dump(false);

   *UObjectIO::os << '\n'
                  << "rdb           (URDB       " << (void*)rdb << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
