// update.cpp - dynamic page translation (update.usp => update.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static World* pworld_update;
   static UOrmSession* psql_update;
   static UOrmStatement* pstmt1;
   static UOrmStatement* pstmt2;
   static UValue* pvalue;
   static UVector<World*>* pvworld_update;
   static void usp_fork_update()
   {
    U_TRACE(5, "::usp_fork_update()")
    U_NEW(UOrmSession, psql_update, UOrmSession(U_CONSTANT_TO_PARAM("hello_world")));
    if (psql_update->isReady())
     {
     U_NEW(UOrmStatement, pstmt1, UOrmStatement(*psql_update, U_CONSTANT_TO_PARAM("SELECT randomNumber FROM World WHERE id = ?")));
     U_NEW(UOrmStatement, pstmt2, UOrmStatement(*psql_update, U_CONSTANT_TO_PARAM("UPDATE World SET randomNumber = ? WHERE id = ?")));
     if (pstmt1 == 0 ||
       pstmt2 == 0)
      {
      U_WARNING("usp_fork_update(): we cound't connect to db");
      }
     if (UOrmDriver::isPGSQL()) *psql_update << "SET synchronous_commit TO OFF";
     U_NEW(World, pworld_update, World);
     pstmt1->use( pworld_update->id);
     pstmt1->into(pworld_update->randomNumber);
     pstmt2->use( pworld_update->randomNumber, pworld_update->id);
     U_NEW(UValue, pvalue, UValue(ARRAY_VALUE));
     U_NEW(UVector<World*>, pvworld_update, UVector<World*>(500));
     }
   }
   static void usp_end_update()
   {
    U_TRACE(5, "::usp_end_update()")
    if (pstmt1 &&
      pstmt2)
     {
     delete pstmt1;
     delete pstmt2;
     delete psql_update;
     delete pworld_update;
     delete pvalue;
     delete pvworld_update;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_update(int param);
       U_EXPORT void runDynamicPage_update(int param)
{
   U_TRACE(0, "::runDynamicPage_update(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_update(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_update(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   int i = 0, num_queries = UHTTP::getFormFirstNumericValue(1, 500);
   while (true)
    {
    pworld_update->id = u_get_num_random(10000-1);
    pstmt1->execute();
    pworld_update->randomNumber = u_get_num_random(10000-1);
    pstmt2->execute();
    World* pworld;
    U_NEW(World, pworld, World(*pworld_update)));
    pvworld_update->push_back(pworld)));
    if (++i == num_queries) break;
    }
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld_update);
   pvalue->clear();
   pvworld_update->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }