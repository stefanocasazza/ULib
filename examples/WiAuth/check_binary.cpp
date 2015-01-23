// check_binary.cpp

#include <ulib/file.h>
#include <ulib/base/utility.h>

#undef  PACKAGE
#define PACKAGE "check_binary"

#define ARGS "[path of file]"

#define U_OPTIONS \
"purpose 'check for binary character in file'\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      UString content = UFile::contentOf(argv[optind]);

      if (content)
         {
         int color;
         const char* msg;

         int32_t n        =                  content.size();
         unsigned char* s = (unsigned char*) content.data();

         msg = (u_isText(s, n) ?  (color = GREEN, "is") : (color = RED, "NOT"));

         U_MESSAGE("%W%s%W Text", color, msg, RESET);

         msg = (u_isPrintable((const char*)s, n, false) ? (color = GREEN, "is") : (color = RED, "NOT"));

         U_MESSAGE("%W%s%W Printable", color, msg, RESET);

         msg = (u_isUTF8(s, n) ? (color = GREEN, "is") : (color = RED, "NOT"));

         U_MESSAGE("%W%s%W UTF-8", color, msg, RESET);

         msg = (u_isUTF16(s, n) ? (color = GREEN, "is") : (color = RED, "NOT"));

         U_MESSAGE("%W%s%W UTF-16", color, msg, RESET);

         if (content.isBinary())
            {
            unsigned char c = *s;

            for (int32_t i = 0; i < n; ++i)
               {
               if (u__istext(c) == false)
                  {
                  int32_t n1 = 1, n2 = 1;

                  while (u__islterm(s[-n1]) == false)
                     {
                     if ((i - ++n1) <= 0) break;
                     }

                  while (u__islterm(s[n2]) == false)
                     {
                     if ((i + ++n2) >= n) break;
                     }

                  U_MESSAGE("char %W%C%W at pos 0%o(0x%x) %Wnot text%W%s%W: %.*s%W%c%W%.*s", RED, c, RESET, i, i,
                              MAGENTA, BRIGHTCYAN, (u__isprint(c) ? "but printable" : ", not printable"), RESET, n1, s-n1, RED, c, RESET, n2, s+1);
                  }

               c = *(++s);
               }
            }
         }
      }

private:
};

U_MAIN
