#include "uint32.h"

void uint32_unpack(char s[4],uint32 *u)
{
  /*
  uint32 result;

  result = (unsigned char) s[3];
  result <<= 8;
  result += (unsigned char) s[2];
  result <<= 8;
  result += (unsigned char) s[1];
  result <<= 8;
  result += (unsigned char) s[0];

  *u = result;
  */

  *u = *(uint32*)s;
}
