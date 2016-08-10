// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_shib.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_SHIB_H
#define U_MOD_SHIB_H 1

#ifdef SOLARIS2
#undef _XOPEN_SOURCE    // causes gethostname conflict in unistd.h
#endif

#define _DEBUG

// SAML Runtime
#include <saml/saml.h>
#include <shib/shib.h>
#include <shib/shib-threads.h>
#include <shib-target/shib-target.h>
#include <xercesc/util/regx/RegularExpression.hpp>

using namespace std;
using namespace saml;
using namespace shibboleth;
using namespace shibtarget;

/*
The Apache module provided can also interpret AAP settings to map attributes to HTTP request headers and to Require rules, permitting
protection of both static and dynamic content. Any of the typical ways of protecting content may be used ( .htaccess , or httpd.conf blocks
including <Directory> , <Location> , <Files> , etc.). They define what content is to be protected and access control rules to apply against
it.

There are two ways to require Shibboleth authentication, but both also require enabling the module to activate by specifying an AuthType of
shibboleth and supplying at least one Require rule in httpd.conf or .htaccess files. This is an Apache requirement. Without AuthType and
Require in effect for a request, the Shibboleth filter will not be invoked by Apache and the request just passes through.

The Require rule can enforce a specific access control policy based on attributes, can specify valid-user to require any authenticated
session, or you can use a placeholder rule name of Shibboleth . This can be used when the actual protection rules are placed in the
RequestMap, or to support LazySessions. In such cases, the module is activated, but in a passive mode that does not automatically force
a session, but will process and validate a session if one exists, leaving the authorization decision to the application. Using a static
access control rule that will fail in the absence of a session is only sensible if one of the two approaches below that force a session
are used.

To require a session, either the Apache command, ShibRequireSession On , or the requireSession boolean XML attribute on the closest
RequestMap, Host, or Path elements in Shibboleth Xml can be used. Both approaches are equivalent, and using either one to require a
session will supersede a false or absent setting of the other type. If your requests are still passing through, you may have forgotten
to apply the Apache settings above.

As an example, the following commands will simply require Shibboleth authentication for a resource:

AuthType shibboleth
   ShibRequireSession On
   Require valid-user

When you want to defer all decisions to the RequestMap, you can apply the filter passively to all content like so:

<Location />
   AuthType shibboleth
   Require shibboleth
</Location>

A complete list of Shibboleth Apache directives and their values is below:

AuthType <string>          Use shibboleth for direct invocation, or Basic plus the ShibBasicHijack option described below.

ShibBasicHijack <on/off>   Controls whether Shibboleth should or should not ignore requests with AuthType Basic . Defaults to off.

AuthGroupFile <pathname>   Same as mod_auth ; collects values found in REMOTE_USER into a named group for access control. An attribute
                           must be mapped to REMOTE_USER for this to work (defaults to eduPersonPrincipalName). mod_auth will not support
                           group files when the Shibboleth module is loaded, since they share the same command.

ShibRequireSession <on/off> Controls whether to require an authenticated session before passing control to the authorization phase
                            or the actual resource. Defaults to off .

ShibRequireSessionWith <SessionInitiator_id> Initiate a session using a specific SessionInitiator if no session exists.

ShibApplicationId <ApplicationId>   Set ApplicationId for this content.

ShibRequireAll <on/off> Controls whether all Require rules specified must be satisfied before access to the resource is granted.
                        Defaults to off , which means any single rule can be satisfied, the usual Apache behavior.

ShibDisable <on/off>    Disable all Shibboleth module activity here to save processing effort. Defaults to off. note: if a require
                        statement is set, there needs to be another AuthType defined to handle it.

ShibRedirectToSSL <portnumber>   When this directive is set all non-ssl http GET or HEAD request are automatically redirected to https
                                 before processing by Shibboleth. Other methods (like POST) are presented an error page to block access.
                                 This is highly recommended in combination with setting your cookies to secure as it will prevent looping
                                 and enhance security. Of course you'll set it as ShibRedirectToSSL 443 most of the time.
                                 (available from v1.3d)

ShibURLScheme <http/https>    Used in advanced virtual hosting environments which need to generate SSL redirects from virtual servers
                              that use only HTTP (eg when offloading SSL to another machine). With this directive you can force Shibboleth
                              to send out a redirect to the HANDLER (eg Shibboleth.sso) on https, even though it runs on http when looking
                              at the local machine. Supplements the Apache ServerName and Port commands with this missing option. Defaults
                              to a null value in which the scheme for redirects is based on the physical connection to the server. This is
                              a server-level command, while the rest of the commands listed are content commands that can appear anywhere.

ShibExportAssertion <on/off>  Controls whether the SAML attribute assertion provided by the AA is exported in a base64-encoded HTTP
                              header, HTTP_SHIB_ATTRIBUTES . Defaults to off .

There are many ways that authorization can be performed once Shibboleth has successfully transported attributes; any module that performs
a standard call can be used. The below example uses the standard pre-installed mod_auth by placing a .htaccess file that references a
group file stored at /pathname :

AuthGroupFile /pathname
    require group workgroup

An AuthGroupFile used by Shibboleth might resemble:
   workgroup: joe@example.edu, jane@demo.edu, jim@sample.edu

Require <string>  Enforce authorization using one of the following methods:

  * valid-user : Any Shibboleth user from a trusted !IdP site is accepted, even if no actual attributes are received. This is a very
                 minimal kind of policy, but is useful for testing or for deferring real policy to an application.
  * user       : A space-delimited list of values compared against REMOTE_USER , which is populated by default from
                 urn:mace:dir:attribute-def:eduPersonPrincipalName . Any attribute can be mapped to REMOTE_USER as defined by AAP.xml .
  * group      : A space-delimited list of group names defined within AuthGroupFile files, again provided that a mapping to REMOTE_USER
                 exists.
  * alias      : An arbitrary rule name that matches an Alias defined in an AAP.xml . The rule value is a space-delimited list of attribute
                 values, whose format depends on the attribute in question (e.g. an affiliation rule might look like:
                 require affiliation staff@osu.edu faculty@mit.edu=
  * shibboleth : If a session cookie of the expected name exists, the corresponding session will be validated and any cached attributes
                 exported as otherwise specified. Authorization will be controlled by the resource, unless additional rules are specified.
                 If however a session does not already exist, or if the current session expires or times out, no session will be requested
                 and control will pass to the resource. This is known as "lazy session" (see below).

For user and alias-based rules, if a tilde character (~) is placed immediately following user or alias , the expressions that follow are
treated as regular expressions. The syntax supported is generally based on the one defined by XML Schema. This specification borders on
unreadable, but the syntax is generally Perl-like. Expressions should generally be "anchored" with the ^ and $ symbols to ensure mid-string
matches don't cause false positives.

For example, the rule:

require affiliation ~ ^member@.+\.edu$

would evaluate to allowing anyone with an eduPersonAffiliation of member from a .edu domain.
*/

// per-dir module configuration structure

typedef struct shib_dir_config {
   //
   // RM Configuration

   char* szAuthGrpFile;    // Auth GroupFile name
   int bRequireAll;        // all require directives must match, otherwise OR logic

   // Content Configuration

   char* szApplicationId;  // Shib applicationId value
   char* szRequireWith;    // require a session using a specific initiator?
   char* szRedirectToSSL;  // redirect non-SSL requests to SSL port
   int bOff;               // flat-out disable all Shib processing
   int bBasicHijack;       // activate for AuthType Basic?
   int bRequireSession;    // require a session?
   int bExportAssertion;   // export SAML assertion to the environment?
} shib_dir_config;

#include <ulib/string.h>

class UShibTarget : public ShibTarget {
public:
   shib_dir_config* m_dc;

   static const char* protocol;
   static const char* hostname;
   static int         port;
   static const char* uri;
   static const char* content_type;
   static const char* remote_addr;

   static const char* cookies;
   static const char* location;

   static uint32_t    postdata_len;
   static const char* postdata_ptr;

   static int               codepage;
   static UVector<UString>* sendpage;
   static UVector<UString>* setcookie;

    UShibTarget();
   ~UShibTarget() {}

   // Send a message to the Webserver log

   virtual void log(ShibLogLevel level, const string &msg);

   // Get/Set a cookie for this request

   virtual string getCookies(void) const;
   virtual void   setCookie(const string& name, const string& value);

   // Get the request's GET arguments or POST data from the server

   virtual string getArgs(void);
   virtual string getPostData(void);

   // Clear a header, set a header
   // These APIs are used for exporting the Assertions into the
   // Headers.  It will clear some well-known headers first to make
   // sure none remain.  Then it will process the set of assertions
   // and export them via setHeader().

   virtual void   clearHeader(const string& name);
   virtual void   setHeader(const string& name, const string& value);
   virtual string getHeader(const string& name);
   virtual void   setRemoteUser(const string& user);
   virtual string getRemoteUser(void);

   // We're done.  Finish up. Send specific result content or a redirect.
   // If there are no headers supplied assume the content-type is text/html

   virtual void* sendPage(const std::string& msg,
                          int code = 200, const std::string& content_type = "text/html",
                          const saml::Iterator<header_t>& headers = EMPTY(header_t));

   virtual void* sendRedirect(const string& url);

   // These next two APIs are used to obtain the module-specific "OK"
   // and "Decline" results.  OK means "we believe that this request
   // should be accepted".  Declined means "we believe that this is
   // not a shibbolized request so we have no comment".

   virtual void* returnDecline(void);
   virtual void* returnOK(void);
};

#include <ulib/net/server/server_plugin.h>

class U_EXPORT UShibPlugIn : public UServerPlugIn {
public:

            UShibPlugIn();
   virtual ~UShibPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg);
   virtual int handlerInit();

   // Connection-wide hooks

   virtual int handlerRequest();

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString cookies;
   ShibTargetConfig* conf;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UShibPlugIn)
};

#endif
