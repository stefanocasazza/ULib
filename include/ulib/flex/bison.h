// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    bison.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_BISON_H
#define U_BISON_H

#include <ulib/flex/flexer.h>

/**
 * @class UBison
 *
 * Implementazione di Bison per ULib
 */

// extern int yydebug;
   extern int yyparse(void*);

class U_EXPORT UBison : public UFlexer {
public:

   // COSTRUTTORI

   UBison()
      {
      U_TRACE_REGISTER_OBJECT(0, UBison, "", 0)
      }

   UBison(const UString& data_) : UFlexer(data_)
      {
      U_TRACE_REGISTER_OBJECT(0, UBison, "%V", data_.rep)
      }

   ~UBison()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UBison)
      }

   // VARIE

   bool parse(void* obj = 0)
      {
      U_TRACE(0, "UBison::parse(%p)", obj)

      U_INTERNAL_ASSERT(data)

      // yydebug = 1;

      if (obj == 0) obj = this;

      bool ok = (yyparse(obj) == 0);

      U_INTERNAL_DUMP("yyparse() = %b, parsed_chars = %d, size() = %u", ok, parsed_chars, data.size())

      U_RETURN(ok);
      }

   bool parse(const UString& _data, void* obj = 0)
      {
      U_TRACE(0, "UBison::parse(%V,%p)", _data.rep, obj)

      setData(_data);

      bool ok = parse(obj);

      U_RETURN(ok);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UBison(const UBison&) = delete;
   UBison& operator=(const UBison&) = delete;
#else
   UBison(const UBison&) : UFlexer() {}
   UBison& operator=(const UBison&)  { return *this; }
#endif
};

#endif
