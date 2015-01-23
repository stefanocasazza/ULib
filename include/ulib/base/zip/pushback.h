/* pushback.h - header for pushback.c */

#ifndef pushback_H
#define pushback_H 1

struct pb_file {
  unsigned buff_amt;
  ub1 pb_buff[RDSZ];
  int fd;
  ub1* next;
};

typedef struct pb_file pb_file;

#ifdef __cplusplus
extern "C" {
#endif

U_EXPORT int  pb_push(pb_file*, void*, int);
U_EXPORT int  pb_read(pb_file*, void*, int);
U_EXPORT void pb_init(pb_file*, int fd, ub1* data);

#ifdef __cplusplus
}
#endif

#endif
