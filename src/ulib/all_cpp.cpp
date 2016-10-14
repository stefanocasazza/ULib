// all.cpp

#define U_ALL_CPP

#include <ulib/all.h>

#ifdef DEBUG
#  include "debug/trace.cpp"
#  include "debug/debug_common.cpp"
#  include "debug/error_memory.cpp"
#  include "debug/error_simulation.cpp"
#endif

#ifdef U_STDCPP_ENABLE
#  include "internal/objectIO.cpp"
#  ifdef DEBUG
#     include "debug/objectDB.cpp"
#  endif
#endif

#include "internal/error.cpp"
#include "internal/memory_pool.cpp"

#include "command.cpp"
#include "options.cpp"
#include "timeval.cpp"
#include "tokenizer.cpp"
#include "timer.cpp"
#include "notifier.cpp"
#include "string.cpp"
#include "file.cpp"
#include "process.cpp"
#include "file_config.cpp"
#include "log.cpp"
#include "application.cpp"
#include "cache.cpp"
#include "date.cpp"
#include "url.cpp"
#include "internal/common.cpp"
#include "ui/dialog.cpp"
#include "db/cdb.cpp"
#include "db/rdb.cpp"
#include "query/query_parser.cpp"
#include "mime/header.cpp"
#include "mime/entity.cpp"
#include "mime/multipart.cpp"
#include "container/vector.cpp"
#include "container/hash_map.cpp"
#include "container/tree.cpp"
#include "event/event_time.cpp"
#include "net/ping.cpp"
#include "net/socket.cpp"
#include "net/ipaddress.cpp"
#include "net/ipt_ACCOUNT.cpp"
#include "net/client/client.cpp"
#include "net/client/pop3.cpp"
#include "net/client/imap.cpp"
#include "net/client/smtp.cpp"
#include "net/client/ftp.cpp"
#include "net/client/http.cpp"
#include "net/client/redis.cpp"
#include "net/client/elasticsearch.cpp"
#include "net/rpc/rpc.cpp"
#include "net/rpc/rpc_client.cpp"
#include "net/rpc/rpc_encoder.cpp"
#include "net/rpc/rpc_envelope.cpp"
#include "net/rpc/rpc_fault.cpp"
#include "net/rpc/rpc_gen_method.cpp"
#include "net/rpc/rpc_method.cpp"
#include "net/rpc/rpc_object.cpp"
#include "net/rpc/rpc_parser.cpp"
#include "net/server/server.cpp"
#include "net/server/server_rdb.cpp"
#include "net/client/client_rdb.cpp"
#include "net/server/client_image.cpp"
#include "net/server/client_image_rdb.cpp"
#include "net/server/plugin/mod_proxy_service.cpp"
#include "orm/orm.cpp"
#include "orm/orm_driver.cpp"
#include "json/value.cpp"
#include "utility/lock.cpp"
#include "utility/uhttp.cpp"
#include "utility/base64.cpp"
#include "utility/dir_walk.cpp"
#include "utility/interrupt.cpp"
#include "utility/services.cpp"
#include "utility/semaphore.cpp"
#include "utility/websocket.cpp"
#include "utility/string_ext.cpp"
#include "utility/socket_ext.cpp"
#include "utility/ring_buffer.cpp"
#include "utility/data_session.cpp"
#include "lemon/expression.cpp"

#ifdef USE_LIBTDB
#  include "db/tdb.cpp"
#endif

#ifdef USE_MONGODB
#  include "net/client/mongodb.cpp"
#endif

#ifndef U_HTTP2_DISABLE
#  include "utility/http2.cpp"
#endif

#ifndef _MSWINDOWS_
#  include "net/unixsocket.cpp"
#endif

#ifdef ENABLE_ZIP
#  include "zip/zip.cpp"
#endif

#ifdef USE_LIBSSL
#  include "ssl/certificate.cpp"
#  include "ssl/pkcs7.cpp"
#  include "ssl/crl.cpp"
#  include "ssl/pkcs10.cpp"
#  include "net/client/twilio.cpp"
#  include "ssl/mime/mime_pkcs7.cpp"
#  include "ssl/net/sslsocket.cpp"
#  include "ssl/net/ssl_session.cpp"
#  include "utility/des3.cpp"
#  ifdef HAVE_SSL_TS
#     include "ssl/timestamp.cpp"
#  endif
#endif

#ifdef USE_LIBSSH
#  include "ssh/net/sshsocket.cpp"
#endif

#ifdef USE_LIBPCRE
#  include "pcre/pcre.cpp"
#endif

#ifdef USE_LIBEXPAT
#  include "xml/expat/attribute.cpp"
#  include "xml/expat/element.cpp"
#  include "xml/expat/xml_parser.cpp"
#  include "xml/expat/xml2txt.cpp"
#  include "xml/soap/soap_encoder.cpp"
#  include "xml/soap/soap_fault.cpp"
#  include "xml/soap/soap_gen_method.cpp"
#  include "xml/soap/soap_parser.cpp"
#  include "xml/soap/soap_client.cpp"
#endif

#ifdef USE_LIBXML2
#  include "xml/libxml2/node.cpp"
#  include "xml/libxml2/schema.cpp"
#  include "xml/libxml2/document.cpp"
#endif

#ifdef USE_LIBCURL
#  include "curl/curl.cpp"
#endif

#ifdef USE_PARSER
#  include "flex/flexer.cpp"
#  include "flex/bison.cpp"
#endif

#ifdef USE_LIBMAGIC
#  include "magic/magic.cpp"
#endif

#ifdef USE_LIBLDAP
#  include "ldap/ldap.cpp"
#endif

#ifdef USE_LIBDBI
#  include "dbi/dbi.cpp"
#endif

#ifdef USE_LIBEVENT
#  include "libevent/event.cpp"
#endif

#ifdef ENABLE_THREAD
#  include "thread.cpp"
#endif

// Handler static/dynamic plugin

#include "dynamic/dynamic.cpp"
#include "dynamic/plugin.cpp"

#ifdef U_STATIC_HANDLER_ECHO
#  include "net/server/plugin/mod_echo.cpp"
#endif

#ifdef U_STATIC_HANDLER_STREAM
#  include "net/server/plugin/mod_stream.cpp"
#endif

#ifdef U_STATIC_HANDLER_NOCAT
#  include "net/server/plugin/mod_nocat.cpp"
#endif

#ifdef U_STATIC_HANDLER_SOCKET
#  include "net/server/plugin/mod_socket.cpp"
#endif

#ifdef U_STATIC_HANDLER_SCGI
#  include "net/server/plugin/mod_scgi.cpp"
#endif

#ifdef U_STATIC_HANDLER_FCGI
#  include "net/server/plugin/mod_fcgi.cpp"
#endif

#ifdef U_STATIC_HANDLER_SHIB
#  include "net/server/plugin/mod_shib/mod_shib.cpp"
#endif

#if defined(HAVE_LIBGEOIP) && defined(U_STATIC_HANDLER_GEOIP)
#  include "net/server/plugin/mod_geoip/mod_geoip.cpp"
#endif

#if defined(USE_LIBPCRE) && defined(U_STATIC_HANDLER_PROXY)
#  include "net/server/plugin/mod_proxy.cpp"
#endif

#if defined(USE_LIBEXPAT) && defined(U_STATIC_HANDLER_SOAP)
#  include "net/server/plugin/mod_soap.cpp"
#endif

#ifdef U_STATIC_HANDLER_SSI
#  include "net/server/plugin/mod_ssi.cpp"
#endif

#ifdef U_STATIC_HANDLER_TSA
#  include "net/server/plugin/mod_tsa.cpp"
#endif

#ifdef U_STATIC_HANDLER_HTTP
#  include "net/server/plugin/mod_http.cpp"
#endif

#ifdef U_STATIC_HANDLER_RPC
#  include "net/server/plugin/mod_rpc.cpp"
#endif

// Handler static orm driver

#ifdef U_STATIC_ORM_DRIVER_SQLITE
#  include "orm/driver/orm_driver_sqlite.cpp"
#endif

#ifdef U_STATIC_ORM_DRIVER_MYSQL
#  include "orm/driver/orm_driver_mysql.cpp"
#endif

#ifdef U_STATIC_ORM_DRIVER_PGSQL
#  include "orm/driver/orm_driver_pgsql.cpp"
#endif

// Handler static http servlet

#ifdef U_STATIC_ONLY
#  ifdef U_STATIC_SERVLET
#     include "net/server/plugin/usp/loader.autoconf.inc"
#  endif
#  ifdef U_STATIC_SERVLET_WI_AUTH
#     include "../../examples/WiAuth/wi_auth.cpp"
#  endif
#endif
