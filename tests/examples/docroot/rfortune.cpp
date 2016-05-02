// rfortune.cpp - dynamic page translation (rfortune.usp => rfortune.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "fortune.h"
   static UString* pencoded;
   static UREDISClient_Base* rc;
   static Fortune* pfortune2add;
   static UVector<Fortune*>* pvfortune;
   static void usp_fork_rfortune()
   {
    U_TRACE(5, "::usp_fork_rfortune()")
    U_NEW(UREDISClient<UTCPSocket>, rc, UREDISClient<UTCPSocket>);
    if (rc->connect() == false)
     {
     U_WARNING("usp_fork_rfortune(): %V", rc->UClient_Base::getResponse().rep);
     return;
     }
    U_NEW(UString, pencoded, UString(100U));
    U_NEW(UVector<Fortune*>, pvfortune, UVector<Fortune*>);
    U_NEW(Fortune, pfortune2add, Fortune(0, U_STRING_FROM_CONSTANT("Additional fortune added at request time.")));
   }
   static void usp_end_rfortune()
   {
    U_TRACE(5, "::usp_end_rfortune()")
    delete rc;
    if (pencoded)
     {
     delete pencoded;
     delete pvfortune;
     delete pfortune2add;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_rfortune(int param);
       U_EXPORT void runDynamicPage_rfortune(int param)
{
   U_TRACE(0, "::runDynamicPage_rfortune(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_rfortune(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_rfortune(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<!doctype html><html><head><title>Fortunes</title></head><body><table><tr><th>id</th><th>message</th></tr>")
   );
   
   Fortune* item;
   uint32_t i, n;
   U_NEW(Fortune, item, Fortune(*pfortune2add)));
   pvfortune->push_back(item);
   (void) rc->lrange(U_CONSTANT_TO_PARAM("fortunes 0 -1"));
   for (i = 0, n = rc->vitem.size(); i < n; ++i)
    {
    U_NEW(Fortune, item, Fortune(i+1, rc->vitem[i])));
    pvfortune->push_back(item);
    }
   pvfortune->sort(Fortune::cmp_obj);
   for (i = 0, ++n; i < n; ++i)
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