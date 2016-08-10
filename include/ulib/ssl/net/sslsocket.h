// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    sslsocket.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SSLSOCKET_H
#define ULIB_SSLSOCKET_H 1

#include <ulib/net/socket.h>

#ifndef USE_LIBSSL
#define USE_LIBSSL
#endif

#include <ulib/utility/services.h>

#include <openssl/ssl.h>
#include <openssl/x509.h>

#if !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
#  include <openssl/ocsp.h>
#  ifndef U_OCSP_MAX_RESPONSE_SIZE
#  define U_OCSP_MAX_RESPONSE_SIZE 10240
#  endif
#endif

/**
 * This class implements TCP/IP sockets with the Secure Sockets Layer (SSL v2/v3) and
 * Transport Layer Security (TLS v1) protocols. The OpenSSL library is used in this implementation,
 * see the OpenSSL homepage for more information
 */

#define SSL_VERIFY_PEER_STRICT (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT)

typedef int (*PEM_PASSWD_CB)(char* buf, int size, int rwflag, void* password); // Password callback

class UHTTP;
class UTCPSocket;
class UHttpPlugIn;
class UClient_Base;
class UServer_Base;
class UClientImage_Base;

template <class T> class UClient;
template <class T> class UServer;
template <class T> class UClientImage;

class U_EXPORT USSLSocket : public USocket {
public:

   // see: https://wiki.mozilla.org/Security/Server_Side_TLS

   enum CipherSuiteModel {
      Intermediate = 0x000,
      Modern       = 0x001,
      Old          = 0x002
   };

    USSLSocket(bool bSocketIsIPv6 = false, SSL_CTX* ctx = 0, bool server = false);
   ~USSLSocket();

   bool secureConnection();
   bool acceptSSL(USSLSocket* pcConnection);

   const char* getProtocolList()       { return (ciphersuite_model == 1 ? "TLSv1.2,TLSv1.1" :
                                                 ciphersuite_model == 2 ? "TLSv1.2,TLSv1.1,TLSv1.0,SSLv3" : "TLSv1.2,TLSv1.1,TLSv1.0"); }

   const char* getConfigurationModel() { return (ciphersuite_model == 1 ? "Modern" :
                                                 ciphersuite_model == 2 ? "Old"    : "Intermediate"); }

   static long getOptions(const UVector<UString>& vec);

   /**
    * Load Diffie-Hellman parameters from file. These are used to generate a DH key exchange.
    * See man SSL_CTX_set_tmp_dh_callback(3) and www.skip-vpn.org/spec/numbers.html for more information.
    * Should be called before accept() or connect() if used. Returns true on success
    */

   bool useDHFile(const char* dh_file = 0);

   /**
    * Load a certificate. A socket used on the server side needs to have a certificate (but a temporary RSA session
    * certificate may be created if you don't load one yourself). The client side can also load certificates but it
    * is not required. The files should be in ASCII PEM format and the certificate and the private key can either be
    * in the same file or two separate files. OpenSSL's standard password prompt will be used if the private key uses
    * a password. You should load the certificate before calling accept() or connect(). Should the peer certificate be
    * verified ? The arguments specify the locations of trusted CA certificates used in the verification. Either CAfile
    * or CApath can be set to NULL. See man SSL_CTX_load_verify_locations(3) for format information. Should be called
    * before accept() or connect() if used and the verification result is then available by calling getVerifyResult()
    * on the connected socket (the new socket from accept() on the server side, the same socket on the client side).
    * Returns true on success
    */

   bool setContext(const char* dh_file, const char* cert_file,
                   const char* private_key_file, const char* passwd,
                   const char* CAfile, const char* CApath,
                   int mode = SSL_VERIFY_PEER_STRICT | SSL_VERIFY_CLIENT_ONCE);

   /**
    * For successful verification, the peer certificate must be signed with the CA certificate directly or indirectly
    * (a proper certificate chain exists). The certificate chain length from the CA certificate to the peer certificate
    * can be set in the verify_depth field of the SSL_CTX and SSL structures. (The value in SSL is inherited from SSL_CTX
    * when you create an SSL structure using the SSL_new() API). Setting verify_depth to 1 means that the peer certificate
    * must be directly signed by the CA certificate 
    */

   void setVerifyDepth(int depth = 1)
      {
      U_TRACE(1, "USSLSocket::setVerifyDepth(%d)", depth)

      U_INTERNAL_ASSERT_POINTER(ctx)

      U_SYSCALL_VOID(SSL_CTX_set_verify_depth, "%p,%d", ctx, depth);
      }

   /**
    * Verify callback
    */

   void setVerifyCallback(verify_cb func, int mode = SSL_VERIFY_PEER_STRICT | SSL_VERIFY_CLIENT_ONCE)
      {
      U_TRACE(1, "USSLSocket::setVerifyCallback(%p,%d)", func, mode)

      U_INTERNAL_ASSERT_POINTER(func)

      U_SYSCALL_VOID(SSL_CTX_set_verify, "%p,%d,%p", ctx, mode, func);
      }

   /**
    * Gets the peer certificate verification result. Should be called after connect() or accept() where the verification is
    * done. On the server side this should be done on the new object returned by accept() and not on the listener object! If
    * you don't get X509_V_OK and don't trust the peer you should disconnect. If you trust the peer (or perhaps ask the user
    * if he/she does) but didn't get X509_V_OK you might consider adding this certificate to the trusted CA certificates loaded
    * by setContext(), but don't add invalid certificates...
    */

   long getVerifyResult()
      {
      U_TRACE_NO_PARAM(1, "USSLSocket::getVerifyResult()")

      U_INTERNAL_ASSERT_POINTER(ssl)

      long result = U_SYSCALL(SSL_get_verify_result, "%p", ssl);

#  ifdef DEBUG
      UServices::setVerifyStatus(result);

      U_INTERNAL_DUMP("verify_status = %.*S", u_buffer_len, u_buffer)

      u_buffer_len = 0;
#  endif

      U_RETURN(result);
      }

   /**
    * Get peer certificate. Should be called after connect() or accept() when using verification
    * NB: OpenSSL already tested the cert validity during SSL handshake and returns a X509 ptr just if the certificate is valid...
    */

   X509* getPeerCertificate()
      {
      U_TRACE_NO_PARAM(1, "USSLSocket::getPeerCertificate()")

      X509* peer = (X509*) (ssl ? U_SYSCALL(SSL_get_peer_certificate, "%p", ssl) : 0);

      U_RETURN_POINTER(peer, X509);
      }

   /**
    * Server side RE-NEGOTIATE asking for client cert
    */

   bool askForClientCertificate();

   /**
    * Returns the number of bytes which are available inside ssl for immediate read
    */

   uint32_t pending() const
      {
      U_TRACE_NO_PARAM(0, "USSLSocket::pending()")

      if (USocket::isSSLActive())
         {
         U_INTERNAL_DUMP("this = %p ssl = %p", this, ssl)

         U_INTERNAL_ASSERT_POINTER(ssl)

         // NB: data are received in blocks from the peer. Therefore data can be buffered
         // inside ssl and are ready for immediate retrieval with SSL_read()...

         uint32_t result = U_SYSCALL(SSL_pending, "%p", ssl);

         U_RETURN(result);
         }

      U_RETURN(0);
      }

   // VIRTUAL METHOD

   virtual int send(const char* pData,   uint32_t iDataLen) U_DECL_FINAL;
   virtual int recv(      void* pBuffer, uint32_t iBufferLen) U_DECL_FINAL;

   /**
    * This method is called to connect the socket to a server SSL that is specified
    * by the provided host name and port number. We call the SSL_connect() function to perform the connection
    */

   virtual bool connectServer(const UString& server, unsigned int iServPort, int timeoutMS = 0) U_DECL_FINAL;

   /**
    * OCSP stapling is a way for a SSL server to obtain OCSP responses for his own certificate, and provide them to the client,
    * under the assumption that the client may need them. This makes the whole process more efficient: the client does not have
    * to open extra connections to get the OCSP responses itself, and the same OCSP response can be sent by the server to all
    * clients within a given time frame. One way to see it is that the SSL server acts as a Web proxy for the purpose of
    * downloading OCSP responses
    */

#if !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   typedef struct stapling {
      void* data;
      char* path;
      long valid;
      X509* cert;
      X509* issuer;
      UString* url;
      EVP_PKEY* pkey;
      int len, verify;
      OCSP_CERTID* id;
      OCSP_REQUEST* req;
      UClient<UTCPSocket>* client;
   } stapling;

   static stapling staple;
   static bool doStapling();

   static void cleanupStapling();
   static bool setDataForStapling();
   static void certificate_status_callback(SSL* _ssl, void* data);
#endif

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   SSL* ssl;
   SSL_CTX* ctx;
   int ret, renegotiations, ciphersuite_model;

   void closesocket();

   static SSL_CTX* cctx; // client
   static SSL_CTX* sctx; // server

   static int session_cache_index;

   static void setStatus(SSL* _ssl, int _ret, bool _flag);
   static void info_callback(const SSL* ssl, int where, int ret);

   void setStatus(bool _flag) const { setStatus(ssl, ret, _flag); }

#ifdef DEBUG
   void dumpStatus(bool _flag) const
      {
      setStatus(ssl, ret, _flag);

      u_buffer_len = 0;
      }

   void dumpStatus(int _ret, bool _flag) const
      {
      setStatus(ssl, _ret, _flag);

      u_buffer_len = 0;
      }
#endif

   static SSL_CTX* getClientContext() { return getContext(0, false, 0); }
   static SSL_CTX* getServerContext() { return getContext(0, true,  0); }

   static SSL_CTX* getContext(SSL_METHOD* method, bool server, long options);

   /**
    * A TLS extension called "Server Name Indication" (SNI) allows name-based HTTPS virtual hosting.
    * This is especially common when serving HTTPS requests with a wildcard certificate (*.domain.tld)
    */

#if !defined(OPENSSL_NO_TLSEXT) && defined(SSL_set_tlsext_host_name)
   static int callback_ServerNameIndication(SSL* _ssl, int* alert, void* data);
#endif

private:
   static int nextProto(SSL* ssl, const unsigned char** data, unsigned int* len, void* arg)
      {
      U_TRACE(0, "USSLSocket::nextProto(%p,%p,%p,%p)", ssl, data, len, arg)

      *data = (unsigned char*)arg;
      *len  = U_CONSTANT_SIZE("\x2h2\x5h2-16\x5h2-14");

      U_RETURN(SSL_TLSEXT_ERR_OK);
      }

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
   static int selectProto(SSL* ssl, const unsigned char** out, unsigned char* outlen, const unsigned char* in, unsigned int inlen, void* arg) U_NO_EXPORT;
#endif

   U_DISALLOW_COPY_AND_ASSIGN(USSLSocket)

                      friend class UHTTP;
                      friend class USocket;
                      friend class UHttpPlugIn;
                      friend class UClient_Base;
                      friend class UServer_Base;
                      friend class UClientImage_Base;
   template <class T> friend class UClient;
   template <class T> friend class UServer;
   template <class T> friend class UClientImage;
};

#endif
