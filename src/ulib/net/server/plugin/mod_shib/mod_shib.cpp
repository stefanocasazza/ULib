// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_shib.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/server/server.h>

#include <ulib/net/server/plugin/mod_shib.h>

#undef _XPG4_2
#include <sstream>

// creates per-directory config structure

static shib_dir_config* create_shib_dir_config()
{
   static shib_dir_config dc;

   // RM Configuration

   dc.szAuthGrpFile = NULL;    // Auth GroupFile name
   dc.bRequireAll = -1;        // all require directives must match, otherwise OR logic

   // Content Configuration

   dc.szApplicationId = NULL;  // Shib applicationId value
   dc.szRequireWith = NULL;    // require a session using a specific initiator?
   dc.szRedirectToSSL = NULL;  // redirect non-SSL requests to SSL port
   dc.bOff = -1;               // flat-out disable all Shib processing
   dc.bBasicHijack = -1;       // activate for AuthType Basic?
   dc.bRequireSession = 1;     // Initiates a new session if one does not exist?
   dc.bExportAssertion = 1;    // export SAML assertion to the environment?

   return &dc;
}

const char* UShibTarget::protocol = "http";
const char* UShibTarget::hostname;
int         UShibTarget::port = 80;
const char* UShibTarget::uri;
const char* UShibTarget::content_type;
const char* UShibTarget::remote_addr;

const char* UShibTarget::location;
const char* UShibTarget::cookies;

uint32_t    UShibTarget::postdata_len;
const char* UShibTarget::postdata_ptr;

int               UShibTarget::codepage;
UVector<UString>* UShibTarget::sendpage;
UVector<UString>* UShibTarget::setcookie;

UShibTarget::UShibTarget()
{
   U_TRACE_NO_PARAM(5, "UShibTarget::UShibTarget()")

   m_dc = create_shib_dir_config();

   // Initialize the request from the parsed URL
   // protocol == http, https, etc
   // hostname == server name
   // port == server port
   // uri == resource path
   // method == GET, POST, etc.

   U_INTERNAL_DUMP("protocol = %S hostname = %S port = %d uri = %S content_type = %S remote_addr = %S method = %.*S",
                    protocol,     hostname,     port,     uri,     content_type,     remote_addr,     U_HTTP_METHOD_TO_TRACE)

   ShibTarget::init(protocol, hostname, port, uri, content_type, remote_addr, U_http_method_list[U_http_method_num].name); 
}

void UShibTarget::log(ShibLogLevel level, const string& msg)
{
   U_TRACE(0, "UShibTarget::log(%d,%S)", level, msg.c_str())

#ifdef DEBUG
   const char* l = (level == LogLevelDebug ? "LogLevelDebug" :
                    level == LogLevelInfo  ? "LogLevelInfo"  :
                    level == LogLevelWarn  ? "LogLevelWarn"  : "LogLevelError");

   U_MESSAGE("%s - %s", l, msg.c_str());
#endif
}

string UShibTarget::getCookies() const
{
   U_TRACE_NO_PARAM(0, "UShibTarget::getCookies()")

   string c = cookies;

   U_INTERNAL_DUMP("Cookie = %S", c.c_str())

   return c;
}

void UShibTarget::setCookie(const string& name, const string& value)
{
   U_TRACE(0, "UShibTarget::setCookies(%S,%S)\n", name.c_str(), value.c_str())

   if (!setcookie) setcookie = new UVector<UString>();

   setcookie->push(UString((void*)name.c_str()));
   setcookie->push(UString((void*)value.c_str()));
}

string UShibTarget::getArgs()
{
   U_TRACE_NO_PARAM(0, "UShibTarget::getArgs()")

   return string("");
}

string UShibTarget::getPostData()
{
   U_TRACE_NO_PARAM(0, "UShibTarget::getPostData()")

   string cgistr(postdata_ptr, postdata_len);

   postdata_len = 0;

   return cgistr;
}

void UShibTarget::clearHeader(const string& name)
{
   U_TRACE(0, "UShibTarget::clearHeader(%S)", name.c_str())
}

void UShibTarget::setHeader(const string& name, const string& value)
{
   U_TRACE(0, "UShibTarget::setHeader(%S,%S)", name.c_str(), value.c_str())
}

string UShibTarget::getHeader(const string& name)
{
   U_TRACE(0, "UShibTarget::getHeader(%S)", name.c_str())

   return string("");
}

void UShibTarget::setRemoteUser(const string& user)
{
   U_TRACE(0, "UShibTarget::setRemoteUser(%S)", user.c_str())
}

string UShibTarget::getRemoteUser()
{
   U_TRACE_NO_PARAM(0, "UShibTarget::getRemoteUser()")

   string c = remote_addr;

   U_INTERNAL_DUMP("remote_addr = %S", c.c_str())

   return c;
}

void* UShibTarget::sendPage(const std::string& msg, int code, const std::string& content_type, const saml::Iterator<header_t>& headers)
{
   U_TRACE(0, "UShibTarget::sendPage(%S,%d,%S)", msg.c_str(), code, content_type.c_str())

   if (!sendpage) sendpage = new UVector<UString>;

   while (headers.hasNext())
      {
      const header_t& h = headers.next();

      sendpage->push(UString((void*)h.first.c_str()));
      sendpage->push(UString((void*)h.second.c_str()));
      }

   codepage = code;

   return (void*)0;
}

void* UShibTarget::sendRedirect(const string& url)
{
   U_TRACE(0, "UShibTarget::sendRedirect(%S)", url.c_str())

   UShibTarget::location = strdup(url.c_str());

   return (void*)0;
}

void* UShibTarget::returnDecline()
{
   U_TRACE_NO_PARAM(0, "UShibTarget::returnDecline()")

   return (void*)1;
}

void* UShibTarget::returnOK()
{
   U_TRACE_NO_PARAM(0, "UShibTarget::returnOK()")

   return (void*)0;
}

/* Access control plugin that enforces access rules
*/

class U_EXPORT UAccessControl : virtual public IAccessControl {
public:
    UAccessControl() {}
   ~UAccessControl() {}

   void lock() {}
   void unlock() {}

   bool authorized(ShibTarget* st, ISessionCacheEntry* entry) const;
};

class U_EXPORT URequestMapper : public virtual IRequestMapper, public virtual IPropertySet {
public:

   URequestMapper(const DOMElement* e);
   ~URequestMapper() { delete m_mapper; delete m_htaccess; delete m_staKey; delete m_propsKey; }

   void lock()    { m_mapper->lock(); }
   void unlock()  { m_staKey->setData(NULL); m_propsKey->setData(NULL); m_mapper->unlock(); }

   Settings getSettings(ShibTarget* st) const;

   pair<bool,bool> getBool(const char* name, const char* ns=NULL) const;
   pair<bool,const char*> getString(const char* name, const char* ns=NULL) const;
   pair<bool,const XMLCh*> getXMLString(const char* name, const char* ns=NULL) const;
   pair<bool,unsigned int> getUnsignedInt(const char* name, const char* ns=NULL) const;
   pair<bool,int> getInt(const char* name, const char* ns=NULL) const;
   const IPropertySet* getPropertySet(const char* name, const char* ns="urn:mace:shibboleth:target:config:1.0") const;
   const DOMElement* getElement() const;

private:
   IRequestMapper* m_mapper;
   ThreadKey* m_staKey;
   ThreadKey* m_propsKey;
   IAccessControl* m_htaccess;
};

static PlugManager* m_plugMgr;

bool UAccessControl::authorized(ShibTarget* st, ISessionCacheEntry* entry) const
{
   U_TRACE(0, "UAccessControl::authorized(%p,%p)", st, entry)

   // Make sure the object is our type.

   UShibTarget* sta = dynamic_cast<UShibTarget*>(st);

   if (!sta) throw ConfigurationException("Request wrapper object was not of correct type.");

   return true;
}

URequestMapper::URequestMapper(const DOMElement* e) : m_mapper(NULL), m_staKey(NULL), m_propsKey(NULL), m_htaccess(NULL)
{
   U_TRACE(0, "URequestMapper::URequestMapper(%p)", e)

   IPlugIn* p = m_plugMgr->newPlugin(shibtarget::XML::XMLRequestMapType,e);

   U_INTERNAL_DUMP("p = %p", p)

   m_mapper = dynamic_cast<IRequestMapper*>(p); // __dynamic_cast(0x643e08, 0x61d760, 0x61d6c0, -1, 0x6b0530) = 0

   U_INTERNAL_DUMP("m_mapper = %p", m_mapper)

   if (!m_mapper)
      {
      delete p;

      throw UnsupportedExtensionException("Embedded request mapper plugin was not of correct type.");
      }

   m_htaccess = new UAccessControl();

   m_staKey = ThreadKey::create(NULL);

   m_propsKey = ThreadKey::create(NULL);
}

