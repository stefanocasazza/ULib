// rdbgen.cpp

#include <ulib/db/rdb.h>

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

      UString path_of_db_file(argv[optind++]); 

      if (path_of_db_file.empty()) U_ERROR("missing <path_of_db_file> argument");

      URDB x(path_of_db_file, false);

      if (x.UFile::getSuffix().equal(U_CONSTANT_TO_PARAM("jnl")))
         {
         U_ERROR("you must avoid the jnl suffix, exiting");
         }

      if (x.open(10 * 1024 * 1024, false, false, true)) // bool open(uint32_t log_size, bool btruncate, bool cdb_brdonly, bool breference)
         {
         const char* method = argv[optind++];

         if (method == 0)                  U_ERROR("<number_of_command> argument is missing");
         if (u__isdigit(*method) == false) U_ERROR("<number_of_command> argument is not numeric");

         if (method[1] == 's') x.setShared(0,0); // POSIX shared memory object (interprocess - can be used by unrelated processes)
         else                  x.resetReference();

         int op = method[0] - '0';

         switch (op)
            {
            case 1: // get
               {
               UString key(argv[optind]), value = x[key]; 

               (void) UFile::writeToTmp(U_STRING_TO_PARAM(value), false, U_FILE_OUTPUT, 0);
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
               UString key(argv[optind]), value(argv[++optind]);

               if (value.equal(U_CONSTANT_TO_PARAM(U_DIR_OUTPUT U_FILE_OUTPUT))) value = UStringExt::trim(UFile::contentOf(U_DIR_OUTPUT U_FILE_OUTPUT));

               UApplication::exit_value = x.store(key, value, RDB_REPLACE);
               }
            break;

            case 4: // size, capacity
               {
               char buffer[64];
               uint32_t sz = x.getCapacity(),
                        n  = u__snprintf(buffer, sizeof(buffer), "%u record(s) - capacity: %.2fM (%u bytes)\n",
                                         x.size(), (double)sz / (1024.0 * 1024.0), sz);

               (void) write(1, buffer, n);
               }
            break;

            case 5: // dump
               {
               UString value = x.print();

               if (value.empty()) (void) UFile::_unlink(U_DIR_OUTPUT U_FILE_OUTPUT);
               else               (void) UFile::writeToTmp(U_STRING_TO_PARAM(value), false, U_FILE_OUTPUT, 0);
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
