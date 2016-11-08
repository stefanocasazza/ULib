// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    multipart.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/mime/multipart.h>
#include <ulib/utility/base64.h>
#include <ulib/utility/services.h>
#include <ulib/utility/quoted_printable.h>

#ifdef USE_LIBMAGIC
#  include <ulib/magic/magic.h>
#endif

#ifndef   RFC2045MIMEMSG
#  define RFC2045MIMEMSG "This is a MIME-formatted message. If you see this text it means that your\n" \
                         "E-mail software does not support MIME-formatted messages.\n"
#endif

#ifndef   RFC2231DOENCODE
#  define RFC2231DOENCODE(c) (strchr("()'\"\\%:;=", (c)) || (c) <= ' ' || (c) >= 127)
#endif

const char* UMimeMultipartMsg::str_encoding[4] = {
   "7bit",
   "8bit",
   "quoted-printable",
   "base64"
};

UMimeMultipartMsg::UMimeMultipartMsg(const char* type, uint32_t type_len, Encoding encoding, const char* header, uint32_t header_len, bool bRFC2045MIMEMSG)
{
   U_TRACE_REGISTER_OBJECT(0, UMimeMultipartMsg, "%.*S,%u,%d,%.*S,%u,%b", type_len, type, type_len, encoding, header_len, header, header_len, bRFC2045MIMEMSG)

   U_INTERNAL_ASSERT_POINTER(type)
   U_INTERNAL_ASSERT_POINTER(header)
   U_INTERNAL_ASSERT_MAJOR(type_len, 1)
   U_INTERNAL_ASSERT_MAJOR(U_line_terminator_len, 0)

   uint32_t start = 4;
   char* ptr = boundary;
   const char* line_terminator;

   if (U_line_terminator_len == 1)
      {
      start  = 3;
      *ptr++ = '\n';

      line_terminator = U_LF;
      }
   else
      {
      u_put_unalignedp16(ptr, U_MULTICHAR_CONSTANT16('\r','\n'));
                         ptr += 2;

      line_terminator = U_CRLF;
      }

   u_put_unalignedp32(ptr, U_MULTICHAR_CONSTANT32('-','-','=','_'));
                      ptr += 4;

   boundary_len = (u_num2str64(UServices::getUniqUID(), ptr) - boundary);

   UString buffer(U_CAPACITY);

   // MIME-Version: 1.0
   // Content-Transfer-Encoding: ...
   // \r\n
   // RFC2045MIMEMSG

   buffer.snprintf(U_CONSTANT_TO_PARAM("%.*s%.*s"  
                   "Content-Type: multipart/%.*s; boundary=\"%.*s\"%.*s"
                   "%s%s%.*s"
                   "%.*s"
                   "%s"),  
                   header_len, header, (header_len ? U_line_terminator_len : 0), line_terminator,
                   type_len, type, boundary_len - start, boundary + start, U_line_terminator_len, line_terminator,
                   (encoding == NONE ? "" : "Content-Transfer-Encoding: "),
                   (encoding == NONE ? "" : str_encoding[encoding - 1]),
                   (encoding == NONE ?  0 : U_line_terminator_len), line_terminator,
                                            U_line_terminator_len,  line_terminator,
                   (bRFC2045MIMEMSG ? RFC2045MIMEMSG : ""));

   vec_part.push(buffer);
}

uint32_t UMimeMultipartMsg::message(UString& body, bool bterminator)
{
   U_TRACE(0, "UMimeMultipartMsg::message(%V,%b)", body.rep, bterminator)

   U_ASSERT_MAJOR(vec_part.size(), 1)

   const char* line_terminator = (U_line_terminator_len == 1 ? U_LF : U_CRLF);

   char _buf[64];
   uint32_t content_length = vec_part[0].size(),
            len = u__snprintf(_buf, sizeof(_buf), U_CONSTANT_TO_PARAM("%.*s%.*s"), boundary_len, boundary, U_line_terminator_len, line_terminator);

   body = vec_part.join(_buf, len);

                    (void) body.append(_buf, u__snprintf(_buf, sizeof(_buf), U_CONSTANT_TO_PARAM("%.*s--"), boundary_len, boundary));
   if (bterminator) (void) body.append(line_terminator, U_line_terminator_len);

   content_length = body.size() - content_length;

   U_RETURN(content_length);
}

// Determine encoding as follows:
// -------------------------------------------------------------
// Default to 7bit.
// Use 8bit if high-ascii bytes found.
// Use quoted printable if lines more than 200 characters found.
// Use base64 if a null byte is found
// -------------------------------------------------------------

