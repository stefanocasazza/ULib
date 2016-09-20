// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ssl_session.h - ssl session utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SSL_SESSION_H
#define ULIB_SSL_SESSION_H 1

#include <ulib/ssl/net/sslsocket.h>
#include <ulib/utility/data_session.h>

/**
 * SSL Session Information
 *
 * This class contains data about an SSL session
 */

class UHTTP;
class UHttpPlugIn;

class U_EXPORT USSLSession : public UDataStorage {
public:

   USSLSession()
      {
      U_TRACE_REGISTER_OBJECT(0, USSLSession, "", 0)
      }

   virtual ~USSLSession() U_DECL_FINAL
      {
      U_TRACE_UNREGISTER_OBJECT(0, USSLSession)
      }

   // define method VIRTUAL of class UDataStorage

   virtual char* toBuffer() U_DECL_FINAL;
   virtual void  fromData(const char* ptr, uint32_t len) U_DECL_FINAL;

   // SERVICES

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const { return UDataStorage::dump(reset); }
#endif

private:
   static SSL_SESSION* sess;

   static int     newSession(SSL* ssl,     SSL_SESSION* sess);
   static void removeSession(SSL_CTX* ctx, SSL_SESSION* sess);

   static SSL_SESSION* getSession(SSL* ssl, unsigned char* id, int len, int* copy);

   U_DISALLOW_COPY_AND_ASSIGN(USSLSession)

   friend class UHTTP;
   friend class UHttpPlugIn;
};

#endif
