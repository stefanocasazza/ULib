// test_unixsocket_client.cpp

#include <ulib/net/unixsocket.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UUnixSocket x;

   if (x.connectServer(UString(argv[1]), 0))
      {
      const char* str = "Hello";
      int size        = strlen(str);

      for (int i = 0; i < 2; ++i)
         {
         if (x.send(str, size) == size)
            {
            cout << str << '\n';

            char buffer[1024];

            if (x.recv(buffer, size) == size)
               {
               cout.write(buffer, size);

               cout << '\n';
               }
            }
         }
      }
}
