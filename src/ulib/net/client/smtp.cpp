// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    smtp.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file_config.h>
#include <ulib/mime/multipart.h>
#include <ulib/net/client/smtp.h>
#include <ulib/utility/string_ext.h>
#include <ulib/utility/socket_ext.h>

void USmtpClient::setStatus()
{
   U_TRACE_NO_PARAM(0, "USmtpClient::setStatus()")

   const char* descr;

   switch (response)
      {
      case CONNREFUSED:                   descr = 0;                                   break; //   1
      case GREET:                         descr = "greeting from server";              break; // 200
      case GOODBYE:                       descr = "server acknolages quit";            break; // 221
      case SUCCESSFUL:                    descr = "command successful";                break; // 250
      case READYDATA:                     descr = "server ready to receive data";      break; // 354
      case ERR_PROCESSING:                descr = "error in processing";               break; // 451
      case UNAVAILABLE:                   descr = "service not available";             break; // 450
      case INSUFFICIENT_SYSTEM_STORAGE:   descr = "error insufficient system storage"; break; // 452
      case SERROR:                        descr = "error";                             break; // 501
      case BAD_SEQUENCE_OF_COMMAND:       descr = "error bad sequence of command";     break; // 503
      case NOT_IMPLEMENT:                 descr = "not implemented";                   break; // 504
      case MAILBOX_UNAVAILABLE:           descr = "error mailbox unavailable";         break; // 550
      case EXCEED_STORAGE_ALLOCATION:     descr = "error exceed storage allocation";   break; // 552
      case MAILBOX_NAME_NOT_ALLOWED:      descr = "error mailbox name not allowed";    break; // 553
      case TRANSACTION_FAILED:            descr = "error transaction failed";          break; // 554
      default:                            descr = "???";                               break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("(%d, %s)"), response, descr);
}

bool USmtpClient::_connectServer(const UString& server, unsigned int port, int timeoutMS)
{
   U_TRACE(0, "USmtpClient::_connectServer(%V,%u,%d)", server.rep, port, timeoutMS)

   if (Socket::connectServer(server, port, timeoutMS) == false)
      {
      response = CONNREFUSED;

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      // NB: the last argument (0) is necessary...

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("Sorry, couldn't connect to server '%v:%u'%R"), server.rep, port, 0);
      }
   else
      {
      response = USocketExt::readMultilineReply(this);

#  ifdef DEBUG
      setStatus();

      U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

      u_buffer_len = 0;
#  endif

      if (response == GREET)
         {
         state = LOG_IN;

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

bool USmtpClient::_connectServer(UFileConfig& cfg, unsigned int port, int timeoutMS)
{
   U_TRACE(0, "USmtpClient::_connectServer(%p,%u,%d)", &cfg, port, timeoutMS)

   U_ASSERT_EQUALS(cfg.empty(), false)

   // USmtpClient - configuration parameters
   // ----------------------------------------------------------------------------------------------------------------------
   // SMTP_SERVER host name or ip address for server
   //
   //       TO_ADDRESS
   //   SENDER_ADDRESS
   // REPLY_TO_ADDRESS
   // ----------------------------------------------------------------------------------------------------------------------

   setSenderAddress(cfg.at(U_CONSTANT_TO_PARAM("SENDER_ADDRESS")));
   setRecipientAddress(cfg.at(U_CONSTANT_TO_PARAM("TO_ADDRESS")));

   if (_connectServer(cfg.at(U_CONSTANT_TO_PARAM("SMTP_SERVER")), port, timeoutMS))
      {
      UString replyToAddress = cfg.at(U_CONSTANT_TO_PARAM("REPLY_TO_ADDRESS"));

      if (replyToAddress)
         {
         U_ASSERT(UStringExt::isEmailAddress(replyToAddress))

         UString tmp(10U + replyToAddress.size());

         tmp.snprintf(U_CONSTANT_TO_PARAM("Reply-To: %v"), replyToAddress.rep);

         setMessageHeader(tmp);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

void USmtpClient::setSenderAddress(const UString& sender)
{
   U_TRACE(0, "USmtpClient::setSenderAddress(%V)", sender.rep)

   uint32_t index = sender.find('<');

   if (index == U_NOT_FOUND)
      {
      senderAddress = sender;

      return;
      }

   ++index;

   uint32_t n = sender.find('>', index);

   if (n != U_NOT_FOUND) n -= index;

   UString tmp = UStringExt::simplifyWhiteSpace(sender.data() + index, n);

   n = 0;

   while (true)
      {
      index = tmp.find(' ', n);

      if (index == U_NOT_FOUND) break;

      n = index + 1; // take one side
      }

   senderAddress.replace(tmp.data() + n, tmp.size() - n);

   index = senderAddress.find('@');

   // won't go through without a local mail system

   if (index == U_NOT_FOUND) senderAddress.append(U_CONSTANT_TO_PARAM("@localhost"));
}

U_NO_EXPORT void USmtpClient::setStateFromResponse()
{
   U_TRACE_NO_PARAM(0, "USmtpClient::setStateFromResponse()")

   switch (response)
      {
      case -1:              state = CERROR; break;
      case GREET:           state = LOG_IN; break;
      case GOODBYE:         state = QUIT;   break;
      case READYDATA:       state = DATA;   break;

      case SUCCESSFUL:
         {
         switch (state)
            {
            case LOG_IN:   state = READY;    break;
            case READY:    state = SENTFROM; break;
            case SENTFROM: state = SENTTO;   break;
            case DATA:     state = FINISHED; break;

            case INIT:
            case QUIT:
            case SENTTO:
            case FINISHED:
            case LOG_OUT:
            case CERROR:
            default:       state = CERROR;   break;
            }
         }
      break;

      default: state = CERROR; break;
      }

   U_INTERNAL_DUMP("state = %d", state)
}

// Send a command to the SMTP server and wait for a response

U_NO_EXPORT bool USmtpClient::syncCommand(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "USmtpClient::syncCommand(%.*S,%u)", fmt_size, format, fmt_size)

   va_list argp;
   va_start(argp, fmt_size);

   response = USocketExt::vsyncCommandML(this, format, fmt_size, argp);

   va_end(argp);

   setStateFromResponse();

   U_RETURN(true);
}

bool USmtpClient::startTLS()
{
   U_TRACE_NO_PARAM(0, "USmtpClient::startTLS()")

#ifdef USE_LIBSSL
   U_ASSERT(Socket::isSSL())

   if (syncCommand(U_CONSTANT_TO_PARAM("STARTTLS")) &&
       response == GREET       &&
       ((USSLSocket*)this)->secureConnection())
      {
      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

// Execute an smtp transaction

bool USmtpClient::sendMessage(bool secure)
{
   U_TRACE(0, "USmtpClient::sendMessage(%b)", secure)

   U_INTERNAL_ASSERT_EQUALS(state,LOG_IN)

   if (domainName.empty())
      {
      /**
       * struct utsname uts;
       * uname(&uts);
       * domainName = uts.nodename;
       */

      (void) domainName.assign(U_CONSTANT_TO_PARAM("somemachine.nowhere.org"));
      }

   if (secure == false) (void) syncCommand(U_CONSTANT_TO_PARAM("helo %v"), domainName.rep);
   else
      {
      (void) syncCommand(U_CONSTANT_TO_PARAM("ehlo %v"), domainName.rep);

      if (response != SUCCESSFUL                ||
          strstr(u_buffer, "250-STARTTLS") == 0 ||
          startTLS() == false)
         {
         U_RETURN(false);
         }

      (void) syncCommand(U_CONSTANT_TO_PARAM("ehlo %v"), domainName.rep);
      }

   if (response != SUCCESSFUL) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(state,READY)

   if (senderAddress.empty()) senderAddress = *UString::str_address;

   U_ASSERT(UStringExt::isEmailAddress(senderAddress))

   (void) syncCommand(U_CONSTANT_TO_PARAM("mail from: %v"), senderAddress.rep);

   if (response != SUCCESSFUL) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(state,SENTFROM)

   if (rcptoAddress.empty()) rcptoAddress = *UString::str_address;

   U_ASSERT(UStringExt::isEmailAddress(rcptoAddress))

   (void) syncCommand(U_CONSTANT_TO_PARAM("rcpt to: %v"), rcptoAddress.rep);

   if (response != SUCCESSFUL) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(state,SENTTO)

   (void) syncCommand(U_CONSTANT_TO_PARAM("data"), 0);

   if (response != READYDATA) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(state,DATA)

   if (messageSubject.empty()) (void) messageSubject.assign(U_CONSTANT_TO_PARAM("no subject"));

        if (messageBody.empty()) (void) messageBody.assign(U_CONSTANT_TO_PARAM("empty"));
   else if (strncmp(messageBody.data(), U_CONSTANT_TO_PARAM("MIME-Version: ")) == 0)
      {
      U_line_terminator_len = 2;

      messageBody = UMimeMultipartMsg::section(messageBody, 0, 0, UMimeMultipartMsg::AUTO, "", "", U_CONSTANT_TO_PARAM("MIME-Version: 1.0"));
      }

   UString msg(rcptoAddress.size() + messageSubject.size() + messageHeader.size() + messageBody.size() + 32U);

   msg.snprintf(U_CONSTANT_TO_PARAM("To: %v\r\n"
                "Subject: %v\r\n"
                "%v\r\n"
                "%v\r\n"
                ".\r\n"), rcptoAddress.rep, messageSubject.rep, messageHeader.rep, messageBody.rep);

   response = (USocketExt::write(this, msg, U_TIMEOUT_MS) ? USocketExt::readMultilineReply(this) : -1);

   setStateFromResponse();

   if (response != SUCCESSFUL) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(state,FINISHED)

   (void) syncCommand(U_CONSTANT_TO_PARAM("quit"), 0);

   if (response != GOODBYE) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(state, QUIT)

   U_RETURN(true);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USmtpClient::dump(bool reset) const
{
   Socket::dump(false);

   *UObjectIO::os << '\n'
                  << "state                         " << state                  << '\n'
                  << "response                      " << response               << '\n'
                  << "domainName      (UString      " << (void*)&domainName     << ")\n"
                  << "messageBody     (UString      " << (void*)&messageBody    << ")\n"
                  << "rcptoAddress    (UString      " << (void*)&rcptoAddress   << ")\n"
                  << "messageHeader   (UString      " << (void*)&messageHeader  << ")\n"
                  << "senderAddress   (UString      " << (void*)&senderAddress  << ")\n"
                  << "messageSubject  (UString      " << (void*)&messageSubject << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
