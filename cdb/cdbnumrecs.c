#include "uint32.h"
#include "fmt.h"
#include "buffer.h"
#include "strerr.h"
#include "seek.h"
#include "cdb.h"

#define FATAL "cdbnumrecs: fatal: "

void die_read(void)
{
  strerr_die2sys(111,FATAL,"unable to read input: ");
}
void die_write(void)
{
  strerr_die2sys(111,FATAL,"unable to write output: ");
}
void put(char *buf,unsigned int len)
{
  if (buffer_put(buffer_1small,buf,len) == -1) die_write();
}
void putflush(void)
{
  if (buffer_flush(buffer_1small) == -1) die_write();
}

uint32 pos = 0;

void get(char *buf,unsigned int len)
{
  int r;
  while (len > 0) {
	 r = buffer_get(buffer_0,buf,len);
	 if (r == -1) die_read();
	 if (r == 0)
		strerr_die2x(111,FATAL,"unable to read input: truncated file");
	 pos += r;
	 buf += r;
	 len -= r;
  }
}

void getnum(uint32 *num)
{
  char buf[4];
  get(buf,4);
  uint32_unpack(buf,num);
}

char strnum[FMT_ULONG];

void putnum(unsigned long count)
{
  put(strnum,fmt_ulong(strnum,count));
  put("\n",1);
}

static struct cdb c;
static unsigned long numrecords;

main()
{
	uint32 eod;
	uint32 klen;
	uint32 dlen;
	seek_pos rest;
	int r;

	cdb_init(&c,0);

	getnum(&eod);

	numrecords = (c.size - eod) / 8; 

	putnum(numrecords);
	putflush();
	_exit(0);
}
