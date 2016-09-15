// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_ssi.h - Server Side Includes (SSI)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_SSI_H
#define U_MOD_SSI_H 1

#include <ulib/net/server/server_plugin.h>

class U_EXPORT USSIPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            USSIPlugIn();
   virtual ~USSIPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // SERVICES

   static UString* environment;
   static int alternative_response;
   static UString* alternative_include;

   static void setMessagePage(const UString& tmpl, const char* message)
      {
      U_TRACE(0, "USSIPlugIn::setMessagePage(%V,%S)", tmpl.rep, message)

      setAlternativeInclude(tmpl, 1024, false, "Service not available", 0, 0,
                            message); // NB: vararg...
      }

   static void setMessagePage(const UString& tmpl, const char* title_txt, const char* message)
      {
      U_TRACE(0, "USSIPlugIn::setMessagePage(%V,%S,%S)", tmpl.rep, title_txt, message)

      setAlternativeInclude(tmpl, 1024, false, title_txt, 0, 0,
                            title_txt, message); // NB: vararg...
      }

   static void setMessagePageWithVar(const UString& tmpl, const char* title_txt, const char* fmt, uint32_t fmt_size, ...);

   static void setBadRequest();
   static void setAlternativeResponse();
   static void setAlternativeResponse(UString& body);
   static void setAlternativeRedirect(const char* fmt, ...);
   static void setAlternativeInclude(const char* title_txt, const UString& output);
   static void setAlternativeInclude(const UString& tmpl, uint32_t estimated_size, bool bprocess, const char* title_txt, const char* ssi_head, const char* body_style, ...);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static UString* body;
   static UString* header;
   static UString* errmsg;
   static UString* timefmt;
   static UString* docname;
   static bool use_size_abbrev;
   static time_t last_modified;

private:
   static UString processSSIRequest(const UString& content, int include_level) U_NO_EXPORT;
   static UString getInclude(const UString& include, int include_level, bool bssi) U_NO_EXPORT;

   static bool    callService(const UString& name, const UString& value) U_NO_EXPORT;
   static UString getPathname(const UString& name, const UString& value, const UString& directory) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(USSIPlugIn)
};

#endif
