// main.cpp

#include <ulib/date.h>
#include <ulib/utility/services.h>
#include <ulib/ssl/mime/mime_pkcs7.h>

#include "DocumentClassifier.h"

#undef  PACKAGE
#define PACKAGE "doc_classifier"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for classify document specified with arg <file1 file2 ...>\"\n" \
"option c CApath            1 \"the path for certificates verification\" \"./CApath\"\n" \
"option t verification-time 1 \"the UTC time in YYYY-MM-DD HH:MM:SS format used for certificates verification\" \"\"\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      UString str_time   = opt['t'],
              str_CApath = opt['c'];

      if (str_time.empty() == false)
         {
         DocumentClassifier::certsVerificationTime = UTimeDate::getSecondFromTime(str_time.data(), false, "%4d-%02d-%02d+%02d:%02d:%02d");

         U_INTERNAL_DUMP("certsVerificationTime = %ld", DocumentClassifier::certsVerificationTime)
         }

      // EXTENSION

      (void) UServices::setupOpenSSLStore(U_NULLPTR, str_CApath.c_str());

      // NORMAL

      for (int i = optind; i < argc; ++i)
         {
         DocumentClassifier dc(argv[i]);

         std::cout << dc.whatIs()           << '\n';
         std::cout << dc.print()            << '\n';
         std::cout << dc.printLabel()       << '\n';
         std::cout << dc.printCertificate() << '\n';
         }
      }
};

U_MAIN
