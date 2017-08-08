// test_log.cpp

#include <ulib/net/server/server.h>

int
U_EXPORT main (int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   u_init_ulib_hostname();
   u_init_ulib_username();

   ULog y(U_STRING_FROM_CONSTANT("$PWD/test_log.log"), 1024);

#ifdef USE_LIBZ
   y.setLogRotate("tmp");
#endif

   y.setPrefix(U_CONSTANT_TO_PARAM(U_SERVER_LOG_PREFIX));

   uint32_t i, n = (argc > 1 ? u_atoi(argv[1]) : 10);

   for (i = 0; i < n; ++i)
      {
      y.log(U_CONSTANT_TO_PARAM("message %6d - %H %U %w"), i+1);

      y.msync();
      }

   cout << "ok" << '\n';
}
