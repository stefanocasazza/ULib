// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    sslsocket.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/services.h>
#include <ulib/ssl/net/sslsocket.h>
#include <ulib/net/client/client.h>
#include <ulib/net/server/server.h>

#ifdef _MSWINDOWS_
#  undef X509_NAME
#else
#  include <openssl/x509v3.h>
#endif

#if OPENSSL_VERSION_NUMBER >= 0x0090800fL && !defined(OPENSSL_NO_ECDH) && defined(NID_X9_62_prime256v1)
#include <openssl/ec.h>
#endif
#include <openssl/err.h>

#ifndef SSL_ERROR_WANT_ACCEPT
#define SSL_ERROR_WANT_ACCEPT SSL_ERROR_WANT_READ
#endif

int         USSLSocket::session_cache_index;
SSL_CTX*    USSLSocket::cctx; // client
SSL_CTX*    USSLSocket::sctx; // server

#if !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
USSLSocket::stapling USSLSocket::staple;
#endif

/**
 * The OpenSSL ssl library implements the Secure Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1) protocols.
 * It provides a rich API. At first the library must be initialized; see SSL_library_init(3). Then an SSL_CTX object is created
 * as a framework to establish TLS/SSL enabled connections (see SSL_CTX_new(3)). Various options regarding certificates, algorithms
 * etc. can be set in this object. When a network connection has been created, it can be assigned to an SSL object. After the SSL
 * object has been created using SSL_new(3), SSL_set_fd(3) or SSL_set_bio(3) can be used to associate the network connection with
 * the object. Then the TLS/SSL handshake is performed using SSL_accept(3) or SSL_connect(3) respectively. SSL_read(3) and SSL_write(3)
 * are used to read and write data on the TLS/SSL connection. SSL_shutdown(3) can be used to shut down the TLS/SSL connection. 
 *
 * When packets in SSL arrive at a destination, they are pulled off the socket in chunks of sizes controlled by the encryption protocol being
 * used, decrypted, and placed in SSL-internal buffers. The buffer content is then transferred to the application program through SSL_read().
 * If you've read only part of the decrypted data, there will still be pending input data on the SSL connection, but it won't show up on the
 * underlying file descriptor via select(). Your code needs to call SSL_pending() explicitly to see if there is any pending data to be read.
 *
 * NON-blocking I/O
 *
 * A pitfall to avoid: Don't assume that SSL_read() will just read from the underlying transport or that SSL_write() will just write to it
 * it is also possible that SSL_write() cannot do any useful work until there is data to read, or that SSL_read() cannot do anything until
 * it is possible to send data. One reason for this is that the peer may request a new TLS/SSL handshake at any time during the protocol,
 * requiring a bi-directional message exchange; both SSL_read() and SSL_write() will try to continue any pending handshake
 */

USSLSocket::USSLSocket(bool bSocketIsIPv6, SSL_CTX* _ctx, bool bserver) : USocket(bSocketIsIPv6)
{
   U_TRACE_REGISTER_OBJECT(0, USSLSocket, "%b,%p,%b", bSocketIsIPv6, _ctx, bserver)

   ciphersuite_model = Intermediate;

   if (_ctx) ctx = _ctx;
   else
      {
      if (bserver)
         {
         if (sctx == 0) sctx = getServerContext();

         ctx = sctx;
         }
      else
         {
         if (cctx == 0) cctx = getClientContext();

         ctx = cctx;
         }

      U_INTERNAL_ASSERT_POINTER(ctx)
      }

   // We don't want our destructor to delete ctx if still in use...

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   ctx->references++;
#else
   SSL_CTX_up_ref(ctx);
#endif

   ssl = 0;
   ret = renegotiations = 0;

   U_socket_Type(this) |= USocket::SK_SSL;

   U_ASSERT(USocket::isSSL())
}

USSLSocket::~USSLSocket()
{
   U_TRACE_UNREGISTER_OBJECT(0, USSLSocket)

   U_INTERNAL_ASSERT_POINTER(ctx)

   if (ssl) SSL_free(ssl); // SSL_free will free UServices::store
            SSL_CTX_free(ctx);
}

void USSLSocket::info_callback(const SSL* ssl, int where, int ret)
{
   U_TRACE(0, "USSLSocket::info_callback(%p,%d,%d)", ssl, where, ret)

   if ((where & SSL_CB_HANDSHAKE_START) != 0)
      {
      U_INTERNAL_DUMP("SSL_CB_HANDSHAKE_START")

      USSLSocket* pobj = (USSLSocket*) SSL_get_app_data((SSL*)ssl);

      if (pobj)
         {
         pobj->renegotiations++;

         U_INTERNAL_DUMP("pobj->renegotiations = %d", pobj->renegotiations)
         }
      }
   else if ((where & SSL_CB_HANDSHAKE_DONE) != 0)
      {
      U_INTERNAL_DUMP("SSL_CB_HANDSHAKE_DONE")

#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      if (ssl->s3) ssl->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
#  endif
      }
}

SSL_CTX* USSLSocket::getContext(SSL_METHOD* method, bool bserver, long options)
{
   U_TRACE(0, "USSLSocket::getContext(%p,%b,%ld)", method, bserver, options)

   if (method == 0)
      {
      /**
       * Counter-intuitively, the OpenSSL folks have TLSv1_client_method() negotiate only TLSv1.0, and SSLv23_client_method()
       * remains the only method which can negotiate different versions. This is true at least as of 1.0.1c (the latest release
       * at time of writing). And as you can see here: http://www.openssl.org/docs/ssl/SSL_CTX_new.html
       *
       * TLSv1_method(void), TLSv1_server_method(void), TLSv1_client_method(void) A TLS/SSL connection established with these
       * methods will only understand the TLSv1 protocol. A client will send out TLSv1 client hello messages and will indicate
       * that it only understands TLSv1. A server will only understand TLSv1 client hello messages. This especially means, that
       * it will not understand SSLv2 client hello messages which are widely used for compatibility reasons, see SSLv23_*_method().
       * It will also not understand SSLv3 client hello messages.
       *
       * SSLv23_method(void), SSLv23_server_method(void), SSLv23_client_method(void) A TLS/SSL connection established with these
       * methods will understand the SSLv2, SSLv3, and TLSv1 protocol. A client will send out SSLv2 client hello messages and will
       * indicate that it also understands SSLv3 and TLSv1. A server will understand SSLv2, SSLv3, and TLSv1 client hello messages.
       * This is the best choice when compatibility is a concern. The list of protocols available can later be limited using the
       * SSL_OP_NO_SSLv2, SSL_OP_NO_SSLv3, SSL_OP_NO_TLSv1 options of the SSL_CTX_set_options() or SSL_set_options() functions.
       * Using these options it is possible to choose e.g. SSLv23_server_method() and be able to negotiate with all possible clients,
       * but to only allow newer protocols like SSLv3 or TLSv1
       */

#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      if (bserver) method = (SSL_METHOD*)SSLv23_server_method();
      else         method = (SSL_METHOD*)SSLv23_client_method();
#  else
      if (bserver) method = (SSL_METHOD*)TLS_server_method();
      else         method = (SSL_METHOD*)TLS_client_method();
#  endif
      }

   SSL_CTX* ctx = (SSL_CTX*) U_SYSCALL(SSL_CTX_new, "%p", method);

   U_SYSCALL_VOID(SSL_CTX_set_quiet_shutdown,     "%p,%d", ctx, 1);
   U_SYSCALL_VOID(SSL_CTX_set_default_read_ahead, "%p,%d", ctx, 1);

   if (options) (void) U_SYSCALL(SSL_CTX_set_options, "%p,%d", ctx, options);

   if (bserver)
      {
      U_INTERNAL_ASSERT_MINOR(u_progname_len, SSL_MAX_SSL_SESSION_ID_LENGTH)

      (void) U_SYSCALL(SSL_CTX_set_session_cache_mode, "%p,%d",    ctx, SSL_SESS_CACHE_SERVER);
      (void) U_SYSCALL(SSL_CTX_set_session_id_context, "%p,%p,%u", ctx, (const unsigned char*)u_progname, u_progname_len);

      // We need this to disable client-initiated renegotiation

      U_SYSCALL_VOID(SSL_CTX_set_info_callback, "%p,%p", ctx, USSLSocket::info_callback);
      }

   // Release/reuse buffers as soon as possibile

#ifdef SSL_MODE_RELEASE_BUFFERS
   (void) U_SYSCALL(SSL_CTX_set_mode, "%p,%d", ctx, SSL_CTX_get_mode(ctx) | SSL_MODE_RELEASE_BUFFERS);
#endif

   U_RETURN_POINTER(ctx, SSL_CTX);
}

/**
 * get OpenSSL-specific options (default: NO_SSLv2, CIPHER_SERVER_PREFERENCE, NO_COMPRESSION)
 *
 * to overwrite defaults you need to explicitly specify the reverse flag (toggle "NO_" prefix)
 *
 * example: use sslv2 and compression: [ options: ("SSLv2", "COMPRESSION") ]
 */

long USSLSocket::getOptions(const UVector<UString>& vec)
{
   U_TRACE(0, "USSLSocket::getOptions(%p", &vec)

   static const struct {
      const char*   name;      // without "NO_" prefix
      uint32_t      name_len;
      unsigned long value;
      char          positive;  // 0 means option is usually prefixed with "NO_"; otherwise use 1
   } option_table[] = {
   { U_CONSTANT_TO_PARAM("MICROSOFT_SESS_ID_BUG"), SSL_OP_MICROSOFT_SESS_ID_BUG, 1 },
   { U_CONSTANT_TO_PARAM("NETSCAPE_CHALLENGE_BUG"), SSL_OP_NETSCAPE_CHALLENGE_BUG, 1 },
#ifdef SSL_OP_LEGACY_SERVER_CONNECT
   { U_CONSTANT_TO_PARAM("LEGACY_SERVER_CONNECT"), SSL_OP_LEGACY_SERVER_CONNECT, 1 },
#endif
   { U_CONSTANT_TO_PARAM("NETSCAPE_REUSE_CIPHER_CHANGE_BUG"), SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG, 1 },
   { U_CONSTANT_TO_PARAM("SSLREF2_REUSE_CERT_TYPE_BUG"), SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG, 1 },
   { U_CONSTANT_TO_PARAM("MICROSOFT_BIG_SSLV3_BUFFER"), SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER, 1 },
   { U_CONSTANT_TO_PARAM("MSIE_SSLV2_RSA_PADDING"), SSL_OP_MSIE_SSLV2_RSA_PADDING, 1 },
   { U_CONSTANT_TO_PARAM("SSLEAY_080_CLIENT_DH_BUG"), SSL_OP_SSLEAY_080_CLIENT_DH_BUG, 1 },
   { U_CONSTANT_TO_PARAM("TLS_D5_BUG"), SSL_OP_TLS_D5_BUG, 1 },
   { U_CONSTANT_TO_PARAM("TLS_BLOCK_PADDING_BUG"), SSL_OP_TLS_BLOCK_PADDING_BUG, 1 },
#ifdef SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS
   { U_CONSTANT_TO_PARAM("DONT_INSERT_EMPTY_FRAGMENTS"), SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS, 1 },
#endif
   { U_CONSTANT_TO_PARAM("ALL"), SSL_OP_ALL, 1 },
#ifdef SSL_OP_NO_QUERY_MTU
   { U_CONSTANT_TO_PARAM("QUERY_MTU"), SSL_OP_NO_QUERY_MTU, 0 },
#endif
#ifdef SSL_OP_COOKIE_EXCHANGE
   { U_CONSTANT_TO_PARAM("COOKIE_EXCHANGE"), SSL_OP_COOKIE_EXCHANGE, 1 },
#endif
#ifdef SSL_OP_NO_TICKET
   { U_CONSTANT_TO_PARAM("TICKET"), SSL_OP_NO_TICKET, 0 },
#endif
#ifdef SSL_OP_CISCO_ANYCONNECT
   { U_CONSTANT_TO_PARAM("CISCO_ANYCONNECT"), SSL_OP_CISCO_ANYCONNECT, 1 },
#endif
#ifdef SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
   { U_CONSTANT_TO_PARAM("SESSION_RESUMPTION_ON_RENEGOTIATION"), SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION, 0 },
#endif
#ifdef SSL_OP_NO_COMPRESSION
   { U_CONSTANT_TO_PARAM("COMPRESSION"), SSL_OP_NO_COMPRESSION, 0 },
#endif
#ifdef SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION
   { U_CONSTANT_TO_PARAM("ALLOW_UNSAFE_LEGACY_RENEGOTIATION"), SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION, 1 },
#endif
#ifdef SSL_OP_SINGLE_ECDH_USE
   { U_CONSTANT_TO_PARAM("SINGLE_ECDH_USE"), SSL_OP_SINGLE_ECDH_USE, 1 },
#endif
   { U_CONSTANT_TO_PARAM("SINGLE_DH_USE"), SSL_OP_SINGLE_DH_USE, 1 },
   { U_CONSTANT_TO_PARAM("EPHEMERAL_RSA"), SSL_OP_EPHEMERAL_RSA, 1 },
#ifdef SSL_OP_CIPHER_SERVER_PREFERENCE
   { U_CONSTANT_TO_PARAM("CIPHER_SERVER_PREFERENCE"), SSL_OP_CIPHER_SERVER_PREFERENCE, 1 },
#endif
   { U_CONSTANT_TO_PARAM("TLS_ROLLBACK_BUG"), SSL_OP_TLS_ROLLBACK_BUG, 1 },
   { U_CONSTANT_TO_PARAM("SSLv2"), SSL_OP_NO_SSLv2, 0 },
   { U_CONSTANT_TO_PARAM("SSLv3"), SSL_OP_NO_SSLv3, 0 },
   { U_CONSTANT_TO_PARAM("TLSv1"), SSL_OP_NO_TLSv1, 0 },
   { U_CONSTANT_TO_PARAM("PKCS1_CHECK_1"), SSL_OP_PKCS1_CHECK_1, 1 },
   { U_CONSTANT_TO_PARAM("PKCS1_CHECK_2"), SSL_OP_PKCS1_CHECK_2, 1 },
   { U_CONSTANT_TO_PARAM("NETSCAPE_CA_DN_BUG"), SSL_OP_NETSCAPE_CA_DN_BUG, 1 },
   { U_CONSTANT_TO_PARAM("NETSCAPE_DEMO_CIPHER_CHANGE_BUG"), SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG, 1 },
#ifdef SSL_OP_CRYPTOPRO_TLSEXT_BUG
   { U_CONSTANT_TO_PARAM("CRYPTOPRO_TLSEXT_BUG"), SSL_OP_CRYPTOPRO_TLSEXT_BUG, 1 }
#endif
   };

   uint32_t j;
   UString key;

   long options = SSL_OP_NO_SSLv2       |
#              ifdef SSL_OP_NO_COMPRESSION
                  SSL_OP_NO_COMPRESSION |
#              endif
                  SSL_OP_CIPHER_SERVER_PREFERENCE;

   for (uint32_t i = 0, n = vec.size(); i < n; ++i)
      {
      uint32_t len    = key.size();
      const char* ptr = key.data();

      char positive = 1;

      if (u__strncasecmp(ptr, U_CONSTANT_TO_PARAM("NO_")) == 0)
         {
         ptr += 3;
         len -= 3;

         positive = 0;
         }

      for (j = 0; j < U_NUM_ELEMENTS(option_table); ++j)
         {
         if (option_table[j].name_len == len &&
             u__strncasecmp(ptr, option_table[j].name, option_table[j].name_len) == 0)
            {
            if (option_table[j].positive == positive) options |=  option_table[j].value;
            else                                      options &= ~option_table[j].value;
            }
         }
      }

   U_RETURN(options);
}

bool USSLSocket::useDHFile(const char* dh_file)
{
   U_TRACE(1, "USSLSocket::useDHFile(%S)", dh_file)

   U_INTERNAL_ASSERT_POINTER(ctx)

   /**
    * When an ephemeral Diffie-Hellman cipher is used, the server and the client negotiate a pre-master key using the Diffie-Hellman algorithm.
    * This algorithm requires that the server sends the client a prime number and a generator. Neither are confidential, and are sent in clear
    * text. However, they must be signed, such that a MITM cannot hijack the handshake.
    * As an example, TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 works as follow:
    *
    * 1. Server sends Client a [SERVER KEY EXCHANGE] message during the SSL Handshake. The message contains:
    *    Prime number p
    *    Generator g
    *    Server's Diffie-Hellman public value A = g^X mod p, where X is a private integer chosen by the server at random, and never shared with the client
    *    signature S of the above (plus two random values) computed using the Server's private RSA key
    *
    * 2. Client verifies the signature S
    * 3. Client sends server a [CLIENT KEY EXCHANGE] message. The message contains:
    *    Client's Diffie-Hellman public value B = g^Y mod p, where Y is a private integer chosen at random and never shared
    *
    * 4. The Server and the Client can now calculate the pre-master secret using each other's public values:
    *    server calculates PMS = B^X mod p
    *    client calculates PMS = A^Y mod p
    *
    * 5. Client sends a [CHANGE CIPHER SPEC] message to the server, and both parties continue the handshake using ENCRYPTED HANDSHAKE MESSAGES
    *
    * The size of the prime number p constrains the size of the pre-master key PMS, because of the modulo operation. A smaller prime almost
    * means weaker values of A and B, which could leak the secret values X and Y. Thus, the prime p should not be smaller than the size of
    * the RSA private key
    */

   DH* dh = 0;

   if ( dh_file &&
       *dh_file)
      {
      FILE* paramfile = (FILE*) U_SYSCALL(fopen, "%S,%S", dh_file, "r");

      if (paramfile == 0) U_RETURN(false);

      dh = (DH*) U_SYSCALL(PEM_read_DHparams, "%p,%p,%p,%p", paramfile, 0, 0, 0);

      (void) U_SYSCALL(fclose, "%p", paramfile);
      }
   else
      {
      /**
       * The concept of forward secrecy is simple: client and server negotiate a key that never hits the wire,
       * and is destroyed at the end of the session. The RSA private from the server is used to sign a Diffie-Hellman
       * key exchange between the client and the server. The pre-master key obtained from the Diffie-Hellman handshake
       * is then used for encryption. Since the pre-master key is specific to a connection between a client and a server,
       * and used only for a limited amount of time, it is called Ephemeral. With Forward Secrecy, if an attacker gets a
       * hold of the server's private key, it will not be able to decrypt past communications. The private key is only
       * used to sign the DH handshake, which does not reveal the pre-master key. Diffie-Hellman ensures that the pre-master
       * keys never leave the client and the server, and cannot be intercepted by a MITM
       */
#  if OPENSSL_VERSION_NUMBER >= 0x0090800fL && !defined(OPENSSL_NO_ECDH) && defined(NID_X9_62_prime256v1)
      EC_KEY* ecdh = (EC_KEY*) U_SYSCALL(EC_KEY_new_by_curve_name, "%d", NID_X9_62_prime256v1);

      (void) U_SYSCALL(SSL_CTX_set_tmp_ecdh, "%p,%p", ctx, ecdh);

      U_SYSCALL_VOID(EC_KEY_free, "%p", ecdh);

      U_RETURN(true);
#  else
      static unsigned char dhxxx2_g[] = { 0x02 };
      static unsigned char dh1024_p[] = {
         0xA2,0x95,0x7E,0x7C,0xA9,0xD5,0x55,0x1D,0x7C,0x77,0x11,0xAC,
         0xFD,0x48,0x8C,0x3B,0x94,0x1B,0xC5,0xC0,0x99,0x93,0xB5,0xDC,
         0xDC,0x06,0x76,0x9E,0xED,0x1E,0x3D,0xBB,0x9A,0x29,0xD6,0x8B,
         0x1F,0xF6,0xDA,0xC9,0xDF,0xD5,0x02,0x4F,0x09,0xDE,0xEC,0x2C,
         0x59,0x1E,0x82,0x32,0x80,0x9B,0xED,0x51,0x68,0xD2,0xFB,0x1E,
         0x25,0xDB,0xDF,0x9C,0x11,0x70,0xDF,0xCA,0x19,0x03,0x3D,0x3D,
         0xC1,0xAC,0x28,0x88,0x4F,0x13,0xAF,0x16,0x60,0x6B,0x5B,0x2F,
         0x56,0xC7,0x5B,0x5D,0xDE,0x8F,0x50,0x08,0xEC,0xB1,0xB9,0x29,
         0xAA,0x54,0xF4,0x05,0xC9,0xDF,0x95,0x9D,0x79,0xC6,0xEA,0x3F,
         0xC9,0x70,0x42,0xDA,0x90,0xC7,0xCC,0x12,0xB9,0x87,0x86,0x39,
         0x1E,0x1A,0xCE,0xF7,0x3F,0x15,0xB5,0x2B };
      static unsigned char dh2048_p[] = {
         0xF2,0x4A,0xFC,0x7E,0x73,0x48,0x21,0x03,0xD1,0x1D,0xA8,0x16,
         0x87,0xD0,0xD2,0xDC,0x42,0xA8,0xD2,0x73,0xE3,0xA9,0x21,0x31,
         0x70,0x5D,0x69,0xC7,0x8F,0x95,0x0C,0x9F,0xB8,0x0E,0x37,0xAE,
         0xD1,0x6F,0x36,0x1C,0x26,0x63,0x2A,0x36,0xBA,0x0D,0x2A,0xF5,
         0x1A,0x0F,0xE8,0xC0,0xEA,0xD1,0xB5,0x52,0x47,0x1F,0x9A,0x0C,
         0x0F,0xED,0x71,0x51,0xED,0xE6,0x62,0xD5,0xF8,0x81,0x93,0x55,
         0xC1,0x0F,0xB4,0x72,0x64,0xB3,0x73,0xAA,0x90,0x9A,0x81,0xCE,
         0x03,0xFD,0x6D,0xB1,0x27,0x7D,0xE9,0x90,0x5E,0xE2,0x10,0x74,
         0x4F,0x94,0xC3,0x05,0x21,0x73,0xA9,0x12,0x06,0x9B,0x0E,0x20,
         0xD1,0x5F,0xF7,0xC9,0x4C,0x9D,0x4F,0xFA,0xCA,0x4D,0xFD,0xFF,
         0x6A,0x62,0x9F,0xF0,0x0F,0x3B,0xA9,0x1D,0xF2,0x69,0x29,0x00,
         0xBD,0xE9,0xB0,0x9D,0x88,0xC7,0x4A,0xAE,0xB0,0x53,0xAC,0xA2,
         0x27,0x40,0x88,0x58,0x8F,0x26,0xB2,0xC2,0x34,0x7D,0xA2,0xCF,
         0x92,0x60,0x9B,0x35,0xF6,0xF3,0x3B,0xC3,0xAA,0xD8,0x58,0x9C,
         0xCF,0x5D,0x9F,0xDB,0x14,0x93,0xFA,0xA3,0xFA,0x44,0xB1,0xB2,
         0x4B,0x0F,0x08,0x70,0x44,0x71,0x3A,0x73,0x45,0x8E,0x6D,0x9C,
         0x56,0xBC,0x9A,0xB5,0xB1,0x3D,0x8B,0x1F,0x1E,0x2B,0x0E,0x93,
         0xC2,0x9B,0x84,0xE2,0xE8,0xFC,0x29,0x85,0x83,0x8D,0x2E,0x5C,
         0xDD,0x9A,0xBB,0xFD,0xF0,0x87,0xBF,0xAF,0xC4,0xB6,0x1D,0xE7,
         0xF9,0x46,0x50,0x7F,0xC3,0xAC,0xFD,0xC9,0x8C,0x9D,0x66,0x6B,
         0x4C,0x6A,0xC9,0x3F,0x0C,0x0A,0x74,0x94,0x41,0x85,0x26,0x8F,
         0x9F,0xF0,0x7C,0x0B };

      dh = (DH*) U_SYSCALL_NO_PARAM(DH_new);

      dh->g = BN_bin2bn(dhxxx2_g, sizeof(dhxxx2_g), 0);

      U_INTERNAL_ASSERT_POINTER(dh->g)

      switch (ciphersuite_model)
         {
         case Modern: dh->p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0); break;
         case    Old: dh->p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), 0); break;
         default:     dh->p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0); break; // Intermediate
         }

      U_INTERNAL_ASSERT_POINTER(dh->p)
