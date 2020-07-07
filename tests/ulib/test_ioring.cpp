// test_ioring.cpp

#include <ulib/net/tcpsocket.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/server/server.h>

U_MACROSERVER(UServerExample, UClientImage<UTCPSocket>, UTCPSocket);

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UFileConfig fcg;

   UServer_Base::brng = true;
   fcg.UFile::setPath(UString(argv[1]));

   UServerExample server(&fcg);

   /*
   UString plugin_dir(argv[1]), plugin_list(argv[2]);

   if (argv[3])
      {
      fcg.load(UString(argv[3]));

      server.pcfg = &fcg;
      }

   server.setDocumentRoot(UString::getStringNull());

   if (UHTTP::cache_file_mask == U_NULLPTR) U_NEW_STRING(UHTTP::cache_file_mask, UString);

   UHTTP::cache_file_mask->assign(U_CONSTANT_TO_PARAM("_off_"));

   server.loadPlugins(plugin_dir, plugin_list);

   server.port = 8080;
   */

   server.run();
}
