// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ldap.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/ldap/ldap.h>

struct timeval ULDAP::timeOut = { 30L, 0L }; // 30 second connection/search timeout

ULDAPEntry::ULDAPEntry(int num_names, const char** names, int num_entry)
{
   U_TRACE_REGISTER_OBJECT(0, ULDAPEntry, "%d,%p,%d", num_names, names, num_entry)

   U_INTERNAL_ASSERT_EQUALS(names[num_names], 0)

   U_DUMP_ATTRS(names)

   n_attr    = num_names;
   n_entry   = num_entry;
   attr_name = names;

   dn       =    (char**) UMemoryPool::_malloc(num_entry,             sizeof(char*),    true);
   attr_val = (UString**) UMemoryPool::_malloc(num_entry * num_names, sizeof(UString*), true);
}

ULDAPEntry::~ULDAPEntry()
{
   U_TRACE_UNREGISTER_OBJECT(0, ULDAPEntry)

   for (int i = 0, j, k = 0; i < n_entry; ++i)
      {
      if (dn[i])
         {
         U_INTERNAL_DUMP("dn[%d]: %S", i, dn[i])

         ldap_memfree(dn[i]);

         for (j = 0; j < n_attr; ++j, ++k)
            {
            if (attr_val[k])
               {
               U_INTERNAL_DUMP("ULDAPEntry(%d): %S = %V", k, attr_name[j], attr_val[k]->rep)

               delete attr_val[k];
               }
            }
         }
      }

   UMemoryPool::_free(dn,       n_entry,          sizeof(char*));
   UMemoryPool::_free(attr_val, n_entry * n_attr, sizeof(UString*));
}

void ULDAPEntry::set(char* attribute, char** values, int index_entry)
{
   U_TRACE(0, "ULDAPEntry::set(%S,%p,%d)", attribute, values, index_entry)

   U_DUMP_ATTRS(values)

   U_INTERNAL_ASSERT_MINOR(index_entry, n_entry)

   for (int j = 0, k = index_entry * n_attr; j < n_attr; ++j, ++k)
      {
      if (strcmp(attr_name[j], attribute) == 0)
         {
         U_INTERNAL_DUMP("ULDAPEntry(%d): %S", k, attr_name[j])

         U_NEW(UString, attr_val[k], UString((void*)values[0], u__strlen(values[0], __PRETTY_FUNCTION__)));

         for (j = 1; values[j]; ++j)
            {
            attr_val[k]->append(U_CONSTANT_TO_PARAM("; "));
            attr_val[k]->append(values[j]);
            }

         U_INTERNAL_DUMP("value = %V", attr_val[k]->rep)

         break;
         }
      }
}

void ULDAPEntry::set(char* attribute, char* value, uint32_t len, int index_entry)
{
   U_TRACE(0, "ULDAPEntry::set(%S,%.*S,%u,%d)", attribute, len, value, len, index_entry)

   U_INTERNAL_ASSERT_MINOR(index_entry, n_entry)

   for (int j = 0, k = index_entry * n_attr; j < n_attr; ++j, ++k)
      {
      if (strcmp(attr_name[j], attribute) == 0)
         {
         U_INTERNAL_DUMP("ULDAPEntry(%d): %S", k, attr_name[j])

         U_NEW(UString, attr_val[k], UString((void*)value, len));

         U_INTERNAL_DUMP("value = %V", attr_val[k]->rep)

         break;
         }
      }
}

UString ULDAPEntry::getString(int index_names, int index_entry)
{
   U_TRACE(0, "ULDAPEntry::getString(%d,%d)", index_names, index_entry)

   U_INTERNAL_ASSERT_MINOR(index_names, n_attr)
   U_INTERNAL_ASSERT_MINOR(index_entry, n_entry)

   int k = (index_entry * n_attr) + index_names;

   if (attr_val[k])
      {
      UString str = *(attr_val[k]);

      U_RETURN_STRING(str);
      }

   return UString::getStringNull();
}

const char* ULDAPEntry::getCStr(int index_names, int index_entry)
{
   U_TRACE(0, "ULDAPEntry::getCStr(%d,%d)", index_names, index_entry)

   U_INTERNAL_ASSERT_MINOR(index_names, n_attr)
   U_INTERNAL_ASSERT_MINOR(index_entry, n_entry)

   int k = (index_entry * n_attr) + index_names;

   if (attr_val[k])
      {
      const char* str = attr_val[k]->c_str();

      U_RETURN(str);
      }

   U_RETURN("");
}

void ULDAP::clear()
{
   U_TRACE_NO_PARAM(1, "ULDAP::clear()")

   if (ludpp)
      {
#  if !defined(_MSWINDOWS_) && !defined(HAVE_WINLDAP_H)
      U_SYSCALL_VOID(ldap_free_urldesc, "%p", ludpp);
#  else
      int i;

      if (ludpp->lud_dn)     U_SYSCALL_VOID(free, "%p", (void*)ludpp->lud_dn);
      if (ludpp->lud_host)   U_SYSCALL_VOID(free, "%p", (void*)ludpp->lud_host);
      if (ludpp->lud_filter) U_SYSCALL_VOID(free, "%p",        ludpp->lud_filter);

      if (ludpp->lud_attrs)
         {
         for (i = 0; ludpp->lud_attrs[i]; ++i) U_SYSCALL_VOID(free, "%p", ludpp->lud_attrs[i]);

         U_SYSCALL_VOID(free, "%p", ludpp->lud_attrs);
         }

      if (ludpp->lud_exts)
         {
         for (i = 0; ludpp->lud_exts[i]; ++i) U_SYSCALL_VOID(free, "%p", ludpp->lud_exts[i]);

         U_SYSCALL_VOID(free, "%p", ludpp->lud_exts);
         }

      U_SYSCALL_VOID(free, "%p", ludpp);
#  endif

      ludpp = 0;
      }

   if (ld)
      {
      if (searchResult)
         {
         U_SYSCALL(ldap_msgfree, "%p", searchResult);
                                       searchResult = 0;
         }

      U_SYSCALL(ldap_unbind_s, "%p", ld);
                                     ld = 0;

#  if defined(HAVE_LDAP_SSL_H) && defined(HAS_NOVELL_LDAPSDK)
      if (isSecure) U_SYSCALL_NO_PARAM(ldapssl_client_deinit);
#  endif
      }
}

