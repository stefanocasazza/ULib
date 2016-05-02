// mfortune.cpp - dynamic page translation (mfortune.usp => mfortune.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include "fortune.h"
   static UString* jquery;
   static UString* pencoded;
   static UMongoDBClient* mc;
   static Fortune* pfortune2add;
   static UVector<Fortune*>* pvfortune;
   static void usp_fork_mfortune()
   {
    U_TRACE(5, "::usp_fork_mfortune()")
    U_NEW(UMongoDBClient, mc, UMongoDBClient);
    if (mc->connect() == false)
     {
     U_WARNING("usp_fork_mfortune(): connection failed");
     return;
     }
    if (mc->selectCollection("hello_world", "Fortune") == false)
     {
     U_WARNING("usp_fork_mfortune(): selectCollection() failed");
     return;
     }
    U_NEW(UString, jquery, U_STRING_FROM_CONSTANT("{'message'"));
    U_NEW(UString, pencoded, UString(100U));
    U_NEW(UVector<Fortune*>, pvfortune, UVector<Fortune*>);
    U_NEW(Fortune, pfortune2add, Fortune(0, U_STRING_FROM_CONSTANT("Additional fortune added at request time.")));
   }
   static void usp_end_mfortune()
   {
    U_TRACE(5, "::usp_end_mfortune()")
    delete mc;
    if (jquery)
     {
     delete jquery;
     delete pencoded;
     delete pvfortune;
     delete pfortune2add;
     }
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_mfortune(int param);
       U_EXPORT void runDynamicPage_mfortune(int param)
{
   U_TRACE(0, "::runDynamicPage_mfortune(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_DESTROY) { usp_end_mfortune(); return; }
   
      if (param == U_DPAGE_FORK) { usp_fork_mfortune(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<!doctype html><html><head><title>Fortunes</title></head><body><table><tr><th>id</th><th>message</th></tr>")
   );
   
   Fortune* item;
   uint32_t i, n;
   UString result;
   U_NEW(Fortune, item, Fortune(*pfortune2add)));
   pvfortune->push_back(item);
   (void) mc->findAll();
   for (i = 0, n = mc->vitem.size(); i < n; ++i)
    {
    (void) UValue::jread(mc->vitem[i], *jquery, result); // { "_id" : 5.000000, "id" : 5.000000, "message" : "A computer program does what you tell it to do, not what you want it to do." }
      U_NEW(Fortune, item, Fortune(i+1, result)));
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