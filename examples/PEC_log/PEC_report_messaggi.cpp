// PEC_reportMessaggi.cpp

/*
versione 1.0
----------------------------------------------------------------------------------------------------
PRIMO REPORT: numero totale di messaggi PEC ricevuti e spediti per ogni casella

I messaggi spediti: (Output)

Ogni tipo "posta-certificata" va ad incrementare il contatore dei messaggi spediti

I messaggi ricevuti: (Input)

Qui gli scenari possono essere molteplici in quanto in ogni casella vanno a
finire diverse tipologie di messaggi PEC, per evitare equivoci possiamo
contare i messaggi classificandoli per tipologia in modo tale che si abbia la
possibilità nella fase di rendering di visualizzare o aggregare le tipologie
a piacimento.

1) Ogni "accettazione" va ad incrementare il contatore delle "accettazione" ricevute dal mittente
2) Ogni "non accettazione" va ad incrementare il contatore delle "non accettazione" ricevute dal mittente
3) Ogni "consegna" va ad incrementare il contatore delle "buste di trasporto" associato al campo "consegna"
4) Ogni "consegna" va ad incrementare il contatore delle "consegna" associato al campo "mittente" 
5) Ogni "errore consegna" va ad incrementare il contatore delle "mancata consegna" associato al campo "mittente" 
6) Ogni "busta di anomalia" incrementa il contatore delle "buste di anomalia" ricevute associato al campo "destinatari"
7) Ogni "preavviso di mancata consegna" va ad incrementare il contatore dei "preavviso di mancata consegna" ricevuti
associato al campo "mittente"

Rimarrebbero esclusi da questo conteggio le prese in carico e le rilevazioni
virus che potrebbero essere considerate di spettanza della casella speciale
utilizzata a tale scopo dal gestore nell'LDIF (versione 2.0)
----------------------------------------------------------------------------------------------------

versione 2.0
----------------------------------------------------------------------------------------------------
Nuovo parametro: casella del gestore

Ogni tipo=presa-in-carico (PresaInCarico) va a incrementare il contatore delle prese in carico
INPUT:  Se il campo mittente  contiene domini specificati
OUTPUT: Se il campo ricezione contiene domini specificati

Ogni tipo=rilevazione-virus (RilevazioneVirus) va a incrementare il contatore delle rivelazioni virus
INPUT:  Se il campo mittente  contiene domini specificati
OUTPUT: Se il campo ricezione contiene domini specificati
----------------------------------------------------------------------------------------------------
*/

#include "PEC_report_options.h"

#define PACKAGE "PEC_reportMessaggi"
#define U_OPTIONS "purpose \"program for report PEC messaggi...\"\n" \
                  "option b box              1 \"mail receipt for gds\" \"\"\n" \
        U_OPTIONS_GEN1 \
        U_OPTIONS_GEN2

#include <ulib/container/hash_map.h>
#include <ulib/utility/string_ext.h>

#include "PEC_report.h"

#define U_XML_MSG_START \
"    <NumMsgInOutPerMailbox>\n"

#define U_XML_MSG_ENTRY_START           \
"      <NumMsgInOutPerMailbox-Entry>\n" \
"        <NumMsgInOutPerMailbox-Mailbox>%.*s</NumMsgInOutPerMailbox-Mailbox>\n"

