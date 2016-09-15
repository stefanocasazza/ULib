// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    pop3.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_POP3_CLIENT_H
#define U_POP3_CLIENT_H 1

#include <ulib/internal/common.h>

#ifdef USE_LIBSSL
#  include <ulib/ssl/net/sslsocket.h>
#  define Socket USSLSocket
#else
#  include <ulib/net/tcpsocket.h>
#  define Socket UTCPSocket
#endif

#include <ulib/container/vector.h>

/**
 * @class UPop3Client
 *
 * @brief Creates and manages a client connection with a remote POP3 server.
 *
 * A POP3 session progresses through a number of states during its lifetime. Once the TCP connection has been opened
 * and the POP3 server has sent the greeting, the session enters the AUTHORIZATION state. In this state, the client
 * must identify itself to the POP3 server. Once the client has successfully done this, the server acquires resources
 * associated with the client's maildrop, and the session enters the TRANSACTION state. In this state, the client
 * requests actions on the part of the POP3 server. When the client has issued the QUIT command, the session enters
 * the UPDATE state. In this state, the POP3 server releases any resources acquired during the TRANSACTION state and
 * says goodbye. The TCP connection is then closed.
 *
 * Example of pop3 session:
 *
 * 1. Establish the connection
 * 2. Read greeting from remote pop3 (expect +OK POP3 server ready <1896.697170952@dbc.mtview.ca.us>)
 * 3. Identify with the POP3 server
 *    send "APOP mrose c4c9334bac560ecc979e58001b3e22fb" (expect +OK mrose's maildrop has 2 messages (320 octets))
 * 3. Query the POP3 server
 *    send "STAT"     (expect +OK 2 320)
 *    send "LIST"     (expect +OK 2 messages (320 octets)  1 120 2 200 .)
 * 4. Request actions
 *    send "RETR 1"   (expect +OK 120 octets <the POP3 server sends message 1> .)
 *    send "DELE 1"   (expect +OK message 1 deleted)
 *    send "RETR 2"   (expect +OK 200 octets <the POP3 server sends message 2> .)
 *    send "DELE 2"   (expect +OK message 2 deleted)
 * 5. Quit
 *    send "QUIT"     (expect +OK dewey POP3 server signing off (maildrop empty))
 *
 * Here is a summary of current POP3 commands and responses:
 *
 * Minimal POP3 Commands:
 *  USER name           valid in the AUTHORIZATION state
 *  PASS string
 *  QUIT
 *  STAT                valid in the TRANSACTION state
 *  LIST [msg]
 *  RETR msg
 *  DELE msg
 *  NOOP
 *  RSET
 *  QUIT
 *
 * Optional POP3 Commands:
 *  APOP name digest    valid in the AUTHORIZATION state
 *  AUTH [auth type]
 *  TOP msg n           valid in the TRANSACTION state
 *  UIDL [msg]
 *
 * POP3 Replies:
 *  +OK
 *  -ERR
 *
 * Extended Commands:
 *  CAPA                valid in AUTHORIZATION or TRANSACTION state
 *  STLS                valid in AUTHORIZATION state
 *
 * Extended Response Codes: (These will be enclosed in square brackets following an -ERR reply)
 *  LOGIN-DELAY
 *  IN-USE
 *  SYS/TEMP
 *  SYS/PERM
 *  AUTH
 */

class U_EXPORT UPop3Client : public Socket {
public:

   enum POP3ClientStatus {
      INIT,          // not logged in yet
      AUTHORIZATION, // recv the greeting
      TRANSACTION,   // identify successfully
      UPDATE         // sent QUIT
      };

   enum POP3ServerStatus {
      OK,
      BAD_STATE,
      UNAUTHORIZED,
      CANT_LIST,
      NO_SUCH_MESSAGE,
      CAPA_NOT_SUPPORTED,
      STLS_NOT_SUPPORTED,
      UIDL_NOT_SUPPORTED
      };

   /**
    * Constructs a new UPop3Client with default values for all properties
    */

   UPop3Client(bool bSocketIsIPv6 = false) : Socket(bSocketIsIPv6), buffer(4000)
      {
      U_TRACE_REGISTER_OBJECT(0, UPop3Client, "%b", bSocketIsIPv6)

      state   = INIT;
      num_msg = -1;
      }

   virtual ~UPop3Client()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPop3Client)
      }

   /**
    * Function called to established a socket connection with the POP3 network server
    */

   bool _connectServer(const UString& server, unsigned int port = 110, int timeout = U_TIMEOUT_MS);

   bool startTLS();

   /**
    * Capabilities
    */

   int getCapabilities(UVector<UString>& vec);

   bool isSTLS(UVector<UString>& v) const       { return (v.find(U_STRING_FROM_CONSTANT("STLS"))       != U_NOT_FOUND); }
   bool isUIDL(UVector<UString>& v) const       { return (v.find(U_STRING_FROM_CONSTANT("UIDL"))       != U_NOT_FOUND); }
   bool isPIPELINING(UVector<UString>& v) const { return (v.find(U_STRING_FROM_CONSTANT("PIPELINING")) != U_NOT_FOUND); }

   /**
    * Sends a login request to the remote POP3 server
    *
    * Note that the user name and password are sent over the network in plain text,
    * so it is not a good idea to use POP3 authentication with sensitive data or passwords
    *
    * @param user     the userid
    * @param password the password
    */

   bool login(const char* user, const char* passwd);

   int getSizeMessage(uint32_t n);
   int getUIDL(UVector<UString>& vec);

   int getNumMessage() { getSizeMessage(0); return num_msg; }

   // Execute an pop3 session

   UString getHeader( uint32_t n);
   UString getMessage(uint32_t n);

   bool reset();
   bool deleteMessage(uint32_t n);

   // PIPELINING

   bool deleteAllMessage();
   int  getAllHeader(UVector<UString>& vec);
   int  getAllMessage(UVector<UString>& vec);

   // Quit an pop3 session

   bool quit();

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer, capa;
   POP3ClientStatus state;
   int pos, end, response, num_msg;

private:
   void setStatus() U_NO_EXPORT;

   bool syncCommand(int eod, const char* format, uint32_t fmt_size, ...) U_NO_EXPORT;
   bool syncCommandML(const UString& req, int* vpos, int* vend) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UPop3Client)
};

#endif
