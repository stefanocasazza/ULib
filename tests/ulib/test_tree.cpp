// test_tree.cpp

#include <ulib/file.h>
#include <ulib/container/tree.h>
#include <ulib/utility/dir_walk.h>

#ifdef __MINGW32__
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#undef min
using std::min;
#include <fstream>
#include <iostream>

static void print(void* elem, void* pnode)
{
   cout << *(UStringRep*)elem << " <-> " << ((UTree<UString>*)pnode)->depth() << endl;
}

static void check(UTree<UString>& y)
{
   U_TRACE(5,"check()")

   const char* input = "[ ROOT [ ROOT_DN ][ PASSWORD ][ ROOT_DN_MAIL ][ PASSWORD_MAIL ][ CHECK_QUOTING ][ LDAP_SERVER_ADDRESS ][ LOG_FILE ][ LDAP_SERVER_ADDRESS_MAIL ][ ADMIN_DN ][ ADMIN_DN_MAIL ][ TIME_SLEEP_LDAP_ERROR ][ TIME_SLEEP_MQSERIES_ERROR ][ FILE_WRONG_MESSAGE ][ MAILDELIVERYOPTION ][ MESSAGE_QUEUE_SERVER  ][ MESSAGE_QUEUE_MANAGER ][ MAILHOST ][ MESSAGE_QUEUE_NAME ][ MAX_ERROR_FOR_CONNECT ]]";

   istrstream is(input, strlen(input));

   is >> y;

   U_ASSERT( y.front() == U_STRING_FROM_CONSTANT("ROOT_DN") )
   U_ASSERT( y[1]      == U_STRING_FROM_CONSTANT("PASSWORD") )
   U_ASSERT( y.back()  == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   U_ASSERT( y[10] == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   U_ASSERT( y[11] == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )
   U_ASSERT( y[12] != U_STRING_FROM_CONSTANT("ROOT_DN") )

   U_ASSERT( y.find(U_STRING_FROM_CONSTANT("NULL"))                  == U_NOT_FOUND )
   U_ASSERT( y.find(U_STRING_FROM_CONSTANT("ROOT_DN"))               == 0U )
   U_ASSERT( y.find(U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT")) == 18U )

   y.pop();
   U_ASSERT( y.back() == U_STRING_FROM_CONSTANT("MESSAGE_QUEUE_NAME") )

   y.push(U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT"));
   U_ASSERT( y.back() == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   y.insert(10, U_STRING_FROM_CONSTANT("NOT_PRESENT"));
   U_ASSERT( y[10] == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   U_ASSERT( y[11] == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   U_ASSERT( y[12] == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.insert(10, U_STRING_FROM_CONSTANT("NOT_PRESENT"));
   y.insert(10, U_STRING_FROM_CONSTANT("NOT_PRESENT"));
   U_ASSERT( y[10] == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   U_ASSERT( y[11] == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   U_ASSERT( y[12] == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   U_ASSERT( y[13] == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   U_ASSERT( y[14] == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.erase(10);
   U_ASSERT( y[10] == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   U_ASSERT( y[11] == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   U_ASSERT( y[12] == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   U_ASSERT( y[13] == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.erase(10);
   y.erase(10);
   U_ASSERT( y[10] == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   U_ASSERT( y[11] == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.insert(19, U_STRING_FROM_CONSTANT("NOT_PRESENT_1"));
   y.insert(20, U_STRING_FROM_CONSTANT("NOT_PRESENT_2"));
   y.insert(20, U_STRING_FROM_CONSTANT("NOT_PRESENT_2"));
   U_ASSERT( y[18] == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )
   U_ASSERT( y[19] == U_STRING_FROM_CONSTANT("NOT_PRESENT_1") )
   U_ASSERT( y[20] == U_STRING_FROM_CONSTANT("NOT_PRESENT_2") )
   U_ASSERT( y[21] == U_STRING_FROM_CONSTANT("NOT_PRESENT_2") )

   y.erase(19);
   y.erase(19);
   y.erase(19);
   U_ASSERT( y.back() == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   y.reserve(y.capacity() * 2);
   y.clear();
   U_ASSERT( y.empty() == true )
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UTree<UString> y;

   cin  >> y;
   cout << y << endl;

   y.callForAllEntry(print);

   y.clear();

   U_ASSERT( y.empty() == true )

   check(y);

   U_ASSERT( y.depth() == 0 )

   UTree<UString>* p;
   y.setRoot(U_STRING_FROM_CONSTANT("ROOT"));
   p = y.push(U_STRING_FROM_CONSTANT("NODO_1.1"));
   U_ASSERT( p->depth() == 1 )
   p = y.push(U_STRING_FROM_CONSTANT("NODO_1.2"));
   U_ASSERT( p->depth() == 1 )
   p = y.push(U_STRING_FROM_CONSTANT("NODO_1.3"));
   U_ASSERT( p->depth() == 1 )

   y.clear();

   U_ASSERT( y.empty() == true )

   UDirWalk dirwalk(0, U_CONSTANT_TO_PARAM("?db.*test*"));

   uint32_t n = dirwalk.walk(y);

   U_ASSERT( n == 3 )

   U_DUMP("y[0] = %.*S", U_STRING_TO_TRACE(y[0]))
   U_DUMP("y[1] = %.*S", U_STRING_TO_TRACE(y[1]))
   U_DUMP("y[2] = %.*S", U_STRING_TO_TRACE(y[1]))

   U_ASSERT( y[0] == U_STRING_FROM_CONSTANT("./cdb.test") ||
             y[0] == U_STRING_FROM_CONSTANT("./rdb.test") ||
             y[0] == U_STRING_FROM_CONSTANT("./tdb.test"))
   U_ASSERT( y[1] == U_STRING_FROM_CONSTANT("./rdb.test") ||
             y[1] == U_STRING_FROM_CONSTANT("./cdb.test") ||
             y[1] == U_STRING_FROM_CONSTANT("./tdb.test"))
   U_ASSERT( y[2] == U_STRING_FROM_CONSTANT("./rdb.test") ||
             y[2] == U_STRING_FROM_CONSTANT("./cdb.test") ||
             y[2] == U_STRING_FROM_CONSTANT("./tdb.test"))
}
