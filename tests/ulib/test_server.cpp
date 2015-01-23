// test_server.cpp

#include <ulib/file_config.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/container/vector.h>
#include <ulib/net/server/server.h>

U_MACROSERVER(UServerExample, UClientImage<UTCPSocket>, UTCPSocket);

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UFileConfig fcg;
   UServerExample server(0);
   UString plugin_dir(argv[1]), plugin_list(argv[2]);

   if (argv[3])
      {
      fcg.load(UString(argv[3]));

      server.cfg = &fcg;
      }

   server.loadPlugins(plugin_dir, plugin_list);

   server.port = 8080;

   server.run();
}
