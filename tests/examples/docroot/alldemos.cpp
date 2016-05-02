// alldemos.cpp - dynamic page translation (alldemos.usp => alldemos.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#include <fstream>   
   
extern "C" {
extern U_EXPORT void runDynamicPage_alldemos(int param);
       U_EXPORT void runDynamicPage_alldemos(int param)
{
   U_TRACE(0, "::runDynamicPage_alldemos(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <style type=\"text/css\">\n   table {\n     width: 100%;\n     border: 3px solid black;\n     padding: 0;\n     border-spacing: 0;\n   }\n   th {\n     border: 1px solid black;\n     border-bottom: 2px solid black;\n     background-color: #ffe;\n   }\n   td {\n     border: 1px solid black;\n     padding-left: 3;\n     padding-right: 10;\n     padding-top: 1;\n     padding-bottom: 1;\n   }\n  </style>\n </head>\n <body>\n  <h1>ULib Servlet Page demos</h1>\n  <table style=\"border: 1px solid black\">\n   <tr><th>Demo</th><th>Source</th><th>Description</th></tr>\n\n   ")
   );
   
   std::string filename = "../demos.txt";
      std::ifstream demos(filename.c_str());
   
      while (demos)
         {
         // read line
         std::string line;
         getline(demos, line);
   
         // find delimiters '|'
         std::string::size_type n0 = line.find('|');
         if (n0 == std::string::npos) continue;
         std::string::size_type n1 = line.find('|', n0 + 1);
         if (n1 == std::string::npos) continue;
   
         // extract parts
         std::string url(line, 0, n0);
         std::string name(line, n0 + 1, n1 - n0 - 1);
         std::string description(line, n1 + 1);
   
         // skip empty urls and lines starting with '#'
         if (url.empty() || line[0] == '#') continue;
   
         USP_PRINTF("<tr>"
                    "<td><a href=\"%.*s\">%.*s</a></td>"
                    "<td><a href=\"%.*s.usp\">source</a></td>"
                    "<td>%.*s</td>"
                    "</tr>\n", U_STRING_TO_TRACE(url), U_STRING_TO_TRACE(name), U_STRING_TO_TRACE(url), U_STRING_TO_TRACE(description));
         }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("  </table>\n </body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }