// PEC_report_rejected.cpp

/*------------------------------------------------------------------------------------------------------------
1) per ogni record, in cui e' presente la stringa "stat=", il cui valore e' diverso da
"Sent" and "Deferred", scrivere un entry xml di tipo NumMsgRejected-Entry che riporti
le seguenti informazioni taggate:

- message identifier:         NumMsgRejected-Identifier
- message date:               NumMsgRejected-Date
- installation identifier:    NumMsgRejected-InstallationId
- error (di tipo delivery):   NumMsgRejected-DeliveryError
- error reason                NumMsgRejected-ErrorReason (desunto dal campo dsn)

2) per ogni record, in cui e' presente la stringa "reject=", scrivere una entry xml di
tipo NumMsgRejected-Entry, che riporti le seguenti informazioni taggate:

- message identifier:         NumMsgRejected-Identifier
- message date:               NumMsgRejected-Date
- installation identifier:    NumMsgRejected-InstallationId
- error (protocol type):      NumMsgRejected-ProtocolError
- error reason                NumMsgRejected-ErrorReason (desunto dal valore che si trova dopo il primo spazio)
------------------------------------------------------------------------------------------------------------*/

#include "PEC_report_options.h"

#define PACKAGE "PEC_report_rejected"
#define U_OPTIONS "purpose \"program for report PEC rejected (apply on MTA log)...\"\n" \
        U_OPTIONS_GEN1 \
        U_OPTIONS_GEN2

#include "PEC_report.h"

#define U_XML_REJECTED_START                                                     \
"    <NumMsgRejected>\n"

#define U_XML_REJECTED_ENTRY                                                     \
"      <NumMsgRejected-Entry>\n"                                                 \
"        <NumMsgRejected-Identifier>%.*s</NumMsgRejected-Identifier>\n"          \
"        <NumMsgRejected-Date>%s</NumMsgRejected-Date>\n"                        \
"        <NumMsgRejected-InstallationId>%.*s</NumMsgRejected-InstallationId>\n"  \
"        <NumMsgRejected-%sError>%.*s</NumMsgRejected-%sError>\n"    \
"        <NumMsgRejected-ErrorReason>%.*s</NumMsgRejected-ErrorReason>\n"        \
"      </NumMsgRejected-Entry>\n"

#define U_XML_REJECTED_END                                                       \
"    </NumMsgRejected>\n"

static UString* sent;
static UString* error;
static UString* reason;
static UString* deferred;
static const char* error_type;

class Application : public PEC_report {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (sent)
         {
         delete sent;
         delete error;
         delete reason;
         delete deferred;
         }
      }

   static void printRejected()
      {
      U_TRACE(5, "Application::printRejected()")

      // report entry

      UString tmp(U_CAPACITY);

      UXMLEscape::encode(*identifier, tmp);

      (void) sprintf(buffer, U_XML_REJECTED_ENTRY,
                             U_STRING_TO_TRACE(tmp),
                             PEC_report::getData(),
                             U_STRING_TO_TRACE(*id),
                             error_type,
                             U_STRING_TO_TRACE(*error),
                             error_type,
                             U_STRING_TO_TRACE(*reason));

      std::cout << buffer;
      }

   static void parseLineForError()
      {
      U_TRACE(5, "Application::parseLineForError()")

      if (rejected_type == U_stat)
         {
         if (line->compare(index,     sent->size(),     *sent) == 0 ||
             line->compare(index, deferred->size(), *deferred) == 0) return;

         *error = line->substr(index, sizeField(index));

         uint32_t n = findField(U_dsn);

         if (n == U_NOT_FOUND) reason->clear();
         else                 *reason = line->substr(n, sizeField(n, ','));

         error_type = "Delivery";
         }
      else
         {
         U_INTERNAL_ASSERT(rejected_type == U_reject)

         UTokenizer tline(line->substr(index));

         (void) tline.next(*error, ' ');
         (void) tline.next(*reason, ' ');

         error_type = "Protocol";
         }

      U_INTERNAL_DUMP("error  = %.*S", U_STRING_TO_TRACE(*error))
      U_INTERNAL_DUMP("reason = %.*S", U_STRING_TO_TRACE(*reason))

      printRejected();
      }

   static void start()
      {
      U_TRACE(5, "Application::start()")

      // append to report

      std::cout << U_XML_REJECTED_START;

      // setting for virus

      sent     = new U_STRING_FROM_CONSTANT("Sent");
      error    = new UString;
      reason   = new UString;
      deferred = new U_STRING_FROM_CONSTANT("Deferred");

      PEC_report::parse = Application::parseLineForError;
      }

   static void end()
      {
      U_TRACE(5, "Application::end()")

      // end report

      std::cout << U_XML_REJECTED_END;
      }

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      PEC_report::rejected = true;

      end_func   = Application::end;
      start_func = Application::start;

      PEC_report::run(argc, argv, env);
      }

private:
};

U_MAIN
