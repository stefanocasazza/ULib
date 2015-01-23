#include "buffer.h"

int buffer_copy(buffer *bout,buffer *bin)
{
  int n;
  char *x;

  for (;;) {
	 n = buffer_feed(bin);
	 if (n < 0) return -2;
	 if (!n) return 0;
	 x = buffer_PEEK(bin);
	 if (buffer_put(bout,x,n) == -1) return -3;
	 buffer_SEEK(bin,n);
  }
}
