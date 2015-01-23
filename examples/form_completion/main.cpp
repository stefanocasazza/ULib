// main.cpp

#include <ulib/ldap/ldap.h>
#include <ulib/file_config.h>
#include <ulib/utility/base64.h>

#undef  PACKAGE
#define PACKAGE "form_completion"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for form completion...\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage arg operation

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("form_completion.cfg");

      cfg.UFile::setPath(cfg_str);

      int LDAP_port;
      UString var_env_name, scan_form, LDAP_host, LDAP_searchbase, password, username,
              form_template, LDAP_rootdn, LDAP_rootpw, subject_attr;

      // --------------------------------------------------------------------------------------------------------------
      // configuration parameters
      // --------------------------------------------------------------------------------------------------------------
      // X509_SUBJECT_VARIABLE    var environment to elaborate  (ex: X509_SUBJECT)
      // X509_SUBJECT_FILTER      scanf form to extract data    (ex: /C=IT/L=Rome/O=Eraclito/OU=Tac/CN=%[^/]s/Email=%*s)
      // LDAP_SERVER_ADDRESS      name ldap host                (ex: slessi)
      // LDAP_SERVER_PORT         port ldap host                (ex: 389)
      // LDAP_SERVER_BASE
      // LDAP_SERVER_ROOTDN       bind DN for ldap query data
      // LDAP_SERVER_ROOTPW       bind password for ldap query data
      // LDAP_SUBJECT_ATTRIBUTE   attribute for query filter
      // LDAP_USER_ATTRIBUTE      name user attribute           (ex: uid)
      // LDAP_PASSWD_ATTRIBUTE    name password attribute       (ex: tacPassword)
      // TEMPLATE                 name form completion to print (ex: ./lms1.templ)
      // --------------------------------------------------------------------------------------------------------------

      var_env_name    = cfg[U_STRING_FROM_CONSTANT("X509_SUBJECT_VARIABLE")];
      scan_form       = cfg[U_STRING_FROM_CONSTANT("X509_SUBJECT_FILTER")];
      LDAP_host       = cfg[U_STRING_FROM_CONSTANT("LDAP_SERVER_ADDRESS")];
      LDAP_searchbase = cfg[U_STRING_FROM_CONSTANT("LDAP_SERVER_BASE")];
      LDAP_rootdn     = cfg[U_STRING_FROM_CONSTANT("LDAP_SERVER_ROOTDN")];
      LDAP_rootpw     = cfg[U_STRING_FROM_CONSTANT("LDAP_SERVER_ROOTPW")];
      subject_attr    = cfg[U_STRING_FROM_CONSTANT("LDAP_SUBJECT_ATTRIBUTE")];
      password        = cfg[U_STRING_FROM_CONSTANT("LDAP_PASSWD_ATTRIBUTE")];
      username        = cfg[U_STRING_FROM_CONSTANT("LDAP_USER_ATTRIBUTE")];
      form_template   = cfg[U_STRING_FROM_CONSTANT("TEMPLATE")];
      LDAP_port       = cfg.readLong(U_STRING_FROM_CONSTANT("LDAP_SERVER_PORT"), 389);

      // scanf

      char buf[256];
      const char* value = (const char*) U_SYSCALL(getenv, "%S", var_env_name.c_str());
      int n = U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p", value, scan_form.c_str(), &buf[0]);

      if (n != 1) U_ERROR("scanf error on var env <%s>", var_env_name.c_str());

      U_INTERNAL_DUMP("sscanf() result = \"%s\"", buf)

      // login to LDAP

      ULDAP ldap;

      if (ldap.init(LDAP_host.c_str(), (LDAP_port ? LDAP_port : LDAP_PORT)) == false ||
          ldap.set_protocol() == false ||
          ldap.bind(LDAP_rootdn.c_str(), LDAP_rootpw.c_str()) == false)

         {
         U_ERROR("login to LDAP failed");
         }

      // search to LDAP

      char buffer[4096];
      const char* attr_name[] = { username.c_str(), password.c_str(), 0 };

      (void) snprintf(buffer, sizeof(buffer), "%s=%s,%.*s", subject_attr.c_str(), buf, U_STRING_TO_TRACE(LDAP_searchbase));

      n = ldap.search(buffer, LDAP_SCOPE_BASE, (char**)attr_name);

      if (n != 1) U_ERROR("search error on ldap");

      ULDAPEntry entry(2, attr_name, 1);

      ldap.get(entry);

      const char* sname     = entry.getCStr(0);
      const char* spassword = entry.getCStr(1);

      // write to stdout

      (void) U_SYSCALL(printf, "%S,%S,%S", UFile::contentOf(form_template).data(), sname, spassword);
      }

private:
   UFileConfig cfg;
   UString data, cfg_str;
};

U_MAIN
