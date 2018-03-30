// test_vector.cpp

#define U_RING_BUFFER
#include <ulib/container/vector.h>
#include <ulib/file.h>
#include <ulib/base/hash.h>
#include <ulib/utility/dir_walk.h>
/*
#include <ulib/net/server/server.h>
*/

#ifdef __MINGW32__
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#undef min
using std::min;
#include <fstream>

class Product {
public:
    Product() {}
   ~Product() { cout << "\ndistruttore Product\n"; }
};

// how to override the default...

template <>
inline void u_destroy(const Product** ptr, uint32_t n)
{
   U_TRACE(0, "u_destroy<Product>(%p,%u)", ptr, n)
}

/**
 * NB: used by method
 *
 * void assign(unsigned n, T* elem))
 * void  erase(unsigned first, unsigned last)
 */

static void check_vector_destructor()
{
   U_TRACE(5, "check_vector_destructor()")

   UVector<Product*> z;

   z.push_back(new Product);
   z.push_back(new Product);
}

static void check(UVector<UString>& y)
{
   U_TRACE(5,"check()")

   unsigned n;
   UString tmp;

/**
 * input [
 * ROOT_DN                       0
 * PASSWORD                      1
 * ROOT_DN_MAIL                  2
 * PASSWORD_MAIL                 3
 * CHECK_QUOTING                 4
 * LDAP_SERVER_ADDRESS           5
 * LOG_FILE                      6
 * LDAP_SERVER_ADDRESS_MAIL      7
 * ADMIN_DN                      8
 * ADMIN_DN_MAIL                 9
 * TIME_SLEEP_LDAP_ERROR         10
 * TIME_SLEEP_MQSERIES_ERROR     11
 * FILE_WRONG_MESSAGE            12
 * MAILDELIVERYOPTION            13
 * MESSAGE_QUEUE_SERVER          14
 * MESSAGE_QUEUE_MANAGER         15
 * MAILHOST                      16
 * MESSAGE_QUEUE_NAME            17
 * MAX_ERROR_FOR_CONNECT         18
 * ]
 */

   tmp = y.front();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("ROOT_DN") )
   tmp = y.at(1);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("PASSWORD") )
   tmp = y.back();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   tmp = y[10];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   tmp = y[11];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )
   tmp = y[12];
   U_ASSERT( tmp != U_STRING_FROM_CONSTANT("ROOT_DN") )

   n = y.find(U_STRING_FROM_CONSTANT("NULL"));
   U_ASSERT( n == unsigned(-1) )
   n = y.find(U_STRING_FROM_CONSTANT("ROOT_DN"));
   U_ASSERT( n == unsigned(0) )
   n = y.find(U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT"));
   U_ASSERT( n == unsigned(18) )

   y.pop();
   tmp = y.back();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("MESSAGE_QUEUE_NAME") )

   y.push_back( U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT"));
   tmp = y.back();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   y.insert(10, U_STRING_FROM_CONSTANT("NOT_PRESENT"));
   tmp = y[10];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   tmp = y[11];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   tmp = y[12];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.insert(10, 2,  U_STRING_FROM_CONSTANT("NOT_PRESENT"));
   tmp = y[10];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   tmp = y[11];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   tmp = y[12];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   tmp = y[13];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   tmp = y[14];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.erase(10);
   tmp = y[10];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   tmp = y[11];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT") )
   tmp = y[12];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   tmp = y[13];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.erase(10, 12);
   tmp = y[10];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_LDAP_ERROR") )
   tmp = y[11];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR") )

   y.insert(19,    U_STRING_FROM_CONSTANT("NOT_PRESENT_1"));
   y.insert(20, 2, U_STRING_FROM_CONSTANT("NOT_PRESENT_2"));
   tmp = y[19];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT_1") )
   tmp = y[20];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT_2") )
   tmp = y[21];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NOT_PRESENT_2") )
   tmp = y[18];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   y.erase(19, 22);
   U_ASSERT( y.back() == U_STRING_FROM_CONSTANT("MAX_ERROR_FOR_CONNECT") )

   y.assign(19, U_STRING_FROM_CONSTANT("NULL"));
   tmp = y.front();
   U_ASSERT( tmp ==U_STRING_FROM_CONSTANT("NULL") )
   tmp = y.back();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NULL") )
   tmp = y[17];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NULL") )
   tmp = y[18];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("NULL") )

   y.reserve(y.capacity() * 2);
   y.clear();
   U_ASSERT( y.empty() == true )
}

static void print(UVector<UString>& y)
{
   U_TRACE(5, "print()")

   UString buffer(3000U);

   char* ptr      = buffer.data();
   unsigned size  = buffer.capacity();

   ostrstream os(ptr, size);

   os << y;

   unsigned output_len = os.pcount();

   U_INTERNAL_DUMP("output_len = %d", output_len)

   U_INTERNAL_ASSERT_MINOR(output_len,size)

   buffer.size_adjust(output_len);

   cout << buffer << endl;
}

static int compareObj(const void* obj1, const void* obj2)
{
   U_TRACE(0, "::compareObj(%p,%p)", obj1, obj2)

#ifdef U_STDCPP_ENABLE
   return (((UStringRep*)obj1)->compare((const UStringRep*)obj2) < 0);
#else
   return (*(UStringRep**)obj1)->compare(*(const UStringRep**)obj2);
#endif
}

static void check_contains()
{
   U_TRACE(5,"check_contains()")

   UString a0 = U_STRING_FROM_CONSTANT("accettazione, non-accettazione, avvenuta-consegna, rilevazione-virus"),
           b0 = U_STRING_FROM_CONSTANT("Consegna Virus");

   UVector<UString> a(a0, ", "), b(b0);

   U_ASSERT( a.contains(b)       == false )
   U_ASSERT( a.contains(b, true) == true )

   U_ASSERT( b.isContained(U_STRING_FROM_CONSTANT("virus"), true) == true )
}

static void check_equal()
{
   U_TRACE(5,"check_equal()")

   UString a0 = U_STRING_FROM_CONSTANT("accettazione, non-accettazione, avvenuta-consegna, rilevazione-virus"),
           b0 = U_STRING_FROM_CONSTANT("Avvenuta-Consegna rilevazione-Virus accettazione non-accettazione");

   UVector<UString> a(a0, ", "), b(b0);

   U_ASSERT( a.isEqual(b)       == false )
   U_ASSERT( a.isEqual(b, true) == true )

   U_ASSERT( b.find(U_STRING_FROM_CONSTANT("rilevazione-virus"), true) == true )
}

/**
static void check_bound(uint32_t last_event_id)
{
   U_TRACE(5,"check_bound(%u)", last_event_id)

   UVector<UString> vec;
   UVector<UString> vmessage;
   uint32_t i, n, pos, start = 0, end = 0;
   UString tmp, message(100U), output(U_CAPACITY);

   for (i = 0, n = vmessage.capacity() * 1.5; i < n; ++i)
      {
      message.snprintf(U_CONSTANT_TO_PARAM("%u"), i);

      tmp = "*="+message;

      vmessage.insertWithBound(tmp, start, end);
      }

   vmessage.getFromLast(last_event_id, start, end, vec);

   for (i = 0, n = vec.size(); i < n; ++i)
      {
      message = vec[i];

      pos = message.find('=');

      U_INTERNAL_DUMP("vmessage[%u] = %V pos = %u", i % vmessage.capacity(), message.rep, pos)

      (void) output.append(UServer_Base::printSSE(i+1, message.substr(pos+1), U_NULLPTR));
      }

   cout << vmessage << "\n" << output;

( *=64 *=65 *=66 *=67 *=68 *=69 *=70 *=71 *=72 *=73 *=74 *=75 *=76 *=77 *=78 *=79 *=80 *=81 *=82 *=83 *=84 *=85 *=86 *=87 *=88 *=89 *=90 *=91 *=92 *=93 *=94 *=95 *=32 *=33 *=34 *=35 *=36 *=37 *=38 *=39 *=40 *=41 *=42 *=43 *=44 *=45 *=46 *=47 *=48 *=49 *=50 *=51 *=52 *=53 *=54 *=55 *=56 *=57 *=58 *=59 *=60 *=61 *=62 *=63 )

id:81
data:80

id:82
data:81

id:83
data:82

id:84
data:83

id:85
data:84

id:86
data:85

id:87
data:86

id:88
data:87

id:89
data:88

id:90
data:89

id:91
data:90

id:92
data:91

id:93
data:92

id:94
data:93

id:95
data:94

id:96
data:95
}
*/

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   /*
   check_bound(80);
   exit(0);
   */

   UString tmp;

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11) && defined(U_COMPILER_RANGE_FOR)
   UVector<UString> v { U_STRING_FROM_CONSTANT(" block1 "), U_STRING_FROM_CONSTANT(" block2 "), U_STRING_FROM_CONSTANT(" block3 "), U_STRING_FROM_CONSTANT(" block4 ") };

   for (UString x : v) tmp += x;

   cerr << "Range-For: " << tmp << '\n';

   U_ASSERT( tmp == U_STRING_FROM_CONSTANT(" block1  block2  block3  block4 ") )
#endif

   UVector<UString> v0(U_STRING_FROM_CONSTANT("[\"\" 0 @PIPPO]"));
   uint32_t n = v0.size();
   U_INTERNAL_ASSERT( n == 3 )
   tmp = v0[0];
   U_ASSERT( tmp.isNull() )
   tmp = v0[1];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("0") )
   tmp = v0[2];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("@PIPPO") )

   UString content = UFile::contentOf(U_STRING_FROM_CONSTANT("inp/wifi-utilizzo"));
   UVector<UString> vec1(content, '\n'), vec2(4);

   n = vec1.size();
   U_INTERNAL_ASSERT( n == 2 )

   n = vec2.split(vec1[0], ',');
   vec1.clear();
   U_INTERNAL_ASSERT( n == 4 )
   tmp = vec2[0];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"2013/09/11\"") )

   tmp = U_STRING_FROM_CONSTANT("museoGalileoLungarno-r29587_locoM2 10.8.0.110\n"
                                "orSmichele-r29587_picoM2 10.8.0.161\n"
                                "paasBracci-r29587_picoM2 10.8.0.50\n"
                                "paasBronzino-r29587_picoM2 10.8.0.29\n");

   vec2.clear();
   n = vec2.split(tmp);
   U_INTERNAL_ASSERT( n == 8 )

   n = vec2.findSorted(U_STRING_FROM_CONSTANT("museoGalileoLungarno-r29587_locoM2"), true, true);
   U_INTERNAL_ASSERT( n == 0 )
   n = vec2.findSorted(U_STRING_FROM_CONSTANT("orSmichele-r29587_picoM2"), false, true);
   U_INTERNAL_ASSERT( n == 2 )
   n = vec2.findSorted(U_STRING_FROM_CONSTANT("paasBracci-r29587_picoM2"), true, true);
   U_INTERNAL_ASSERT( n == 4 )
   n = vec2.findSorted(U_STRING_FROM_CONSTANT("paasBronzino-r29587_picoM2"), false, true);
   U_INTERNAL_ASSERT( n == 6 )
   n = vec2.findSorted(U_STRING_FROM_CONSTANT("paasBronzino"), true, true);
   U_INTERNAL_ASSERT( n == U_NOT_FOUND )

   vec2.clear();

   tmp = UFile::contentOf(UString(argv[1]));
   UVector<UString> y(tmp);
   y.sort();

   uint32_t i = y.findSorted(U_STRING_FROM_CONSTANT("NULL"));
   U_INTERNAL_ASSERT( i == U_NOT_FOUND )

   for (i = 0, n = y.size(); i < n; ++i) { U_ASSERT( i == y.findSorted(y[i]) ) }

   ofstream outf("vector.sort");

   outf << y;

   y.clear();

   check_vector_destructor();
   check_contains();
   check_equal();

   cin >> y;

   print(y);
   check(y);

   // EXTENSION

   {
   UDirWalk dirwalk(U_NULLPTR, U_CONSTANT_TO_PARAM("?db.*test*"));

   n = dirwalk.walk(y);

   U_INTERNAL_ASSERT( n == 3 )
   }

   y.sort();

   U_DUMP("y[0] = %.*S", U_STRING_TO_TRACE(y[0]))
   U_DUMP("y[1] = %.*S", U_STRING_TO_TRACE(y[1]))
   U_DUMP("y[2] = %.*S", U_STRING_TO_TRACE(y[1]))

   U_ASSERT( y[0] == U_STRING_FROM_CONSTANT("cdb.test") )
   U_ASSERT( y[1] == U_STRING_FROM_CONSTANT("rdb.test") )
   U_ASSERT( y[2] == U_STRING_FROM_CONSTANT("tdb.test") )

   y.clear();
   bool res = y.empty();
   U_INTERNAL_ASSERT(res)

   y.clear();

   UString yA = U_STRING_FROM_CONSTANT("\n\n# comment line\n\nriga_0\nriga_1\n\n");

   n = y.split(yA);
   U_INTERNAL_ASSERT( n == 2 )
   tmp = y[0];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga_0") )
   tmp = y[1];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga_1") )

   UString y0 = U_STRING_FROM_CONSTANT("word \"word with space\"");

   n = y.split(y0);
   U_INTERNAL_ASSERT( n == 2 )
   tmp = y[2];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("word") )
   tmp = y[3];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("word with space") )

   UString x = U_STRING_FROM_CONSTANT("  word \"word with space\"    ");

   n = y.split(x);
   U_INTERNAL_ASSERT( n == 2 )
   tmp = y[4];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("word") )
   tmp = y[5];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("word with space") )

   tmp = y.join(0, U_CONSTANT_TO_PARAM("//"));
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga_0//riga_1//word//word with space//word//word with space") )

   y.clear();
   res = y.empty();
   U_INTERNAL_ASSERT( res )

   UString y2 = U_STRING_FROM_CONSTANT("$Version=\"1\";\n Part_Number=\"Riding_Rocket_0023\"; $Path=\"/acme/ammo\";\n Part_Number=\"Rocket_Launcher_0001\"; $Path=\"/acme\"");

   n = y.split(y2, "=;, \n");
   U_INTERNAL_ASSERT( n == 10 )

   tmp = y[0];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("$Version") )
   tmp = y[1];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"1\"") )
   tmp = y[2];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("Part_Number") )
   tmp = y[3];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"Riding_Rocket_0023\"") )
   tmp = y[4];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("$Path") )
   tmp = y[5];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"/acme/ammo\"") )
   tmp = y[6];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("Part_Number") )
   tmp = y[7];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"Rocket_Launcher_0001\"") )
   tmp = y[8];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("$Path") )
   tmp = y[9];
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"/acme\"") )

   y.swap(7, 3);
   tmp = y[3];
   U_INTERNAL_DUMP("tmp = %.*S", U_STRING_TO_TRACE(tmp))
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"Rocket_Launcher_0001\"") )
   tmp = y[7];
   U_INTERNAL_DUMP("tmp = %.*S", U_STRING_TO_TRACE(tmp))
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("\"Riding_Rocket_0023\"") )

   y.UVector<void*>::sort(compareObj);

   cout << y;

   y.clear();
   res = y.empty();
   U_INTERNAL_ASSERT( res )

   // BINARY HEAP

   y.bh_put(U_STRING_FROM_CONSTANT("riga 01"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 02"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 03"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 04"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 05"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 06"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 07"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 08"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 09"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 10"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   y.bh_put(U_STRING_FROM_CONSTANT("riga 11"));
   tmp = y.bh_min();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )

   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 01") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 02") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 03") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 04") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 05") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 06") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 07") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 08") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 09") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 10") )
   tmp = y.bh_get();
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 11") )

   res = y.empty();
   U_INTERNAL_ASSERT( res )

   // RING BUFFER

#if defined(U_RING_BUFFER) && !defined(U_STATIC_ONLY)
   UVector<UString> y1(5);

   tmp = U_STRING_FROM_CONSTANT("riga 1");
   y1.put(tmp);
   tmp = U_STRING_FROM_CONSTANT("riga 2");
   y1.put(tmp);
   tmp = U_STRING_FROM_CONSTANT("riga 3");
   y1.put(tmp);
   tmp = U_STRING_FROM_CONSTANT("riga 4");
   y1.put(tmp);

   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 1") )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 2") )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 3") )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 4") )
   U_ASSERT( y1.get(tmp) == false )

   U_ASSERT( y1.put(U_STRING_FROM_CONSTANT("riga 1")) )
   U_ASSERT( y1.put(U_STRING_FROM_CONSTANT("riga 2")) )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 1") )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 2") )

   U_ASSERT( y1.put(U_STRING_FROM_CONSTANT("riga 3")) )
   U_ASSERT( y1.put(U_STRING_FROM_CONSTANT("riga 4")) )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 3") )
   y1.get(tmp);
   U_ASSERT( tmp == U_STRING_FROM_CONSTANT("riga 4") )
   U_ASSERT( y1.get(tmp) == false )
#endif
}
