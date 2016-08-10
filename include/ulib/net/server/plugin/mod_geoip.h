// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_geoip.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_GEOIP_H
#define U_MOD_GEOIP_H 1

#include <ulib/net/server/server_plugin.h>

#include <GeoIP.h>
#include <GeoIPCity.h>

class U_EXPORT UGeoIPPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            UGeoIPPlugIn();
   virtual ~UGeoIPPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerREAD() U_DECL_FINAL;
   virtual int handlerRequest() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static uint32_t ipnum;
   static const char* org;
   static GeoIPRecord* gir;
   static char* domain_name;
   static GeoIPRegion* region;
   static GeoIP* gi[NUM_DB_TYPES];
   static const char* country_code;
   static const char* country_name;
   static int netspeed, country_id;
   static bool bGEOIP_CITY_EDITION_REV1;
   static UString* country_forbidden_mask;

   static bool setCountryCode();
   static bool checkCountryForbidden();

private:
   U_DISALLOW_COPY_AND_ASSIGN(UGeoIPPlugIn)
};

#endif
