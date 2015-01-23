// main.cpp

#include <ulib/file.h>

#undef  PACKAGE
#define PACKAGE "mail_parser"
#undef  ARGS
#define ARGS "<filename>"

#define U_OPTIONS \
"purpose \"general manager for parsing mail file specified with <filename>...\"\n" \
"option d directory 1 \"directory where to save contents of mail file\" prova"

#include <ulib/application.h>

#include <MailParserInterface.h>

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage mail file

      if (argv[optind] == NULL)
         {
         U_ERROR("mail file not specified with <filename>");
         }

      UFile mail_file;

      if (mail_file.open(argv[optind]) == false)
         {
         U_ERROR("mail file <%s> not valid", mail_file.getPath().data());
         }

      mail_file.readSize();
      mail_file.memmap(PROT_READ | PROT_WRITE, &data);

      MailParserInterface parser(data);

      if (parser.parse())
         {
         UString directory = opt['d'];

         if (directory.empty() == true)
            {
            directory = U_STRING_FROM_CONSTANT("prova");
            }

         parser.save(directory);

#     ifdef DEBUG
         parser.clear();
#     endif
         }
      }

private:
   UString data;
};

U_MAIN
