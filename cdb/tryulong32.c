void main()
{
  unsigned long u;
  u = 1;
  u += u; u += u; u += u; u += u; u += u; u += u; u += u; u += u;
  u += u; u += u; u += u; u += u; u += u; u += u; u += u; u += u;
  u += u; u += u; u += u; u += u; u += u; u += u; u += u; u += u;
  u += u; u += u; u += u; u += u; u += u; u += u; u += u; u += u;
  if (!u) _exit(0);
  _exit(1);
}
