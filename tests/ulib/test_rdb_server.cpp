// test_rdb_server.cpp

#include <ulib/db/rdb.h>
#include <ulib/file_config.h>
#include <ulib/net/server/server_rdb.h>

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UCDB y(false);
   UFileConfig cfg;
   off_t sz = 30000L;
   UString dbname(argv[2]);

   cfg.UFile::setPath(UString(argv[1], strlen(argv[1])));

   URDBServer s(&cfg, false);

   if (y.UFile::creat(dbname))
      {
      y.UFile::ftruncate(sz);
      y.UFile::memmap(PROT_READ | PROT_WRITE);

      cin >> y; // do ftruncate() and munmap()...

      y.UFile::close();
      y.UFile::reset();

      if (s.open(dbname)) s.run();
      }
}
