// action.h

#ifndef ACTION_H
#define ACTION_H 1

#include <ulib/file_config.h>
#include <ulib/net/client/smtp.h>
#include <ulib/net/client/http.h>

#include <ulib/application.h>

class Action : public UApplication {
public:

    Action() : client_http(0), data(U_CAPACITY)
      {
      U_TRACE(5, "Action::Action()")

         id[0] = customer[0] = installation[0] =           uid[0] =
      event[0] =    state[0] =  action_name[0] = error_message[0] = '\0';
      }

   ~Action()
      {
      U_TRACE(5, "Action::~Action()")
      }

   bool sendEmail();
   void processInputData(int expected);
   void writeToSTDOUT(bool ok, bool all);
   bool loadFileConfig(UString& cfg_index);
   bool sendHttpPostRequest(const UString& url, const UString& body, const char* content_type, const char* expected);

   // SERVICES

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "Action::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      UString cfg_str;

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage arg operation

      // manage file configuration

      (void) loadFileConfig(cfg_str);

      UApplication::exit_value = 1;
      }

protected:
   UFileConfig cfg;
   USmtpClient emailClient;
   UHttpClient<USocket> client_http;
   UString data, request, request_decoded, req_tmpl, res_tmpl, err_tmpl, response, toAddress;
   char id[256], customer[128], installation[512], uid[64], event[32], state[32], action_name[32], error_message[256];
};

#endif
