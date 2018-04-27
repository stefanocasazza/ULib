// server.cpp

#include <ulib/file_config.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>

#undef  PACKAGE
#define PACKAGE "simple server"
#undef  ARGS
#define ARGS "<file_config>"

#define U_OPTIONS \
"purpose \"simple server for testing, read config file specified with arg <file_config>...\"\n"

#include <ulib/application.h>

static UVector<UString>* request_response;

class UClientImageExample : public UClientImage<UTCPSocket> {
public:

   UClientImageExample() : UClientImage<UTCPSocket>()
      {
      U_TRACE_CTOR(5, UClientImageExample, "")
      }

   ~UClientImageExample()
      {
      U_TRACE_DTOR(5, UClientImageExample)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UClientImage<UTCPSocket>::dump(_reset); }
#endif

protected:

   // define method VIRTUAL of class UEventFd

   virtual int handlerRead()
      {
      U_TRACE(5, "UClientImageExample::handlerRead()")

      if (UClientImage_Base::manageRead() == U_NOTIFIER_DELETE) U_RETURN(U_NOTIFIER_DELETE);

      if (U_ClientImage_state == U_PLUGIN_HANDLER_OK)
         {
#  ifndef U_LOG_DISABLE
      if (UClientImage_Base::logbuf) 
         {
         *UClientImage_Base::request = *UClientImage_Base::rbuffer;

         UClientImage_Base::logRequest();
         }
#  endif

         for (unsigned i = 0; i < request_response->size(); i += 2)
            {
            if (UServices::dosMatch(*rbuffer, (*request_response)[i]))
               {
               *UClientImage_Base::wbuffer = (*request_response)[i+1];

               break;
               }
            }

         return UClientImage_Base::handlerResponse();
         }

      U_RETURN(U_NOTIFIER_OK);
      }
};

U_MACROSERVER(UServerExample, UClientImageExample, UTCPSocket);

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      server = U_NULLPTR;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (server)           U_DELETE(server)
      if (request_response) U_DELETE(request_response)
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage config file

      if (argv[optind] == U_NULLPTR) U_ERROR("argument 'file_config' not specified");

      cfg.UFile::setPath(UString(argv[optind]));

      // -----------------------------------------
      // server - configuration parameters
      // -----------------------------------------
      // ENABLE_IPV6  flag to indicate use of ipv6
      // MSG_WELCOME  message of welcome
      // LOG_FILE     locations for file log
      // -----------------------------------------

      server = new UServerExample(&cfg);

      // load config file section REQUEST_AND_RESPONSE

      U_NEW(UVector<UString>, request_response, UVector<UString>);

      if (cfg.loadVector(*request_response) == false) U_ERROR("config file '%s' not valid", cfg.getPath().data());

      server->run();
      }

private:
   UServerExample* server;
   UFileConfig cfg;
};

U_MAIN
