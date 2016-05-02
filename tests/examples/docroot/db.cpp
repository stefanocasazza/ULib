// db.cpp - dynamic page translation (db.usp => db.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   static World* pworld_db;
   static UOrmSession* psql_db;
   static UOrmStatement* pstmt_db;
   static UValue* pvalue;
   static void usp_fork_db()
   {
    U_TRACE(5, "::usp_fork_db()")
    U_NEW(UOrmSession, psql_db, UOrmSession(U_CONSTANT_TO_PARAM("hello_world")));
    if (psql_db->isReady())
     {
     U_NEW(UOrmStatement, pstmt_db, UOrmStatement(*psql_db, U_CONSTANT_TO_PARAM("SELECT randomNumber FROM World WHERE id = ?")));
     if (pstmt_db == 0) U_WARNING("usp_fork_db(): we cound't connect to db");
     U_NEW(World, pworld_db, World);
     pstmt_db->use( pworld_db->id);
     pstmt_db->into(pworld_db->randomNumber);
     U_NEW(UValue, pvalue, UValue(OBJECT_VALUE));
     }
   }
   static void usp_end_db()
   {
    U_TRACE(5, "::usp_end_db()")
    if (pstmt_db)
     {
     delete pstmt_db;
     delete psql_db;
     delete pworld_db;
     delete pvalue;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_db(int param);
       U_EXPORT void runDynamicPage_db(int param)
{
   U_TRACE(0, "::runDynamicPage_db(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_db(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_db(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   pworld_db->id = u_get_num_random(10000-1);
   pstmt_db->execute();
   USP_JSON_stringify(*pvalue, World, *pworld_db);
   pvalue->clear();
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }