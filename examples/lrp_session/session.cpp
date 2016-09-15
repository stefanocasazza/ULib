// session.cpp

#include <ulib/base/utility.h>

#include <ulib/log.h>
#include <ulib/process.h>
#include <ulib/ldap/ldap.h>
#include <ulib/file_config.h>
#include <ulib/utility/semaphore.h>
#include <ulib/ssh/net/sshsocket.h>

#undef  PACKAGE
#define PACKAGE "lrp_session"
#undef  ARGS
#define ARGS "<operation(apply|drop)>"

#define U_OPTIONS \
"purpose \"start session - operation defined with ARG operation <apply|drop> on LRP device defined by filter... \"\n" \
"option c config         1 \"configuration file\" \"lrp.cfg\"\n" \
"option H host           1 \"LDAP server\" \"\"\n" \
"option p port           1 \"port on LDAP server\" \"\"\n" \
"option D bindDN         1 \"bind DN for LDAP query data processing\" \"\"\n" \
"option w password       1 \"bind password (for simple authentication)\" \"\"\n" \
"option b baseDN         1 \"DN filter - to get LDAP attribute value for filter specified\" \"\"\n" \
"option B baseDNDevice   1 \"base DN device - to get LDAP attribute value for device specified\" \"\"\n" \
"option u username       1 \"user name to connect LRP server on device (ssh mode)\" \"\"\n"

#include <ulib/application.h>

// ldap attribute for filter

#define FILTER_ATTR_CN_POS          0
#define FILTER_ATTR_CN_STRING       "cn"
#define FILTER_ATTR_RULE_POS        1
#define FILTER_ATTR_RULE_STRING     "tnetLrpFilterRule"
#define FILTER_NUM_ATTR             2

static const char* filter_attr_name[] = { FILTER_ATTR_CN_STRING, FILTER_ATTR_RULE_STRING, 0 };

// ldap attribute for policy

#define POLICY_ATTR_CN_POS          0
#define POLICY_ATTR_CN_STRING       "cn"
#define POLICY_ATTR_POLICY_POS      1
#define POLICY_ATTR_POLICY_STRING   "tnetLrpPolicylabel"
#define POLICY_ATTR_IPMASK_POS      2
#define POLICY_ATTR_IPMASK_STRING   "tnetLrpIpNetworkNumber"
#define POLICY_NUM_ATTR             3

static const char* policy_attr_name[] = { POLICY_ATTR_CN_STRING, POLICY_ATTR_POLICY_STRING,
                                          POLICY_ATTR_IPMASK_STRING, 0 };

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // LDAP attribute for devices
      // manage arg operation
      // manage file configuration
      // manage options
      // login to LDAP

#     include "common1.cpp"

      if (UApplication::isOptions())
         {
         value = opt['b'];

         if (value.empty() == false) DN_filter = value;
         }

      // get filter attribute from LDAP

      int i, n = ldap.search(DN_filter.c_str(), LDAP_SCOPE_BASE, (char**)filter_attr_name);

      if (n != 1) U_ERROR("cannot get filter attribute from LDAP");

      ULDAPEntry filter_entry(FILTER_NUM_ATTR, filter_attr_name);

      ldap.get(filter_entry);

      // dato filtro avvio ricerca policy usando valore attributo filtro <tnetLrpFilterRule>

      const char* filtro = filter_entry.getCStr(FILTER_ATTR_CN_POS);
      const char* rule   = filter_entry.getCStr(FILTER_ATTR_RULE_POS);

      U_INTERNAL_DUMP("RULE = %S FILTRO = %S", rule, filtro)

      // get policy attribute from LDAP

      ULDAP ldap_url;
      char first_char = rule[0];
      bool policy_by_url = (first_char != '(');

      if (policy_by_url)
         {
         if (first_char == '/')
            {
            static char url[1024];

            (void) u__snprintf(url, sizeof(url), U_CONSTANT_TO_PARAM("ldap://%v%s"), LDAP_host.rep, rule);

            rule = url;
            }

         if (ldap_url.init(rule) == false     ||
             ldap_url.set_protocol() == false ||
             ldap_url.simple_bind() == false)
            {
            U_ERROR("login to LDAP with URL failed");
            }

         n = ldap_url.search();
         }
      else
         {
         n = ldap.search("o=Policies,o=tnet", LDAP_SCOPE_SUBTREE, (char**)policy_attr_name, rule);
         }

      if (n <= 0) U_ERROR("cannot find policy from LDAP");

      ULDAPEntry policy_entry(POLICY_NUM_ATTR, policy_attr_name, n);

      if (policy_by_url) ldap_url.get(policy_entry);
      else                   ldap.get(policy_entry);

      // init log

      const char* log_name = filtro;

#     include "common2.cpp"

      // loop for every policy

      int j, k;
      char ipmask[64];
      const char* ptr;
      const char* policy;
      const char* ip_mask;
      const char* ip_device;
      char request_buffer[4096 * 4];
      const char* binddn_device = LDAP_binddn_device.c_str();

      for (i = 0; i < n; ++i)
         {
         policy = policy_entry.getCStr(POLICY_ATTR_CN_POS, i);

         U_INTERNAL_DUMP("POLICY = %S %S", policy_entry[i], policy)

         // data policy avvio ricerca lista device usando valore attributo policy <tnetLrpIpNetworkNumber>

         ip_mask = policy_entry.getCStr(POLICY_ATTR_IPMASK_POS, i);

         (void) u__snprintf(ipmask, sizeof(ipmask), U_CONSTANT_TO_PARAM("(tnetLrpIpHostNumber=%s*)"), ip_mask);

         j = ldap.search(binddn_device, LDAP_SCOPE_SUBTREE, (char**)device_attr_name, ipmask);

         if (j <= 0) continue;

         ptr = policy_entry.getCStr(POLICY_ATTR_POLICY_POS, i);

         // check if to skip xml header...

         if (strncmp(ptr, U_CONSTANT_TO_PARAM("<?xml")) == 0)
            {
            ptr += 5;

            while (*ptr++ != '\n');
            }

         // build request

         // "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE REQUEST SYSTEM \"lrp_request.dtd\">"

         request_size = u__snprintf(request_buffer, sizeof(request_buffer),
                                    U_CONSTANT_TO_PARAM("<REQUEST sid=\"sid1\" version=\"1\"><IMPORT-POLICYLABEL  name=\"%s\">%s</IMPORT-POLICYLABEL></REQUEST>"
                                    "<REQUEST sid=\"sid2\" version=\"1\"><EXECUTE-POLICYLABEL name=\"%s\" command=\"%s\"/></REQUEST>"),
                                    policy, ptr, policy, operation);

         request = UString(request_buffer, request_size);

         // write request to file

#        include "common3.cpp"

         // set devices attribute for LDAP

         ULDAPEntry device_entry(DEVICE_NUM_ATTR, device_attr_name, j);

         // get devices attribute from LDAP
         // loop for every device
         // fork: child
         //  send request  to device
         // write response to file

#        include "common4.cpp"

         // parent
         }

      exit_value = proc.waitAll();

      log.close();
      }

private:
};

U_MAIN
