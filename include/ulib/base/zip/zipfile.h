/* zipfile.h - defines for indexing zipfile headers */

#ifndef zipfile_H
#define zipfile_H 1

#define LOC_EXTRA   6  /* extra bytes */
#define LOC_COMP    8  /* compression method */
#define LOC_MODTIME 10 /* last modification time */
#define LOC_MODDATE 12 /* last modification date */
#define LOC_CRC     14 /* CRC */
#define LOC_CSIZE   18 /* compressed size */
#define LOC_USIZE   22 /* uncompressed size */
#define LOC_FNLEN   26 /* filename length */
#define LOC_EFLEN   28 /* extra-field length */

#define CEN_COMP    10 /* compression method */
#define CEN_MODTIME 12
#define CEN_MODDATE 14
#define CEN_CRC     16
#define CEN_CSIZE   20
#define CEN_USIZE   24
#define CEN_FNLEN   28
#define CEN_EFLEN   30
#define CEN_COMLEN  32
#define CEN_OFFSET  42

/* macros */

#define PACK_UB4(d, o, v) d[o]   = (ub1) ((v) & 0x000000ff); \
                          d[o+1] = (ub1)(((v) & 0x0000ff00) >> 8); \
                          d[o+2] = (ub1)(((v) & 0x00ff0000) >> 16); \
                          d[o+3] = (ub1)(((v) & 0xff000000) >> 24)

#define PACK_UB2(d, o, v) d[o]   = (ub1) ((v) & 0x00ff); \
                          d[o+1] = (ub1)(((v) & 0xff00) >> 8)

#define UNPACK_UB4(s, o) (ub4)s[o]             + (((ub4)s[o+1]) << 8) +\
                         (((ub4)s[o+2]) << 16) + (((ub4)s[o+3]) << 24)

#define UNPACK_UB2(s, o)  (ub2)s[o] + (((ub2)s[o+1]) << 8)

#endif
