// action.cpp

#include "action.h"

#include <ulib/mime/multipart.h>
#include <ulib/utility/escape.h>
#include <ulib/utility/services.h>
#include <ulib/utility/xml_escape.h>

bool Action::loadFileConfig(UString& cfg_str)
{
   U_TRACE(5, "Action::loadFileConfig(%.*S)", U_STRING_TO_TRACE(cfg_str))

   if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("action.cfg");

   cfg.UFile::setPath(cfg_str);

   // -----------------------------------------------------------------------------------------------
   // configuration parameters
   // -----------------------------------------------------------------------------------------------
   //  REQUEST_TEMPLATE  scanf form to extract data
   // RESPONSE_TEMPLATE printf form to output       response
   //    ERROR_TEMPLATE printf form to output error response
   // -----------------------------------------------------------------------------------------------

   req_tmpl = UFile::contentOf(cfg[U_STRING_FROM_CONSTANT( "REQUEST_TEMPLATE")]);
   res_tmpl = UFile::contentOf(cfg[U_STRING_FROM_CONSTANT("RESPONSE_TEMPLATE")]);
   err_tmpl = UFile::contentOf(cfg[U_STRING_FROM_CONSTANT(   "ERROR_TEMPLATE")]);

   U_RETURN(true);
}

void Action::processInputData(int expected)
{
   U_TRACE(5, "Action::processInputData(%d)", expected)

   // read stdin

   UServices::readEOF(STDIN_FILENO, data);

   if (data.empty()) U_ERROR("cannot read data from stdin");

   request.setBuffer(data.size());

   const char* ptr1 =     data.c_str();
         char* ptr2 = req_tmpl.data();
         char* ptr3 =  request.data();

   int n = (expected == 2 ? U_SYSCALL(sscanf, "%S,%S,%p,%p",             ptr1, ptr2, id,                         ptr3)      :
            expected == 5 ? U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p,%p",    ptr1, ptr2, id, customer, installation, ptr3, uid) :
                            U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p,%p,%p", ptr1, ptr2, id, event, state, action_name, error_message, ptr3));

   if (n != expected) U_ERROR("scanf error on input data");

   request.size_adjust();

   if (expected == 5)
      {
      request_decoded.setBuffer(request.size());

      UXMLEscape::decode(request, request_decoded);
      }

   U_INTERNAL_DUMP("id = %S customer = %S installation = %S uid = %S request = %S", id, customer, installation, uid, ptr3)
   U_INTERNAL_DUMP("event = %S state = %S action_name = %S error_message = %S", event, state, action_name, error_message)
}

bool Action::sendHttpPostRequest(const UString& url, const UString& body, const char* content_type, const char* expected)
{
   U_TRACE(5, "Action::sendHttpPostRequest(%.*S,%.*S,%S,%S)", U_STRING_TO_TRACE(url), U_STRING_TO_TRACE(body), content_type, expected)

   bool ok  = client_http.sendPost(url, body, content_type);
   response = client_http.getContent();

   // manage error

   if (response.empty() ||
       u_http_info.nResponseCode != HTTP_OK)
      {
      ok = false;

      response.setBuffer(100U);
      response.snprintf("HTTP response %d",  u_http_info.nResponseCode);
      }
   else if (ok && strncmp(response.data(), expected, u__strlen(expected, __PRETTY_FUNCTION__)))
      {
      ok = false;

      response.setBuffer(100U);
      response.snprintf("Non esiste una una cartella corrispondente all'UID %s", uid);
      }

   U_RETURN(ok);
}

bool Action::sendEmail()
{
   U_TRACE(5, "Action::sendEmail()")

   bool ok = emailClient._connectServer(cfg);

   toAddress = emailClient.getRecipientAddress();

   if (ok)
      {
      UString buffer(U_CAPACITY);

      if (*state)
         {
         buffer.snprintf("Reply-To: %s+%s+%s", state, event, id);

         emailClient.setMessageHeader(buffer);
         }

      // --------------------------------------------------------------------------------------------------------------
      // configuration parameters
      // --------------------------------------------------------------------------------------------------------------
      // EMAIL_BODY_FORM
      // EMAIL_SUBJECT_FORM
      // --------------------------------------------------------------------------------------------------------------

      UString emailBodyForm    = cfg[U_STRING_FROM_CONSTANT("EMAIL_BODY_FORM")],
              emailSubjectForm = cfg[U_STRING_FROM_CONSTANT("EMAIL_SUBJECT_FORM")];
      
      UString tmp(emailBodyForm.size());

      if (UEscape::decode(emailBodyForm, tmp)) emailBodyForm = tmp;

      buffer.setBuffer(emailSubjectForm.size() + 1024U);
      buffer.snprintf( emailSubjectForm.data(), customer, installation, uid);

      emailClient.setMessageSubject(buffer);

      buffer.setBuffer(emailBodyForm.size() + 1024U);

      if (*state == '\0') buffer.snprintf(emailBodyForm.data(), customer, installation, uid);
      else
         {
         UMimeMultipartMsg msg;

         buffer.snprintf(emailBodyForm.data(), event, state, action_name, error_message);

         msg.add(UMimeMultipartMsg::section(buffer, 0, 0, UMimeMultipartMsg::AUTO));
         msg.add(UMimeMultipartMsg::section(data,   0, 0, UMimeMultipartMsg::AUTO, "", "",
                                                    U_CONSTANT_TO_PARAM("Content-Disposition: attachment; filename=\"workflow-message.xml\"")));

         (void) msg.message(buffer);
         }

      emailClient.setMessageBody(buffer);

      ok = emailClient.sendMessage();
      }

   if (ok == false)
      {
      emailClient.setStatus();

      response.setBuffer(u_buffer_len + 100U);

      response.snprintf("Email notification to '%.*s' failed: %.*s", U_STRING_TO_TRACE(toAddress), u_buffer_len, u_buffer);

      u_buffer_len = 0;
      }

   U_RETURN(ok);
}

void Action::writeToSTDOUT(bool ok, bool all)
{
   U_TRACE(5, "Action::writeToSTDOUT(%b,%b)", ok, all)

   UString output;

   if (ok)
      {
      UApplication::exit_value = 0;

      if (all)
         {
         output.setBuffer(res_tmpl.size() + request.size() + 1024U);
         output.snprintf( res_tmpl.data(), id,                              customer, installation, U_STRING_TO_TRACE(request), uid);
         }
      else
         {
         output.setBuffer(res_tmpl.size() + request.size() +  256U);
         output.snprintf( res_tmpl.data(), id,                                                      U_STRING_TO_TRACE(request));
         }
      }
   else
      {
      if (all)
         {
         output.setBuffer(err_tmpl.size() + request.size() + 1024U + response.size());
         output.snprintf( err_tmpl.data(), id, U_STRING_TO_TRACE(response), customer, installation, U_STRING_TO_TRACE(request), uid);
         }
      else
         {
         output.setBuffer(err_tmpl.size() + request.size() +  256U + response.size());
         output.snprintf( err_tmpl.data(), id, U_STRING_TO_TRACE(response),                         U_STRING_TO_TRACE(request));
         }
      }

   (void) UFile::write(STDOUT_FILENO, output.data(), output.size());
}
