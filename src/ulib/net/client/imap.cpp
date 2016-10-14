// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    imap.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/tokenizer.h>
#include <ulib/net/client/imap.h>
#include <ulib/utility/socket_ext.h>

#define U_IMAP_OK  "* OK"
#define U_IMAP_ERR "* BAD"

UImapClient::~UImapClient()
{
   U_TRACE_UNREGISTER_OBJECT(0, UImapClient)
}

U_NO_EXPORT void UImapClient::setStatus()
{
   U_TRACE_NO_PARAM(0, "UImapClient::setStatus()")

   const char* descr1;
   const char* descr2;

   switch (state)
      {
      case LOGOUT:                  descr1 = "LOGOUT";            break;
      case NOT_AUTHENTICATED:       descr1 = "NOT_AUTHENTICATED"; break;
      case AUTHENTICATED:           descr1 = "AUTHENTICATED";     break;
      case SELECTED:                descr1 = "SELECTED";          break;
      default:                      descr1 = "???";               break;
      }

   switch (response)
      {
      case IMAP_SESSION_OK:         descr2 = "OK";                   break;
      case IMAP_SESSION_CONTINUED:  descr2 = "to be continued...";   break;
      case IMAP_SESSION_BAD:        descr2 = "bad state";            break;
      default:                      descr2 = "???";                  break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%s - (%d, %s)"), descr1, response, descr2);
}

bool UImapClient::_connectServer(const UString& server, unsigned int port, int timeoutMS)
{
   U_TRACE(0, "UImapClient::_connectServer(%V,%u,%d)", server.rep, port, timeoutMS)

   if (Socket::connectServer(server, port, timeoutMS) &&
       (USocketExt::readLineReply(this, buffer), memcmp(buffer.data(), U_CONSTANT_TO_PARAM(U_IMAP_OK)) == 0))
      {
      response = IMAP_SESSION_OK;

#  ifdef DEBUG
      setStatus();
      
      U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)
      
      u_buffer_len = 0;
#  endif

      /*
      if (timeoutMS &&
          USocket::isBlocking())
         {
         USocket::setNonBlocking(); // setting socket to nonblocking
         }
      */

      U_RETURN(true);
      }

   U_RETURN(false);
}

// Send a command to the IMAP server and wait for a response...

U_NO_EXPORT bool UImapClient::syncCommand(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UImapClient::syncCommand(%.*S,%u)", fmt_size, format, fmt_size)

#ifdef DEBUG
   setStatus();

   U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

   u_buffer_len = 0;
#endif

   buffer.setEmpty();

   va_list argp;
   va_start(argp, fmt_size);

   end = USocketExt::vsyncCommandToken(this, buffer, format, fmt_size, argp);

   va_end(argp);

   U_INTERNAL_DUMP("end = %d", end)

   if (memcmp(buffer.c_pointer(end), U_CONSTANT_TO_PARAM("OK")) != 0) U_RETURN(false);

   response = IMAP_SESSION_OK;

   U_RETURN(true);
}

bool UImapClient::startTLS()
{
   U_TRACE_NO_PARAM(0, "UImapClient::startTLS()")

#ifdef USE_LIBSSL
   U_ASSERT(Socket::isSSL())

   if (state == NOT_AUTHENTICATED                   &&
       syncCommand(U_CONSTANT_TO_PARAM("STARTTLS")) &&
       ((USSLSocket*)this)->secureConnection())
      {
      U_RETURN(true);
      }

   response = IMAP_SESSION_BAD;
#endif

   U_RETURN(false);
}

bool UImapClient::login(const char* user, const char* passwd)
{
   U_TRACE(0, "UImapClient::login(%S,%S)", user, passwd)

   if (state == NOT_AUTHENTICATED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("LOGIN %s %s"), user, passwd))
         {
         state = AUTHENTICATED;

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

U_NO_EXPORT void UImapClient::setEnd()
{
   U_TRACE_NO_PARAM(0, "UImapClient::setEnd()")

   U_INTERNAL_ASSERT(buffer)

   const char* ptr1;
   const char* ptr2;

   // "\r\nU0001 " <- "OK ..."

   end -= 2;

   for (ptr1 = ptr2 = buffer.c_pointer(end); !u__isspace(*ptr1); --ptr1) {}

   // "\r" -> "\nU0001 OK ..."

   end -= (ptr2 - ptr1) + 1;

   U_INTERNAL_DUMP("end = %d ptr1 = %.*S", end, 9, ptr1)
}

int UImapClient::getCapabilities(UVector<UString>& vec)
{
   U_TRACE(0, "UImapClient::getCapabilities(%p)", &vec)

   if (syncCommand(U_CONSTANT_TO_PARAM("CAPABILITY")))
      {
      setEnd();

      int pos = sizeof("* CAPABILITY");

      U_INTERNAL_ASSERT_MAJOR(end,pos)

      (void) capa.replace(0, capa.size(), buffer, pos, end - pos);

      uint32_t n = vec.split(capa);

      U_RETURN(n);
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(-1);
}

// Logout an imap session

bool UImapClient::logout()
{
   U_TRACE_NO_PARAM(0, "UImapClient::logout()")

   if (syncCommand(U_CONSTANT_TO_PARAM("LOGOUT")))
      {
      state = LOGOUT;

      U_RETURN(true);
      }

   U_RETURN(false);
}

/**
 * (LIST | LSUB) command representation
 *
 * typedef struct ListResponse {
 *    UString name, hierarchyDelimiter;
 *    bool marked, unmarked, noSelect, noInferiors;
 * } ListResponse;
 */

bool UImapClient::list(const UString& ref, const UString& wild, UVector<ListResponse*>& vec, bool subscribedOnly)
{
   U_TRACE(0, "UImapClient::list(%V,%V,%p,%b)", ref.rep, wild.rep, &vec, subscribedOnly)

   if (state >= AUTHENTICATED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("%s \"%v\" %v"), (subscribedOnly ? "LSUB" : "LIST"), ref.rep, wild.rep))
         {
         setEnd();

         UString attributes;
         ListResponse* elem;
         UString x = buffer.substr(0U, (uint32_t)end);
         UVector<UString> v1(x);

         for (uint32_t i = 0, length = v1.size(); i < length; ++i)
            {
            U_ASSERT(v1[i].equal(U_CONSTANT_TO_PARAM("*")))
            U_ASSERT(v1[i+1].equal(U_CONSTANT_TO_PARAM("LIST")))

            i += 2;
            attributes = v1[i++];

            U_NEW(ListResponse, elem, ListResponse);

            elem->marked      = elem->unmarked =
            elem->noSelect    = elem->noInferiors =
            elem->hasChildren = elem->hasNoChildren = false;

            U_ASSERT_EQUALS(attributes[0],'(')

            if (attributes.size() > 2)
               {
               attributes.rep->unQuote();

               UVector<UString> v2(attributes);

               U_INTERNAL_DUMP("attributes = %V", attributes.rep)

               // (\\HasNoChildren \\HasChildren \\Marked \\Unmarked \\Noselect \\Noinferiors)

               for (uint32_t n = 0, l = v2.size(); n < l; ++n)
                  {
                  if (!elem->hasNoChildren &&
                      v2[n].equal(U_CONSTANT_TO_PARAM("\\HasNoChildren")))
                     {
                     elem->hasNoChildren = true;
                     }
                  else if (!elem->hasChildren &&
                           v2[n].equal(U_CONSTANT_TO_PARAM("\\HasChildren")))
                     {
                     elem->hasChildren = true;
                     }
                  else if (!elem->marked &&
                      v2[n].equal(U_CONSTANT_TO_PARAM("\\Marked")))
                     {
                     elem->marked = true;
                     }
                  else if (!elem->unmarked &&
                           v2[n].equal(U_CONSTANT_TO_PARAM("\\Unmarked")))
                     {
                     elem->unmarked = true;
                     }
                  else if (!elem->noSelect &&
                           v2[n].equal(U_CONSTANT_TO_PARAM("\\Noselect")))
                     {
                     elem->noSelect = true;
                     }
                  else if (!elem->noInferiors &&
                           v2[n].equal(U_CONSTANT_TO_PARAM("\\Noinferiors")))
                     {
                     elem->noInferiors = true;
                     }
                  else
                     {
                     U_ERROR("Unknow tag response for LIST command");
                     }
                  }
               }

            elem->hierarchyDelimiter = v1[i++];
            elem->name               = v1[i];

            elem->name.duplicate();
            elem->hierarchyDelimiter.duplicate();

            vec.push_back(elem);
            }

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

/**
 * STATUS command representation
 *
 * typedef struct StatusInfo {
 *   long messageCount, recentCount, nextUID, uidValidity, unseenCount;
 *    bool hasMessageCount, hasRecentCount, hasNextUID, hasUIDValidity, hasUnseenCount;
 * } StatusInfo;
 */

bool UImapClient::status(const UString& mailboxName, StatusInfo& retval, int items)
{
   U_TRACE(1, "UImapClient::status(%V,%p,%d)", mailboxName.rep, &retval, items)

   if (state >= AUTHENTICATED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("STATUS %v (%s %s %s %s %s)"),
               mailboxName.rep,
               (items & MESSAGE_COUNT  ? "MESSAGES"    : ""),
               (items & RECENT_COUNT   ? "RECENT"      : ""),
               (items & NEXT_UID       ? "UIDNEXT"     : ""),
               (items & UID_VALIDITY   ? "UIDVALIDITY" : ""),
               (items & UNSEEN         ? "UNSEEN"      : "")))
         {
         setEnd();

         (void) U_SYSCALL(memset, "%p,%d,%u", &retval, 0, sizeof(StatusInfo));

         const char* ptr1 = buffer.data();
         const char* ptr2 = ptr1 + sizeof("* STATUS") + mailboxName.size();

         while (*ptr2 != '(') ++ptr2;

         uint32_t length, i = (ptr2 - ptr1);

         UString x = buffer.substr(i, end - i);

         UVector<UString> vec(x);

         for (i = 0, length = vec.size(); i < length; ++i)
            {
            if (retval.hasMessageCount == false &&
                vec[i].equal(U_CONSTANT_TO_PARAM("MESSAGES")))
               {
               retval.messageCount    = vec[++i].strtol();
               retval.hasMessageCount = true;
               }
            else if (retval.hasRecentCount == false &&
                     vec[i] == *UString::str_recent)
               {
               retval.recentCount    = vec[++i].strtol();
               retval.hasRecentCount = true;
               }
            else if (retval.hasNextUID == false &&
                     vec[i] == *UString::str_uidnext)
               {
               retval.nextUID    = vec[++i].strtol();
               retval.hasNextUID = true;
               }
            else if (retval.hasUIDValidity == false &&
                     vec[i] == *UString::str_uidvalidity)
               {
               retval.uidValidity    = vec[++i].strtol();
               retval.hasUIDValidity = true;
               }
            else if (retval.hasUnseenCount == false &&
                     vec[i] == *UString::str_unseen)
               {
               retval.unseenCount    = vec[++i].strtol();
               retval.hasUnseenCount = true;
               }
            else
               {
               U_WARNING("Unknow tag response %V for STATUS command", vec[i].rep);
               }
            }

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

/**
 * (SELECT | EXAMINE) command representation
 *
 * typedef struct MailboxInfo {
 *    bool readWrite;
 *    StatusInfo status;
 *    int flags, permanentFlags;
 *    bool flagsAvailable, permanentFlagsAvailable, readWriteAvailable;
 * } MailboxInfo;
 * 
 * enum MailboxFlag {
 *    SEEN      = 1 << 0,
 *    ANSWERED  = 1 << 1,
 *    FLAGGED   = 1 << 2,
 *    DELETED   = 1 << 3,
 *    DRAFT     = 1 << 4,
 *    RECENT    = 1 << 5,
 *    ASTERISK  = 1 << 6,
 *    MDNSent   = 1 << 7,
 *    Junk      = 1 << 8,
 *    NonJunk   = 1 << 9,
 *    Forwarded = 1 << 10
 * };
 */

U_NO_EXPORT void UImapClient::setFlag(int& _flags, UVector<UString>& vec)
{
   U_TRACE(0, "UImapClient::setFlag(%B,%p)", _flags, &vec)

   for (uint32_t i = 0, length = vec.size(); i < length; ++i)
      {
      // * FLAGS (\\Answered \\Flagged \\Draft \\Deleted \\Seen $MDNSent Junk NonJunk)\r\n

      if (!(_flags & ANSWERED) &&
          vec[i].equal(U_CONSTANT_TO_PARAM("\\Answered")))
         {
         _flags |= ANSWERED;
         }
      else if (!(_flags & FLAGGED) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("\\Flagged")))
         {
         _flags |= FLAGGED;
         }
      else if (!(_flags & DRAFT) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("\\Draft")))
         {
         _flags |= DRAFT;
         }
      else if (!(_flags & DELETED) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("\\Deleted")))
         {
         _flags |= DELETED;
         }
      else if (!(_flags & SEEN) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("\\Seen")))
         {
         _flags |= SEEN;
         }
      else if (!(_flags & RECENT) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("\\Recent")))
         {
         _flags |= RECENT;
         }
      else if (!(_flags & ASTERISK) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("\\*")))
         {
         _flags |= ASTERISK;
         }
      else if (!(_flags & MDNSent) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("$MDNSent")))
         {
         _flags |= MDNSent;
         }
      else if (!(_flags & Junk) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("Junk")))
         {
         _flags |= Junk;
         }
      else if (!(_flags & NonJunk) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("NonJunk")))
         {
         _flags |= NonJunk;
         }
      else if (!(_flags & Forwarded) &&
               vec[i].equal(U_CONSTANT_TO_PARAM("$Forwarded")))
         {
         _flags |= Forwarded;
         }
      else
         {
         U_ERROR("Unknow tag response for SELECT command, exit..");
         }
      }

   U_INTERNAL_DUMP("flags = %B", _flags)
}

U_NO_EXPORT void UImapClient::setMailBox(MailboxInfo& retval)
{
   U_TRACE(1, "UImapClient::setMailBox(%p)", &retval)

   setEnd();

   (void) U_SYSCALL(memset, "%p,%d,%u", &retval, 0, sizeof(MailboxInfo));

   /**
    * Example
    * -------------------------------------------------------------------------------
    * U0005 SELECT INBOX\r\n
    * -------------------------------------------------------------------------------
    * FLAGS (\\Answered \\Flagged \\Draft \\Deleted \\Seen)\r\n
    * OK [PERMANENTFLAGS (\\Answered \\Flagged \\Draft \\Deleted \\Seen \\*)]  \r\n
    * 29 EXISTS\r\n
    * 28 RECENT\r\n
    * OK [UNSEEN 1]  \r\n
    * OK [UIDVALIDITY 1148569720]  \r\n
    * OK [UIDNEXT 5774]  \r\n
    * U0005 OK [READ-WRITE] Completed\r\n
    * -------------------------------------------------------------------------------
    */

   uint32_t n;
   UString line;
   const char* ptr1;
   const char* ptr2;
   UVector<UString> vec;
   UString l, x = buffer.substr(0U, (uint32_t)end);
   UTokenizer tok(x, U_CRLF);

   while (tok.next(line, '\n')) // '\n'
      {
      U_ASSERT_EQUALS(line[0],'*')

      if (line[2] == 'O') // OK ...
         {
         U_ASSERT_EQUALS(line[3],'K')
         U_ASSERT_EQUALS(line[4],' ')

         n = sizeof("* OK");

         U_ASSERT_EQUALS(line[n],'[')

         ptr1 = line.c_pointer(++n);

         if (memcmp(ptr1, U_CONSTANT_TO_PARAM("PERMANENTFLAGS")) == 0)
            {
            n += sizeof("PERMANENTFLAGS");

            U_ASSERT_EQUALS(line[n],'(')

            ptr1 = ptr2 = line.c_pointer(++n);

            while (*ptr2 != ')') ++ptr2;

            l = line.substr(n, ptr2 - ptr1);

            (void) vec.split(l);

            setFlag(retval.permanentFlags, vec);

            retval.permanentFlagsAvailable = true;

            vec.clear();
            }
         else
            {
            ptr2 = ptr1;

            while (*ptr2 != ']') ++ptr2;

            l = line.substr(n, ptr2 - ptr1);

            (void) vec.split(l);

            if (!retval.status.hasUnseenCount &&
                vec[0] == *UString::str_unseen)
               {
               retval.status.unseenCount    = vec[1].strtol();
               retval.status.hasUnseenCount = true;
               }
            else if (!retval.status.hasNextUID &&
                     vec[0] == *UString::str_uidnext)
               {
               retval.status.nextUID    = vec[1].strtol();
               retval.status.hasNextUID = true;
               }
            else if (!retval.status.hasUIDValidity &&
                     vec[0] == *UString::str_uidvalidity)
               {
               retval.status.uidValidity    = vec[1].strtol();
               retval.status.hasUIDValidity = true;
               }
            else
               {
               U_ERROR("Unknow tag response for SELECT command, exit..");
               }

            vec.clear();
            }
         }
      else if (line[2] == 'F') // FLAGS ...
         {
         U_ASSERT_EQUALS(line[3],'L')
         U_ASSERT_EQUALS(line[4],'A')
         U_ASSERT_EQUALS(line[5],'G')
         U_ASSERT_EQUALS(line[6],'S')
         U_ASSERT_EQUALS(line[7],' ')

         n = sizeof("* FLAGS");

         U_ASSERT_EQUALS(line[n],'(')

         ptr1 = ptr2 = line.c_pointer(++n);

         while (*ptr2 != ')') ++ptr2;

         l = line.substr(n, ptr2 - ptr1);

         (void) vec.split(l);

         setFlag(retval.flags, vec);

         retval.flagsAvailable = true;

         vec.clear();
         }
      else // EXISTS | RECENT
         {
         l = line.substr(2U);

         (void) vec.split(l);

         if (!retval.status.hasMessageCount &&
             vec[1].equal(U_CONSTANT_TO_PARAM("EXISTS")))
            {
            retval.status.messageCount    = vec[0].strtol();
            retval.status.hasMessageCount = true;
            }
         else if (!retval.status.hasRecentCount &&
                  vec[1] == *UString::str_recent)
            {
            retval.status.recentCount    = vec[0].strtol();
            retval.status.hasRecentCount = true;
            }
         else
            {
            U_ERROR("Unknow tag response for SELECT command, exit..");
            }

         vec.clear();
         }
      }

   ptr1 = buffer.c_pointer(end + sizeof(U_IMAP_OK));

   while (*ptr1 != '[') ++ptr1;

   if (memcmp(++ptr1, U_CONSTANT_TO_PARAM("READ-WRITE")) == 0) retval.readWriteAvailable = retval.readWrite = true;
}

bool UImapClient::selectMailbox(const UString& name, MailboxInfo& retval)
{
   U_TRACE(0, "UImapClient::selectMailbox(%V,%p)", name.rep, &retval)

   U_INTERNAL_ASSERT(name)

   if (state >= AUTHENTICATED)
      {
      // Don't re-select.

      if (state    == SELECTED &&
          selected == name)
         {
         U_RETURN(true);
         }

      if (syncCommand(U_CONSTANT_TO_PARAM("SELECT %v"), name.rep))
         {
         setMailBox(retval);

         state    = SELECTED;
         selected = name;

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::examineMailbox(const UString& name, MailboxInfo& retval)
{
   U_TRACE(0, "UImapClient::examineMailbox(%V,%p)", name.rep, &retval)

   U_INTERNAL_ASSERT(name)

   if (state >= AUTHENTICATED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("EXAMINE %v"), name.rep))
         {
         setMailBox(retval);

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::createMailbox(const UString& name)
{
   U_TRACE(0, "UImapClient::createMailbox(%V)", name.rep)

   U_INTERNAL_ASSERT(name)

   if (state >= AUTHENTICATED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("CREATE %v"), name.rep))
         {
         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

// Attempt to remove the new mailbox with the given name

bool UImapClient::removeMailbox(const UString& name)
{
   U_TRACE(0, "UImapClient::removeMailbox(%V)", name.rep)

   U_INTERNAL_ASSERT(name)

   if (state >= AUTHENTICATED &&
       syncCommand(U_CONSTANT_TO_PARAM("DELETE %v"), name.rep))
      {
      U_RETURN(true);
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

// Attempt to rename the new mailbox with the given name.

bool UImapClient::renameMailbox(const UString& from, const UString& to)
{
   U_TRACE(0, "UImapClient::renameMailbox(%V,%V)", from.rep, to.rep)

   U_INTERNAL_ASSERT(to)
   U_INTERNAL_ASSERT(from)

   if (state >= AUTHENTICATED &&
       syncCommand(U_CONSTANT_TO_PARAM("RENAME %v %v"), from.rep, to.rep))
      {
      U_RETURN(true);
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

// Attempt to subscribe to the new mailbox with the given name. @see RFC

bool UImapClient::subscribeMailbox(const UString& name)
{
   U_TRACE(0, "UImapClient::subscribeMailbox(%V)", name.rep)

   U_INTERNAL_ASSERT(name)

   if (state >= AUTHENTICATED &&
       syncCommand(U_CONSTANT_TO_PARAM("SUBSCRIBE %v"), name.rep))
      {
      U_RETURN(true);
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

// Attempt to unsubscribe from the new mailbox with the given name. @see RFC

bool UImapClient::unsubscribeMailbox(const UString& name)
{
   U_TRACE(0, "UImapClient::unsubscribeMailbox(%V)", name.rep)

   U_INTERNAL_ASSERT(name)

   if (state >= AUTHENTICATED &&
       syncCommand(U_CONSTANT_TO_PARAM("UNSUBSCRIBE %v"), name.rep))
      {
      U_RETURN(true);
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::appendMessage(const UString& mailboxName, const UString& messageData, int _flags, const char* date)
{
   U_TRACE(0, "UImapClient::appendMessage(%V,%V,%d,%S)", mailboxName.rep, messageData.rep, _flags, date)

   U_INTERNAL_ASSERT(mailboxName)
   U_INTERNAL_ASSERT(messageData)
   U_INTERNAL_ASSERT_POINTER(date)

   if (state >= AUTHENTICATED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("APPEND %v %s%s%s%s%s%s%s%s %s\r\n%v"),
               mailboxName.rep,
               (_flags            ? "("           : ""),
               (_flags & SEEN     ? "\\Seen"      : ""),
               (_flags & ANSWERED ? " \\Answered" : ""),
               (_flags & FLAGGED  ? " \\Flagged"  : ""),
               (_flags & DELETED  ? " \\Deleted"  : ""),
               (_flags & DRAFT    ? " \\Draft"    : ""),
               (_flags & RECENT   ? " \\Recent"   : ""),
               (_flags            ? ")"           : ""),
               date,
               messageData.rep))
         {
         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::expunge(int* _ret)
{
   U_TRACE(0, "UImapClient::expunge(%p)", _ret)

   if (state == SELECTED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("EXPUNGE")))
         {
         if (_ret)
            {
            setEnd();

            /**
             * Example
             * -------------------------------------------------------------------------------
             * U0005 EXPUNGE\r\n
             * -------------------------------------------------------------------------------
             * 3 EXPUNGE\r\n
             * 3 EXPUNGE\r\n
             * 5 EXPUNGE\r\n
             * 8 EXPUNGE\r\n
             * U0005 OK EXPUNGE Completed\r\n
             * -------------------------------------------------------------------------------
             */

            UString line;
            UString x = buffer.substr(0U, (uint32_t)end);
            UTokenizer tok(x, U_CRLF);

            while (tok.next(line, '\n')) // '\n'
               {
               U_ASSERT_EQUALS(line[0],'*')

               *_ret++ = ::strtol(line.c_pointer(2), 0, 10);
               }
            }

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::search(int* _ret, const UString& spec, const char* charSet, bool usingUID)
{
   U_TRACE(0, "UImapClient::search(%p,%V,%S,%b)", _ret, spec.rep, charSet, usingUID)

   U_INTERNAL_ASSERT(spec)
   U_INTERNAL_ASSERT_POINTER(_ret)
   U_INTERNAL_ASSERT_POINTER(charSet)

   if (state == SELECTED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("%s SEARCH %s %v"), (usingUID ? "UID" : ""), charSet, spec.rep))
         {
         setEnd();

         /**
          * Example
          * -------------------------------------------------------------------------------
          * U0005 SEARCH TEXT "string not in mailbox"\r\n
          * -------------------------------------------------------------------------------
          * SEARCH 2 84 882\r\n
          * U0005 OK SEARCH Completed\r\n
          * -------------------------------------------------------------------------------
          */

         U_ASSERT_EQUALS(buffer[0],'*')

         UString word;
         UString x = buffer.substr(sizeof("* SEARCH"), end);
         UTokenizer tok(x);

         while (tok.next(word, ' '))
            {
            *_ret++ = ::strtol(word.data(), 0, 10);
            }

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::fetch(UVector<UString>& vec, int start, int _end, const UString& spec, bool usingUID)
{
   U_TRACE(0, "UImapClient::fetch(%p,%d,%d,%V,%b)", &vec, start, _end, spec.rep, usingUID)

   U_INTERNAL_ASSERT(spec)

   if (state == SELECTED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("%s FETCH %d:%d %v"), (usingUID ? "UID" : ""), start, _end, spec.rep))
         {
         setEnd();

         /**
          * Example
          * -------------------------------------------------------------------------------
          * U0005 FETCH 2:4 (FLAGS BODY[HEADER.FIELDS (DATE FROM)])\r\n
          * -------------------------------------------------------------------------------
          * 2 FETCH ....
          * 3 FETCH ....
          * 4 FETCH ....
          * U0005 OK FETCH Completed\r\n
          * -------------------------------------------------------------------------------
          */

         U_ASSERT_EQUALS(buffer[0],'*')

         bool bgroup;
         UString data;
         UTokenizer tok(buffer);
         const char* ptr1 = buffer.data();
         const char* ptr2 = buffer.c_pointer(_end);

         tok.setGroup("()");

         while (true)
            {
            ptr1 += sizeof("* FETCH");

            if  (ptr1 > ptr2) break;

            while (*ptr1 != '(') ++ptr1;

            tok.setPointer(ptr1);

            if (tok.next(data, &bgroup) && bgroup) vec.push_back(data);

            ptr1 += data.size();
            }

         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::setFlags(int start, int _end, int style, int _flags, bool usingUID)
{
   U_TRACE(0, "UImapClient::setFlags(%d,%d,%d,%B,%b)", start, _end, style, _flags, usingUID)

   if (state == SELECTED)
      {
      if (syncCommand(U_CONSTANT_TO_PARAM("%s STORE %d:%d %sFLAGS.SILENT (%s%s%s%s%s%s)"),
               (usingUID ? "UID" : ""),
               start, _end,
               (style & ADD       ? "+"           :
                style & REMOVE    ? "-"           : ""),
               (_flags & SEEN     ? "\\Seen"      : ""),
               (_flags & ANSWERED ? " \\Answered" : ""),
               (_flags & FLAGGED  ? " \\Flagged"  : ""),
               (_flags & DELETED  ? " \\Deleted"  : ""),
               (_flags & DRAFT    ? " \\Draft"    : ""),
               (_flags & RECENT   ? " \\Recent"   : "")))
         {
         U_RETURN(true);
         }
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

bool UImapClient::copy(int start, int _end, const UString& to, bool usingUID)
{
   U_TRACE(0, "UImapClient::copy(%d,%d,%V,%b)", start, _end, to.rep, usingUID)

   U_INTERNAL_ASSERT(to)

   if (state == SELECTED &&
       syncCommand(U_CONSTANT_TO_PARAM("%s COPY %d:%d %v"), (usingUID ? "UID" : ""), start, _end, to.rep))
      {
      U_RETURN(true);
      }

   response = IMAP_SESSION_BAD;

   U_RETURN(false);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UImapClient::dump(bool reset) const
{
   Socket::dump(false);

   *UObjectIO::os << '\n'
                  << "end                           " << end              << '\n'
                  << "state                         " << state            << '\n'
                  << "response                      " << response         << '\n'
                  << "capa            (UString      " << (void*)&capa     << ")\n"
                  << "buffer          (UString      " << (void*)&buffer   << ")\n"
                  << "selected        (UString      " << (void*)&selected << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
