// test_ssl_client.cpp

#include <ulib/net/client/client.h>

static const char* getArg(const char* param) { return (param && *param ? param : 0); }

int
U_EXPORT main (int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString host(argv[8]);
   UClient<USSLSocket> x(0);

   // Check server certificates agains our known trusted certificate

   if (((USSLSocket*)x.socket)->setContext(0, getArg(argv[1]), getArg(argv[2]), getArg(argv[3]), getArg(argv[4]), getArg(argv[5]), atoi(argv[6])) &&
       ((USSLSocket*)x.socket)->connectServer(host, 8080))
      {
      U_DUMP("getPeerCertificate() = %p", ((USSLSocket*)x.socket)->getPeerCertificate())

      const char* str = argv[7];
      int size        = strlen(str);

      for (int i = 0; i < 2; ++i)
         {
         if (((USSLSocket*)x.socket)->send(str, size) == size)
            {
            cout << str << '\n';

            char buffer[1024];

            if (((USSLSocket*)x.socket)->recv(buffer, size) == size)
               {
               cout.write(buffer, size);

               cout << '\n';
               }
            }
         }
      }
}
