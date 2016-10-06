// test_ssh_client.cpp

#include <ulib/file.h>
#include <ulib/ssh/net/sshsocket.h>

static const char* getArg(const char* param) { return (param && *param ? param : 0); }

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   USSHSocket x;
   UString hostname(argv[2]);

   x.setVerbosity(4);
   x.setUser(getArg(argv[1]));
   x.setKey(getArg(argv[3]), getArg(argv[4]));

   if (x.connectServer(hostname, 22))
      {
      UString dati = UFile::contentOf(UString(argv[5]));

      const char* str = dati.data();
      uint32_t size   = dati.size();

      if (x.send(str, size) == size)
         {
         cout.write(str, size);
         cout << '\n';

         x.sendEOF();

         char buffer[4096];

         while (true)
            {
            size = x.recv(buffer, sizeof(buffer));

            if (size > 0)
               {
               cout.write(buffer, size);

               continue;
               }

            if (size != sizeof(buffer)) break;
            }

         cout << '\n';
         }

      exit(0);
      }
}
