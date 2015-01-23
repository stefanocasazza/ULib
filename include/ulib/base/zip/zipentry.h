/* zipentry.h - generic defines, struct defs etc.  */

#ifndef zipentry_H
#define zipentry_H 1

/* Amount of bytes to read at a time. You can change this to optimize for your system */
#define RDSZ 4096

#ifdef ULIB_HASH_H
typedef uint8_t  ub1;
typedef uint16_t ub2;
typedef uint32_t ub4;
#else
/* Change these to match your system:
   ub1 == unsigned 1 byte word
   ub2 == unsigned 2 byte word
   ub4 == unsigned 4 byte word
*/
#  if SIZEOF_UNSIGNED_CHAR == 1
typedef unsigned char ub1;
#  else
typedef u_int8_t ub1;
#  endif
#  if SIZEOF_SHORT == 2
typedef unsigned short ub2;
#  elif SIZEOF_INT == 2
typedef unsigned int ub2;
#  else
typedef uint16_t ub2;
#  endif
#  if SIZEOF_INT == 4
typedef unsigned int ub4;
#  elif SIZEOF_LONG == 4
typedef unsigned long ub4;
#  elif SIZEOF_LONG_LONG == 4
typedef unsigned long long ub4;
#  else
typedef uint32_t ub4;
#  endif
#endif

struct zipentry {
  ub2 mod_time;
  ub2 mod_date;
  ub4 crc;
  ub4 csize;
  ub4 usize;
  ub4 offset;
  ub1 compressed;
  char* filename;
  struct zipentry* next_entry;
};

typedef struct zipentry zipentry;

#endif