__pure inline int UMimeMultipartMsg::encodeAutodetect(const UString& content, const char* charset)
{
   U_TRACE(0, "UMimeMultipartMsg::encodeAutodetect(%V,%S)", content.rep, charset)

   U_INTERNAL_ASSERT_POINTER(charset)

   int l = 0;
   bool longline = false;
   Encoding encoding = BIT7;
   unsigned char* ptr  = (unsigned char*) content.data();
   unsigned char* _end = (unsigned char*) content.pend();

   while (ptr < _end)
      {
      unsigned char ch = *ptr++;

      if (ch >= 0x80) encoding = BIT8;

      if (ch  < 0x20 &&
          ch != '\t' &&
          ch != '\r' &&
          ch != '\n')
         {
         if (*charset) encoding = QUOTED_PRINTABLE;
         }

      if (ch == 0) U_RETURN(BASE64);

      if      (ch == '\n') l = 0;
      else if (++l > 200)
         {
         longline = true;

         if (*charset) encoding = QUOTED_PRINTABLE;
         }
      }

   if (longline) encoding = (*charset ? QUOTED_PRINTABLE : BASE64);

   U_RETURN(encoding);
}

// Creating a single MIME section
// -----------------------------------------------------------------------------------------------------
// The type option encodes content appropriately, adds the "Content-Type: type" and
// "Content-Transfer-Encoding:" and MIME headers. type can be any valid MIME type, except for multipart
// The encoding option should be specified. It's more efficient to do so
// The charset option sets the MIME charset attribute for text/plain content
// The name option sets the name attribute for Content-Type:
// Additional headers are specified by the header option, doesn't do anything with them except to insert
// the headers into the generated MIME section
// -----------------------------------------------------------------------------------------------------

UString UMimeMultipartMsg::section(const UString& content,
                                   const char* type, uint32_t type_len,
                                   Encoding encoding, const char* charset, const char* name,
                                   const char* header, uint32_t header_len)
{
   U_TRACE(0, "UMimeMultipartMsg::section(%V,%.*S,%u,%d,%S,%S,%.*S,%u)", content.rep, type_len, type, type_len, encoding, charset, name, header_len, header, header_len)

   U_INTERNAL_ASSERT_POINTER(name)
   U_INTERNAL_ASSERT_POINTER(charset)

   if (encoding == AUTO) encoding = (Encoding) encodeAutodetect(content, charset);

   if (type_len == 0)
      {
#  ifdef USE_LIBMAGIC
      charset = "";

      if (UMagic::magic == 0) (void) UMagic::init();

      UString value = UMagic::getType(content);

      type     = value.data();
      type_len = value.size();
#  else
      type = (encoding == BASE64 ? (charset = "", type_len = U_CONSTANT_SIZE("application/octet-stream"), "application/octet-stream")
                                 : (              type_len = U_CONSTANT_SIZE("text/plain"),               "text/plain"));
#  endif
      }

   U_INTERNAL_ASSERT_EQUALS(strstr(type,"multipart"),0)

   uint32_t length = content.size();

   U_INTERNAL_ASSERT_MAJOR(length, 0)

   UString buffer(U_CAPACITY);
   const char* line_terminator = (U_line_terminator_len == 1 ? U_LF : U_CRLF);

   buffer.snprintf(U_CONSTANT_TO_PARAM("%.*s%.*s"
                   "Content-Type: %.*s"),
                   header_len, header, (header_len ? U_line_terminator_len : 0), line_terminator,
                   type_len, type);

   if (*charset) buffer.snprintf_add(U_CONSTANT_TO_PARAM("; charset=\"%s\""), charset);
   if (*name)    buffer.snprintf_add(U_CONSTANT_TO_PARAM("; name=\"%s\""), name);

   // \r\n
   // Content-Transfer-Encoding: ...
   // \r\n

   buffer.snprintf_add(U_CONSTANT_TO_PARAM("%.*s"
                       "%s%s%.*s"
                       "%.*s"),
                       U_line_terminator_len,  line_terminator,
                       (encoding == NONE ? "" : "Content-Transfer-Encoding: "),
                       (encoding == NONE ? "" : str_encoding[encoding - 1]),
                       (encoding == NONE ?  0 : U_line_terminator_len), line_terminator,
                       U_line_terminator_len,  line_terminator);

   if (encoding != BASE64 &&
       encoding != QUOTED_PRINTABLE)
      {
      buffer += content;
      }
   else
      {
      UString tmp(length * 4);

      const char* ptr = content.data();

      if (encoding == QUOTED_PRINTABLE) UQuotedPrintable::encode(ptr, length, tmp);
      else
         {
         u_base64_max_columns = U_OPENSSL_BASE64_MAX_COLUMN;

         UBase64::encode(ptr, length, tmp);

         u_base64_max_columns = 0;
         }

      buffer += tmp;
      }

   U_RETURN_STRING(buffer);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, UMimeMultipartMsg& m)
{
   U_TRACE(0+256, "UMimeMultipartMsg::operator<<(%p,%p)", &os, &m)

   UString body;

   (void) m.message(body);

   os << body;

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UMimeMultipartMsg::dump(bool reset) const
{
   *UObjectIO::os << "boundary              " << '"' << boundary         << "\"\n"
                  << "boundary_len          " << boundary_len            << '\n'
                  << "vec_part     (UVector " << (void*)&vec_part        << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
