// mupdate.cpp - dynamic page translation (mupdate.usp => mupdate.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static UMongoDBClient* mc;
   static World* pworld_update;
   static UValue* pvalue;
   static UVector<World*>* pvworld_update;
   static void usp_fork_mupdate()
   {
    U_TRACE(5, "::usp_fork_mupdate()")
      U_NEW(UMongoDBClient, mc, UMongoDBClient);
    if (mc->connect() == false)
     {
     U_WARNING("usp_fork_mupdate(): connection disabled or failed");
     return;
     }
    if (mc->selectCollection("hello_world", "World") == false)
     {
     U_WARNING("usp_fork_mupdate(): selectCollection() failed");
     return;
     }
    U_NEW(World, pworld_update, World);
    U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
    U_NEW(UVector<World*>, pvworld_update, UVector<World*>(500));
   }
   static void usp_end_mupdate()
   {
    U_TRACE(5, "::usp_end_mupdate()")
    delete mc;
    if (pworld_update)
     {
     delete pworld_update;
     delete pvalue;
     delete pvworld_update;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_mupdate(int param);
       U_EXPORT void runDynamicPage_mupdate(int param)
{
   U_TRACE(0, "::runDynamicPage_mupdate(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_mupdate(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_mupdate(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   int i = 0, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   while (true)
    {
    (void) mc->findOne(pworld_update->id = u_get_num_random(10000-1));
    (void) mc->update(pworld_update->id, "randomNumber", pworld_update->randomNumber = u_get_num_random(10000-1));
    World* pworld;
    U_NEW(World, pworld, World(*pworld_update)));
    pvworld_update->push_back(pworld);
      if (++i == num_queries) break;
    }
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld_update);
   pvalue->clear();
   pvworld_update->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }