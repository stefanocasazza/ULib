// test_unixsocket_server.cpp

#include <ulib/file_config.h>
#include <ulib/net/unixsocket.h>
#include <ulib/net/server/server.h>

class UUnixClientImage : public UClientImage<UUnixSocket> {
public:

   UUnixClientImage() : UClientImage<UUnixSocket>()
      {
      U_TRACE_CTOR(5, UUnixClientImage, "", 0)
      }

   ~UUnixClientImage()
      {
      U_TRACE_DTOR(5, UUnixClientImage)
      }

   // DEBUG

#ifdef DEBUG
   const char* dump(bool _reset) const { return UClientImage<UUnixSocket>::dump(_reset); }
#endif

protected:

   // define method VIRTUAL of class UEventFd

   virtual int handlerRead()
      {
      U_TRACE(5, "UUnixClientImage::handlerRead()")

      if (UClientImage_Base::manageRead() == U_NOTIFIER_DELETE) U_RETURN(U_NOTIFIER_DELETE);

      if (U_ClientImage_state == U_PLUGIN_HANDLER_OK)
         {
         *UClientImage_Base::wbuffer = *UClientImage_Base::rbuffer;

         return UClientImage_Base::handlerResponse();
         }

      U_RETURN(U_NOTIFIER_OK);
      }
};

U_MACROSERVER(UUnixServer, UUnixClientImage, UUnixSocket);

int
U_EXPORT main (int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UFileConfig fcg;
   UUnixServer s(U_NULLPTR);
   UString plugin_dir(argv[2]), plugin_list(argv[3]);

   UServer_Base::bipc = true;

   (void) s.name_sock->assign(argv[1]);

   UUnixSocket::setPath(s.name_sock->data());

   if (argv[4])
      {
      UString x(argv[4]);

      (void) fcg.open(x);

      s.pcfg = &fcg;
      }

   s.loadPlugins(plugin_dir, plugin_list);

   s.run();
}