#  endif
      }

   if (dh == 0) U_RETURN(false);

   /*
#ifdef DEBUG
   unsigned char buf[4096];

   int len  = i2d_DHparams(dh, 0),
       size = i2d_DHparams(dh, (unsigned char**)&buf);

   U_INTERNAL_DUMP("len = %d buf(%d) = %#.*S", len, size, size, buf)
#endif
   */

   (void) U_SYSCALL(SSL_CTX_set_tmp_dh, "%p,%p", ctx, dh);

   U_SYSCALL_VOID(DH_free, "%p", dh);

   U_RETURN(true);
}

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
U_NO_EXPORT int USSLSocket::selectProto(SSL* ssl, const unsigned char** out, unsigned char* outlen, const unsigned char* in, unsigned int inlen, void* arg)
{
   U_TRACE(0, "USSLSocket::selectProto(%p,%p,%p,%.*S,%u,%p)", ssl, out, outlen, inlen, in, inlen, arg)

#ifdef DEBUG
   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   for (unsigned int i = 0; i < inlen; i += in[i]+1)
      {
      u_buffer_len += u__snprintf(u_buffer+u_buffer_len, U_BUFFER_SIZE-u_buffer_len, U_CONSTANT_TO_PARAM("%.*s "), in[i], (const char*)(&in[i+1]));
      }

   U_INTERNAL_DUMP("[ALPN] client offers = %.*S", u_buffer_len, u_buffer)

   u_buffer_len = 0;
#endif

   const unsigned char* p;
   const unsigned char* end;

   for (p = in, end = in + inlen; p <= end; p += *p + 1)
      {
      if (memcmp(p, U_CONSTANT_TO_PARAM("\x2h2"))    == 0 ||
          memcmp(p, U_CONSTANT_TO_PARAM("\x5h2-16")) == 0 ||
          memcmp(p, U_CONSTANT_TO_PARAM("\x5h2-14")) == 0)
         {
         *out    =  p+1;
         *outlen = *p;

         U_RETURN(SSL_TLSEXT_ERR_OK);
         }
      }

   U_RETURN(SSL_TLSEXT_ERR_NOACK);
}
#endif

bool USSLSocket::setContext(const char* dh_file, const char* cert_file, const char* private_key_file,
                            const char* passwd,  const char* CAfile,    const char* CApath, int verify_mode)
{
   U_TRACE(1, "USSLSocket::setContext(%S,%S,%S,%S,%S,%S,%d)", dh_file, cert_file, private_key_file, passwd, CAfile, CApath, verify_mode)

   U_INTERNAL_ASSERT_POINTER(ctx)

   // These are the bit DH parameters from "Assigned Number for SKIP Protocols"
   // See there for how they were generated: http://www.skip-vpn.org/spec/numbers.html

#if OPENSSL_VERSION_NUMBER >= 0x10002000L && OPENSSL_VERSION_NUMBER < 0x10100000L
   SSL_CTX_set_ecdh_auto(ctx, 1);
#else
   if (useDHFile(dh_file) == false) U_RETURN(false);
#endif

   int result = 0;

   // Load CERT PEM file

   if ( cert_file &&
       *cert_file)
      {
      result = U_SYSCALL(SSL_CTX_use_certificate_chain_file, "%p,%S", ctx, cert_file);

      if (result == 0) U_RETURN(false);

#  if !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
      UString str(cert_file, u__strlen(cert_file, __PRETTY_FUNCTION__));

      staple.cert = UCertificate::readX509(UFile::contentOf(str), "PEM");

      U_INTERNAL_DUMP("staple.cert = %p", staple.cert)
#  endif
      }

   // Load private key PEM file and give passwd callback if any

   if ( private_key_file &&
       *private_key_file)
      {
      if ( passwd &&
          *passwd)
         {
         U_SYSCALL_VOID(SSL_CTX_set_default_passwd_cb,          "%p,%p", ctx, UServices::passwd_cb);
         U_SYSCALL_VOID(SSL_CTX_set_default_passwd_cb_userdata, "%p,%S", ctx, (void*)passwd);
         }

      for (int i = 0; i < 3; ++i)
         {
         result = U_SYSCALL(SSL_CTX_use_PrivateKey_file, "%p,%S,%d", ctx, private_key_file, SSL_FILETYPE_PEM);

         if (result) break;

         unsigned long error = U_SYSCALL_NO_PARAM(ERR_peek_error);

         if (ERR_GET_REASON(error) == EVP_R_BAD_DECRYPT)
            {
            if (i < 2) // Give the user two tries
               {
               (void) U_SYSCALL_NO_PARAM(ERR_get_error); // remove from stack

               continue;
               }
            }

         U_RETURN(false);
         }

      // Check private key

      result = U_SYSCALL(SSL_CTX_check_private_key, "%p", ctx);

      if (result == 0) U_RETURN(false);

#  if !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
      UString str(private_key_file, u__strlen(private_key_file, __PRETTY_FUNCTION__));

      staple.pkey = UServices::loadKey(UFile::contentOf(str), "PEM", true, passwd, 0);

      U_INTERNAL_DUMP("staple.pkey = %p", staple.pkey)
#  endif
      }

   if (CAfile && *CAfile == '\0') CAfile = 0;
   if (CApath && *CApath == '\0') CApath = 0;

   if (CAfile ||
       CApath)
      {
      if (UServices::setupOpenSSLStore(CAfile, CApath, (verify_mode ? U_STORE_FLAGS : 0)) == false) U_RETURN(false);

      U_SYSCALL_VOID(SSL_CTX_set_cert_store, "%p,%p", ctx, UServices::store);

      // Sets the list of CA sent to the client when requesting a client certificate for ctx

      if (CAfile) // Process CA certificate bundle file
         {
         STACK_OF(X509_NAME)* list = (STACK_OF(X509_NAME)*) U_SYSCALL(SSL_load_client_CA_file, "%S", CAfile);

         U_SYSCALL_VOID(SSL_CTX_set_client_CA_list, "%p,%p", ctx, list);
         }
      }

   setVerifyCallback(UServices::X509Callback, verify_mode);

   USocket::setSSLActive(true);

   /**
    * see: https://wiki.mozilla.org/Security/Server_Side_TLS
    *
    * Three configurations are recommended. Pick the right configuration depending on your audience. If you do not need
    * backward compatibility, and are building a service for modern clients only (post FF27), then use the Modern configuration.
    * Otherwise, prefer the Intermediate configuration. Use the Old backward compatible configuration only if your service will
    * be accessed by very old clients, such as Windows XP IE6, or ancient libraries & bots.
    *
    * Modern       Firefox 27, Chrome 22, IE 11, Opera 14, Safari 7, Android 4.4, Java 8
    * Intermediate Firefox 1, Chrome 1, IE 7, Opera 5, Safari 1, Windows XP IE8, Android 2.3, Java 7
    * Old          Windows XP IE6, Java 6
    */

   const char* ciphersuite;
   int options = SSL_CTX_get_options(ctx);

   U_INTERNAL_DUMP("options = %d", options)

   switch (ciphersuite_model)
      {
      case Modern:
         {
         options |= SSL_OP_NO_SSLv2 |
                    SSL_OP_NO_SSLv3 |
                    SSL_OP_NO_TLSv1; 

         ciphersuite =
            "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:"
            "ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:"
            "ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:"
            "ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:"
            "DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:"
            "DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
         }
      break;

      case Old:
         {
         options |= SSL_OP_NO_SSLv2;

         ciphersuite =
            "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:"
            "ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:"
            "ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:"
            "ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:"
            "DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:"
            "DHE-RSA-AES256-SHA:ECDHE-RSA-DES-CBC3-SHA:ECDHE-ECDSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:"
            "AES128-SHA:AES256-SHA:AES:DES-CBC3-SHA:HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:"
            "!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
         }
      break;

      default: // Intermediate
         {
         options |= SSL_OP_NO_SSLv2 |
                    SSL_OP_NO_SSLv3;

         ciphersuite =
            "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:"
            "DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:"
            "ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:"
            "ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:"
            "DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA:AES256-SHA:AES:CAMELLIA:"
            "DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
         }
      break;
      }

   (void) U_SYSCALL(SSL_CTX_set_options, "%p,%d", ctx,
                     options |
#                 ifdef SSL_OP_NO_COMPRESSION
                     SSL_OP_NO_COMPRESSION |
#                 endif
                     SSL_OP_CIPHER_SERVER_PREFERENCE); // SSLHonorCipherOrder On - determine SSL cipher in server-preferred order, not client-order

   (void) U_SYSCALL(SSL_CTX_set_cipher_list, "%p,%S", ctx, ciphersuite);

#ifndef U_HTTP2_DISABLE
   U_SYSCALL_VOID(SSL_CTX_set_next_protos_advertised_cb, "%p,%p,%p", ctx, nextProto, (unsigned char*)"\x2h2\x5h2-16\x5h2-14");
# if OPENSSL_VERSION_NUMBER >= 0x10002000L
   U_SYSCALL_VOID(SSL_CTX_set_alpn_select_cb, "%p,%p,%p", ctx, selectProto, 0); // ALPN selection callback
# endif
#endif

   U_RETURN(true);
}

