#include "cdb.h"
#include "uint32.h"
#include "fmt.h"
#include "buffer.h"
#include "strerr.h"

#define FATAL "cdbdump: fatal: "

void die_write(void)
{
  strerr_die2sys(111,FATAL,"unable to write output: ");
}
void put(char *buf,unsigned int len)
{
  if (buffer_put(buffer_1,buf,len) == -1) die_write();
}
void putflush(void)
{
  if (buffer_flush(buffer_1) == -1) die_write();
}

uint32 pos = 0;

void get(char *buf,unsigned int len)
{
  int r;
  while (len > 0) {
	 r = buffer_get(buffer_0,buf,len);
	 if (r == -1)
		strerr_die2sys(111,FATAL,"unable to read input: ");
	 if (r == 0)
		strerr_die2x(111,FATAL,"unable to read input: truncated file");
	 pos += r;
	 buf += r;
	 len -= r;
  }
}

char buf[512];

void copy(uint32 len)
{
  unsigned int x;

  while (len) {
	 x = sizeof buf;
	 if (len < x) x = len;
	 get(buf,x);
	 put(buf,x);
	 len -= x;
  }
}

void getnum(uint32 *num)
{
  get(buf,4);
  uint32_unpack(buf,num);
}

char strnum[FMT_ULONG];

main()
{
  uint32 eod;
  uint32 klen;
  uint32 dlen;

  getnum(&eod);
  while (pos < CDB_SIZE_HASH_TABLE_POINTER) getnum(&dlen);

  while (pos < eod) {
	 getnum(&klen);
	 getnum(&dlen);
	 put("+",1); put(strnum,fmt_ulong(strnum,klen));
	 put(",",1); put(strnum,fmt_ulong(strnum,dlen));
	 put(":",1); copy(klen);
	 put("->",2); copy(dlen);
	 put("\n",1);
  }

  put("\n",1);
  putflush();
  _exit(0);
}
