#include "uint32.h"

void uint32_pack(char s[4],uint32 u)
{
  s[0] = u & 255;
  u >>= 8;
  s[1] = u & 255;
  u >>= 8;
  s[2] = u & 255;
  s[3] = u >> 8;
}

void uint32_pack_big(char s[4],uint32 u)
{
  s[3] = u & 255;
  u >>= 8;
  s[2] = u & 255;
  u >>= 8;
  s[1] = u & 255;
  s[0] = u >> 8;
}
