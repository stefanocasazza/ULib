// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    flexer.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/flex/flexer.h>

// DEBUG

#ifdef DEBUG
void UFlexer::test()
{
   U_TRACE_NO_PARAM(0, "UFlexer::test()")

   YYSTYPE yyval;

   while (yylex(&yyval) != 0)
      {
      U_INTERNAL_DUMP("yyval = %.*S", yyval.ref.length, data.data() + yyval.ref.offset)
      }
}

#  if defined(U_STDCPP_ENABLE)
const char* UFlexer::dump(bool _reset) const
{
   *UObjectIO::os << "parsed_chars         " << parsed_chars   << '\n'
                  << "write_position       " << write_position << '\n'
                  << "data        (UString " << (void*)&data   << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
