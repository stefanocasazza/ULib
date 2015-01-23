#include "cdb.h"
#include "seek.h"

static int doit(struct cdb *c,uint32 *kpos) {
  char buf[8];
  uint32 eod,klen;
  if (cdb_read(c,buf,4,0)) return -1;
  uint32_unpack(buf,&eod);
  if (eod<8 || eod-8<*kpos) return 0;
  c->kpos=*kpos+8;
  if (c->kpos<*kpos) return -1; /* wraparound */
  cdb_findstart(c);
  c->hslots=1;
  if (cdb_read(c,buf,8,*kpos) == -1) return -1;
  uint32_unpack(buf,&klen);
  uint32_unpack(buf+4,&c->dlen);
  c->dpos=c->kpos+klen;
  *kpos+=8+klen+c->dlen;
  return 1;
}

int cdb_firstkey(struct cdb *c,uint32 *kpos) {
  *kpos=CDB_SIZE_HASH_TABLE_POINTER;
  return doit(c,kpos);
}

int cdb_nextkey(struct cdb *c,uint32 *kpos) {
  return doit(c,kpos);
}

int cdb_successor(struct cdb *c,char *key,unsigned int klen) {
  int r;
  uint32 kpos;
  if (key) {
	 r=cdb_find(c,key,klen);
	 if (r<1) return r;
	 kpos=c->dpos+c->dlen;
  } else
	 kpos=CDB_SIZE_HASH_TABLE_POINTER;
  return doit(c,&kpos);
}
