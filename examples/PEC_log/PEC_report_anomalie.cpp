// PEC_report_anomalie.cpp

#include "PEC_report_anomalie.h"

#include <ulib/container/hash_map.h>

time_t     Messaggio::now;
time_t     Messaggio::max_ttl;
time_t     Messaggio::range_ttl;
UString*   Messaggio::id_max_ttl;
Messaggio* Messaggio::msg;

uint32_t PEC_report_anomalie::nmsg;
uint32_t PEC_report_anomalie::nanomalie;
uint32_t PEC_report_anomalie::nout_of_range;

#define U_OUTPUT              0
#define U_CONSEGNA            1
#define U_ACCETTAZIONE        2
#define U_BUSTA_TRASPORTO     3
#define U_PRESA_IN_CARICO     4
#define U_MITTENTE_CHANGE     5
#define U_RILEVAZIONE_VIRUS   6
#define U_DESTINATARI_CHANGE  7

#define U_ERROR_MESSAGE_INCORRECT_MESSAGE "incoherent message - id = <%.*s>"

#ifdef U_COVERITY_FALSE_POSITIVE
#define U_MEMSET_VALUE  0
#else
#define U_MEMSET_VALUE '0'
#endif

static inline void swap(time_t& a, time_t& b)
{
   U_TRACE(5, "::swap(%p,%p)", &a, &b)

   time_t tmp = a;
   a          = b;
   b          = tmp;
}

inline void Messaggio::allocDestinatari(int n)
{
   U_TRACE(5, "Messaggio::allocDestinatari(%d)", n)

   U_CHECK_MEMORY

   vdestinatari_domini                 = (char*) malloc(n+1);
   vdestinatari_certificati            = (char*) malloc(n+1);
   vdestinatari_presa_in_carico        = (char*) malloc(n+1);
   vdestinatari_mancata_consegna       = (char*) malloc(n+1);
   vdestinatari_avvenuta_consegna      = (char*) malloc(n+1);
   vdestinatari_rilevazione_virus      = (char*) malloc(n+1);
   vdestinatari_mancata_consegna_virus = (char*) malloc(n+1);

   // coverity[NO_EFFECT]
   (void) memset(vdestinatari_presa_in_carico,        U_MEMSET_VALUE, n);
   (void) memset(vdestinatari_mancata_consegna,       U_MEMSET_VALUE, n);
   (void) memset(vdestinatari_avvenuta_consegna,      U_MEMSET_VALUE, n);
   (void) memset(vdestinatari_rilevazione_virus,      U_MEMSET_VALUE, n);
   (void) memset(vdestinatari_mancata_consegna_virus, U_MEMSET_VALUE, n);

   vdestinatari_domini[n]                 =
   vdestinatari_certificati[n]            =
   vdestinatari_presa_in_carico[n]        =
   vdestinatari_mancata_consegna[n]       =
   vdestinatari_avvenuta_consegna[n]      =
   vdestinatari_rilevazione_virus[n]      =
   vdestinatari_mancata_consegna_virus[n] = '\0';
}

Messaggio::Messaggio() : id(*PEC_report::id), mittente(*PEC_report::mittente), identifier(*PEC_report::identifier)
{
   U_TRACE_REGISTER_OBJECT(5, Messaggio, "")

   (void) memset(flag, U_MEMSET_VALUE, sizeof(flag));

   ttl         = 0;
   start       = PEC_report::nseconds + PEC_report::getTime();
   destinatari = PEC_report::getValueField(U_destinatari);

   U_INTERNAL_DUMP("start                = %ld (%#19D)", start, start)
   U_INTERNAL_DUMP("PEC_report::nseconds = %ld (%#19D)", PEC_report::nseconds, PEC_report::nseconds)

   // NB: need duplicate string because depends on mmap()'s content of document...

            id.duplicate();
      mittente.duplicate();
    identifier.duplicate();
   destinatari.duplicate();

   UVector<UString> vec(destinatari, ", ");

   UString elem;
   uint32_t n = vec.size();

   char* vdestcert   = (char*) malloc(n+1);
   char* vdestdomini = (char*) malloc(n+1);

   // coverity[NO_EFFECT]
   (void) memset(vdestcert,   U_MEMSET_VALUE, n);
   (void) memset(vdestdomini, U_MEMSET_VALUE, n);

   for (uint32_t i = 0; i < n; ++i)
      {
      elem = vec.at(i);

      if (elem.c_char(0) == '(')
         {
         if (memcmp(elem.data(), U_CONSTANT_TO_PARAM("(certificato")) == 0) vdestcert[vdestinatari.size()-1] = '1';

         continue;
         }

      vdestdomini[vdestinatari.size()] = (PEC_report::isDomainAddress(elem) ? '1' : '0');

      vdestinatari.push_back(elem);
      }

   n = vdestinatari.size();

   allocDestinatari(n);

   U_MEMCPY(vdestinatari_domini,      vdestdomini, n);
   U_MEMCPY(vdestinatari_certificati, vdestcert,   n);

   free(vdestcert);
   free(vdestdomini);

   U_INTERNAL_DUMP("vdestinatari_certificati = %S", vdestinatari_certificati)
   U_INTERNAL_DUMP("vdestinatari_domini      = %S", vdestinatari_domini)
}

Messaggio::~Messaggio()
{
   U_TRACE_UNREGISTER_OBJECT(5, Messaggio)

   free(vdestinatari_domini);
   free(vdestinatari_certificati);
   free(vdestinatari_presa_in_carico);
   free(vdestinatari_mancata_consegna);
   free(vdestinatari_avvenuta_consegna);
   free(vdestinatari_rilevazione_virus);
   free(vdestinatari_mancata_consegna_virus);
}

bool Messaggio::setRicezione(const UString& elem)
{
   U_TRACE(5, "Messaggio::setRicezione(%.*S)", U_STRING_TO_TRACE(elem))

   // 3) Ogni indirizzo email certificato contenuto nel campo destinatari di una busta di trasporto, dovrà apparire una sola
   // volta nell'attributo ricezione di una presa in carico/rilevazione virus con lo stesso identificativo

   uint32_t i = msg->vdestinatari.find(elem, true);

   if (i == U_NOT_FOUND ||
       msg->vdestinatari_certificati[i] == '0')
      {
      if (PEC_report::tipology[U_presa_in_carico])
         {
         msg->flag[U_PRESA_IN_CARICO] = '1';
         }
      else
         {
         U_INTERNAL_ASSERT(PEC_report::tipology[U_rilevazione_virus])

         msg->flag[U_RILEVAZIONE_VIRUS] = '1';
         }
      }
   else if (PEC_report::tipology[U_presa_in_carico])
      {
      msg->vdestinatari_presa_in_carico[i] += 1;
      }
   else
      {
      U_INTERNAL_ASSERT(PEC_report::tipology[U_rilevazione_virus])

      msg->vdestinatari_rilevazione_virus[i] += 1;
      }

   U_RETURN(false);
}

bool Messaggio::setConsegna(const UString& elem)
{
   U_TRACE(5, "Messaggio::setConsegna(%.*S)", U_STRING_TO_TRACE(elem))

   // 5) Ogni indirizzo email certificato contenuto nel campo destinatari di una busta di trasporto, dovrà apparire una sola
   //    volta nell'attributo consegna di una avvenuta/mancata consegna con lo stesso identificativo

   uint32_t i = msg->vdestinatari.find(elem, true);

   if (i == U_NOT_FOUND ||
       msg->vdestinatari_certificati[i] == '0')
      {
      if (PEC_report::tipology[U_errore_consegna] ||
          PEC_report::tipology[U_avvenuta_consegna])
         {
         msg->flag[U_CONSEGNA] = '1';
         }
      else
         {
         U_INTERNAL_ASSERT(PEC_report::tipology[U_rilevazione_virus])

         msg->flag[U_RILEVAZIONE_VIRUS] = '1';
         }
      }
   else if (PEC_report::tipology[U_errore_consegna])
      {
      msg->vdestinatari_mancata_consegna[i] += 1;

      bool errore_virus = PEC_report::checkValueField(U_errore, U_STRING_FROM_CONSTANT("virus"));

      U_INTERNAL_DUMP("errore_virus = %b", errore_virus)

      if (errore_virus) msg->vdestinatari_mancata_consegna_virus[i] += 1;
      }
   else if (PEC_report::tipology[U_avvenuta_consegna])
      {
      msg->vdestinatari_avvenuta_consegna[i] += 1;
      }
   else
      {
      U_INTERNAL_ASSERT(PEC_report::tipology[U_rilevazione_virus])

      msg->vdestinatari_rilevazione_virus[i] += 1;
      }

   U_RETURN(false);
}

void PEC_report_anomalie::processLine(bool bnew)
{
   U_TRACE(5, "PEC_report_anomalie::processLine(%b)", bnew)

   if (bnew == false)
      {
      Messaggio::now = PEC_report::nseconds + PEC_report::getTime();

      if (Messaggio::now < Messaggio::msg->start) swap(Messaggio::now, Messaggio::msg->start);

      Messaggio::msg->ttl = (Messaggio::now - Messaggio::msg->start);

      if (Messaggio::id_max_ttl &&
          Messaggio::msg->ttl > Messaggio::max_ttl)
         {
          Messaggio::max_ttl    = Messaggio::msg->ttl;
         *Messaggio::id_max_ttl = Messaggio::msg->identifier;

         U_INTERNAL_DUMP("Messaggio::max_ttl    = %ld", Messaggio::max_ttl)
         U_INTERNAL_DUMP("Messaggio::id_max_ttl = %.*S", U_STRING_TO_TRACE(*Messaggio::id_max_ttl))
         }

      if (Messaggio::msg->ttl > Messaggio::range_ttl)
         {
         static UString* prev_id_ttl;

         if (prev_id_ttl != PEC_report::identifier)
            {
            prev_id_ttl = PEC_report::identifier;

            ++nout_of_range;

            U_WARNING(U_ERROR_MESSAGE_OUT_OF_TTL, Messaggio::msg->ttl  / U_ONE_DAY_IN_SECOND,
                                                  Messaggio::range_ttl / U_ONE_DAY_IN_SECOND,
                                                  U_STRING_TO_TRACE(*PEC_report::identifier));
            }
         }

      if (Messaggio::msg->flag[U_MITTENTE_CHANGE]    == '1' ||
          Messaggio::msg->flag[U_DESTINATARI_CHANGE] == '1') return;

      if (mittente->equalnocase(Messaggio::msg->mittente) == false)
         {
         Messaggio::msg->flag[U_MITTENTE_CHANGE] = '1';

         if (tipology[U_posta_certificata]) Messaggio::msg->mittente.replace(*mittente);

         return;
         }

      UString destinatari = PEC_report::getValueField(U_destinatari);

      if (destinatari.equalnocase(Messaggio::msg->destinatari) == false)
         {
         Messaggio::msg->flag[U_DESTINATARI_CHANGE] = '1';

         return;
         }
      }

   if (tipology[U_posta_certificata])
      {
      // 1) Deve esistere una BustaTrasporto (tipo=posta-certificata)

      Messaggio::msg->flag[U_BUSTA_TRASPORTO] += 1;

      U_INTERNAL_DUMP("flag[U_BUSTA_TRASPORTO] = %C", Messaggio::msg->flag[U_BUSTA_TRASPORTO])

      // check if messaggio incoerente

      if (PEC_report::isDomainAddress(Messaggio::msg->mittente)) Messaggio::msg->flag[U_OUTPUT] = '1';

      U_INTERNAL_DUMP("flag[U_OUTPUT] = %C", Messaggio::msg->flag[U_OUTPUT])

      if (Messaggio::msg->flag[U_OUTPUT] == '0' &&
          atoi(Messaggio::msg->vdestinatari_domini) == 0)
         {
         U_WARNING(U_ERROR_MESSAGE_INCORRECT_MESSAGE, U_STRING_TO_TRACE(*PEC_report::identifier));
         }
      }
   else if (tipology[U_accettazione])
      {
      // 2) Per una BustaTrasporto (tipo=posta-certificata) dove il mittente appartiene a dominio specificato
      // deve esistere una sola Accettazione (tipo=accettazione)

      Messaggio::msg->flag[U_ACCETTAZIONE] += 1;
      }
   else if (tipology[U_presa_in_carico])
      {
      (void) PEC_report::callForAllEntryField(U_ricezione, Messaggio::setRicezione);
      }
   else if (tipology[U_errore_consegna])
      {
      (void) PEC_report::callForAllEntryField(U_consegna, Messaggio::setConsegna);
      }
   else if (tipology[U_avvenuta_consegna])
      {
      (void) PEC_report::callForAllEntryField(U_consegna, Messaggio::setConsegna);
      }
   else if (tipology[U_rilevazione_virus])
      {
      // from version > 2.0...

   // int U_field       = U_consegna;
      UString U_value   = getValueField(U_consegna);
      bPFstr U_function = Messaggio::setConsegna; 

      if (U_value.empty())
         {
     //  U_field    = U_ricezione;
         U_value    = getValueField(U_ricezione);
         U_function = Messaggio::setRicezione; 
         }

      (void) PEC_report::callForAllEntryField(U_value, U_function);
      }

   // cerr << *Messaggio::msg << std::endl;
}

