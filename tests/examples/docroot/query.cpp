// query.cpp - dynamic page translation (query.usp => query.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static World* pworld_query;
   static UOrmSession* psql_query;
   static UOrmStatement* pstmt_query;
   static UValue* pvalue;
   static UVector<World*>* pvworld_query;
   static void usp_fork_query()
   {
    U_TRACE(5, "::usp_fork_query()")
    U_NEW(UOrmSession, psql_query, UOrmSession(U_CONSTANT_TO_PARAM("hello_world")));
    if (psql_query->isReady())
     {
     U_NEW(UOrmStatement, pstmt_query, UOrmStatement(*psql_query, U_CONSTANT_TO_PARAM("SELECT randomNumber FROM World WHERE id = ?")));
     if (pstmt_query == 0) U_WARNING("usp_fork_query(): we cound't connect to db");
     if (UOrmDriver::isPGSQL()) *psql_query << "BEGIN TRANSACTION";
     U_NEW(World, pworld_query, World);
     pstmt_query->use( pworld_query->id);
     pstmt_query->into(pworld_query->randomNumber);
     U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
     U_NEW(UVector<World*>, pvworld_query, UVector<World*>(500));
     }
   }
   static void usp_end_query()
   {
    U_TRACE(5, "::usp_end_query()")
    if (pstmt_query)
     {
     delete pstmt_query;
     delete psql_query;
     delete pworld_query;
     delete pvalue;
     delete pvworld_query;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_query(int param);
       U_EXPORT void runDynamicPage_query(int param)
{
   U_TRACE(0, "::runDynamicPage_query(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_query(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_query(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   int i = 0, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   while (true)
    {
    pworld_query->id = u_get_num_random(10000-1);
    pstmt_query->execute();
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