// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client_image_rdb.h - Handles accepted TCP/IP connections from RDB server
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_CLIENT_IMAGE_RDB_H
#define U_CLIENT_IMAGE_RDB_H 1

#include <ulib/net/tcpsocket.h>
#include <ulib/net/server/client_image.h>

/**
 * @class URDBClientImage
 *
 * @brief Handles accepted TCP/IP connections from URDBServer
 */

class URDB;

class U_EXPORT URDBClientImage : public UClientImage<UTCPSocket> {
public:

   URDBClientImage() : UClientImage<UTCPSocket>()
      {
      U_TRACE_REGISTER_OBJECT(0, URDBClientImage, "", 0)
      }

   virtual ~URDBClientImage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URDBClientImage)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static URDB* rdb;

   // define method VIRTUAL of class UEventFd

   virtual int handlerRead() U_DECL_FINAL;

private:
   U_DISALLOW_COPY_AND_ASSIGN(URDBClientImage)

   friend class URDBServer;
};

#endif
