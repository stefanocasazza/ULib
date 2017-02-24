// test_cdb.cpp

#include <ulib/db/cdb.h>

static int print(UStringRep* key, UStringRep* data)
{
   cout << UString(key) << " -> " << UString(data) << endl;

   return 1;
}

int
U_EXPORT main (int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   if (argc > 1)
      {
      UCDB tmp(UString((const char*)argv[1]), true); // NB: ignore case...

      (void) tmp.open();

      if (argc != 2) (void) tmp.find(UString(argv[2]));

      return 0;
      }

   UString str;
   UCDB x(false);

   if (x.UFile::creat(U_STRING_FROM_CONSTANT("tmp/input.cdb")))
      {
      x.UFile::ftruncate(30000);
      x.UFile::memmap(PROT_READ | PROT_WRITE);

      cin >> x; // NB: this do ftruncate() e munmap()...

      x.UFile::close();
      x.UFile::reset();
      }

   if (x.open(true))
      {
      // Network services, Internet style
      // --------------------------------
      // +6,4:@7/tcp->echo
      // +8,1:echo/tcp->7
      // +6,4:@7/udp->echo
      // +8,1:echo/udp->7
      // +6,7:@9/tcp->discard
      // +11,1:discard/tcp->9
      // +8,1:sink/tcp->9
      // +8,1:null/tcp->9
      // +6,7:@9/udp->discard
      // +11,1:discard/udp->9
      // +8,1:sink/udp->9
      // +8,1:null/udp->9
      // +7,6:@11/tcp->systat
      // +10,2:systat/tcp->11
      // +9,2:users/tcp->11
      // +7,6:@11/udp->systat
      // +10,2:systat/udp->11
      // +9,2:users/udp->11

      char buffer1[128];
      char buffer2[128];
      const char* tbl[9]  = {   "@7", "echo",      "@9", "discard",
                              "sink", "null",    "@11", "systat", "users" };
      const char* data[9] = { "echo",    "7", "discard",       "9",
                                 "9",    "9", "systat",     "11",    "11" };

      for (int i = 0; i < 9; ++i)
         {
         strcat(strcpy(buffer1, tbl[i]), "/tcp");
         strcat(strcpy(buffer2, tbl[i]), "/udp");

         U_ASSERT( x[UString(buffer1)] == UString(data[i]) )
         U_ASSERT( x.findNext() == 0 )
         U_ASSERT( x[UString(buffer2)] == UString(data[i]) )
         U_ASSERT( x.findNext() == 0 )
         }

      // handles repeated keys
      // ---------------------
      // +3,5:one->Hello
      // +3,7:one->Goodbye
      // +3,7:one->Another
      // +3,5:two->Hello
      // +3,7:two->Goodbye
      // +3,7:two->Another

      U_ASSERT( x[U_STRING_FROM_CONSTANT("one")] == U_STRING_FROM_CONSTANT("Hello") )
      U_ASSERT( x.findNext() == 1 )
      U_ASSERT( x.elem() == U_STRING_FROM_CONSTANT("Goodbye") )
      U_ASSERT( x.findNext() == 1 )
      U_ASSERT( x.elem() == U_STRING_FROM_CONSTANT("Another") )
      U_ASSERT( x.findNext() == 0 )

      U_ASSERT( x[U_STRING_FROM_CONSTANT("two")] == U_STRING_FROM_CONSTANT("Hello") )
      U_ASSERT( x.findNext() == 1 )
      U_ASSERT( x.elem() == U_STRING_FROM_CONSTANT("Goodbye") )
      U_ASSERT( x.findNext() == 1 )
      U_ASSERT( x.elem() == U_STRING_FROM_CONSTANT("Another") )
      U_ASSERT( x.findNext() == 0 )

      // handles long keys and data
      // --------------------------
      // +320,320:ba483b3442e75cace82def4b5df25bfca887b41687537.....

#define LKEY "ba483b3442e75cace82def4b5df25bfca887b41687537c21dc4b82cb4c36315e2f6a0661d1af2e05e686c4c595c16561d8c1b3fbee8a6b99c54b3d10d61948445298e97e971f85a600c88164d6b0b09\nb5169a54910232db0a56938de61256721667bddc1c0a2b14f5d063ab586a87a957e87f704acb7246c5e8c25becef713a365efef79bb1f406fecee88f3261f68e239c5903e3145961eb0fbc538ff506a\n"
#define LDATA "152e113d5deec3638ead782b93e1b9666d265feb5aebc840e79aa69e2cfc1a2ce4b3254b79fa73c338d22a75e67cfed4cd17b92c405e204a48f21c31cdcf7da46312dc80debfbdaf6dc39d74694a711\n6d170c5fde1a81806847cf71732c7f3217a38c6234235951af7b7c1d32e62d480d7c82a63a9d94291d92767ed97dd6a6809d1eb856ce23eda20268cb53fda31c016a19fc20e80aec3bd594a3eb82a5a\n"

      str = x[U_STRING_FROM_CONSTANT(LKEY)];

      U_ASSERT( str == U_STRING_FROM_CONSTANT(LDATA) )
      U_ASSERT( x.findNext() == 0 )

   // x.UFile::close();
      x.UFile::munmap();
      x.UFile::reset();
      }

   if (x.open(true))
      {
      cout << "--------------------------" << endl;
      x.callForAllEntryWithPattern(print, 0);
      cout << "-------- sorted ----------" << endl;
      x.callForAllEntrySorted(print);
      cout << "--------------------------" << endl;

      str = U_STRING_FROM_CONSTANT("sys");

      x.callForAllEntryWithPattern(print, &str);

      UVector<UString> vec_values;

      uint32_t n = x.getValuesWithKeyNask(vec_values, U_STRING_FROM_CONSTANT("s*"));

      U_ASSERT( n == 4 )

      cout << vec_values << endl;

      vec_values.clear();

   // x.UFile::close();
      x.UFile::munmap();
      x.UFile::reset();
      }

   if (x.UFile::open(U_STRING_FROM_CONSTANT("random.cdb")) &&
       x.open(true))
      {
      char buffer[4096];
      std::ostrstream os(buffer, sizeof(buffer));

      os << x;

      x.UFile::munmap();

      if (x.UFile::creat(U_STRING_FROM_CONSTANT("tmp/test.cdb")))
         {
         x.UFile::ftruncate(40000);
         x.UFile::memmap(PROT_READ | PROT_WRITE);

         istrstream is(os.str(), os.pcount());

         is >> x; // NB: this do ftruncate() e munmap()...
         }
      }
}
