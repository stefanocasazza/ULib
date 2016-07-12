// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    entity.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/mime/entity.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/base64.h>
#include <ulib/utility/quoted_printable.h>

// gcc - call is unlikely and code size would grow

UMimeEntity::UMimeEntity() : content(U_CAPACITY)
{
   U_TRACE_REGISTER_OBJECT(0, UMimeEntity, "")

   header       = 0;
   startHeader  = endHeader = 0;
   parse_result = false;
}

UMimeEntity::UMimeEntity(const UString& _data) : data(_data)
{
   U_TRACE_REGISTER_OBJECT(0, UMimeEntity, "%V", _data.rep)

   startHeader  = 0;
   parse_result = false;

   U_NEW(UMimeHeader, header, UMimeHeader);

   if (parse()) decodeBody();
}

UMimeEntity::UMimeEntity(const char* ptr, uint32_t len) : data(ptr, len)
{
   U_TRACE_REGISTER_OBJECT(0, UMimeEntity, "%.*S,%u", len, ptr, len)

   startHeader  = 0;
   parse_result = false;

   U_NEW(UMimeHeader, header, UMimeHeader);

   if (parse(ptr, len)) decodeBody();
}

__pure bool UMimeEntity::isXML() const               { return UMimeHeader::isXML(content_type); }
__pure bool UMimeEntity::isText() const              { return UMimeHeader::isText(content_type); }
__pure bool UMimeEntity::isPKCS7() const             { return UMimeHeader::isPKCS7(content_type); }
__pure bool UMimeEntity::isRFC822() const            { return UMimeHeader::isRFC822(content_type); }
__pure bool UMimeEntity::isMessage() const           { return UMimeHeader::isMessage(content_type); }
__pure bool UMimeEntity::isURLEncoded() const        { return UMimeHeader::isURLEncoded(content_type); }
__pure bool UMimeEntity::isApplication() const       { return UMimeHeader::isApplication(content_type); }
__pure bool UMimeEntity::isMultipartFormData() const { return UMimeHeader::isMultipartFormData(content_type); }

__pure bool UMimeEntity::isMultipart() const
{
   U_TRACE_NO_PARAM(0, "UMimeEntity::isMultipart()")

   bool result = UMimeHeader::isContentType(content_type, U_CONSTANT_TO_PARAM("multipart"), true); // NB: ignore case - kmail use "Multipart"

   U_RETURN(result);
}

bool UMimeEntity::parse(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UMimeEntity::parse(%.*S,%u)", len, ptr, len)

   U_INTERNAL_ASSERT_POINTER(header)
   U_ASSERT(header->empty())
   U_ASSERT(content.empty())

   endHeader = u_findEndHeader(ptr, len);

   if (endHeader != U_NOT_FOUND &&
       header->parse(ptr+startHeader, endHeader) > 0)
      {
      parse_result = checkContentType();

      U_RETURN(parse_result);
      }

   parse_result = false;

   const char* _ptr = ptr;
   const char* _end = ptr + len;

   while ((_ptr < _end) && u__isspace(*_ptr)) ++_ptr;

   endHeader = _ptr - ptr;

   U_INTERNAL_DUMP("endHeader = %u", endHeader)

   U_RETURN(false);
}

void UMimeEntity::decodeBody()
{
   U_TRACE_NO_PARAM(0, "UMimeEntity::decodeBody()")

   U_INTERNAL_ASSERT_POINTER(header)

   if (content.empty()) content = getBody();

   uint32_t length = content.size();

   // Content transfer encoding: [ "base64", "quoted-printable", "7bit", "8bit", "binary" ]

   UString tfrEncoding = header->getHeader(U_CONSTANT_TO_PARAM("Content-Transfer-Encoding"));

   if (tfrEncoding)
      {
      const char* ptr = tfrEncoding.data();

      if (u__strncasecmp(ptr, U_CONSTANT_TO_PARAM("base64")) == 0)
         {
         UString buffer(length);

         UBase64::decode(content.data(), length, buffer);

         if (buffer)
            {
            content = buffer;

            return;
            }
         }
      else if (u__strncasecmp(ptr, U_CONSTANT_TO_PARAM("quoted-printable")) == 0)
         {
         UString buffer(length);

         UQuotedPrintable::decode(content.data(), length, buffer);

         if (buffer)
            {
            content = buffer;

            return;
            }
         }
   // else if (u__strncasecmp(ptr, U_CONSTANT_TO_PARAM("binary") == 0) return;
      }

   U_INTERNAL_DUMP("content.reference() = %u", content.reference())
}

// read with socket

bool UMimeEntity::readHeader(USocket* socket)
{
   U_TRACE(0, "UMimeEntity::readHeader(%p)", socket)

   U_INTERNAL_ASSERT_POINTER(header)
   U_ASSERT_EQUALS(header->empty(), false)

   data.setBuffer(U_CAPACITY);

   if (header->readHeader(socket, data))
      {
      endHeader   = U_http_info.endHeader;
      startHeader = U_http_info.startHeader;

      U_RETURN(true);
      }

   U_RETURN(false);
}

// checks whether [current,end[ matches -*[\r\t ]*(\n|$)

__pure bool UMimeMultipart::isOnlyWhiteSpaceOrDashesUntilEndOfLine(const char* current, const char* _end)
{
   U_TRACE(0, "UMimeMultipart::isOnlyWhiteSpaceOrDashesUntilEndOfLine(%S,%p)", current, _end)

   bool dashesStillAllowed = true;

   while (current < _end)
      {
      switch (*current)
         {
         case ' ':
         case '\t':
         case '\r':
            {
            dashesStillAllowed = false;

            ++current;

            continue;
            }

         case '\n': U_RETURN(true);

         case '-':
            {
            if (dashesStillAllowed == false) U_RETURN(false);

            ++current;

            continue;
            }

         default: U_RETURN(false);
         }
      }

   // end of buffer is ok, too:

   U_RETURN(true);
}

U_NO_EXPORT bool UMimeMultipart::findBoundary(uint32_t pos)
{
   U_TRACE(0, "UMimeMultipart::findBoundary(%u)", pos)

   // Search for the boundary.
   // The leading CR LF ('\n') is part of the boundary, but if there is no preamble, there may be no leading CR LF ('\n').
   // The case of no leading CR LF ('\n') is a special case that will occur only when '-' is the first character of the body

   if (buf[pos]   == '-'       &&
       buf[pos+1] == '-'       &&
       ((pos+blen+1) < endPos) &&
       (memcmp(&buf[pos+2], bbuf, blen) == 0))
      {
      boundaryStart = pos;

      pos += blen + 2;

      // Check for final boundary

      if (pos+1 < endPos    &&
          buf[pos]   == '-' &&
          buf[pos+1] == '-')
         {
         pos += 2;

         isFinal = true;
         }
      else
         {
         isFinal = false;
         }

      // Advance position past end of line

      while (pos < endPos)
         {
         if (buf[pos] == '\n')
            {
            ++pos;

            break;
            }

         ++pos;
         }

      boundaryEnd = pos;

      U_RETURN(true);
      }

   bool isFound = false;

   while ((pos+blen+2) < endPos)
      {
      // Case of leading LF

      if (buf[pos]   == '\n'                     &&
          buf[pos+1] == '-'                      &&
          buf[pos+2] == '-'                      &&
          (memcmp(&buf[pos+3], bbuf, blen) == 0) &&
          isOnlyWhiteSpaceOrDashesUntilEndOfLine(buf+pos+blen+3, buf+endPos))
         {
         boundaryStart = pos;

         pos += blen + 3;

         isFound = true;
         }

      // Case of leading CR LF

      else if (buf[pos]   == '\r'                     &&
               buf[pos+1] == '\n'                     &&
               buf[pos+2] == '-'                      &&
               ((pos+blen+3) < endPos)                &&
               buf[pos+3] == '-'                      &&
               (memcmp(&buf[pos+4], bbuf, blen) == 0) &&
               isOnlyWhiteSpaceOrDashesUntilEndOfLine(buf+pos+blen+4, buf+endPos))
         {
         boundaryStart = pos;

         pos += blen + 4;

         isFound = true;
         }

      if (isFound)
         {
         // Check for final boundary

         if (pos < endPos &&
             buf[pos] == '-')
            {
            // NOTE: Since we must be fault tolerant for being able to understand messaged that were damaged during
            // transportation we now accept final boundaries ending with "-" instead of "--"

            ++pos;

            isFinal = true;

            // if there *is* the 2nd '-' we of course process it

            if (((pos+1) < endPos) && buf[pos+1] == '-') ++pos;
            }
         else
            {
            isFinal = false;
            }

         // Advance position past end of line

         while (pos < endPos)
            {
            if (buf[pos] == '\n')
               {
               ++pos;

               break;
               }

            ++pos;
            }

         boundaryEnd = pos;

         U_RETURN(true);
         }

      ++pos;
      }

   // Exceptional case: no boundary found

   boundaryStart = boundaryEnd = endPos;

   isFinal = true;

   U_RETURN(false);
}

