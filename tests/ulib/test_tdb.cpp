// test_tdb.cpp

#include <ulib/db/tdb.h>

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UTDB x;
   int result;

   if (x.create(argv[1]))
      {
      cin >> x;

      UString key  = U_STRING_FROM_CONSTANT("foo");
      UString data = U_STRING_FROM_CONSTANT("bar");

      UString value = x[key];

      U_ASSERT( value.empty() == true )

      result = x.remove(key);

      U_ASSERT( result == 0 )

      x.store(key, data, TDB_INSERT);
      x.store(key, data, TDB_REPLACE);

      value = x[key];

      U_ASSERT( value == data )

      result = x.remove(key);

      U_ASSERT( result )

      result = x.remove(key);

      U_ASSERT( result == 0 )

      x.store(key, data, TDB_INSERT);

      UString new_key  = U_STRING_FROM_CONSTANT("foo1");
      UString new_data = U_STRING_FROM_CONSTANT("bar1");

      x.substitute(key, new_key, new_data);

      value = x[key];

      U_ASSERT( value.empty() == true )

      value = x[new_key];

      U_ASSERT( value == new_data )

      x.substitute(new_key, key, data);

      value = x[new_key];

      U_ASSERT( value.empty() == true )

      value = x[key];

      U_ASSERT( value == data )

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

      UString key1 = U_STRING_FROM_CONSTANT("one");
      UString key2 = U_STRING_FROM_CONSTANT("two");

      value = x[key1];

      U_ASSERT( value == U_STRING_FROM_CONSTANT("Another") )

      value = x[key2];

      U_ASSERT( value == U_STRING_FROM_CONSTANT("Another") )

      // handles long keys and data
      // --------------------------
      // +320,320:ba483b3442e75cace82def4b5df25bfca887b41687537.....

#define LKEY "ba483b3442e75cace82def4b5df25bfca887b41687537c21dc4b82cb4c36315e2f6a0661d1af2e05e686c4c595c16561d8c1b3fbee8a6b99c54b3d10d61948445298e97e971f85a600c88164d6b0b09\nb5169a54910232db0a56938de61256721667bddc1c0a2b14f5d063ab586a87a957e87f704acb7246c5e8c25becef713a365efef79bb1f406fecee88f3261f68e239c5903e3145961eb0fbc538ff506a\n"
#define LDATA "152e113d5deec3638ead782b93e1b9666d265feb5aebc840e79aa69e2cfc1a2ce4b3254b79fa73c338d22a75e67cfed4cd17b92c405e204a48f21c31cdcf7da46312dc80debfbdaf6dc39d74694a711\n6d170c5fde1a81806847cf71732c7f3217a38c6234235951af7b7c1d32e62d480d7c82a63a9d94291d92767ed97dd6a6809d1eb856ce23eda20268cb53fda31c016a19fc20e80aec3bd594a3eb82a5a\n"

      U_ASSERT( x[U_STRING_FROM_CONSTANT(LKEY)] == U_STRING_FROM_CONSTANT(LDATA) )

      x.close();

      if (x.open(argv[1]))
         {
         value = x[key1];

         U_ASSERT( value == U_STRING_FROM_CONSTANT("Another") )

         value = x[key2];

         U_ASSERT( value == U_STRING_FROM_CONSTANT("Another") )

         U_ASSERT( x[U_STRING_FROM_CONSTANT(LKEY)] == U_STRING_FROM_CONSTANT(LDATA) )

         UString _key  = U_STRING_FROM_CONSTANT("chiave_di_prova");
         UString data1 = U_STRING_FROM_CONSTANT("valore_di_prova");

         x.store(_key, data1, TDB_INSERT);
         x.store(_key, data1, TDB_REPLACE);

         value = x[_key];

         U_ASSERT( value == data1 )

         result = x.remove(_key);

         U_ASSERT( result )

         result = x.remove(_key);

         U_ASSERT( result == 0 )

         cout << "--------------------------" << endl;
         cout << x.print();
         cout << "-------- sorted ----------" << endl;
         cout << x.printSorted();
         cout << "--------------------------" << endl;

         value.clear();

         x.close();
         }
      }
}
