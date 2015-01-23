// OtpAuthToken.h

#ifndef OTP_AUTH_TOKEN_H
#define OTP_AUTH_TOKEN_H 1

#include <HttpField.h>

#ifdef U_PROXY_UNIT
#  include <CryptEngine.h>
#else
class CryptEngine;
#endif

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

class OtpAuthToken {
public:
   //// Check for memory error
   U_MEMORY_TEST

   /// = Allocator e Deallocator.
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   OtpAuthToken()
      {
      U_TRACE_REGISTER_OBJECT(5, OtpAuthToken, "")
      }

   /**
   * @param eng_ Crypto engine to decrypt and encrypt data
   * @param buf  string buffer containing OTP authentication token
   */
   OtpAuthToken(CryptEngine* eng_, const UString& buf);

    /**
      * @param eng_        Crypto engine to encrypt and decrypt data
      * @param srvid       trustACCESS identifier for session
      * @param uid_        identificatore dell'utente loggato
      * @param sid_        User identifier for session
      * @param cf_         Codice Fiscale utente reperito dai dati di profilo
      * @param timestamp_  time to insert in token
      * @param migrate_    Tell next server to migrate
      */
   OtpAuthToken(CryptEngine* eng_, const UString& srvid, const UString& uid_, const UString& sid_,
                                   const UString& cf_,   time_t timestamp_, bool migrate_ = false)
         : tid(srvid), uid(uid_), sid(sid_), cf(cf_)
      {
      U_TRACE_REGISTER_OBJECT(5, OtpAuthToken, "%p,%.*S,%.*S,%.*S,%.*S,%p,%b", eng_, U_STRING_TO_TRACE(srvid),
               U_STRING_TO_TRACE(uid_), U_STRING_TO_TRACE(sid_), U_STRING_TO_TRACE(cf_), timestamp_, migrate_)

      valid     = true;
      eng       = eng_;
      migrate   = migrate_;
      timestamp = timestamp_;
      }

   /** Destructor of the class.
   */
   ~OtpAuthToken()
      {
      U_TRACE_UNREGISTER_OBJECT(0, OtpAuthToken)
      }

   /**
   * @return if token is valid
   */
   bool is_valid()
      {
      U_TRACE(5, "OtpAuthToken::is_valid()")

      U_RETURN(valid);
      }

   /**
   * @return converts ts string to calendar time representation
   */
   time_t get_timestamp()
      {
      U_TRACE(5, "OtpAuthToken::get_timestamp()")

      U_RETURN(timestamp);
      }

   /**
   * @param name_   name of field in Profile_Header
   * @param value_ Value of field in Profile_Header
   */
   void add(const UString& name_, const UString& value_);

   /**
   * @param name_  Field name in Profile_Header
   * @param value_ Value of Profile_Header
   * return: true if succeded
   */
   bool find(const UString& name_, UString& value_);

   /**
   * @param name_  Name of Profile_Header
   * return: true if succeded
   */
   bool del(const UString& name_);

   /**
   * @param field String where to save token as string
   */
   void stringify(UString& field);

   /// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer;
   time_t timestamp;
public:
   CryptEngine* eng;
   UString tid, uid, sid, ts, cf;
   UVector<UString> vec, hp;
   bool valid, migrate;
};

#endif