bool Messaggio::isAnomalia()
{
   U_TRACE(5, "Messaggio::isAnomalia()")

   U_CHECK_MEMORY

   if (flag[U_BUSTA_TRASPORTO] == '0')
      {
      if (PEC_report::isDomainAddress(mittente)) flag[U_OUTPUT] = '1';

      U_INTERNAL_DUMP("flag[U_OUTPUT] = %C", flag[U_OUTPUT])
      }

   if (flag[U_MITTENTE_CHANGE]    == '1' ||
       flag[U_DESTINATARI_CHANGE] == '1' ||
       flag[U_BUSTA_TRASPORTO]    != '1' ||
       flag[U_CONSEGNA]           != '0' ||
       flag[U_PRESA_IN_CARICO]    != '0' ||
       flag[U_RILEVAZIONE_VIRUS]  != '0' ||
       flag[U_OUTPUT]             != flag[U_ACCETTAZIONE])
      {
      U_INTERNAL_DUMP("flag[U_OUTPUT]             = %C", Messaggio::msg->flag[U_OUTPUT])
      U_INTERNAL_DUMP("flag[U_ACCETTAZIONE]       = %C", Messaggio::msg->flag[U_ACCETTAZIONE])
      U_INTERNAL_DUMP("flag[U_CONSEGNA]           = %C", Messaggio::msg->flag[U_CONSEGNA])
      U_INTERNAL_DUMP("flag[U_BUSTA_TRASPORTO]    = %C", Messaggio::msg->flag[U_BUSTA_TRASPORTO])
      U_INTERNAL_DUMP("flag[U_PRESA_IN_CARICO]    = %C", Messaggio::msg->flag[U_PRESA_IN_CARICO])
      U_INTERNAL_DUMP("flag[U_MITTENTE_CHANGE]    = %C", Messaggio::msg->flag[U_MITTENTE_CHANGE])
      U_INTERNAL_DUMP("flag[U_RILEVAZIONE_VIRUS]  = %C", Messaggio::msg->flag[U_RILEVAZIONE_VIRUS])
      U_INTERNAL_DUMP("flag[U_DESTINATARI_CHANGE] = %C", Messaggio::msg->flag[U_DESTINATARI_CHANGE])

      U_RETURN(true);
      }

   for (uint32_t i = 0, n = vdestinatari.size(); i < n; ++i)
      {
      /*
      3A) Per ogni rilevazione virus con lo stesso identificativo ci deve essere una sola mancata consegna (errore=virus)
      con cui il campo consegna coincide

      3B) Per ogni mancata consegna (errore=virus) con lo stesso identificativo ci deve essere una sola rilevazione virus
      con cui il campo consegna coincide
      */

      if ((flag[U_OUTPUT] == '1') &&
          (vdestinatari_rilevazione_virus[i] != vdestinatari_mancata_consegna_virus[i]))
         {
         U_INTERNAL_DUMP("vdestinatari_rilevazione_virus[%d]      = %C", i, vdestinatari_rilevazione_virus[i])
         U_INTERNAL_DUMP("vdestinatari_mancata_consegna_virus[%d] = %C", i, vdestinatari_mancata_consegna_virus[i])

         U_RETURN(true); 
         }

      if (vdestinatari_certificati[i] == '0')
         {
         if (vdestinatari_presa_in_carico[i]   != '0' ||
             vdestinatari_mancata_consegna[i]  != '0' ||
             vdestinatari_avvenuta_consegna[i] != '0' ||
             vdestinatari_rilevazione_virus[i] != '0')
            {
            U_RETURN(true);
            }
         }
      else
         {
         if (flag[U_OUTPUT] == '1')
            {
            // msg SPEDITO - tutti i destinatari CERTIFICATI...

            if (vdestinatari_presa_in_carico[i] != '1'||
                ((vdestinatari_mancata_consegna[i]  - '0') +
                 (vdestinatari_avvenuta_consegna[i] - '0')) != 1)
               {
               U_RETURN(true);
               }
            }
         else
            {
            // msg RICEVUTO - tutti i destinatari CERTIFICATI di domini APPARTENENTI a domini specificati...

            if (vdestinatari_domini[i] == '1')
               {
               if (vdestinatari_presa_in_carico[i] != '1')
                  {
                  U_RETURN(true);
                  }

               if (((vdestinatari_mancata_consegna[i]  - '0') +
                    (vdestinatari_avvenuta_consegna[i] - '0') +
                    (vdestinatari_rilevazione_virus[i] - '0')) != 1)
                  {
                  U_RETURN(true);
                  }
               }
            else
               {
               // ...i destinatari CERTIFICATI DEVONO appartenere a domini specificati...

               if (vdestinatari_presa_in_carico[i]   != '0' ||
                   vdestinatari_mancata_consegna[i]  != '0' ||
                   vdestinatari_avvenuta_consegna[i] != '0' ||
                   vdestinatari_rilevazione_virus[i] != '0')
                  {
                  U_RETURN(true);
                  }
               }
            }
         }
      }

   U_RETURN(false);
}

