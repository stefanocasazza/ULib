// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ftp.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/client/ftp.h>
#include <ulib/utility/socket_ext.h>

UFtpClient::~UFtpClient()
{
   U_TRACE_UNREGISTER_OBJECT(0, UFtpClient)
}

void UFtpClient::setStatus()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::setStatus()")

   const char* descr;

   switch (response)
      {
      case 110:                           descr = "Restart marker reply";                                break;
      case 120:                           descr = "Service ready in nnn minutes";                        break;
      case FTP_DATA_CONNECTION_OPEN:      descr = "Data connection already open; transfer starting";     break;
      case FTP_OPENING_DATA_CONNECTION:   descr = "File status okay; about to open data connection";     break;
      case FTP_COMMAND_OK:                descr = "Command okay";                                        break;
      case 202:                           descr = "Command not implemented, superfluous at this site";   break;
      case 211:                           descr = "System status, or system help reply";                 break;
      case 212:                           descr = "Directory status";                                    break;
      case FTP_FILE_STATUS:               descr = "File status";                                         break;
      case 214:                           descr = "Help message";                                        break;
      case 215:                           descr = "NAME system type";                                    break;
      case FTP_READY_FOR_NEW_USER:        descr = "Service ready for new user";                          break;
      case FTP_CONTROL_CONNECTION_CLOSED: descr = "Service closing control connection";                  break;
      case 225:                           descr = "Data connection open; no transfer in progress";       break;
      case FTP_CLOSING_DATA_CONNECTION:   descr = "Closing data connection";                             break;
      case FTP_ENTERING_PASSIVE_MODE:     descr = "Entering Passive Mode <h1,h2,h3,h4,p1,p2>";           break;
      case FTP_USER_LOGGED_IN:            descr = "User logged in, proceed";                             break;
      case FTP_FILE_ACTION_OK:            descr = "Requested file action okay, completed";               break;
      case FTP_DIRECTORY_CREATED:         descr = "PATHNAME created";                                    break;
      case FTP_NEED_PASSWORD:             descr = "User name okay, need password";                       break;
      case 332:                           descr = "Need account for login";                              break;
      case FTP_FILE_ACTION_PENDING:       descr = "Requested file action pending further information";   break;
      case 421:                           descr = "Service not available, closing control connection";   break;
      case 425:                           descr = "Can't open data connection";                          break;
      case FTP_TRANSFER_ABORTED:          descr = "Connection closed; transfer aborted";                 break;
      case FTP_FILE_ACTION_NOT_TAKEN:     descr = "Requested file action not taken";                     break;
      case 451:                           descr = "Requested action aborted: local error in processing"; break;
      case 452:                           descr = "Requested action not taken";                          break;
      case 500:                           descr = "Syntax error, command unrecognized";                  break;
      case 501:                           descr = "Syntax error in parameters or arguments";             break;
      case 502:                           descr = "Command not implemented";                             break;
      case 503:                           descr = "Bad sequence of commands";                            break;
      case 504:                           descr = "Command not implemented for that parameter";          break;
      case FTP_BAD_LOGIN:                 descr = "Not logged in";                                       break;
      case 532:                           descr = "Need account for storing files";                      break;
      case FTP_ACTION_NOT_TAKEN:          descr = "Requested action not taken";                          break;
      case 551:                           descr = "Requested action aborted: page type unknown";         break;
      case 552:                           descr = "Requested file action aborted";                       break;
      case 553:                           descr = "Requested action not taken";                          break;
      default:                            descr = "Code unknown";                                        break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("(%d, %s)"), response, descr);
}

bool UFtpClient::waitReady(uint32_t timeoutMS)
{
   U_TRACE(0, "UFtpClient::waitReady(%u)", timeoutMS)

   (void) USocket::setTimeoutRCV(timeoutMS);

   readCommandResponse();

   bool result = (response == FTP_READY_FOR_NEW_USER);

   U_RETURN(result);
}

// Send a command to the FTP server and wait for a response

bool UFtpClient::syncCommand(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UFtpClient::syncCommand(%.*S,%u)", fmt_size, format, fmt_size)

   va_list argp;
   va_start(argp, fmt_size);

   response = USocketExt::vsyncCommandML(this, format, fmt_size, argp);

   va_end(argp);

   bool result = (response != USocket::BROKEN);

   U_RETURN(result);
}

void UFtpClient::readCommandResponse()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::readCommandResponse()")

   response = USocketExt::readMultilineReply(this);

#ifdef DEBUG
   setStatus();

   U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

   u_buffer_len = 0;
#endif
}

bool UFtpClient::negotiateEncryption()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::negotiateEncryption()")

#ifdef USE_LIBSSL
   U_ASSERT(Socket::isSSL())

   if (syncCommand(U_CONSTANT_TO_PARAM("AUTH TLS")) &&
       response == 234         &&
       ((USSLSocket*)this)->secureConnection())
      {
      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

bool UFtpClient::setDataEncryption(bool secure)
{
   U_TRACE(0, "UFtpClient::setDataEncryption(%b)", secure)

#ifdef USE_LIBSSL
   U_ASSERT(Socket::isSSL())

   if (syncCommand(U_CONSTANT_TO_PARAM("PBSZ 0"))                        && response == FTP_COMMAND_OK &&
       syncCommand(U_CONSTANT_TO_PARAM("PROT %c"), (secure ? 'P' : 'C')) && response == FTP_COMMAND_OK)
      {
      pasv.setSSLActive(true);

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

bool UFtpClient::login(const char* user, const char* passwd)
{
   U_TRACE(0, "UFtpClient::login(%S,%S)", user, passwd)

                                      (void) syncCommand(U_CONSTANT_TO_PARAM("USER %s"), user);
   if (response == FTP_NEED_PASSWORD) (void) syncCommand(U_CONSTANT_TO_PARAM("PASS %s"), passwd);

   if (response == FTP_USER_LOGGED_IN) U_RETURN(true);

   U_RETURN(false);
}

// parse response for port number to connect to

inline bool UFtpClient::readPortToConnect()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::readPortToConnect()")

   U_INTERNAL_ASSERT_EQUALS(response, FTP_ENTERING_PASSIVE_MODE)

   // skip reply code

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len,0)

   const char* s = u_buffer + 4;

   // find address in form ip1,ip2,ip3,ip4,p1,p2

   while (*s && !u__isdigit(*s)) { ++s; }

   int i, addr[6];

   for (i = 0; i < 6; ++i)
      {
      addr[i] = 0;

      while (u__isdigit(*s)) { addr[i] = 10 * addr[i] + *s++ - '0'; }

      if (*s == ',' || i == 5) ++s;
      else
         {
         // unexpected address separator

         U_RETURN(false);
         }
      }

   port = addr[4] * (1 << 8) + addr[5];

   U_INTERNAL_DUMP("address = %u.%u.%u.%u:%u", addr[0], addr[1], addr[2], addr[3], port)

   U_RETURN(true);
}

// parse response - compute the expected number of bytes

inline void UFtpClient::readNumberOfByte()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::readNumberOfByte()")

   U_INTERNAL_ASSERT(response == FTP_OPENING_DATA_CONNECTION || response == FTP_DATA_CONNECTION_OPEN)

   // skip reply code

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len,0)

   const char* p = u_buffer + 4;

   while (*p)
      {
      if (*p == '(')
         {
         ++p;

         while (u__isspace(*p)) ++p;

         if (u__isdigit(*p))
            {
            while (u__isdigit(*p)) { bytes_to_read = 10 * bytes_to_read + (*p++ - '0'); }

            while (u__isspace(*p)) ++p;

            if (u__tolower(*p)     == 'b' &&
                u__tolower(*(p+1)) == 'y' &&
                u__tolower(*(p+2)) == 't' &&
                u__tolower(*(p+3)) == 'e')
               {
               break;   
               }
            }

         continue;
         }

      ++p;
      }

   U_INTERNAL_DUMP("bytes_to_read = %lu", bytes_to_read)
}

bool UFtpClient::createPassiveDataConnection()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::createPassiveDataConnection()")

   (void) syncCommand(U_CONSTANT_TO_PARAM("PASV")); // Enter Passive mode

   if (response == FTP_ENTERING_PASSIVE_MODE &&
       readPortToConnect())
      {
      // we have decoded the message, so now connect to the designated IP address/Port.

      if (pasv.USocket::connectServer(USocket::remoteIPAddress(), port)) U_RETURN(true);
      }

   U_RETURN(false);
}

int UFtpClient::retrieveFile(const UString& path, off_t offset)
{
   U_TRACE(0, "UFtpClient::retrieveFile(%V,%I)", path.rep, offset)

   bytes_to_read = 0;

   if (createPassiveDataConnection())
      {
      if (offset == 0 || restart(offset))
         {
         (void) syncCommand(U_CONSTANT_TO_PARAM("RETR %v"), path.rep);

         if (response == FTP_OPENING_DATA_CONNECTION ||
             response == FTP_DATA_CONNECTION_OPEN)
            {
            readNumberOfByte(); // parse response - compute the expected number of bytes
            }
         else
            {
            // If we get a bad response from the FTP server, this could be because the
            // file was not found. We communicate this to the caller be returning
            // a null stream, and the caller will translate the error into an appropriate exception.

            pasv.close();
            }

         U_INTERNAL_DUMP("bytes_to_read = %lu", bytes_to_read)
         }
      }

   U_RETURN(pasv.USocket::getFd());
}

