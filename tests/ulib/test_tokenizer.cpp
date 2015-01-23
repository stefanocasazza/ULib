// test_tokenizer.cpp

#include <ulib/file.h>
#include <ulib/tokenizer.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   UString dati, y, z = U_STRING_FROM_CONSTANT("mnt mirror home stefano spool cross");

   UTokenizer t(z);

   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mnt") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mirror") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("home") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("stefano") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("spool") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("cross") )
   U_ASSERT( t.next(y,(bool*)0) == false )

   t.setData(z);
   t.setDelimiter(" \t\n");

   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mnt") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("mirror") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("home") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("stefano") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("spool") )
   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("cross") )
   U_ASSERT( t.next(y,(bool*)0) == false )

   UString z1 = U_STRING_FROM_CONSTANT("\"spool cross\"");

   t.setData(z1);
   t.setDelimiter(0);

   U_ASSERT( t.next(y,(bool*)0) == true )
   U_ASSERT( y         == U_STRING_FROM_CONSTANT("spool cross") )
   U_ASSERT( t.next(y,(bool*)0) == false )

   UString z2 = U_STRING_FROM_CONSTANT("(pippo OR pluto) AND NOT (paperino AND paperone)");

   t.setData(z2);
   t.setDelimiter(0);
   t.setGroup(U_CONSTANT_TO_PARAM("()"));

   bool bgroup = false;

   U_ASSERT( t.next(y,&bgroup)  == true )
   U_ASSERT( bgroup             == true )
   U_ASSERT( y                  == U_STRING_FROM_CONSTANT("pippo OR pluto") )
   U_ASSERT( t.next(y,&bgroup)  == true )
   U_ASSERT( y                  == U_STRING_FROM_CONSTANT("AND") )
   U_ASSERT( bgroup             == false )
   U_ASSERT( t.next(y,&bgroup)  == true )
   U_ASSERT( y                  == U_STRING_FROM_CONSTANT("NOT") )
   U_ASSERT( bgroup             == false )
   U_ASSERT( t.next(y,&bgroup)  == true )
   U_ASSERT( y                  == U_STRING_FROM_CONSTANT("paperino AND paperone") )
   U_ASSERT( bgroup             == true )
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

      dati = UFile::contentOf(argv[1]);

      t.setData(dati);
      t.setGroup(U_CONSTANT_TO_PARAM("<%%>"));

      while (t.next(y,(bool*)0))
         {
         cout << "------------------------------------------\n";
         cout << y;
         cout << "------------------------------------------\n";

         uint32_t distance = t.getDistance(),
                  pos      = dati.find("<%", distance);

         if (pos == U_NOT_FOUND) pos = dati.size();

         cout << "------------------------------------------\n";
         cout << dati.substr(distance, pos - distance);
         cout << "------------------------------------------\n";

         t.setDistance(pos);
         }
      }

   y.clear();

   t.setData(U_STRING_FROM_CONSTANT(" ( $QUERY_STRING  =  'submitted' ) "));

   while (t.getTokenId(z) > 0);

   U_ASSERT( t.getTokenId(z) == 0 )

   t.setData(U_STRING_FROM_CONSTANT(" ( ${QUERY_STRING}  !=  submitted ) "));

   while (t.getTokenId(z) > 0);

   U_ASSERT( t.getTokenId(z) == 0 )

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
