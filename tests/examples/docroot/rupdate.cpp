// rupdate.cpp - dynamic page translation (rupdate.usp => rupdate.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static UREDISClient_Base* rc;
   static UVector<World*>* pvworld;
   static UValue* pvalue;
   static void usp_fork_rupdate()
   {
    U_TRACE(5, "::usp_fork_rupdate()")
    U_NEW(UREDISClient<UTCPSocket>, rc, UREDISClient<UTCPSocket>);
    if (rc->connect() == false)
     {
     U_WARNING("usp_fork_rupdate(): %V", rc->UClient_Base::getResponse().rep);
     return;
     }
    U_NEW(UVector<World*>, pvworld, UVector<World*>(500));
    U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
   }
   static void usp_end_rupdate()
   {
    U_TRACE(5, "::usp_end_rupdate()")
    delete rc;
    if (pvworld)
     {
     delete pvworld;
     delete pvalue;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_rupdate(int param);
       U_EXPORT void runDynamicPage_rupdate(int param)
{
   U_TRACE(0, "::runDynamicPage_rupdate(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_rupdate(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_rupdate(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   World* pworld;
   char* pbuffer = u_buffer;
   int i, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   for (i = 0; i < num_queries; ++i)
    {
    U_NEW(World, pworld, World));
    pvworld->push_back(pworld);
    u_put_unalignedp64(pbuffer, U_MULTICHAR_CONSTANT64(' ','w','o','r','l','d',':','\0'));
    pbuffer += 7+u_num2str32(pbuffer+7, pworld->id = u_get_num_random(10000-1));
    }
   (void) rc->mget(u_buffer, pbuffer-u_buffer);
   i = 0;
   pbuffer = u_buffer;
   while (true)
      {
    pworld = pvworld->at(i);
    u_put_unalignedp64(pbuffer, U_MULTICHAR_CONSTANT64(' ','w','o','r','l','d',':','\0'));
    pbuffer += 7+u_num2str32(pbuffer+7, pworld->id);
     *pbuffer = ' ';
    pbuffer += 1+u_num2str32(pbuffer+1, pworld->randomNumber = u_get_num_random(10000-1));
      if (++i == num_queries) break;
    }
   (void) rc->mset(u_buffer, pbuffer-u_buffer);
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld);
   pvalue->clear();
   pvworld->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }