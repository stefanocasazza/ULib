// login_adjust.cpp

#include <ulib/file.h>
#include <ulib/container/vector.h>

#undef  PACKAGE
#define PACKAGE "login_adjust"

#define ARGS "[path of table name Access Point => IP address] [path of WIFI log event records]"

#define U_OPTIONS \
"purpose 'adjust WIFI log event records'\n"

#include <ulib/application.h>

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

      UString riga;
      UString content1 = UFile::contentOf(argv[optind++]),
              content2 = UFile::contentOf(argv[optind]);

      UVector<UString> vec_entry(10),
                       vec_ap_name(content1), // '\n'),
                       vec_events_log(content2, '\n');

      /*
      vec_ap_name.sort();

      riga = vec_ap_name.join('\n');

      (void) write(1, U_STRING_TO_PARAM(riga));

      U_EXIT(1);
      */

      // ........
      // 2013/09/15 03:49:58 op: MAC_AUTH_all, uid: 60:fa:cd:7d:14:06,
      //                     ap: ap@10.8.0.69:5280/wimoMichelangelo-r29587_rspro,
      //                     ip: 172.16.69.111, mac: 60:fa:cd:7d:14:06, timeout: 93, traffic: 295, policy: DAILY
      // ........

      UString ap_entry, ap_name, ap_address1, ap_address2;

      for (uint32_t i = 0, n = vec_events_log.size(); i < n; ++i)
         {
         (void) vec_entry.split(vec_events_log[i], ',');

         ap_entry = vec_entry[2];

         uint32_t pos1 = ap_entry.find_first_of('@'),
                  pos2 = ap_entry.find_first_of('/', pos1),
                  pos3 = pos1+1,
                  len3 = pos2-pos1-6;

         ap_name = ap_entry.substr(pos2+1);

         ap_address1 = ap_entry.substr(pos3, len3);

         uint32_t pos = vec_ap_name.findSorted(ap_name, false, true);

         if (pos == U_NOT_FOUND)
            {
            char buffer[4096];

            (void) write(2, buffer,
                u__snprintf(buffer, sizeof(buffer),
                            U_CONSTANT_TO_PARAM("NOT FOUND: %.*s %.*s %.*s\n"),
                            U_STRING_TO_TRACE(ap_name),
                            U_STRING_TO_TRACE(ap_address1),
                            U_STRING_TO_TRACE(ap_address2)));
            }
         else
            {
            ap_address2 = vec_ap_name[pos+1];

            if (ap_address1 != ap_address2)
               {
               char buffer[4096];

               (void) write(2, buffer,
                   u__snprintf(buffer, sizeof(buffer),
                            U_CONSTANT_TO_PARAM("ERROR: %.*s %.*s %.*s\n"),
                            U_STRING_TO_TRACE(ap_name),
                            U_STRING_TO_TRACE(ap_address1),
                            U_STRING_TO_TRACE(ap_address2)));
               }

            vec_entry.replace(2, ap_entry.replace(pos3, len3, ap_address2));
            }

         riga = vec_entry.join(',');

         (void) write(1, U_STRING_TO_PARAM(riga));
         (void) write(1, U_CONSTANT_TO_PARAM("\n"));

         vec_entry.clear();
         }
      }

private:
};

U_MAIN
