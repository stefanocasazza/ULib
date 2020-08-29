// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http3.h - HTTP/3 utility
//
// = AUTHOR
//    Stefano Casazza + Victor Stewart
//
// ============================================================================

#ifndef ULIB_HTTP3_H
#define ULIB_HTTP3_H 1

#include <ulib/utility/uquic.h>
#include <ulib/utility/http2.h>

/**
 * HTTP3 connection Information
 *
 * This class contains data about an HTTP3 connection
 */

class U_EXPORT UHTTP3 {
public:

   static int loadConfigParam();

protected:
   static uint32_t headers_len;
   static quiche_h3_conn* http3;
   static quiche_h3_header* headers;
   static UHashMap<UString>* itable; // headers request
   static quiche_h3_config* http3_config;

   // SERVICES

   static void ctor()
      {
      U_TRACE_NO_PARAM(0, "UHTTP3::ctor()")

#  ifdef U_HTTP2_DISABLE
      UHTTP2::buildTable();
#  endif

      U_NEW(UHashMap<UString>, itable, UHashMap<UString>(64, UHTTP2::setIndexStaticTable));
      }

   static void dtor()
      {
      U_TRACE_NO_PARAM(0, "UHTTP3::dtor()")

      if (itable) U_DELETE(itable)

      if (http3_config) U_SYSCALL_VOID(quiche_h3_config_free, "%p", http3_config);
      }

   static bool parseHeader();
   static bool handlerNewConnection();
   static void handlerRequest(quiche_conn* lconn, quiche_h3_conn* lh3);

   static int for_each_header(uint8_t* name, size_t name_len, uint8_t* value, size_t value_len, void* argp);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHTTP3)

   friend class UHTTP;
   friend class Application;
   friend class UHttpPlugIn;
   friend class UServer_Base;
   friend class UClientImage_Base;
};
#endif
