// test_ldap.cpp

#include <ulib/ldap/ldap.h>

static const char* attr_name[] = { "cn", "mail", 0 };

#define NUM_ATTR 2

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   ULDAP ldap;

   if (ldap.init(argv[1]) &&
       ldap.set_protocol() &&
       ldap.bind(argv[2],argv[3]))
      {
      int i, n = ldap.search(argv[4], LDAP_SCOPE_ONELEVEL, (char**)attr_name);

      if (n > 0)
         {
         ULDAPEntry entry(NUM_ATTR, attr_name, n);

         ldap.get(entry);

         for (i = 0; i < n; ++i)
            {
            cout << "dn:   " << entry[i]            << "\n"
                 << "cn:   " << entry.getCStr(0, i) << "\n"
                 << "mail: " << entry.getCStr(1, i) << "\n\n";
            }
         }
      }
}
