// fortune.cpp - dynamic page translation (fortune.usp => fortune.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "fortune.h"
   
   static UOrmSession*       psql_fortune;
   static UOrmStatement*     pstmt_fortune;
   static Fortune*           pfortune;
   static UString*           pmessage;
   static UVector<Fortune*>* pvfortune;
   
   static void usp_init_fortune()
   {
      U_TRACE(5, "::usp_init_fortune()")
   
      pfortune  = U_NEW(Fortune);
      pvfortune = U_NEW(UVector<Fortune*>);
      pmessage  = U_NEW(U_STRING_FROM_CONSTANT("Additional fortune added at request time."));
   }
   
   static void usp_fork_fortune()
   {
      U_TRACE(5, "::usp_fork_fortune()")
   
      psql_fortune  = U_NEW(UOrmSession(U_CONSTANT_TO_PARAM("fortune")));
      pstmt_fortune = U_NEW(UOrmStatement(*psql_fortune, U_CONSTANT_TO_PARAM("SELECT id, message FROM Fortune")));
   
      if (pstmt_fortune == 0) U_ERROR("usp_fork_fortune(): we cound't connect to db");
   
      pstmt_fortune->into(*pfortune);
   }
   
   static void usp_end_fortune()
   {
      U_TRACE(5, "::usp_end_fortune()")
   
      delete pstmt_fortune;
      delete psql_fortune;
      delete pvfortune;
      delete pfortune;
      delete pmessage;
   }  
   
extern "C" {
extern U_EXPORT int runDynamicPage_fortune(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_fortune(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_fortune(%p)", client_image)
   
   
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
      if (client_image == (void*)-1) { usp_init_fortune(); U_RETURN(0); }
   
      if (client_image == (void*)-3) { usp_end_fortune(); U_RETURN(0); }
   
      if (client_image == (void*)-5) { usp_fork_fortune(); U_RETURN(0); }
   
      if (client_image >= (void*)-5) U_RETURN(0);
   
      UHTTP::mime_index = U_html;
   
      u_http_info.endHeader = 0;
      }
      
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<!doctype html><html><head><title>Fortunes</title></head><body><table><tr><th>id</th><th>message</th></tr>")
   );
   
   Fortune* elem;
   unsigned char encoded[1024];
   
   pstmt_fortune->execute();
   
        pvfortune->push_back(U_NEW(Fortune(0, *pmessage)));
   do { pvfortune->push_back(U_NEW(Fortune(*pfortune))); } while (pstmt_fortune->nextRow());
   
   pvfortune->sort(Fortune::cmp_obj);
   
   for (uint32_t i = 0, n = pvfortune->size(); i < n; ++i)
      {
      elem = (*pvfortune)[i];
   
      USP_PRINTF_ADD(
         "<tr>"
         "<td>%u</td>"
         "<td>%.*s</td>"
         "</tr>",
         elem->id, u_xml_encode((const unsigned char*)U_STRING_TO_PARAM(elem->message), encoded), encoded);
      }
   
   pvfortune->clear();
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</table></body></html>")
   );
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }