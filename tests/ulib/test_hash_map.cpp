// test_hash_map.cpp

#include <ulib/file.h>
#include <ulib/container/hash_map.h>

#include <iostream>

extern "C" {
#include "file_config.gperf"
}

static bool setIndex(UHashMap<void*>* pthis)
{
   U_TRACE(5, "setIndex(%p)", pthis)

   UHashMap<void*>::index = (UHashMap<void*>::lhash = gperf_hash(U_STRING_TO_PARAM(*UHashMap<void*>::lkey))) & pthis->getMask();

   U_RETURN(false);
}

static void check0(UHashMap<UString>& y)
{
   U_TRACE_NO_PARAM(5, "check0()")

   U_ASSERT( y.invariant() )

   U_ASSERT( y["LOG_FILE"]                  == U_STRING_FROM_CONSTANT("ldap_update.log") )
   U_ASSERT( y["LDAP_SERVER_ADDRESS"]       == U_STRING_FROM_CONSTANT("10.10.15.1:389") )
   U_ASSERT( y["ROOT_DN"]                   == U_STRING_FROM_CONSTANT("o=BNL,c=IT") )
   U_ASSERT( y["ADMIN_DN"]                  == U_STRING_FROM_CONSTANT("cn=Manager,o=BNL,c=IT") )
   U_ASSERT( y["PASSWORD"]                  == U_STRING_FROM_CONSTANT("secret") )
   U_ASSERT( y["LDAP_SERVER_ADDRESS_MAIL"]  == U_STRING_FROM_CONSTANT("10.10.15.1:389") )
   U_ASSERT( y["ROOT_DN_MAIL"]              == U_STRING_FROM_CONSTANT("ou=Utenti,ou=e-family.it,o=BNL,c=IT") )
   U_ASSERT( y["ADMIN_DN_MAIL"]             == U_STRING_FROM_CONSTANT("cn=Manager,o=BNL,c=IT") )
   U_ASSERT( y["PASSWORD_MAIL"]             == U_STRING_FROM_CONSTANT("secret") )
   U_ASSERT( y["MAILHOST"]                  == U_STRING_FROM_CONSTANT("mailsrv.bf.bnl.it") )
   U_ASSERT( y["MAILDELIVERYOPTION"]        == U_STRING_FROM_CONSTANT("mailbox") )
   U_ASSERT( y["FILE_WRONG_MESSAGE"]        == U_STRING_FROM_CONSTANT("ldap_update.wrg") )
   U_ASSERT( y["MESSAGE_QUEUE_NAME"]        == U_STRING_FROM_CONSTANT("LDAP.UPDATE.QUEUE") )
   U_ASSERT( y["MESSAGE_QUEUE_SERVER"]      == U_STRING_FROM_CONSTANT("JAVA.CHANNEL/TCP/lobelia(1414)") )
   U_ASSERT( y["MESSAGE_QUEUE_MANAGER"]     == U_STRING_FROM_CONSTANT("frontend") )
   U_ASSERT( y["MAX_ERROR_FOR_CONNECT"]     == U_STRING_FROM_CONSTANT("2") )
   U_ASSERT( y["TIME_SLEEP_LDAP_ERROR"]     == U_STRING_FROM_CONSTANT("10") )
   U_ASSERT( y["TIME_SLEEP_MQSERIES_ERROR"] == U_STRING_FROM_CONSTANT("60") )
   U_ASSERT( y["CHECK_QUOTING"]             == U_STRING_FROM_CONSTANT("str = \"Manager of my caz\"...") )
}

static void check1(UHashMap<UString>& y)
{
   U_TRACE_NO_PARAM(5, "check1()")

   U_ASSERT( y.invariant() )

   UString value = y.erase(U_STRING_FROM_CONSTANT("LDAP_SERVER_ADDRESS"));

   U_ASSERT( y["LDAP_SERVER_ADDRESS"] == UString::getStringNull() )
   U_ASSERT( value == U_STRING_FROM_CONSTANT("10.10.15.1:389") )

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("ROOT_DN")) == U_STRING_FROM_CONSTANT("o=BNL,c=IT") )
   U_ASSERT(       y[U_STRING_FROM_CONSTANT("ROOT_DN")] == UString::getStringNull() )

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("ADMIN_DN")) == U_STRING_FROM_CONSTANT("cn=Manager,o=BNL,c=IT") )
   U_ASSERT(       y[U_STRING_FROM_CONSTANT("ADMIN_DN")] == UString::getStringNull() )

   y.clear();

   U_ASSERT( y.empty() )
   U_ASSERT( y[U_STRING_FROM_CONSTANT("TIME_SLEEP_MQSERIES_ERROR")] == UString::getStringNull() )
}

static void check2(UHashMap<UString>& y)
{
   U_TRACE_NO_PARAM(5, "check2()")

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("NOT_PRESENT")) == UString::getStringNull() )

   UString x = y["NOT_PRESENT"];

   U_ASSERT_EQUALS(x, UString::getStringNull())

   y.insertAfterFind(U_STRING_FROM_CONSTANT("60M"));

   x = y["NOT_PRESENT"];

   U_ASSERT_EQUALS(x, U_STRING_FROM_CONSTANT("60M"))

   U_ASSERT( y.erase(U_STRING_FROM_CONSTANT("NOT_PRESENT")) == U_STRING_FROM_CONSTANT("60M") )

   U_ASSERT_EQUALS(x, U_STRING_FROM_CONSTANT("60M"))

   U_ASSERT( x.strtoul(true)  == 60   * 1024   * 1024 )
   U_ASSERT( x.strtoull(true) == 60LL * 1024LL * 1024LL )
}

static void print(UStringRep* key, void* value)
{
   U_TRACE(5, "print(%V,%p)", key, value)

   cout << '\n';
   cout.write(key->data(), key->size());
   cout << " -> ";
   cout.write(((UStringRep*)value)->data(), ((UStringRep*)value)->size());
}

static bool cancella(UStringRep* key, void* value)
{
   U_TRACE(5, "cancella(%V,%p)", key, value)

   static int cnt;

   if (++cnt & 1) U_RETURN(true);

   U_RETURN(false);
}

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX14)
class MessageDelivery {
public:
   int64_t messageDateTime;

   MessageDelivery() { messageDateTime = 0; }
};

typedef UVector<MessageDelivery*> vmsg;
#endif

static void testHashMapIterator()
{
   U_TRACE_NO_PARAM(5, "testHashMapIterator()")

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX14)
   UVector<UString> badUsers;
   UHashMap<vmsg*> deliveries;
   int64_t twentyFourHoursAgoInMilliseconds = 1;

   auto badUserMapCleaner = [&] (UHashMapNode*& node) -> bool
      {
      for (UString badUser : badUsers)
         {
         if (badUser.rep->equal(node->key)) return true;
         }

      return false;
      };

   UHashMapAnonIter<vmsg> it = deliveries.begin();

   while (it != deliveries.end())
      {
      UHashMapNode* node = *it;

      // remove expired users

      if (badUsers.size() &&
          badUserMapCleaner(node))
         {
         it = deliveries.erase(it);

         continue;
         }

      ++it;

      // remove expired deliveries

      unsigned a = 0;
      vmsg* vdeliveries = (vmsg*)node->elem;

      while (a < vdeliveries->size())
         {
         if (vdeliveries->at(a)->messageDateTime < twentyFourHoursAgoInMilliseconds)
            {
            vdeliveries->erase(a);

            continue;
            }

         a++;
         }
      }
#endif
}

int U_EXPORT main(int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   UHashMap<UString> table(16, setIndex), z, x;

   table.loadFromData(UFile::contentOf(U_STRING_FROM_CONSTANT("file_config.cf")));

   U_ASSERT( table.size() == TOTAL_KEYWORDS+1 )

   uint32_t n = table.first();

   while (table.next()) ++n;

   U_ASSERT( n == TOTAL_KEYWORDS+1 )

   check0(table);
   check1(table);

   table.setIgnoreCase(false);

   cin  >> table;
   cout << table;

   z.assign(table);

   cout << "\n" << z;

   x.assign(z);

   cout << "\n---------------------------";
   z.callForAllEntrySorted(print);
   z.callWithDeleteForAllEntry(cancella);
   cout << "\n---------------------------";
   z.callForAllEntrySorted(print);
   cout << "\n---------------------------";

   z.clear();

   check1(table);
   check2(table);

   table.assign(x);

   x.clear();

   const char* dump = UObject2String<UHashMap<UString> >(table);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   U_INTERNAL_ASSERT_EQUALS(UObjectIO::buffer_output_len, 609)

   U_ASSERT( table.invariant() )
}
