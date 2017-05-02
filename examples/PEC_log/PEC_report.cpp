// PEC_report.cpp

#include "PEC_report.h"

#include <ulib/utility/dir_walk.h>
#include <ulib/utility/string_ext.h>

vPF                PEC_report::parse;
vPF                PEC_report::end_func;
vPF                PEC_report::start_func;
vPF                PEC_report::change_file;
UApplication*      PEC_report::pthis;

time_t             PEC_report::nseconds;
uint64_t           PEC_report::bytes;
UTimeDate*         PEC_report::to;
UTimeDate*         PEC_report::fix;
UTimeDate*         PEC_report::from;
UTimeDate*         PEC_report::date;
UTimeDate*         PEC_report::date1;
UFile*             PEC_report::file;
UString*           PEC_report::title;
UString*           PEC_report::domain;
UString*           PEC_report::content;
UString*           PEC_report::directory;
UString*           PEC_report::filter;
UString*           PEC_report::filter_ext;
UTokenizer*        PEC_report::t;
UHashMap<UString>* PEC_report::tfile;

int                PEC_report::day;
int                PEC_report::year;
char               PEC_report::time[32];
char               PEC_report::data[128];
char               PEC_report::month[32];
bool               PEC_report::year_present;

bool               PEC_report::id_all;
char               PEC_report::buffer[4096];
char               PEC_report::cnt[2];
bool               PEC_report::tipology[9];
uint32_t           PEC_report::index;
uint32_t           PEC_report::id_index;
UString*           PEC_report::line;
UString*           PEC_report::scan_form;
UString*           PEC_report::id;
UString*           PEC_report::identifier;
UString*           PEC_report::mittente;
UString*           PEC_report::vtipos;
UString*           PEC_report::vfields;
const char*        PEC_report::ptipo;
UVector<UString>*  PEC_report::vid;
UVector<UString>*  PEC_report::vtipo;
UVector<UString>*  PEC_report::vfield;
UVector<UString>*  PEC_report::vdomain;

bool               PEC_report::failed;
bool               PEC_report::rejected;
bool               PEC_report::checklink;
bool               PEC_report::non_firmati;
int                PEC_report::rejected_type;
UString*           PEC_report::last_dominio;
int                PEC_report::last_field;
UString*           PEC_report::last_value_field;

static bool optimization;

PEC_report::~PEC_report()
{
   U_TRACE(5, "PEC_report::~PEC_report()")

   if (id)
      {
      delete t;
      delete file;
      delete date;
      delete date1;
      delete title;
      delete directory;
      delete last_dominio;
      delete last_value_field;

      delete id;
      delete mittente;
      delete identifier;

      delete line;
      delete content;

      if (rejected)
         {
         delete scan_form;
         }
      else
         {
         delete fix;
         delete vtipo;
         delete vtipos;

         if (domain)
            {
            delete vdomain;
            delete domain;
            }
         }

      delete vfield;
      delete vfields;
      }

   if (to)         delete to;
   if (vid)        delete vid;
   if (from)       delete from;
   if (filter)     delete filter;
   if (filter_ext) delete filter_ext;
   if (tfile)      delete tfile;
}

uint32_t PEC_report::findField(int i)
{
   U_TRACE(5, "PEC_report::findField(%d)", i)

   // check if line contain <field>...

   UString field = (*vfield)[i];

   uint32_t n = line->find(field, U_LINESTART);

   if (n != U_NOT_FOUND) n += field.size();

   U_RETURN(n);
}

__pure uint32_t PEC_report::sizeField(uint32_t start, char c)
{
   U_TRACE(5, "PEC_report::sizeField(%u,%C)", start, c)

   const char* p1 = line->c_pointer(start);
   const char* p2 = p1;

   while (true)
      {
      if (*p2 == '\n') break;

      if (*p2 == c)
         {
         if (*(p2+1) == ' ') break;
         }

      ++p2;
      }

   U_RETURN(p2 - p1);
}

/*
PARSING
=======================================================
si parte dopo la data <-> "delimitatore..." ':'

IDENTIFICATIVO 2 (deve contenere carattere '@')
TIPO           3 or 4 (se presente queueid)
MITTENTE       4 or 5 ...
DESTINATARI    5 or 6 ...  : oggetto=
-------------------------------------------------------
si parte dalla fine record <-> "delimitatore..." '='

CONSEGNA
if (preavviso-errore-consegna || avvenuta-consegna) 1
if (errore-consegna)                                3 : errore=
if (rilevazione-virus)                              3 : errore=

RICEZIONE
if (presa-in-carico)                                1
if (rilevazione-virus)                              3 : errore=

ERRORE (errore=virus)                               2 : virus= | errore-esteso=
ci sono due possibilita':
if (rilevazione_virus) (obbligatorio)
if (non_accettazione) (check valore campo)
=======================================================
gen 19 15:09:19 2007 PEC_Milter root [4621]: 20070119150915.938835@pec.internal.domain: queueid=l0JE9EDP004620: tipo=avvenuta-consegna (Consegna): mittente=user1@pec.internal.domain: destinatari=user2@pec.internal.domain (certificato), user1@pec.internal.domain (certificato): oggetto=MESSAGGIO CON PIU DESTINATARI: message-id=20070119150919.318531@pec.internal.domain: gestore=TMail PEC test: tipo_ricevuta=completa: consegna=user2@pec.internal.domain
*/

static const char* ptr1;
static const char* ptr2;

#define U_TIPO          "tipo="
#define U_MITTENTE      "mittente="
#define U_DESTINATARI   "destinatari="
#define U_OGGETTO       "oggetto="
#define U_CONSEGNA      "consegna="
#define U_RICEZIONE     "ricezione="
#define U_ERRORE        "errore="
#define U_VIRUS         "virus="
#define U_ERRORE_ESTESO "errore-esteso="

#define U_PARSE_ERROR_MESSAGE                         "missing separator - id = <%.*s>"
#define U_PARSE_ERROR_MESSAGE_TAG_EMPTY               "missing value of tag <%.*s...> - id = <%.*s>"
#define U_PARSE_ERROR_MESSAGE_ID_NOT_VALID            "not valid message id <%.*s> - first 50 character of record <%.*s>"
#define U_PARSE_ERROR_MESSAGE_REC_NOT_VALID           "not valid record - size inferior to 50 character <%.*s>"
#define U_PARSE_ERROR_MESSAGE_FILTER_MATCH            "record skip for matching filter <%.*s> - id = <%.*s>"
#define U_PARSE_ERROR_MESSAGE_TAG_NOT_FOUND           "not found expected tag <%s...> - id = <%.*s>"
#define U_PARSE_ERROR_MESSAGE_TYPE_NOT_VALID          "unexpected record type <%.*s>  - id = <%.*s>"
#define U_PARSE_ERROR_MESSAGE_INCORRECT_SYNTAX        "incorrect syntax <consegna=%.*s> - id = <%.*s>"
#define U_PARSE_ERROR_EMAIL_ADDRESS_NOT_WELL_FORMED   "not well formed email address <%.*s> - id = <%.*s>"

#define U_PARSE_ERROR_OLD_FORMAT                      "old format for record type <rilevazione_virus> - id = <%.*s>"

inline const char* PEC_report::next(const char* start, char c)
{
   const char* ptr = (const char*) memchr(start, c, line->remain(start));

   if (!ptr)
      {
      U_ERROR(U_PARSE_ERROR_MESSAGE, U_STRING_TO_TRACE(*identifier));

      return start;
      }

   return ptr;
}

inline const char* PEC_report::next(const char* start, uint32_t size)
{
   const char* ptr = (const char*) memchr(start, '=', size);

   if (!ptr)
      {
      U_ERROR(U_PARSE_ERROR_MESSAGE, U_STRING_TO_TRACE(*identifier));

      return start;
      }

   return ptr;
}

static bool field_missing_throw_exception;

inline const char* PEC_report::prev(const char* start, uint32_t size)
{
   U_TRACE(5, "PEC_report::prev(%.*S,%u)", size, start, size)

   const char* ptr = (const char*) memrchr(start, '=', size);

   if (!ptr)
      {
      if (field_missing_throw_exception) U_ERROR(U_PARSE_ERROR_MESSAGE, U_STRING_TO_TRACE(*identifier));

      return start;
      }

   return ptr;
}

UString PEC_report::getValueLastField(const char* field, uint32_t size)
{
   U_TRACE(5, "PEC_report::getValueLastField(%S,%u)", field, size)

   UString value;
   const char* p1;
   uint32_t r = 0;

loop:
   p1 = prev(ptr1, line->remain(ptr1) - r);

   if (p1 == ptr1) goto end;

   if (strncmp(p1 - size + 1, field, size))
      {
      r = line->remain(p1);

      goto loop;
      }

   ++p1;

   value = line->substr(p1, line->remain(p1));

end:
   U_RETURN_STRING(value);
}

UString PEC_report::getValuePrevField(const char* pfield, uint32_t psize, const char* field, uint32_t size)
{
   U_TRACE(5, "PEC_report::getValuePrevField(%S,%u,%S,%u)", pfield, psize, field, size)

   UString value;
   const char* p1;
   const char* p2;
   uint32_t r = 0;

loop:
   p1 = prev(ptr1, line->remain(ptr1) - r);

   if (p1 == ptr1) goto end;

   if (strncmp(p1 - psize + 1, pfield, psize))
      {
      r = line->remain(p1);

      goto loop;
      }

   r = line->remain(p1);

loop1:
   p2 = prev(ptr1, line->remain(ptr1) - r);

   if (p2 == ptr1) goto end;

   if (strncmp(p2 - size + 1, field, size))
      {
      r = line->remain(p2);

      goto loop1;
      }

   p1 -= psize + 1;

   ++p2;

   value = line->substr(p2, p1 - p2);

end:
   U_RETURN_STRING(value);
}

UString PEC_report::getValueField(int field)
{
   U_TRACE(5, "PEC_report::getValueField(%d)", field)

   if (field != last_field) last_field = field;
   else                     U_RETURN_STRING(*last_value_field);

   uint32_t r;
   UString value;
   const char* p1;
   const char* p2;
   bool errore_virus, errore_esteso;

   switch (field)
      {
      case U_destinatari:
         {
         ptr2 = ptr1;

loop:
         ptr1 = next(ptr1);

         if (strncmp(++ptr1 - U_CONSTANT_SIZE(U_OGGETTO), U_CONSTANT_TO_PARAM(U_OGGETTO))) goto loop;

         value = line->substr(ptr2, ptr1 - U_CONSTANT_SIZE(U_OGGETTO) - sizeof(": ")+1 - ptr2);

         U_INTERNAL_DUMP("destinatari = %.*S", U_STRING_TO_TRACE(value))
         }
      break;

      case U_ricezione:
         {
         if (tipology[U_presa_in_carico])
            {
            value = getValueLastField(U_CONSTANT_TO_PARAM(U_RICEZIONE));
            }
         else if (tipology[U_rilevazione_virus])
            {
            value = getValuePrevField(U_CONSTANT_TO_PARAM(U_ERRORE), U_CONSTANT_TO_PARAM(U_RICEZIONE));
            }

         U_INTERNAL_DUMP("ricezione = %.*S", U_STRING_TO_TRACE(value))
         }
      break;

      case U_consegna:
         {
         if (tipology[U_avvenuta_consegna] ||
             tipology[U_preavviso_errore_consegna])
            {
            value = getValueLastField(U_CONSTANT_TO_PARAM(U_CONSEGNA));
            }
         else if (tipology[U_errore_consegna])
            {
            value = getValuePrevField(U_CONSTANT_TO_PARAM(U_ERRORE), U_CONSTANT_TO_PARAM(U_CONSEGNA));
            }
         else if (tipology[U_rilevazione_virus])
            {
            value = getValuePrevField(U_CONSTANT_TO_PARAM(U_ERRORE), U_CONSTANT_TO_PARAM(U_CONSEGNA));
            }

         U_INTERNAL_DUMP("consegna = %.*S", U_STRING_TO_TRACE(value))

         if (value.find(',') != U_NOT_FOUND)
            {
            U_WARNING(U_PARSE_ERROR_MESSAGE_INCORRECT_SYNTAX, U_STRING_TO_TRACE(value), U_STRING_TO_TRACE(*identifier));
            }
         }
      break;

      case U_errore:
         {
         r = 0;

loop3:
         p1 = prev(ptr1, line->remain(ptr1) - r);

         if (p1 == ptr1) goto end;

         errore_virus  =                         (strncmp(p1 - U_CONSTANT_SIZE(U_VIRUS)         + 1, U_CONSTANT_TO_PARAM(U_VIRUS))         == 0);
         errore_esteso = (errore_virus ? false : (strncmp(p1 - U_CONSTANT_SIZE(U_ERRORE_ESTESO) + 1, U_CONSTANT_TO_PARAM(U_ERRORE_ESTESO)) == 0));

         if (errore_virus  == false &&
             errore_esteso == false)
            {
            r = line->remain(p1);

            U_INTERNAL_DUMP("r = %u", r)

            goto loop3;
            }

         r = line->remain(p1);

loop3A:
         p2 = prev(ptr1, line->remain(ptr1) - r);

         if (p2 == ptr1) goto end;

         if (strncmp(p2 - U_CONSTANT_SIZE(U_ERRORE) + 1, U_CONSTANT_TO_PARAM(U_ERRORE)))
            {
            r = line->remain(p2);

            U_INTERNAL_DUMP("r = %u", r)

            goto loop3A;
            }

         if (errore_virus) p1 -= U_CONSTANT_SIZE(U_VIRUS)         + 1;
         else              p1 -= U_CONSTANT_SIZE(U_ERRORE_ESTESO) + 1;

         ++p2;

         value = line->substr(p2, p1 - p2);

         U_INTERNAL_DUMP("errore = %.*S", U_STRING_TO_TRACE(value))
         }
      break;
      }

end:
   if (value.empty())
      {
      U_INTERNAL_DUMP("field_missing_throw_exception = %b", field_missing_throw_exception)

      // 2) Per ogni "rilevazione virus" dove il campo "ricezione" ("consegna" nella successiva versione 2.1)...

      if (field == U_consegna)
         {
         U_WARNING(U_PARSE_ERROR_OLD_FORMAT, U_STRING_TO_TRACE(*identifier));
         }

      else if (field_missing_throw_exception)
         {
         U_WARNING(U_PARSE_ERROR_MESSAGE_TAG_EMPTY, U_STRING_TO_TRACE((*vfield)[field]), U_STRING_TO_TRACE(*identifier));
         }
      }

   *last_value_field = value;

   U_RETURN_STRING(value);
}

bool PEC_report::setLineID()
{
   U_TRACE(5, "PEC_report::setLineID()")

   if (line->size() < 50)
      {
      U_WARNING(U_PARSE_ERROR_MESSAGE_REC_NOT_VALID, U_STRING_TO_TRACE(*line));

      U_RETURN(false);
      }

   // find id content...

   ptr1 = line->c_pointer(year_present ? sizeof("feb 22 15:03:40 2007") : sizeof("feb 22 15:03:40"));

   ptr2 = next(ptr1, ':'); // for setLineIdentifier()...

   UTokenizer tline(line->substr(ptr1, line->remain(ptr1)));

   (void) tline.next(*id, ' ');

   // check if this id is request...

   id_index = vid->find(*id);

   if (id_index == U_NOT_FOUND)
      {
      if (id_all == false) U_RETURN(false);

      id_index = vid->size();

      id->duplicate(); // NB: need duplicate string because depends on mmap()'s content of document...

      vid->push(*id);
      }

   *id = vid->at(id_index);  

   U_INTERNAL_DUMP("id_index = %u id = %.*S", id_index, U_STRING_TO_TRACE(*id))

   U_RETURN(true);
}

void PEC_report::setLineIdentifier()
{
   U_TRACE(5, "PEC_report::setLineIdentifier()")

   // find and check identifier content...

   ptr2 += sizeof(": ")-1;

   ptr1 = next(ptr2, ':');

   *identifier = line->substr(ptr2, ptr1 - ptr2);

   U_INTERNAL_DUMP("identifier = %.*S", U_STRING_TO_TRACE(*identifier))

   if (rejected == false &&
       UStringExt::isEmailAddress(*identifier) == false)
      {
      U_WARNING(U_PARSE_ERROR_MESSAGE_ID_NOT_VALID, U_STRING_TO_TRACE(*identifier), 50, line->data());

      if (identifier->size() == 9 &&
          ptr2[3] == ' ' &&
          ptr2[6] == ' ' &&
          u_getMonth(ptr2))
         {
/*
Feb 24 19:42:21 PEC_Milter root [1542]: Feb 24 19:42:35 PEC_Milter root [1546]: E452BE9C.0004057A.F50FB719.D305B319.posta-certificata@postecert.it: tipo=posta-certificata (BustaTrasporto): mittente=ifinanziarie@pec.gdf.it: destinatari=attivitariunite@actaliscertymail.it (certificato): oggetto=Richiesta n.<jdz9yb_1wrjlt>: message-id=E452BE9C.0004057A.F50FB719.D305B319.posta-certificata@postecert.it: gestore=Postecom S.p.A.: tipo_ricevuta=sintetica
*/

         *line = line->substr(line->distance(ptr2));

         ptr2 = next(ptr1 + sizeof("42:35"), ':');

         setLineIdentifier();
         }
      }
}

/*
#define U_ACCETTAZIONE              "accettazione"                // 0
#define U_NON_ACCETTAZIONE          "non-accettazione"            // 1
#define U_AVVENUTA_CONSEGNA         "avvenuta-consegna"           // 2
#define U_RILEVAZIONE_VIRUS         "rilevazione-virus"           // 3
#define U_POSTA_CERTIFICATA         "posta-certificata"           // 4
#define U_ERRORE_CONSEGNA           "errore-consegna"             // 5
#define U_PRESA_IN_CARICO           "presa-in-carico"             // 6
#define U_PREAVVISO_ERRORE_CONSEGNA "preavviso-errore-consegna"   // 7
*/

#define U_ACCETTAZIONE              "Accettazione"                // 0 (Accettazione) 
#define U_NON_ACCETTAZIONE          "NonAccettazione"             // 1 (NonAccettazione) (NonAccettazioneVirus)
#define U_AVVENUTA_CONSEGNA         "Consegna"                    // 2 (Consegna)
#define U_RILEVAZIONE_VIRUS         "RilevazioneVirus"            // 3 (RilevazioneVirus)
#define U_POSTA_CERTIFICATA         "BustaTrasporto"              // 4 (BustaTrasporto)
#define U_ERRORE_CONSEGNA           "MancataConsegna"             // 5 (MancataConsegna) (MancataConsegnaVirus)
#define U_PRESA_IN_CARICO           "PresaInCarico"               // 6 (PresaInCarico)
#define U_PREAVVISO_ERRORE_CONSEGNA "MancataConsegna"             // 7 (MancataConsegna12h) (MancataConsegna24h)
#define U_BUSTA_ANOMALIA            "BustaAnomalia"               // 8 (BustaAnomalia)

bool PEC_report::setLineTipology()
{
   U_TRACE(5, "PEC_report::setLineTipology()")

   // find and check line tipology...

   (void) memset(tipology, 0, sizeof(tipology));

   ptr1 += sizeof(": ")-1;

   bool queueid = (strncmp(ptr1, U_CONSTANT_TO_PARAM("queueid=")) == 0);

   U_INTERNAL_DUMP("queueid = %b", queueid)

   if (queueid) ptr1 = next(ptr1, ':') + sizeof(": ")-1;

   if (strncmp(ptr1, U_CONSTANT_TO_PARAM(U_TIPO)) != 0)
      {
      U_WARNING(U_PARSE_ERROR_MESSAGE_TAG_NOT_FOUND, U_TIPO, U_STRING_TO_TRACE(*identifier));

      U_RETURN(false);
      }

   ptr1 += U_CONSTANT_SIZE(U_TIPO);
   ptr2  = next(ptr1, ':');

   field_missing_throw_exception = true;

   ptipo = ptr1;

   U_INTERNAL_DUMP("ptipo = %.*S", 30, ptipo)

   const char* ntipo = next(ptipo, '(') + 1;

   if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_ACCETTAZIONE)) == 0)
      {
      tipology[U_accettazione] = true;

      U_INTERNAL_DUMP("tipology[U_accettazione] = %b", tipology[U_accettazione])
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_AVVENUTA_CONSEGNA)) == 0)
      {
      tipology[U_avvenuta_consegna] = true;

      U_INTERNAL_DUMP("tipology[U_avvenuta_consegna] = %b", tipology[U_avvenuta_consegna])
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_POSTA_CERTIFICATA)) == 0)
      {
      tipology[U_posta_certificata] = true;

      U_INTERNAL_DUMP("tipology[U_posta_certificata] = %b", tipology[U_posta_certificata])
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_PRESA_IN_CARICO)) == 0)
      {
      tipology[U_presa_in_carico] = true;

      U_INTERNAL_DUMP("tipology[U_presa_in_carico] = %b", tipology[U_presa_in_carico])
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_NON_ACCETTAZIONE)) == 0)
      {
      tipology[U_non_accettazione] = true;

      U_INTERNAL_DUMP("tipology[U_non_accettazione] = %b", tipology[U_non_accettazione])
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_RILEVAZIONE_VIRUS)) == 0)
      {
      tipology[U_rilevazione_virus] = true;

      U_INTERNAL_DUMP("tipology[U_rilevazione_virus] = %b", tipology[U_rilevazione_virus])

      // 2) Per ogni "rilevazione virus" dove il campo "ricezione" ("consegna" nella successiva versione 2.1)...

      field_missing_throw_exception = false;
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_ERRORE_CONSEGNA)) == 0)
      {
      ntipo += U_CONSTANT_SIZE(U_ERRORE_CONSEGNA);

      U_INTERNAL_DUMP("ntipo[0] = %C", ntipo[0])

      if (ntipo[0] == ')' ||  // (MancataConsegna)
          ntipo[0] == 'V')    // (MancataConsegnaVirus)
         {
         tipology[U_errore_consegna] = true;

         U_INTERNAL_DUMP("tipology[U_errore_consegna] = %b", tipology[U_errore_consegna])
         }
      else // (MancataConsegna12h) (MancataConsegna24h)
         {
         U_INTERNAL_ASSERT(ntipo[0] == '1' || ntipo[0] == '2')

         tipology[U_preavviso_errore_consegna] = true;

         U_INTERNAL_DUMP("tipology[U_preavviso_errore_consegna] = %b", tipology[U_preavviso_errore_consegna])
         }
      }
   else if (strncmp(ntipo, U_CONSTANT_TO_PARAM(U_BUSTA_ANOMALIA)) == 0)
      {
      tipology[U_busta_anomalia] = true;

      U_INTERNAL_DUMP("tipology[U_busta_anomalia] = %b", tipology[U_busta_anomalia])
      }
   else
      {
      U_WARNING(U_PARSE_ERROR_MESSAGE_TYPE_NOT_VALID, 30, ntipo, U_STRING_TO_TRACE(*identifier));

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool PEC_report::setLineMittente()
{
   U_TRACE(5, "PEC_report::setLineMittente()")

   // find and check sender content...

   ptr2 += sizeof(": ")-1;

   if (strncmp(ptr2, U_CONSTANT_TO_PARAM(U_MITTENTE)) != 0)
      {
      U_WARNING(U_PARSE_ERROR_MESSAGE_TAG_NOT_FOUND, U_MITTENTE, U_STRING_TO_TRACE(*identifier));

      U_RETURN(false);
      }

   ptr2 += U_CONSTANT_SIZE(U_MITTENTE);

   ptr1 = ptr2;

   if (filter->empty() == false &&
       line->find(*filter, line->distance(ptr1)) != U_NOT_FOUND)
      {
      U_WARNING(U_PARSE_ERROR_MESSAGE_FILTER_MATCH, U_STRING_TO_TRACE(*filter), U_STRING_TO_TRACE(*identifier));

      U_RETURN(false);
      }

loop:
   ptr1 = next(ptr1);

   if (strncmp(++ptr1 - U_CONSTANT_SIZE(U_DESTINATARI), U_CONSTANT_TO_PARAM(U_DESTINATARI)) != 0) goto loop;

   *mittente = line->substr(ptr2, ptr1 - U_CONSTANT_SIZE(U_DESTINATARI) - sizeof(": ")+1 - ptr2);

   U_INTERNAL_DUMP("mittente = %.*S", U_STRING_TO_TRACE(*mittente))

   // reset getValueField() cache...

   last_field = -1;
   last_value_field->clear();

   U_RETURN(true);
}

bool PEC_report::callForAllEntryField(const UString& value, bPFstr function)
{
   U_TRACE(5, "PEC_report::callForAllEntryField(%.*S,%p)", U_STRING_TO_TRACE(value), function)

   UVector<UString> vec(value, ", ");

   UString elem;

   for (uint32_t i = 0, n = vec.size(); i < n; ++i)
      {
      elem = vec.at(i);

      if (elem.c_char(0) == '(') continue;

      if (function(elem)) U_RETURN(true);
      }

   U_RETURN(false);
}

bool PEC_report::callForAllEntryField(int field, bPFstr function)
{
   U_TRACE(5, "PEC_report::callForAllEntryField(%d,%p)", field, function)

   UString value = getValueField(field);

   return callForAllEntryField(value, function);
}

// check se l'indirizzo ha un dominio APPARTENENTE ai domini specificati...

bool PEC_report::isDomainAddress(const UString& address)
{
   U_TRACE(5, "PEC_report::isDomainAddress(%.*S)", U_STRING_TO_TRACE(address))

   uint32_t n = address.find('@');

   if (n == U_NOT_FOUND)
      {
      U_WARNING(U_PARSE_ERROR_EMAIL_ADDRESS_NOT_WELL_FORMED, U_STRING_TO_TRACE(address), U_STRING_TO_TRACE(*identifier));

      U_RETURN(false);
      }

   UString dominio = address.substr(n+1);

   U_INTERNAL_DUMP("dominio = %.*S", U_STRING_TO_TRACE(dominio))

   static bool last_result;

   if (dominio != *last_dominio)
      {
      *last_dominio = dominio;

      if (UStringExt::isEmailAddress(address) == false)
         {
         U_WARNING(U_PARSE_ERROR_EMAIL_ADDRESS_NOT_WELL_FORMED, U_STRING_TO_TRACE(address), U_STRING_TO_TRACE(*identifier));
         }

      last_result = (vdomain->findSorted(dominio, true) != U_NOT_FOUND);
      }
   /*
   else
      {
      static uint32_t cache_hit;

      U_MESSAGE("cache_hit = %u", ++cache_hit);
      }
   */

   U_RETURN(last_result);
}

static int mese1, mese2;
static const char* enddoc;

bool PEC_report::setLine()
{
   U_TRACE(5, "PEC_report::setLine()")

   int nfault = 0;

loop1:
   if (t->next(*line, '\n'))
      {
      const char* end;

      if (mese2) mese1 = mese2;
      else
         {
         const char* ptr = line->data();

         mese1 = u_getMonth(ptr);

         U_INTERNAL_DUMP("mese1 = %d", mese1)

         if (mese1 == 0                 ||
             ptr[U_LINEDATA + 3] != ':' ||
             ptr[U_LINEDATA + 6] != ':')
            {
            if (++nfault >= 3)
               {
               U_WARNING("max 3 consecutive lines with unexpected format, file <%s> skipped...", file->getPathRelativ());

               failed = true;

               U_RETURN(false);
               }

            goto loop1;
            }
         }

loop2:
      if (t->atEnd()) U_RETURN(true);

      end = line->pend();

      while ((*end == '\n') && (++end < enddoc)) {}

      mese2 = u_getMonth(end);

      U_INTERNAL_DUMP("mese2 = %d", mese2)

      if (mese2                      &&
          end[U_LINEDATA + 3] == ':' &&
          end[U_LINEDATA + 6] == ':') U_RETURN(true);

      if (t->extend(*line, '\n')) goto loop2;
      }

   U_RETURN(false);
}

bool PEC_report::readContent()
{
   U_TRACE(5, "PEC_report::readContent()")

   *content = file->getContent();

   if (content->empty()) U_RETURN(false);

   if (rejected)
      {
      *content = UStringExt::gunzip(*content); 
      }
   else if (non_firmati == false)
      {
      UPKCS7 item(*content, "DER");

      if (item.isValid()) *content = item.getContent();
      else
         {
         U_WARNING("Error reading S/MIME message, may be the file <%s> is not signed...", file->getPathRelativ());

         // return; si prova comunque ad elaborare, eventualmente viene skippato tramite il contenuto...
         }
      }

   if (content->empty()) U_RETURN(false);

   mese2  = 0;
   enddoc = content->pend();

   // loop for all lines in file

   const char* ptr = U_NULLPTR;
   const char* prev = U_NULLPTR;
   bool ok = false, dicembre = false;

   // depends on content...

   clearData();

   t->setData(*content);

   while (setLine())
      {
      ptr = line->data();

      U_INTERNAL_DUMP("ptr = %.*S", 100, ptr)

      (void) u__strncpy(time, ptr + U_LINEDATA + 1, 8);

   // time[8] = '\0';

      if (prev == U_NULLPTR ||
          memcmp(ptr, prev, U_LINEDATA) != 0)
         {
         prev = ptr;

         // scanf for date at the start of line (feb 22 15:03:40 2007)

         /*
         int n = U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p", ptr, "%3s %2d %8s %4d", month, &day, time, &year);

         U_INTERNAL_DUMP("sscanf() - %d/%s/%d", day, month, year)

         if (n < 3) continue;
         */

         (void) u__strncpy(month, ptr, 3);

         // month[3] = '\0';

         day = atoi(ptr + 4);

         if (rejected == false)
            {
            year = atoi(ptr + U_LINEDATA + 10);

            year_present = (year > 0); // check if year is present or not...

            if (year_present == false) year = 2007;
            }

         U_INTERNAL_DUMP("scan line date: %d/%s/%d - %s", day, month, year, time)

         U_INTERNAL_ASSERT_EQUALS(mese1, (int)u_getMonth(month))

         date->set(day, mese1, year);

         U_ASSERT(date->isValid())

         if (rejected == false)
            {
            // check if year is present or not...

            if (year_present == false &&
                *date >= *fix)
               {
               date->setYear(year = 2006);
               }
            }
         else
            {
            // check if year is changed...

            if (date->getMonth() == 12)
               {
               if (dicembre == false) dicembre = true;
               }
            else
               {
               if (dicembre)
                  {
                  dicembre = false;

                  date->setYear(++year);
                  }
               }

            U_INTERNAL_DUMP("year = %d - dicembre = %b", year, dicembre)
            }

         if (checklink)
            {
            parse(); // request for parsing...

            continue;
            }

         ok = (!from || *date >= *from) &&
              (!to   || *date <= *to);

         // check for change date...

         if (ok) nseconds = date->getSecond();
         }

      if (ok)
         {
         if (rejected)
            {
                                      index = findField(rejected_type = U_stat);
            if (index == U_NOT_FOUND) index = findField(rejected_type = U_reject);
            if (index == U_NOT_FOUND) continue;
            }

         // find and check id content...

         if (setLineID() == false) continue;

         // find and check identifier content...

         setLineIdentifier();

         if (rejected ||
             (setLineTipology() &&  // find and check line tipology...
              setLineMittente()))   // find and check sender content...
            {
            parse(); // request for parsing...
            }
         }
      }

   if (ptr != U_NULLPTR) U_RETURN(true);

   U_RETURN(false);
}

bool PEC_report::processFile(UStringRep* key, void* elem)
{
   U_TRACE(5, "PEC_report::processFile(%.*S,%p)", U_STRING_TO_TRACE(*key), elem)

   UString _filename((UStringRep*)elem);

   file->setPath(_filename);

   U_MESSAGE("processing file <%s>...", file->getPathRelativ());

   failed = (readContent() == false);

   if (change_file)
      {
      cnt[0] = key->at(10);

      U_INTERNAL_DUMP("cnt[0] = %C", cnt[0])

      change_file(); // request for change file...
      }

   U_RETURN(true);
}

static uint32_t nfiles, nskipped;

void PEC_report::processFiles()
{
   U_TRACE(5, "PEC_report::processFiles()")

   U_INTERNAL_ASSERT_EQUALS(UDirWalk::isDirectory(), false)

   ++nfiles;

   // NB: need duplicate string because depends on buffer content of filename...

   UString _filename;

   UDirWalk::setFoundFile(_filename);

   const char* ptr      = _filename.c_str();
   const char* basename = u_basename(ptr);

   // check for file name ends with...

   if (filter_ext &&
       UStringExt::endsWith(_filename, *filter_ext) == false)
      {
      U_WARNING("file <%s> skipped...", ptr);

      ++nskipped;

      return;
      }

   char buf[12];
   UString key = _filename;

   if (rejected)
      {
      // scan_form -> "%*[^-]-%4c"

      int n = U_SYSCALL(sscanf, "%S,%S,%p", basename, scan_form->c_str(), buf);

      if (n != 1)
         {
         U_WARNING("found filename <%s> not valid...", ptr);

         ++nskipped;

         return;
         }

      buf[4] = '\0';

      year = atoi(buf);

      U_INTERNAL_DUMP("year = %d", year)
      }
   else
      {
      // scanf for date in the filename... (PEC1-2007-03-22#2007-03-23.log.p7m)

      int mese = 0, yearE = 0, meseE = 0, dayE = 0,
          n = U_SYSCALL(sscanf, "%S,%S,%p", basename, "PEC%1c-%4d-%2d-%2d#%4d-%2d-%2d", cnt, &year,  &mese,  &day,
                                                                                             &yearE, &meseE, &dayE);

      U_INTERNAL_DUMP("sscanf() = %d - %d/%d/%d # %d/%d/%d", n, day, mese, year, dayE, meseE, yearE)

      bool flag = false;

      if (n == 7)
         {
          date->set(day,  mese,  year);
         date1->set(dayE, meseE, yearE);

         flag = (date->isValid() && date1->isValid());
         }
      else if (n == 4)
         {
         date->set(day, mese, year);

         flag = date->isValid();
         }

      if (flag == false)
         {
         U_WARNING("found filename <%s> not valid...", ptr);

         ++nskipped;

         return;
         }

      if (optimization)
         {
         if (n == 4)
            {
            *date1 = *to + 1; // tmail sign after midnigth...

            flag = ((from && *date < *from) ||
                    (to   && *date > *date1));
            }
         else // n == 7
            {
            /* NB: attenzione, e' giusto cosi...
            *
            * a ------ b
            *             A ---------- B
            *                             a ------- b
            *
            * skip for (b < A || a > B)
            */

            flag = ((from && *date1 < *from) ||
                    (to   && *date  > *to));
            }

         if (flag)
            {
            U_WARNING("file <%s> out of requested date range: skipped...", ptr);

            ++nskipped;

            return;
            }
         }

      key = _filename.substr(_filename.distance(basename) + 5, 10);

      (void) sprintf(buf, "%c%d", cnt[0], nfiles);

      (void) key.append(buf);

      U_INTERNAL_DUMP("key = %.*S", U_STRING_TO_TRACE(key))
      }

   if (UFile::stat(ptr, (struct stat*)file))
      {
      bytes += file->st_size;

   // processFile(_filename.rep, _filename.rep);

      tfile->insert(key, _filename);
      }
}

void PEC_report::loadFiles()
{
   U_TRACE(5, "PEC_report::loadFiles()")

   UDirWalk dirwalk;

   UDirWalk::setRecurseSubDirs(true, false);

   dirwalk.call_internal = PEC_report::processFiles;

   dirwalk.walk();

   (void) u_printSize(buffer, bytes);

   U_MESSAGE("checked %u file(s) - skipped %u file(s)\n"
             "start processing %u file(s) for %s of data...", nfiles, nskipped, tfile->size(), buffer);

   start_func();

   if (bytes &&
       tfile->empty() == false) tfile->callForAllEntrySorted(PEC_report::processFile);

   (void) UFile::chdir(U_NULLPTR, true);
}

void PEC_report::manageOptions()
{
   U_TRACE(5, "PEC_report::manageOptions()")

   title     = new UString;
   filter    = new UString;
   directory = new UString;

   cfg_to      = opt['t'];
   cfg_str     = opt['c'];
   cfg_from    = opt['f'];
   cfg_suffix  = opt['e'];
   *filter     = opt['F'];
   *directory  = opt['d'];

   if (checklink == false)
      {
      *title = opt['T'];
      cfg_id = opt['i'];

      if (rejected == false) cfg_domain = opt['n'];
      }

   if (cfg_str)
      {
      if (!(cfg_to.empty()     &&
            cfg_id.empty()     &&
            title->empty()     &&
            cfg_from.empty()   &&
            directory->empty() &&
            cfg_suffix.empty() &&
            cfg_domain.empty()))
         {
         U_ERROR("you can't specify both configuration file and other parameters");
         }

      // manage file configuration

      cfg.load(cfg_str);

      // -----------------------------------------------------------------------------------------------
      // configuration parameters
      // -----------------------------------------------------------------------------------------------
      // REPORT_TITLE          title of the report
      // DIRECTORY             directory path containing log files
      // PARSING_FILE_NAME     format parsing for retrieve year from file name
      // FILE_NAME_ENDS_WITH   proces only filenames that ends with this string - OPTIONAL
      // DOMAINS_LIST_FILE     file containing list of own domain name - OPTIONAL
      // INSTALLATION_ID_LIST  list of installation ID to be used as filter separate by comma - OPTIONAL
      // STARTING_DATE         processing log entry from this date (dd/mm/yyyy) - OPTIONAL
      // ENDING_DATE           processing log entry to this date (dd/mm/yyyy) - OPTIONAL
      // FILTER                string for filter records that match this
      // -----------------------------------------------------------------------------------------------

      *title      = cfg[U_STRING_FROM_CONSTANT("REPORT_TITLE")];
      *directory  = cfg[U_STRING_FROM_CONSTANT("DIRECTORY")];
      cfg_suffix  = cfg[U_STRING_FROM_CONSTANT("FILE_NAME_ENDS_WITH")];
      cfg_id      = cfg[U_STRING_FROM_CONSTANT("INSTALLATION_ID_LIST")];
      cfg_from    = cfg[U_STRING_FROM_CONSTANT("STARTING_DATE")];
      cfg_to      = cfg[U_STRING_FROM_CONSTANT("ENDING_DATE")];
      *filter     = cfg[U_STRING_FROM_CONSTANT("FILTER")];

      if (rejected == false) cfg_domain = cfg[U_STRING_FROM_CONSTANT("DOMAINS_LIST_FILE")]; 
      }

   if (directory->empty()) U_ERROR("parameter directory is mandatory");

   optimization = (opt['o'] == U_STRING_FROM_CONSTANT("1"));

   if (checklink == false)
      {
      if (title->empty()) U_ERROR("parameter title report is mandatory");

      if (cfg_domain.empty())
         {
         if (rejected) scan_form = new U_STRING_FROM_CONSTANT("%*[^-]-%4c");
         else          U_ERROR("parameter domain is mandatory");
         }

      if (rejected == false)
         {
         domain = new UString(UFile::contentOf(cfg_domain));

         if (domain->empty()) U_ERROR("domain file <%.*s> not valid", U_STRING_TO_TRACE(cfg_domain));

         vdomain = new UVector<UString>(*domain);

         if (vdomain->size() > 1) vdomain->sort(true);
         }
      }

   if (UFile::chdir(directory->c_str(), true) == false) U_ERROR("chdir() to directory <%s> failed", directory->data());

   if (cfg_from)
      {
      from = new UTimeDate(cfg_from.c_str());

      if (from->isValid() == false) U_ERROR("starting date <%s> not valid", cfg_from.data());
      }

   if (cfg_to)
      {
      to = new UTimeDate(cfg_to.c_str());

      if (to->isValid() == false) U_ERROR("ending date <%s> not valid", cfg_to.data());
      }

   if (from && to && *from > *to) U_ERROR("start date <%s> and ending date <%s> not coherent", cfg_from.data(), cfg_to.data());

   if (cfg_suffix) filter_ext = new UString(cfg_suffix);

   id_all = cfg_id.empty(); // want all id...?

   vid = (id_all ? new UVector<UString>()
                 : new UVector<UString>(cfg_id, ","));

   t                = new UTokenizer;
   file             = new UFile;
   date             = new UTimeDate;
   date1            = new UTimeDate;
   line             = new UString;
   content          = new UString;
   last_dominio     = new UString;
   last_value_field = new UString;

   id               = new UString;
   mittente         = new UString;
   identifier       = new UString;

   tfile = new UHashMap<UString>(U_GET_NEXT_PRIME_NUMBER(1024));

   if (rejected)
      {
      vfields = new UString(U_CONSTANT_TO_PARAM("stat= "     // 0
                                                "reject= "   // 1
                                                "dsn="));    // 2
      vfield = new UVector<UString>(*vfields);
      }
   else
      {
      fix         = new UTimeDate("01/04/2007");

      vfields     = new UString(U_CONSTANT_TO_PARAM(U_TIPO " "        // 0
                                                    U_MITTENTE " "    // 1
                                                    U_DESTINATARI " " // 2
                                                    U_CONSEGNA " "    // 3
                                                    U_RICEZIONE " "   // 4
                                                    U_ERRORE));       // 5

      vtipos = new UString(U_CONSTANT_TO_PARAM(U_ACCETTAZIONE " "              // 0
                                               U_NON_ACCETTAZIONE " "          // 1
                                               U_AVVENUTA_CONSEGNA " "         // 2
                                               U_RILEVAZIONE_VIRUS " "         // 3
                                               U_POSTA_CERTIFICATA " "         // 4
                                               U_ERRORE_CONSEGNA " "           // 5
                                               U_PRESA_IN_CARICO " "           // 6
                                               U_PREAVVISO_ERRORE_CONSEGNA));  // 7

      vfield = new UVector<UString>(*vfields);
      vtipo  = new UVector<UString>(*vtipos);
      }
}

#define U_XML_OUT_START                               \
"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"   \
"<!DOCTYPE REPORT SYSTEM \"PEC_Report.dtd\">\n"       \
"\n"                                                  \
"<REPORT>\n"                                          \
"  <NAME>%.*s</NAME>\n"                               \
"  <QUERY>\n"                                         \
"    <START-DATE>%.*s</START-DATE>\n"                 \
"    <END-DATE>%.*s</END-DATE>\n"
#define U_XML_OUT_START_A                             \
"    <INSTALLATION-ID>%.*s</INSTALLATION-ID>\n"
#define U_XML_OUT_START_B                             \
"  </QUERY>\n"                                        \
"  <RESULTS>\n"

#define U_XML_OUT_END \
"  </RESULTS>\n"      \
"</REPORT>\n"

void PEC_report::startReport()
{
   U_TRACE(5, "PEC_report::startReport()")

   (void) sprintf(buffer, U_XML_OUT_START, U_STRING_TO_TRACE(*title), U_STRING_TO_TRACE(cfg_from), U_STRING_TO_TRACE(cfg_to));

   std::cout << buffer;

   UString elem;

   for (uint32_t i = 0, n = vid->size(); i < n; ++i)
      {
      elem = vid->at(i);

      (void) sprintf(buffer, U_XML_OUT_START_A, U_STRING_TO_TRACE(elem));

      std::cout << buffer;
      }

   std::cout << U_XML_OUT_START_B;
}

void PEC_report::endReport()
{
   U_TRACE(5, "PEC_report::endReport()")

   clearData();

   end_func();

   std::cout << U_XML_OUT_END;

   clearData();
}
