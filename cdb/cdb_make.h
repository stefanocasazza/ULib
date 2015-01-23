/* Public domain. */

#ifndef CDB_MAKE_H
#define CDB_MAKE_H

#include "cdb.h"
#include "buffer.h"
#include "uint32.h"

#define CDB_HPLIST 1000

struct cdb_hp { uint32 h; uint32 p; } ;

struct cdb_hplist {
  struct cdb_hp hp[CDB_HPLIST];
  struct cdb_hplist *next;
  int num;
} ;

struct cdb_make {
  char bspace[8192];
  char final[CDB_SIZE_HASH_TABLE_POINTER];
  uint32 count[CDB_NUM_HASH_TABLE_POINTER];
  uint32 start[CDB_NUM_HASH_TABLE_POINTER];
  struct cdb_hplist *head;
  struct cdb_hp *split; /* includes space for hash */
  struct cdb_hp *hash;
  uint32 numentries;
  buffer b;
  uint32 pos;
  int fd;
} ;

extern int cdb_make_start(struct cdb_make *,int);
extern int cdb_make_addbegin(struct cdb_make *,unsigned int,unsigned int);
extern int cdb_make_addend(struct cdb_make *,unsigned int,unsigned int,uint32);
extern int cdb_make_add(struct cdb_make *,char *,unsigned int,char *,unsigned int);
extern int cdb_make_finish(struct cdb_make *);

#endif
