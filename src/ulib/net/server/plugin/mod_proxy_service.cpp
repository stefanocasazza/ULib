// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_proxy_service.cpp - service for plugin proxy for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/date.h>
#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/net/server/plugin/mod_proxy_service.h>

UModProxyService::UModProxyService()
{
   U_TRACE_REGISTER_OBJECT(0, UModProxyService, "")

   command = 0;
   vremote_address = 0;
   port = method_mask = 0;
   request_cert = follow_redirects = response_client = websocket = false;
}

UModProxyService::~UModProxyService()
{
   U_TRACE_UNREGISTER_OBJECT(0, UModProxyService)

   if (vremote_address) delete vremote_address;
}

bool UModProxyService::loadConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UModProxyService::loadConfig(%p)", &cfg)

   // proxy - plugin parameters
   // -----------------------------------------------------------------------------------------------------------------------------------
   // ERROR MESSAGE        Allows you to tell clients about what type of error
   //
   // URI                  uri mask trigger
   // HOST                 name host client
   // METHOD_NAME          mask name of what type of HTTP method is permitted (GET|POST|...)
   // CLIENT_CERTIFICATE   yes if client must comunicate a certificate in the SSL connection
   // REMOTE_ADDRESS_IP    list of comma separated client address for IP-based control (IPADDR[/MASK]) for routing-like policy
   // WEBSOCKET            yes if the proxy act as a Reverse Proxy Web Sockets
   //
   // COMMAND              command to execute
   // ENVIRONMENT          environment for command to execute
   // RESPONSE_TYPE        output type of the command (yes = response for client, no = request to server)
   //
   // PORT                 port of server for connection
   // SERVER               name of server for connection
   //
   // FOLLOW_REDIRECTS     yes if     manage to automatically follow redirects from server
   // USER                     if     manage to follow redirects, in response to a HTTP_UNAUTHORISED response from the HTTP server: user
   // PASSWORD                 if     manage to follow redirects, in response to a HTTP_UNAUTHORISED response from the HTTP server: password
   // REPLACE_RESPONSE         if NOT manage to follow redirects, maybe vector of substitution string
   // -----------------------------------------------------------------------------------------------------------------------------------

   UVector<UString> tmp;

   if (cfg.loadVector(tmp, "ERROR MESSAGE"))
      {
      uint32_t n = tmp.size();

      if (n)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::vmsg_error, 0)

         U_NEW(UVector<UString>, UHTTP::vmsg_error, UVector<UString>(n));

         for (uint32_t i = 0; i < n; ++i)
            {
            UStringRep* r = tmp.UVector<UStringRep*>::at(i);

            UHTTP::vmsg_error->UVector<UStringRep*>::push(r);
            }

         tmp.clear();
         }
      }

   UString x;
   UModProxyService* service;

   U_NEW(UVector<UModProxyService*>, UHTTP::vservice, UVector<UModProxyService*>);

   while (cfg.searchForObjectStream())
      {
      U_NEW(UModProxyService, service, UModProxyService);

      (void) cfg.loadVector(service->vreplace_response, "REPLACE_RESPONSE");

      if (cfg.loadTable())
         {
         service->user      = cfg.at(U_CONSTANT_TO_PARAM("USER"));
         service->server    = cfg.at(U_CONSTANT_TO_PARAM("SERVER"));;
         service->password  = cfg.at(U_CONSTANT_TO_PARAM("PASSWORD"));
         service->host_mask = cfg.at(U_CONSTANT_TO_PARAM("HOST"));

         service->port             = cfg.readLong(   U_CONSTANT_TO_PARAM("PORT"), 80);
         service->websocket        = cfg.readBoolean(U_CONSTANT_TO_PARAM("WEBSOCKET"));
         service->request_cert     = cfg.readBoolean(U_CONSTANT_TO_PARAM("CLIENT_CERTIFICATE"));
         service->response_client  = cfg.readBoolean(U_CONSTANT_TO_PARAM("RESPONSE_TYPE"));
         service->follow_redirects = cfg.readBoolean(U_CONSTANT_TO_PARAM("FOLLOW_REDIRECTS"));

         x = cfg.at(U_CONSTANT_TO_PARAM("URI"));

         if (x)
            {
#        ifndef USE_LIBPCRE
            service->uri_mask = x;
#        else
            service->uri_mask.set(x, 0U);
            service->uri_mask.study();
#        endif
            }

         x = cfg.at(U_CONSTANT_TO_PARAM("METHOD_NAME"));

         if (x)
            {
            U_INTERNAL_ASSERT(x.isNullTerminated())

            const char* msk = x.data();

loop:       switch (u_get_unalignedp32(msk))
               {
               case U_MULTICHAR_CONSTANT32('G','E','T', 0):  service->method_mask |= HTTP_GET; break;
               case U_MULTICHAR_CONSTANT32('P','U','T', 0):  service->method_mask |= HTTP_PUT; break;

               case U_MULTICHAR_CONSTANT32('G','E','T','|'): service->method_mask |= HTTP_GET;                               msk +=  4; goto loop;
               case U_MULTICHAR_CONSTANT32('P','U','T','|'): service->method_mask |= HTTP_PUT;                               msk +=  4; goto loop;

               case U_MULTICHAR_CONSTANT32('H','E','A','D'): service->method_mask |= HTTP_HEAD;        if (msk[ 4] == '|') { msk +=  5; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('P','O','S','T'): service->method_mask |= HTTP_POST;        if (msk[ 4] == '|') { msk +=  5; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('D','E','L','E'): service->method_mask |= HTTP_DELETE;      if (msk[ 6] == '|') { msk +=  7; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('O','P','T','I'): service->method_mask |= HTTP_OPTIONS;     if (msk[ 7] == '|') { msk +=  8; goto loop; } break;

               case U_MULTICHAR_CONSTANT32('T','R','A','C'): service->method_mask |= HTTP_TRACE;       if (msk[ 5] == '|') { msk +=  6; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('C','O','N','N'): service->method_mask |= HTTP_CONNECT;     if (msk[ 7] == '|') { msk +=  8; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('C','O','P','Y'): service->method_mask |= HTTP_COPY;        if (msk[ 4] == '|') { msk +=  5; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('M','O','V','E'): service->method_mask |= HTTP_MOVE;        if (msk[ 4] == '|') { msk +=  5; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('L','O','C','K'): service->method_mask |= HTTP_LOCK;        if (msk[ 4] == '|') { msk +=  5; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('U','N','L','O'): service->method_mask |= HTTP_UNLOCK;      if (msk[ 6] == '|') { msk +=  7; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('M','K','C','O'): service->method_mask |= HTTP_MKCOL;       if (msk[ 5] == '|') { msk +=  6; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('S','E','A','R'): service->method_mask |= HTTP_SEARCH;      if (msk[ 6] == '|') { msk +=  7; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('P','R','O','P'): service->method_mask |= HTTP_PROPFIND;    if (msk[ 8] == '|') { msk +=  9; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('P','A','T','C'): service->method_mask |= HTTP_PATCH;       if (msk[ 5] == '|') { msk +=  6; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('P','U','R','G'): service->method_mask |= HTTP_PURGE;       if (msk[ 5] == '|') { msk +=  6; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('M','E','R','G'): service->method_mask |= HTTP_MERGE;       if (msk[ 5] == '|') { msk +=  6; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('R','E','P','O'): service->method_mask |= HTTP_REPORT;      if (msk[ 6] == '|') { msk +=  7; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('C','H','E','C'): service->method_mask |= HTTP_CHECKOUT;    if (msk[ 8] == '|') { msk +=  9; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('M','K','A','C'): service->method_mask |= HTTP_MKACTIVITY;  if (msk[10] == '|') { msk += 11; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('N','O','T','I'): service->method_mask |= HTTP_NOTIFY;      if (msk[ 6] == '|') { msk +=  7; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('M','S','E','A'): service->method_mask |= HTTP_MSEARCH;     if (msk[ 7] == '|') { msk +=  8; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('S','U','B','S'): service->method_mask |= HTTP_SUBSCRIBE;   if (msk[ 9] == '|') { msk += 10; goto loop; } break;
               case U_MULTICHAR_CONSTANT32('U','N','S','U'): service->method_mask |= HTTP_UNSUBSCRIBE; if (msk[11] == '|') { msk += 12; goto loop; } break;
               }
            }

         service->command = UServer_Base::loadConfigCommand();

         if (service->command) service->environment = service->command->getStringEnvironment();

         // REMOTE ADDRESS IP

         x = cfg.at(U_CONSTANT_TO_PARAM("REMOTE_ADDRESS_IP"));

         if (x)
            {
            U_NEW(UVector<UIPAllow*>, service->vremote_address, UVector<UIPAllow*>);

            if (UIPAllow::parseMask(x, *(service->vremote_address)) == 0)
               {
               delete service->vremote_address;
                      service->vremote_address = 0;
               }
            }

         UHTTP::vservice->push_back(service);

         cfg.clear();
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

__pure UModProxyService* UModProxyService::findService()
{
   U_TRACE_NO_PARAM(0, "UModProxyService::findService()")

   uint32_t sz;
   const char* ptr = UClientImage_Base::getRequestUri(sz);

   // ------------------------------------------------------------------------------
   // The difference between U_HTTP_HOST_.. and U_HTTP_VHOST_.. is that
   // U_HTTP_HOST_.. can include the :PORT text, and U_HTTP_VHOST_.. only the name
   // ------------------------------------------------------------------------------

   return findService(U_HTTP_HOST_TO_PARAM, ptr, sz);
}

__pure UModProxyService* UModProxyService::findService(const char* host, uint32_t host_len, const char* uri, uint32_t uri_len)
{
   U_TRACE(0, "UModProxyService::findService(%.*S,%u,%.*S,%u)", host_len, host, host_len, uri_len, uri, uri_len)

   if (UHTTP::vservice &&
       host_len <= 255)
      {
      for (uint32_t i = 0, n = UHTTP::vservice->size(); i < n; ++i)
         {
         UModProxyService* elem = (*UHTTP::vservice)[i];

         U_INTERNAL_DUMP("host_mask = %V method_mask = %B", elem->host_mask.rep, elem->method_mask)

         if ((elem->method_mask     == 0    || (U_http_method_type & elem->method_mask) != 0)                                                  &&
             (elem->vremote_address == 0    || UClientImage_Base::isAllowed(*(elem->vremote_address)))                                         &&
             (elem->host_mask.empty()       || (host_len && UServices::dosMatchWithOR(host, host_len, U_STRING_TO_PARAM(elem->host_mask), 0))) &&
#           ifdef USE_LIBPCRE
             (elem->uri_mask.getPcre() == 0 || elem->uri_mask.search(uri, uri_len)))
#           else
             (elem->uri_mask.empty()        || UServices::dosMatchWithOR(uri, uri_len, U_STRING_TO_PARAM(elem->uri_mask), 0)))
#           endif
            {
            U_RETURN_POINTER(elem, UModProxyService);
            }
         }
      }

   U_RETURN_POINTER(0, UModProxyService);
}

#define U_SRV_ADDR_FMT "%v/%.*s:%u.srv"

bool UModProxyService::setServerAddress(const UString& dir, const char* address, uint32_t address_len)
{
   U_TRACE(0, "UModProxyService::setServerAddress(%V,%.*S,%u)", dir.rep, address_len, address, address_len)

   UString dest(U_CAPACITY);

   dest.snprintf(U_CONSTANT_TO_PARAM(U_SRV_ADDR_FMT), dir.rep, U_CLIENT_ADDRESS_TO_TRACE, UHTTP::getUserAgent());

   if (UFile::writeTo(dest, address, address_len)) U_RETURN(true);

   U_RETURN(false);
}

UString UModProxyService::getServer() const
{
   U_TRACE_NO_PARAM(0, "UModProxyService::getServer()")

   const char* ptr = server.data();

   if (u_get_unalignedp16(ptr) == U_MULTICHAR_CONSTANT16('$','<'))
      {
      // NB: look for example at Service_GOOGLE_MAP for nodog...

      UString result(U_HTTP_VHOST_TO_PARAM);

      U_RETURN_STRING(result);
      }

   if (*ptr == '~')
      {
      UString dir = UStringExt::expandPath(server, 0), pathname(U_CAPACITY);

      U_INTERNAL_ASSERT(dir)

      pathname.snprintf(U_CONSTANT_TO_PARAM(U_SRV_ADDR_FMT), dir.rep, U_CLIENT_ADDRESS_TO_TRACE, UHTTP::getUserAgent());

      UString address = UFile::contentOf(pathname);

      if (address)
         {
         U_INTERNAL_ASSERT_EQUALS(address.findWhiteSpace(), U_NOT_FOUND)

         U_RETURN_STRING(address);
         }
      }

   U_RETURN_STRING(server);
}

UString UModProxyService::replaceResponse(const UString& msg)
{
   U_TRACE(0, "UModProxyService::replaceResponse(%V)", msg.rep)

   UString result = msg;

#ifdef USE_LIBPCRE
   for (int32_t i = 0, n = vreplace_response.size(); i < n; i += 2)
      {
      // Searches subject for matches to pattern and replaces them with replacement

      result = UStringExt::pregReplace(vreplace_response[i], vreplace_response[i+1], result);
      }
#endif

   U_RETURN_STRING(result);
}

void UModProxyService::setMsgError(int err)
{
   U_TRACE(0, "UModProxyService::setMsgError(%d)", err)

   /**
    * INTERNAL_ERROR = 1, // NB: we need to start from 1 because we use a vector...
    * BAD_REQUEST    = 2,
    * NOT_FOUND      = 3,
    * FORBIDDEN      = 4,
    * ....
    */

   U_INTERNAL_ASSERT_RANGE(1, err, UModProxyService::ERROR_A_X509_NOBASICAUTH)

   switch (err)
      {
      case UModProxyService::FORBIDDEN:      UHTTP::setForbidden();     break;
      case UModProxyService::NOT_FOUND:      UHTTP::setNotFound();      break;
      case UModProxyService::BAD_REQUEST:    UHTTP::setBadRequest();    break;
      case UModProxyService::INTERNAL_ERROR: UHTTP::setInternalError(); break;

      default:
         {
         if (UHTTP::vmsg_error)
            {
            UString fmt = (*UHTTP::vmsg_error)[err - UModProxyService::FORBIDDEN - 1];

            UClientImage_Base::wbuffer->snprintf(U_STRING_TO_PARAM(fmt));
            }
         }
      }
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UModProxyService::dump(bool reset) const
{
   U_CHECK_MEMORY

   *UObjectIO::os << "port                                 " << port                      << '\n'
                  << "websocket                            " << websocket                 << '\n'
                  << "method_mask                          " << method_mask               << '\n'
                  << "request_cert                         " << request_cert              << '\n'
                  << "response_client                      " << response_client           << '\n'
                  << "follow_redirects                     " << follow_redirects          << '\n'
#              ifdef USE_LIBPCRE
                  << "uri_mask          (UPCRE             " << (void*)&uri_mask          << ")\n"
#              else
                  << "uri_mask          (UString           " << (void*)&uri_mask          << ")\n"
#              endif
                  << "user              (UString           " << (void*)&user              << ")\n"
                  << "server            (UString           " << (void*)&server            << ")\n"
                  << "host_mask         (UString           " << (void*)&host_mask         << ")\n"
                  << "password          (UString           " << (void*)&password          << ")\n"
                  << "environment       (UString           " << (void*)&environment       << ")\n"
                  << "vremote_address   (UVector<UIPAllow> " << (void*)vremote_address    << ")\n"
                  << "vreplace_response (UVector<UString>  " << (void*)&vreplace_response << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
