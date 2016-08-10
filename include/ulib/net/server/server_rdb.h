// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    server_rdb.h - RDB Server
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_SERVER_RDB_H
#define U_SERVER_RDB_H 1

#include <ulib/net/tcpsocket.h>
#include <ulib/net/server/server.h>

/**
 * @class URDBServer
 *
 * @brief Handles incoming TCP/IP connections from URDBClient
 */

class URDB;

class U_EXPORT URDBServer : public UServer<UTCPSocket> {
public:

            URDBServer(UFileConfig* cfg, bool ignore_case = false);
   virtual ~URDBServer();

   // Open a reliable database

   static bool open(const UString& pathdb, uint32_t log_size = 1024 * 1024);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static URDB* rdb;

   // method VIRTUAL to redefine

   virtual void preallocate()  U_DECL_FINAL;
#ifdef DEBUG
   virtual void  deallocate()  U_DECL_FINAL;
   virtual bool check_memory() U_DECL_FINAL;
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(URDBServer)
};

#endif
