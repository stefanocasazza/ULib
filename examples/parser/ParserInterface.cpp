// ParserInterface.cpp

#include <ParserInterface.h>

bool ParserInterface::parse()
{
   U_TRACE(5, "ParserInterface::parse()")

   /*
   extern int yydebug;
   yydebug = 1;
   */

   if (UBison::parse(this))
      {
      U_RETURN(true);
      }
   else
      {
      U_RETURN(false);
      }
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* ParserInterface::dump(bool reset) const
{
   UBison::dump(false);

   *UObjectIO::os << "\n"
                  << "pcdata  (UString  " << (void*)&pcdata << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
