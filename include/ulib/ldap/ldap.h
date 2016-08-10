// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ldap.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_LDAP_H
#define ULIB_LDAP_H 1

#include <ulib/string.h>

#if defined(_MSWINDOWS_) && defined(HAVE_WINLDAP_H)
#  undef  CRL_REASON_UNSPECIFIED             // 0
#  undef  CRL_REASON_KEY_COMPROMISE          // 1
#  undef  CRL_REASON_CA_COMPROMISE           // 2
#  undef  CRL_REASON_AFFILIATION_CHANGED     // 3
#  undef  CRL_REASON_SUPERSEDED              // 4
#  undef  CRL_REASON_CESSATION_OF_OPERATION  // 5
#  undef  CRL_REASON_CERTIFICATE_HOLD        // 6
#  undef  CRL_REASON_REMOVE_FROM_CRL         // 8

#  include <winldap.h>
#  define LDAPS_PORT   636 /* ldaps:///  default LDAP over TLS port */
#  define LDAP_TIMEVAL struct l_timeval
#  define ldapssl_init ldap_sslinit
typedef struct {
   const char* lud_host;
   int         lud_port;
   const char* lud_dn;
   char**      lud_attrs;
   int         lud_scope;
   char*       lud_filter;
   char**      lud_exts;
} LDAPURLDesc;
#else
#  define LDAP_DEPRECATED 1
#  define LDAP_TIMEVAL struct timeval
#  include <ldap.h>
#  ifdef HAVE_LDAP_SSL_H
#     include <ldap_ssl.h>
#  endif
#endif

class U_EXPORT ULDAPEntry {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   char** dn;
   UString** attr_val;
   int n_entry, n_attr;
   const char** attr_name;

    ULDAPEntry(int num_names, const char** names, int num_entry = 1);
   ~ULDAPEntry();

   char* operator[](int pos) const
      {
      U_TRACE(0, "ULDAPEntry::operator[](%d)", pos)

      U_INTERNAL_ASSERT_MINOR(pos, n_entry)

      return dn[pos];
      }

   const char* getCStr(  int index_names, int index_entry = 0);
   UString     getString(int index_names, int index_entry = 0);

   void set(char* attribute, char** values,              int index_entry = 0);
   void set(char* attribute, char*  value, uint32_t len, int index_entry = 0);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
   ULDAPEntry(const ULDAPEntry&)            {}
   ULDAPEntry& operator=(const ULDAPEntry&) { return *this; }
};

#ifndef   LDAP_FILTER_ALL
#  define LDAP_FILTER_ALL "(objectClass=*)"
#endif

/**
 * types for ldap URL handling
 * ----------------------------------
 *  typedef struct ldap_url_desc {
 *  char*    lud_scheme;
 *  char*    lud_host;
 *  int      lud_port;
 *  char*    lud_dn;
 *  char**   lud_attrs;
 *  int      lud_scope;
 *  char*    lud_filter;
 *  char**   lud_exts;
 *  int      lud_crit_exts;
 *  } LDAPURLDesc;
 */

class U_EXPORT ULDAP {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   ULDAP()
      {
      U_TRACE_REGISTER_OBJECT(0, ULDAP, "", 0)

      ld           = 0;
      ludpp        = 0;
      result       = 52;
      pTimeOut     = &timeOut;
      isSecure     = false;
      searchResult = 0;
      }

   ~ULDAP()
      {
      U_TRACE_UNREGISTER_OBJECT(0, ULDAP)

      clear();
      }

   void clear();
   void setStatus();

   bool set_protocol(int protocol = LDAP_VERSION3)
      {
      U_TRACE(1, "ULDAP::set_protocol(%d)", protocol)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_set_option, "%p,%d,%d", ld, LDAP_OPT_PROTOCOL_VERSION, &protocol);

      U_RETURN(result == LDAP_OPT_SUCCESS);
      }

   bool simple_bind(const char* who = "", const char* passwd = "")
      {
      U_TRACE(1, "ULDAP::simple_bind(%S,%S)", who, passwd)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_simple_bind_s, "%p,%S,%S", ld, (char*)who, (char*)passwd);

      U_RETURN(result == LDAP_SUCCESS);
      }

   bool bind(const char* who = "", const char* cred = "", int authmethod = LDAP_AUTH_SIMPLE)
      {
      U_TRACE(1, "ULDAP::bind(%S,%S,%d)", who, cred, authmethod)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_bind_s, "%p,%S,%S,%d", ld, (char*)who, (char*)cred, authmethod);

      U_RETURN(result == LDAP_SUCCESS);
      }

   // ---------------------------------------------------------------
   // LDAP url operations
   // ---------------------------------------------------------------
   // ldap://ou=Sales,o=Acme?sn?one
   // ldaps://Acme.com:636/o=Acme?objectclass?one\n
   // ldap://Acme.com:389/ou=Sales,o=Acme?sn,mail?sub?(objectclass=*)
   // ---------------------------------------------------------------

   bool init(const char* url);

   bool init(const char* host, int port) // LDAP_PORT(389)
      {
      U_TRACE(1, "ULDAP::init(%S,%d)", host, port)

      U_INTERNAL_ASSERT_EQUALS(ld,0)

      ld = (LDAP*) U_SYSCALL(ldap_init, "%S,%d", (char*)host, port);

      if (ld == 0)
         {
         result = 52;

         U_RETURN(false);
         }

      U_RETURN(true);
      }

   // LDAP manual operations

   bool open(const char* host = "localhost", int port = 389)
      {
      U_TRACE(1, "ULDAP::open(%S,%d)", host, port)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(ld,0)

      ld = (LDAP*) U_SYSCALL(ldap_open, "%S,%d", (char*)host, port);

      U_RETURN(ld != 0);
      }

   bool add(const char* dn, LDAPMod* ldapmods[])
      {
      U_TRACE(1, "ULDAP::add(%S,%p)", dn, ldapmods)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_add_s, "%p,%S,%p", ld, (char*)dn, ldapmods);

      U_RETURN(result == LDAP_SUCCESS);
      }

   bool modify(const char* dn, LDAPMod* ldapmods[])
      {
      U_TRACE(1, "ULDAP::modify(%S,%p)", dn, ldapmods)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_modify_s, "%p,%S,%p", ld, (char*)dn, ldapmods);

      U_RETURN(result == LDAP_SUCCESS);
      }

   bool remove(const char* dn)
      {
      U_TRACE(1, "ULDAP::remove(%S)", dn)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_delete_s, "%p,%S", ld, (char*)dn);

      U_RETURN(result == LDAP_SUCCESS);
      }

   void setTimeOut(struct timeval* timeout)
      {
      U_TRACE(0, "ULDAP::setTimeOut(%p)", timeout)

#if !defined(LDAP_OPT_NETWORK_TIMEOUT) && defined(LDAP_OPT_TIMELIMIT)
#  define LDAP_OPT_NETWORK_TIMEOUT LDAP_OPT_TIMELIMIT
#endif

      (void) U_SYSCALL(ldap_set_option, "%p,%d,%d", 0, LDAP_OPT_NETWORK_TIMEOUT, (LDAP_TIMEVAL*)(pTimeOut = timeout));
      }

   // ---------------------
   // search scopes
   // ---------------------
   // LDAP_SCOPE_BASE     0
   // LDAP_SCOPE_ONELEVEL 1
   // LDAP_SCOPE_SUBTREE  2
   // ---------------------

   int search(const char* dn, int scope = LDAP_SCOPE_BASE, char* attrs[] = 0, const char* filter = LDAP_FILTER_ALL)
      {
      U_TRACE(1, "ULDAP::search(%S,%d,%p,%S)", dn, scope, attrs, filter)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      result = U_SYSCALL(ldap_search_st, "%p,%S,%d,%S,%p,%d,%p,%p", ld, (char*)dn, scope, (char*)filter, attrs, 0, (LDAP_TIMEVAL*)pTimeOut, &searchResult);

      int num = (result == LDAP_SUCCESS ? U_SYSCALL(ldap_count_entries, "%p,%p", ld, searchResult) : -1);

      U_RETURN(num);
      }

   int search()
      {
      U_TRACE_NO_PARAM(1, "ULDAP::search()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)
      U_INTERNAL_ASSERT_POINTER(ludpp)

      return search(ludpp->lud_dn, ludpp->lud_scope, ludpp->lud_attrs, ludpp->lud_filter);
      }

#if defined(HAVE_LDAP_SSL_H) && !defined(_MSWINDOWS_) && !defined(HAVE_WINLDAP_H)
   bool search(const char* url)
      {
      U_TRACE(1, "ULDAP::search(%S)", url)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ld)

      /* Search the directory */

      result = U_SYSCALL(ldap_url_search_st, "%p,%S,%d,%p,%p",
                           ld,             /* LDAP session handle */
                           url,            /* LDAP URL to use in the search operation */
                           0,              /* return attributes and values */
                           pTimeOut,       /* search timeout */
                           &searchResult); /* returned results */

      U_RETURN(result == LDAP_SUCCESS);
      }
#endif

#if !defined(_MSWINDOWS_) || !defined(HAVE_WINLDAP_H)
   void sort(const char* sortAttribute = "sn")
      {
      U_TRACE(0, "ULDAP::sort(%S)", sortAttribute)

      U_INTERNAL_ASSERT_POINTER(ld)
      U_INTERNAL_ASSERT_POINTER(searchResult)

      ldap_sort_entries(ld, &searchResult, (char*)sortAttribute, strcmp); /* client-sort */
      }
#endif

   /**
    * Example (from NGSS):
    * ---------------------------------------------------------------------------------------------------------
    * #define HOST_STRING     "host"
    * #define RULE_STRING     "rule"
    * #define VERSION_STRING  "version"
    * #define MIN_STRING      "min"
    * #define AGE_STRING      "age"
    *
    * static const char* attr_name[] = { HOST_STRING, RULE_STRING, VERSION_STRING, MIN_STRING, AGE_STRING, 0 };
    *
    * LDAPEntry entry(attr_name, 5};
    * ---------------------------------------------------------------------------------------------------------
    */

   const char** getAttrs() const
      {
      U_TRACE_NO_PARAM(0, "ULDAP::getAttrs()")

      U_INTERNAL_ASSERT_POINTER(ludpp)

      return (const char**)ludpp->lud_attrs;
      }

   void get(ULDAPEntry& e);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   LDAP* ld;
   LDAPURLDesc* ludpp;
   struct timeval* pTimeOut;
   LDAPMessage* searchResult;
   int result;
   bool isSecure;

   static struct timeval timeOut;

private:
#if defined(_MSWINDOWS_) && defined(HAVE_WINLDAP_H)
   static char** split_str(char* str) U_NO_EXPORT;
#endif

   U_DISALLOW_COPY_AND_ASSIGN(ULDAP)
};

#endif
