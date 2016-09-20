// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ssl_session.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/rdb.h>
#include <ulib/utility/uhttp.h>
#include <ulib/ssl/net/ssl_session.h>

/**
 * Forward secrecy
 *
 * You should consider forward secrecy. Forward secrecy means that the keys for a connection aren't stored on disk.
 * You might have limited the amount of information that you log in order to protect the privacy of your users, but
 * if you don't have forward secrecy then your private key is capable of decrypting all past connections. Someone else
 * might be doing the logging for you. In order to enable forward secrecy you need to have DHE or ECDHE ciphersuites as
 * your top preference. DHE ciphersuites are somewhat expensive if you're terminating lots of SSL connections and you
 * should be aware that your server will probably only allow 1024-bit DHE. I think 1024-bit DHE-RSA is preferable to
 * 2048-bit RSA, but opinions vary. If you're using ECDHE, use P-256. You also need to be aware of Session Tickets in
 * order to implement forward secrecy correctly. There are two ways to resume a TLS connection: either the server chooses
 * a random number and both sides store the session information, of the server can encrypt the session information with a
 * secret, local key and send that to the client. The former is called Session IDs and the latter is called Session Tickets.
 * But Session Tickets are transmitted over the wire and so the server's Session Ticket encryption key is capable of decrypting
 * past connections. Most servers will generate a random Session Ticket key at startup unless otherwise configured, but you should check
 */

SSL_SESSION* USSLSession::sess;

// define method VIRTUAL of class UDataStorage

char* USSLSession::toBuffer()
{
   U_TRACE_NO_PARAM(0, "USSLSession::toBuffer()")

   U_INTERNAL_ASSERT_POINTER(sess)

   // converts SSL_SESSION object to ASN1 representation

   unsigned char* p = (unsigned char*)u_buffer;

   u_buffer_len = U_SYSCALL(i2d_SSL_SESSION, "%p,%p", sess, &p);

   U_INTERNAL_ASSERT_MAJOR(u_buffer_len, 0)

   buffer_len = u_buffer_len;

   U_RETURN(u_buffer);
}

void USSLSession::fromData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "USSLSession::fromData(%.*S,%u)", len, ptr, len)

   U_INTERNAL_ASSERT_POINTER(ptr)

   // converts SSL_SESSION object from ASN1 representation

#ifdef HAVE_OPENSSL_97
         unsigned char* p =       (unsigned char*)ptr;
#else
   const unsigned char* p = (const unsigned char*)ptr;
#endif

   sess = (SSL_SESSION*) U_SYSCALL(d2i_SSL_SESSION, "%p,%p,%ld", 0, &p, (long)len);
}

int USSLSession::newSession(SSL* ssl, SSL_SESSION* _sess)
{
   U_TRACE(0, "USSLSession::newSession(%p,%p)", ssl, _sess)

/*
#ifdef DEBUG
   static FILE* fp = (FILE*) U_SYSCALL(fopen, "%S,%S", "/tmp/ssl_session.new", "a");
   if (fp) (void) U_SYSCALL(SSL_SESSION_print_fp, "%p,%p", fp, _sess);
#endif
*/

   sess = _sess;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   UHTTP::db_session_ssl->insertDataStorage((const char*)sess->session_id, sess->session_id_length);
#else
   unsigned int idlen;
   const unsigned char* id = (const unsigned char*) U_SYSCALL(SSL_SESSION_get_id, "", sess, &idlen);

   UHTTP::db_session_ssl->insertDataStorage((const char*)id, idlen);
#endif

   U_RETURN(0);
}

SSL_SESSION* USSLSession::getSession(SSL* ssl, unsigned char* id, int len, int* copy)
{
   U_TRACE(0, "USSLSession::getSession(%p,%.*S,%d,%p)", ssl, len, id, len, copy)

   sess  = 0;
   *copy = 0;

   UHTTP::db_session_ssl->getDataStorage((const char*)id, (uint32_t)len);

/*
#ifdef DEBUG
   static FILE* fp = (FILE*) U_SYSCALL(fopen, "%S,%S", "/tmp/ssl_session.get", "a");
   if (fp) (void) U_SYSCALL(SSL_SESSION_print_fp, "%p,%p", fp, sess);
#endif
*/

   U_RETURN_POINTER(sess, SSL_SESSION);
}

void USSLSession::removeSession(SSL_CTX* ctx, SSL_SESSION* _sess)
{
   U_TRACE(0, "USSLSession::removeSession(%p,%p)", ctx, _sess)

/*
#ifdef DEBUG
   static FILE* fp = (FILE*) U_SYSCALL(fopen, "%S,%S", "/tmp/ssl_session.del", "a");
   if (fp) (void) U_SYSCALL(SSL_SESSION_print_fp, "%p,%p", fp, _sess);
#endif
*/

   U_INTERNAL_ASSERT_POINTER(UHTTP::db_session_ssl)

   int result;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   result = UHTTP::db_session_ssl->remove((const char*)_sess->session_id, (uint32_t)_sess->session_id_length);
#else
   unsigned int idlen;
   const unsigned char* id = (const unsigned char*) U_SYSCALL(SSL_SESSION_get_id, "", sess, &idlen);

   result = UHTTP::db_session_ssl->remove((const char*)id, (uint32_t)idlen);
#endif

   // -2: The entry was already marked deleted in the hash-tree

   if (result &&
       result != -2)
      {
      U_WARNING("Remove of SSL session on db failed with error %d", result);
      }
}
