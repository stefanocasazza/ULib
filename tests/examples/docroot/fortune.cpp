// fortune.cpp - dynamic page translation (fortune.usp => fortune.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "fortune.h"
   static Fortune* pfortune;
   static Fortune* pfortune2add;
   static UString* pencoded;
   static UOrmSession* psql_fortune;
   static UOrmStatement* pstmt_fortune;
   static UVector<Fortune*>* pvfortune;
   static void usp_fork_fortune()
   {
    U_TRACE(5, "::usp_fork_fortune()")
    U_NEW(UOrmSession, psql_fortune, UOrmSession(U_CONSTANT_TO_PARAM("fortune")));
    if (psql_fortune->isReady())
     {
     U_NEW(UOrmStatement, pstmt_fortune, UOrmStatement(*psql_fortune, U_CONSTANT_TO_PARAM("SELECT id, message FROM Fortune")));
     if (pstmt_fortune == 0) U_WARNING("usp_fork_fortune(): we cound't connect to db");
     if (UOrmDriver::isPGSQL()) *psql_fortune << "BEGIN ISOLATION LEVEL SERIALIZABLE; COMMIT";
     U_NEW(Fortune, pfortune, Fortune);
     pstmt_fortune->into(*pfortune);
     U_NEW(UString, pencoded, UString(100U));
     U_NEW(UVector<Fortune*>, pvfortune, UVector<Fortune*>);
     U_NEW(Fortune, pfortune2add, Fortune(0, U_STRING_FROM_CONSTANT("Additional fortune added at request time.")));
     }
   }
   static void usp_end_fortune()
   {
    U_TRACE(5, "::usp_end_fortune()")
    if (pstmt_fortune)
     {
     delete pstmt_fortune;
     delete pencoded;
     delete psql_fortune;
     delete pvfortune;
     delete pfortune;
     delete pfortune2add;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_fortune(int param);
       U_EXPORT void runDynamicPage_fortune(int param)
{
   U_TRACE(0, "::runDynamicPage_fortune(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_fortune(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_fortune(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<!doctype html><html><head><title>Fortunes</title></head><body><table><tr><th>id</th><th>message</th></tr>")
   );
   
   Fortune* item;
   U_NEW(Fortune, item, Fortune(*pfortune2add)));
   pvfortune->push_back(item);
   pstmt_fortune->execute();
   do {
    U_NEW(Fortune, item, Fortune(*pfortune)));
      pvfortune->push_back(item);
    }
   while (pstmt_fortune->nextRow());
   pvfortune->sort(Fortune::cmp_obj);
   for (uint32_t i = 0, n = pvfortune->size(); i < n; ++i)
    {
    Fortune* elem = (*pvfortune)[i];
    UXMLEscape::encode(elem->message, *pencoded);
    USP_PRINTF_ADD(
     "<tr>"
     "<td>%u</td>"
     "<td>%v</td>"
     "</tr>",
     elem->id, pencoded->rep);
    }
   pvfortune->clear();
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</table></body></html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }