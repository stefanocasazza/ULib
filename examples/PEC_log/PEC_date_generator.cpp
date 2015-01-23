// PEC_date_generator.cpp

#include <ulib/date.h>

#undef  PACKAGE
#define PACKAGE "PEC_date_generator"
#undef  VERSION
#define VERSION "1.0"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for printing on stdout of a sequence of intervals of dates...\"\n" \
"option f from         1 \"starting date (dd/mm/yyyy)\" \"\"\n" \
"option t to           1 \"ending date (dd/mm/yyyy)\" \"\"\n" \
"option d days_between 1 \"number of days between dates\" \"\"\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions() == false) U_ERROR("parameters <from> and <to> are mandatory");

      UString cfg_from = opt['f'],
              cfg_to   = opt['t'],
              cfg_day  = opt['d'];

      if (cfg_from.empty() ||
            cfg_to.empty()) U_ERROR("parameters <from> and <to> are mandatory");

      UTimeDate from(cfg_from.c_str()),
              to(cfg_to.c_str());

      if (from.isValid() == false) U_ERROR("starting date <%s> not valid", cfg_from.data());
      if (  to.isValid() == false) U_ERROR("ending date <%s> not valid", cfg_to.data());

      int ndays        = from.daysTo(to),
          days_between = cfg_day.strtol();

      U_INTERNAL_DUMP("ndays = %d days_between = %d", ndays, days_between)

      if (ndays < days_between)
         {
         U_WARNING("the number of days between <from> and <to> date is minor of param <days_between>...");

         std::cout << from << ' ' << to << '\n';
         }
      else
         {
         --days_between;

         if (days_between < 0) days_between = 0;

         while (true)
            {
            std::cout << from << ' ';

            if (days_between)
               {
               from += days_between; 

               if (from > to) from = to;
               }

            std::cout << from << '\n';

            from += 1;

            if (from > to) break;
            }
         }
      }

private:
};

U_MAIN
