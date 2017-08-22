// test_ipaddress.cpp

#include <ulib/net/ipaddress.h>
#include <ulib/container/vector.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   u_init_ulib_hostname();

   bool result;
   UIPAddress x;
   char address[16];
   UString name = UString(argv[1]), domain, name_domain, name_ret;

   if (argv[2] &&
       argv[2][0])
      {
      domain      = argv[2];
      name_domain = name + '.' + domain;
      }

   U_MESSAGE("addr = %#08x %B mask = %#08x %B network = %#08x %B", 0x100007f, 0x100007f, 0xffffffff, 0xffffffff, 0x100007f, 0x100007f);

   result = x.setHostName(U_STRING_FROM_CONSTANT("pippo1"));

   U_ASSERT( result == false )

   // Test code for matching IP masks

   UIPAllow a;

   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("127.0.0.5")) == true )

   // 127.0.0.5

   x.setHostName(U_STRING_FROM_CONSTANT("127.0.0.5"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == true )

   x.setHostName(U_STRING_FROM_CONSTANT("127.0.0.0"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == false )

   x.setHostName(U_STRING_FROM_CONSTANT("127.0.0.2"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == false )

   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("127.0.0.1/8")) == true )

   // 127.0.0.1/8

   U_ASSERT( a.isAllowed(x.getInAddr()) == true )

   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("10.113.0.0/16")) == true )

   // 10.113.0.0/16
        
   x.setHostName(U_STRING_FROM_CONSTANT("10.113.45.67"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == true )

   x.setHostName(U_STRING_FROM_CONSTANT("10.11.45.67"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == false )

   x.setHostName(U_STRING_FROM_CONSTANT("127.0.0.1"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == false )

   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("1.2.3.4/0")) == true )

   // 1.2.3.4/0 
        
   x.setHostName(U_STRING_FROM_CONSTANT("4.3.2.1"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == true )

   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("1.2.3.4/40")) == false )
        
   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("1.2.3.4.5.6.7/8")) == false )

   U_ASSERT( a.parseMask(U_STRING_FROM_CONSTANT("1.2.3.4/8")) == true )

   // 1.2.3.4/8 
        
   x.setHostName(U_STRING_FROM_CONSTANT("4.3.2.1"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == false )

   result = a.parseMask(U_STRING_FROM_CONSTANT("192.168.1.64/28")); 

   U_INTERNAL_ASSERT( result )

   // 192.168.1.64/28

   x.setHostName(U_STRING_FROM_CONSTANT("192.168.1.70"));

   U_ASSERT( a.isAllowed(x.getInAddr()) == true )

   x.setHostName(U_STRING_FROM_CONSTANT("194.168.1.7"));

   result = a.isAllowed(x.getInAddr());

   U_INTERNAL_ASSERT( result == false )

   UVector<UIPAllow*> vipallow;

   uint32_t n = UIPAllow::parseMask(U_STRING_FROM_CONSTANT("172.0.0.0/8, 151.11.47.120"), vipallow);

   U_INTERNAL_ASSERT_EQUALS(n, 2)

   // 172.0.0.0/8

   x.setHostName(U_STRING_FROM_CONSTANT("172.31.17.195"));

   result = UIPAllow::isAllowed(x.getInAddr(), vipallow);

   U_INTERNAL_ASSERT( result )

   x.setHostName(U_STRING_FROM_CONSTANT("172.16.17.195"));

   result = UIPAllow::isAllowed(x.getInAddr(), vipallow);

   U_INTERNAL_ASSERT( result )

   x.setHostName(U_STRING_FROM_CONSTANT("172.16.100.47"));

   result = UIPAllow::isAllowed(x.getInAddr(), vipallow);

   U_INTERNAL_ASSERT( result )

   vipallow.clear();

   // 172.16.59.0/24 192.168.253.0/24

   n = UIPAllow::parseMask(U_STRING_FROM_CONSTANT("172.16.59.0/24 192.168.253.0/24"), vipallow);

   U_INTERNAL_ASSERT_EQUALS(n, 2)

   n = UIPAllow::find("192.168.253.253", vipallow);

   U_INTERNAL_ASSERT( n != U_NOT_FOUND )

   bool esito = x.setHostName(name);

   U_INTERNAL_ASSERT(esito)

   if (esito)
      {
      esito = x.setHostName(name);

      U_INTERNAL_ASSERT(esito)

      U_MEMCPY(address, x.get_in_addr(), x.getInAddrLength());

      x.setAddress(address);

      name_ret = x.getHostName();

      U_INTERNAL_DUMP("name_ret = %.*S name = %.*S name_domain = %.*S", U_STRING_TO_TRACE(name_ret), U_STRING_TO_TRACE(name), U_STRING_TO_TRACE(name_domain))

      U_ASSERT( name_ret.equalnocase(name) || name_ret.equalnocase(name_domain) )

      cout << name;
      }
}
