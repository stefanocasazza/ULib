/* Public domain. */

#include "str.h"

unsigned int strlen(const char *s)
{
  register const char *t;

  t = s;
  for (;;) {
	 if (!*t) return t - s; ++t;
	 if (!*t) return t - s; ++t;
	 if (!*t) return t - s; ++t;
	 if (!*t) return t - s; ++t;
  }
}
