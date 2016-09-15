// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ftp.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_FTP_CLIENT_H
#define U_FTP_CLIENT_H 1

#include <ulib/internal/common.h>

#ifdef USE_LIBSSL
#  include <ulib/ssl/net/sslsocket.h>
#  define Socket USSLSocket
#else
#  include <ulib/net/tcpsocket.h>
#  define Socket UTCPSocket
#endif

/**
 * @class UFtpClient
 *
 * @brief Creates and manages a client connection with a remote FTP server.
 *
 * The FTP protocol, described in <a href="http://www.ietf.org/rfc/rfc959.txt">RFC 959</a>,
 * facilitates the transfer of files from one host to another. This class
 * manages the socket connections with the remote server as well as providing
 * a high-level interface to the commands that are defined by the protocol.
 *
 * <h4>FTP Restart</h4>
 * UFtpClient allows applications to restart failed transfers when the remote
 * FTP server supports stream-mode restart. Stream-mode restart is not
 * specified in RFC 959, but is specified in 
 * <a href="http://www.ietf.org/internet-drafts/draft-ietf-ftpext-mlst-16.txt">
 * Extensions to FTP</a> and is widely supported.
 *
 * Where stream-mode restart is supported, restarting binary transfers is
 * considerably easier than restarting ASCII transfers. This is due to the
 * fact that the FTP @a SIZE command, which reports the size of the remote
 * file, reports the <i>transfer size</i> of the file, which is not necessarily
 * the same as the physical size of the file. The is described in more detail
 * in the documentation for the getFileSize() method
 */

#define FTP_DATA_CONNECTION_OPEN       125
#define FTP_OPENING_DATA_CONNECTION    150
#define FTP_COMMAND_OK                 200
#define FTP_FILE_STATUS                213
#define FTP_READY_FOR_NEW_USER         220
#define FTP_CONTROL_CONNECTION_CLOSED  221
#define FTP_CLOSING_DATA_CONNECTION    226
#define FTP_ENTERING_PASSIVE_MODE      227
#define FTP_USER_LOGGED_IN             230
#define FTP_FILE_ACTION_OK             250
#define FTP_DIRECTORY_CREATED          257
#define FTP_NEED_PASSWORD              331
#define FTP_FILE_ACTION_PENDING        350
#define FTP_TRANSFER_ABORTED           426
#define FTP_FILE_ACTION_NOT_TAKEN      450
#define FTP_BAD_LOGIN                  530 // FTP user authentication failed
#define FTP_ACTION_NOT_TAKEN           550

/* How to execute an ftp transaction:
 *
 *   1. Establish control connection
 *   2. Read greeting from remote ftp  (expect 220 or 2xx code)
 *   3. Execute login
 *         send "USER name"            (expect 331 or 2xx code meaning no password required)
 *         send "PASS password"        (expect 230 or 2xx code - 332 means needs ACCOUNT - not yet supported)
 *   4. Set type to binary (or appropriate)
 *         send "TYPE I"               (expect 200 or 2xx code)
 *         send "STRU F"  // note: default
 *         send "MODE S"  // note: default
 *   5. Cd to path
 *         send "CWD path"             (expect 250 or 2xx code or 550 or 5xx if doesn't exist)
 *   6. get file [see also below]
 *         send "PASV"                 (expect 227 followed by host ip address and port number as 8 bit ASCII numbers)
 *         connect to returned address
 *         send "RETR name"            (expect 150 or 1xx codes - 5xx if doesn't exist)
 *                                     (150 code will be followed by ".*( *[0-9]+ *byte" where number indicates file size)
 *         close data connection       (expect 2xx code - allow error on data connection if all bytes received)
 *      put file (optional ALLO size)
 *         send "STOR name"
 *      or put unique
 *         send "STOU name"
 *   7. Quit
 *         send "QUIT"                 (expect 221 then disconnect)
 *
 * handling restart for file transfer: (size==bytes received previously)
 *      send "REST size"               (expect 3xx)
 *      send "RETR name"...
 *
 * if PASV fails:
 *      choose port to listen to
 *      send "PORT a,b,c,d,p1,p2"
 *         (where a,b,c,d are ip address and p1,p2 are port)
 *      send "RETR"...
 *      wait for connection from ftp server

   How to establishing a protected session:

           Client                                 Server
  control          data                   data               control
====================================================================
                                                             socket()
                                                             bind()
  socket()
  connect()  ----------------------------------------------> accept()
            <----------------------------------------------  220
  AUTH TLS   ---------------------------------------------->
            <----------------------------------------------  234
  TLSneg()  <----------------------------------------------> TLSneg()
  PBSZ 0     ---------------------------------------------->
            <----------------------------------------------  200
  PROT P     ---------------------------------------------->
            <----------------------------------------------  200
  USER fred  ---------------------------------------------->
            <----------------------------------------------  331
  PASS pass  ---------------------------------------------->
            <----------------------------------------------  230
*/

