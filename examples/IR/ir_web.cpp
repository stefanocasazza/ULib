// ir_web.cpp - dynamic page translation (ir_web.usp => ir_web.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
#include "ir_session.h"
#include <ulib/debug/crono.h>
#define IR_SESSION (*(IRDataSession*)UHTTP::data_session)
static IR* ir;
static Query* query;
static UCrono* crono;
static UString* footer;
static void usp_init_ir_web()
{
   U_TRACE(5, "::usp_init_ir_web()")
   U_INTERNAL_ASSERT_EQUALS(ir, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(query, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(crono, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(footer, U_NULLPTR)
   U_NEW(IR, ir, IR);
   U_NEW(Query, query, Query);
   U_NEW_WITHOUT_CHECK_MEMORY(UCrono, crono, UCrono);
   U_NEW_STRING(footer, UString(200U));
   ir->loadFileConfig();
   if (ir->openCDB(false) == false)
      {
      U_ERROR("usp_init() of servlet ir_web failed");
      }
   footer->snprintf(U_CONSTANT_TO_PARAM("ver. %.*s, with %u documents and %u words."), U_CONSTANT_TO_TRACE(ULIB_VERSION), cdb_names->size(), cdb_words->size());
   U_NEW(IRDataSession, UHTTP::data_session, IRDataSession);
   UHTTP::initSession();
}
static void usp_end_ir_web()
{
   U_TRACE(5, "::usp_end_ir_web()")
   U_INTERNAL_ASSERT_POINTER(ir)
   U_INTERNAL_ASSERT_POINTER(query)
   U_INTERNAL_ASSERT_POINTER(crono)
   U_INTERNAL_ASSERT_POINTER(footer)
   U_DELETE(ir)
   U_DELETE(query)
   U_DELETE(crono)
   U_DELETE(footer)
}
   
static bool usp_bSESSION;
   
#define USP_SESSION_VAR_GET(index,varname) \
   { \
   UString varname##_value; \
   if (UHTTP::getDataSession(index, varname##_value) && \
       (usp_sz = varname##_value.size())) \
      { \
      UString2Object(varname##_value.data(), usp_sz, varname); \
      } \
   U_INTERNAL_DUMP("%s(%u) = %V", #varname, usp_sz, varname##_value.rep) \
   }
   
#define USP_SESSION_VAR_PUT(index,varname) \
   { \
   usp_sz = UObject2String(varname, usp_buffer, sizeof(usp_buffer)); \
   if (usp_sz) \
      { \
      UString varname##_value((void*)usp_buffer, usp_sz); \
      UHTTP::data_session->putValueVar(index, varname##_value); \
      U_INTERNAL_DUMP("%s(%u) = %V", #varname, usp_sz, varname##_value.rep) \
      } \
   else \
      { \
      UHTTP::data_session->putValueVar(index, UString::getStringNull()); \
      } \
   }
   
static void usp_body_ir_web()
{
   U_TRACE(5, "::usp_body_ir_web()")
   
   usp_bSESSION = (UHTTP::getDataSession() ? true : (UHTTP::setSessionCookie(), false));
   if (usp_bSESSION) {
   UHTTP::putDataSESSION();
   }
   usp_bSESSION = true;
   const char* ref     = "?ext=help";
   uint32_t num_args   = UHTTP::processForm() / 2;
   bool form_with_help = false;
   
   if (UHTTP::isGET())
      {
      if (num_args == 1)
         {
         UString ext_val = USP_FORM_VALUE_FROM_NAME("ext");
   
         if (ext_val.equal(U_CONSTANT_TO_PARAM("help")))
            {
            ref            = "ir_web";
            form_with_help = true;
            }
         else
            {
            UHTTP::num_item_tot = IR_SESSION.size();
            UHTTP::num_page_cur = USP_FORM_VALUE(0).strtol();
            }
         }
      }
   else if (UHTTP::isPOST())
      {
      IR_SESSION.query = USP_FORM_VALUE(0);
   
      if (num_args != 2 ||
          IR_SESSION.query.empty())
         {
         UHTTP::setBadRequest();
   
         return;
         }
   
      IR_SESSION.for_page = UHTTP::num_item_for_page = USP_FORM_VALUE(1).strtol();
   
      query->clear();
   
      crono->start();
   
      query->run(U_STRING_TO_PARAM(IR_SESSION.query), &IR_SESSION.vec);
   
      crono->stop();
   
      if ((UHTTP::num_item_tot = WeightWord::size()))
         {
         UHTTP::num_page_start = 1;
         UHTTP::num_page_end   = UHTTP::num_item_for_page;
   
         WeightWord::sortObjects();
         }
   
      WeightWord::vec     = U_NULLPTR;
      IR_SESSION.timerun  = UStringExt::numberToString(crono->getTimeElapsedInSecond());
      UHTTP::num_page_cur = 1;
      }
   else
      {
      UHTTP::setBadMethod();
   
      return;
      }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n<head>\n  <title>ULib search engine: a full-text search system for communities</title>\n  <link title=\"Services\" rel=\"stylesheet\" href=\"/css/pagination.min.css\" type=\"text/css\">\n</head>\n<body>\n  <div id=\"estform\" class=\"estform\">\n    <form action=\"ir_web\" method=\"post\" id=\"form_self\" name=\"form_self\">\n\n      <div class=\"form_navi\">\n        <a href=\"")
   );
   if (ref) (void) UClientImage_Base::wbuffer->append((ref));
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\" class=\"navilink\">help</a>\n      </div>\n\n      <div class=\"form_basic\">\n        <input type=\"text\" name=\"phrase\" value=\"\" size=\"80\" id=\"phrase\" class=\"text\" tabindex=\"1\" accesskey=\"0\">\n        <input type=\"submit\" value=\"Search\" id=\"search\" class=\"submit\" tabindex=\"2\" accesskey=\"1\">\n      </div>\n\n      <div class=\"form_extension\">\n        <select name=\"perpage\" id=\"perpage\" tabindex=\"3\">\n          <option value=\"10\" selected=\"selected\">10</option>\n          <option value=\"20\">20</option> \n          <option value=\"30\">30</option>\n          <option value=\"50\">50</option>\n          <option value=\"60\">60</option>\n          <option value=\"70\">70</option>\n          <option value=\"80\">80</option>\n          <option value=\"90\">90</option>\n          <option value=\"100\">100</option>\n        </select> per page\n      </div>\n    </form>\n  </div>\n\n")
   );
   if (form_with_help) {
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("   <div class=\"help\">\n    <h1 class=\"title\">Help</h1>\n\n    <h2>What is This?</h2>\n\n    <p>This is a full-text search system. You can search for documents including some specified words.</p>\n\n    <h2>How to Use</h2>\n\n    <p>Input search phrase into the field at the top of the page. For example, if you search for documents including \"computer\", input the\n    following.</p>\n    <pre>computer</pre>\n\n    <p>If you search for documents including both of \"network\" and \"socket\", input the following.</p>\n    <pre>network socket</pre>\n\n    <p>It is the same as the following.</p>\n    <pre>network AND socket</pre>\n\n    <p>If you search for documents including \"network\" followed by \"socket\", input the following.</p>\n    <pre>\"network socket\"</pre>\n\n    <p>If you search for documents including one or both of \"network\" and \"socket\", input the following.</p>\n    <pre>network OR socket</pre>\n\n    <p>If you search for documents including \"network\" but without \"socket\", input the following.</p>\n    <pre>network AND NOT socket</pre>\n\n    <p>For more complex query, you can use \"<code>(</code>\". Note that the priority of \"<code>(</code>\" is higher than that of \"<code>AND</code>\",\n    \"<code>OR</code>\" and \"<code>NOT</code>\". So, the following is to search for documents including one of \"F1\", \"F-1\", \"Formula One\", and including\n    one of \"champion\" and \"victory\".</p>\n    <pre>(F1 OR F-1 OR \"Formula One\") AND (champion OR victory)</pre>\n\n    <h2>You can use DOS wildcard characters</h2>\n\n    <p>If you search for documents including some words beginning with \"inter\", input the following.</p>\n    <pre>inter*</pre>\n\n    <p>If you search for documents including some words ending with \"sphere\", input the following.</p>\n    <pre>*sphere</pre>\n\n    <p>If you search for documents matching some words matching \"?n*able\" (unable, unavoidable, inevitable, ...), input the following.</p>\n    <pre>?n*able</pre>\n\n    <h2>Other Faculties</h2>\n\n    <p>\"<code>[...] per page</code>\" specifies the number of shown documents per page. If documents over one page correspond, you can move to another\n    page via anchors of \"<code>PREV</code>\" and \"<code>NEXT</code>\" at the bottom of the page.</p>\n\n    <h2>Information</h2>\n\n    <p>See <a href=\"http://www.unirel.com/\">the project site</a> for more detail.</p>\n   </div>\n\n")
   );
   } else {
      if (num_args == 0) {
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("   <div class=\"logo\">\n      <h1 class=\"title\">ULib search engine</h1>\n      <div class=\"caption\">a full-text search system for communities</div>\n   </div>\n\n")
   );
   } else {
   
         UString link_paginazione = UHTTP::getLinkPagination();
   
         USP_PRINTF("<div id=\"estresult\" class=\"estresult\">\n"
                    "  <div class=\"resinfo\">\n"
                    "  Results of <strong>%u</strong> - <strong>%u</strong> of about <strong>%u</strong> for <strong>%v</strong> (%v sec.)\n"
                    "  </div>\n",
                    UHTTP::num_page_start, UHTTP::num_page_end, UHTTP::num_item_tot, IR_SESSION.query.rep, IR_SESSION.timerun.rep);
   
         if (UHTTP::num_item_tot == 0) {
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("         <p class=\"note\">Your search did not match any documents.</p>\n")
   );
   } else {
   
            UString doc, snippet_doc(U_CAPACITY), basename, filename, pathname1(U_CAPACITY), pathname2(U_CAPACITY);
   
            for (uint32_t i = UHTTP::num_page_start-1; i < UHTTP::num_page_end; ++i)
               {
               filename = IR_SESSION.vec[i]->filename;
               basename = UStringExt::basename(filename);
   
               pathname1.snprintf(U_CONSTANT_TO_PARAM(  "/doc/%v"), filename.rep);
               pathname2.snprintf(U_CONSTANT_TO_PARAM("%w/doc/%v"), filename.rep);
   
               doc = UFile::contentOf(pathname2);
   
               UXMLEscape::encode(doc, snippet_doc);
   
               USP_PRINTF("<dl class=\"doc\"\n"
                          "  <dt><a href=\"%v\" class=\"doc_title\">%v</a></dt>\n"
                          "  <dd class=\"doc_text\">%v <code class=\"delim\">...</code></dd>\n"
                          "  <dd class=\"doc_navi\"><span class=\"doc_link\">file://%v</span></dd>\n"
                          "</dl>\n",
                          pathname1.rep, basename.rep, snippet_doc.rep, pathname2.rep);
               }
            }
   
         USP_PRINTF("<div class=\"paging\">%v</div></div>\n", link_paginazione.rep);
         }
   }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("  <div id=\"estinfo\" class=\"estinfo\">\n    Powered by <a href=\"http://www.unirel.com/\">ULib search engine</a> ")
   );
   if (*footer) (void) UClientImage_Base::wbuffer->append((*footer));
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("  </div>\n</body>\n</html>")
   );
}
   
extern "C" {
extern U_EXPORT void runDynamicPageParam_ir_web(uint32_t param);
       U_EXPORT void runDynamicPageParam_ir_web(uint32_t param)
{
   U_TRACE(0, "::runDynamicPageParam_ir_web(%u)", param)
   
   if (param == U_DPAGE_INIT) { usp_init_ir_web(); return; }
   if (param == U_DPAGE_DESTROY) { usp_end_ir_web(); return; }
   return;
} }
   
extern "C" {
extern U_EXPORT void runDynamicPage_ir_web();
       U_EXPORT void runDynamicPage_ir_web()
{
   U_TRACE_NO_PARAM(0, "::runDynamicPage_ir_web()")
   
   usp_body_ir_web();
   if (usp_bSESSION) {
   UHTTP::putDataSESSION();
   }
   usp_bSESSION = true;
   UHTTP::mime_index = U_html;
   U_http_info.endHeader = 0;
} }