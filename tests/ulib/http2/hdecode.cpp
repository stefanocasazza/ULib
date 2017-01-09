// hdecode.cpp

#include <ulib/utility/http2.h>

#undef  PACKAGE
#define PACKAGE "hdecode"

#define ARGS "[dump file...]" // The file contains a dump of HPACK octets

#define U_OPTIONS \
"purpose 'simple HPACK decoder'\n" \
"option e expect-error  1 '<ERR>' ''\n" \
"option d decoding-spec 1 'Spec format: <letter><size>' ''\n" \
"option t table-size    1 'Default table size: 4096' ''\n"

#include <ulib/application.h>

#define WRT(buf, len) cout.write(buf, len)

#define OUT(msg) cout.write(msg, strlen(msg))

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   /*
   static bool print(UStringRep* key, void* value)
      {
      U_TRACE(5, "Application::print(%V,%p)", key, value)

      OUT("\n");

      WRT(key->data(), key->size());

      OUT(": ");

      WRT(((UStringRep*)value)->data(), ((UStringRep*)value)->size());

      U_RETURN(true);
      }
   */

   static void TST_decode(const UString& content, const UString& spec, int exp)
      {
      U_TRACE(5, "Application::TST_decode(%V,%V,%d)", content.rep, spec.rep, exp)

      bool cut, resize;
      int32_t index_cut = 0;
      const char* pspec = spec.data();
      uint32_t len = 0, sz = 0, clen = content.size();
      unsigned char* cbuf = (unsigned char*)content.data();

      UHashMap<UString>* table = &(UHTTP2::pConnection->itable);
      UHTTP2::HpackDynamicTable* idyntbl = &(UHTTP2::pConnection->idyntbl);

      OUT("Decoded header list:\n");

      /**
       * Spec format: <letter><size>
       *
       * d - decode <size> bytes from the dump
       * p - decode a partial block of <size> bytes
       * r - resize the dynamic table to <size> bytes
       *
       * The last empty spec decodes the rest of the dump
       */

      do {
         cut    =
         resize = false;

         switch (*pspec)
            {
            case '\0': len = clen; break;

            case 'r': resize = true; // resize the dynamic table to <size> bytes
            case 'p':    cut = true; // decode a partial block of <size> bytes
            case 'd':    len = atoi(++pspec); break;

            default: U_ERROR("Invalid spec");
            }

         if (*pspec != '\0') pspec = strchr(pspec, ',') + 1;

         if (resize) UHTTP2::setHpackDynTblCapacity(idyntbl, len);
         else
            {
            UHTTP2::nerror      =
            UHTTP2::hpack_errno = 0;
            UHTTP2::index_ptr   = 0;

            UHTTP2::decodeHeaders(table, idyntbl, cbuf-index_cut, cbuf+len);

            U_INTERNAL_DUMP("UHTTP2::hpack_errno = %d UHTTP2::nerror = %d cut = %b len = %u", UHTTP2::hpack_errno, UHTTP2::nerror, cut, len)

            if (exp &&
                exp == UHTTP2::hpack_errno)
               {
               return;
               }

            index_cut = 0;

            if (cut)
               {
               if (UHTTP2::nerror == UHTTP2::COMPRESSION_ERROR)
                  {
                  if (UHTTP2::index_ptr)
                     {
                     uint32_t advance = UHTTP2::index_ptr - cbuf;

                     U_INTERNAL_DUMP("UHTTP2::index_ptr = %p advance = %u", UHTTP2::index_ptr, advance)

                     if (len > advance) index_cut = (len - advance);
                     }

                  if (index_cut == 0) index_cut = (sz += len);
                  }
               else
                  {
                  if (UHTTP2::index_ptr)
                     {
                     uint32_t advance = UHTTP2::index_ptr - cbuf;

                     U_INTERNAL_DUMP("UHTTP2::index_ptr = %p advance = %u", UHTTP2::index_ptr, advance)

                     if (len < advance) index_cut = (len - advance);
                     }
                  }

               U_INTERNAL_DUMP("index_cut = %d", index_cut)
               }
            else
               {
               if (UHTTP2::nerror != 0 &&
                   UHTTP2::hpack_errno == 0)
                  {
                  return;
                  }
               }

            cbuf += len;
            clen -= len;
            }
         }
      while (clen > 0);

      OUT("\n\n");
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage arg

      UString filename = UString(argv[optind]);

      if (filename.empty()) U_ERROR("missing argument");

      UString content = UFile::contentOf(filename);

      if (content.empty()) U_ERROR("empty file");

      // manage options

      int exp = 0;
      UString spec;

      UHTTP2::btest = true;

      UHTTP2::ctor();

      if (UApplication::isOptions())
         {
         spec = opt['d'];

         UString tmp = opt['t'];

         if (tmp)
            {
            UHTTP2::pConnection->idyntbl.hpack_capacity     =
            UHTTP2::pConnection->idyntbl.hpack_max_capacity =
            UHTTP2::pConnection->odyntbl.hpack_capacity     =
            UHTTP2::pConnection->odyntbl.hpack_max_capacity = tmp.strtoul();
            }

         tmp = opt['e'];

         if (tmp)
            {
            for (int i = 0; i < 11; ++i)
               {
               if (tmp.equal(UHTTP2::hpack_error[i].str, 3))
                  {
                  exp = UHTTP2::hpack_error[i].value;

                  goto next;
                  }
               }

            U_ERROR("Unknown error");
            }
         }

next: TST_decode(content, spec, exp);

      U_INTERNAL_DUMP("UHTTP2::hpack_errno = %d exp = %d", UHTTP2::hpack_errno, exp)

      /*
      UHashMap<UString>* table = &(UHTTP2::pConnection->itable);

      if (table->empty() == false)
         {
         UHTTP2::bhash = true;

         table->callForAllEntrySorted(print);

         UHTTP2::bhash = false;
         }
      */

      cout.write(U_CONSTANT_TO_PARAM("Dynamic Table (after decoding):"));

      UHTTP2::printHpackInputDynTable();

      if (UHTTP2::hpack_errno != 0)
         {
         char buffer[256];

         cerr.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("main: hpack result: %s (%d)\n"), UHTTP2::hpack_strerror(), UHTTP2::hpack_errno));
         }

      if (UHTTP2::hpack_errno != exp) UApplication::exit_value = 1;
      }

private:
#ifndef U_COVERITY_FALSE_POSITIVE
   U_DISALLOW_COPY_AND_ASSIGN(Application)
#endif
};

U_MAIN
