/* signer.cpp */

#include "window.h"

#include <ulib/file_config.h>

#undef  PACKAGE
#define PACKAGE "signer"
#undef  VERSION
#define VERSION "1.0"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for sign document...\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

HINSTANCE hinstance;

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      UString cfg_str;

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage arg operation

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("signer.cfg");

      cfg.UFile::setPath(cfg_str);

      // --------------------------------------------------------------------------------------------------------------
      // configuration parameters
      // --------------------------------------------------------------------------------------------------------------
      // --------------------------------------------------------------------------------------------------------------

      // form_template = cfg[U_STRING_FROM_CONSTANT("TEMPLATE")];

      hinstance = GetModuleHandle(0);

      Window MainWindow;
      //PropSheet MainWindow;

      // Initialize common controls
      InitCommonControls();

      // Init window class lib
      Window::SetAppInstance(hinstance);

      // Create pages

      // Add pages to sheet

      // Create the PropSheet main window
      MainWindow.Create();

      // Clean exit.. save user options.
      }

private:
   UFileConfig cfg;

   U_APPLICATION_PRIVATE
};

U_MAIN
