// rdb.cpp - dynamic page translation (rdb.usp => rdb.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static World* pworld;
   static UREDISClient_Base* rc;
   static UValue* pvalue;
   static void usp_fork_rdb()
   {
    U_TRACE(5, "::usp_fork_rdb()")
    U_NEW(UREDISClient<UTCPSocket>, rc, UREDISClient<UTCPSocket>);
    if (rc->connect() == false)
     {
     U_WARNING("usp_fork_rdb(): %V", rc->UClient_Base::getResponse().rep);
     return;
     }
    U_NEW(World, pworld, World);
    U_NEW(UValue, pvalue, UValue(OBJECT_VALUE));
    u__memcpy(u_buffer, "world:", U_CONSTANT_SIZE("world:"), __PRETTY_FUNCTION__);
   }
   static void usp_end_rdb()
   {
    U_TRACE(5, "::usp_end_rdb()")
    delete rc;
    if (pworld)
     {
     delete pworld;
     delete pvalue;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_rdb(int param);
       U_EXPORT void runDynamicPage_rdb(int param)
{
   U_TRACE(0, "::runDynamicPage_rdb(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_rdb(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_rdb(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   UStringRep* rep;
   (void) rc->get(u_buffer, 6+u_num2str32(u_buffer+6, pworld->id = u_get_num_random(10000-1)));
   rep = rc->vitem[0].rep;
   pworld->randomNumber = u_strtoul(rep->data(), rep->end());
   USP_JSON_stringify(*pvalue, World, *pworld);
   pvalue->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }