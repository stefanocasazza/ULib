// test_tokenizer.cpp

#include <ulib/file.h>
#include <ulib/tokenizer.h>

static void checkRealNumber(UTokenizer& t)
{
   U_TRACE(5, "::checkRealNumber(%p)", &t)

   t.skipSpaces();

   const char* start = t.getPointer();

   (void) t.next();

   int type_num = t.getTypeNumber();

   if (type_num != 0)
      {
      if (type_num < 0)
         {
         double real_ = (type_num == INT_MIN // scientific notation (Ex: 1.45e10)
                           ?   strtod(start, 0)
                           : u_strtod(start, t.getPointer(), type_num));

         U_INTERNAL_DUMP("real_ = %g", real_)

         U_INTERNAL_ASSERT_EQUALS(real_, strtod(start, 0))
         }
      }
}

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   UTokenizer t(U_STRING_FROM_CONSTANT("-73.99548457138242"));

   checkRealNumber(t);

   UString dati, y, z = U_STRING_FROM_CONSTANT("mnt mirror home stefano spool cross");

   t.setData(z);

   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mnt") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mirror") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("home") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("stefano") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("spool") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("cross") )
   U_ASSERT( t.next(y,(bool*)0) == false )

   t.setData(z);
   t.setDelimiter(" \t\n");

   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mnt") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mirror") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("home") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("stefano") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("spool") )
   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("cross") )
   U_ASSERT( t.next(y,(bool*)0) == false )

   UString z1 = U_STRING_FROM_CONSTANT("\"spool cross\"");

   t.setData(z1);
   t.setDelimiter(0);

   U_ASSERT( t.next(y,(bool*)0) )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("spool cross") )
   U_ASSERT( t.next(y,(bool*)0) == false )

   UString z2 = U_STRING_FROM_CONSTANT("(pippo OR pluto) AND NOT (paperino AND paperone)");

   t.setData(z2);
   t.setDelimiter(0);
   t.setGroup(U_CONSTANT_TO_PARAM("()"));

   bool bgroup = false;

   U_ASSERT( t.next(y,&bgroup) )
   U_ASSERT( bgroup )
   U_ASSERT( y == U_STRING_FROM_CONSTANT("pippo OR pluto") )
   U_ASSERT( t.next(y,&bgroup) )
   U_ASSERT( y == U_STRING_FROM_CONSTANT("AND") )
   U_ASSERT( bgroup == false )
   U_ASSERT( t.next(y,&bgroup) )
   U_ASSERT( y == U_STRING_FROM_CONSTANT("NOT") )
   U_ASSERT( bgroup == false )
   U_ASSERT( t.next(y,&bgroup) )
   U_ASSERT( y == U_STRING_FROM_CONSTANT("paperino AND paperone") )
   U_ASSERT( bgroup )
   U_ASSERT( t.next(y,(bool*)0) == false )

   t.setGroup(0);
   t.setDelimiter(0);
   t.setSkipTagXML(true);

   UString filename;

   while (cin >> filename)
      {
      dati = UFile::contentOf(filename);

      t.setData(dati);

      while (t.next(y,(bool*)0)) cout << y << "\n";
      }

   if (argv[1])
      {
      y.clear();

      dati = UFile::contentOf(UString(argv[1]));

      t.setData(dati);
      t.setGroup(U_CONSTANT_TO_PARAM("<%%>"));

      while (t.next(y,(bool*)0))
         {
         cout << "------------------------------------------\n";
         cout << y;
         cout << "------------------------------------------\n";

         uint32_t distance = t.getDistance(),
                  pos      = U_STRING_FIND(dati, distance, "<%");

         if (pos == U_NOT_FOUND) pos = dati.size();

         cout << "------------------------------------------\n";
         cout << dati.substr(distance, pos - distance);
         cout << "------------------------------------------\n";

         t.setDistance(pos);
         }
      }

   y.clear();

   t.setData(U_STRING_FROM_CONSTANT(" ( $QUERY_STRING  =  'submitted' ) "));

   while (t.getTokenId(0) > 0);

   U_ASSERT( t.getTokenId(0) == 0 )

   t.setData(U_STRING_FROM_CONSTANT(" ( ${QUERY_STRING}  !=  submitted ) "));

   while (t.getTokenId(0) > 0);

   U_ASSERT( t.getTokenId(0) == 0 )

   t.setData(U_STRING_FROM_CONSTANT("!!!.,;'?pippo.,;'?!!!"));

   t.setAvoidPunctuation(true);

   bool result = t.next(y,(bool*)0);

   U_ASSERT( result )

   result = (y == U_STRING_FROM_CONSTANT("pippo"));

   U_ASSERT( result )

   result = t.next(y,(bool*)0);

   U_ASSERT( result == false )

   y.clear();
}
