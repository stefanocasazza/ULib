// production.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "production"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for workflow production...\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include "action.h"

class Application : public Action {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      Action::run(argc, argv, env);

      // read stdin and scanf

      Action::processInputData(5);

      // --------------------------------------------------------------------------------------------------------------
      // configuration parameters
      // --------------------------------------------------------------------------------------------------------------
      // ARCHIVE_TEMPLATE                    printf form to output request of archive
      // ELECTRONIC_ARCHIVATION_SERVICE_URL  url to do request of archive
      // --------------------------------------------------------------------------------------------------------------

      UString arch_tmpl = UFile::contentOf(cfg[U_STRING_FROM_CONSTANT("ARCHIVE_TEMPLATE")]),
              url       =                  cfg[U_STRING_FROM_CONSTANT("ELECTRONIC_ARCHIVATION_SERVICE_URL")];

      UString body(arch_tmpl.size() + request_decoded.size() + (u__strlen(uid, __PRETTY_FUNCTION__) * 3) + 100U);

      body.snprintf(U_STRING_TO_PARAM(arch_tmpl), uid, uid, uid, U_STRING_TO_TRACE(request_decoded));

      bool ok = Action::sendHttpPostRequest(url, body, "multipart/form-data; boundary=4MYWPDUi9kH5-ipE_f6CiZXFFn4SaQQOj", "OK") && Action::sendEmail();

      Action::writeToSTDOUT(ok, true);
      }
};

U_MAIN
