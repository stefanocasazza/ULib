// PEC_report_anomalie.h

#ifndef PEC_REPORT_ANOMALIE_H
#define PEC_REPORT_ANOMALIE_H 1

/*
--------------------------------------------------------------------------------------------------------------------------
A) Per ogni messaggio con lo stesso id devono essere identici (a meno del case) i campi mittente= e destinatari=

eccezione = "Mittente incoerente"
eccezione = "Destinatari incoerenti"

il mittente prioritario e' (tipo=posta-certificata)

B) numero totale di messaggi SPEDITI che non hanno una corretta presa in carico e accettazione

implementazione:

Per ogni id-header [esclusi (tipo=non-accettazione o senza tipo es: tipo= (BustaAnomalia))] si traccia lo stato...
Si memorizzano id-header, data, subject (oggetto=<>), destinatari e mittente, installation-id

Nelle righe successive si elaborano le righe con lo stesso id-header... 

Il percorso corretto e':
**************************************************************************************************************************
1) Deve esistere una BustaTrasporto (tipo=posta-certificata) dove il mittente appartiene a domini specificati

=0 eccezione = "Busta di trasporto mancante"
>1 eccezione = "Busta di trasporto duplicata"

2) Per una BustaTrasporto (tipo=posta-certificata) dove il mittente appartiene a domini specificati
deve esistere una sola Accettazione (tipo=accettazione)

=0 eccezione = "Accettazione mancante"
>1 eccezione = "Accettazione duplicata"

3) Ogni indirizzo email CERTIFICATO contenuto nel campo destinatari di una busta di trasporto, dovrà apparire una sola
volta nell'attributo ricezione di una presa in carico con lo stesso identificativo

=0 eccezione = "Presa in carico mancante per almeno un destinatario"
>1 eccezione = "Presa in carico duplicata per almeno un destinatario"

3A) Per ogni rilevazione virus con lo stesso identificativo ci deve essere una sola mancata consegna (errore=virus)
con cui il campo consegna coincide

=0 eccezione = "Mancata consegna virus mancante per un destinatario"
>1 eccezione = "Mancata consegna virus duplicata per un destinatario"

3B) Per ogni mancata consegna (errore=virus) con lo stesso identificativo ci deve essere una sola rilevazione virus
con cui il campo consegna coincide

=0 eccezione = "Rilevazione virus mancante per un destinatario"
>1 eccezione = "Rilevazione virus duplicata per un destinatario"

4A) Ogni indirizzo email contenuto nel campo ricezione di una presa in carico, dovrà corrispondere
nell'attributo destinatari a un destinatario CERTIFICATO della busta di trasporto con lo stesso identificativo

eccezione = "Presa in carico incoerente per almeno un destinatario"

4B) Ogni indirizzo email contenuto nel campo ricezione (o consegna da versione 2.1) di una rilevazione virus, dovrà corrispondere
nell'attributo destinatari a un destinatario CERTIFICATO della busta di trasporto con lo stesso identificativo

eccezione = "Rilevazione virus incoerente per almeno un destinatario"

5) Ogni indirizzo email contenuto nel campo consegna di una avvenuta/mancata consegna, dovrà corrispondere
nell'attributo destinatari a un destinatario CERTIFICATO della busta di trasporto con lo stesso identificativo

eccezione = "Avvenuta consegna o errore consegna incoerente per almeno un destinatario"

6) Ogni indirizzo email CERTIFICATO contenuto nel campo destinatari di una busta di trasporto, dovrà apparire una sola
volta nell'attributo consegna di una avvenuta/mancata consegna con lo stesso identificativo

=0                         eccezione = "Avvenuta consegna o errore consegna mancante per almeno un destinatario"
>1                         eccezione = "Avvenuta consegna o errore consegna duplicata per almeno un destinatario"
>1 (con tipologie diverse) eccezione = "Avvenuta consegna o errore consegna in conflitto per almeno un destinatario"

???) Tutti i destinatari CERTIFICATI nei casi in cui ci sia anomalia al punto 2 o 3 devono essere contenuti nell'attributo
consegna delle tipo=preavviso-errore-consegna
**************************************************************************************************************************

C) numero totale di messaggi RICEVUTI (mittente NON appartiene a domini specificati) che non hanno prodotto una corretta presa in carico

implementazione:

Per ogni id-header [esclusi (tipo=non-accettazione o senza tipo es: tipo= (BustaAnomalia))] si traccia lo stato...
Si memorizzano id-header, data, subject (oggetto=<>), destinatari e mittente, installation-id

Nelle righe successive si elaborano le righe con lo stesso id-header...

Il percorso corretto e':
**************************************************************************************************************************
1) Deve esistere una BustaTrasporto (tipo=posta-certificata) dove il mittente NON appartiene a domini specificati

=0 eccezione = "Busta di trasporto mancante"
>1 eccezione = "Busta di trasporto duplicata"

2) Per una BustaTrasporto (tipo=posta-certificata) dove il mittente NON appartiene a domini specificati
non devono esistere Accettazione (tipo=accettazione)

>0 eccezione = "Accettazione incoerente"

6) Ogni indirizzo email certificato con dominio APPARTENENTE a domini specificati contenuto nel campo destinatari di
una busta di trasporto, dovrà apparire una sola volta nell'attributo consegna di una avvenuta o mancata consegna o
rilevazione virus con lo stesso identificativo

=0                         eccezione = "Avvenuta consegna o errore consegna o rilevazione virus mancante per almeno un destinatario"
>1                         eccezione = "Avvenuta consegna o errore consegna o rilevazione virus duplicata per almeno un destinatario"
>1 (con tipologie diverse) eccezione = "Avvenuta consegna o errore consegna o rilevazione virus in conflitto per almeno un destinatario"

##### le stesse regole dei messaggi SPEDITI salvo che i destinatari DEVONO appartenere a domini specificati ##########
SALVO 3A, 3B e 6
**************************************************************************************************************************
*/

#include "PEC_report.h"

#include <ulib/internal/objectIO.h>

#define U_XML_MSG_START \
"    <NumMsgExceptionInOut>\n"

#define U_XML_MSG_ENTRY_START \
"      <NumMsgExceptionInOut-Entry>\n"                                                        \
"        <NumMsgExceptionInOut-%s>\n"                                                         \
"          <NumMsgExceptionInOut-Identifier>%.*s</NumMsgExceptionInOut-Identifier>\n"         \
"          <NumMsgExceptionInOut-Date>%s</NumMsgExceptionInOut-Date>\n"                       \
"          <NumMsgExceptionInOut-InstallationId>%.*s</NumMsgExceptionInOut-InstallationId>\n" \
"          <NumMsgExceptionInOut-Sender>%.*s</NumMsgExceptionInOut-Sender>\n"

// "          <NumMsgExceptionInOut-Subject>%.*s</NumMsgExceptionInOut-Subject>\n"

#define U_XML_MSG_ENTRY_Recipients \
"          <NumMsgExceptionInOut-Recipients>%.*s</NumMsgExceptionInOut-Recipients>\n"

#define U_XML_MSG_ENTRY_Exception \
"          <NumMsgExceptionInOut-Exception>%s</NumMsgExceptionInOut-Exception>\n"

#define U_XML_MSG_ENTRY_END            \
"        </NumMsgExceptionInOut-%s>\n" \
"      </NumMsgExceptionInOut-Entry>\n"

#define U_XML_MSG_END \
"    </NumMsgExceptionInOut>\n"

#define U_ERROR_MESSAGE_OUT_OF_TTL "TTL MESSAGE <%T> OUT OF RANGE <%T> - id = <%.*s>"

class Messaggio {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   time_t start, ttl;
   char* vdestinatari_domini;
   char* vdestinatari_certificati;
   char* vdestinatari_presa_in_carico;
   char* vdestinatari_mancata_consegna;
   char* vdestinatari_avvenuta_consegna;
   char* vdestinatari_rilevazione_virus;
   char* vdestinatari_mancata_consegna_virus;
   UString id, mittente, identifier, destinatari;
   UVector<UString> vdestinatari;
   char flag[9];

   static Messaggio* msg;
   static UString* id_max_ttl;
   static time_t now, max_ttl, range_ttl;

   // COSTRUTTORE

    Messaggio();
   ~Messaggio();

   // VARIE

   bool isOld()
      {
      U_TRACE(5, "Messaggio::isOld()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("now = %T, start = %T, ttl = %T", now, start, ttl)

      if ((now - start) > range_ttl) U_RETURN(true);

      U_RETURN(false);
      }

   const char* getData()
      {
      U_TRACE(5, "Messaggio::getData()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("start = %T", start)

      (void) u_strftime2(PEC_report::data, sizeof(PEC_report::data), "%b %e %T %Y", start);

      U_RETURN(PEC_report::data);
      }

   bool isAnomalia();
   void setRicezioneConsegna(int field);

   // SERVICES

   static bool setConsegna(const UString& elem);
   static bool setRicezione(const UString& elem);

   // STREAM

   friend U_EXPORT istream& operator>>(istream& is,       Messaggio& m);
   friend U_EXPORT ostream& operator<<(ostream& os, const Messaggio& m);

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

private:
   inline void allocDestinatari(int n) U_NO_EXPORT;
};

class PEC_report_anomalie : public PEC_report {
public:

   static uint32_t nmsg;
   static uint32_t nanomalie;
   static uint32_t nout_of_range;

   ~PEC_report_anomalie()
      {
      U_TRACE(5, "PEC_report_anomalie::~PEC_report_anomalie()")
      }

   static void processLine(bool bnew);
   static void reportDestinatari(void* elem);
   static bool reportAnomalie(UStringRep* key, void* elem);

   static bool checkLineForAnomalie()
      {
      U_TRACE(5, "PEC_report_anomalie::checkLineForAnomalie()")

      // Per ogni riga [ ESCLUSI tipo=non-accettazione o senza tipo es: tipo= (BustaAnomalia)) ] si traccia lo stato...

      if (tipology[U_busta_anomalia] ||
          tipology[U_non_accettazione]) U_RETURN(false);

      U_RETURN(true);
      }

   static void reportMaxTTL()
      {
      U_TRACE(5, "PEC_report_anomalie::reportMaxTTL()")

      U_INTERNAL_DUMP("Messaggio::max_ttl    = %T", Messaggio::max_ttl)
      U_INTERNAL_DUMP("Messaggio::id_max_ttl = %.*S", U_STRING_TO_TRACE(*Messaggio::id_max_ttl))

      if (Messaggio::max_ttl)
         {
         (void) u__snprintf(buffer, sizeof(buffer), " - max TTL: %T day(s) for message <%.*s>",
                        Messaggio::max_ttl / U_ONE_DAY_IN_SECOND, U_STRING_TO_TRACE(*Messaggio::id_max_ttl));
         }

      U_MESSAGE("number of messages processed %u%s...", nmsg, (Messaggio::max_ttl ? buffer : ""));
      }

   static void start()
      {
      U_TRACE(5, "PEC_report_anomalie::start()")

      // append to report

      std::cout << U_XML_MSG_START;

      // setting for anomalie
      }

   static void end()
      {
      U_TRACE(5, "PEC_report_anomalie::end()")

      // end report

      std::cout << U_XML_MSG_END;
      }
};

#endif