#define U_XML_MSG_ENTRY_Mailbox                                                                                \
"        <NumMsgInOutPerMailbox-Input>\n"                                                                      \
"          <NumMsgInOutPerMailbox-InstallationId>%.*s</NumMsgInOutPerMailbox-InstallationId>\n"                \
"          <NumMsgInOutPerMailbox-Accettazioni>%u</NumMsgInOutPerMailbox-Accettazioni>\n"                      \
"          <NumMsgInOutPerMailbox-NonAccettazioni>%u</NumMsgInOutPerMailbox-NonAccettazioni>\n"                \
"          <NumMsgInOutPerMailbox-BusteTrasporto>%u</NumMsgInOutPerMailbox-BusteTrasporto>\n"                  \
"          <NumMsgInOutPerMailbox-Consegne>%u</NumMsgInOutPerMailbox-Consegne>\n"                              \
"          <NumMsgInOutPerMailbox-MancateConsegne>%u</NumMsgInOutPerMailbox-MancateConsegne>\n"                \
"          <NumMsgInOutPerMailbox-BusteAnomalia>%u</NumMsgInOutPerMailbox-BusteAnomalia>\n"                    \
"          <NumMsgInOutPerMailbox-PreavvisiErroreConsegna>%u</NumMsgInOutPerMailbox-PreavvisiErroreConsegna>\n"\
"          <NumMsgInOutPerMailbox-PreseInCarico>%u</NumMsgInOutPerMailbox-PreseInCarico>\n"                    \
"          <NumMsgInOutPerMailbox-RilevazioneVirus>%u</NumMsgInOutPerMailbox-RilevazioneVirus>\n"              \
"        </NumMsgInOutPerMailbox-Input>\n"                                                                     \
"        <NumMsgInOutPerMailbox-Output>\n"                                                                     \
"          <NumMsgInOutPerMailbox-InstallationId>%.*s</NumMsgInOutPerMailbox-InstallationId>\n"                \
"          <NumMsgInOutPerMailbox-BusteTrasporto>%u</NumMsgInOutPerMailbox-BusteTrasporto>\n"                  \
"          <NumMsgInOutPerMailbox-PreseInCarico>%u</NumMsgInOutPerMailbox-PreseInCarico>\n"                    \
"          <NumMsgInOutPerMailbox-RilevazioneVirus>%u</NumMsgInOutPerMailbox-RilevazioneVirus>\n"              \
"        </NumMsgInOutPerMailbox-Output>\n"

#define U_XML_MSG_ENTRY_END \
"      </NumMsgInOutPerMailbox-Entry>\n"

#define U_XML_MSG_END \
"    </NumMsgInOutPerMailbox>\n"

#define U_INP_OUT_ERROR_MESSAGE "impossible to evaluate wether the message is received or sent - id = <%.*s>"

typedef struct CasellaCounter {
   uint32_t Accettazioni, NonAccettazioni, BusteTrasportoInp, Consegne,
            MancateConsegne, BusteAnomalia, PreavvisoMancataConsegna, BusteTrasportoOut,
            PreseInCaricoInp, PreseInCaricoOut,
            RilevazioneVirusInp, RilevazioneVirusOut;
} CasellaCounter;

class CasellaIdCounter {
public:
   UString cid;
   CasellaCounter count;

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   CasellaIdCounter(const UString& x) : cid(x)
      {
      U_TRACE(5, "CasellaIdCounter::CasellaIdCounter(%.*S)",  U_STRING_TO_TRACE(x))

      (void) memset(&count, 0, sizeof(CasellaCounter));
      }

   CasellaIdCounter(const CasellaIdCounter& c) : cid(c.cid)
      {
      U_TRACE(5, "CasellaIdCounter::CasellaIdCounter(%p)", &c)

      count.Accettazioni             = c.count.Accettazioni;
      count.NonAccettazioni          = c.count.NonAccettazioni;
      count.BusteTrasportoInp        = c.count.BusteTrasportoInp;
      count.Consegne                 = c.count.Consegne;
      count.MancateConsegne          = c.count.MancateConsegne;
      count.BusteAnomalia            = c.count.BusteAnomalia;
      count.PreavvisoMancataConsegna = c.count.PreavvisoMancataConsegna;
      count.BusteTrasportoOut        = c.count.BusteTrasportoOut;
      count.PreseInCaricoInp         = c.count.PreseInCaricoInp;
      count.PreseInCaricoOut         = c.count.PreseInCaricoOut;
      count.RilevazioneVirusInp      = c.count.RilevazioneVirusInp;
      count.RilevazioneVirusOut      = c.count.RilevazioneVirusOut;
      }

   void add(CasellaIdCounter& c)
      {
      U_TRACE(5, "CasellaIdCounter::add(%p)", &c)

      count.Accettazioni             += c.count.Accettazioni;
      count.NonAccettazioni          += c.count.NonAccettazioni;
      count.BusteTrasportoInp        += c.count.BusteTrasportoInp;
      count.Consegne                 += c.count.Consegne;
      count.MancateConsegne          += c.count.MancateConsegne;
      count.BusteAnomalia            += c.count.BusteAnomalia;
      count.PreavvisoMancataConsegna += c.count.PreavvisoMancataConsegna;
      count.BusteTrasportoOut        += c.count.BusteTrasportoOut;
      count.PreseInCaricoInp         += c.count.PreseInCaricoInp;
      count.PreseInCaricoOut         += c.count.PreseInCaricoOut;
      count.RilevazioneVirusInp      += c.count.RilevazioneVirusInp;
      count.RilevazioneVirusOut      += c.count.RilevazioneVirusOut;
      }
};

typedef UVector<CasellaIdCounter*> VCasellaIdCounter;

static UString* gds;
static CasellaIdCounter* item;
static UHashMap<VCasellaIdCounter*>* table;

class Application : public PEC_report {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (gds)
         {
         delete gds;
         delete table;
         }
      }

   static void reportCasella(void* elem)
      {
      U_TRACE(5, "Application::reportCasella(%p)", elem)

      UString str_id = ((CasellaIdCounter*)elem)->cid;

      UString tmp(U_CAPACITY);

      UXMLEscape::encode(str_id, tmp);

      (void) sprintf(buffer, U_XML_MSG_ENTRY_Mailbox,
                     U_STRING_TO_TRACE(tmp),
                     ((CasellaIdCounter*)elem)->count.Accettazioni,
                     ((CasellaIdCounter*)elem)->count.NonAccettazioni,
                     ((CasellaIdCounter*)elem)->count.BusteTrasportoInp,
                     ((CasellaIdCounter*)elem)->count.Consegne,
                     ((CasellaIdCounter*)elem)->count.MancateConsegne,
                     ((CasellaIdCounter*)elem)->count.BusteAnomalia,
                     ((CasellaIdCounter*)elem)->count.PreavvisoMancataConsegna,
                     ((CasellaIdCounter*)elem)->count.PreseInCaricoInp,
                     ((CasellaIdCounter*)elem)->count.RilevazioneVirusInp,
                     U_STRING_TO_TRACE(str_id),
                     ((CasellaIdCounter*)elem)->count.BusteTrasportoOut,
                     ((CasellaIdCounter*)elem)->count.PreseInCaricoOut,
                     ((CasellaIdCounter*)elem)->count.RilevazioneVirusOut);

      std::cout << buffer;
      }

   static bool reportMessaggi(UStringRep* key, void* list)
      {
      U_TRACE(5, "Application::reportMessaggi(%V,%p)", key, list)

      UString tmp(U_CAPACITY);

      UXMLEscape::encode(U_STRING_TO_PARAM(*key), tmp);

      (void) sprintf(buffer, U_XML_MSG_ENTRY_START, U_STRING_TO_TRACE(tmp));

      std::cout << buffer;

      ((VCasellaIdCounter*)list)->callForAllEntry(Application::reportCasella);

      std::cout << U_XML_MSG_ENTRY_END;

      U_RETURN(true);
      }

   static bool checkCasella(void* elem)
      {
      U_TRACE(5, "Application::checkCasella(%p)", elem)

      bool result = (*id == ((CasellaIdCounter*)elem)->cid);

      U_RETURN(result);
      }

   static bool casellaUpdate(const UString& key)
      {
      U_TRACE(5, "Application::casellaUpdate(%.*S)", U_STRING_TO_TRACE(key))

      U_INTERNAL_ASSERT(key)

      if (key.equalnocase(gds->rep) ||
          isDomainAddress(key))
         {
         VCasellaIdCounter* vc;

         if (table->find(key)) vc = table->elem();
         else
            {
            vc = new VCasellaIdCounter();

            key.duplicate(); // NB: need duplicate string because depends on mmap()'s content of document...

            table->insertAfterFind(key, vc);
            }

         CasellaIdCounter* c;
         uint32_t i = vc->find(Application::checkCasella);

         if (i == U_NOT_FOUND)
            {
            c = new CasellaIdCounter(*item);

            vc->push(c);
            }
         else
            {
            c = vc->at(i);

            c->add(*item);
            }
         }

      U_RETURN(false);
      }

   static void gdsUpdate(const UString& key, uint32_t& inp, uint32_t& out)
      {
      U_TRACE(5, "Application::gdsUpdate(%.*S,%u,%u)", U_STRING_TO_TRACE(key), inp, out)

      // INPUT: Se il campo mittente contiene domini specificati

      if (isDomainAddress(key)) inp = 1;

      // OUTPUT: Se il campo ricezione (o consegna from version > 2.0 nel caso di rilevazione virus) contiene domini specificati

      int U_field = U_ricezione;

      if (tipology[U_rilevazione_virus] &&
          getValueField(U_consegna).empty() == false)
         {
         U_field = U_consegna;
         }

      if (isDomainField(U_field)) out = 1;

      U_INTERNAL_DUMP("inp = %u out = %u", inp, out)

      if (inp == 0 &&
          out == 0)
         {
         U_WARNING(U_INP_OUT_ERROR_MESSAGE, U_STRING_TO_TRACE(*PEC_report::identifier));
         }
      }

   static void parseLineForMessaggi()
      {
      U_TRACE(5, "Application::parseLineForMessaggi()")

      UString key = *mittente;
      CasellaIdCounter elem(*id);

      item = &elem;

      if (tipology[U_rilevazione_virus])
         {
         // Ogni tipo "rilevazione-virus" va a incrementare il contatore delle rivelazioni virus

         gdsUpdate(key, elem.count.RilevazioneVirusInp, elem.count.RilevazioneVirusOut);

         key = *gds;
         }
      else if (tipology[U_presa_in_carico])
         {
         // Ogni tipo "presa-in-carico" va a incrementare il contatore delle prese in carico

         gdsUpdate(key, elem.count.PreseInCaricoInp, elem.count.PreseInCaricoOut);

         key = *gds;
         }
      else if (tipology[U_accettazione])
         {
         // Ogni "accettazione" va ad incrementare il contatore delle "accettazione" ricevute dal mittente

         elem.count.Accettazioni      = 1;
      // elem.count.BusteTrasportoOut = 1;
         }
      else if (tipology[U_non_accettazione])
         {
         // Ogni "non accettazione" va ad incrementare il contatore delle "non accettazione" ricevute dal mittente

         elem.count.NonAccettazioni = 1;
         }
      else if (tipology[U_errore_consegna])
         {
         // Ogni "errore consegna" va ad incrementare il contatore delle "mancata consegna" associato al campo "mittente" 

         elem.count.MancateConsegne = 1;
         }
      else if (tipology[U_posta_certificata])
         {
         // Ogni tipo "posta-certificata" va ad incrementare il contatore dei messaggi spediti

         elem.count.BusteTrasportoOut = 1;
         }
      else if (tipology[U_avvenuta_consegna])
         {
         // Ogni "consegna" va ad incrementare il contatore delle "consegna" associato al campo "mittente" 

         elem.count.Consegne = 1;

         (void) casellaUpdate(key);

         // Ogni "consegna" va ad incrementare il contatore delle "buste di trasporto" associato al campo "consegna"

         elem.count.Consegne          = 0;
         elem.count.BusteTrasportoInp = 1;

         key = getValueField(U_consegna);
         }
      else if (tipology[U_preavviso_errore_consegna])
         {
         // Ogni "preavviso di mancata consegna" va ad incrementare il contatore dei "preavviso di mancata consegna"
         // ricevuti associato al campo "mittente"

         elem.count.PreavvisoMancataConsegna = 1;
         }
      else if (tipology[U_busta_anomalia])
         {
         // Ogni "busta di anomalia" va ad incrementare il contatore delle "buste di anomalia" ricevute associato al campo "destinatari"

         elem.count.BusteAnomalia = 1;

         if (*ptipo == ' ')
            {
            (void) callForAllEntryField(U_destinatari, Application::casellaUpdate);

            return;
            }

         key = *gds;
         }

      // update...

      (void) casellaUpdate(key);
      }

   static void reportNumCaselle()
      {
      U_TRACE(5, "Application::reportNumCaselle()")

      U_MESSAGE("number of caselle processed %u...", table->size());
      }

   static void start()
      {
      U_TRACE(5, "Application::start()")

      // option b (box)

       gds = new UString;
      *gds = pthis->opt['b'];

      if (gds->empty()) U_ERROR("parameter box is mandatory");

      // append to report

      std::cout << U_XML_MSG_START;

      // setting for messaggi

      table = new UHashMap<VCasellaIdCounter*>(U_GET_NEXT_PRIME_NUMBER(16 * 1024), true); // ignore case

      PEC_report::parse       = Application::parseLineForMessaggi;
      PEC_report::change_file = Application::reportNumCaselle;
      }

   static void end()
      {
      U_TRACE(5, "Application::end()")

      // end report

      table->callForAllEntrySorted(Application::reportMessaggi);

      std::cout << U_XML_MSG_END;
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
