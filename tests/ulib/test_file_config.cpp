// test_file_config.cpp

#include <ulib/json/value.h>
#include <ulib/file_config.h>
#include <ulib/debug/crono.h>

#include <iostream>

extern "C" {
#  include "file_config.gperf"
}

static void setIndex(UHashMap<void*>* pthis, const UStringRep* keyr)
{
   U_TRACE(5, "setIndex(%p,%V)", pthis, keyr)

   pthis->index = gperf_hash(U_STRING_TO_PARAM(*keyr));
}

static void check(UFileConfig& y)
{
   U_TRACE(5,"check()")

   U_ASSERT( y[U_STRING_FROM_CONSTANT("LOG_FILE")]                  == U_STRING_FROM_CONSTANT("ldap_update.log") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("LDAP_SERVER_ADDRESS")]       == U_STRING_FROM_CONSTANT("10.10.15.1:389") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("ROOT_DN")]                   == U_STRING_FROM_CONSTANT("o=BNL,c=IT") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("ADMIN_DN")]                  == U_STRING_FROM_CONSTANT("cn=Manager,o=BNL,c=IT") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("PASSWORD")]                  == U_STRING_FROM_CONSTANT("secret") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("LDAP_SERVER_ADDRESS_MAIL")]  == U_STRING_FROM_CONSTANT("10.10.15.1:389") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("ROOT_DN_MAIL")]              == U_STRING_FROM_CONSTANT("ou=Utenti,ou=e-family.it,o=BNL,c=IT") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("ADMIN_DN_MAIL")]             == U_STRING_FROM_CONSTANT("cn=Manager,o=BNL,c=IT") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("PASSWORD_MAIL")]             == U_STRING_FROM_CONSTANT("secret") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("MAILHOST")]                  == U_STRING_FROM_CONSTANT("mailsrv.bf.bnl.it") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("MAILDELIVERYOPTION")]        == U_STRING_FROM_CONSTANT("mailbox") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("FILE_WRONG_MESSAGE")]        == U_STRING_FROM_CONSTANT("ldap_update.wrg") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("MESSAGE_QUEUE_NAME")]        == U_STRING_FROM_CONSTANT("LDAP.UPDATE.QUEUE") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("MESSAGE_QUEUE_SERVER")]      == U_STRING_FROM_CONSTANT("JAVA.CHANNEL/TCP/lobelia(1414)") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("MESSAGE_QUEUE_MANAGER")]     == U_STRING_FROM_CONSTANT("frontend") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT")]     == U_STRING_FROM_CONSTANT("2") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR")]     == U_STRING_FROM_CONSTANT("10") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR")] == U_STRING_FROM_CONSTANT("60") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("CHECK_QUOTING")]             == U_STRING_FROM_CONSTANT("str = \"Manager of my caz\"...") )
}

static void check1(UFileConfig& y)
{
   U_TRACE(5,"check1()")

// y.table.reserve(y.table.capacity() * 2);

   UString value = y.erase(U_STRING_FROM_CONSTANT("LDAP_SERVER_ADDRESS"));

   U_ASSERT( value                                            == U_STRING_FROM_CONSTANT("10.10.15.1:389") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("LDAP_SERVER_ADDRESS")] == U_STRING_FROM_CONSTANT("") )

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("ROOT_DN"))  == U_STRING_FROM_CONSTANT("o=BNL,c=IT") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("ROOT_DN")]        == U_STRING_FROM_CONSTANT("") )

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("ADMIN_DN")) == U_STRING_FROM_CONSTANT("cn=Manager,o=BNL,c=IT") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("ADMIN_DN")]       == U_STRING_FROM_CONSTANT("") )

   y.table.clear();

   U_ASSERT( y.empty() == true )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR")] == U_STRING_FROM_CONSTANT("") )
}

static void check2(UFileConfig& y)
{
   U_TRACE(5,"check2()")

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("NOT_PRESENT")) == U_STRING_FROM_CONSTANT("") )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("NOT_PRESENT")]       == U_STRING_FROM_CONSTANT("") )

   y.table.insertAfterFind(U_STRING_FROM_CONSTANT("NOT_PRESENT"), U_STRING_FROM_CONSTANT("60M"));

   U_ASSERT( y[U_STRING_FROM_CONSTANT("NOT_PRESENT")]  == U_STRING_FROM_CONSTANT("60M") )

   UString value = y[U_STRING_FROM_CONSTANT("NOT_PRESENT")];

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("NOT_PRESENT")) == U_STRING_FROM_CONSTANT("60M") )

   U_ASSERT( value           == U_STRING_FROM_CONSTANT("60M") )
   U_ASSERT( value.strtol()  == 60   * 1024   * 1024 )
#ifdef HAVE_STRTOLL
   U_ASSERT( value.strtoll() == 60LL * 1024LL * 1024LL )
#endif
#ifdef HAVE_STRTOF
   U_ASSERT( value.strtof()  == 60.0 )
#endif
   U_ASSERT( value.strtod()  == 60.0 )
#ifdef HAVE_STRTOLD
   U_ASSERT( value.strtold() == 60.0 )
#endif
}

static bool print(UStringRep* key, void* value)
{
   U_TRACE(5, "print(%.*S,%p)", U_STRING_TO_TRACE(*key), value)

   cout << '\n';
   cout.write(key->data(), key->size());
   cout << " -> ";
   cout.write(((UStringRep*)value)->data(), ((UStringRep*)value)->size());

   U_RETURN(true);
}

static bool cancella(UStringRep* key, void* value)
{
   U_TRACE(5, "cancella(%.*S,%p)", U_STRING_TO_TRACE(*key), value)

   static int cnt;

   if (++cnt & 1) U_RETURN(true);

   U_RETURN(false);
}

int U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UFileConfig y;

   y.table.set_index = setIndex;

   y.table.allocate(MAX_HASH_VALUE+1);

   y.load(U_STRING_FROM_CONSTANT("file_config.cf"));

   U_ASSERT( y.table.size() == TOTAL_KEYWORDS )

   int n = y.table.first();

   while (y.table.next()) ++n;

   U_ASSERT( n == TOTAL_KEYWORDS )

   check(y);
   check1(y);

   y.table.set_index = UHashMap<void*>::setIndex;

   cin  >> y.table;
   cout << y.table;

   UHashMap<UString> z, x;

   z.assign(y.table);

   cout << "\n" << z;

   x.assign(z);

   cout << "\n---------------------------";
   z.callForAllEntrySorted(print);
   z.callWithDeleteForAllEntry(cancella);
   cout << "\n---------------------------";
   z.callForAllEntrySorted(print);
   cout << "\n---------------------------";

   z.clear();

   check1(y);
   check2(y);

   y.table.assign(x);

   x.clear();

   UString tmp = U_STRING_FROM_CONSTANT("{ \"key1\" : \"riga 1\", \"key2\" : \"riga 2\", \"key3\" : \"riga 3\", \"key4\" : \"riga 4\" }");

   bool result = JSON_parse(tmp, x);

   U_INTERNAL_ASSERT( result )

   tmp    = x["key1"];
   result = ( tmp == U_STRING_FROM_CONSTANT("riga 1") );

   U_INTERNAL_ASSERT( result )

   tmp    = x["key2"];
   result = ( tmp == U_STRING_FROM_CONSTANT("riga 2") );

   U_INTERNAL_ASSERT( result )

   tmp    = x["key3"];
   result = ( tmp == U_STRING_FROM_CONSTANT("riga 3") );

   U_INTERNAL_ASSERT( result )

   tmp    = x["key4"];
   result = ( tmp == U_STRING_FROM_CONSTANT("riga 4") );

   U_INTERNAL_ASSERT( result )

   UValue json(OBJECT_VALUE);
   tmp = JSON_stringify(json, x);

   U_ASSERT( tmp.size() == U_CONSTANT_SIZE("{\"key4\":\"riga 4\",\"key3\":\"riga 3\",\"key1\":\"riga 1\",\"key2\":\"riga 2\"}") )

   x.clear();

   // Time Consumed with num_iteration(10) = 543 ms

   n = (argc > 1 ? atoi(argv[1]) : 5);

   UCrono crono;

   crono.start();
   for (int i = 0; i < n; ++i) check(y);
   crono.stop();

   check1(y);

   if (argc > 1) printf("\n# Time Consumed with num_iteration(%d) = %ld ms\n", n, crono.getTimeElapsed());
}
