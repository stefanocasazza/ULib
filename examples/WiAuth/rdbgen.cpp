// rdbgen.cpp

#include <ulib/db/rdb.h>
#include <ulib/net/server/server.h>

#undef  PACKAGE
#define PACKAGE "rdbgen"

#undef  ARGS
#define ARGS "<path_of_db_file> <number_of_command> [ parameters ]"

#define PURPOSE \
"reliable database (rdb) managing\n" \
"------------------------------------------\n" \
"List of commands:\n" \
"\n" \
" 1 - get        -    parameter: <key>\n" \
" 2 - del        -    parameter: <key>\n" \
" 3 - store      -    parameter: <key> <value>\n" \
" 4 - size       - no parameter\n" \
" 5 - dump       - no parameter\n" \
" 6 - compact    - no parameter\n" \
" 7 - reorganize - no parameter\n" \
"--------------------------------------------"

#include <ulib/application.h>

#include <string.h>

#define U_DIR_OUTPUT  "/tmp/"
#define U_FILE_OUTPUT "rdbdump.txt"

class Application : public UApplication {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      uint32_t plen;
      const char* p = argv[optind++];

      if (p == U_NULLPTR ||
          (plen = u__strlen(p, __PRETTY_FUNCTION__)) == 0)
         {
         U_ERROR("missing <path_of_db_file> argument");
         }

      U_INTERNAL_DUMP("optind = %d argv[optind] = %S", optind, argv[optind])

      const char* method = argv[optind++];

      if (method == U_NULLPTR) U_ERROR("<number_of_command> argument is missing");

      if (u__isdigit(*method) == false) U_ERROR("<number_of_command> argument is not numeric");

      int op = method[0] - '0';
      bool bjournal2remove = false;
      const char* suffix = u_getsuffix(p, plen);

      if (suffix)
         {
         U_INTERNAL_DUMP("suffix = %S", suffix)

         if (memcmp(suffix+1, U_CONSTANT_TO_PARAM("cdb")) == 0)
            {
            char buffer[U_PATH_MAX];

            (void) memcpy(buffer, p, plen);
            (void) memcpy(buffer+plen, ".jnl", sizeof(".jnl"));

            if (UFile::access(buffer, R_OK) == false) bjournal2remove = true;
            }
         else if (memcmp(suffix+1, U_CONSTANT_TO_PARAM("jnl")) == 0)
            {
            plen -= U_CONSTANT_SIZE(".jnl");

            U_WARNING("you must avoid the jnl suffix");
            }
         }

      URDB x(UString(p, plen), false);

      bool bshm  = (method[1] == 's'),
           bopen = (bshm ? x.open(10 * 1024 * 1024, false, (op == 6), true, U_NULLPTR)
                         : x.open(10 * 1024 * 1024, false, (op == 6), true));

      if (bopen)
         {
         if (bjournal2remove) (void) x.getJournal()._unlink(); // NB: we have only the constant db
         if (x.UFile::st_size == 0) (void) x.UFile::_unlink(); // NB: we have only the journal

         if (bshm == false) x.resetReference();

         U_INTERNAL_DUMP("optind = %d argv[optind] = %S op = %d", optind, argv[optind], op)

         switch (op)
            {
            case 1: // get
               {
               UString key(argv[optind]), value = x[key]; 

               (void) UFile::writeToTmp(U_STRING_TO_PARAM(value), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM(U_FILE_OUTPUT), 0);
               }
            break;

            case 2: // del
               {
#           if defined(U_STDCPP_ENABLE) && !defined(HAVE_OLD_IOSTREAM)
               string input = "";

               cout << "Are you sure to want <DELETE> ?\n>";
               getline(cin, input);
               cout << "You entered: " << input << endl << endl;

               UString key(argv[optind]); 

               UApplication::exit_value = x.remove(key);
#           endif
               }
            break;

            case 3: // store
               {
               UString key(argv[optind]);

               p = argv[optind++]; 

               UString value(p, strlen(p));

               if (value.equal(U_CONSTANT_TO_PARAM(U_DIR_OUTPUT U_FILE_OUTPUT)))
                  {
                  p = U_DIR_OUTPUT U_FILE_OUTPUT;

                  value = UStringExt::trim(UFile::contentOf(UString(p, strlen(p))));
                  }

               UApplication::exit_value = x.store(key, value, RDB_REPLACE);
               }
            break;

            case 4: // size, capacity
               {
               char buffer[128];
               uint32_t sz  = x.getCapacity(),
                        jsz = x.getJournalSize(),
                        n   = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("journal.size() =  %.2fM (%u bytes) - %u record(s) - capacity: %.2fM (%u bytes)\n"),
                                         (double)jsz / (1024.0 * 1024.0), jsz, x.size(), (double)sz / (1024.0 * 1024.0), sz);

               (void) write(1, buffer, n);
               }
            break;

            case 5: // dump
               {
               UString y = x.UFile::getName();

               p = y.data();

#           ifdef U_EVASIVE_SUPPORT
               if (memcmp(p, U_CONSTANT_TO_PARAM("Evasive")) == 0) UString::printValueToBuffer = UServer_Base::printEvasiveRecToBuffer;
#           endif
#           ifdef U_THROTTLING_SUPPORT
               if (memcmp(p, U_CONSTANT_TO_PARAM("BandWidthThrottling")) == 0) UString::printValueToBuffer = UServer_Base::printThrottlingRecToBuffer;
#           endif

               y = x.print();

               if (y.empty()) (void) UFile::_unlink(U_DIR_OUTPUT U_FILE_OUTPUT);
               else           (void) UFile::writeToTmp(U_STRING_TO_PARAM(y), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM(U_FILE_OUTPUT), 0);
               }
            break;

            case 6: // compact
               {
#           if defined(U_STDCPP_ENABLE) && !defined(HAVE_OLD_IOSTREAM)
               string input = "";

               cout << "Are you sure to want <JOURNAL COMPACTION> ?\n>";
               getline(cin, input);
               cout << "You entered: " << input << endl << endl;

               UApplication::exit_value = (x.compactionJournal() == false);
#           endif
               }
            break;

            case 7: // reorganize
               {
#           if defined(U_STDCPP_ENABLE) && !defined(HAVE_OLD_IOSTREAM)
               string input = "";

               cout << "Are you sure to want <REORGANIZE> ?\n>";
               getline(cin, input);
               cout << "You entered: " << input << endl << endl;

               UApplication::exit_value = (x.closeReorganize() == false);
#           endif

               return;
               }

            default:
               U_ERROR("<number_of_command> argument is not valid");
            break;
            }

         x.close(false);
         }
      }

private:
};

U_MAIN
