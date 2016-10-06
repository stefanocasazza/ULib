// test_file.cpp

#include <ulib/file.h>

#ifdef __MINGW32__
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#include <fstream>

// #define U_TEST_C_STR

class MyFile : public UFile {
public:

   void check_rdb()
      {
      U_TRACE(5,"check_rdb()")

      UFile cdb;
      UString name;

      name = U_STRING_FROM_CONSTANT("/tmp/test_rdb.tmp"); // evitare NFS lock...

      int result = cdb.creat(name) &&
                   cdb.ftruncate(67444);

      if (result)
         {
         cdb.memmap();

#     if defined(__CYGWIN__) || defined(__MINGW32__)
         cdb.munmap(); // for ftruncate()...
#     endif

         result = cdb.ftruncate(3258);

         U_ASSERT( result == true )

         if (result)
            {
#        if defined(__MINGW32__) || defined(__CYGWIN__)
#           ifdef   __MINGW32__
            cdb.close();
#           endif
            UFile::munmap(); // for rename()...
#        endif

            result = cdb._rename(UFile::path_relativ); // fallisce se __MINGW32__ && NFS...
            }

         U_ASSERT( result == true )

         if (result)
            {
#        if defined(__MINGW32__) || defined(__CYGWIN__)
#           ifdef   __MINGW32__
            result = cdb.open(UFile::path_relativ);
                     cdb.st_size = 3258;
#           endif
            result = cdb.memmap();
#        endif

            cdb.close();

            UFile::substitute(cdb);
            }
         }

      U_ASSERT( result == true )
      }
};

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   U_INTERNAL_DUMP("argv[0] = %S argv[1] = %S argv[2] = %S argv[3] = %S", argv[0], argv[1], argv[2], argv[3])

   UFile x;
   UString buffer(500U), tmp = U_STRING_FROM_CONSTANT("/mnt/source");

   x.setPath(tmp);
   U_ASSERT( x.getPath() == tmp )

   UFile::close( UFile::mkTemp() );

#ifndef __MINGW32__
   buffer.assign(argv[1]);
   x.setPath(U_STRING_FROM_CONSTANT("~"));
   U_ASSERT( x.getPath() == buffer )

   buffer.push_back('/');
   x.setPath(U_STRING_FROM_CONSTANT("~/"));
   U_ASSERT( x.getPath() == buffer )

   buffer.assign(argv[2]);
   x.setPath(U_STRING_FROM_CONSTANT("~root/"));
   U_ASSERT( x.getPath() == buffer )

   buffer.assign(argv[1]);
   buffer.append("/.bash_profile");
   x.setPath(U_STRING_FROM_CONSTANT("~/.bash_profile"));
   U_ASSERT( x.getPath() == buffer )

   buffer.assign(argv[2]);
   buffer.append(".bash_profile");
   x.setPath(U_STRING_FROM_CONSTANT("~root/.bash_profile"));
   U_ASSERT( x.getPath() == buffer )

   buffer.assign(argv[3]);
   x.setPath(U_STRING_FROM_CONSTANT("$PWD"));
   U_ASSERT( x.getPath() == buffer )

   buffer.append("/");
   x.setPath(U_STRING_FROM_CONSTANT("$PWD/"));
   U_ASSERT( x.getPath() == buffer )

   buffer.append("test_file.cpp");
   x.setPath(U_STRING_FROM_CONSTANT("$PWD/test_file.cpp"));
   U_ASSERT( x.getPath() == buffer )
#endif

   MyFile y;
   UString name;
   name = U_STRING_FROM_CONSTANT("/tmp/test_rdb"); // evitare NFS lock...

   if (y.creat(name) && y.lock())
      {
      y.ftruncate(6000);
      y.memmap();

      y.unlock();
      y.close();

      y.check_rdb();

      y.munmap();

      U_ASSERT( y.open(name) == true )
      U_ASSERT( y.size()     == 3258 )

      if (y.isOpen()) y.close();

      y._unlink();
      }

   bool result = UFile::chdir("tmp", true);
   U_ASSERT( result == true )

   result = UFile::mkdirs("tmp1/tmp2/tmp3/tmp4/tmp5/tmp6");
   U_ASSERT( result == true )

   result = UFile::rmdirs(U_STRING_FROM_CONSTANT("tmp1/tmp2/tmp3/tmp4/tmp5/tmp6"));
   U_ASSERT( result == true )

   cout << y.getPath() << endl;

#ifdef U_TEST_C_STR
   // test c_str()
   UString tmp0(PAGESIZE, 'a'), x(U_STRING_FROM_CONSTANT("PROVA"));
   UFile::writeTo(x, tmp0);
   UString tmp1 = UFile::contentOf(x);
   tmp1.c_str();
#endif

#ifdef U_LINUX_BUG
   // test CRESCIOLI

   UString buffer1;
   ofstream of("DA_BUTTARE");

   of << U_STRING_FROM_CONSTANT("contenuto file da buttare");
   of.close();

   UFile t(U_STRING_FROM_CONSTANT("DA_BUTTARE"));

   t.open();
   t.readSize();
   t.memmap(PROT_READ|PROT_WRITE,&buffer1);
   t.close();

   UFile::writeTo(U_STRING_FROM_CONSTANT("DA_BUTTARE"), buffer1);
#endif
}
