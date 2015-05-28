// test_rdb_client.cpp

#include <ulib/string.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/client_rdb.h>

static void print(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::print(%.*S,%.*S)", U_STRING_TO_TRACE(*key), U_STRING_TO_TRACE(*data))

   cout << UString(key) << "->" << UString(data) << endl;
}

static void transaction(URDBClient<UTCPSocket>& rdb)
{
   U_TRACE(5, "::transaction(%p)", &rdb)

   if (rdb.beginTransaction())
      {
      UString key0  = U_STRING_FROM_CONSTANT("chiave_di_prova0"),
              data0 = U_STRING_FROM_CONSTANT("valore_di_prova0"),
              key1  = U_STRING_FROM_CONSTANT("chiave_di_prova1"),
              data1 = U_STRING_FROM_CONSTANT("valore_di_prova1"),
              key2  = U_STRING_FROM_CONSTANT("chiave_di_prova2"),
              data2 = U_STRING_FROM_CONSTANT("valore_di_prova2"),
              key3  = U_STRING_FROM_CONSTANT("chiave_di_prova3"),
              data3 = U_STRING_FROM_CONSTANT("valore_di_prova3");

      rdb.store(key0, data0, RDB_INSERT);
      rdb.store(key1, data1, RDB_REPLACE);

      U_ASSERT( rdb.remove(key1) == 0 )
      U_ASSERT( rdb[key1].empty() )

      rdb.store(key2, data2, RDB_INSERT);

      U_ASSERT( rdb[key2] == data2 )

      rdb.substitute(key2, key3, data3);

      rdb.commitTransaction();

      U_ASSERT( rdb[key3]        == data3 )
      U_ASSERT( rdb.remove(key2) == -2 )

      rdb.abortTransaction();

      U_ASSERT( rdb[key0].empty() )
      U_ASSERT( rdb[key1].empty() )
      U_ASSERT( rdb[key2].empty() )
      U_ASSERT( rdb[key3].empty() )
      }
}

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   int result;
   UString host(argv[1]);
   URDBClient<UTCPSocket> x(0);

   if (x.setHostPort(host, 8080) && x.connect())
      {
      UString key  = U_STRING_FROM_CONSTANT("foo");
      UString data = U_STRING_FROM_CONSTANT("bar");

      UString value = x[key];

      U_ASSERT( value.empty() )

      result = x.remove(key);

      U_ASSERT( result == -1 )

      x.store(key, data, RDB_INSERT);
      x.store(key, data, RDB_REPLACE);

      value = x[key];

      U_ASSERT( value == data )

      result = x.remove(key);

      U_ASSERT( result ==  0 )

      result = x.remove(key);

      U_ASSERT( result == -2 )

      x.store(key, data, RDB_INSERT);

      UString new_key  = U_STRING_FROM_CONSTANT("foo1");
      UString new_data = U_STRING_FROM_CONSTANT("bar1");

      x.substitute(key, new_key, new_data);

      value = x[key];

      U_ASSERT( value.empty() )

      value = x[new_key];

      U_ASSERT( value == new_data )

      x.substitute(new_key, key, data);

      value = x[new_key];

      U_ASSERT( value.empty() )

      value = x[key];

      U_ASSERT( value == data )

      // Network services, Internet style
      // --------------------------------
      // +6,4:@7/tcp.echo
      // +8,1:echo/tcp.7
      // +6,4:@7/udp.echo
      // +8,1:echo/udp.7
      // +6,7:@9/tcp.discard
      // +11,1:discard/tcp.9
      // +8,1:sink/tcp.9
      // +8,1:null/tcp.9
      // +6,7:@9/udp.discard
      // +11,1:discard/udp.9
      // +8,1:sink/udp.9
      // +8,1:null/udp.9
      // +7,6:@11/tcp.systat
      // +10,2:systat/tcp.11
      // +9,2:users/tcp.11
      // +7,6:@11/udp.systat
      // +10,2:systat/udp.11
      // +9,2:users/udp.11

      char buffer1[128];
      char buffer2[128];
      const char* tbl[9]   = {   "@7", "echo",      "@9", "discard", "sink", "null",    "@11", "systat", "users" };
      const char* _data[9] = { "echo",    "7", "discard",       "9",    "9",    "9", "systat",     "11",    "11" };

      for (int i = 0; i < 9; ++i)
         {
         strcat(strcpy(buffer1, tbl[i]), "/tcp");
         strcat(strcpy(buffer2, tbl[i]), "/udp");

         value = x[UString(buffer1)];
         U_ASSERT( value == UString(_data[i]) )
         value = x[UString(buffer2)];
         U_ASSERT( value == UString(_data[i]) )
         }

      // handles repeated keys
      // ---------------------
      // +3,5:one.Hello
      // +3,7:one.Goodbye
      // +3,7:one.Another
      // +3,5:two.Hello
      // +3,7:two.Goodbye
      // +3,7:two.Another

      UString key1 = U_STRING_FROM_CONSTANT("one");
      UString key2 = U_STRING_FROM_CONSTANT("two");

      value = x[key1];
      U_ASSERT( value == U_STRING_FROM_CONSTANT("Hello") )
      value = x[key2];
      U_ASSERT( value == U_STRING_FROM_CONSTANT("Hello") )

      // handles long keys and data
      // --------------------------
      // +320,320:ba483b3442e75cace82def4b5df25bfca887b41687537.....

#define LKEY "ba483b3442e75cace82def4b5df25bfca887b41687537c21dc4b82cb4c36315e2f6a0661d1af2e05e686c4c595c16561d8c1b3fbee8a6b99c54b3d10d61948445298e97e971f85a600c88164d6b0b09\nb5169a54910232db0a56938de61256721667bddc1c0a2b14f5d063ab586a87a957e87f704acb7246c5e8c25becef713a365efef79bb1f406fecee88f3261f68e239c5903e3145961eb0fbc538ff506a\n"
#define LDATA "152e113d5deec3638ead782b93e1b9666d265feb5aebc840e79aa69e2cfc1a2ce4b3254b79fa73c338d22a75e67cfed4cd17b92c405e204a48f21c31cdcf7da46312dc80debfbdaf6dc39d74694a711\n6d170c5fde1a81806847cf71732c7f3217a38c6234235951af7b7c1d32e62d480d7c82a63a9d94291d92767ed97dd6a6809d1eb856ce23eda20268cb53fda31c016a19fc20e80aec3bd594a3eb82a5a\n"

      U_ASSERT( x[U_STRING_FROM_CONSTANT(LKEY)] == U_STRING_FROM_CONSTANT(LDATA) )

      x.closeReorganize();

      if (x.connect())
         {
         /*
         value = x[key1];
         U_ASSERT( value == U_STRING_FROM_CONSTANT("Another"))

         value = x[key2];
         U_ASSERT( value == U_STRING_FROM_CONSTANT("Another"))
         */

         U_ASSERT( x[U_STRING_FROM_CONSTANT(LKEY)] == U_STRING_FROM_CONSTANT(LDATA) )

         UString _key  = U_STRING_FROM_CONSTANT("chiave_di_prova");
         UString data1 = U_STRING_FROM_CONSTANT("valore_di_prova");

         x.store(_key, data1, RDB_INSERT);
         x.store(_key, data1, RDB_REPLACE);

         value = x[_key];

         U_ASSERT( value == data1 )

         result = x.remove(_key);

         U_ASSERT( result == 0 )

         result = x.remove(_key);

         U_ASSERT( result == -2 )

         transaction(x);

         cout << "--------------------------" << endl;
         x.callForAllEntry(print);
         cout << "-------- sorted ----------" << endl;
         x.callForAllEntrySorted(print);
         cout << "--------------------------" << endl;

         value.clear();

         x.close();
         }
      }
}
