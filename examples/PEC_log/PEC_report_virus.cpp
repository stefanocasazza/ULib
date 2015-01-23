// PEC_report_virus.cpp

/*-------------------------------------------------------------------------------
SECONDO REPORT: numero totale di virus rilevati in ingresso e uscita (con 
tutte le info necessarie al tracciamento del msg e della casella)

Virus in ingresso:
I virus possono essere riscontrati in un messaggio sottomesso da un utente del 
gestore o nella busta di trasporto inviata da un altro gestore, rimangono 
fuori dal conteggio i virus ricevuti da utenti di posta ordinaria in quanto 
in questo caso la comunicazione SMTP sarebbe interrotta e il messaggio 
rifiutato.

1) Per ogni "non accettazione" dove "errore=virus" va incrementato il 
contatore dei virus ricevuti ovviamente mantenendosi l'identificativo e le 
caselle mittente e destinatarie

2) Per ogni "rilevazione virus" dove il campo "ricezione" ("consegna" nella successiva versione 2.1)
contengono utenti del gestore va incrementato il contatore dei virus ricevuti ovviamente 
mantenendo l'identificativo e le caselle mittente e ricezione

Virus in uscita:
Per virus in uscita si intende che il controllo antivirus non è stato efficace 
ed è stata prodotta una busta di trasporto che per qualche altro gestore 
contiene virus

1) Per ogni "rilevazione virus" dove il campo "mittente" contiene un utente 
del gestore va incrementato il contatore di virus inviati ovviamente 
mantenendo l'identificativo e le caselle mittente, destinatari e ricezione
*/

#include "PEC_report_options.h"

#define PACKAGE "PEC_report_virus"
#define U_OPTIONS "purpose \"program for report PEC virus...\"\n" \
        U_OPTIONS_GEN1 \
        U_OPTIONS_GEN2

#include "PEC_report.h"

#define U_XML_VIRUS_START                                                              \
"    <NumMsgVirusInOut>\n"

#define U_XML_VIRUS_SUMMARY                                                            \
"      <NumMsgVirusInOut-Summary>\n"                                                   \
"        <NumMsgVirusInOut-InstallationId>%.*s</NumMsgVirusInOut-InstallationId>\n"    \
"        <NumMsgVirusInOut-CountInput>%u</NumMsgVirusInOut-CountInput>\n"              \
"        <NumMsgVirusInOut-CountOutput>%u</NumMsgVirusInOut-CountOutput>\n"            \
"      </NumMsgVirusInOut-Summary>\n"

#define U_XML_VIRUS_ENTRY_START                                                        \
"      <NumMsgVirusInOut-Entry>\n"                                                     \
"        <NumMsgVirusInOut-%s>\n"                                                      \
"          <NumMsgVirusInOut-Identifier>%.*s</NumMsgVirusInOut-Identifier>\n"          \
"          <NumMsgVirusInOut-Date>%s</NumMsgVirusInOut-Date>\n"                        \
"          <NumMsgVirusInOut-InstallationId>%.*s</NumMsgVirusInOut-InstallationId>\n"  \
"          <NumMsgVirusInOut-Sender>%.*s</NumMsgVirusInOut-Sender>\n"

#define U_XML_VIRUS_ENTRY_Recipients                                                   \
"          <NumMsgVirusInOut-Recipients>%.*s</NumMsgVirusInOut-Recipients>\n"
#define U_XML_VIRUS_ENTRY_Rejected                                                     \
"          <NumMsgVirusInOut-Rejected>%.*s</NumMsgVirusInOut-Rejected>\n"

#define U_XML_VIRUS_ENTRY_END                                                          \
"        </NumMsgVirusInOut-%s>\n"                                                     \
"      </NumMsgVirusInOut-Entry>\n"

#define U_XML_VIRUS_END                                                                \
"    </NumMsgVirusInOut>\n"

#define U_VIRUS_IN   counter_virus_InOut[id_index][0]
#define U_VIRUS_OUT  counter_virus_InOut[id_index][1]

static const char* form;
static uint32_t counter_virus_InOut[U_MaxID][2];

#define U_PARSE_ERROR_MESSAGE_UNEXPECTED_VALUE  "not found expected value <error=virus> - id = <%.*s>"

class Application : public PEC_report {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   static bool reportEntry(const UString& elem)
      {
      U_TRACE(5, "Application::reportEntry(%.*S)", U_STRING_TO_TRACE(elem))

      UString tmp(U_CAPACITY);

      UXMLEscape::encode(elem, tmp);

      (void) sprintf(buffer, form, U_STRING_TO_TRACE(tmp));

      std::cout << buffer;

      U_RETURN(false);
      }

   static void parseLineForVirus()
      {
      U_TRACE(5, "Application::parseLineForVirus()")

      if (tipology[U_non_accettazione]  == false &&
          tipology[U_rilevazione_virus] == false) return;

      /*
      -------------------------------------------------------
      si parte dalla fine record <-> "delimitatore..." ':'

      ERRORE 2 (errore=virus)
      ci sono due possibilita':
      if (rilevazione_virus) (obbligatorio)
      if (non_accettazione) (check valore campo)
      -------------------------------------------------------
      */

      bool input        = true;
      int U_field       = U_ricezione;
      bool errore_virus = PEC_report::checkValueField(U_errore, U_STRING_FROM_CONSTANT("virus"));

      if (tipology[U_rilevazione_virus])
         {
         if (errore_virus == false)
            {
            U_WARNING(U_PARSE_ERROR_MESSAGE_UNEXPECTED_VALUE, U_STRING_TO_TRACE(*identifier));
            }

         // from version > 2.0...

         if (getValueField(U_consegna).empty() == false) U_field = U_consegna;

         input = isDomainField(U_field);
         }
      else if (errore_virus)
         {
         U_INTERNAL_ASSERT(tipology[U_non_accettazione])

      // input = true;
         }
      else
         {
         U_INTERNAL_ASSERT(tipology[U_non_accettazione])

         return;
         }

      if (input) U_VIRUS_IN  += 1;
      else       U_VIRUS_OUT += 1;

      U_INTERNAL_DUMP("U_VIRUS_IN [%u] = %u", id_index, U_VIRUS_IN)
      U_INTERNAL_DUMP("U_VIRUS_OUT[%u] = %u", id_index, U_VIRUS_OUT)

      // start report entry

      UString tmp1(U_CAPACITY), tmp2(U_CAPACITY);

      UXMLEscape::encode(*identifier, tmp1);
      UXMLEscape::encode(*mittente,   tmp2);

      (void) sprintf(buffer, U_XML_VIRUS_ENTRY_START, (input ? "Input" : "Output"),
                             U_STRING_TO_TRACE(tmp1),
                             PEC_report::getData(),
                             U_STRING_TO_TRACE(*id), 
                             U_STRING_TO_TRACE(tmp2));

      std::cout << buffer;

      // report entry recipients

      form = U_XML_VIRUS_ENTRY_Recipients;

      (void) PEC_report::callForAllEntryField(U_destinatari, Application::reportEntry);

      if (tipology[U_rilevazione_virus])
         {
         // report entry rejected

         form = U_XML_VIRUS_ENTRY_Rejected;

         (void) PEC_report::callForAllEntryField(U_field, Application::reportEntry);
         }

      // end report entry

      (void) sprintf(buffer, U_XML_VIRUS_ENTRY_END, (input ? "Input" : "Output"));

      std::cout << buffer;
      }

   static void start()
      {
      U_TRACE(5, "Application::start()")

      // append to report

      std::cout << U_XML_VIRUS_START;

      // setting for virus

      PEC_report::parse = Application::parseLineForVirus;
      }

   static void end()
      {
      U_TRACE(5, "Application::end()")

      /* end report

      UString elem;
      uint32_t n = vid->size();

      for (id_index = 0; id_index < n; ++id_index)
         {
         elem = vid->at(id_index);

         (void) sprintf(buffer, U_XML_VIRUS_SUMMARY, U_STRING_TO_TRACE(elem), U_VIRUS_IN, U_VIRUS_OUT);

         std::cout << buffer;
         }
      */

      std::cout << U_XML_VIRUS_END;
      }

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      end_func   = Application::end;
      start_func = Application::start;

      PEC_report::run(argc, argv, env);
      }

private:
};

U_MAIN
