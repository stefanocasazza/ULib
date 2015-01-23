// pusher.cpp

#include <ulib/base/utility.h>

#include <ulib/log.h>
#include <ulib/process.h>
#include <ulib/ldap/ldap.h>
#include <ulib/file_config.h>
#include <ulib/utility/semaphore.h>
#include <ulib/utility/string_ext.h>
#include <ulib/ssh/net/sshsocket.h>

#undef  PACKAGE
#define PACKAGE "lrp_pusher"
#undef  ARGS
#define ARGS "<ip_device> <file_request> <operation(apply|drop)>"

#define U_OPTIONS \
"purpose \"push operation <apply|drop> by request <file_request> to device <ip_device>... \"\n" \
"option c config       1 \"configuration file\" \"lrp.cfg\"\n" \
"option H host         1 \"LDAP server\" \"\"\n" \
"option p port         1 \"port on LDAP server\" \"\"\n" \
"option D bindDN       1 \"bind DN for LDAP query data processing\" \"\"\n" \
"option B baseDNDevice 1 \"base DN device - to get LDAP attribute value for device specified\" \"\"\n" \
"option w password     1 \"bind password (for simple authentication)\" \"\"\n" \
"option u username     1 \"user name to connect LRP server on device (ssh mode)\" \"\"\n"

#include <ulib/application.h>

static int j = 1, k;
static char filtro[128], policy[128], ipfilter[64];

// scan file name: WebServices_webservices-amount_130405_140344.req

#define  SEP  "_"
#define NSEP  "%[^_]"
#define NSEPD "%*[^_]"

static int scanFileName(const char* file_request)
{
   U_TRACE(5, "scanFileName(%S)", file_request)

// char date[20];
// char tempo[20];

   int n = U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p", file_request, // WebServices_webservices-amount_130405_140344.req
                     NSEP  SEP // FILTRO
                     NSEP  SEP // POLICY
                     NSEPD SEP // DATE
                     NSEPD SEP // TIME
                     ".req", &filtro[0], &policy[0]); //, &date[0], &tempo[0]);

   return n;
}

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage args

      const char* ip_device = argv[optind++];

      if (ip_device == 0) U_ERROR("<ip_device> not specified");

      const char* file_request = argv[optind++];

      if (file_request == 0) U_ERROR("<file_request> not specified");

      // LDAP attribute for devices
      // manage arg operation
      // manage file configuration
      // manage options
      // login to LDAP

#     include "common1.cpp"

      // init log

      const char* log_name = "Custom_Sessions";

#     include "common2.cpp"

      // set devices attribute for LDAP

      ULDAPEntry device_entry(DEVICE_NUM_ATTR, device_attr_name, j);

      // loop for every triple args (<ip_device> <file_request> <operation>)...

      const char* binddn_device = LDAP_binddn_device.c_str();

      for (int n; (ip_device && file_request && operation); ip_device    = argv[optind++],
                                                            file_request = argv[optind++],
                                                            operation    = argv[optind++])
         {
         // search device into LDAP

         (void) snprintf(ipfilter, sizeof(ipfilter), "(tnetLrpIpHostNumber=%s)", ip_device);

         n = ldap.search(binddn_device, LDAP_SCOPE_SUBTREE, (char**)device_attr_name, ipfilter);

         if (n != 1)
            {
            U_WARNING("cannot find device <%s> into LDAP...", ip_device);

            continue;
            }

         // scan file name: WebServices_webservices-amount_130405_140344.req

         n = scanFileName(file_request);

         if (n != 2) // filtro and policy...
            {
            U_WARNING("error parsing <%s> name...", file_request);

            continue;
            }

         // load old request file

         name.snprintf("%s/%s", directory.c_str(), file_request);

         request = UFile::contentOf(name);

         if (request.empty())
            {
            U_WARNING("error reading <%.*s>...", U_STRING_TO_TRACE(name));

            continue;
            }

         // substitute operation

         request = (strcmp(operation, "apply") == 0
                           ? UStringExt::substitute(request, U_CONSTANT_TO_PARAM("command=\"drop\"/>"),
                                                             U_CONSTANT_TO_PARAM("command=\"apply\"/>"))
                           : UStringExt::substitute(request, U_CONSTANT_TO_PARAM("command=\"apply\"/>"),
                                                             U_CONSTANT_TO_PARAM("command=\"drop\"/>")));

         request_size = request.size();

         // write new request to file

#        include "common3.cpp"

         // get devices attribute from LDAP
         // fork: child
         //  send request  to device
         // write response to file

#        include "common4.cpp"

         // parent manage args
         }

      exit_value = proc.waitAll();

      log.close();
      }

private:
};

U_MAIN
