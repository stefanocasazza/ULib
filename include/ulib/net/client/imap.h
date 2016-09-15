// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    imap.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_IMAP_CLIENT_H
#define U_IMAP_CLIENT_H 1

#include <ulib/container/vector.h>

#ifdef USE_LIBSSL
#  include <ulib/ssl/net/sslsocket.h>
#  define Socket USSLSocket
#else
#  include <ulib/net/tcpsocket.h>
#  define Socket UTCPSocket
#endif

/**
 * @class UImapClient
 *
 * @brief Creates and manages a client connection with a remote IMAP server.
 *
 * Typical IMAP session goes like this:
 *
 * Connect to the IMAP server
 *  Login - authenticate with user name and password
 *  Work with folders and messages
 *  Disconnect
 *
 * Unlike much simpler POP3 protocol, IMAP is a multi-session protocol that supports multiple folders,
 * also somewhat confusingly called mailboxes. While you are connected and authenticated, you will be informed
 * about new incoming messages, and also about messages deleted or marked by other concurrent sessions using the
 * same account. Luckily, you usually only have to care about most of this if you want to
 */

class U_EXPORT UImapClient : public Socket {
public:

   enum IMAPClientStatus {
      LOGOUT,
      NOT_AUTHENTICATED,
      AUTHENTICATED,
      SELECTED
   };

   enum IMAPServerStatus {
      IMAP_SESSION_OK,
      IMAP_SESSION_CONTINUED,
      IMAP_SESSION_BAD
   };

   /**
    * Constructs a new UImapClient with default values for all properties
    */

   UImapClient(bool bSocketIsIPv6 = false) : Socket(bSocketIsIPv6), buffer(4000)
      {
      U_TRACE_REGISTER_OBJECT(0, UImapClient, "%b", bSocketIsIPv6)

      U_INTERNAL_ASSERT_POINTER(UString::str_recent)

      state = NOT_AUTHENTICATED;
      }

   virtual ~UImapClient();

   /**
    * Function called to established a socket connection with the IMAP network server
    */

   bool _connectServer(const UString& server, unsigned int port = 143, int timeout = U_TIMEOUT_MS);

   /**
    * Sends a login request to the remote IMAP server
    *
    * Note that the user name and password are sent over the network in plain text,
    * so it is not a good idea to use IMAP authentication with sensitive data or passwords
    *
    * @param user     the userid
    * @param password the password
    */

   bool login(const char* user, const char* passwd);

   bool startTLS();

   /**
    * Universal Commands:
    *
    * CAPABILITY, NOOP, LOGOUT
    *
    * @return list of capability given by server @see RFC
    */

   int getCapabilities(UVector<UString>& vec);

   bool isSTLS(UVector<UString>& v) const { return (v.find(U_CONSTANT_TO_PARAM("STARTTLS")) != U_NOT_FOUND); }

   bool isCapability(UVector<UString>& vec, const UString& x) const  { return (vec.find(x) != U_NOT_FOUND); }

   /**
    * Send 'NOOP' to the server. This is handy if you're just polling
    * for new messages. Well, it will be when it actually returns something
    */

   bool noop() { return syncCommand(U_CONSTANT_TO_PARAM("NOOP")); }

   /**
    * Tell the server we want to log out. If this returns true,
    * you'll have to re-login to do anything else
    */

   bool logout();

   /**
    * Authenticated State Commands:
    *
    * LIST, LSUB, STATUS, SELECT, EXAMINE,
    * CREATE, RENAME, DELETE, SUBSCRIBE, UNSUBSCRIBE, APPEND
    *
    * Get a list of mailboxes matching the specification given in wild,
    * under the mailbox path given in ref.
    *
    * \Marked
    *   The mailbox has been marked "interesting" by the server; the
    *   mailbox probably contains messages that have been added since
    *   the last time the mailbox was selected.
    *
    * \Unmarked
    *   The mailbox does not contain any additional messages since the
    *   last time the mailbox was selected.
    *
    * \Noselect
    *   It is not possible to use this name as a selectable mailbox.
    *
    * \Noinferiors
    *   It is not possible for any child levels of hierarchy to exist
    *   under this name; no child levels exist now and none can be
    *   created in the future
    */

   typedef struct ListResponse { // (LIST | LSUB) command representation
      UString name, hierarchyDelimiter;
      bool marked, unmarked, noSelect, noInferiors, hasChildren, hasNoChildren;
   } ListResponse;

   bool list(const UString& ref, const UString& wild, UVector<ListResponse*>& vec, bool subscribedOnly = false);

   /**
    * Request status information on a mailbox.
    * The currently defined status data items that can be requested are:
    */

   typedef struct StatusInfo { // STATUS command representation
      long messageCount, recentCount, nextUID, uidValidity, unseenCount;
      bool hasMessageCount, hasRecentCount, hasNextUID, hasUIDValidity, hasUnseenCount;
   } StatusInfo;

   enum StatusInfoType {
      MESSAGE_COUNT = 1 << 0, // The number of messages in the mailbox
      RECENT_COUNT  = 1 << 1, // The number of messages with the \Recent flag set
      NEXT_UID      = 1 << 2, // The next unique identifier value of the mailbox
      UID_VALIDITY  = 1 << 3, // The unique identifier validity value of the mailbox
      UNSEEN        = 1 << 4  // The number of messages which do not have the \Seen flag set
   };

   // items is a logical OR of StatusInfoType item

   bool status(const UString& mailboxName, StatusInfo& retval,
               int items = MESSAGE_COUNT | RECENT_COUNT | NEXT_UID | UID_VALIDITY | UNSEEN);

   typedef struct MailboxInfo { // (SELECT | EXAMINE) command representation
      StatusInfo status;
      int flags, permanentFlags;
      bool readWrite, flagsAvailable, permanentFlagsAvailable, readWriteAvailable;
   } MailboxInfo;

   // The flags that are applicable for a mailbox. Flags other than the system flags can also exist,
   // depending on server implementation. The PERMANENTFLAGS list can also include the special flag \*,
   // which indicates that it is possible to create new keywords by attempting to store those flags in the mailbox

   enum MailboxFlag {
      SEEN      = 1 << 0,
      ANSWERED  = 1 << 1,
      FLAGGED   = 1 << 2,
      DELETED   = 1 << 3,
      DRAFT     = 1 << 4,
      RECENT    = 1 << 5,
      ASTERISK  = 1 << 6,
      MDNSent   = 1 << 7,
      Junk      = 1 << 8,
      NonJunk   = 1 << 9,
      Forwarded = 1 << 10
   };

   /**
    * Select a mailbox. The client has a concept of the 'current' mailbox.
    * Some operations (check, close, expunge, search, fetch, store, copy)
    * require a mailbox to be selected before they will work
    */

   bool selectMailbox(const UString& name, MailboxInfo& ret);

   /**
    * Get info about a mailbox without selecting it, otherwise identical to @see select
    */

   bool examineMailbox(const UString& name, MailboxInfo& ret);

   /**
    * Attempt to create a new mailbox with the given name
    */

   bool createMailbox(const UString& name);

   /**
    * Attempt to remove the new mailbox with the given name
    */

   bool removeMailbox(const UString& name);

   /**
    * Attempt to rename the new mailbox with the given name
    */

   bool renameMailbox(const UString& from, const UString& to);

   /**
    * Attempt to subscribe to the new mailbox with the given name. @see RFC
    */

   bool subscribeMailbox(const UString& name);

   /**
    * Attempt to unsubscribe from the new mailbox with the given name. @see RFC
    */

   bool unsubscribeMailbox(const UString& name);

   /**
    * Attempt to add a message to a mailbox, with optional initial flags.
    * If date is specified, the date of the message is set accordingly.
    * For the format of the date parameter, @see RFC
    */

   bool appendMessage(const UString& mailboxName, const UString& messageData, int flags = 0, const char* date = "");

   // ------------------------------------------------------------------------------------------------------
   // In the selected state, commands that manipulate messages in a mailbox are permitted.
   // the following commands are valid in the selected state:
   //
   // CHECK, CLOSE, EXPUNGE, SEARCH, FETCH, STORE, COPY, UID
   // ------------------------------------------------------------------------------------------------------

   /**
    * @see RFC
    */

   bool check() { return (state == SELECTED ? syncCommand(U_CONSTANT_TO_PARAM("CHECK")) : false); }

   /**
    * The CLOSE command permanently removes all messages that have the
    * \Deleted flag set from the currently selected mailbox, and returns
    * to the authenticated state from the selected state. No untagged
    * EXPUNGE responses are sent.
    * No messages are removed, and no error is given, if the mailbox is
    * selected by an EXAMINE command or is otherwise selected read-only
    */

   bool close() { return (state == SELECTED ? syncCommand(U_CONSTANT_TO_PARAM("CLOSE")) : false); }

   /**
    * Attempt to remove all message from a mailbox. Note that some
    * implementations may do something like move all messages to the
    * 'trash' box, unless of course the currently selected mailbox IS
    * the trash box
    */

   bool expunge(int* ret = 0);

   /**
    * Search for messages. For the format of the spec parameter, @see RFC.
    * You may specify the charset to be used when searching.
    * If you don't specify usingUID, then the returned message numbers
    * will be indices, not UIDs
    */

   bool search(int* ret, const UString& spec, const char* charSet = "", bool usingUID = false);

   /**
    * Retrieve information about message(s) within the range [start ... end].
    * For the format of the spec parameter and the returned value, @see RFC.
    * To operate on only one message, specify end == start.
    * Specifying usingUID means you're giving UIDs, not indices
    */

   bool fetch(UVector<UString>& vec, int start, int end, const UString& spec, bool usingUID = false);

   /**
    * Set flags on the message(s) within the range [start ... end].
    * To operate on only one message, specify end == start.
    * You may set, add or remove flags. @see FlagSetStyle.
    * Specifying usingUID means you're giving UIDs, not indices
    */

   enum FlagSetStyle {
      SET    = 1 << 0,
      ADD    = 1 << 1,
      REMOVE = 1 << 2
   };

   bool setFlags(int start, int end, int style, int flags, bool usingUID = false);

   /**
    * Copy message(s) within the range [start ... end] from the currently
    * selected mailbox to that specified in destination.
    * Specifying usingUID means you're giving UIDs, not indices
    */

   bool copy(int start, int end, const UString& destination, bool usingUID = false);
   // ----------------------------------------------------------------------------------------

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer, capa, selected;
   int response, end;
   IMAPClientStatus state;

private:
   void setEnd() U_NO_EXPORT;
   void setStatus() U_NO_EXPORT;
   void setMailBox(MailboxInfo& ret) U_NO_EXPORT;
   void setFlag(int& flags, UVector<UString>& vec) U_NO_EXPORT;

   /**
    * Run a command and return the response from the server.
    * Takes care of prepending the unique string necessary
    */

   bool syncCommand(const char* format, uint32_t fmt_size, ...) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UImapClient)
};

#endif