void ULDAP::setStatus()
{
   U_TRACE_NO_PARAM(0, "ULDAP::setStatus()")

   U_CHECK_MEMORY

   static const char* errlist[] = {
      "LDAP_SUCCESS",                        //  0 0x00
      "LDAP_OPERATIONS_ERROR",               //  1 0x01
      "LDAP_PROTOCOL_ERROR",                 //  2 0x02
      "LDAP_TIMELIMIT_EXCEEDED",             //  3 0x03
      "LDAP_SIZELIMIT_EXCEEDED",             //  4 0x04
      "LDAP_COMPARE_FALSE",                  //  5 0x05
      "LDAP_COMPARE_TRUE",                   //  6 0x06
      "LDAP_STRONG_AUTH_NOT_SUPPORTED",      //  7 0x07
      "LDAP_STRONG_AUTH_REQUIRED",           //  8 0x08
      "LDAP_PARTIAL_RESULTS",                //  9 0x09
      "LDAP_REFERRAL",                       // 10
      "LDAP_ADMINLIMIT_EXCEEDED",            // 11
      "LDAP_UNAVAILABLE_CRITICAL_EXTENSION", // 12
      "LDAP_CONFIDENTIALITY_REQUIRED",       // 13
      "LDAP_SASL_BIND_IN_PROGRESS",          // 14
      "",                                    // 15
      "LDAP_NO_SUCH_ATTRIBUTE",              // 16 0x10
      "LDAP_UNDEFINED_TYPE",                 // 17 0x11
      "LDAP_INAPPROPRIATE_MATCHING",         // 18 0x12
      "LDAP_CONSTRAINT_VIOLATION",           // 19 0x13
      "LDAP_TYPE_OR_VALUE_EXISTS",           // 20 0x14
      "LDAP_INVALID_SYNTAX",                 // 21 0x15
      "",                                    // 22
      "",                                    // 23
      "",                                    // 24
      "",                                    // 25
      "",                                    // 26
      "",                                    // 27
      "",                                    // 28
      "",                                    // 29
      "",                                    // 30
      "",                                    // 31
      "LDAP_NO_SUCH_OBJECT",                 // 32 0x20
      "LDAP_ALIAS_PROBLEM",                  // 33 0x21
      "LDAP_INVALID_DN_SYNTAX",              // 34 0x22
      "LDAP_IS_LEAF",                        // 35 0x23
      "LDAP_ALIAS_DEREF_PROBLEM",            // 36 0x24
      "",                                    // 37
      "",                                    // 38
      "",                                    // 39
      "",                                    // 40
      "",                                    // 41
      "",                                    // 42
      "",                                    // 43
      "",                                    // 44
      "",                                    // 45
      "",                                    // 46
      "",                                    // 47
      "LDAP_INAPPROPRIATE_AUTH",             // 48 0x30
      "LDAP_INVALID_CREDENTIALS",            // 49 0x31
      "LDAP_INSUFFICIENT_ACCESS",            // 50 0x32
      "LDAP_BUSY",                           // 51 0x33
      "LDAP_UNAVAILABLE",                    // 52 0x34
      "LDAP_UNWILLING_TO_PERFORM",           // 53 0x35
      "LDAP_LOOP_DETECT",                    // 54 0x36
      "",                                    // 55
      "",                                    // 56
      "",                                    // 57
      "",                                    // 58
      "",                                    // 59
      "",                                    // 60
      "",                                    // 61
      "",                                    // 62
      "",                                    // 63
      "LDAP_NAMING_VIOLATION",               // 64 0x40
      "LDAP_OBJECT_CLASS_VIOLATION",         // 65 0x41
      "LDAP_NOT_ALLOWED_ON_NONLEAF",         // 66 0x42
      "LDAP_NOT_ALLOWED_ON_RDN",             // 67 0x43
      "LDAP_ALREADY_EXISTS",                 // 68 0x44
      "LDAP_NO_OBJECT_CLASS_MODS",           // 69 0x45
      "LDAP_RESULTS_TOO_LARGE",              // 70 0x46
      "LDAP_AFFECTS_MULTIPLE_DSAS",          // 71
      "",                                    // 72
      "",                                    // 73
      "",                                    // 74
      "",                                    // 75
      "",                                    // 76
      "",                                    // 77
      "",                                    // 78
      "",                                    // 79
      "LDAP_OTHER",                          // 80 0x50
      "LDAP_SERVER_DOWN",                    // 81 0x51
      "LDAP_LOCAL_ERROR",                    // 82 0x52
      "LDAP_ENCODING_ERROR",                 // 83 0x53
      "LDAP_DECODING_ERROR",                 // 84 0x54
      "LDAP_TIMEOUT",                        // 85 0x55
      "LDAP_AUTH_UNKNOWN",                   // 86 0x56
      "LDAP_FILTER_ERROR",                   // 87 0x57
      "LDAP_USER_CANCELLED",                 // 88 0x58
      "LDAP_PARAM_ERROR",                    // 89 0x59
      "LDAP_NO_MEMORY",                      // 90 0x5a
      "LDAP_CONNECT_ERROR",                  // 91 0x5b
      "LDAP_NOT_SUPPORTED",                  // 92 0x5c
      "LDAP_CONTROL_NOT_FOUND",              // 93 0x5d
      "LDAP_NO_RESULTS_RETURNED",            // 94 0x5e
      "LDAP_MORE_RESULTS_TO_RETURN",         // 95 0x5f
      "LDAP_CLIENT_LOOP",                    // 96 0x60
      "LDAP_REFERRAL_LIMIT_EXCEEDED"         // 97 0x61
   };

   char* descr = ldap_err2string(result);

   /**
    * get a meaningful error string back from the security library
    * this function should be called, if ldap_err2string doesn't
    * identify the error code
    */
#if defined(HAVE_LDAP_SSL_H) && !defined(_MSWINDOWS_) && !defined(HAVE_WINLDAP_H)
   if (descr == 0) descr = (char*)ldapssl_err2string(result);
#endif

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%s (%d, %s)"), (result >= 0 && result < 97 ? errlist[result] : ""), result, descr);
}

#if defined(_MSWINDOWS_) && defined(HAVE_WINLDAP_H)

/**
 * Split 'str' into strings separated by commas.
 *
 * Note: res[] points into 'str'
 */

U_NO_EXPORT char** ULDAP::split_str(char* str)
{
   U_TRACE(0, "ULDAP::split_str(%S)", str)

   int i;
   char **res, *s;

   for (i = 2, s = strchr(str, ','); s; ++i) s = strchr(++s, ',');

   res = (char**)calloc(i, sizeof(char*));

   for (i = 0, s = strtok(str, ","); s; s = strtok(0, ","), ++i) res[i] = s;

   U_DUMP_ATTRS(res)

   return res;
}

#endif

/* Initialize the LDAP session */

bool ULDAP::init(const char* url)
{
   U_TRACE(1, "ULDAP::init(%S)", url)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(url)

   if (ludpp) clear();

#if !defined(_MSWINDOWS_) && !defined(HAVE_WINLDAP_H)
   result = U_SYSCALL(ldap_url_parse, "%S,%p", url, &ludpp);
#else
   result = LDAP_INVALID_SYNTAX;

   /**
    * Break apart the pieces of an LDAP URL
    *
    * Syntax:
    *   ldap://<hostname>:<port>/<base_dn>?<attributes>?<scope>?<filter>?<ext>
    *
    * Defined in RFC4516 section 2
    */

   Url _url(url, u__strlen(url, __PRETTY_FUNCTION__));

   ludpp = (LDAPURLDesc*) calloc(1, sizeof(LDAPURLDesc));

   ludpp->lud_host  = _url.getHost().c_strdup();
   ludpp->lud_port  = _url.getPort();
   ludpp->lud_dn    = _url.getPath().c_strndup(1);
   ludpp->lud_scope = LDAP_SCOPE_BASE;

   U_INTERNAL_DUMP("host = %S dn = %S", ludpp->lud_host, ludpp->lud_dn)

   // parse attributes, skip "??"

   UString query = _url.getQuery();

   char* p = query.data();
   char* q = strchr(p, '?');

   if (q) *q++ = '\0';

   U_INTERNAL_DUMP("attributes = %S", p)

   if (*p && *p != '?') ludpp->lud_attrs = split_str(p);

   p = q;

   if (!p) goto next;

   // parse scope, skip "??"

   q = strchr(p, '?');

   if (q) *q++ = '\0';

   if (*p && *p != '?')
      {
           if (strncmp(p, U_CONSTANT_TO_PARAM("base"))    == 0)   ludpp->lud_scope = LDAP_SCOPE_BASE;
      else if (strncmp(p, U_CONSTANT_TO_PARAM("one"))     == 0)   ludpp->lud_scope = LDAP_SCOPE_ONELEVEL;
      else if (strncmp(p, U_CONSTANT_TO_PARAM("onetree")) == 0)   ludpp->lud_scope = LDAP_SCOPE_ONELEVEL;
      else if (strncmp(p, U_CONSTANT_TO_PARAM("sub"))     == 0)   ludpp->lud_scope = LDAP_SCOPE_SUBTREE;
      else if (strncmp(p, U_CONSTANT_TO_PARAM("subtree")) == 0)   ludpp->lud_scope = LDAP_SCOPE_SUBTREE;

      U_INTERNAL_DUMP("scope = %d %S", ludpp->lud_scope, p)
      }

   p = q;

   if (!p) goto next;

   // parse filter

   q = strchr(p, '?');

   if (q) *q++ = '\0';

   if (!*p) goto next;

   ludpp->lud_filter = strdup(p);

   U_INTERNAL_DUMP("filter = %S", ludpp->lud_filter)

   // parse extensions

   if (q) ludpp->lud_exts = split_str(q);

   result = LDAP_SUCCESS;

next:
#endif

   if (result != LDAP_SUCCESS)
      {
#  ifdef DEBUG
      char buffer[1024];
      const char* descr  = "???";
      const char* errstr = "";

      switch (result)
         {
#  if !defined(_MSWINDOWS_) || !defined(HAVE_WINLDAP_H)
         case LDAP_INVALID_SYNTAX:
            descr  = "LDAP_INVALID_SYNTAX";
            errstr = "Invalid URL syntax";
         break;

         case LDAP_URL_ERR_MEM:
            descr  = "LDAP_URL_ERR_MEM";
            errstr = "Cannot allocate memory space";
         break;

         case LDAP_URL_ERR_PARAM:
            descr  = "LDAP_URL_ERR_PARAM";
            errstr = "Invalid parameter";
         break;

         case LDAP_URL_ERR_BADSCOPE:
            descr  = "LDAP_URL_ERR_BADSCOPE";
            errstr = "Invalid or missing scope string";
         break;

#     if defined(HAVE_LDAP_SSL_H)
         case LDAP_URL_ERR_NOTLDAP:
            descr  = "LDAP_URL_ERR_NOTLDAP";
            errstr = "URL doesn't begin with \"ldap://\"";
         break;

         case LDAP_URL_ERR_NODN:
            descr  = "LDAP_URL_ERR_NODN";
            errstr = "URL has no DN (required)";
         break;

         case LDAP_URL_UNRECOGNIZED_CRITICAL_EXTENSION:
            descr  = "LDAP_URL_UNRECOGNIZED_CRITICAL_EXTENSION";
            errstr = "";
         break;
#     else
         case LDAP_URL_ERR_BADSCHEME:
            descr  = "LDAP_URL_ERR_BADSCHEME";
            errstr = "URL doesnt begin with \"ldap[s]://\"";
         break;

         case LDAP_URL_ERR_BADENCLOSURE:
            descr  = "LDAP_URL_ERR_BADENCLOSURE";
            errstr = "URL is missing trailing \">\"";
         break;

         case LDAP_URL_ERR_BADURL:
            descr  = "LDAP_URL_ERR_BADURL";
            errstr = "Invalid URL";
         break;

         case LDAP_URL_ERR_BADHOST:
            descr  = "LDAP_URL_ERR_BADHOST";
            errstr = "Host port is invalid";
         break;

         case LDAP_URL_ERR_BADATTRS:
            descr  = "LDAP_URL_ERR_BADATTRS";
            errstr = "Invalid or missing attributes";
         break;

         case LDAP_URL_ERR_BADFILTER:
            descr  = "LDAP_URL_ERR_BADFILTER";
            errstr = "Invalid or missing filter";
         break;

         case LDAP_URL_ERR_BADEXTS:
            descr  = "LDAP_URL_ERR_BADEXTS";
            errstr = "Invalid or missing extensions";
         break;
#     endif
#  endif
         }

      U_INTERNAL_DUMP("ldap_url_parse() failed - %.*s", u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("(%d, %s) - %s"), result, descr, errstr), buffer)
#  endif

      U_RETURN(false);
      }

   /* Get the URL scheme (either ldap or ldaps) */

