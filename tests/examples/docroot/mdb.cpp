// mdb.cpp - dynamic page translation (mdb.usp => mdb.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static World* pworld;
   static UString* jquery;
   static UMongoDBClient* mc;
   static UValue* pvalue;
   static void usp_fork_mdb()
   {
    U_TRACE(5, "::usp_fork_mdb()")
    U_NEW(UMongoDBClient, mc, UMongoDBClient);
    if (mc->connect() == false)
     {
     U_WARNING("usp_fork_mdb(): connection failed");
     return;
     }
    if (mc->selectCollection("hello_world", "World") == false)
     {
     U_WARNING("usp_fork_mdb(): selectCollection() failed");
     return;
     }
    U_NEW(World, pworld, World);
    U_NEW(UString, jquery, U_STRING_FROM_CONSTANT("{'randomNumber'"));
    U_NEW(UValue, pvalue, UValue(OBJECT_VALUE));
   }
   static void usp_end_mdb()
   {
    U_TRACE(5, "::usp_end_mdb()")
    delete mc;
    if (pworld)
     {
     delete pworld;
     delete jquery;
     delete pvalue;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_mdb(int param);
       U_EXPORT void runDynamicPage_mdb(int param)
{
   U_TRACE(0, "::runDynamicPage_mdb(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_mdb(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_mdb(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   UString result;
   (void) mc->findOne(pworld->id = u_get_num_random(10000-1));
   (void) UValue::jread(mc->vitem[0], *jquery, result); // { "_id" : 8980.000000, "id" : 8980.000000, "randomNumber" : 2131.000000 }
   pworld->randomNumber = u_strtoul(result.data(), result.end());
   USP_JSON_stringify(*pvalue, World, *pworld);
   pvalue->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }