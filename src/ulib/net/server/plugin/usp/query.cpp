// query.cpp - dynamic page translation (query.usp => query.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "world.h"
   
   #ifndef AS_cpoll_cppsp_DO
   static UValue* pvalue;
   #endif
   static UOrmSession*     psql_query;
   static UOrmStatement*   pstmt_query;
   static World*           pworld_query;
   static UVector<World*>* pvworld_query;
   
   static void usp_init_query()
   {
      U_TRACE(5, "::usp_init_query()")
   
      pworld_query  = U_NEW(World);
      pvworld_query = U_NEW(UVector<World*>(500));
   
   #ifndef AS_cpoll_cppsp_DO
      pvalue = U_NEW(UValue(ARRAY_VALUE));
   #endif
   }
   
   static void usp_fork_query()
   {
      U_TRACE(5, "::usp_fork_query()")
   
      psql_query  = U_NEW(UOrmSession(U_CONSTANT_TO_PARAM("hello_world")));
      pstmt_query = U_NEW(UOrmStatement(*psql_query, U_CONSTANT_TO_PARAM("SELECT randomNumber FROM World WHERE id = ?")));
   
      if (pstmt_query == 0) U_ERROR("usp_fork_query(): we cound't connect to db");
   
      pstmt_query->use( pworld_query->id);
      pstmt_query->into(pworld_query->randomNumber);
   }
   
   static void usp_end_query()
   {
      U_TRACE(5, "::usp_end_query()")
   
      delete pstmt_query;
      delete psql_query;
      delete pvworld_query;
      delete pworld_query;
   #ifndef AS_cpoll_cppsp_DO
      delete pvalue;
   #endif
   }  
   
extern "C" {
extern U_EXPORT int runDynamicPage_query(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_query(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_query(%p)", client_image)
   
   
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
      if (client_image == (void*)-1) { usp_init_query(); U_RETURN(0); }
   
      if (client_image == (void*)-3) { usp_end_query(); U_RETURN(0); }
   
      if (client_image == (void*)-5) { usp_fork_query(); U_RETURN(0); }
   
      if (client_image >= (void*)-5) U_RETURN(0);
   
      (void) UClientImage_Base::wbuffer->append(
         U_CONSTANT_TO_PARAM("Content-Type: application/json; charset=UTF-8\r\n\r\n"));
   
      u_http_info.endHeader = UClientImage_Base::wbuffer->size();
   
      (void) UHTTP::processForm();
      }
      
   UString queries = USP_FORM_VALUE(0);
   
   int i = 0, num_queries = queries.strtol();
   
        if (num_queries <   1) num_queries = 1;
   else if (num_queries > 500) num_queries = 500;
   
   #ifdef AS_cpoll_cppsp_DO
   USP_PUTS_CHAR('[');
   #endif
   
   while (true)
      {
      pworld_query->id = u_get_num_random(10000);
   
      pstmt_query->execute();
   
   #ifdef AS_cpoll_cppsp_DO
      USP_PRINTF("{\"id\":%u,\"randomNumber\":%u}", pworld_query->id, pworld_query->randomNumber);
   #endif
   
      pvworld_query->push_back(U_NEW(World(*pworld_query)));
   
      if (++i == num_queries) break;
   
   #ifdef AS_cpoll_cppsp_DO
      USP_PUTS_CHAR(',');
   #endif
      }
   
   #ifdef AS_cpoll_cppsp_DO
   USP_PUTS_CHAR(']');
   #else
   USP_JSON_stringify(*pvalue, UVector<World*>, *pvworld_query);
   #endif
   pvworld_query->clear();
   
      U_http_content_type_len = 1;
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }