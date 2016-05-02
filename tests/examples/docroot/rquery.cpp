// rquery.cpp - dynamic page translation (rquery.usp => rquery.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static UREDISClient_Base* rc;
   static UVector<World*>* pvworld_query;
   static UValue* pvalue;
   static void usp_fork_rquery()
   {
    U_TRACE(5, "::usp_fork_rquery()")
    U_NEW(UREDISClient<UTCPSocket>, rc, UREDISClient<UTCPSocket>);
    if (rc->connect() == false)
     {
     U_WARNING("usp_fork_rquery(): %V", rc->UClient_Base::getResponse().rep);
     return;
     }
    U_NEW(UVector<World*>, pvworld_query, UVector<World*>(500));
    U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
   }
   static void usp_end_rquery()
   {
    U_TRACE(5, "::usp_end_rquery()")
    delete rc;
    if (pvworld_query)
     {
     delete pvworld_query;
     delete pvalue;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_rquery(int param);
       U_EXPORT void runDynamicPage_rquery(int param)
{
   U_TRACE(0, "::runDynamicPage_rquery(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_rquery(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_rquery(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   World* pworld;
   UStringRep* rep;
   char* pbuffer = u_buffer;
   int i, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   for (i = 0; i < num_queries; ++i)
    {
    U_NEW(World, pworld, World);
    pvworld_query->push_back(pworld);
    u_put_unalignedp64(pbuffer, U_MULTICHAR_CONSTANT64(' ','w','o','r','l','d',':','\0'));
    pbuffer += 7+u_num2str32(pbuffer+7, pworld->id = u_get_num_random(10000-1));
    }
   (void) rc->mget(u_buffer, pbuffer-u_buffer);
   i = 0;
   while (true)
      {
    pworld = pvworld_query->at(i);
    rep = rc->vitem[i].rep;
    pworld->randomNumber = u_strtoul(rep->data(), rep->end());
      if (++i == num_queries) break;
    }
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld_query);
   pvalue->clear();
   pvworld_query->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }