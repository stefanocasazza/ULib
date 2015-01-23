// db.cpp - dynamic page translation (db.usp => db.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   
   #ifndef AS_cpoll_cppsp_DO
   static UValue* pvalue;
   #endif
   static World*         pworld_db;
   static UOrmSession*   psql_db;
   static UOrmStatement* pstmt_db;
   
   static void usp_init_db()
   {
      U_TRACE(5, "::usp_init_db()")
   
      pworld_db = U_NEW(World);
   
   #ifndef AS_cpoll_cppsp_DO
      pvalue = U_NEW(UValue(OBJECT_VALUE));
   #endif
   }
   
   static void usp_fork_db()
   {
      U_TRACE(5, "::usp_fork_db()")
   
      psql_db  = U_NEW(UOrmSession(U_CONSTANT_TO_PARAM("hello_world")));
      pstmt_db = U_NEW(UOrmStatement(*psql_db, U_CONSTANT_TO_PARAM("SELECT randomNumber FROM World WHERE id = ?")));
   
      if (pstmt_db == 0) U_ERROR("usp_fork_db(): we cound't connect to db");
   
      pstmt_db->use( pworld_db->id);
      pstmt_db->into(pworld_db->randomNumber);
   }
   
   static void usp_end_db()
   {
      U_TRACE(5, "::usp_end_db()")
   
      delete pstmt_db;
      delete psql_db;
      delete pworld_db;
   #ifndef AS_cpoll_cppsp_DO
      delete pvalue;
   #endif
   }  
   
extern "C" {
extern U_EXPORT int runDynamicPage_db(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_db(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_db(%p)", client_image)
   
   
   // ------------------------------
   // special argument value:
   // ------------------------------
   //  0 -> call it as service
   // -1 -> init
   // -2 -> reset
   // -3 -> destroy
   // -4 -> call it for sigHUP
   // -5 -> call it after fork
   // ------------------------------
   
   if (client_image)
      {
      if (client_image == (void*)-1) { usp_init_db(); U_RETURN(0); }
   
      if (client_image == (void*)-3) { usp_end_db(); U_RETURN(0); }
   
      if (client_image == (void*)-5) { usp_fork_db(); U_RETURN(0); }
   
      if (client_image >= (void*)-5) U_RETURN(0);
   
      (void) UClientImage_Base::wbuffer->append(
         U_CONSTANT_TO_PARAM("Content-Type: application/json; charset=UTF-8\r\n\r\n"));
   
      u_http_info.endHeader = UClientImage_Base::wbuffer->size();
      }
      
   pworld_db->id = u_get_num_random(10000);
   
   pstmt_db->execute();
   
   #ifdef AS_cpoll_cppsp_DO
   USP_PRINTF_ADD("{\"id\":%u,\"randomNumber\":%u}", pworld_db->id, pworld_db->randomNumber);
   #else
   USP_JSON_stringify(*pvalue, World, *pworld_db);
   #endif
   
      U_http_content_type_len = 1;
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }