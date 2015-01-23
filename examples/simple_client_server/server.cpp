// server.cpp

#include <ulib/file_config.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/utility/services.h>
#include <ulib/container/vector.h>
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
      U_TRACE_REGISTER_OBJECT(5, UClientImageExample, "")
      }

   ~UClientImageExample()
      {
      U_TRACE_UNREGISTER_OBJECT(5, UClientImageExample)
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

      u_clientimage_flag.u = 0;

      if (UClientImage_Base::genericRead() == false)
         {
         if (U_ClientImage_state == U_PLUGIN_HANDLER_AGAIN) U_RETURN(U_NOTIFIER_OK); // NOT BLOCKING...

         U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state, U_PLUGIN_HANDLER_ERROR)

         U_RETURN(U_NOTIFIER_DELETE);
         }

#ifdef U_LOG_ENABLE
   if (UClientImage_Base::logbuf) 
      {
      *UClientImage_Base::request = *UClientImage_Base::rbuffer;

      UClientImage_Base::logRequest();
      }
#endif

      for (unsigned i = 0; i < request_response->size(); i += 2)
         {
         if (UServices::match(*rbuffer, (*request_response)[i]))
            {
            *UClientImage_Base::wbuffer = (*request_response)[i+1];

            break;
            }
         }

      return UClientImage_Base::handlerResponse();
      }
};

U_MACROSERVER(UServerExample, UClientImageExample, UTCPSocket);

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      server = 0;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (server)           delete server;
      if (request_response) delete request_response;
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage config file

      if (argv[optind] == 0) U_ERROR("argument 'file_config' not specified");

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

      request_response = U_NEW(UVector<UString>);

      if (cfg.loadVector(*request_response) == false) U_ERROR("config file '%s' not valid", cfg.getPath().data());

      server->run();
      }

private:
   UServerExample* server;
   UFileConfig cfg;
};

U_MAIN
