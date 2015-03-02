// count_traffic.cpp

#include <ulib/file.h>
#include <ulib/container/vector.h>

#undef  PACKAGE
#define PACKAGE "count_traffic"

#define ARGS "[path of WIFI apache-log-like event records]"

#define U_OPTIONS \
"purpose 'count traffic from WIFI apache-log-like event records'\n"

#include <ulib/application.h>

#define MAXREFH 256 /* Max referrer field size in htab  */

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

      UString content = UFile::contentOf(argv[optind]);

      // ........
      // 10.10.25.2 - - [21/May/2012:16:29:41 +0200] "GET /unirel_logo.gif HTTP/1.1" 200 3414 "http://www.unirel.com/" "Mozilla/5.0 (X11; Linux x86_64)"
      // ........

      UVector<UString> vec_entry(10),
                       vec_events_log(content, '\n');

      void* p;
      const char* s;
      const char* ptr;
      char buffer[32];
      UString date, request, referer;
      uint64_t traffico = 0, traffic1; // , traffic2;
      uint32_t referer_size = 0, n, start = 0, end, u_printf_string_max_length_save;

      buffer[0] = 0;

      for (uint32_t i = 0, n1 = vec_events_log.size(); i < n1; ++i)
         {
         (void) vec_entry.split(vec_events_log[i]);

         date    = vec_entry[3];
         request = vec_entry[5];
         referer = vec_entry[8];

         if (referer.size() > referer_size) referer_size = referer.size();

         if (memcmp(request.data(), U_CONSTANT_TO_PARAM("GET /info?")) == 0)
            {
            U_INTERNAL_DUMP("date = %.*S buffer = %.*S", U_STRING_TO_TRACE(date), U_CONSTANT_SIZE("21/Dec/2012"), buffer)

            if (buffer[0] == 0 ||
                memcmp(date.c_pointer(1), buffer, U_CONSTANT_SIZE("21/Dec/2012")) != 0)
               {
               if (traffico)
                  {
                  U_MESSAGE("%.*s: Traffico generato giornaliero (GB) %u",
                            U_CONSTANT_SIZE("21/Dec/2012"), buffer, (uint32_t)(traffico / (1024ULL * 1024ULL * 1024ULL)));

                  traffico = 0;
                  }

               U_MEMCPY(buffer, date.c_pointer(1), U_CONSTANT_SIZE("21/Dec/2012"));
               }

            s     = request.data();
            n     = request.size();
            start = U_CONSTANT_SIZE("GET /info?");

#        ifdef DEBUG
            u_printf_string_max_length_save = u_printf_string_max_length;
                                              u_printf_string_max_length = n;

            U_INTERNAL_DUMP("request = %.*S", n, s)

            u_printf_string_max_length = u_printf_string_max_length_save;
#        endif

            while ((p = u_find(s + start, n - start, U_CONSTANT_TO_PARAM("traffic="))))
               {
               end   = (const char*)p - s;
               start = end + U_CONSTANT_SIZE("traffic=");
               ptr   = s + start;

               if (*ptr != '0')
                  {
                  traffic1 = strtoll(ptr, 0, 10);

                  /*
                  traffic2 = request.substr(ptr, n).strtoll();

                  if (traffic1 != traffic2) U_ERROR("traffic1 = %llu different from traffic2 = %llu", traffic1, traffic2);
                  */

                  traffico += traffic1;

                  U_INTERNAL_DUMP("traffico = %llu", traffico)
                  }
               }
            }

         vec_entry.clear();
         }

      U_MESSAGE("%.*s: Traffico generato giornaliero (GB) %u",
                U_CONSTANT_SIZE("21/Dec/2012"), buffer, (uint32_t)(traffico / (1024ULL * 1024ULL * 1024ULL)));

      U_MESSAGE("referrer field max size (%u)", referer_size);
      }

private:
};

U_MAIN
