// count_traffic1.cpp

#include <ulib/file.h>
#include <ulib/container/hash_map.h>
#include <ulib/utility/string_ext.h>

#undef  PACKAGE
#define PACKAGE "count_traffic1"

#define ARGS "[path of WIFI log event records]"

#define U_OPTIONS \
"purpose 'count traffic from WIFI log event records'\n"

#include <ulib/application.h>

static uint32_t traffico;

class Application : public UApplication {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   static bool addTraffic(UStringRep* key, void* value)
      {
      U_TRACE(5, "Application::addTraffic(%.*S,%p)", U_STRING_TO_TRACE(*key), value)

      long traffic1 = ((UStringRep*)value)->strtol();

      traffico += traffic1;

      U_INTERNAL_DUMP("traffico = %u traffic1 = %ld", traffico, traffic1)

      U_RETURN(true);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      UString content = UFile::contentOf(argv[optind]);

      // ........
      // 2013/09/15 03:49:58 op: MAC_AUTH_all, uid: 60:fa:cd:7d:14:06,
      //                     ap: ap@10.8.0.69:5280/wimoMichelangelo-r29587_rspro,
      //                     ip: 172.16.69.111, mac: 60:fa:cd:7d:14:06, timeout: 93, traffic: 295, policy: DAILY
      // ........

      UVector<UString> vec_entry(18),
                       vec_events_log(content, '\n');

      bool bpolicy;
      long traffic1;
      char buffer[32];
      UHashMap<UString> table;
      UString date, op, uid, _uid, traffic, _traffic, policy;

      buffer[0] = 0;

      for (uint32_t i = 0, n1 = vec_events_log.size(); i < n1; ++i)
         {
         (void) vec_entry.split(vec_events_log[i]);

         traffic = vec_entry[15];

         if (memcmp(traffic.data(), U_CONSTANT_TO_PARAM("300,")) != 0)
            {
            op = vec_entry[3];

            char c = op.first_char();

            if (c == 'E' ||                       // EXIT
                c == 'Q' ||                       // QUIT
                c == 'R' ||                       // RESYNC
               (c == 'L' && op.c_char(3) == 'O')) // LOGOUT
               {
               uid = vec_entry[5];

               if (uid != _uid)
                  {
                  _uid = uid;
                  date = vec_entry[0];

                  U_INTERNAL_DUMP("date = %.*S buffer = %.*S", U_STRING_TO_TRACE(date), U_CONSTANT_SIZE("21/12/12"), buffer)

                  if (memcmp(date.data(), buffer, U_CONSTANT_SIZE("21/12/12")) != 0)
                     {
                     if (traffico)
                        {
                        traffico += 10 * (traffico / 100);

                        U_MESSAGE("%.*s: Traffico generato giornaliero (GB) %u",
                                  U_CONSTANT_SIZE("21/12/12"), buffer, (uint32_t)(traffico / 1024ULL));

                        table.callForAllEntry(addTraffic);
                        table.clear();

                        traffico = 0;
                        }

                     U_MEMCPY(buffer, date.data(), U_CONSTANT_SIZE("21/12/12"));
                     }

                  policy  = vec_entry[17];
                  bpolicy = (memcmp(policy.data(), U_CONSTANT_TO_PARAM("TRAFFIC")) == 0); 

                  U_INTERNAL_DUMP("%.*s %.*s: op = %.*S uid = %.*S traffic = %.*S policy = %.*S",
                                  U_STRING_TO_TRACE(date), U_STRING_TO_TRACE(vec_entry[1]), 
                                  U_STRING_TO_TRACE(op), U_STRING_TO_TRACE(uid), U_STRING_TO_TRACE(traffic), U_STRING_TO_TRACE(policy))

                  _traffic = table[uid];

                  if (_traffic)
                     {
                     U_INTERNAL_DUMP("uid = %.*S _traffic = %.*S traffic = %.*S",
                                      U_STRING_TO_TRACE(uid), U_STRING_TO_TRACE(_traffic), U_STRING_TO_TRACE(traffic))

                     if (_traffic != traffic)
                        {
                        UString tmp(10U);

                        traffic1 = traffic.strtol();
                        traffic1 = ((bpolicy ? 2048 : 300) - traffic1);

                        tmp.setFromNumber32s(traffic1);

                        table.replaceAfterFind(tmp);
                        }

                     continue;
                     }

                  traffic1  = traffic.strtol();
                  traffico += ((bpolicy ? 2048 : 300) - traffic1);

                  U_INTERNAL_DUMP("traffico = %u traffic1 = %ld", traffico, traffic1)

                  table.insert(uid.copy(), traffic.copy());
                  }
               }
            }

         vec_entry.clear();
         }

      table.callForAllEntry(addTraffic);
      table.clear();

      traffico += 10 * (traffico / 100);

      U_MESSAGE("%.*s: Traffico generato giornaliero (GB) %u", U_CONSTANT_SIZE("21/12/12"), buffer, (uint32_t)(traffico / 1024ULL));
      }

private:
};

U_MAIN
