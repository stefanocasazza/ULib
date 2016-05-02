// businesses.cpp - dynamic page translation (businesses.usp => businesses.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include <ulib/json/value.h>
   /**
    * {
    * "token": "A619828KAIJ6D3",
    * "type": "localesData",
    * "radius": "near",
    * "location": "40.7831 N, 73.9712 W"
    * }
    */
   class Request {
   public:
    // Check for memory error
    U_MEMORY_TEST
    // Allocator e Deallocator
    U_MEMORY_ALLOCATOR
    U_MEMORY_DEALLOCATOR
    UString token, type, radius, location;
    Request()
     {
     U_TRACE_REGISTER_OBJECT(5, Request, "")
     }
    Request(const Request& r) : token(r.token), type(r.type), radius(r.radius), location(r.location)
     {
     U_TRACE_REGISTER_OBJECT(5, Request, "%p", &r)
     U_MEMORY_TEST_COPY(r)
     }
    ~Request()
     {
     U_TRACE_UNREGISTER_OBJECT(5, Request)
     }
    const char* dump(bool breset) const
     {
     *UObjectIO::os << "token    (UString " << (void*)&token << ")\n"
          << "type     (UString " << (void*)&type << ")\n"
          << "radius   (UString " << (void*)&radius << ")\n"
          << "location (UString " << (void*)&location << ')';
     if (breset)
      {
      UObjectIO::output();
      return UObjectIO::buffer_output;
      }
     return 0;
     }
   private:
    Request& operator=(const Request&) { return *this; }
   };
   // JSON TEMPLATE SPECIALIZATIONS
   template <> class U_EXPORT UJsonTypeHandler<Request> : public UJsonTypeHandler_Base {
   public:
    explicit UJsonTypeHandler(Request& val) : UJsonTypeHandler_Base(&val) {}
    void toJSON(UValue& json)
     {
     U_TRACE(0, "UJsonTypeHandler<Request>::toJSON(%p)", &json)
     json.toJSON(U_JSON_TYPE_HANDLER(Request, token, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Request, type, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Request, radius, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Request, location, UString));
     }
    void fromJSON(UValue& json)
     {
     U_TRACE(0, "UJsonTypeHandler<Request>::fromJSON(%p)", &json)
     json.fromJSON(U_JSON_TYPE_HANDLER(Request, token, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Request, type, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Request, radius, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Request, location, UString));
     }
   };
   /**
    * [
    * { "name": "Business 1"
    *   "rating": "Red"
    *   "address": "123 park lane, New York, NY, USA 10028"
    *   "phone": "12126465788"
    *   "url": "www.business1.com" } ,
    *
    * ....
    *
    * { "name": "Business 20"
    *   "rating": "Yellow"
    *   "address": "837 mott street, New York, NY, USA 10019"
    *   "phone": "12124829384"
    *   "url": "www.business2.com" }
    * ]
    */
   class Response {
   public:
    // Check for memory error
    U_MEMORY_TEST
    // Allocator e Deallocator
    U_MEMORY_ALLOCATOR
    U_MEMORY_DEALLOCATOR
    UString name, rating, address, phone, url;
    Response()
     {
     U_TRACE_REGISTER_OBJECT(5, Response, "")
     }
    Response(const Response& r) : name(r.name), rating(r.rating), address(r.address), phone(r.phone), url(r.url)
     {
     U_TRACE_REGISTER_OBJECT(5, Response, "%p", &r)
     U_MEMORY_TEST_COPY(r)
     }
    ~Response()
     {
     U_TRACE_UNREGISTER_OBJECT(5, Response)
     }
    // SERVICES
    bool operator<(const Response& other) const { return cmp_obj(&name, &other.name); }
    static int cmp_obj(const void* a, const void* b)
     {
     U_TRACE(5, "Response::cmp_obj(%p,%p)", a, b)
     return (*(const Response**)a)->name.compare((*(const Response**)b)->name);
     }
    const char* dump(bool breset) const
     {
     *UObjectIO::os << "url     (UString " << (void*)&url << ")\n"
          << "name    (UString " << (void*)&name << ")\n"
          << "phone   (UString " << (void*)&phone << ")\n"
          << "rating  (UString " << (void*)&rating << ")\n"
          << "address (UString " << (void*)&address << ')';
     if (breset)
      {
      UObjectIO::output();
      return UObjectIO::buffer_output;
      }
     return 0;
     }
   private:
    Response& operator=(const Response&) { return *this; }
   };
   // JSON TEMPLATE SPECIALIZATIONS
   template <> class U_EXPORT UJsonTypeHandler<Response> : public UJsonTypeHandler_Base {
   public:
    explicit UJsonTypeHandler(Response& val) : UJsonTypeHandler_Base(&val) {}
    void toJSON(UValue& json)
     {
     U_TRACE(0, "UJsonTypeHandler<Response>::toJSON(%p)", &json)
     json.toJSON(U_JSON_TYPE_HANDLER(Response, name, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Response, rating, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Response, address, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Response, phone, UString));
     json.toJSON(U_JSON_TYPE_HANDLER(Response, url, UString));
     }
    void fromJSON(UValue& json)
     {
     U_TRACE(0, "UJsonTypeHandler<Response>::fromJSON(%p)", &json)
     json.fromJSON(U_JSON_TYPE_HANDLER(Response, name, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Response, rating, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Response, address, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Response, phone, UString));
     json.fromJSON(U_JSON_TYPE_HANDLER(Response, url, UString));
     }
   };
   static UValue* pvalue;
   static Request* prequest;
   static UMongoDBClient* mc;
   static Response* presponse;
   static UVector<Response*>* pvresponse;
   static void usp_fork_businesses()
   {
    U_TRACE(5, "::usp_fork_businesses()")
    U_NEW(UMongoDBClient, mc, UMongoDBClient);
    if (mc->connect() == false)
     {
     U_WARNING("usp_fork_businesses(): connection failed");
     return;
     }
    if (mc->selectCollection("database", "businesses") == false)
     {
     U_WARNING("usp_fork_businesses(): selectCollection() failed");
     return;
     }
    U_NEW(Request, prequest, Request);
    U_NEW(Response, presponse, Response);
    U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
    U_NEW(UVector<Response*>, pvresponse, UVector<Response*>);
   }
   static void usp_end_businesses()
   {
    U_TRACE(5, "::usp_end_businesses()")
    delete mc;
    if (pvalue)
     {
     delete pvalue;
     delete prequest;
     delete presponse;
     delete pvresponse;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_businesses(int param);
       U_EXPORT void runDynamicPage_businesses(int param)
{
   U_TRACE(0, "::runDynamicPage_businesses(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_businesses(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_businesses(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   if (JSON_parse(*UClientImage_Base::body, *prequest))
    {
    uint32_t i, n;
    // 1) identify user by TOKEN
    (void) mc->findAll(); // mc->find(prequest->token);
    for (i = 0, n = mc->vitem.size(); i < n; ++i)
     {
     JSON_parse(mc->vitem[i], *presponse);
     // 2) Filter by RADIUS around LOCATION
     if (true)
      {
      Response* nresponse;
      U_NEW(Response, nresponse, Response(*presponse));
      pvresponse->push_back(nresponse);
      }
     }
    n = pvresponse->size();
    // 3) sort list by ranking algorithm
    pvresponse->sort(Response::cmp_obj);
    // 4) return first 20 businesses
    if (n > 20) pvresponse->erase(20, n);
    }
   USP_JSON_stringify(*pvalue, UVector<Response*>, *pvresponse);
   pvalue->clear();
   pvresponse->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }