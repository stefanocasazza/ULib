// ir_session.h

#ifndef IR_SESSION_H
#define IR_SESSION_H 1

#include <ulib/debug/crono.h>
#include <ulib/utility/data_session.h>

#include "cquery.h"

class IRDataSession : public UDataSession {
public:
   uint32_t sz, for_page;
   UVector<WeightWord*> vec;
   UString query, timerun, buffer_data;

   // COSTRUTTORE

   IRDataSession()
      {
      U_TRACE_CTOR(5, IRDataSession, "")

      sz = for_page = 0;
      }

   ~IRDataSession()
      {
      U_TRACE_DTOR(5, IRDataSession)
      }

   // SERVICES

   uint32_t size() { return sz; }

   // define method VIRTUAL of class UDataStorage

   virtual void clear()
      {
      U_TRACE(5, "IRDataSession::clear()")

      sz = for_page = 0;

              vec.clear();
            query.clear();
          timerun.clear();
      buffer_data.clear();
      }

   virtual char* toBuffer()
      {
      U_TRACE(5, "IRDataSession::toBuffer()")

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      (void) buffer_data.reserve(U_CAPACITY);

      buffer_data.snprintf(U_CONSTANT_TO_PARAM("%ld %u \"%.*s\" \"%.*s\" ["), creation, for_page, U_STRING_TO_TRACE(timerun), U_STRING_TO_TRACE(query));

      sz = vec.size();

      for (uint32_t i = 0; i < sz; ++i) vec[i]->toBuffer(buffer_data);

      (void) buffer_data.append(U_CONSTANT_TO_PARAM(" ]"));

      buffer_len = buffer_data.size();

      U_RETURN(buffer_data.data());
      }

   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "IRDataSession::fromStream(%p)", &is)

      U_ASSERT(vec.empty())
      U_INTERNAL_ASSERT_EQUALS(sz, 0)

#  ifdef U_STDCPP_ENABLE
      is >> creation
         >> for_page;

      is.get(); // skip ' '

      timerun.get(is);

      is.get(); // skip ' '

      query.get(is);

      is.get(); // skip ' '

      is >> vec;

      sz = vec.size();
#  endif

      last_access = u_now->tv_sec;
      }

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend istream& operator>>(istream& is, IRDataSession& d) { d.fromStream(is); return is; }
   friend ostream& operator<<(ostream& os, IRDataSession& d) {   d.toStream(os); return os; }
#endif

   // DEBUG

#ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
#endif
};

#endif
