// OtpAuthToken.cpp

#include <OtpAuthToken.h>

#ifdef U_PROXY_UNIT
#  include <DES3engine.h>
#  include <Base64engine.h>
extern "C" { char* strptime(const char* s, const char* format, struct tm* tm); }
#else
#  include <ulib/base/coder/base64.h>
#  include <ulib/base/ssl/des3.h>
#endif

#include <time.h>
#include <stdio.h>

/*
Formato del cookie di autenticazione
--------------------------------------------------------------------------------
Authentication Token
Des3_encrypt(TID=Current_Server_ID;UID=User_ID;SID=Session_ID;TS=Timestamp;CF=Codice_Fiscale[;MIGRATE])

Nota:
E' auspicabile che  CF=Codice_Fiscale sia esprimibile tramite configurazione in
Des3_encrypt(TID=Current_Server_ID;UID=User_ID;SID=Session_ID;TS=Timestamp[;[HP1=Profile_Header1][,HPn=Profile_Headern][...]][;MIGRATE])

Descrizione dei campi
--------------------------------------------------------------------------------
Current_Server_ID identificatore del trustACCESS che ha richiesto il Cookie con Set-Cookie
User_ID           identificatore dell'utente loggato
Session_ID        Identificativo di sessione
Timestamp         timestamp nel formato AAAMMGGHHSS al momento del Set-Cookie
Codice_Fiscale    Codice Fiscale utente reperito dai dati di profilo.
MIGRATE           identificatore della richiesta di migrazione da effettuare inviando una response di tipo 401
--------------------------------------------------------------------------------
*/

extern "C" { char* strptime(const char* s, const char* format, struct tm* tm); }

OtpAuthToken::OtpAuthToken(CryptEngine* eng_, const UString& buf)
{
   U_TRACE_REGISTER_OBJECT(5, OtpAuthToken, "%p,%.*S", eng_,  U_STRING_TO_TRACE(buf))

   eng = eng_;

   U_STR_RESERVE(buffer, 1000);

   long pos;
   unsigned char ptr[1000];

#ifdef U_PROXY_UNIT
   Base64engine eng1;
   pos = eng1.decode((unsigned char*)buf.data(), (long)buf.size(), ptr);
   pos = eng->decrypt(ptr, pos, (unsigned char*)buffer.data());
#else
   pos = u_base64_decode(U_STRING_TO_PARAM(buf), ptr);
#  ifdef USE_LIBSSL
   pos = u_des3_decode(ptr, pos, (unsigned char*)buffer.data());
#  endif
#endif

   U_STR_SIZE_ADJUST_FORCE(buffer, pos);

   migrate   = false;
   valid     = (memcmp(buffer.data(), U_CONSTANT_TO_PARAM("TID=")) == 0);
   timestamp = 0;

   if (valid)
      {
      unsigned n = U_VEC_SPLIT(vec, buffer, "=; \r\n"), i = 10;

      if (n < i)
         {
         valid = false;

         return;
         }

      U_INTERNAL_ASSERT(vec[0] == U_STRING_FROM_CONSTANT("TID"))
      tid = vec[1];
      U_INTERNAL_ASSERT(vec[2] == U_STRING_FROM_CONSTANT("UID"))
      uid = vec[3];
      U_INTERNAL_ASSERT(vec[4] == U_STRING_FROM_CONSTANT("SID"))
      sid = vec[5];
      U_INTERNAL_ASSERT(vec[6] == U_STRING_FROM_CONSTANT("TS"))
      ts = vec[7];
      U_INTERNAL_ASSERT(vec[8] == U_STRING_FROM_CONSTANT("CF"))
      cf = vec[9];

      while (i < n)
         {
         if (vec[i] == U_STRING_FROM_CONSTANT("MIGRATE"))
            {
            migrate = true;

            ++i;
            }
         else
            {
            hp.push_back(vec[i]);

            ++i;

            if (i >= n)
               {
               valid = false;

               return;
               }

            hp.push_back(vec[i]);

            ++i;
            }
         }

#  ifdef U_STD_STRING
      char* _ptr = (char*) ts.c_str();
#  else
      char* _ptr = ts.data();
#  endif

      static struct tm tm;

      (void) strptime(_ptr, "%Y%m%d%H%M%S", &tm);

      timestamp = mktime(&tm);
      }
}

void OtpAuthToken::add(const UString& name_, const UString& value_)
{
   U_TRACE(5, "OtpAuthToken::add(%.*S,%.*S)", U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_))

   hp.push_back(name_);
   hp.push_back(value_);
}

bool OtpAuthToken::find(const UString& name_, UString& value_)
{
   U_TRACE(5, "OtpAuthToken::find(%.*S,%.*S)", U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_))

   unsigned i;

   for (i = 0; i < hp.size(); i += 2)
      {
      if (name_ == hp[i])
         {
         if ((i + 1) < hp.size())
            {
            value_ = hp[i+1];

            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

bool OtpAuthToken::del(const UString& name_)
{
   U_TRACE(5, "OtpAuthToken::del(%.*S)", U_STRING_TO_TRACE(name_))

   unsigned i;

   for (i = 0; i < hp.size(); i += 2)
      {
      if (name_ == hp[i])
         {
         // erase [first,last[

         unsigned first = i, last = i+2;

         U_VEC_ERASE2(hp, first, last);

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

void OtpAuthToken::stringify(UString& field)
{
   U_TRACE(5, "OtpAuthToken::stringify(%.*S)", U_STRING_TO_TRACE(field))

   int len;
   UString tmp1;

   if (ts.empty() == true)
      {
      U_STR_RESERVE(ts, 32);

      // timestamp nel formato AAAMMGGHHSS al momento del Set-Cookie

      (void) U_SYSCALL(strftime, "%S,%d,%S,%p", (char*)ts.data(), 32, (char*)"%Y%m%d%H%M%S", localtime(&timestamp));

      len = u__strlen(ts.data(), __PRETTY_FUNCTION__);

      U_STR_SIZE_ADJUST_FORCE(ts, len);
      }

   U_STR_RESERVE(tmp1, 1000);

   len = u__snprintf((char*)tmp1.data(), tmp1.capacity(), U_CONSTANT_TO_PARAM("TID=%.*s;UID=%.*s;SID=%.*s;TS=%.*s;CF=%.*s"),
                  tid.size(), tid.data(),
                  uid.size(), uid.data(),
                  sid.size(), sid.data(),
                  ts.size(),  ts.data(),
                  cf.size(),  cf.data());

   U_STR_SIZE_ADJUST_FORCE(tmp1, len);

   unsigned i = 0;

   while (i < hp.size())
      {
      tmp1.append(U_CONSTANT_TO_PARAM("; "));

      tmp1 += hp[i];
      tmp1.append(1, '=');
      tmp1 += hp[i+1];

      i += 2;
      }

   if (migrate)
      {
      tmp1.append(U_CONSTANT_TO_PARAM("; MIGRATE"));
      }

   U_STR_RESERVE(buffer, 1000);

   long pos;
   unsigned char ptr[1000];

#ifdef U_PROXY_UNIT
   Base64engine eng1;
   pos = eng->encrypt((unsigned char*)tmp1.data(), (long)tmp1.size(), ptr);
   pos = eng1.encode(ptr, pos, (unsigned char*)buffer.data());
#else
#  ifdef USE_LIBSSL
   pos = u_des3_encode((const unsigned char*)tmp1.data(), (long)tmp1.size(), ptr);
#  endif
   pos = u_base64_encode(ptr, pos, (unsigned char*)buffer.data());
#endif

   U_STR_SIZE_ADJUST_FORCE(buffer, pos);

   field += '"' + buffer + '"';

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* OtpAuthToken::dump(bool reset) const
{
   *UObjectIO::os << "eng                " << eng            << "\n"
                  << "valid              " << valid          << "\n"
                  << "migrate            " << migrate        << "\n"
                  << "timestamp          " << timestamp      << "\n"
                  << "ts        (UString " << (void*)&ts     << ")\n"
                  << "cf        (UString " << (void*)&cf     << ")\n"
                  << "tid       (UString " << (void*)&tid    << ")\n"
                  << "uid       (UString " << (void*)&uid    << ")\n"
                  << "sid       (UString " << (void*)&sid    << ")\n"
                  << "hp        (UVector " << (void*)&hp     << ")\n"
                  << "vec       (UVector " << (void*)&vec    << ")\n"
                  << "buffer    (UString " << (void*)&buffer << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
