// example.cpp - dynamic page translation (example.usp => example.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_example(int param);
       U_EXPORT void runDynamicPage_example(int param)
{
   U_TRACE(0, "::runDynamicPage_example(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n<head>\n  <title>ULib start page</title>\n</head>\n<body>\n  <blockquote>\n    <blockquote>\n      <table width=\"750\" cellpadding=\"5\">\n        <tr>\n          <td bgcolor=\"white\">\n            <center>\n              <h3>Welcome to Ulib (Ver. ")
   );
   
   (void) UClientImage_Base::wbuffer->append((ULIB_VERSION));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(")!</h3>\n            </center>\n          </td>\n        </tr>\n      </table>\n    </blockquote>\n\n    <blockquote>\n      <table width=\"750\" cellpadding=\"5\">\n        <tr>\n          <td bgcolor=\"#E0E0FF\">")
   );
   
   USP_PRINTF("ULib Servlet Page (USP): %N (pid %P) [%U@%H] - %D", 0);
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n        </tr>\n      </table>\n    </blockquote>\n\n    <p>About Ulib:</p>\n\n    <blockquote>\n      <table width=\"750\" cellpadding=\"5\">\n        <tr>\n          <td bgcolor=\"#E0E0FF\">\n          <a href=\"http://www.unirel.com/ULib/index.html\">Info</a><br>\n          <a href=\"http://www.unirel.com/ULib/features.html\">Features</a><br>\n          <a href=\"http://www.unirel.com/ULib/tut.html\">Tutorial</a><br>\n          <a href=\"http://www.unirel.com/ULib/support.html\">Support</a><br></td>\n        </tr>\n      </table>\n    </blockquote><font size=\"-1\">Copyright (c) 2009 by <a href=\"http://www.unirel.com\">Unirel s.r.l.</a> - All rights reserved.</font>\n  </blockquote>\n</body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }