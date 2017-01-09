// hencode.cpp

#include <ulib/utility/http2.h>

#undef  PACKAGE
#define PACKAGE "hencode"

#define U_OPTIONS \
"purpose 'simple HPACK encoder'\n" \
"option e expect-error 1 '<ERR>' ''\n" \
"option l table-limit  1 '' ''\n" \
"option t table-size   1 'Default table size: 4096' ''\n"

#include <ulib/application.h>

#define U_ENCODE_INT_DUMP(a,b,c,d) \
   U_INTERNAL_DUMP("hpackEncodeInt("#a","#b","#c"): %s", u_memoryDump(buffer,buf,UHTTP2::hpackEncodeInt(buf,a,b,c)-buf)) \
   U_INTERNAL_ASSERT_EQUALS(u_get_unalignedp32(buf),d)

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

   static void TST_encode(const UString& content)
      {
      U_TRACE(5, "Application::TST_encode(%V)", content.rep)

      U_INTERNAL_ASSERT_EQUALS(UHTTP2::isHeaderName(U_CONSTANT_TO_PARAM("name:")), false)

      const char* ptr;
      char buffer[4096];
      unsigned char* dst;
      UString token, name, value;
      uint8_t prefix_max, pattern;
      UVector<UString> vline(content,'\n'), vtoken;
      uint32_t i, l, n = vline.size(), num, cnt = 0, idx, index;
      bool cut = false, bvalue_is_indexed = false, binsert_dynamic_table = false;

      UHTTP2::HpackHeaderTableEntry* entry;
      UHTTP2::HpackDynamicTable* dyntbl = &(UHTTP2::pConnection->odyntbl);

      for (i = 0; i < n; ++i)
         {
         U_INTERNAL_ASSERT_EQUALS(bvalue_is_indexed, false)
         U_INTERNAL_ASSERT_EQUALS(binsert_dynamic_table, false)

          name.clear();
         value.clear();

         token = vline[i];

         U_INTERNAL_DUMP("vline[%u] = %V", i, token.rep)

                 vtoken.clear();
           num = vtoken.split(vline[i], ' ');
start:   token = vtoken[0];

         U_INTERNAL_DUMP("token = %V", token.rep)

         if (num == 1)
            {
            if (token.equal(U_CONSTANT_TO_PARAM("send")))
               {
               U_INTERNAL_ASSERT_MAJOR(cnt, 0)

               cnt = 0;
               }
            else if (token.equal(U_CONSTANT_TO_PARAM("push")))
               {
               U_INTERNAL_ASSERT_MAJOR(cnt, 0)

               cnt = 0;
               cut = true;
               }
            else if (token.equal(U_CONSTANT_TO_PARAM("trim")))
               {
               U_INTERNAL_ASSERT_EQUALS(cnt, 0)
               }

            continue;
            }

         dst = (unsigned char*)(ptr = buffer);

         if (token.equal(U_CONSTANT_TO_PARAM("resize")))
            {
            U_INTERNAL_ASSERT_EQUALS(cnt, 0)

            uint32_t val[4096], i0 = i;

            val[0] = vtoken[1].strtoul();

            U_INTERNAL_DUMP("val[0] = %u", val[0])

            l = 0;

            while ((i+1) < n                                                    &&
                   (vtoken.clear(), (num = vtoken.split(vline[++i], ' '))) == 2 &&
                   vtoken[0].equal(U_CONSTANT_TO_PARAM("resize")))
               {
               val[++l] = vtoken[1].strtoul();

               U_DUMP("val[%u] = %u vline[%u] = %V", l, val[l], i, vline[i].rep)
               }

            U_INTERNAL_DUMP("i = %u i0 = %u l = %u", i, i0, l)

            if (l <= 1)
               {
                           dst = UHTTP2::setHpackOutputDynTblCapacity(dst, val[0]);
               if (l == 1) dst = UHTTP2::setHpackOutputDynTblCapacity(dst, val[1]);
               }
            else
               {
               dst = UHTTP2::setHpackOutputDynTblCapacity(dst, val[l-1]);
               dst = UHTTP2::setHpackOutputDynTblCapacity(dst, val[l]);
               }

            cout.write(ptr, dst-(unsigned char*)ptr);

            if (i > i0) goto start;

            continue;
            }

         if (token.equal(U_CONSTANT_TO_PARAM("update")))
            {
            idx = vtoken[1].strtoul();

            U_INTERNAL_DUMP("idx = %u dyntbl->hpack_max_capacity = %u", idx, dyntbl->hpack_max_capacity)

            if (idx < dyntbl->hpack_max_capacity)
               {
               dst = UHTTP2::setHpackOutputDynTblCapacity(dst, idx);

               cout.write(ptr, dst-(unsigned char*)ptr);
               }

            continue;
            }

         ++cnt;

         if (token.equal(U_CONSTANT_TO_PARAM("indexed")))
            {
            U_INTERNAL_ASSERT_EQUALS(num, 2)

            // indexed 6

               pattern = 0x80;
            prefix_max = (1<<7)-1;

            bvalue_is_indexed = true;

            idx = vtoken[1].strtoul();

            goto check2;
            }

         if (token.equal(U_CONSTANT_TO_PARAM("dynamic")))
            {
            U_INTERNAL_ASSERT(num >= 5)

            // dynamic idx 55 huf foo=ASDJKHQKBZXOQWEOPIUAXQWEOIU; max-age=3600; version=1

               pattern = 0x40;
            prefix_max = (1<<6)-1;

            binsert_dynamic_table = true;
            }
         else if (token.equal(U_CONSTANT_TO_PARAM("never")))
            {
            U_INTERNAL_ASSERT(num >= 5)

            // never idx 99999 str value

               pattern = 0x10;
            prefix_max = (1<<4)-1;
            }
         else if (token.equal(U_CONSTANT_TO_PARAM("literal")))
            {
            U_INTERNAL_ASSERT(num >= 5)

            // literal idx 4 str /sample/path

               pattern = 0x00;
            prefix_max = (1<<4)-1;
            }
         else
            {
            U_ERROR("Unknown token");
            }

         value = vtoken[4];

         for (l = 5; l < num; ++l) value += ' ' + vtoken[l];

check1:  if (vtoken[1].equal(U_CONSTANT_TO_PARAM("idx")) == false)
            {
            // not-existing name

            U_INTERNAL_ASSERT_EQUALS(bvalue_is_indexed, false)

            dst = UHTTP2::hpackEncodeInt(dst, 0, prefix_max, pattern);

            if (UHTTP2::isHeaderName((name = vtoken[2])) == false)
               {
               UHTTP2::hpack_errno = -7; // A invalid header name or value character was coded

               return;
               }

            dst = UHTTP2::hpackEncodeString(dst, name, vtoken[1].equal(U_CONSTANT_TO_PARAM("huf")));

            if (UHTTP2::isHpackError()) return;

            dst = UHTTP2::hpackEncodeString(dst, value, vtoken[3].equal(U_CONSTANT_TO_PARAM("huf")));

            if (UHTTP2::isHpackError()) return;

            cout.write(ptr, dst-(unsigned char*)ptr);

            if (binsert_dynamic_table)
               {
               binsert_dynamic_table = false;

               UHTTP2::addHpackDynTblEntry(dyntbl, name.copy(), value.copy());
               }

            continue;
            }

         idx = vtoken[2].strtoul();

check2:  U_INTERNAL_DUMP("idx = %u", idx)

         if (idx == 0 ||
             idx > 4096)
            {
idx_err:    UHTTP2::hpack_errno = -4; // The decoded or specified index is out of range

            return;
            }

         dst = UHTTP2::hpackEncodeInt(dst, idx, prefix_max, pattern);

         if (idx >= HTTP2_HEADER_TABLE_OFFSET)
            {
            index = idx - HTTP2_HEADER_TABLE_OFFSET;

            U_INTERNAL_DUMP("index = %d dyntbl->num_entries = %u bvalue_is_indexed = %b binsert_dynamic_table = %b", index, dyntbl->num_entries, bvalue_is_indexed, binsert_dynamic_table)

            if (index >= dyntbl->num_entries) goto idx_err;

            bvalue_is_indexed = false;

            if (binsert_dynamic_table)
               {
               binsert_dynamic_table = false;

               dst = UHTTP2::hpackEncodeString(dst, value, vtoken[3].equal(U_CONSTANT_TO_PARAM("huf")));

               if (UHTTP2::isHpackError()) return;

               entry = UHTTP2::getHpackDynTblEntry(dyntbl, index);

               name = *(entry->name);

               UHTTP2::evictHpackDynTblEntry(dyntbl, entry);

               UHTTP2::addHpackDynTblEntry(dyntbl, name, value.copy());
               }

            cout.write(ptr, dst-(unsigned char*)ptr);

            continue;
            }

         // existing name

         U_INTERNAL_DUMP("idx = %u bvalue_is_indexed = %b binsert_dynamic_table = %b", idx, bvalue_is_indexed, binsert_dynamic_table)

         if (bvalue_is_indexed) bvalue_is_indexed = false;
         else
            {
            dst = UHTTP2::hpackEncodeString(dst, value, vtoken[3].equal(U_CONSTANT_TO_PARAM("huf")));

            if (UHTTP2::isHpackError()) return;
            }

         cout.write(ptr, dst-(unsigned char*)ptr);

         if (binsert_dynamic_table)
            {
            binsert_dynamic_table = false;

            entry = UHTTP2::hpack_static_table + idx-1;

            name = *(entry->name);

            U_INTERNAL_DUMP("name = %V value = %V", name.rep, value.rep)

            UHTTP2::addHpackDynTblEntry(dyntbl, name, value.copy());
            }

         U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u hpack_capacity = %u hpack_max_capacity = %u",
                        dyntbl->num_entries, dyntbl->entry_capacity, dyntbl->entry_start_index, dyntbl->hpack_size, dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)
         }
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      char buffer[256];

      /*
      unsigned char buf[32];

      U_ENCODE_INT_DUMP( 8,(1<<7)-1,0x80, U_MULTICHAR_CONSTANT16(0x88,0x00))
      U_ENCODE_INT_DUMP( 8,(1<<4)-1,0x00, U_MULTICHAR_CONSTANT16(0x08,0x00))
      U_ENCODE_INT_DUMP(62,(1<<6)-1,0x40, U_MULTICHAR_CONSTANT16(0x7e,0x00))
      U_ENCODE_INT_DUMP(54,(1<<6)-1,0x40, U_MULTICHAR_CONSTANT16(0x76,0x00))

      return;
      */

      // manage input

      UString x(U_CAPACITY);

      UServices::readEOF(STDIN_FILENO, x);

      if (x.empty()) U_ERROR("cannot read data from <stdin>");

      // manage options

      int exp = 0;

      UHTTP2::btest = true;

      UHTTP2::ctor();

      if (UApplication::isOptions())
         {
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

next: TST_encode(x);

      U_INTERNAL_DUMP("UHTTP2::hpack_errno = %d exp = %d", UHTTP2::hpack_errno, exp)

      if (UHTTP2::hpack_errno != exp) UApplication::exit_value = 1;

      if (UHTTP2::hpack_errno) cerr.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("main: hpack result: %s (%d)\n"), UHTTP2::hpack_strerror(), UHTTP2::hpack_errno));
      else
         {
         cout.flush();

         (void) U_SYSCALL(dup2, "%d,%d", 3, STDOUT_FILENO);

         cout.write(U_CONSTANT_TO_PARAM("Dynamic Table (after decoding):"));

         UHTTP2::printHpackOutputDynTable();
         }
      }

private:
#ifndef U_COVERITY_FALSE_POSITIVE
   U_DISALLOW_COPY_AND_ASSIGN(Application)
#endif
};

U_MAIN
