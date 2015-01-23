#include "auto_home.h"

void hier()
{
  h(auto_home,-1,-1,02755);
  d(auto_home,"bin",-1,-1,02755);
  d(auto_home,"man",-1,-1,0755);
  d(auto_home,"man/man1",-1,-1,0755);
  d(auto_home,"man/man3",-1,-1,0755);


  c(auto_home,"bin","cdbget",-1,-1,0755);
  c(auto_home,"bin","cdbmake",-1,-1,0755);
  c(auto_home,"bin","cdbdump",-1,-1,0755);
  c(auto_home,"bin","cdbstats",-1,-1,0755);
  c(auto_home,"bin","cdbtest",-1,-1,0755);
  c(auto_home,"bin","cdbmake-12",-1,-1,0755);
  c(auto_home,"bin","cdbmake-sv",-1,-1,0755);

  c(auto_home,"man/man1","cdbdump.1",-1,-1,0755);
  c(auto_home,"man/man1","cdbget.1",-1,-1,0755);
  c(auto_home,"man/man1","cdbmake.1",-1,-1,0755);
  c(auto_home,"man/man1","cdbstats.1",-1,-1,0755);
  c(auto_home,"man/man1","cdbtest.1",-1,-1,0755);

  c(auto_home,"man/man3","cdb_datalen.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_datapos.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_find.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_firstkey.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_free.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_init.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_keylen.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_keypos.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_nextkey.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_read.3",-1,-1,0644);
  c(auto_home,"man/man3","cdb_successor.3",-1,-1,0644);
}
