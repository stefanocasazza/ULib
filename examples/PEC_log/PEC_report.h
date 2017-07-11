// PEC_report.h

#ifndef PEC_report_H
#define PEC_report_H 1

/*
TIPOLOGIE: 8 + 1

accettazione
------------------------------------------------------------------------------------------------------------------------------------------
Sep 21 17:54:23 PEC_Milter root [1957]: 20060921175422.380241@pec.internal.domain: tipo=accettazione (Accettazione): mittente=user1@pec.internal.domain: destinatari=user2@pec.internal.domain (certificato), user1@pec.internal.domain (certificato): oggetto=MESSAGGIO CON PIU DESTINATARI: message-id=20060921175423.866599@pec.internal.domain: gestore=TMail PEC test
------------------------------------------------------------------------------------------------------------------------------------------
non-accettazione
------------------------------------------------------------------------------------------------------------------------------------------
gen 19 16:41:09 PEC_Milter root [5808]: 20070119164109.997272@pec.internal.domain: queueid=l0JFf8Uh005807: tipo=non-accettazione (NonAccettazione): mittente=user1@pec.internal.domain: destinatari=user2@pec.internal.domain (certificato): oggetto=MESSAGGIO CON PIU DESTINATARI: message-id=20070119164109.718137@pec.internal.domain: gestore=TMail PEC test: errore=altro: errore-esteso=Sono state riscontrate le seguenti anomalie:
  - non e' stata utilizzata la cifratura del canale

Mar  5 10:26:31 2007 PEC_Milter root [15033]: 20070305102630.525133@actaliscertymail.it: tipo=non-accettazione (NonAccettazioneVirus): mittente=teststudioinfo1@actaliscertymail.it: destinatari=sandro.chierici@gmail.com (esterno): oggetto=test: message-id=20070305102631.786126@actaliscertymail.it: gestore=Actalis Certymail: errore=virus: virus=Encrypted.Zip
------------------------------------------------------------------------------------------------------------------------------------------
avvenuta-consegna
------------------------------------------------------------------------------------------------------------------------------------------
gen 19 15:09:19 PEC_Milter root [4621]: 20070119150915.938835@pec.internal.domain: queueid=l0JE9EDP004620: tipo=avvenuta-consegna (Consegna): mittente=user1@pec.internal.domain: destinatari=user2@pec.internal.domain (certificato), user1@pec.internal.domain (certificato): oggetto=MESSAGGIO CON PIU DESTINATARI: message-id=20070119150919.318531@pec.internal.domain: gestore=TMail PEC test: tipo_ricevuta=completa: consegna=user2@pec.internal.domain
------------------------------------------------------------------------------------------------------------------------------------------
rilevazione-virus
------------------------------------------------------------------------------------------------------------------------------------------
mag 10 11:01:59 PEC_Milter root [4063]: 20060510110152.637634@pec.internal.domain: tipo=rilevazione-virus (RilevazioneVirus): mittente=user1@pec.internal.domain: destinatari=user1@pec.internal.domain (certificato), user2@anotherpec.external.domain (certificato), user3@pec.internal.domain (certificato), user4@nonpec.external.domain (esterno): oggetto=Test con euro nel subject: message-id=20060510110154.278581@anotherpec.external.domain: gestore=TMail AnotherPEC test: ricezione=user2@anotherpec.external.domain: errore=virus: virus=ClamAV-Test-File
------------------------------------------------------------------------------------------------------------------------------------------
posta-certificata
------------------------------------------------------------------------------------------------------------------------------------------
mag 10 11:04:54 PEC_Milter root [4063]: 20060510110446.583732@pec.internal.domain: tipo=posta-certificata (BustaTrasporto): mittente=user1@pec.internal.domain: destinatari=user2@pec.internal.domain (certificato): oggetto=Test con euro nel subject  e caratteri speciali ï¿½@ï¿½#ï¿½ï¿½^~?`'ï¿½"ï¿½$%&|\<>-_: message-id=20060510110152.637634@pec.unirel.test: gestore=TMail PEC test: tipo_ricevuta=completa
------------------------------------------------------------------------------------------------------------------------------------------
errore-consegna
------------------------------------------------------------------------------------------------------------------------------------------
gen 19 16:45:47 PEC_Milter root [5989]: 20070119164543.063754@pec.internal.domain: queueid=l0JFjgrx005988: tipo=errore-consegna (MancataConsegna): mittente=user1@pec.internal.domain: destinatari=user2@pec.internal.domain (certificato), USER1@pec.internal.domain (certificato): oggetto=MESSAGGIO CON PIU DESTINATARI: message-id=20070119164547.838736@pec.internal.domain: gestore=TMail PEC test: consegna=USER1@pec.internal.domain: errore=no-dest: errore-esteso=Casella del destinatario inesistente

mag 10 11:01:59 PEC_Milter root [4063]: 20060510110152.637634@pec.internal.domain: tipo=errore-consegna (MancataConsegnaVirus): mittente=user1@pec.internal.domain: destinatari=user1@pec.internal.domain (certificato), user2@anotherpec.external.domain (certificato), user3@pec.internal.domain (certificato), user4@nonpec.external.domain (esterno): oggetto=Test con euro nel subject: message-id=20060510110155.602802@anotherpec.external.domain: gestore=TMail AnotherPEC test: tipo_ricevuta=completa: consegna=user2@anotherpec.external.domain: errore=virus: virus=ClamAV-Test-File
------------------------------------------------------------------------------------------------------------------------------------------
presa-in-carico
------------------------------------------------------------------------------------------------------------------------------------------
mag 10 11:01:54 PEC_Milter root [4063]: 20060510110152.637634@pec.internal.domain: tipo=presa-in-carico (PresaInCarico): mittente=user1@pec.internal.domain: destinatari=user1@pec.internal.domain (certificato), user2@anotherpec.external.domain (certificato), user3@pec.internal.domain (certificato), user4@nonpec.external.domain (esterno): oggetto=Test con euro nel subject: message-id=20060510110154.278581@pec.internal.domain: gestore=TMail PEC test: ricezione=user1@pec.internal.domain, user3@pec.internal.domain
------------------------------------------------------------------------------------------------------------------------------------------
preavviso-errore-consegna
------------------------------------------------------------------------------------------------------------------------------------------
mag 10 15:48:52 PEC_Notify root [26148]: 20060510154758.522061@pec.internal.domain: tipo=preavviso-errore-consegna (MancataConsegna12h): mittente=user1@pec.internal.domain: destinatari=user2@pec.external.domain (certificato): oggetto=Test 19 ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ Ã²@Ã§#Â°Ã¹Â§^~?`'Ã¬"Â£$%&|\<>-_: message-id=20060510154852.289790@pec.internal.domain: gestore=TMail PEC test: consegna=user2@pec.external.domain
mag 10 15:51:05 PEC_Notify root [26409]: 20060510154758.522061@pec.internal.domain: tipo=preavviso-errore-consegna (MancataConsegna24h): mittente=user1@pec.internal.domain: destinatari=user2@pec.external.domain (certificato): oggetto=Test 19 ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ Ã²@Ã§#Â°Ã¹Â§^~?`'Ã¬"Â£$%&|\<>-_: message-id=20060510155105.396263@pec.internal.domain: gestore=TMail PEC test: consegna=user2@pec.external.domain
------------------------------------------------------------------------------------------------------------------------------------------
* ANOMALIA *
------------------------------------------------------------------------------------------------------------------------------------------
set 20 19:48:03 PEC_Milter root [2504]: 20060920194803.492346@pec.internal.domain: tipo= (BustaAnomalia): mittente=user1@nonpec.unirel.test: destinatari=user1@pec.internal.domain (certificato): oggetto=Test: message-id=: gestore=TMail PEC test: eccezioni=domino mittente non di posta certificata
------------------------------------------------------------------------------------------------------------------------------------------
*/

#include <ulib/date.h>
#include <ulib/timeval.h>
#include <ulib/tokenizer.h>
#include <ulib/ssl/pkcs7.h>
#include <ulib/file_config.h>
#include <ulib/container/vector.h>
#include <ulib//utility/services.h>
#include <ulib//utility/xml_escape.h>

#include <ulib/application.h>

// -----------------------------------------------------
// line
// -----------------------------------------------------
#define U_LINEDATA    (sizeof("feb 22")-1)
#define U_LINESTART   (sizeof("feb 22 15:03:40")-1)
// -----------------------------------------------------
// line fields
// -----------------------------------------------------
#define U_stat        0
#define U_reject      1
#define U_dsn         2

#define U_tipo        0
#define U_mittente    1
#define U_destinatari 2
#define U_consegna    3
#define U_ricezione   4
#define U_errore      5
// -----------------------------------------------------
// line tipology
// -----------------------------------------------------
#define U_accettazione              0
#define U_non_accettazione          1
#define U_avvenuta_consegna         2
#define U_rilevazione_virus         3
#define U_posta_certificata         4
#define U_errore_consegna           5
#define U_presa_in_carico           6
#define U_preavviso_errore_consegna 7
#define U_busta_anomalia            8
// -----------------------------------------------------
// id
// -----------------------------------------------------
#define U_MaxID 16

#define U_DIMENSIONE_MEDIA_RECORD_LOG 300LL

typedef bool (*bPFstr)(const UString&);

class PEC_report : public UApplication {
public:

   static vPF parse;
   static vPF end_func;
   static vPF start_func;
   static vPF change_file;
   static UApplication* pthis;

   static char cnt[2];
   static uint64_t bytes;
   static time_t nseconds;
   static char buffer[4096];

   static bool failed;
   static bool rejected;
   static bool checklink;
   static bool non_firmati;
   static int  rejected_type;
   static UString* last_dominio;
   static int      last_field;
   static UString* last_value_field;

   static UTimeDate* to;
   static UTimeDate* fix;
   static UTimeDate* from;
   static UTimeDate* date;
   static UTimeDate* date1;
   static UFile* file;
   static UTokenizer* t;
   static UString* title;
   static UString* content;
   static UString* filename;
   static UString* scan_form;
   static UString* consegna;
   static UString* directory;
   static UString* filter;
   static UString* filter_ext;
   static UHashMap<UString>* tfile;

   // ---------------------------------
   static UString* domain;
   static UVector<UString>* vdomain;
   // ---------------------------------
   // line
   // ---------------------------------
   static int day, year;
   static char time[32];
   static char data[128];
   static char month[32];
   static bool year_present;
   static UString* line;
   static UString* mittente;
   static UString* identifier;
   // ---------------------------------
   // line tipology
   // ---------------------------------
   static uint32_t index;
   static bool tipology[9];
   static UString* vtipos;
   static const char* ptipo;
   static UVector<UString>* vtipo;
   // ---------------------------------
   // line fields
   // ---------------------------------
   static UString* vfields;
   static UVector<UString>* vfield;
   // ---------------------------------
   // id
   // ---------------------------------
   static bool id_all;
   static UString* id;
   static uint32_t id_index;
   static UVector<UString>* vid;

   PEC_report()
      {
      U_TRACE(5, "PEC_report::PEC_report()")

      pthis = this;
      }

   ~PEC_report();

   // SERVICES

   static const char* getData()
      {
      U_TRACE(5, "PEC_report::getData()")

      (void) sprintf(data, "%s %2d %s %4d", month, day, time, year);

      U_RETURN(data);
      }

   static time_t getTime()
      {
      U_TRACE(5, "PEC_report::getTime()")

      U_INTERNAL_DUMP("line->c_pointer(U_LINEDATA+1) = %.*S", 20, line->c_pointer(U_LINEDATA+1))

      time_t _t = UTimeDate::getSecondFromTime(line->c_pointer(U_LINEDATA+1));

      U_RETURN(_t);
      }

   static bool setLineID();         // find and check id content...
   static bool setLineMittente();   // find sender content...
   static bool setLineTipology();   // find line tipology...
   static void setLineIdentifier(); // find identifier content...

   // check if line contain <field>...

   static uint32_t findField(int field);
   static uint32_t sizeField(uint32_t start, char c = ':') __pure;

   static UString getValueField(int field);
   static UString getValueLastField(const char* field, uint32_t size);
   static UString getValuePrevField(const char* pfield, uint32_t psize, const char* field, uint32_t size);

   static bool checkValueField(int field, const UString& value)
      {
      U_TRACE(5, "PEC_report::checkValueField(%d,%.*S)", field, U_STRING_TO_TRACE(value))

      if (getValueField(field) == value) U_RETURN(true);

      U_RETURN(false);
      }

   static bool callForAllEntryField(int field,            bPFstr function);
   static bool callForAllEntryField(const UString& value, bPFstr function);

   // check se tra i vari indirizzi c'e' un dominio APPARTENENTE ai domini specificati...

   static bool isDomainField(int field)
      {
      U_TRACE(5, "PEC_report::isDomainField(%d)", field)

      if (callForAllEntryField(field, PEC_report::isDomainAddress)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isDomainAddress(const UString& address); // only one e-mail address...

   // dipendenze da content...

   static void clearData()
      {
      U_TRACE(5, "PEC_report::clearData()")

      identifier->clear();
      mittente->clear();
      last_dominio->clear();
      last_field = -1;
      last_value_field->clear();
      line->clear();
      }

   // MAIN

   void run(int argc, char* argv[], char* env[]) // MUST BE INLINE...
      {
      U_TRACE(5, "PEC_report::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions() == false) U_ERROR("you must specify at least parameters directory and title report");

      manageOptions();

      if (checklink == false) startReport(); // start report

      // process all filenames in parameter directory...

      loadFiles();

      if (checklink == false) endReport(); // end report
      }

protected:
   UFileConfig cfg;
   UString cfg_str, cfg_suffix, cfg_domain, cfg_id, cfg_from, cfg_to;

   void endReport();
   void startReport();
   void manageOptions();

   static unsigned getMonth(const char* ptr)
      {
      U_TRACE(5, "PEC_report::getMonth(%p)", ptr)

      while (u__isspace(*ptr)) ++ptr;

      return u_getMonth(ptr);
      }

   static bool setLine();
   static void loadFiles();
   static bool readContent();
   static void processFiles();
   static bool processFile(UStringRep* key, void* elem);

   static inline const char* next(const char* start, char c = '=') U_NO_EXPORT;
   static inline const char* next(const char* start, uint32_t size) U_NO_EXPORT;

   static inline const char* prev(const char* start) U_NO_EXPORT;
   static inline const char* prev(const char* start, uint32_t size) U_NO_EXPORT;
};

#endif
