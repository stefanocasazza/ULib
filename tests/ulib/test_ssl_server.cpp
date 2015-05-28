// test_ssl_server.cpp

#include <ulib/file_config.h>
#include <ulib/ssl/certificate.h>
#include <ulib/net/server/server.h>
#include <ulib/ssl/net/sslsocket.h>

class USSLClientImage : public UClientImage<USSLSocket> {
public:

   USSLClientImage() : UClientImage<USSLSocket>()
      {
      U_TRACE_REGISTER_OBJECT(5, USSLClientImage, "", 0)
      }

   ~USSLClientImage()
      {
      U_TRACE_UNREGISTER_OBJECT(5, USSLClientImage)
      }

   // DEBUG

#ifdef DEBUG
   const char* dump(bool _reset) const { return UClientImage<USSLSocket>::dump(_reset); }
#endif

protected:

   // define method VIRTUAL of class UEventFd

   virtual int handlerRead()
      {
      U_TRACE(5, "USSLClientImage::handlerRead()")

      if ((UClientImage_Base::prepareForRead(), UClientImage_Base::genericRead()) == false)
         {
         if (U_ClientImage_state == U_PLUGIN_HANDLER_AGAIN) U_RETURN(U_NOTIFIER_OK); // NOT BLOCKING...

         U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state, U_PLUGIN_HANDLER_ERROR)

         U_RETURN(U_NOTIFIER_DELETE);
         }

      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state, U_PLUGIN_HANDLER_GO_ON)

      static bool binit;

      if (binit == false)
         {
         binit = true;

         X509* x509 = ((USSLSocket*)socket)->getPeerCertificate();

         if (x509 == 0 &&
             ((USSLSocket*)socket)->askForClientCertificate())
            {
            x509 = ((USSLSocket*)socket)->getPeerCertificate();

            U_INTERNAL_ASSERT_DIFFERS(x509, 0)
            }

         if (x509) cerr << UCertificate(x509).print();
         }

      *UClientImage_Base::wbuffer = *UClientImage_Base::rbuffer;

      return UClientImage_Base::handlerResponse();
      }
};

U_MACROSERVER(USSLServer, USSLClientImage, USSLSocket);

static const char* getArg(const char* param) { return (param && *param ? param : 0); }

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UFileConfig fcg;
   USSLServer server(0);
   UString plugin_dir(argv[7]), plugin_list(argv[8]);

   if (argv[9])
      {
      UString x(argv[9]);

      (void) fcg.open(x);

      server.cfg = &fcg;
      }

   UServer_Base::bssl = true;

   server.loadPlugins(plugin_dir, plugin_list);

   // Load our certificate

   ((USSLSocket*)server.socket)->setContext(0, getArg(argv[1]), getArg(argv[2]), getArg(argv[3]), getArg(argv[4]), getArg(argv[5]), atoi(argv[6]));

   server.port = 8080;

   server.run();
}
