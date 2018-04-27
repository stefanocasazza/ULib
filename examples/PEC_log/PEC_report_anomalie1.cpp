// PEC_report_anomalie1.cpp

#include "PEC_report_options.h"

#define PACKAGE   "PEC_report_anomalie1"
#define U_OPTIONS "purpose \"program for report PEC anomalie...\"\n" \
                  "option r ttl_range        1 \"Time To Live (TTL) range for messages\" \"\"\n" \
                  "option s max_size_table   1 \"max size table for messages\" \"\"\n" \
        U_OPTIONS_GEN1 \
        U_OPTIONS_GEN2

#include "PEC_report_anomalie.h"

#include <ulib/db/rdb.h>

// #define U_DB_MANAGE

#ifdef U_DB_MANAGE
#  include <ulib/utility/string_ext.h>

static UVector<URDB*>* lrdb;
#else
static URDB* rdb;
#endif

#define MAX_NUM_MSG  (256 * 1024)
#define MARGINE      (MAX_NUM_MSG / 10)

static UString* rdbname;
static uint32_t table_space;
static uint32_t max_size_table;
static uint32_t ntable_discharges;
static UHashMap<Messaggio*>* table;

typedef void* (*pvPFpmpb)(Messaggio*,bool*);

class Application : public PEC_report_anomalie {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (table)
         {
         U_DELETE(table)
         U_DELETE(rdbname)
         U_DELETE(Messaggio::id_max_ttl)

#     ifdef U_DB_MANAGE
         U_DELETE(lrdb)
#     else
         if (rdb) U_DELETE(rdb)
#     endif
         }
      }

#ifdef U_DB_MANAGE
   static void* MessageToString(Messaggio* msg, bool* bdelete)
      {
      U_TRACE(5, "Application::MessageToString(%p,%p)", msg, bdelete)

      *bdelete        = true;
      UStringRep* rep = UObject2StringRep<Messaggio>(*msg);

      U_RETURN_POINTER(rep,UStringRep);
      }
#else
   static void* FirstCheckForOldMessage(Messaggio* msg, bool* bdelete) // writeTo()...
      {
      U_TRACE(5, "Application::FirstCheckForOldMessage(%p,%p)", msg, bdelete)

      U_INTERNAL_DUMP("ntable_discharges = %u", ntable_discharges)

      U_INTERNAL_ASSERT_POINTER(msg)
      U_INTERNAL_ASSERT_EQUALS(ntable_discharges, 1)

      if ((*bdelete = msg->isOld()))
         {
         if (msg->isAnomalia()) PEC_report_anomalie::reportAnomalie(U_NULLPTR, msg);

         UStringRep* rep;

         U_NEW(UStringRep, rep, UStringRep((const char*)&(msg->start), sizeof(time_t)));

         U_DELETE(msg)

         U_RETURN_POINTER(rep,UStringRep);
         }

      U_RETURN_POINTER(U_NULLPTR,UStringRep);
      }

   static void findKeyOnDatabase(const UStringRep* key, Messaggio* msg)
      {
      U_TRACE(5, "Application::findKeyOnDatabase(%.*S,%p)", U_STRING_TO_TRACE(*key), msg)

      ++nout_of_range;

      U_INTERNAL_DUMP("nout_of_range = %u - rdb->getDataSize() = %u - rdb->getDataPointer() = %p",
                       nout_of_range, rdb->getDataSize(), rdb->getDataPointer())

      U_INTERNAL_ASSERT_EQUALS(rdb->getDataSize(), sizeof(time_t))

      time_t old_start = *(time_t*)rdb->getDataPointer(),
             new_ttl   = (msg->start + msg->ttl - old_start);

      U_INTERNAL_DUMP("old_start = %T, msg->start = %T, new_ttl = %T", old_start, msg->start, new_ttl)

      U_WARNING(U_ERROR_MESSAGE_OUT_OF_TTL, new_ttl / U_ONE_DAY_IN_SECOND,
                                            Messaggio::range_ttl / U_ONE_DAY_IN_SECOND,
                                            U_STRING_TO_TRACE(*key));

      if (Messaggio::id_max_ttl &&
          new_ttl > Messaggio::range_ttl)
         {
          Messaggio::max_ttl    = new_ttl;
         *Messaggio::id_max_ttl = msg->identifier;
         }
      }

   static bool checkForOldMessage(UStringRep* key, void* elem) // callWithDeleteForAllEntry()...
      {
      U_TRACE(5, "Application::checkForOldMessage(%.*S,%p)", U_STRING_TO_TRACE(*key), elem)

      U_INTERNAL_DUMP("ntable_discharges = %u", ntable_discharges)

      U_INTERNAL_ASSERT_POINTER(rdb)
      U_INTERNAL_ASSERT_POINTER(elem)
      U_INTERNAL_ASSERT_MAJOR(ntable_discharges, 1)

      Messaggio* msg = (Messaggio*)elem;

      if (msg->isOld())
         {
         if (msg->isAnomalia()) PEC_report_anomalie::reportAnomalie(U_NULLPTR, msg);

         // insert to database...

         rdb->UCDB::setKey(key);
         rdb->UCDB::setData(&(msg->start), sizeof(time_t));

         switch (rdb->store(RDB_INSERT))
            {
            case 0: // OK...
            break;

            case -1: // flag was RDB_INSERT and the key already existed...
               findKeyOnDatabase(key, msg);
            break;

            case -3:
               U_ERROR("there is not enough (virtual) memory available on writing journal of database file %.*S", U_STRING_TO_TRACE(*rdbname));
            break;
            }

         U_RETURN(true); // cancella messaggio...
         }

      U_RETURN(false);
      }

   static bool LastCheckForOldMessage(UStringRep* key, void* elem) // callWithDeleteForAllEntry()...
      {
      U_TRACE(5, "Application::LastCheckForOldMessage(%.*S,%p)", U_STRING_TO_TRACE(*key), elem)

      U_INTERNAL_DUMP("ntable_discharges = %u", ntable_discharges)

      U_INTERNAL_ASSERT_POINTER(rdb)
      U_INTERNAL_ASSERT_POINTER(elem)
      U_INTERNAL_ASSERT_MAJOR(ntable_discharges, 1)

      Messaggio* msg = (Messaggio*)elem;

      if (msg->isAnomalia()) PEC_report_anomalie::reportAnomalie(U_NULLPTR, msg);

      // check if the key already existed in database...

      rdb->UCDB::setKey(key);

      if (rdb->fetch()) findKeyOnDatabase(key, msg);  

      U_RETURN(true); // cancella messaggio...
      }
#endif

   static bool deleteNotAnomalie(UStringRep* key, void* elem)
      {
      U_TRACE(5, "Application::deleteNotAnomalie(%p,%p)", key, elem)

      U_INTERNAL_ASSERT_POINTER(elem)

      if (((Messaggio*)elem)->isAnomalia()) U_RETURN(false);

      U_RETURN(true); // cancella messaggio...
      }

   static void changeFile()
      {
      U_TRACE(5, "Application::changeFile()")

      if (PEC_report::failed) return;

      PEC_report_anomalie::reportMaxTTL();

#  ifdef U_DB_MANAGE
      if (rdbname->empty())
         {
         UString tmp = UStringExt::basename(PEC_report::file->getPath());

         rdbname->snprintf(U_CONSTANT_TO_PARAM("%s/%.*s"), u_tmpdir, U_STRING_TO_TRACE(tmp));

         U_INTERNAL_DUMP("rdbname = %.*S", U_STRING_TO_TRACE(*rdbname))
         }
#  endif

      if (table->size() >= max_size_table)
         {
#     ifdef U_DB_MANAGE
         UString tmp = UStringExt::basename(PEC_report::file->getPath());

         rdbname->snprintf_add(U_CONSTANT_TO_PARAM("#%.*s.cdb"), U_STRING_TO_TRACE(tmp));

         U_INTERNAL_DUMP("rdbname = %.*S", U_STRING_TO_TRACE(*rdbname))

         URDB* rdb = new URDB(rdbname->copy(), false);

         lrdb->push(rdb);

         U_MESSAGE("start session <%d>: write table on database %.*S...", lrdb->size(), U_STRING_TO_TRACE(*rdbname));

         table_space = table->size() * U_DIMENSIONE_MEDIA_RECORD_LOG * 2;

         pvPFpmpb func = Application::MessageToString;

         if (rdb->UCDB::writeTo(table, table_space, (pvPFpvpb)func) == false) U_ERROR("failed to write table on database");

         rdbname->setEmpty();
#     else
         ++ntable_discharges;

         U_MESSAGE("table maximum size reached <%u>: discharging table started...", table->size());

         if (rdb == U_NULLPTR)
            {
            rdbname->snprintf(U_CONSTANT_TO_PARAM("%s/PEC_report_anomalie1.%4D.cdb"), u_tmpdir);

            U_INTERNAL_DUMP("rdbname = %.*S", U_STRING_TO_TRACE(*rdbname))
      
            rdb = new URDB(*rdbname, true);

            table_space = table->size() * 512 + MARGINE;

            pvPFpmpb func = Application::FirstCheckForOldMessage;

            if (rdb->UCDB::writeTo(table, table_space, (pvPFpvpb)func) == false)
               {
               U_ERROR("write to database file %.*S failed", U_STRING_TO_TRACE(*rdbname));
               }
            }
         else
            {
            if (ntable_discharges == 2 &&
             // rdb->open(table_space + MARGINE) == false)
                rdb->open(1024 * 1024 * 1024) == false)
               {
               U_ERROR("open database file %.*S failed", U_STRING_TO_TRACE(*rdbname));
               }

            table->callWithDeleteForAllEntry(Application::checkForOldMessage);
            }

         U_MESSAGE("discharging table finished: actual table size <%u>...", table->size());
#     endif
         }
      }

   static void parseLineForAnomalie()
      {
      U_TRACE(5, "Application::parseLineForAnomalie()")

      // Per ogni riga [ ESCLUSI tipo=non-accettazione o senza tipo es: tipo= (BustaAnomalia)) ] si traccia lo stato...

      if (checkLineForAnomalie() == false) return;

      bool bnew;

      if (table->find(*identifier))
         {
         bnew = false;

         Messaggio::msg = table->elem();
         }
      else
         {
         bnew = true;

         Messaggio::msg = new Messaggio();

         ++nmsg;

         table->hold();

         table->insertAfterFind(Messaggio::msg);
         }

      processLine(bnew);
      }

   static void start()
      {
      U_TRACE(5, "Application::start()")

      // option -s (max size table) and -r (TTL range)...

      UString cfg_range_ttl      = pthis->opt['r'],
              cfg_max_size_table = pthis->opt['s'];

      max_size_table       = (cfg_max_size_table.empty() ? MAX_NUM_MSG
                                                         : cfg_max_size_table.strtol());
      Messaggio::range_ttl = (((cfg_range_ttl.empty()    ? 5
                                                         : cfg_range_ttl.strtol())+1) * U_ONE_DAY_IN_SECOND) - 1;

      PEC_report_anomalie::start();

      // setting for anomalie

      table = new UHashMap<Messaggio*>(u_nextPowerOfTwo(max_size_table + MARGINE), true); // ignore case

      rdbname               = new UString(100U);
      Messaggio::id_max_ttl = new UString;

      PEC_report::parse       = Application::parseLineForAnomalie;
      PEC_report::change_file = Application::changeFile;

#  ifdef U_DB_MANAGE
      lrdb = new UVector<URDB*>;
#  endif
      }

#define U_SUMMARY                                                                      \
"SUMMARY OF ELABORATION\n"                                                             \
"---------------------------------------------------------------------------------\n"  \
"NUMBER OF MESSAGES..................: <%u>\n"                                         \
"NUMBER OF TABLE DISCHARGES..........: <%u> (max size table read as parameter: %u)\n"  \
"NUMBER OF MESSAGES OUT OF TTL RANGE.: <%u> (TTL range read as parameter: %T days)\n"  \
"MAX TTL.............................: <%T> day(s), message id = <%.*s>\n"             \
"NUMBER OF ANOMALOUS MESSAGES........: <%u>\n"                                         \
"---------------------------------------------------------------------------------"

   static void end()
      {
      U_TRACE(5, "Application::end()")

      if (table->size())
         {
         if (ntable_discharges)
            {
            table->callWithDeleteForAllEntry(Application::LastCheckForOldMessage);

            rdb->close();
            }
         else
            {
                               table->callWithDeleteForAllEntry(Application::deleteNotAnomalie);
            if (table->size()) table->callForAllEntrySorted(PEC_report_anomalie::reportAnomalie);
            }
         }

      U_MESSAGE(U_SUMMARY, PEC_report_anomalie::nmsg,
                           ntable_discharges, max_size_table,
                           PEC_report_anomalie::nout_of_range, Messaggio::range_ttl / U_ONE_DAY_IN_SECOND,
                           Messaggio::max_ttl / U_ONE_DAY_IN_SECOND, U_STRING_TO_TRACE(*Messaggio::id_max_ttl),
                           PEC_report_anomalie::nanomalie);

      PEC_report_anomalie::end();
      }

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      end_func   = Application::end;
      start_func = Application::start;

      PEC_report::run(argc, argv, env);

   // exit(0);
      }

private:
};

U_MAIN
