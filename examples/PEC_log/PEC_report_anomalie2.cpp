// PEC_report_anomalie2.cpp

#include "PEC_report_options.h"

#define PACKAGE   "PEC_report_anomalie2"
#define U_OPTIONS "purpose \"program for report PEC anomalie (require data sorted for message id)...\"\n" \
        U_OPTIONS_GEN1 \
        U_OPTIONS_GEN2

#include "PEC_report_anomalie.h"

class Application : public PEC_report_anomalie {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (Messaggio::msg) delete Messaggio::msg;
      }

   static void parseLineForAnomalie()
      {
      U_TRACE(5, "Application::parseLineForAnomalie()")

      // Per ogni riga [ ESCLUSI tipo=non-accettazione o senza tipo es: tipo= (BustaAnomalia)) ] si traccia lo stato...

      if (checkLineForAnomalie())
         {
         bool bnew = false;

         if (Messaggio::msg == 0)
            {
            bnew = true;

            Messaggio::msg = new Messaggio();
            }
         else if (Messaggio::msg->identifier != *identifier)
            {
            bnew = true;

            if (Messaggio::msg->isAnomalia()) reportAnomalie(0, Messaggio::msg);

            delete Messaggio::msg;

            Messaggio::msg = new Messaggio();
            }

         processLine(bnew);
         }
      }

   static void start()
      {
      U_TRACE(5, "Application::start()")

      PEC_report_anomalie::start();

      // setting for anomalie

      PEC_report::parse = Application::parseLineForAnomalie;
      }

   static void end()
      {
      U_TRACE(5, "Application::end()")

      if (Messaggio::msg && Messaggio::msg->isAnomalia()) reportAnomalie(0, Messaggio::msg);

      PEC_report_anomalie::end();
      }

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      PEC_report::non_firmati = true;

      end_func   = Application::end;
      start_func = Application::start;

      PEC_report::run(argc, argv, env);
      }

private:
};

U_MAIN
