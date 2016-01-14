// mquery.cpp - dynamic page translation (mquery.usp => mquery.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static UREDISClient_Base* rc;
   static UVector<World*>* pvworld;
   static UValue* pvalue;
   static void usp_fork_rquery()
   {
    U_TRACE(5, "::usp_fork_rquery()")
    rc = U_NEW(UREDISClient<UTCPSocket>);
    if (rc->connect() == false)
     {
     U_WARNING("usp_fork_rquery(): %V", rc->UClient_Base::getResponse().rep);
     return;
     }
    pvworld = U_NEW(UVector<World*>(500));
    pvalue = U_NEW(UValue(ARRAY_VALUE));
   }
   static void usp_end_rquery()
   {
    U_TRACE(5, "::usp_end_rquery()")
    delete rc;
    if (pvworld)
     {
     delete pvworld;
     delete pvalue;
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
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString queries = USP_FORM_VALUE(0);
   
   World* pworld;
   UStringRep* rep;
   int i, num_queries;
   char* pbuffer = u_buffer;
   const char* ptr = queries.data();
   if (u__isdigit(*ptr) == false) num_queries = 1;
   else
    {
    num_queries = u_strtoul(ptr, queries.end());
       if (num_queries < 1) num_queries = 1;
    else if (num_queries > 500) num_queries = 500;
    }
   for (i = 0; i < num_queries; ++i)
    {
    pvworld->push_back(pworld = U_NEW(World));
    u_put_unalignedp64(pbuffer, U_MULTICHAR_CONSTANT64(' ','w','o','r','l','d',':','\0'));
    pbuffer += 7+u_num2str32(pbuffer+7, pworld->id = u_get_num_random(10000));
    }
   (void) rc->mget(u_buffer, pbuffer-u_buffer);
   i = 0;
   while (true)
      {
    pworld = pvworld->at(i);
    rep = rc->vitem[i].rep;
    pworld->randomNumber = u_strtoul(rep->data(), rep->end());
      if (++i == num_queries) break;
    }
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld);
   pvalue->clear();
   pvworld->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }