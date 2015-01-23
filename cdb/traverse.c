#include <unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include "cdb.h"
#include "fmt.h"

main(int argc,char *argv[]) {
  int fd;
  struct cdb c;
  char strnum[FMT_ULONG];
  uint32 kpos;
  fd=open(argc>1?argv[1]:"foo.cdb",O_RDONLY);
  assert(fd>=0);
  cdb_init(&c,fd);
  if (cdb_firstkey(&c,&kpos)==1) {
	 do {
		uint32 kp,klen,dp,dlen;
		char *key,*data;
		kp=cdb_keypos(&c);
		klen=cdb_keylen(&c);
		dp=cdb_datapos(&c);
		dlen=cdb_datalen(&c);
		{
	key=alloca(klen);
	data=alloca(dlen);
/*	printf("%lu %lu; %lu %lu\n",kp,klen,dp,dlen); */
	assert(cdb_read(&c,key,klen,kp)==0);
	assert(cdb_read(&c,data,dlen,dp)==0);
	write(1,"+",1);
	write(1,strnum,fmt_ulong(strnum,klen));
	write(1,",",1);
	write(1,strnum,fmt_ulong(strnum,dlen));
	write(1,":",1);
	write(1,key,klen);
	write(1,"->",2);
	write(1,data,dlen);
	write(1,"\n",1);
		}
	 } while (cdb_nextkey(&c,&kpos)==1);
	 write(1,"\n",1);
  }
}