void UMimeMultipart::init()
{
   U_TRACE_NO_PARAM(0, "UMimeMultipart::init()")

   U_INTERNAL_ASSERT(UMimeEntity::content)

   boundary = UMimeHeader::getBoundary(content_type);

   U_INTERNAL_ASSERT(boundary)

   // Assume the starting position is the beginning of a line

   buf    = UMimeEntity::content.data();
   endPos = UMimeEntity::content.size();

   bbuf = boundary.data();
   blen = boundary.size();

   // Find the preamble

   isFinal = false;
   boundaryStart = boundaryEnd = 0;

   if (findBoundary(0)) (void) parse(UMimeHeader::isType(content_type, U_CONSTANT_TO_PARAM("digest")));
   else                 UMimeEntity::content.clear();
}

void UMimeMultipart::reset()
{
   U_TRACE_NO_PARAM(0, "UMimeMultipart::reset()")

   if (preamble.isNull() == false) preamble.clear();
   if (epilogue.isNull() == false) epilogue.clear();
   if (boundary.isNull() == false) boundary.clear();

   UMimeEntity::content.clear();
}

bool UMimeMultipart::init(const UString& body)
{
   U_TRACE(0, "UMimeMultipart::init(%V)", body.rep)

   U_ASSERT(isEmpty())
   U_INTERNAL_ASSERT(body)

   // Assume the starting position is the beginning of a line

   buf    = body.data();
   endPos = body.size();

   const char* ptr = bbuf = buf + 2;

   while (u__isspace(*ptr) == false) ++ptr; // find space...

   blen = ptr - bbuf;

   // Find the preamble

   isFinal = false;
   boundaryStart = boundaryEnd = 0;

   if (findBoundary(0))
      {
      UMimeEntity::content = body;

      if (parse(false)) U_RETURN(true);

      reset();
      }

   U_ASSERT(isEmpty())

   U_RETURN(false);
}

bool UMimeMultipart::parse(bool digest)
{
   U_TRACE(0, "UMimeMultipart::parse(%b)", digest)

   U_INTERNAL_DUMP("boundaryStart = %u boundaryEnd = %u isFinal = %b", boundaryStart, boundaryEnd, isFinal)

   if (boundaryStart > 0) preamble = UMimeEntity::content.substr(0U, (uint32_t)boundaryStart);

   // Find the body parts

   uint32_t pos = boundaryEnd, len;

   while (isFinal == false)
      {
      bool bfind = findBoundary(pos);

      U_INTERNAL_DUMP("boundaryStart = %u boundaryEnd = %u isFinal = %b", boundaryStart, boundaryEnd, isFinal)

      // NOTE: For enhanced fault tolerance we *accept* a missing last boundary.
      // If no last boundary is found (but at leat a first one was there) we just
      // assume the end of the text ebing the end of the last part. By doing so we
      // can safely parse some buggy MS Outlook clients' messages...

      len = (bfind ? (boundaryStart - pos) : (isFinal = true, endPos - pos));

      UMimeEntity* item;

      U_NEW(UMimeEntity, item, UMimeEntity(UMimeEntity::content.c_pointer(pos), len));

      if (item->isParsingOk() == false)
         {
         if (digest)
            {
            // in a digest, the default Content-Type value for a body part is changed from "text/plain" to "message/rfc822"

            item->content_type = *UString::str_msg_rfc;
            }
         else
            {
            // implicitly typed plain ASCII text

            item->content_type = *UString::str_txt_plain;
            }

         (void) item->header->setHeaderIfAbsent(U_CONSTANT_TO_PARAM("Content-Type"), item->content_type);
         }

      bodypart.push(item);

      pos = (bfind ? boundaryEnd : endPos);
      }

   // Find the epilogue

   if ((len = (endPos - pos)) > 0) epilogue = UMimeEntity::content.substr(pos, len);

   U_INTERNAL_DUMP("getNumBodyPart() = %u", getNumBodyPart())

   if (getNumBodyPart()) U_RETURN(true);

   U_RETURN(false);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, const UMimeMultipart& ml)
{
   U_TRACE(0+256, "UMimeMultipart::operator<<(%p,%p)", &os, &ml)

   UString b;
   UMimeEntity* e;

   if (ml.header) os << *(ml.header);

   for (uint32_t i = 0, n = ml.bodypart.size(); i < n; ++i)
      {
      e = ml.bodypart[i];
      b = e->getBody();

      os.write(U_CONSTANT_TO_PARAM("--"));
      os.write(U_STRING_TO_PARAM(ml.boundary));
      os.write(U_CONSTANT_TO_PARAM(U_CRLF));
      os << *(e->getHeader());
      os.write(U_STRING_TO_PARAM(b));
      os.write(U_CONSTANT_TO_PARAM(U_CRLF));
      }

   os.write(U_CONSTANT_TO_PARAM("--"));
   os.write(U_STRING_TO_PARAM(ml.boundary));
   os.write(U_CONSTANT_TO_PARAM("--"));
   os.write(U_CONSTANT_TO_PARAM(U_CRLF));

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UMimeEntity::dump(bool reset) const
{
   U_CHECK_MEMORY

   *UObjectIO::os << "endHeader                 " << endHeader             << '\n'
                  << "startHeader               " << startHeader           << '\n'
                  << "parse_result              " << parse_result          << '\n'
                  << "data         (UString     " << (void*)&data          << ")\n"
                  << "header       (UMimeHeader " << (void*)header         << ")\n"
                  << "content      (UString     " << (void*)&content       << ")\n"
                  << "content_type (UString     " << (void*)&content_type  << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UMimeMessage::dump(bool reset) const
{
   U_CHECK_MEMORY

   UMimeEntity::dump(false);

   *UObjectIO::os << '\n'
                  << "rfc822       (UMimeEntity " << (void*)&rfc822 << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UMimeMultipart::dump(bool _reset) const
{
   UMimeEntity::dump(false);

   *UObjectIO::os << '\n'
                  << "boundaryEnd               " << boundaryEnd      << '\n'
                  << "boundaryStart             " << boundaryStart    << '\n'
                  << "preamble     (UString     " << (void*)&preamble << ")\n"
                  << "epilogue     (UString     " << (void*)&epilogue << ")\n"
                  << "boundary     (UString     " << (void*)&boundary << ")\n"
                  << "bodypart     (UVector     " << (void*)&bodypart << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
