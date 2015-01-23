// ParserInterface.h

#ifndef PARSER_INTERFACE_H
#define PARSER_INTERFACE_H

#include <ulib/flex/bison.h>

class ParserInterface : public UBison {
public:

   // COSTRUTTORI

   ParserInterface()
      {
      U_TRACE_REGISTER_OBJECT(5, ParserInterface, "")
      }

   ParserInterface(const UString& data) : UBison(data)
      {
      U_TRACE_REGISTER_OBJECT(5, ParserInterface, "%p", data.data())
      }

   ~ParserInterface()
      {
      U_TRACE_UNREGISTER_OBJECT(5, ParserInterface)
      }

   // VARIE

   bool isMultipart()         { return false; }
   bool isBoundary(char*&)    { return false; }
   bool isEndBoundary(char*&) { return false; }

   bool parse();

   bool parse(int offset, int length)
      {
      U_TRACE(5, "ParserInterface::parse(%d,%d)", offset, length)

      ParserInterface parser(substr(offset, length));

      if (parser.parse()) U_RETURN(true);

      U_RETURN(false);
      }

   virtual int yylex(void* yyval);

   // DEBUG

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   UString pcdata;
};

#endif