void USSLSocket::setStatus(SSL* _ssl, int _ret, bool _flag)
{
   U_TRACE(1, "USSLSocket::setStatus(%p,%d,%b)", _ssl, _ret, _flag)

   uint32_t sz;
   char buf[1024];
   long i1 = 0, i2;
   const char* descr  = "SSL_ERROR_NONE";
   const char* errstr = "ok";

   if (_ret != SSL_ERROR_NONE) // 0
      {
      if (_flag) _ret = U_SYSCALL(SSL_get_error, "%p,%d", _ssl, _ret);

      /* -------------------------------------
       * #define SSL_ERROR_NONE              0
       * #define SSL_ERROR_SSL               1
       * #define SSL_ERROR_WANT_READ         2
       * #define SSL_ERROR_WANT_WRITE        3
       * #define SSL_ERROR_WANT_X509_LOOKUP  4
       * #define SSL_ERROR_SYSCALL           5
       * #define SSL_ERROR_ZERO_RETURN       6
       * #define SSL_ERROR_WANT_CONNECT      7
       * #define SSL_ERROR_WANT_ACCEPT       8
       * -------------------------------------
       */

      switch (_ret)
         {
         case SSL_ERROR_SSL:
            {
            descr  = "SSL_ERROR_SSL";
            errstr = "SSL error";
            }
         break;

         case SSL_ERROR_SYSCALL:
            {
            descr  = "SSL_ERROR_SYSCALL";
            errstr = "SSL EOF observed that violates the protocol";
            }
         break;

         case SSL_ERROR_ZERO_RETURN:
            {
            descr  = "SSL_ERROR_ZERO_RETURN";
            errstr = "SSL connection closed by peer";
            }
         break;

         case SSL_ERROR_WANT_X509_LOOKUP:
            {
            descr  = "SSL_ERROR_WANT_X509_LOOKUP";
            errstr = "SSL operation didn't complete, the same function should be called again later";
            }
         break;

         case SSL_ERROR_WANT_READ:
            {
            descr  = "SSL_ERROR_WANT_READ";
            errstr = "SSL Read operation didn't complete, the same function should be called again later";
            }
         break;

         case SSL_ERROR_WANT_WRITE:
            {
            descr  = "SSL_ERROR_WANT_WRITE";
            errstr = "SSL Write operation didn't complete, the same function should be called again later";
            }
         break;

         case SSL_ERROR_WANT_CONNECT:
            {
            descr  = "SSL_ERROR_WANT_CONNECT";
            errstr = "SSL Connect operation didn't complete, the same function should be called again later";
            }
         break;

#     if defined(HAVE_OPENSSL_97) || defined(HAVE_OPENSSL_98)
         case SSL_ERROR_WANT_ACCEPT:
            {
            descr  = "SSL_ERROR_WANT_ACCEPT";
            errstr = "SSL Accept operation didn't complete, the same function should be called again later";
            }
         break;
#     endif
         }
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("(%d, %s) - %s"), _ret, descr, errstr);

   while ((i2 = ERR_get_error()))
      {
      if (i1 == i2) continue;

      (void) ERR_error_string_n(i1 = i2, buf, sizeof(buf));

      sz = u__strlen(buf, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("buf = %.*S", sz, buf)

      u_buffer_len += u__snprintf(u_buffer + u_buffer_len, U_BUFFER_SIZE - u_buffer_len, U_CONSTANT_TO_PARAM(" (%ld, %.*s)"), i2, sz, buf);
      }

   U_INTERNAL_DUMP("status = %.*S", u_buffer_len, u_buffer)
}

bool USSLSocket::secureConnection()
{
   U_TRACE_NO_PARAM(1, "USSLSocket::secureConnection()")

   int lerrno;

   if  (ssl)  (void) U_SYSCALL(SSL_clear, "%p", ssl); // reuse old
   else ssl = (SSL*) U_SYSCALL(SSL_new,   "%p", ctx);

   // When beginning a new handshake, the SSL engine must know whether it must call the connect (client) or accept (server) routines.
   // Even though it may be clear from the method chosen, whether client or server mode was requested, the handshake routines must be explicitly set.
   //
   // U_SYSCALL_VOID(SSL_set_connect_state, "%p", ssl); // init SSL client session

   ret = 0;

   if (U_SYSCALL(SSL_set_fd,  "%p,%d", ssl, getFd())) // get SSL to use our socket
      {
      lerrno = 0;

      /**
       * Enable SNI for backend requests. Make sure we don't do it for pure SSLv3 connections, and also
       * prevent IP addresses from being included in the SNI extension. (OpenSSL would simply pass them
       * on, but RFC 6066 is quite clear on this: "Literal IPv4 and IPv6 addresses are not permitted")
       */

#  if !defined(OPENSSL_NO_TLSEXT) && defined(SSL_set_tlsext_host_name)
      (void) SSL_set_tlsext_host_name(ssl,cRemoteAddress.strHostName.data());
#  endif

loop:
      errno = 0;
      ret   = U_SYSCALL(SSL_connect, "%p", ssl); // get SSL to handshake with server

      if (ret == 1)
         {
         USocket::iState = CONNECT;

#     ifdef _MSWINDOWS_
         USocket::fh = (SOCKET)_get_osfhandle(USocket::iSockDesc);
#     endif

         USocket::setSSLActive(true);

         U_RETURN(true);
         }

      U_INTERNAL_DUMP("errno = %d", errno)

      if (errno) lerrno = errno;

      ret = U_SYSCALL(SSL_get_error, "%p,%d", ssl, ret);

#  ifdef DEBUG
      dumpStatus(false);
#  endif

           if (ret == SSL_ERROR_WANT_READ)  { if (UNotifier::waitForRead( USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }
      else if (ret == SSL_ERROR_WANT_WRITE) { if (UNotifier::waitForWrite(USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }

      errno = lerrno;
      }

   USocket::setSSLActive(false);

   U_RETURN(false);
}

// server side RE-NEGOTIATE asking for client cert

bool USSLSocket::askForClientCertificate()
{
   U_TRACE_NO_PARAM(1, "USSLSocket::askForClientCertificate()")

   /**
    * SSL_VERIFY_CLIENT_ONCE
    * -------------------------------------------------------------------------------------
    * Client mode: ignored
    * Server mode: only request a client certificate on the initial TLS/SSL handshake.
    *              Do not ask for a client certificate again in case of a renegotiation.
    *              This flag must be used together with SSL_VERIFY_PEER
    * -------------------------------------------------------------------------------------
    * The only difference between the calls is that SSL_CTX_set_verify() sets the verification
    * mode for all SSL objects derived from a given SSL_CTX as long as they are created
    * after SSL_CTX_set_verify() is called, whereas SSL_set_verify() only affects the SSL
    * object that it is called on
    */

   U_SYSCALL_VOID(SSL_set_verify, "%p,%d,%p", ssl, SSL_VERIFY_PEER_STRICT, 0); // | SSL_VERIFY_CLIENT_ONCE

   // Stop the client from just resuming the un-authenticated session

   (void) U_SYSCALL(SSL_set_session_id_context, "%p,%p,%u", ssl, (const unsigned char*)this, sizeof(void*));

   ret = U_SYSCALL(SSL_renegotiate, "%p", ssl);

   if (ret != 1)
      {
#  ifdef DEBUG
      dumpStatus(true);
#  endif

      U_RETURN(false);
      }

   ret = U_SYSCALL(SSL_do_handshake, "%p", ssl);

   if (ret != 1)
      {
#  ifdef DEBUG
      dumpStatus(true);
#  endif

      U_RETURN(false);
      }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   ssl->state = SSL_ST_ACCEPT;
#endif

   ret = U_SYSCALL(SSL_do_handshake, "%p", ssl);

   if (ret != 1)
      {
#  ifdef DEBUG
      dumpStatus(true);
#  endif

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool USSLSocket::acceptSSL(USSLSocket* pcNewConnection)
{
   U_TRACE(1+256, "USSLSocket::acceptSSL(%p)", pcNewConnection)

   int fd         = pcNewConnection->iSockDesc;
   uint32_t count = 0;

   U_DUMP("fd = %d isBlocking() = %b", fd, pcNewConnection->isBlocking())

   U_INTERNAL_ASSERT_EQUALS(ssl, 0)

   ssl = (SSL*) U_SYSCALL(SSL_new, "%p", ctx);

   // --------------------------------------------------------------------------------------------------
   // When beginning a new handshake, the SSL engine must know whether it must call the connect (client)
   // or accept (server) routines. Even though it may be clear from the method chosen, whether client or
   // server mode was requested, the handshake routines must be explicitly set
   // --------------------------------------------------------------------------------------------------
   // U_SYSCALL_VOID(SSL_set_accept_state, "%p", ssl); // init SSL server session
   // --------------------------------------------------------------------------------------------------

   (void) U_SYSCALL(SSL_set_fd, "%p,%d", ssl, fd); // get SSL to use our socket

loop:
   errno = 0;
   ret   = U_SYSCALL(SSL_accept, "%p", ssl); // get SSL handshake with client

   if (ret == 1)
      {
      SSL_set_app_data(ssl, pcNewConnection);

      pcNewConnection->ssl            = ssl;
      pcNewConnection->ret            = SSL_ERROR_NONE;
      pcNewConnection->iState         = CONNECT;
      pcNewConnection->renegotiations = 0;

      ssl = 0;

      U_RETURN(true);
      }

   U_INTERNAL_DUMP("errno = %d", errno)

   if (errno) pcNewConnection->iState = -errno;

   pcNewConnection->ret = U_SYSCALL(SSL_get_error, "%p,%d", ssl, ret);

#ifdef DEBUG
   dumpStatus(pcNewConnection->ret, false);
#endif

   U_INTERNAL_DUMP("count = %u", count)

   if (count++ < 5)
      {
           if (pcNewConnection->ret == SSL_ERROR_WANT_READ)  { if (UNotifier::waitForRead( fd, U_SSL_TIMEOUT_MS) > 0) goto loop; }
      else if (pcNewConnection->ret == SSL_ERROR_WANT_WRITE) { if (UNotifier::waitForWrite(fd, U_SSL_TIMEOUT_MS) > 0) goto loop; }
      }

   errno = -pcNewConnection->iState;

   U_SYSCALL_VOID(SSL_free, "%p", ssl);
                                  ssl = 0;

   pcNewConnection->USocket::_closesocket();

   U_INTERNAL_DUMP("pcNewConnection->ret = %d", pcNewConnection->ret)

   U_RETURN(false);
}

#ifdef closesocket
#undef closesocket
#endif

void USSLSocket::closesocket()
{
   U_TRACE_NO_PARAM(1, "USSLSocket::closesocket()")

   if (ssl)
      {
      U_INTERNAL_DUMP("isTimeout() = %b", USocket::isTimeout())

      int mode = SSL_SENT_SHUTDOWN     |
                 SSL_RECEIVED_SHUTDOWN |
                 (USocket::isTimeout() ? 0
                                       : SSL_get_shutdown(ssl));

      U_SYSCALL_VOID(SSL_set_shutdown,       "%p,%d", ssl, mode);
      U_SYSCALL_VOID(SSL_set_quiet_shutdown, "%p,%d", ssl, 1);

loop: ret = U_SYSCALL(SSL_shutdown, "%p", ssl); // Send SSL shutdown signal to peer

      if (ret <= 0)
         {
         ret = U_SYSCALL(SSL_get_error, "%p,%d", ssl, ret);

#     ifdef DEBUG
         dumpStatus(false);
#     endif

              if (ret == SSL_ERROR_WANT_READ)  { if (UNotifier::waitForRead( USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }
         else if (ret == SSL_ERROR_WANT_WRITE) { if (UNotifier::waitForWrite(USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }
         }

      U_SYSCALL_VOID(SSL_free, "%p", ssl);
                                     ssl = 0;                
      }

   USocket::setSSLActive(false);
}

// VIRTUAL METHOD

bool USSLSocket::connectServer(const UString& server, unsigned int iServPort, int timeoutMS)
{
   U_TRACE(0, "USSLSocket::connectServer(%V,%u,%d)", server.rep, iServPort, timeoutMS)

   if (USocket::connectServer(server, iServPort, timeoutMS) &&
       (USocket::isSSLActive() == false                     ||
        secureConnection()))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

int USSLSocket::recv(void* pBuffer, uint32_t iBufferLen)
{
   U_TRACE(1, "USSLSocket::recv(%p,%u)", pBuffer, iBufferLen)

   U_ASSERT(USocket::isSSL())
   U_INTERNAL_ASSERT(USocket::isConnected())

   int iBytesRead, lerrno;

   if (USocket::isSSLActive() == false)
      {
      iBytesRead = USocket::recv(pBuffer, iBufferLen);

      goto end;
      }

   U_INTERNAL_ASSERT_POINTER(ssl)

   lerrno     = 0;
loop:
   errno      = 0;
   iBytesRead = U_SYSCALL(SSL_read, "%p,%p,%d", ssl, CAST(pBuffer), iBufferLen);

   U_INTERNAL_DUMP("renegotiations = %d", renegotiations)

   if (renegotiations > 1)
      {
      U_WARNING("SSL: renegotiation initiated by client");

      while (ERR_peek_error())
         {
         U_WARNING("SSL: ignoring stale global SSL error");
         }

      ERR_clear_error();

      U_RETURN(-1);
      }

   if (iBytesRead > 0)
      {
      U_INTERNAL_DUMP("BytesRead(%d) = %#.*S", iBytesRead, iBytesRead, CAST(pBuffer))

      goto end;
      }

   U_INTERNAL_DUMP("errno = %d", errno)

   if (errno) lerrno = errno;

   ret = U_SYSCALL(SSL_get_error, "%p,%d", ssl, iBytesRead);

#ifdef DEBUG
   dumpStatus(false);
#endif

        if (ret == SSL_ERROR_WANT_WRITE) { if (UNotifier::waitForWrite(USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }
// else if (ret == SSL_ERROR_WANT_READ)  { if (UNotifier::waitForRead( USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }

   errno = lerrno;

end:
   U_RETURN(iBytesRead);
}

int USSLSocket::send(const char* pData, uint32_t iDataLen)
{
   U_TRACE(1, "USSLSocket::send(%p,%u)", pData, iDataLen)

   U_ASSERT(USocket::isSSL())
   U_INTERNAL_ASSERT(USocket::isOpen())

   int iBytesWrite, lerrno;

   if (USocket::isSSLActive() == false)
      {
      iBytesWrite = USocket::send(pData, iDataLen);

      goto end;
      }

   U_INTERNAL_ASSERT_POINTER(ssl)

   lerrno      = 0;
loop:
   errno       = 0;
   iBytesWrite = U_SYSCALL(SSL_write, "%p,%p,%d", ssl, CAST(pData), iDataLen);

   if (iBytesWrite > 0)
      {
      U_INTERNAL_DUMP("BytesWrite(%d) = %#.*S", iBytesWrite, iBytesWrite, CAST(pData))

      goto end;
      }

   U_INTERNAL_DUMP("errno = %d", errno)

   if (errno) lerrno = errno;

   ret = U_SYSCALL(SSL_get_error, "%p,%d", ssl, iBytesWrite);

#ifdef DEBUG
   dumpStatus(false);
#endif

        if (ret == SSL_ERROR_WANT_READ)  { if (UNotifier::waitForRead( USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }
   else if (ret == SSL_ERROR_WANT_WRITE) { if (UNotifier::waitForWrite(USocket::iSockDesc, U_SSL_TIMEOUT_MS) > 0) goto loop; }

   errno = lerrno;

end:
   U_RETURN(iBytesWrite);
}

#if !defined(OPENSSL_NO_TLSEXT) && defined(SSL_set_tlsext_host_name)

// This callback function is executed when OpenSSL encounters an extended
// client hello with a server name indication extension ("SNI", cf. RFC 6066)

int USSLSocket::callback_ServerNameIndication(SSL* _ssl, int* alert, void* data)
{
   U_TRACE(1, "USSLSocket::callback_ServerNameIndication(%p,%p,%p)", _ssl, alert, data)

   U_INTERNAL_ASSERT_POINTER(sctx)

   const char* servername = (const char*) U_SYSCALL(SSL_get_servername, "%p,%d", _ssl, TLSEXT_NAMETYPE_host_name);

   if (servername == 0)
      {
   // U_DEBUG("SSL: server name not provided via TLS extension");

      U_RETURN(SSL_TLSEXT_ERR_OK);
      }

   // TODO: check and set SSL_CTX (if matched)

   U_RETURN(SSL_TLSEXT_ERR_OK);

   /**
    * RFC 6066 section 3 says:
    *
    * "It is NOT RECOMMENDED to send a warning-level unrecognized_name(112) alert,
    * because the client's behavior in response to warning-level alerts is unpredictable"
    *
    * We no send any alert (neither warning- nor fatal-level), i.e. we take the second action suggested in RFC 6066:
    * "If the server understood the ClientHello extension but does not recognize the server name, the server SHOULD take
    * one of two actions: either abort the handshake by sending a fatal-level unrecognized_name(112) alert or continue
    * the handshake"

    U_DEBUG("SSL: no matching SSL virtual host for servername %s found", servername);

    U_RETURN(SSL_TLSEXT_ERR_NOACK);
    */
}
#endif

/**
 * OCSP stapling is a way for a SSL server to obtain OCSP responses for his own certificate, and provide them to the client,
 * under the assumption that the client may need them. This makes the whole process more efficient: the client does not have
 * to open extra connections to get the OCSP responses itself, and the same OCSP response can be sent by the server to all
 * clients within a given time frame. One way to see it is that the SSL server acts as a Web proxy for the purpose of
 * downloading OCSP responses
 */

#if !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
bool USSLSocket::setDataForStapling()
{
   U_TRACE_NO_PARAM(1, "USSLSocket::setDataForStapling()")

   U_INTERNAL_ASSERT_POINTER(sctx)

   if (staple.cert == 0) U_RETURN(false);

   char* s;
   bool result = false;
   STACK_OF(X509)* chain;
   X509_STORE_CTX* store_ctx;
   STACK_OF(OPENSSL_STRING)* aia;

#if OPENSSL_VERSION_NUMBER >= 0x10001000L
   SSL_CTX_get_extra_chain_certs(sctx, &chain);
#else
   chain = sctx->extra_certs;
#endif

   int n = U_SYSCALL(sk_X509_num, "%p", chain);

   for (int i = 0; i < n; ++i)
      {
      staple.issuer = sk_X509_value(chain, i);

      if (U_SYSCALL(X509_check_issued, "%p,%p", staple.issuer, staple.cert) == X509_V_OK)
         {
#     if OPENSSL_VERSION_NUMBER < 0x10100000L
         CRYPTO_add(&(staple.issuer->references), 1, CRYPTO_LOCK_X509);
#     endif

         goto next;
         }
      }

   // initialize an X509 STORE context

   store_ctx = (X509_STORE_CTX*) U_SYSCALL_NO_PARAM(X509_STORE_CTX_new); // create an X509 store context

   if (store_ctx == 0) U_RETURN(false);

#ifdef HAVE_OPENSSL_97
   (void) U_SYSCALL(X509_STORE_CTX_init, "%p,%p,%p,%p", store_ctx, UServices::store, 0, 0);
#else
   U_SYSCALL_VOID(  X509_STORE_CTX_init, "%p,%p,%p,%p", store_ctx, UServices::store, 0, 0);
#endif

   n = U_SYSCALL(X509_STORE_CTX_get1_issuer, "%p,%p,%p", &staple.issuer, store_ctx, staple.cert);

   U_SYSCALL_VOID(X509_STORE_CTX_free, "%p", store_ctx);

   /**
    * Return values of X509_STORE_CTX_get1_issuer() are:
    *
    *  1 lookup successful
    *  0 certificate not found
    * -1 some other error
    */

   if (n <= 0)
      {
      if (n != -1) U_WARNING("SSL: OCSP stapling ignored, issuer certificate not found...");

      U_RETURN(false);
      }

next: // extract OCSP responder URL from certificate

   aia = (STACK_OF(OPENSSL_STRING)*) U_SYSCALL(X509_get1_ocsp, "%p", staple.cert);

   if (aia == 0) U_RETURN(false);

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
   s = sk_OPENSSL_STRING_value(aia, 0);
#else
   s = sk_value(aia, 0);
#endif

   if (s)
      {
      int len = u__strlen(s, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("url = %.*S", len, s) // "http://portal.actalis.it/VA/AUTH-G2"

      if (u_isURL(s, len))
         {
         int ssl = 0;
         char* host = 0;
         char* port = 0;

         U_INTERNAL_ASSERT_EQUALS(memcmp(s,"http://",7), 0)

         s[len] = 0;

         if (U_SYSCALL(OCSP_parse_url, "%S,%p,%p,%p,%p", s, &host, &port, &staple.path, &ssl))
            {
            U_INTERNAL_ASSERT_POINTER(port)
            U_INTERNAL_ASSERT_POINTER(host)
            U_INTERNAL_ASSERT_EQUALS(ssl, 0)

            U_SYSCALL_VOID(OPENSSL_free, "%p", port);
            U_SYSCALL_VOID(OPENSSL_free, "%p", host);

            staple.req = OCSP_REQUEST_new();

            staple.id = (OCSP_CERTID*) U_SYSCALL(OCSP_cert_to_id, "%p,%p,%p", 0, staple.cert, staple.issuer);

            U_NEW(UString, staple.url, UString((void*)s, len));

            (void) U_SYSCALL(OCSP_request_add0_id, "%p,%p", staple.req, staple.id);

            (void) U_SYSCALL(OCSP_request_add1_nonce, "%p,%p,%d", staple.req, 0, -1);

            // sign the request

            if (staple.pkey) (void) U_SYSCALL(OCSP_request_sign, "%p,%p,%p,%p,%p,%ld", staple.req, staple.cert, staple.pkey, EVP_sha1(), 0, 0);

            SSL_CTX_set_tlsext_status_cb(sctx, USSLSocket::certificate_status_callback);

            result = true;
            }
         }
      }

   U_SYSCALL_VOID(X509_email_free, "%p", aia);

   if (staple.cert)
      {
      U_SYSCALL_VOID(X509_free,"%p", staple.cert);
                                     staple.cert = 0;
      }

   if (staple.issuer)
      {
      U_SYSCALL_VOID(X509_free, "%p", staple.issuer);
                                      staple.issuer = 0;
      }

   if (staple.pkey)
      {
      U_SYSCALL_VOID(EVP_PKEY_free, "%p", staple.pkey);
                                          staple.pkey = 0;
      }

   U_RETURN(result);
}

bool USSLSocket::doStapling()
{
   U_TRACE_NO_PARAM(1, "USSLSocket::doStapling()")

   U_INTERNAL_ASSERT_POINTER(sctx)
   U_INTERNAL_ASSERT_POINTER(staple.id)
   U_INTERNAL_ASSERT_POINTER(staple.req)
   U_INTERNAL_ASSERT_POINTER(staple.path)
   U_INTERNAL_ASSERT_POINTER(staple.client)

   bool result = false;
   OCSP_RESPONSE* resp = 0;
   OCSP_BASICRESP* basic = 0;

   if (staple.client->connect())
      {
      unsigned char* p;
      BIO* conn = (BIO*) U_SYSCALL(BIO_new_fd, "%d,%d", staple.client->getFd(), BIO_NOCLOSE);

      // send the request and get a response

      resp = (OCSP_RESPONSE*) U_SYSCALL(OCSP_sendreq_bio, "%p,%S,%p", conn, staple.path, staple.req);

      U_SYSCALL_VOID(BIO_free_all, "%p", conn);

      if (resp)
         {
         UString nextupdate_str;
         ASN1_GENERALIZEDTIME* thisupdate;
         ASN1_GENERALIZEDTIME* nextupdate;

         int status, rc = U_SYSCALL(OCSP_response_status, "%p", resp);

         result = (rc == OCSP_RESPONSE_STATUS_SUCCESSFUL);

#     ifdef DEBUG
         const char* descr  = 0;
         const char* errstr = 0;

         switch (rc)
            {
            case OCSP_RESPONSE_STATUS_SUCCESSFUL:
               {
               descr  = "OCSP_RESPONSE_STATUS_SUCCESSFUL";
               errstr = "successful";
               }
            break;

            case OCSP_RESPONSE_STATUS_MALFORMEDREQUEST:
               {
               descr  = "OCSP_RESPONSE_STATUS_MALFORMEDREQUEST";
               errstr = "malformedrequest";
               }
            break;

            case OCSP_RESPONSE_STATUS_INTERNALERROR: 
               {
               descr  = "OCSP_RESPONSE_STATUS_INTERNALERROR";
               errstr = "internalerror";
               }
            break;

            case OCSP_RESPONSE_STATUS_TRYLATER: 
               {
               descr  = "OCSP_RESPONSE_STATUS_TRYLATER";
               errstr = "trylater";
               }
            break;

            case OCSP_RESPONSE_STATUS_SIGREQUIRED: 
               {
               descr  = "OCSP_RESPONSE_STATUS_SIGREQUIRED";
               errstr = "sigrequired";
               }
            break;

            case OCSP_RESPONSE_STATUS_UNAUTHORIZED: // 6
               {
               descr  = "OCSP_RESPONSE_STATUS_UNAUTHORIZED";
               errstr = "unauthorized";
               }
            break;
            }

         U_INTERNAL_DUMP("OCSP_response_status() - %b = (%d, %s) - %s", result, rc, descr, errstr)
#     endif

         if (result == false) goto end;

         // verify the response

         basic = (OCSP_BASICRESP*) U_SYSCALL(OCSP_response_get1_basic, "%p", resp);

         result = (basic && U_SYSCALL(OCSP_check_nonce, "%p,%p", staple.req, basic) > 0);

         if (result == false) goto end;

         // verify signature

         result = (U_SYSCALL(OCSP_basic_verify, "%p,%p,%p,%lu", basic, 0, UServices::store, staple.verify ? OCSP_TRUSTOTHER : OCSP_NOVERIFY) == 1);

         if (result == false) goto end;

         result = (U_SYSCALL(OCSP_resp_find_status, "%p,%p,%p,%lu", basic, staple.id, &status, 0, 0, &thisupdate, &nextupdate) == 1);

         nextupdate_str = UStringExt::ASN1TimetoString(nextupdate);

         U_INTERNAL_DUMP("OCSP_resp_find_status() - %d: %s This update: %s Next update: %v", status,
                          OCSP_cert_status_str(status), UStringExt::ASN1TimetoString(thisupdate).data(), nextupdate_str.rep)

         if (result == false ||
             status != V_OCSP_CERTSTATUS_GOOD)
            {
            goto end;
            }

         // check if the response is valid for at least six minutes

         result = (U_SYSCALL(OCSP_check_validity, "%p,%p,%p,%lu", thisupdate, nextupdate, 360, -1) == 1);

         if (result == false) goto end;

         staple.valid = u_now->tv_sec + UTimeDate::getSecondFromTime(nextupdate_str.data(), true);

         U_INTERNAL_DUMP("staple.valid = %ld now = %ld", staple.valid, u_now->tv_sec)

         p = (unsigned char*) staple.data;

#     if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
         UServer_Base::lock_ocsp_staple->lock();
#     endif

         staple.len = i2d_OCSP_RESPONSE(resp, &p);

#     if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
         UServer_Base::lock_ocsp_staple->unlock();
#     endif

         U_INTERNAL_DUMP("staple.data(%d) = %p %#.*S", staple.len, staple.data, staple.len, staple.data)

         U_INTERNAL_ASSERT_MINOR(staple.len, U_OCSP_MAX_RESPONSE_SIZE)
         }
      }

end:
   if (resp)  OCSP_RESPONSE_free(resp);
   if (basic) OCSP_BASICRESP_free(basic);

   if (staple.client->isOpen()) staple.client->close();

   U_RETURN(result);
}

void USSLSocket::certificate_status_callback(SSL* _ssl, void* data)
{
   U_TRACE(0, "USSLSocket::certificate_status_callback(%p,%p)", _ssl, data)

   U_INTERNAL_ASSERT_POINTER(sctx)

   U_INTERNAL_DUMP("staple.data(%d) = %p %#.*S", staple.len, staple.data, staple.len, staple.data)

   if (staple.len &&
       staple.valid < u_now->tv_sec)
      {
      unsigned char* p;

      U_INTERNAL_ASSERT_MINOR(staple.len, U_OCSP_MAX_RESPONSE_SIZE)

#  if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
      UServer_Base::lock_ocsp_staple->lock();
#  endif

      // we have to copy ocsp response as OpenSSL will free it by itself

      p = (unsigned char*) OPENSSL_malloc(staple.len);

      U_MEMCPY(p, staple.data, staple.len);

#  if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
      UServer_Base::lock_ocsp_staple->unlock();
#  endif

      SSL_set_tlsext_status_ocsp_resp(_ssl, p, staple.len);
      }
}

void USSLSocket::cleanupStapling()
{
   U_TRACE_NO_PARAM(1, "USSLSocket::cleanupStapling()")

   if (staple.id)  OCSP_CERTID_free(staple.id);
// if (staple.req) OCSP_REQUEST_free(staple.req);

   if (staple.url)    delete staple.url;
   if (staple.client) delete staple.client;

   if (staple.cert)   U_SYSCALL_VOID(X509_free,     "%p", staple.cert);
   if (staple.issuer) U_SYSCALL_VOID(X509_free,     "%p", staple.issuer);
   if (staple.path)   U_SYSCALL_VOID(OPENSSL_free,  "%p", staple.path);
   if (staple.pkey)   U_SYSCALL_VOID(EVP_PKEY_free, "%p", staple.pkey);

   staple.id     = 0;
   staple.req    = 0;
   staple.url    = 0;
   staple.cert   = 0;
   staple.path   = 0;
   staple.pkey   = 0;
   staple.issuer = 0;
   staple.client = 0;
}
#endif

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USSLSocket::dump(bool reset) const
{
   USocket::dump(false);

   *UObjectIO::os << '\n'
                  << "ret                           " << ret            << '\n'
                  << "ssl                           " << (void*)ssl     << '\n'
                  << "ctx                           " << (void*)ctx     << '\n'
                  << "renegotiations                " << renegotiations << '\n'
                  << "ciphersuite_model             " << ciphersuite_model;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