IRequestMapper::Settings URequestMapper::getSettings(ShibTarget* st) const
{
   U_TRACE(0, "URequestMapper::getSettings(%p)", st)

   m_staKey->setData(dynamic_cast<UShibTarget*>(st));

   Settings s = m_mapper->getSettings(st);

   m_propsKey->setData((void*)s.first);

   return pair<const IPropertySet*,IAccessControl*>(this, s.second ? s.second : m_htaccess);
}

pair<bool,bool> URequestMapper::getBool(const char* name, const char* ns) const
{
   U_TRACE(0, "URequestMapper::getBool(%S,%S)", name, ns)

   UShibTarget* sta = reinterpret_cast<UShibTarget*>(m_staKey->getData());

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   if (sta && !ns)
      {
      U_INTERNAL_DUMP("bRequireSession = %d bExportAssertion = %d", sta->m_dc->bRequireSession, sta->m_dc->bExportAssertion)

      // Override settable boolean properties.

      if      (name && !strcmp(name, "requireSession")  && sta->m_dc->bRequireSession  == 1) return make_pair(true,true);
      else if (name && !strcmp(name, "exportAssertion") && sta->m_dc->bExportAssertion == 1) return make_pair(true,true);
      }

   return s ? s->getBool(name,ns) : make_pair(false,false);
}

pair<bool,const char*> URequestMapper::getString(const char* name, const char* ns) const
{
   U_TRACE(0, "URequestMapper::getString(%S,%S)", name, ns)

   UShibTarget* sta = reinterpret_cast<UShibTarget*>(m_staKey->getData());

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   if (sta && !ns)
      {
      // Override Apache-settable string properties.

      if (name && !strcmp(name,"authType"))
         {
         const char* auth_type = "shibboleth";

         return make_pair(true, auth_type);
         }
      else if (name && !strcmp(name,"applicationId") && sta->m_dc->szApplicationId)
         return pair<bool,const char*>(true,sta->m_dc->szApplicationId);
      else if (name && !strcmp(name,"requireSessionWith") && sta->m_dc->szRequireWith)
         return pair<bool,const char*>(true,sta->m_dc->szRequireWith);
      else if (name && !strcmp(name,"redirectToSSL") && sta->m_dc->szRedirectToSSL)
         return pair<bool,const char*>(true,sta->m_dc->szRedirectToSSL);
      }

   return s ? s->getString(name,ns) : pair<bool,const char*>(false,NULL);
}

pair<bool,const XMLCh*> URequestMapper::getXMLString(const char* name, const char* ns) const
{
   U_TRACE(0, "URequestMapper::getXMLString(%S,%S)", name, ns)

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   return s ? s->getXMLString(name,ns) : pair<bool,const XMLCh*>(false,NULL);
}

pair<bool,unsigned int> URequestMapper::getUnsignedInt(const char* name, const char* ns) const
{
   U_TRACE(0, "URequestMapper::getUnsignedInt(%S,%S)", name, ns)

   UShibTarget* sta = reinterpret_cast<UShibTarget*>(m_staKey->getData());

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   if (sta && !ns)
      {
      // Override Apache-settable int properties.
      if (name &&
          !strcmp(name,"redirectToSSL") &&
          sta->m_dc->szRedirectToSSL) return pair<bool,unsigned int>(true,strtol(sta->m_dc->szRedirectToSSL,NULL,10));
      }

   return s ? s->getUnsignedInt(name,ns) : pair<bool,unsigned int>(false,0);
}

pair<bool,int> URequestMapper::getInt(const char* name, const char* ns) const
{
   U_TRACE(0, "URequestMapper::getInt(%S,%S)", name, ns)

   UShibTarget* sta = reinterpret_cast<UShibTarget*>(m_staKey->getData());

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   if (sta && !ns)
      {
      // Override Apache-settable int properties.

      if (name &&
          !strcmp(name,"redirectToSSL") &&
          sta->m_dc->szRedirectToSSL) return pair<bool,int>(true,atoi(sta->m_dc->szRedirectToSSL));
      }

   return s ? s->getInt(name,ns) : pair<bool,int>(false,0);
}

const IPropertySet* URequestMapper::getPropertySet(const char* name, const char* ns) const
{
   U_TRACE(0, "URequestMapper::getPropertySet(%S,%S)", name, ns)

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   return s ? s->getPropertySet(name,ns) : NULL;
}

const DOMElement* URequestMapper::getElement() const
{
   U_TRACE_NO_PARAM(0, "URequestMapper::getElement()")

   const IPropertySet* s = reinterpret_cast<const IPropertySet*>(m_propsKey->getData());

   return s ? s->getElement() : NULL;
}

// SERVICES

#include <ulib/date.h>
#include <ulib/mime/header.h>

extern "C" {

static IPlugIn* UAccessFactory(const DOMElement* e)
{
   U_TRACE(0, "::UAccessFactory(%p)", e)

   return new UAccessControl();
}

static IPlugIn* URequestMapFactory(const DOMElement* e)
{
   U_TRACE(0, "::URequestMapFactory(%p)", e)

   return new URequestMapper(e);
}

static int shib_check_user()
{
   U_TRACE_NO_PARAM(0, "shib_check_user()")

   ostringstream threadid;
   threadid << "[" << getpid() << "] shib_check_user" << '\0';
   saml::NDC ndc(threadid.str().c_str());

   try {
      UShibTarget sta;

      // Check user authentication and export information, then set the handler bypass

      pair<bool,void*> res = sta.ShibTarget::doCheckAuthN(true);

      if (res.first) U_RETURN((long)res.second);

      // user auth was okay -- export the assertions now

      res = sta.doExportAssertions();

      if (res.first) U_RETURN((long)res.second);

      // export happened successfully..  this user is ok.

      U_RETURN(0);
      }
   catch (SAMLException& e)
      {
      fprintf(stderr, "shib_check_user threw an exception: %s\n", e.what());

      return -2;
      }
#ifndef DEBUG
   catch (...)
      {
      fprintf(stderr, "shib_check_user threw an uncaught exception!\n");

      return -2;
      }
#endif
}

static int shib_handler()
{
   U_TRACE_NO_PARAM(0, "shib_handler()")

   ostringstream threadid;
   threadid << "[" << getpid() << "] shib_handler" << '\0';
   saml::NDC ndc(threadid.str().c_str());

   try {
      UShibTarget sta;

      pair<bool,void*> res = sta.ShibTarget::doHandler();

      if (res.first) U_RETURN((long)res.second);

      U_WARNING("doHandler() did not do anything");

      U_RETURN(-2);
      }
   catch (SAMLException& e)
      {
      fprintf(stderr, "shib_handler threw an exception: %s\n", e.what());

      return -2;
      }
#ifndef DEBUG
   catch (...)
      {
      fprintf(stderr, "shib_handler threw an uncaught exception!\n");

      return -2;
      }
#endif
}

}

U_CREAT_FUNC(mod_shib, UShibPlugIn)

UShibPlugIn::UShibPlugIn()
{
   U_TRACE_NO_PARAM(0, "UShibPlugIn::UShibPlugIn()")

   conf = 0;
}

UShibPlugIn::~UShibPlugIn()
{
   U_TRACE_NO_PARAM(0, "UShibPlugIn::~UShibPlugIn()")

   conf->shutdown();
}

// Server-wide hooks

int UShibPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UShibPlugIn::handlerConfig(%p)", &cfg)

   if (UModProxyService::loadConfig(cfg)) U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

/*
 * SHIB_SCHEMAS defines the default location where the schemas will be installed.
 */
#undef  SHIB_SCHEMAS
#define SHIB_SCHEMAS "/usr/share/xml/shibboleth"

/*
 * SHIB_CONFIG defines the default location of the Shib Target Configuration File.
 */
#undef  SHIB_CONFIG
#define SHIB_CONFIG "/etc/shibboleth/shibboleth.xml"

int UShibPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UShibPlugIn::handlerInit()")

   U_INTERNAL_ASSERT_EQUALS(conf,0)

   if (UHTTP::vservice->empty() == false)
      {
      // initialize the shib-target library

      conf = &(ShibTargetConfig::getConfig());

      conf->setFeatures(
         ShibTargetConfig::Listener |
         ShibTargetConfig::Metadata |
         ShibTargetConfig::AAP |
         ShibTargetConfig::RequestMapper |
         ShibTargetConfig::LocalExtensions |
         ShibTargetConfig::Logging
      );

      // SHIB_SCHEMAS -> Path to Shibboleth XML schema directory
      // SHIB_CONFIG  -> Path to shibboleth.xml config file

      if (conf->init(SHIB_SCHEMAS))
         {
         m_plugMgr = &(SAMLConfig::getConfig().getPlugMgr());

         m_plugMgr->regFactory(shibtarget::XML::htAccessControlType,  &UAccessFactory);
         m_plugMgr->regFactory(shibtarget::XML::NativeRequestMapType, &URequestMapFactory);
         m_plugMgr->regFactory(shibtarget::XML::LegacyRequestMapType, &URequestMapFactory);

         if (conf->load(SHIB_CONFIG)) U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
         }
      }

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

// Connection-wide hooks

int UShibPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UShibPlugIn::handlerRequest()")

   U_INTERNAL_DUMP("uri = %.*S", U_HTTP_URI_TO_TRACE)

   // find shib service for the HTTP request

   UModProxyService* service = UModProxyService::findService(U_HTTP_HOST_TO_PARAM, U_HTTP_URI_TO_PARAM);

   if (service)
      {
      UString host(U_HTTP_HOST_TO_PARAM);
      uint32_t pos = host.find(':');

      if (pos == U_NOT_FOUND) pos               = host.size();
      else                    UShibTarget::port = atoi(host.c_pointer(pos+1));

   // UShibTarget::protocol    = "http";
      UShibTarget::hostname    = strndup(host.data(), pos);
      UShibTarget::uri         = strndup(U_HTTP_URI_TO_PARAM);
      UShibTarget::remote_addr = strdup(UServer_Base::client_address);
      UShibTarget::cookies     = (U_http_info.cookie_len ? ((void)cookies.replace(U_http_info.cookie, U_http_info.cookie_len), cookies.c_str()) : "");

      int mode              = 0;
      UShibTarget::location = 0;
      UShibTarget::sendpage = 0;

      if (UHTTP::isGET())
         {
         if (shib_check_user()) mode = -1;
         else
            {
            // maybe cookie _shibsession_* already set...

            if (UShibTarget::sendpage)
               {
               if (UShibTarget::codepage == 500) mode = -1;
               }
            else
               {
               if (UShibTarget::location == 0)
                  {
                  static char buf[1024];

                  (void) snprintf(buf, sizeof(buf), U_CONSTANT_TO_PARAM("%s://%s%s/"), UShibTarget::protocol, UShibTarget::hostname, UShibTarget::uri);

                  UShibTarget::location = buf;
                  }
               }
            }
         }
      else
         {
         U_ASSERT(UHTTP::isPOST())
         U_ASSERT(U_HTTP_CTYPE_STRNEQ("application/x-www-form-urlencoded"))

         UShibTarget::content_type = "application/x-www-form-urlencoded";
         UShibTarget::postdata_ptr = UClientImage_Base::body->data();
         UShibTarget::postdata_len = UClientImage_Base::body->size();

         if (shib_handler()) mode = -1;
         else
            {
            // check if have read post data

            if (UShibTarget::postdata_len) mode = -1;
            }
         }

      // redirection...

      if (mode == 0)
         {
         if (UShibTarget::location == 0) mode = -1;
         else
            {
            if (UShibTarget::setcookie)
               {
               UString name, value;

               UHTTP::ext->setBuffer(U_CAPACITY);

               for (uint32_t i = 0, length = UShibTarget::setcookie->size(); i < length; i += 2)
                  {
                  name  = UShibTarget::setcookie->at(i);
                  value = UShibTarget::setcookie->at(i+1);

                  UHTTP::ext->snprintf_add(U_CONSTANT_TO_PARAM("Set-Cookie: %v=%v\r\n"), name.rep, value.rep);
                  }

               delete UShibTarget::setcookie;

               UShibTarget::setcookie = 0;
               }

            UHTTP::setRedirectResponse(0, UShibTarget::location, u__strlen(UShibTarget::location, __PRETTY_FUNCTION__));
            }
         }

      UShibTarget::hostname = UShibTarget::uri = 0;

      U_SYSCALL_VOID(free, "%p", (void*)UShibTarget::uri);
      U_SYSCALL_VOID(free, "%p", (void*)UShibTarget::hostname);
      U_SYSCALL_VOID(free, "%p", (void*)UShibTarget::remote_addr);

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}
