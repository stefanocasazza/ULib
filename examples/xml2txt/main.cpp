// xml2txt.cpp

#include <ulib/file.h>
#include <ulib/xml/expat/xml2txt.h>

#undef  PACKAGE
#define PACKAGE "xml2txt"
#undef  ARGS
#define ARGS "XML"

#define U_OPTIONS \
"purpose \"Expects xml as input, outputs text only\"\n" \
"option t tag     1 \"list of tag separated by comma to use as filter\" \"\"\n" \
"option x exclude 0 \"the tag listed are excluded\" \"\"\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      UString tag_list;
      bool excluded = false;

      if (UApplication::isOptions())
         {
         tag_list = opt['t'];

         if (opt['x'].empty() == false) excluded = true;
         }

      UXml2Txt converter(tag_list, excluded, false);

      if (converter.parse(UFile::contentOf(argv[optind])) == false)
         {
         UApplication::exit_value = 1;

         U_ERROR("xml parsing error: %s", converter.getErrorMessage()); 
         }

     (void) write(1, U_STRING_TO_PARAM(converter.getText()));
      }
};

U_MAIN
