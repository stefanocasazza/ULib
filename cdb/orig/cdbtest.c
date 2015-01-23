#include "uint32.h"
#include "fmt.h"
#include "buffer.h"
#include "strerr.h"
#include "seek.h"
#include "cdb.h"

#define FATAL "cdbtest: fatal: "

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

void putnum(char *label,unsigned long count)
{
  put(label,strlen(label));
  put(strnum,fmt_ulong(strnum,count));
  put("\n",1);
}

char key[1024];

unsigned long numuntested = 0;
unsigned long numnotfound = 0;
unsigned long numotherpos = 0;
unsigned long numbadlen = 0;
unsigned long numfound = 0;

static struct cdb c;

main()
{
  uint32 eod;
  uint32 klen;
  uint32 dlen;
  seek_pos rest;
  int r;

  cdb_init(&c,0);

  getnum(&eod);
  while (pos < 2048) getnum(&dlen);

  while (pos < eod) {
	 getnum(&klen);
	 getnum(&dlen);
	 if (klen > sizeof key) {
		++numuntested;
		while (klen) { get(key,1); --klen; }
	 }
	 else {
		get(key,klen);
		rest = seek_cur(0);
		switch(cdb_find(&c,key,klen)) {
	case -1:
	  die_read();
	case 0:
	  ++numnotfound;
	  break;
	default:
	  if (cdb_datapos(&c) != pos)
		 ++numotherpos;
	  else
		 if (cdb_datalen(&c) != dlen)
			++numbadlen;
		 else
			++numfound;
		}
		if (seek_set(0,rest) == -1) die_read();
	 }
	 while (dlen) { get(key,1); --dlen; }
  }
  
  putnum("found: ",numfound);
  putnum("different record: ",numotherpos);
  putnum("bad length: ",numbadlen);
  putnum("not found: ",numnotfound);
  putnum("untested: ",numuntested);
  putflush();
  _exit(0);
}
