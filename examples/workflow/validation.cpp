// validation.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "validation"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for workflow validation...\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include "action.h"

class Application : public Action {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      Action::run(argc, argv, env);

      // read stdin and scanf

      Action::processInputData(2);

      // --------------------------------------------------------------------------------------------------------------
      // configuration parameters
      // --------------------------------------------------------------------------------------------------------------
      // SUBJECT_TEMPLATE                 scanf form to extract data from mail header Subject:
      // SEARCH_ENGINE_FOLDER_SEARCH_URL  url to do query to search engine
      // --------------------------------------------------------------------------------------------------------------

      UString subj_tmpl = cfg[U_STRING_FROM_CONSTANT("SUBJECT_TEMPLATE")],
              url       = cfg[U_STRING_FROM_CONSTANT("SEARCH_ENGINE_FOLDER_SEARCH_URL")];

      uint32_t subject = U_STRING_FIND(request, 0, "Subject: ");

      if (subject == U_NOT_FOUND) U_ERROR("cannot find mail header subject on input data");

      // scanf mail header Subject: from request
      // ----------------------------------------------------------------------------------------------------------------------------
      // "Subject: %*[^"]\"%[^"]\",  %*[^"]\"%[^"]\", %*[^"]\"%[^"]\""
      // ----------------------------------------------------------------------------------------------------------------------------
      // Subject: Re: TUnwired processo di registrazione utente residenziale con CPE:
      //              notifica produzione CPE cliente "workflow test", indirizzo installazione "Via Volturno, 12 50100 Firenze (FI)",
      //              UID "37723e2d-d052-4c13-a1e9-134563eb9666"
      // ----------------------------------------------------------------------------------------------------------------------------

      int n = U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p", request.c_pointer(subject), subj_tmpl.data(), customer, installation, uid);

      if (n != 3) U_ERROR("scanf error on mail header subject");

      U_INTERNAL_DUMP("sscanf() customer = %S installation = %S uid = %S", customer, installation, uid)

      // query to Search Engine

      UString body(10U + u__strlen(uid, __PRETTY_FUNCTION__));

      body.snprintf(U_CONSTANT_TO_PARAM("query=%s"), uid);

      bool ok = Action::sendHttpPostRequest(url, body, "application/x-www-form-urlencoded", "1\n");

      Action::writeToSTDOUT(ok, ok);
      }
};

U_MAIN
