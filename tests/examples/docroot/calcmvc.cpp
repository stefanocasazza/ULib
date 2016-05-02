// calcmvc.cpp - dynamic page translation (calcmvc.usp => calcmvc.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
class calcBean {
      double arg1;
      double arg2;
      double result;
      char op;
      bool resultOk;
   
   public:
      calcBean() : arg1(0), arg2(0), op('+'), resultOk(false) { }
   
      double getArg1() const    { return arg1; }
      double getArg2() const    { return arg2; }
      char   getOp() const      { return op; }
      bool   isResultOk() const { return resultOk; }
      double getResult() const  { return result; }
   
      void set(double arg1_, double arg2_, char op_)
         {
         op   = op_;
         arg1 = arg1_;
         arg2 = arg2_;
   
         resultOk = true;
   
         switch (op)
            {
            case '+': result = arg1 + arg2; break;
            case '-': result = arg1 - arg2; break;
            case '*': result = arg1 * arg2; break;
            case '/': result = arg1 / arg2; break;
   
            default:  resultOk = false;
            }
         }
   }; 
   
extern "C" {
extern U_EXPORT void runDynamicPage_calcmvc(int param);
       U_EXPORT void runDynamicPage_calcmvc(int param)
{
   U_TRACE(0, "::runDynamicPage_calcmvc(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString arg1 = USP_FORM_VALUE(0);
   
   if (arg1.empty()) arg1 = U_STRING_FROM_CONSTANT("0");
   
   UString arg2 = USP_FORM_VALUE(1);
   
   if (arg2.empty()) arg2 = U_STRING_FROM_CONSTANT("0");
   
   UString opval = USP_FORM_VALUE(2);
   
   if (opval.empty()) opval = U_STRING_FROM_CONSTANT(" ");
   
   calcBean calc;
   
   calc.set(arg1.strtod(), arg2.strtod(), opval.first_char());
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <title>Calculator</title>\n </head>\n <body bgcolor=#ffffcc>\n  <h1>Tommi's Calculator</h1>\n\n  <form>\n   <input type=\"text\"   name=\"arg1\" value=\"")
   );
   
   (void) UClientImage_Base::wbuffer->append((arg1));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\" ><br>\n   <input type=\"text\"   name=\"arg2\" value=\"")
   );
   
   (void) UClientImage_Base::wbuffer->append((arg2));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\" ><br>\n   <input type=\"submit\" name=\"op\"   value=\"+\">\n   <input type=\"submit\" name=\"op\"   value=\"-\">\n   <input type=\"submit\" name=\"op\"   value=\"*\">\n   <input type=\"submit\" name=\"op\"   value=\"/\">\n  </form>\n\n")
   );
   
   if (calc.isResultOk())
      {
      USP_PRINTF("<hr>\n%v %v %v = %g", arg1.rep, opval.rep, arg2.rep, calc.getResult());
      }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(" </body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }