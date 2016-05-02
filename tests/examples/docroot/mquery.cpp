// mquery.cpp - dynamic page translation (mquery.usp => mquery.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static UString* jquery;
   static UMongoDBClient* mc;
   static World* pworld_query;
   static UValue* pvalue;
   static UVector<World*>* pvworld_query;
   static void usp_fork_mquery()
   {
    U_TRACE(5, "::usp_fork_mquery()")
      U_NEW(UMongoDBClient, mc, UMongoDBClient);
    if (mc->connect() == false)
     {
     U_WARNING("usp_fork_mquery(): connection disabled or failed");
     return;
     }
    if (mc->selectCollection("hello_world", "World") == false)
     {
     U_WARNING("usp_fork_mquery(): selectCollection() failed");
     return;
     }
    U_NEW(UString, jquery, U_STRING_FROM_CONSTANT("{'randomNumber'"));
    U_NEW(World, pworld_query, World);
    U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
    U_NEW(UVector<World*>, pvworld_query, UVector<World*>(500));
   }
   static void usp_end_mquery()
   {
    U_TRACE(5, "::usp_end_mquery()")
    delete mc;
    if (jquery)
     {
     delete jquery;
     delete pworld_query;
     delete pvalue;
     delete pvworld_query;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_mquery(int param);
       U_EXPORT void runDynamicPage_mquery(int param)
{
   U_TRACE(0, "::runDynamicPage_mquery(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_mquery(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_mquery(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   UString rnumber;
   int i = 0, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   while (true)
    {
    (void) mc->findOne(pworld_query->id = u_get_num_random(10000-1));
    (void) UValue::jread(mc->vitem[0], *jquery, rnumber); // { "_id" : 8980.000000, "id" : 8980.000000, "randomNumber" : 2131.000000 }
    pworld_query->randomNumber = u_strtoul(rnumber.data(), rnumber.end());
    World* pworld;
    U_NEW(World, pworld, World(*pworld_query)));
    pvworld_query->push_back(pworld);
    if (++i == num_queries) break;
    }
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld_query);
   pvalue->clear();
   pvworld_query->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }