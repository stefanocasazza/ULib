// cdbmake.cpp

#include <ulib/db/cdb.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>

#undef  PACKAGE
#define PACKAGE "cdbmake"

#define ARGS "[path of file that contains a series of encoded records] [check input]"

#define U_OPTIONS \
"purpose 'create a constant database (cdb)'\n"

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

      uint32_t len;
      const char* path_of_record_file = argv[optind];

      if (path_of_record_file == U_NULLPTR ||
          (len = u__strlen(path_of_record_file, __PRETTY_FUNCTION__)) == 0)
         {
         U_ERROR("missing <path_of_record_file> argument");
         }

      UString filename = UString(path_of_record_file, len);

#  ifdef U_STDCPP_ENABLE
      UString records = UFile::contentOf(filename);

      if (records)
         {
         UCDB x(false);

         if (x.UFile::creat(U_STRING_FROM_CONSTANT("/tmp/cdbmake.cdb")) &&
             x.UFile::ftruncate(UCDB::sizeFor(1) + records.size() * 2)  &&
             x.UFile::memmap(PROT_READ | PROT_WRITE))
            {
            if (argv[++optind])
               {
               // +11,10:74@10.8.1.5->0 10.8.1.5
               // ....
               // +12,10:114@10.8.1.2->0 10.8.1.2

               UVector<UString> vec_records(records, '\n');
               UString line, buffer(U_CAPACITY), output(U_CAPACITY);

               for (uint32_t i = 0, n = vec_records.size(); i < n; ++i)
                  {
                  line = UStringExt::trim(vec_records[i]);

                  U_INTERNAL_ASSERT_EQUALS(line.first_char(), '+')

                  uint32_t sz   = line.size(),
                           pos1 = line.find_first_of(':'),
                           pos2 = line.find_first_of('-', pos1);

                  U_INTERNAL_ASSERT_DIFFERS(pos2, U_NOT_FOUND)

                  while (line.c_char(++pos2) != '>')
                     {
                     pos2 = line.find_first_of('-', pos2);

                     U_INTERNAL_ASSERT_DIFFERS(pos2, U_NOT_FOUND)
                     }

                  uint32_t sz1 = pos2-pos1-2,
                           sz2 = sz-pos2-1;

                  buffer.snprintf(U_CONSTANT_TO_PARAM("+%u,%u:%.*s->%.*s\n"), sz1, sz2, sz1, line.c_pointer(pos1+1), sz2, line.c_pointer(pos2+1));

                  U_INTERNAL_DUMP("sz = %u sz1 = %u sz2 = %u pos1 = %u pos2 = %u line = %V buffer = %V", sz, sz1, sz2, pos1, pos2, line.rep, buffer.rep)

                  (void) output.append(buffer);
                  }

#           ifdef DEBUG // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
                      line.clear();
               vec_records.clear();
#           endif

               records = output;
               }

            UString basename = UStringExt::basename(filename);

            const char* p = basename.data();

#           ifdef U_EVASIVE_SUPPORT
               if (memcmp(p, U_CONSTANT_TO_PARAM("Evasive")) == 0) UCDB::getValueFromBuffer = UServer_Base::getEvasiveRecFromBuffer;
#           endif
#           ifdef U_THROTTLING_SUPPORT
               if (memcmp(p, U_CONSTANT_TO_PARAM("BandWidthThrottling")) == 0) UCDB::getValueFromBuffer = UServer_Base::getThrottlingRecFromBuffer;
#           endif

            istrstream is(U_STRING_TO_PARAM(records));

            is >> x; // NB: this do ftruncate() e munmap()...

            x.UFile::close();
            x.UFile::reset();
            }
         }
#  endif
      }

private:
};

U_MAIN