// MARK: send QUIT after receiving failed status instead of just closing connection (nicer)

class U_EXPORT UFtpClient : public Socket {
public:

   enum TransferType { Binary, /*!< Treats files an an opaque stream of bytes */
                       Ascii   /*!< Translates line-feeds into the appropriate local format */ };

   enum DataConnectionType { Passive, /*!< Client connects to the (passive) server for data transfers */
                             Active   /*!< (Active) server connects to the  client for data transfers */ };

   /**
    * Constructs a new UFtpClient with default values for all properties
    */

   UFtpClient(bool bSocketIsIPv6 = false) : Socket(bSocketIsIPv6), pasv(bSocketIsIPv6)
      {
      U_TRACE_REGISTER_OBJECT(0, UFtpClient, "%b", bSocketIsIPv6)
      }

   virtual ~UFtpClient();

   /**
    * function called to established a socket connection with the FTP network server
    */

   bool _connectServer(const UIPAddress& ip, unsigned int _port = 21, int timeoutMS = U_TIMEOUT_MS)
      {
      U_TRACE(0, "UFtpClient::_connectServer(%p,%u,%d)", &ip, _port, timeoutMS)

      bool result = (USocket::connectServer(ip, _port) && waitReady(timeoutMS));

      U_RETURN(result);
      }

   bool _connectServer(const UString& server, unsigned int _port = 21, int timeoutMS = U_TIMEOUT_MS)
      {
      U_TRACE(0, "UFtpClient::_connectServer(%V,%u,%d)", server.rep, _port, timeoutMS)

      bool result = (Socket::connectServer(server, _port, timeoutMS) && waitReady(timeoutMS));

      U_RETURN(result);
      }

   /**
    * This method is to be called after _connectServer() and before login() to secure the ftp communication channel.
    * 
    * @returns @c true if successful and @c false if the ssl negotiation failed
    *
    * Notes: The library uses an ssl/tls encryption approach defined in the draft-murray-auth-ftp-ssl
    *        available at http://www.ford-hutchinson.com/~fh-1-pfh/ftps-ext.html
    */

   bool negotiateEncryption();

   /**
    * Sends a login request to the remote FTP server.
    *
    * Note that the user name and password are sent over the network in plain text,
    * so it is not a good idea to use FTP authentication with sensitive data or passwords.
    *
    * @param user     the userid
    * @param password the password
    */

   bool login(const char* user   = "anonymous",
              const char* passwd = "-lara@gmx.co.uk" /* Dash the password: save traffic by trying
                                                        to avoid multi-line responses */ );

   /**
    * Sets the transfer type that will be used for subsequent data operations:
    * TransferType::Ascii or TransferType::Binary.  Binary transfers treat
    * files as an opaque stream of bytes whereas Ascii transfers translate line-feeds
    * into <CRLF> pairs for transmission over the network, and then translate
    * these back into the format appropriate for the target platform
    */

   bool setTransferType(TransferType type = Binary);

   /**
    * Changes the current working directory on the remote FTP server.
    *
    * @param @path the name of the directory
    * @returns @c true if the current working directory was changed; @c false otherwise
    */

   bool changeWorkingDirectory(const UString& path);

   /**
    * On an already secured ftp session, setDataEncryption() specifies if the data connection
    * channel will be secured for the next data transfer.
    *
    * @param @secure flag either unencrypted (=false) or secure (=true)
    * @returns @c true if successful and @c false if the control connection isn't secure or on error
    */

   bool setDataEncryption(bool secure = true);

   /**
    * Tests whether the preceding data transfer request completed successfully.
    *
    * Asynchronous data transfer requests signal their completion by returning EndOfFile
    * (for read) or by the application closing the stream (for write). In both cases,
    * the remote FTP server sends a response message on the control connection to indicate
    * if the remote operation completed successfully. This method interrogates the response
    * and indicates the success of the operation
    */

   bool dataTransferComplete()
      {
      U_TRACE_NO_PARAM(0, "UFtpClient::dataTransferComplete()")

      readCommandResponse();

      bool result = (response == FTP_CLOSING_DATA_CONNECTION);

      U_RETURN(result);
      }