bool UFtpClient::setTransferType(TransferType type)
{
   U_TRACE(0, "UFtpClient::setTransferType(%d)", type)

   if (syncCommand(U_CONSTANT_TO_PARAM("TYPE %c"), (type == Binary ? 'I' : 'A')))
      {
      bool result = (response == FTP_COMMAND_OK);

      U_RETURN(result);
      }

   U_RETURN(false);
}

bool UFtpClient::changeWorkingDirectory(const UString& path)
{
   U_TRACE(0, "UFtpClient::changeWorkingDirectory(%V)", path.rep)

   if (syncCommand(U_CONSTANT_TO_PARAM("CWD %v"), path.rep)) // Change working directory
      {
      // RFC 959 states that the correct response to CDUP is 200 - command_ok
      // But also states that it should respond with the same codes as
      // CWD (250 - file_action_ok). We accept both for both.

      bool result = (response == FTP_COMMAND_OK ||
                     response == FTP_FILE_ACTION_OK);

      U_RETURN(result);
      }

   U_RETURN(false);
}

size_t UFtpClient::getFileSize(const UString& path)
{
   U_TRACE(0, "UFtpClient::getFileSize(%V)", path.rep)

   if (syncCommand(U_CONSTANT_TO_PARAM("SIZE %v"), path.rep) &&
       response == FTP_FILE_STATUS)
      {
      // skip over the response code

      size_t size = strtoul(u_buffer + 4, 0, 10);

      U_RETURN(size);
      }

   U_RETURN(0);
}

bool UFtpClient::restart(off_t offset)
{
   U_TRACE(0, "UFtpClient::restart(%I)", offset)

   if (syncCommand(U_CONSTANT_TO_PARAM("REST %I"), offset))
      {
      bool result = (response == FTP_FILE_ACTION_PENDING);

      U_RETURN(result);
      }

   U_RETURN(false);
}

// Execute an ftp transaction

bool UFtpClient::setConnection()
{
   U_TRACE_NO_PARAM(0, "UFtpClient::setConnection()")

   // 3. Execute login
   // -------------------------------------------------------------------------------------------
   // send "USER name"     (expect 331 or 2xx code meaning no password required)
   // send "PASS password" (expect 230 or 2xx code - 332 means needs ACCOUNT - not yet supported)
   // -------------------------------------------------------------------------------------------

   if (login() == false) U_RETURN(false); // login with ftp server failed...

   // 4. Set type to binary (or appropriate)
   // ------------------------------------------
   // send "TYPE I"  (expect 200 or 2xx code)
   // send "STRU F"  // note: default
   // send "MODE S"  // note: default
   // ------------------------------------------

   if (setTransferType() == false) U_RETURN(false); // setTransferType() with ftp server failed...

   U_RETURN(true);
}

size_t UFtpClient::getFileSize(const UIPAddress& ip, const UString& path)
{
   U_TRACE(0, "UFtpClient::getFileSize(%p,%V)", &ip, path.rep)

   if (_connectServer(ip) == false) U_RETURN(0); // connect with ftp server failed...

   if (setConnection() == false) U_RETURN(0); // login with ftp server failed...

   size_t size = getFileSize(path);

   U_RETURN(size);
}

int UFtpClient::download(const UString& path, off_t offset)
{
   U_TRACE(0, "UFtpClient::download(%V,%I)", path.rep, offset)

   if (setConnection() == false) U_RETURN(-1); // login with ftp server failed...

   // 5. get file
   // ---------------------------------------------------------------------------------------------------------
   // send "PASV"           (expect 227 followed by host ip address and port number as 8 bit ASCII numbers)
   // connect to returned address
   // handling restart for file transfer: (size == bytes received previously)
   // send "REST size"      (expect 3xx)
   // send "RETR name"      (expect 150 or 1xx codes - 5xx if doesn't exist)
   //                       (150 code will be followed by ".*( *[0-9]+ *byte" where number indicates file size)
   // close data connection (expect 2xx code - allow error on data connection if all bytes received)
   // ---------------------------------------------------------------------------------------------------------

   int fd = retrieveFile(path, offset);

   U_RETURN(fd);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UFtpClient::dump(bool reset) const
{
   Socket::dump(false);

   *UObjectIO::os << '\n'
                  << "port                           " << port            << '\n'
                  << "response                       " << response        << '\n'
                  << "bytes_to_read                  " << bytes_to_read   << '\n'
                  << "pasv              (Socket      " << (void*)&pasv    << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
