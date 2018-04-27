// PEC_check_namefile.cpp

/*-----------------------------------------------------------------------------------------------------------------------------
Per ogni namefile (PECx-yyyy-mm-dd.log.p7m), viene elaborata l'estensione temporale del righe di log contenute e suggerito
un nome di link simbolico (PEC1-2007-03-22#2007-03-23.lnk) che specifica la corretta informazione per successive elaborazioni...
-----------------------------------------------------------------------------------------------------------------------------*/

#include "PEC_report_options.h"

#define PACKAGE "PEC_check_namefile"
#define U_OPTIONS "purpose \"program to check PEC log namefile...\"\n" \
        U_OPTIONS_GEN1

#include "PEC_report.h"

static UTimeDate* inizio;
static UTimeDate* fine;

class Application : public PEC_report {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (inizio)
         {
         U_DELETE(inizio)
         U_DELETE(fine)
         }
      }

   static void resetDate()
      {
      U_TRACE(5, "Application::resetDate()")

      inizio->fromTime(u_now->tv_sec);

      fine->set(1,1,1970);
      }

   static void checkDateRange()
      {
      U_TRACE(5, "Application::checkDateRange()")

      if (*inizio > *PEC_report::date) *inizio = *PEC_report::date;
      if (*fine   < *PEC_report::date) *fine   = *PEC_report::date;
      }

   static void changeFile()
      {
      U_TRACE(5, "Application::changeFile()")

      if (PEC_report::failed) return;

      UString l(100U),
              s = inizio->strftime(U_CONSTANT_TO_PARAM("%Y-%m-%d")),
              e =   fine->strftime(U_CONSTANT_TO_PARAM("%Y-%m-%d"));

      l.snprintf(U_CONSTANT_TO_PARAM("PEC%c-%s#%s.lnk"), PEC_report::cnt[0], s.data(), e.data());

      U_INTERNAL_DUMP("l = %.*S", U_STRING_TO_TRACE(l))

      (void) sprintf(PEC_report::buffer, "suggested symbolic link name <%.*s> for file <%s>...\n",
                     U_STRING_TO_TRACE(l), PEC_report::file->getPathRelativ());

      std::cout << PEC_report::buffer;

      resetDate();
      }

   static void start()
      {
      U_TRACE(5, "Application::start()")

      inizio = new UTimeDate;
      fine   = new UTimeDate;

      resetDate();

      PEC_report::parse       = Application::checkDateRange;
      PEC_report::change_file = Application::changeFile;
      }

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      PEC_report::checklink = true;

      start_func = Application::start;

      PEC_report::run(argc, argv, env);
      }

private:
};

U_MAIN
