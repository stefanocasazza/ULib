// example_form.cpp - dynamic page translation (example_form.usp => example_form.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_example_form(int param);
       U_EXPORT void runDynamicPage_example_form(int param)
{
   U_TRACE(0, "::runDynamicPage_example_form(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString first_name = USP_FORM_VALUE(0);
   
   UString last_name = USP_FORM_VALUE(1);
   
   UString email = USP_FORM_VALUE(2);
   
   UString comments = USP_FORM_VALUE(3);
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n<head>\n  <title>Welcome to ULib Web Server!</title>\n</head>\n<body bgcolor=\"white\" text=\"black\">\n  <center>\n    <h1>Welcome to ULib Web Server! (Ver. ")
   );
   
   (void) UClientImage_Base::wbuffer->append((ULIB_VERSION));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(")</h1>\n  </center>\n")
   );
   
   if (UHTTP::isGET())
   {
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("  <form action=\"example_form\" method=\"post\">\n    <p>First Name <input name=\"name\" type=\"text\" class=\"inputbox\" title=\"Enter your first name\"></p>\n    <p>Last Name <input name=\"last\" type=\"text\" class=\"inputbox\" title=\"Enter your last name\"></p>\n    <p>Email <input name=\"email\" type=\"text\" class=\"inputbox\" title=\"Enter your email address\"></p>\n    <p>Enter your comments below:</p>\n    <p><textarea name=\"comments\" title=\"Enter your comments\" rows=\"10\" cols=\"10\"></textarea></p>\n    <p><input type=\"submit\" value=\"Send Data\"></p>\n  </form>\n")
   );
   
   }
   else
   {
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\t<blockquote>\n\t<table width=\"750\" cellpadding=\"5\">\n\t  <tr>\n\t\t <td bgcolor=\"#E0E0FF\">First Name: ")
   );
   
   (void) UClientImage_Base::wbuffer->append((first_name));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t\t <td bgcolor=\"#E0E0FF\">Last Name: ")
   );
   
   (void) UClientImage_Base::wbuffer->append((last_name));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t\t <td bgcolor=\"#E0E0FF\">Email: ")
   );
   
   (void) UClientImage_Base::wbuffer->append((email));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t\t <td bgcolor=\"#E0E0FF\">your comments: ")
   );
   
   USP_XML_PUTS((comments));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t  </tr>\n\t</table>\n\t</blockquote>\n")
   );
   
   }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }