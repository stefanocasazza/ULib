// test_imap.cpp

#include <ulib/net/client/imap.h>

static void printStatus(UImapClient::StatusInfo& info)
{
   U_TRACE(5, "printStatus(%p)", &info)

   if (info.hasMessageCount)  cout << info.messageCount  << " ";
   if (info.hasRecentCount)   cout << info.recentCount   << " ";
   if (info.hasNextUID)       cout << info.nextUID       << " ";
   if (info.hasUIDValidity)   cout << info.uidValidity   << " ";
   if (info.hasUnseenCount)   cout << info.unseenCount   << " ";

   cout << endl;
}

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString::str_allocate(STR_ALLOCATE_IMAP);

   UImapClient imap;
   UVector<UString> vec1;
   UString tmp(argv[1], strlen(argv[1])), str_asterisk(U_CONSTANT_TO_PARAM("*"));

   if (imap._connectServer(tmp))
      {
      if (imap.getCapabilities(vec1) && imap.isSTLS(vec1)) (void) imap.startTLS();

      if (imap.login(argv[2], argv[3]))
         {
         cout << vec1 << endl;

         /* (LIST | LSUB) command representation
          * typedef struct ListResponse {
          *    UString name, hierarchyDelimiter;
          *    bool marked, unmarked, noSelect, noInferiors, hasChildren, hasNoChildren;
          *    } ListResponse;
          */

         UString ref;
         UVector<UImapClient::ListResponse*> vec;

         if (imap.list(ref, str_asterisk, vec))
            {
            for (uint32_t i = 0, length = vec.size(); i < length; ++i)
               {
               cout  << vec[i]->marked             << " "
                     << vec[i]->unmarked           << " "
                     << vec[i]->noSelect           << " "
                     << vec[i]->noInferiors        << " "
                     << vec[i]->hasChildren        << " "
                     << vec[i]->hasNoChildren      << " "
                     << vec[i]->hierarchyDelimiter << " "
                     << vec[i]->name               << endl;
               }

            vec.clear();
            }

         if (imap.list(U_STRING_FROM_CONSTANT("INBOX"), str_asterisk, vec))
            {
            for (uint32_t i = 0, length = vec.size(); i < length; ++i)
               {
               cout  << vec[i]->marked             << " "
                     << vec[i]->unmarked           << " "
                     << vec[i]->noSelect           << " "
                     << vec[i]->noInferiors        << " "
                     << vec[i]->hasChildren        << " "
                     << vec[i]->hasNoChildren      << " "
                     << vec[i]->hierarchyDelimiter << " "
                     << vec[i]->name               << endl;
               }

            vec.clear();
            }

         /* STATUS command representation
         typedef struct StatusInfo {
            long messageCount, recentCount, nextUID, uidValidity, unseenCount;
            bool hasMessageCount, hasRecentCount, hasNextUID, hasUIDValidity, hasUnseenCount;
         } StatusInfo;
         */

         UImapClient::StatusInfo info;

         if (imap.status(U_STRING_FROM_CONSTANT("INBOX"), info))
            {
            cout << "INBOX ";

            printStatus(info);
            }

         /* (SELECT | EXAMINE) command representation
         typedef struct MailboxInfo {
            bool readWrite;
            StatusInfo status;
            long flags, permanentFlags;
            bool flagsAvailable, permanentFlagsAvailable, readWriteAvailable;
         } MailboxInfo;
         */

         UImapClient::MailboxInfo mail;

         if (imap.examineMailbox(U_STRING_FROM_CONSTANT("INBOX"), mail))
            {
            cout << "INBOX ";

            printStatus(mail.status);

            if (mail.flagsAvailable)            cout << mail.flags            << " ";
            if (mail.permanentFlagsAvailable)   cout << mail.permanentFlags   << " ";
            if (mail.readWriteAvailable)        cout << mail.readWrite        << " ";

            cout << endl;
            }

         if (imap.selectMailbox(U_STRING_FROM_CONSTANT("INBOX"), mail))
            {
            cout << "INBOX ";

            printStatus(mail.status);

            if (mail.flagsAvailable)            cout << mail.flags            << " ";
            if (mail.permanentFlagsAvailable)   cout << mail.permanentFlags   << " ";
            if (mail.readWriteAvailable)        cout << mail.readWrite        << " ";

            cout << endl;
            }

         /*
         (void) imap.createMailbox(U_STRING_FROM_CONSTANT("INBOX/pippo"));
         (void) imap.renameMailbox(U_STRING_FROM_CONSTANT("INBOX/pippo"), U_STRING_FROM_CONSTANT("INBOX/pluto"));
         (void) imap.subscribeMailbox(U_STRING_FROM_CONSTANT("INBOX/pluto"));
         (void) imap.unsubscribeMailbox(U_STRING_FROM_CONSTANT("INBOX/pluto"));
         (void) imap.removeMailbox(U_STRING_FROM_CONSTANT("INBOX/pluto"));
         (void) imap.unsubscribeMailbox(U_STRING_FROM_CONSTANT("INBOX/pippo"));
         (void) imap.removeMailbox(U_STRING_FROM_CONSTANT("INBOX/pippo"));
         */
         }

      cerr << imap.logout();

   // exit(0);
      }
}