   /**
    * Retrieves the specified file from the remote server and makes it available as an stream.
    *
    * The application should read from the stream until it receives an EOF marker.
    * At this point the application should check the success of the remote
    * operation by calling dataTransferComplete().
    *
    * If this UFtpClient is going to be used for further operations it is essential
    * that dataTransferComplete() is called. However, if the application is not
    * interested in testing the success of the transfer, and no other FTP operations
    * are required, the UFtpClient may be deleted before the stream is fully processed.
    *
    * This method can be used to restart a failed retrieve operation
    * by specifying a value for the @c offset parameter. Note that this value refers
    * to the number of bytes to skip from the network transfer, not a number of
    * bytes from the remote file. However, for binary transfers, these two values are the same
    *
    * @param path the file name to retrieve.
    * @param offset the number of bytes of the transfer to skip.
    * @returns A stream attached to the remote file.
    *
    * @sa dataTransferComplete()
    */

   int retrieveFile(const UString& path, off_t offset = 0);

   /**
    * Execute an ftp transaction:
    *
    * 1. Establish control connection
    * 2. Read greeting from remote ftp (expect 220 or 2xx code)
    */

   int download(const UIPAddress& ip, const UString& path, off_t offset = 0)
      {
      U_TRACE(0, "UFtpClient::download(%p,%V,%I)", &ip, path.rep, offset)

      int result = (_connectServer(ip) ? download(path, offset) : -1);

      U_RETURN(result);
      }

   int download(const UString& server, const UString& path, off_t offset = 0)
      {
      U_TRACE(0, "UFtpClient::download(%V,%V,%I)", server.rep, path.rep, offset)

      int result = (_connectServer(server) ? download(path, offset) : -1);

      U_RETURN(result);
      }

   size_t getFileSize() const { return bytes_to_read; }

   size_t getFileSize(const UIPAddress& ip, const UString& path);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   int response;
   unsigned int port;
   uint32_t bytes_to_read;
   Socket pasv;

   void setStatus();

   bool setConnection();
   void readCommandResponse();
   bool syncCommand(const char* format, uint32_t fmt_size, ...); // Send a command to the FTP server and wait for a response

   bool waitReady(uint32_t timeoutMS);
   int  download(const UString& path, off_t offset);

   /**
    * Passive Mode FTP
    *
    * As you know by now, the FTP protocol utilises two Tcp connections:
    * the command connection and a data connection.
    *
    * In standard (AKA active) FTP, the data channel is created by the client
    * listening on a socket (default port 20) and the server connecting to that
    * port. However, this regime causes difficulties for clients sitting behind
    * firewalls because the firewall will normally prevent the (unknown) server
    * connecting to the arbitrary FTP data port.
    *
    * In passive FTP it is the server which manages the data connection by
    * listening for a connection on a server port.
    *
    * To enter passive mode, the client sends the PASV command. The server
    * should respond with a "227 entering passive mode (x,x,x,x,p,p)" message.
    * The message tokens describe an IPv4 intenet address followed by two
    * digits desribing the upper and lower 8-bits of the port number.
    *
    * In stream mode operations (the default), the data connection is closed
    * to mark the end of each data transfer. Additionally, the server only
    * listens for a single connection on the passive port, so there is no
    * point in remembering the host/port assignement - it changes each time
    */

   bool createPassiveDataConnection();

   /**
    * helper function to issue the REST command.
    *
    * The offset value gives the number of octets of the immediately
    * following transfer to not actually send, effectively causing the
    * transmission to be restarted at a later point. A value of zero
    * effectively disables restart, causing the entire file to be
    * transmitted.  The server will respond to the REST command with a
    * 350 reply, indicating that the REST parameter has been saved, and
    * that another command, which should be either RETR or STOR, should
    * then follow to complete the restart
    */

   bool restart(off_t offset);

   /**
    * Returns the transfer size of the remote file.
    *
    * This uses the @a SIZE FTP command which is not defined in RFC 959, but is
    * usually implemented by FTP servers nonetheless.
    *
    * The @a SIZE command is supposed to return the <i>transfer size</i> of the
    * file, which is determined for the transfer mode in operation.  For IMAGE
    * mode, this will equate to the size (in bytes) of the remote file. For
    * ASCII mode, this will equate to the number of bytes that will be used to
    * transfer the file over the network, with line-feeds translated into <CRLF> pairs.
    *
    * <h4>Using getFileSize() to control restart operations</h4>
    * getFileSize() can be used to restart remote store operations if the
    * transfer mode is IMAGE (binary), but care must be taken when using it for
    * ASCII mode transfers from UNIX-based hosts.
    *
    * Line feeds in text files on UNIX hosts are represented by a single <LF> character,
    * and therefore, if a local file is fully transfered to a remote host, the
    * getFileSize() command is likely to report a size larger than the actual size
    * of the local file
    * 
    * @param path the path name of the file
    */

   size_t getFileSize(const UString& path);

private:
   inline void readNumberOfByte() U_NO_EXPORT;
   inline bool readPortToConnect() U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UFtpClient)
};

#endif
