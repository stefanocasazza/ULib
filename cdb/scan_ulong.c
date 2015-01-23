#include "scan.h"

unsigned int scan_ulong(register char *s,register unsigned long *u)
{
  register unsigned int pos; register unsigned long result;
  register unsigned long c;
  pos = 0; result = 0;
  while ((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 10)
	 { result = result * 10 + c; ++pos; }
  *u = result; return pos;
}
