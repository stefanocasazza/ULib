// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    entity.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MIME_ENTITY_H
#define U_MIME_ENTITY_H 1

#include <ulib/mime/header.h>

/**
 * UMimeEntity -- class representing a MIME entity
 *
 * RFC-2045 defines an entity as either a message or a body part, both of which have a collection of headers and a body
 *
 * MIME headers comprise a keyword followed by ":" followed by whitespace and the value.
 * If a line begins with whitespace it is a continuation of the preceeding line.
 * MIME headers are delimited by an empty line
 *
 * The only reliable way to determine the type of body is to access the Content-Type header field from the
 * headers object of the entity that contains it. For this reason, a body should always be part of a entity.
 * Only types "multipart" and "message" need to be parsed, and in all content types, the body contains a
 * string of characters
 *
 * If the content type is 'message'   then the body contains an encapsulated message
 * If the content type is 'multipart' then the body contains one or more body parts
 */

class UHTTP;
class UMimeMultipart;

class U_EXPORT UMimeEntity {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UMimeEntity();
   UMimeEntity(const UString& _data);
   UMimeEntity(const char* ptr, uint32_t len);

   UMimeEntity(const UMimeEntity& item) : data(item.data), content_type(item.content_type), content(item.content)
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeEntity, "%p", &item)

      U_INTERNAL_ASSERT_POINTER(item.header)

      parse_result = false;
      endHeader    = item.endHeader;
      startHeader  = item.startHeader;

      header = item.header; // NB: move...

      ((UMimeEntity*)&item)->header = 0;
      }

   ~UMimeEntity()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMimeEntity)

      if (header) delete header;
      }

   bool isEmpty()
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::isEmpty()")

      if (content.empty()) U_RETURN(true);

      U_RETURN(false);
      }

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::clear()")

              data.clear();
           content.clear();
      content_type.clear();

      if (header) header->clear();
      }

   UMimeHeader* getHeader() const { return header; }

   // PARSING

   bool parse()
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::parse()")

      U_INTERNAL_ASSERT(data)

      if (parse(U_STRING_TO_PARAM(data))) U_RETURN(true);

      U_RETURN(false);
      }

   bool parse(const UString& _data)
      {
      U_TRACE(0, "UMimeEntity::parse(%V)", _data.rep)

      U_ASSERT(data.empty())

      data = _data;

      return parse();
      }

   bool isParsingOk()
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::isParsingOk()")

      U_INTERNAL_ASSERT_POINTER(header)

      U_RETURN(parse_result);
      }

   UString getData() const        { return data; }
   UString getBody() const        { return data.substr(endHeader); }
   UString getContent() const     { return content; }
   UString getContentType() const { return content_type; }
   UString getMimeVersion() const { return header->getMimeVersion(); }

   // =============================================================================================================================
   // Content types: "multipart"   / [ "mixed", "alternative", "digest", "parallel", "signed", "encrypted", "report", "form-data" ],
   //                "message"     / [ "rfc822", "disposition-notification" ],
   //                "image"       / [ "jpeg", "gif" ],
   //                "audio"       / [ "basic" ],
   //                "video"       / [ "mpeg" ],
   //                "application" / [ "postscript", "octet-stream", "pgp-signature", "pgp-encrypted", "pgp-clearsigned",
   //                                  "pkcs7-signature", "pkcs7-mime", "ms-tnef", "x-www-form-urlencoded" ]
   //                "text"        / [ "plain" (RFC-1521), "richtext" (RFC-1341), "enriched", "html", "xvcard", "vcal",
   //                                  "rtf", "xml" ],
   // =============================================================================================================================

   bool isMime() { return header->isMime(); }

   bool isXML() const __pure;
   bool isText() const __pure;
   bool isPKCS7() const __pure;
   bool isRFC822() const __pure;
   bool isMessage() const __pure;
   bool isMultipart() const __pure;
   bool isURLEncoded() const __pure;
   bool isApplication() const __pure;
   bool isMultipartFormData() const __pure;

   UString getCharSet() const        { return UMimeHeader::getCharSet(content_type); } // get charset/content-type info
   UString shortContentType() const  { return UMimeHeader::shortContentType(content_type); }

   UString getValueAttributeFromKey(const UString& key, const UString& name) const
      { return header->getValueAttributeFromKey(key, name); }

   bool isType(       const char* type, uint32_t len) const { return UMimeHeader::isType(       content_type, type, len); }
   bool isContentType(const char* type, uint32_t len) const { return UMimeHeader::isContentType(content_type, type, len); }

   // Disposition type (Content-Disposition header field, see RFC-1806): "inline", "attachment"

   UString getContentDisposition() const
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::getContentDisposition()")

      U_INTERNAL_ASSERT_POINTER(header)

      UString value = header->getContentDisposition();

      U_RETURN_STRING(value);
      }

   bool isAttachment() const
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::isAttachment()")

      UString value = getContentDisposition();

      if (value.empty()                         == false &&
          U_STRING_FIND(value, 0, "attachment") != U_NOT_FOUND)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isBodyMessage() const
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::isBodyMessage()")

      if (isText()                                                                                             &&
          UMimeHeader::getValueAttributeFromKeyValue(content_type, U_CONSTANT_TO_PARAM("name"), false).empty() &&
          isAttachment() == false)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   UString getFileName() const { return UMimeHeader::getFileName(getContentDisposition()); }

   // read with socket

   bool readHeader(USocket* socket);

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& os, const UMimeEntity& e)
      {
      U_TRACE(0+256, "UMimeEntity::operator<<(%p,%p)", &os, &e)

      U_INTERNAL_ASSERT_POINTER(e.header)

      os << *(e.header);

      os.write(e.content.data(), e.content.size());

      return os;
      }

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UMimeHeader* header; // mutable
   uint32_t startHeader, endHeader;
   UString data, content_type, content;
   bool parse_result;

   bool checkContentType()
      {
      U_TRACE_NO_PARAM(0, "UMimeEntity::checkContentType()")

      U_INTERNAL_ASSERT_POINTER(header)

      content_type = header->getContentType();

      if (content_type) U_RETURN(true);

      U_RETURN(false);
      }

   void decodeBody();
   bool parse(const char* ptr, uint32_t len);

private:
   U_DISALLOW_ASSIGN(UMimeEntity)

   friend class UHTTP;
   friend class UMimeMultipart;
};

// If the content type is 'message' then the body contains an encapsulated message

class U_EXPORT UMimeMessage : public UMimeEntity {
public:

   UMimeMessage(UMimeEntity& item) : UMimeEntity(item), rfc822(UMimeEntity::content)
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeMessage, "%p", &item)

      U_ASSERT(UMimeEntity::isMessage())
      }

   UMimeMessage(const UString& _data) : UMimeEntity(_data), rfc822(UMimeEntity::content)
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeMessage, "%V", _data.rep)
      }

   ~UMimeMessage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMimeMessage)
      }

   UMimeEntity& getRFC822() { return rfc822; }

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& os, const UMimeMessage& m)
      {
      U_TRACE(0+256, "UMimeMessage::operator<<(%p,%p)", &os, &m)

      U_INTERNAL_ASSERT_POINTER(m.header)

      os << *(m.header) << m.rfc822;

      return os;
      }

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UMimeEntity rfc822;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UMimeMessage)
};

// If the content type is 'multipart' then the body contains one or more body parts

class U_EXPORT UMimeMultipart : public UMimeEntity {
public:

   UMimeMultipart() : UMimeEntity()
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeMultipart, "", 0)

      buf = bbuf = 0;
      isFinal = false;
      blen = boundaryStart = boundaryEnd = endPos = 0;
      }

   UMimeMultipart(UMimeEntity& item) : UMimeEntity(item)
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeMultipart, "%p", &item)

      U_ASSERT(UMimeEntity::isMultipart())

      init();
      }

   UMimeMultipart(const UString& _data) : UMimeEntity(_data)
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeMultipart, "%V", _data.rep)

      init();
      }

   ~UMimeMultipart()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMimeMultipart)

      setEmpty();

      clear();
      }

   UString getBoundary() const { return boundary; }
   UString getPreamble() const { return preamble; } 
   UString getEpilogue() const { return epilogue; }

   // manage parts

   UMimeEntity* operator[](uint32_t pos) const   { return bodypart.at(pos); }

   UVector<UMimeEntity*>& getBodyPart()          { return bodypart; }
   uint32_t               getNumBodyPart() const { return bodypart.size(); }

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UMimeMultipart& ml);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   const char* buf;
   const char* bbuf;
   UVector<UMimeEntity*> bodypart;
   UString boundary, preamble, epilogue;
   uint32_t blen, boundaryStart, boundaryEnd, endPos;
   bool isFinal;

   void init();
   bool init(const UString& body);

   void reset();
   bool parse(bool digest);

   bool isEmpty()
      {
      U_TRACE_NO_PARAM(0, "UMimeMultipart::isEmpty()")

      if (UMimeEntity::isEmpty() &&
          getNumBodyPart() == 0)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   void setEmpty()
      {
      U_TRACE_NO_PARAM(0, "UMimeMultipart::setEmpty()")

      bodypart.clear();

      reset();
      }

private:
          bool findBoundary(uint32_t pos) U_NO_EXPORT;
   static bool isOnlyWhiteSpaceOrDashesUntilEndOfLine(const char* current, const char* end) U_NO_EXPORT __pure;

   U_DISALLOW_COPY_AND_ASSIGN(UMimeMultipart)

   friend class UHTTP;
};

#endif
