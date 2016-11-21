// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    data_session.h - data session utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DATA_SESSION_H
#define ULIB_DATA_SESSION_H 1

#include <ulib/container/vector.h>

template <class T> class URDBObjectHandler;

class U_EXPORT UDataStorage {
public:

   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UDataStorage()
      {
      U_TRACE_REGISTER_OBJECT(0, UDataStorage, "", 0)
      }

   UDataStorage(const UString& key) : keyid(key)
      {
      U_TRACE_REGISTER_OBJECT(0, UDataStorage, "%V", key.rep)
      }

   virtual ~UDataStorage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UDataStorage)
      }

   // method VIRTUAL to define

   virtual void clear() {}

   // SERVICES

   void setKeyId()
      {
      U_TRACE_NO_PARAM(0, "UDataStorage::setKeyId()")

      U_INTERNAL_DUMP("keyid = %V", keyid.rep)

      keyid = *UString::str_storage_keyid;
      }

   bool isDataSession()
      {
      U_TRACE_NO_PARAM(0, "UDataStorage::isDataSession()")

      U_INTERNAL_DUMP("keyid = %V", keyid.rep)

      if (keyid) U_RETURN(true);

      U_RETURN(false);
      }

   void resetDataSession()
      {
      U_TRACE_NO_PARAM(0, "UDataStorage::resetDataSession()")

      U_INTERNAL_DUMP("keyid = %V", keyid.rep)

      if (keyid.isNull() == false) keyid.clear();
      }

   void setKeyIdDataSession(const UString& key)
      {
      U_TRACE(0, "UDataStorage::setKeyIdDataSession(%V)", key.rep)

      U_INTERNAL_DUMP("keyid = %V", keyid.rep)

      U_INTERNAL_ASSERT(key)

      keyid = key;
      }

   void setKeyIdDataSession(const char* s, uint32_t n)
      {
      U_TRACE(0, "UDataStorage::setKeyIdDataSession(%.*S,%u)", n, s, n)

      U_INTERNAL_DUMP("keyid = %V", keyid.rep)

      (void) keyid.assign(s, n);
      }

#ifdef U_STDCPP_ENABLE
   virtual void   toStream(ostream& os) {}
   virtual void fromStream(istream& is) {}

   friend istream& operator>>(istream& is, UDataStorage& d) { d.fromStream(is); return is; }
   friend ostream& operator<<(ostream& os, UDataStorage& d) {   d.toStream(os); return os; }

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UString keyid;

   static uint32_t buffer_len;

   // method VIRTUAL to define

   virtual char* toBuffer();
   virtual void  fromData(const char* ptr, uint32_t len);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UDataStorage)

   template <class T> friend class URDBObjectHandler;
};

class U_EXPORT UDataSession : public UDataStorage {
public:

   UDataSession()
      {
      U_TRACE_REGISTER_OBJECT(0, UDataSession, "", 0)

      init();
      }

   UDataSession(const UString& key) : UDataStorage(key)
      {
      U_TRACE_REGISTER_OBJECT(0, UDataSession, "%V", key.rep)

      init();
      }

   virtual ~UDataSession()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UDataSession)

      delete vec_var;
      }

   // SERVICES

   bool isNewSession()
      {
      U_TRACE_NO_PARAM(0, "UDataSession::isNewSession()")

      if (last_access == creation) U_RETURN(true);

      U_RETURN(false);
      }

   bool isDataSessionExpired()
      {
      U_TRACE_NO_PARAM(0, "UDataSession::isDataSessionExpired()")

      U_INTERNAL_DUMP("keyid = %V", keyid.rep)

      bool result = ((last_access - creation) > U_ONE_DAY_IN_SECOND);

      U_RETURN(result);
      }

   UString getSessionCreationTime()
      {
      U_TRACE_NO_PARAM(0, "UDataSession::getSessionCreationTime()")

      UString x(40U);

      x.snprintf(U_CONSTANT_TO_PARAM("%#5D"), creation);

      U_RETURN_STRING(x);
      }

   UString getSessionLastAccessedTime()
      {
      U_TRACE_NO_PARAM(0, "UDataSession::getSessionLastAccessedTime()")

      UString x(40U);

      x.snprintf(U_CONSTANT_TO_PARAM("%#5D"), last_access);

      U_RETURN_STRING(x);
      }

   void getValueVar(uint32_t index, UString& value)
      {
      U_TRACE(0, "UDataSession::getValueVar(%u,%p)", index, &value)

      if (index < vec_var->size()) value = vec_var->at(index);
      else                         value.clear();

      U_INTERNAL_DUMP("value = %V", value.rep)
      }

   void putValueVar(uint32_t index, const UString& value)
      {
      U_TRACE(0, "UDataSession::putValueVar(%u,%V)", index, value.rep)

      if (index < vec_var->size()) vec_var->replace(index, value);
      else
         {
         U_INTERNAL_ASSERT_EQUALS(index, vec_var->size())

         vec_var->push_back(value);
         }
      }

   UString setKeyIdDataSession(uint32_t counter);
   UString setKeyIdDataSession(uint32_t counter, const UString& data);

   // define method VIRTUAL of class UDataStorage

   virtual void clear() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0, "UDataSession::clear()")

      vec_var->clear();
      }

   // STREAM

#ifdef U_STDCPP_ENABLE
   virtual void   toStream(ostream& os);
   virtual void fromStream(istream& is);

   friend istream& operator>>(istream& is, UDataSession& d) { d.fromStream(is); return is; }
   friend ostream& operator<<(ostream& os, UDataSession& d) {   d.toStream(os); return os; }

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UVector<UString>* vec_var;
   long creation, last_access;

   void init()
      {
      U_TRACE_NO_PARAM(0, "UDataSession::init()")

      U_NEW(UVector<UString>, vec_var, UVector<UString>);

      creation = last_access = u_now->tv_sec;
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UDataSession)

                      friend class UHTTP;
   template <class T> friend class URDBObjectHandler;
};

#endif