void PEC_report_anomalie::reportDestinatari(void* elem)
{
   U_TRACE(5, "PEC_report_anomalie::reportDestinatari(%p)", elem)

   UStringRep* str = (UStringRep*)elem;

   UString tmp(U_CAPACITY);

   UXMLEscape::encode(UString(str), tmp);

   (void) sprintf(buffer, U_XML_MSG_ENTRY_Recipients, U_STRING_TO_TRACE(tmp));

   std::cout << buffer;
}

bool PEC_report_anomalie::reportAnomalie(UStringRep* key, void* elem)
{
   U_TRACE(5, "PEC_report_anomalie::reportAnomalie(%p,%p)", key, elem)

   U_INTERNAL_ASSERT_POINTER(elem);

   Messaggio::msg = (Messaggio*)elem;

   ++nanomalie;

   UString tmp1(U_CAPACITY), tmp2(U_CAPACITY);

   UXMLEscape::encode(Messaggio::msg->identifier, tmp1);
   UXMLEscape::encode(Messaggio::msg->mittente,   tmp2);

   (void) sprintf(buffer, U_XML_MSG_ENTRY_START,
                  Messaggio::msg->flag[U_OUTPUT] == '1' ? "Output" : "Input",
                  U_STRING_TO_TRACE(tmp1),
                  Messaggio::msg->getData(),
                  U_STRING_TO_TRACE(Messaggio::msg->id),
                  U_STRING_TO_TRACE(tmp2));

   std::cout << buffer;

   Messaggio::msg->vdestinatari.callForAllEntry(PEC_report_anomalie::reportDestinatari);

   if (Messaggio::msg->flag[U_MITTENTE_CHANGE] == '1')
      {
      (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Mittente incoerente");

      std::cout << buffer;
      }

   if (Messaggio::msg->flag[U_DESTINATARI_CHANGE] == '1')
      {
      (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Destinatari incoerenti");

      std::cout << buffer;
      }

   const char* testo;
   uint32_t num_consegne, i, n;
   bool flag_presa_in_carico_mancante, flag_presa_in_carico_duplicato, flag_consegna_mancante, flag_consegna_duplicato;

   if (Messaggio::msg->flag[U_MITTENTE_CHANGE]    == '1' ||
       Messaggio::msg->flag[U_DESTINATARI_CHANGE] == '1') goto end;

   if (Messaggio::msg->flag[U_BUSTA_TRASPORTO] != '1')
      {
      if (Messaggio::msg->flag[U_BUSTA_TRASPORTO] == '0')(void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Busta di trasporto mancante");
      else                                               (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Busta di trasporto duplicata");

      std::cout << buffer;
      }

   if (Messaggio::msg->flag[U_OUTPUT] != Messaggio::msg->flag[U_ACCETTAZIONE])
      {
      if      (Messaggio::msg->flag[U_ACCETTAZIONE] == '0') (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Accettazione mancante");
      else if (Messaggio::msg->flag[U_ACCETTAZIONE] == '1') (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Accettazione incoerente");
      else                                                  (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Accettazione duplicata");

      std::cout << buffer;
      }

   flag_presa_in_carico_mancante = flag_presa_in_carico_duplicato = flag_consegna_mancante = flag_consegna_duplicato = false;

   for (i = 0, n = Messaggio::msg->vdestinatari.size(); i < n; ++i)
      {
      /*
      3A) Per ogni rilevazione virus con lo stesso identificativo ci deve essere una sola mancata consegna (errore=virus)
      con cui il campo consegna coincide

      3B) Per ogni mancata consegna (errore=virus) con lo stesso identificativo ci deve essere una sola rilevazione virus
      con cui il campo consegna coincide
      */

      if ((Messaggio::msg->flag[U_OUTPUT] == '1') &&
          (Messaggio::msg->vdestinatari_rilevazione_virus[i] !=
           Messaggio::msg->vdestinatari_mancata_consegna_virus[i]))
         {
         if (Messaggio::msg->vdestinatari_rilevazione_virus[i] == '0')
            {
            testo = "Rilevazione virus mancante per un destinatario";
            }
         else if (Messaggio::msg->vdestinatari_mancata_consegna_virus[i] == '0')
            {
            testo = "Mancata consegna virus mancante per un destinatario";
            }
         else
            {
            if (Messaggio::msg->vdestinatari_rilevazione_virus[i] >
                Messaggio::msg->vdestinatari_mancata_consegna_virus[i])
               {
               testo = "Rilevazione virus duplicata per un destinatario";
               }
            else
               {
               U_INTERNAL_ASSERT_MINOR(Messaggio::msg->vdestinatari_rilevazione_virus[i],
                                       Messaggio::msg->vdestinatari_mancata_consegna_virus[i])

               testo = "Mancata consegna virus duplicata per un destinatario";
               }
            }

         (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, testo);

         std::cout << buffer;
         }

      // destinatari NON CERTIFICATI...

      if (Messaggio::msg->vdestinatari_certificati[i] == '0')
         {
         // 4A

         if (Messaggio::msg->vdestinatari_presa_in_carico[i] != '0')
            {
            Messaggio::msg->flag[U_PRESA_IN_CARICO] = '1';
            }

         // 4B

         if (Messaggio::msg->vdestinatari_rilevazione_virus[i] != '0')
            {
            Messaggio::msg->flag[U_RILEVAZIONE_VIRUS] = '1';
            }

         // 5

         if (Messaggio::msg->vdestinatari_mancata_consegna[i]  != '0' ||
             Messaggio::msg->vdestinatari_avvenuta_consegna[i] != '0')
            {
            Messaggio::msg->flag[U_CONSEGNA] = '1';
            }
         }
      else
         {
         if (Messaggio::msg->flag[U_OUTPUT] == '1')
            {
            // msg SPEDITO - tutti i destinatari CERTIFICATI...

            if (Messaggio::msg->vdestinatari_presa_in_carico[i] != '1')
               {
               if (Messaggio::msg->vdestinatari_presa_in_carico[i] == '0')
                  {
                  if (flag_presa_in_carico_mancante) continue;

                  flag_presa_in_carico_mancante = true;

                  testo = "Presa in carico mancante per almeno un destinatario";
                  }
               else
                  {
                  if (flag_presa_in_carico_duplicato) continue;

                  flag_presa_in_carico_duplicato = true;

                  testo = "Presa in carico duplicata per almeno un destinatario";
                  }

               (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, testo);

               std::cout << buffer;
               }

            num_consegne = (Messaggio::msg->vdestinatari_mancata_consegna[i]  - '0') +
                           (Messaggio::msg->vdestinatari_avvenuta_consegna[i] - '0');

            U_INTERNAL_DUMP("num_consegne = %u", num_consegne)

            if (num_consegne != 1)
               {
               if (num_consegne == 0)
                  {
                  if (flag_consegna_mancante) continue;

                  flag_consegna_mancante = true;

                  testo = "Avvenuta consegna o errore consegna mancante per almeno un destinatario";
                  }
               else
                  {
                  if (flag_consegna_duplicato) continue;

                  flag_consegna_duplicato = true;

                  testo = (Messaggio::msg->vdestinatari_mancata_consegna[i]  != '0' &&
                           Messaggio::msg->vdestinatari_avvenuta_consegna[i] != '0')
                           ? "Avvenuta consegna o errore consegna in conflitto per almeno un destinatario"
                           : "Avvenuta consegna o errore consegna duplicata per almeno un destinatario";
                  }

               (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, testo);

               std::cout << buffer;
               }
            }
         else
            {
            // msg RICEVUTO - tutti i destinatari CERTIFICATI di domini APPARTENENTI a domini specificati...

            if (Messaggio::msg->vdestinatari_domini[i] == '1')
               {
               if (Messaggio::msg->vdestinatari_presa_in_carico[i] != '1')
                  {
                  if (Messaggio::msg->vdestinatari_presa_in_carico[i] == '0')
                     {
                     if (flag_presa_in_carico_mancante) continue;

                     flag_presa_in_carico_mancante = true;

                     testo = "Presa in carico mancante per almeno un destinatario";
                     }
                  else
                     {
                     if (flag_presa_in_carico_duplicato) continue;

                     flag_presa_in_carico_duplicato = true;

                     testo = "Presa in carico duplicata per almeno un destinatario";
                     }

                  (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, testo);

                  std::cout << buffer;
                  }

               num_consegne = (Messaggio::msg->vdestinatari_mancata_consegna[i]  - '0') +
                              (Messaggio::msg->vdestinatari_avvenuta_consegna[i] - '0') +
                              (Messaggio::msg->vdestinatari_rilevazione_virus[i] - '0');

               U_INTERNAL_DUMP("num_consegne = %u", num_consegne)

               if (num_consegne != 1)
                  {
                  if (num_consegne == 0)
                     {
                     if (flag_consegna_mancante) continue;

                     flag_consegna_mancante = true;

                     testo = "Avvenuta consegna o errore consegna o rilevazione virus mancante per almeno un destinatario";
                     }
                  else
                     {
                     if (flag_consegna_duplicato) continue;

                     flag_consegna_duplicato = true;

                     testo = (Messaggio::msg->vdestinatari_mancata_consegna[i]  != '0' &&
                              Messaggio::msg->vdestinatari_avvenuta_consegna[i] != '0' &&
                              Messaggio::msg->vdestinatari_rilevazione_virus[i] != '0')
                           ? "Avvenuta consegna o errore consegna o rilevazione virus in conflitto per almeno un destinatario"
                           : "Avvenuta consegna o errore consegna o rilevazione virus duplicata per almeno un destinatario";
                     }

                  (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, testo);

                  std::cout << buffer;
                  }
               }
            else
               {
               // ...i destinatari CERTIFICATI DEVONO appartenere a domini specificati...

               if (Messaggio::msg->vdestinatari_presa_in_carico[i] != '0')
                  {
                  Messaggio::msg->flag[U_PRESA_IN_CARICO] = '1';
                  }

               if (Messaggio::msg->vdestinatari_rilevazione_virus[i] != '0')
                  {
                  Messaggio::msg->flag[U_RILEVAZIONE_VIRUS] = '1';
                  }

               if (Messaggio::msg->vdestinatari_mancata_consegna[i]  != '0' ||
                   Messaggio::msg->vdestinatari_avvenuta_consegna[i] != '0')
                  {
                  Messaggio::msg->flag[U_CONSEGNA] = '1';
                  }
               }
            }
         }
      }

   // 4A

   if (Messaggio::msg->flag[U_PRESA_IN_CARICO] != '0')
      {
      (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Presa in carico incoerente per almeno un destinatario");

      std::cout << buffer;
      }

   // 4B

   if (Messaggio::msg->flag[U_RILEVAZIONE_VIRUS] != '0')
      {
      (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Rilevazione virus incoerente per almeno un destinatario");

      std::cout << buffer;
      }

   // 5

   if (Messaggio::msg->flag[U_CONSEGNA] != '0')
      {
      (void) sprintf(buffer, U_XML_MSG_ENTRY_Exception, "Avvenuta consegna o errore consegna incoerente per almeno un destinatario");

      std::cout << buffer;
      }

end:
   (void) sprintf(buffer, U_XML_MSG_ENTRY_END, Messaggio::msg->flag[U_OUTPUT] == '1' ? "Output" : "Input");

   std::cout << buffer;

   U_RETURN(true);
}

// STREAM

U_EXPORT istream& operator>>(istream& is, Messaggio& m)
{
   U_TRACE(5, "Messaggio::operator>>(%p,%p)", &is, &m)

   U_INTERNAL_ASSERT_EQUALS(is.peek(), '{')

   is.get(); // skip '{'

   is >> m.flag
      >> m.start
      >> m.ttl
      >> m.id
      >> m.mittente
      >> m.identifier
      >> m.destinatari;

   int num_destinatari = 0;

   is >> num_destinatari;

   if (num_destinatari > 0) m.allocDestinatari(num_destinatari);

   is >> m.vdestinatari_certificati
      >> m.vdestinatari_domini
      >> m.vdestinatari_presa_in_carico
      >> m.vdestinatari_mancata_consegna
      >> m.vdestinatari_avvenuta_consegna
      >> m.vdestinatari_rilevazione_virus
      >> m.vdestinatari_mancata_consegna_virus
      >> m.vdestinatari;

   // skip '}'

   int c;
   streambuf* sb = is.rdbuf();

   do { c = sb->sbumpc(); } while (c != '}' && c != EOF);

   if (c == EOF) is.setstate(ios::eofbit);

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, const Messaggio& m)
{
   U_TRACE(5, "Messaggio::operator<<(%p,%p)", &os, &m)

   os.put('{');
   os.put(' ');
   os << m.flag;
   os.put(' ');
   os << m.start;
   os.put(' ');
   os << m.ttl;
   os.put(' ');
   m.id.write(os);
   os.put(' ');
   m.mittente.write(os);
   os.put(' ');
   m.identifier.write(os);
   os.put(' ');
   m.destinatari.write(os);
   os.put(' ');
   os << m.vdestinatari.size();
   os.put(' ');
   os << m.vdestinatari_certificati;
   os.put(' ');
   os << m.vdestinatari_domini;
   os.put(' ');
   os << m.vdestinatari_presa_in_carico;
   os.put(' ');
   os << m.vdestinatari_mancata_consegna;
   os.put(' ');
   os << m.vdestinatari_avvenuta_consegna;
   os.put(' ');
   os << m.vdestinatari_rilevazione_virus;
   os.put(' ');
   os << m.vdestinatari_mancata_consegna_virus;
   os.put(' ');
   os << m.vdestinatari;
   os.put(' ');
   os.put('}');

   return os;
}

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* Messaggio::dump(bool reset) const
{
   *UObjectIO::os << "ttl                                  " << ttl          << '\n'
                  << "start                                " << (void*)start << '\n'
                  << "flag                                 ";

   char buffer[2048];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", flag));

   *UObjectIO::os << '\n'
                  << "vdestinatari_domini                  ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_domini));

   *UObjectIO::os << '\n'
                  << "vdestinatari_certificati             ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_certificati));

   *UObjectIO::os << '\n'
                  << "vdestinatari_presa_in_carico         ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_presa_in_carico));

   *UObjectIO::os << '\n'
                  << "vdestinatari_mancata_consegna        ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_mancata_consegna));

   *UObjectIO::os << '\n'
                  << "vdestinatari_avvenuta_consegna       ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_avvenuta_consegna));

   *UObjectIO::os << '\n'
                  << "vdestinatari_rilevazione_virus       ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_rilevazione_virus));

   *UObjectIO::os << '\n'
                  << "vdestinatari_mancata_consegna_virus  ";

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%S", vdestinatari_mancata_consegna_virus));

   *UObjectIO::os << '\n'
                  << "id                (UString           " << (void*)&id           << ")\n"
                  << "mittente          (UString           " << (void*)&mittente     << ")\n"
                  << "identifier        (UString           " << (void*)&identifier   << ")\n"
                  << "destinatari       (UString           " << (void*)&destinatari  << ")\n"
                  << "vdestinatari      (UVector<UString>) " << (void*)&vdestinatari << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
