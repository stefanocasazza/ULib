#include "uint32.h"
#include "scan.h"
#include "strerr.h"
#include "cdb_make.h"

#define FATAL "testzero: fatal: "

void die_write(void)
{
  strerr_die2sys(111,FATAL,"unable to write: ");
}

static char key[4];
static char data[65536];
struct cdb_make c;

main(int argc,char **argv)
{
  int fd;
  unsigned long loop;

  if (!*argv) _exit(0);
  if (!*++argv) _exit(0);
  scan_ulong(*argv,&loop);

  if (cdb_make_start(&c,1) == -1) die_write();

  while (loop) {
	 uint32_pack(key,--loop);
	 if (cdb_make_add(&c,key,4,data,sizeof data) == -1) die_write();
  }

  if (cdb_make_finish(&c) == -1) die_write();
  _exit(0);
}