#ifdef HAVE_LDAP_SSL_H
   if (strncmp(url, U_CONSTANT_TO_PARAM("ldaps:")) == 0)
      {
      /* Making encrypted connection */

      isSecure = true;

#  ifdef HAS_NOVELL_LDAPSDK
      /**
       * Initialize the ssl library. The first parameter of ldapssl_client_init is a certificate file. However, when used
       * the file must be a DER encoded file. 0 is passed in for the certificate file because ldapssl_set_verify_mode will be used
       * to specify no server certificate verification. ldapssl_client_init is an application level initialization not a
       * thread level initilization and should be done once
       */

      result = U_SYSCALL(ldapssl_client_init, "%p,%p", 0,  /* DER encoded cert file */
                                                       0); /* reserved, use 0 */

      if (result != LDAP_SUCCESS) U_RETURN(false);

      /**
       * Configure the LDAP SSL library to not verify the server certificate. The default is LDAPSSL_VERIFY_SERVER which validates all servers
       * against the trusted certificates normally passed in during ldapssl_client_init or ldapssl_add_trusted_cert.
       *
       * WARNING:  Setting the verify mode to LDAPSSL_VERIFY_NONE turns off server certificate verification. This means all servers are
       * considered trusted.  This should be used only in controlled environments where encrypted communication between servers and
       * clients is desired but server verification is not necessary
       */

      result = U_SYSCALL(ldapssl_set_verify_mode, "%d", LDAPSSL_VERIFY_NONE);

      if (result != LDAP_SUCCESS)
         {
         U_SYSCALL_NO_PARAM(ldapssl_client_deinit);

         U_RETURN(false);
         }
#  endif

      /* create a LDAP session handle that is enabled for ssl connection */

      ld = (LDAP*) U_SYSCALL(ldapssl_init, "%S,%d,%d", (char*)ludpp->lud_host, /* host name */
                        (ludpp->lud_port ? ludpp->lud_port : LDAPS_PORT),      /* port number */
                        1);                                                    /* 0 - clear text, 1 - enable for ssl */

      if (ld == 0)
         {
#     ifdef HAS_NOVELL_LDAPSDK
         U_SYSCALL_NO_PARAM(ldapssl_client_deinit);
#     endif

         U_RETURN(false);
         }

#  if defined(_MSWINDOWS_) && defined(HAVE_WINLDAP_H)
      (void) U_SYSCALL(ldap_set_option, "%p,%d,%d", ld, LDAP_OPT_SSL, LDAP_OPT_ON);
#  endif
      }
   else
#endif
      {
      /* Making clear text connection */

      if (init(ludpp->lud_host, ludpp->lud_port) == false) U_RETURN(false);
      }

   U_RETURN(true);
}

void ULDAP::get(ULDAPEntry& e)
{
   U_TRACE(1, "ULDAP::get(%p)", &e)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(ld)
   U_INTERNAL_ASSERT_POINTER(searchResult)

   // Go through the search results by checking entries

   int i = 0, j;
   char** values;
   char* attribute;
   LDAPMessage* entry;
   BerElement* ber = 0;

   char* _attribute;
   struct berval** bvs = 0;

   for (entry = ldap_first_entry(ld, searchResult); entry;
        entry = ldap_next_entry( ld, entry), ++i)
      {
      e.dn[i] = ldap_get_dn(ld, entry);

      U_INTERNAL_DUMP("dn[%d]: %S", i, e.dn[i])

      for (attribute = ldap_first_attribute(ld, entry, &ber); attribute;
           attribute = ldap_next_attribute( ld, entry,  ber))
         {
         values = ldap_get_values(ld, entry, attribute); /* Get values */

         U_INTERNAL_DUMP("values = %p attribute = %S", values, attribute)

         if (values)
            {
            e.set(attribute, values, i);

            ldap_value_free(values);
            }
         else
            {
            for (j = 0; j < e.n_attr; ++j)
               {
               // be prepared to the 'attr;binary' versions of 'attr'

               _attribute = (char*)e.attr_name[j];

               U_INTERNAL_DUMP("e.attr_name[%d] = %S", j, _attribute)

               if (strncmp(attribute, _attribute, u__strlen(_attribute, __PRETTY_FUNCTION__)) == 0)
                  {
                  bvs = ldap_get_values_len(ld, entry, attribute);

                  if (bvs)
                     {
                     e.set(_attribute, bvs[0]->bv_val, bvs[0]->bv_len, i);

                     ldap_value_free_len(bvs);

                     break;
                     }
                  }
               }
            }

         ldap_memfree(attribute);
         }

      ber_free(ber, 0);
      }
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* ULDAPEntry::dump(bool reset) const
{
   *UObjectIO::os << "dn                 " << (void*)dn        << '\n'
                  << "n_attr             " << n_attr           << '\n'
                  << "n_entry            " << n_entry          << '\n'
                  << "attr_val           " << (void*)attr_val  << '\n'
                  << "attr_name          " << (void*)attr_name;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* ULDAP::dump(bool reset) const
{
   *UObjectIO::os << "ld                 " << (void*)ld           << '\n'
                  << "ludpp              " << (void*)ludpp        << '\n'
                  << "result             " << result              << '\n'
                  << "isSecure           " << isSecure            << '\n'
                  << "pTimeOut           " << (void*)pTimeOut     << '\n'
                  << "searchResult       " << (void*)searchResult;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
