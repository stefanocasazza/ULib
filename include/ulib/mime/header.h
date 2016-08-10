// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    header.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MIMEHEADER_H
#define U_MIMEHEADER_H 1

#include <ulib/net/socket.h>
#include <ulib/container/hash_map.h>

/**
 * Mime-type headers comprise a keyword followed by ":" followed by whitespace and the value.
 * If a line begins with whitespace it is a continuation of the preceeding line.
 * MIME headers are delimited by an empty line. Well-known header fields are those documented
 * in RFC-822 (standard email), RFC-1036 (USENET messages), RFC-2045 (MIME messages), and possibly other RFCs.
 *
 * Text&         Subject()                { return (Text&)           FieldBody("Subject"); }
 * Text&         Comments()               { return (Text&)           FieldBody("Comments"); }
 * Text&         InReplyTo()              { return (Text&)           FieldBody("In-Reply-To"); }
 * Text&         Keywords()               { return (Text&)           FieldBody("Keywords"); }
 * Text&         Encrypted()              { return (Text&)           FieldBody("Encrypted"); }
 * Text&         Received()               { return (Text&)           FieldBody("Received"); }
 * Text&         References()             { return (Text&)           FieldBody("References"); }
 * MsgId&        MessageId()              { return (MsgId&)          FieldBody("Message-Id"); }
 * MsgId&        ResentMessageId()        { return (MsgId&)          FieldBody("Resent-Message-Id"); }
 * DateTime&     Date()                   { return (DateTime&)       FieldBody("Date"); }
 * DateTime&     ResentDate()             { return (DateTime&)       FieldBody("Resent-Date"); }
 * Address&      ReturnPath()             { return (Address&)        FieldBody("Return-Path"); }
 * AddressList&  Bcc()                    { return (AddressList&)    FieldBody("Bcc"); }
 * AddressList&  Cc()                     { return (AddressList&)    FieldBody("Cc"); }
 * AddressList&  ReplyTo()                { return (AddressList&)    FieldBody("Reply-To"); }
 * AddressList&  ResentBcc()              { return (AddressList&)    FieldBody("Resent-Bcc"); }
 * AddressList&  ResentCc()               { return (AddressList&)    FieldBody("Resent-Cc"); }
 * AddressList&  ResentReplyTo()          { return (AddressList&)    FieldBody("Resent-Reply-To"); }
 * AddressList&  ResentTo()               { return (AddressList&)    FieldBody("Resent-To"); }
 * AddressList&  To()                     { return (AddressList&)    FieldBody("To"); }
 * Mailbox&      ResentSender()           { return (Mailbox&)        FieldBody("Resent-Sender"); }
 * Mailbox&      Sender()                 { return (Mailbox&)        FieldBody("Sender"); }
 * MailboxList&  From()                   { return (MailboxList&)    FieldBody("From"); }
 * MailboxList&  ResentFrom()             { return (MailboxList&)    FieldBody("Resent-From"); }
 *
 * RFC-822 fields
 *
 * Text& Approved()                       { return (Text&)           FieldBody("Approved"); }
 * Text& Control()                        { return (Text&)           FieldBody("Control"); }
 * Text& Distribution()                   { return (Text&)           FieldBody("Distribution"); }
 * Text& Expires()                        { return (Text&)           FieldBody("Expires"); }
 * Text& FollowupTo()                     { return (Text&)           FieldBody("Followup-To"); }
 * Text& Lines()                          { return (Text&)           FieldBody("Lines"); }
 * Text& Newsgroups()                     { return (Text&)           FieldBody("Newsgroups"); }
 * Text& Organization()                   { return (Text&)           FieldBody("Organization"); }
 * Text& Path()                           { return (Text&)           FieldBody("Path"); }
 * Text& Summary()                        { return (Text&)           FieldBody("Summary"); }
 * Text& Xref()                           { return (Text&)           FieldBody("Xref"); }
 *
 * RFC-1036 fields (USENET messages)
 *
 * Text&       MimeVersion()              { return (Text&)           FieldBody("MIME-Version"); }
 * Text&       ContentDescription()       { return (Text&)           FieldBody("Content-Description"); }
 * MsgId&      ContentId()                { return (MsgId&)          FieldBody("Content-Id"); }
 * Mechanism&  ContentTransferEncoding()  { return (Mechanism&)      FieldBody("Content-Transfer-Encoding"); }
 * MediaType&  ContentType()              { return (MediaType&)      FieldBody("Content-Type"); }
 *
 * RFC-2045 fields
 *
 * DispositionType& ContentDisposition()  { return (DispositionType&) FieldBody("Content-Disposition"); }
 */

class UHTTP;
class UHTTP2;

class U_EXPORT UMimeHeader {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UMimeHeader()
      {
      U_TRACE_REGISTER_OBJECT(0, UMimeHeader, "", 0)
      }

   ~UMimeHeader()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMimeHeader)
      }

   // Wrapper for table

   void clear()                  {        table.clear(); header.clear(); }
   bool empty() const            { return table.empty(); }
   bool ignoreCase() const       { return table.ignoreCase(); }
   void setIgnoreCase(bool flag) {        table.setIgnoreCase(flag); }

   UString      erase(const UString& key) { return table.erase(key); }
   UString operator[](const UString& key) { return table[key]; }

   uint32_t parse(const char* ptr, uint32_t n);

   uint32_t parse(const UString& buffer) { return parse(buffer.data(), buffer.size()); }

   void   removeHeader(const UString& key) { return removeHeader(U_STRING_TO_PARAM(key)); }
   bool containsHeader(const UString& key) { return table.find(key); }
   UString   getHeader(const UString& key) { return table[key]; }

   void   removeHeader(const char* key, uint32_t keylen);
   bool containsHeader(const char* key, uint32_t keylen) { return table.find(key, keylen); }
   UString   getHeader(const char* key, uint32_t keylen) { return table.at(  key, keylen); }

   // Sets a header field, overwriting any existing value

   void setHeader(const UString& key, const UString& value)
      {
      U_TRACE(0, "UMimeHeader::setHeader(%V,%V)", key.rep, value.rep)

      if (containsHeader(key)) table.replaceAfterFind(value);
      else                     table.insertAfterFind(key, value);
      }

   void setHeader(const char* key, uint32_t keylen, const UString& value)
      {
      U_TRACE(0, "UMimeHeader::setHeader(%.*S,%u,%V)", keylen, key, keylen, value.rep)

      if (containsHeader(key, keylen)) table.replaceAfterFind(value);
      else
         {
         UString x(key, keylen);

         table.insertAfterFind(x, value);
         }
      }

   bool setHeaderIfAbsent(const char* key, uint32_t keylen, const UString& value)
      {
      U_TRACE(0, "UMimeHeader::setHeaderIfAbsent(%.*S,%u,%V)", keylen, key, keylen, value.rep)

      if (containsHeader(key, keylen) == false)
         {
         UString x(key, keylen);

         table.insertAfterFind(x, value);

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // Writes all the headers to the supplied buffer

   UString getHeaders();
   void    writeHeaders(UString& buffer);

   // Host

   UString getHost()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getHost()")

      U_ASSERT(empty() == false)

      UString host = getHeader(U_CONSTANT_TO_PARAM("Host"));

      U_RETURN_STRING(host);
      }

   // Connection: close

   bool isClose()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::isClose()")

      U_ASSERT(empty() == false)

      if (getHeader(U_CONSTANT_TO_PARAM("Connection")).equal(U_CONSTANT_TO_PARAM("close")))

      U_RETURN(true);
      U_RETURN(false);
      }

   // Transfer-Encoding: chunked

   bool isChunked()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::isChunked()")

      U_ASSERT(empty() == false)

      if (getHeader(U_CONSTANT_TO_PARAM("Transfer-Encoding")) == *UString::str_chunked) U_RETURN(true);

      U_RETURN(false);
      }

   // Cookie

   UString getCookie()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getCookie()")

      U_ASSERT(empty() == false)

      UString cookie = getHeader(U_CONSTANT_TO_PARAM("Cookie"));

      U_RETURN_STRING(cookie);
      }

   // Set-Cookie

   bool isSetCookie()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::isSetCookie()")

      U_ASSERT(empty() == false)

      if (containsHeader(U_CONSTANT_TO_PARAM("Set-Cookie"))) U_RETURN(true);

      U_RETURN(false);
      }

   // Location

   UString getLocation()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getLocation()")

      U_ASSERT(empty() == false)

      UString location = getHeader(U_CONSTANT_TO_PARAM("Location"));

      U_RETURN_STRING(location);
      }

   // Refresh

   UString getRefresh()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getRefresh()")

      U_ASSERT(empty() == false)

      UString refresh = getHeader(U_CONSTANT_TO_PARAM("Refresh"));

      U_RETURN_STRING(refresh);
      }

   // Mime

   UString getMimeVersion()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getMimeVersion()")

      U_ASSERT(empty() == false)

      UString value = getHeader(U_CONSTANT_TO_PARAM("MIME-Version"));

      U_RETURN_STRING(value);
      }

   bool isMime()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::isMime()")

      if (table.empty()            == false &&
          getMimeVersion().empty() == false)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   /**
    * ===================================================================================================================
    * Content types:
    *   "text"        / [ "plain" (RFC-1521), "richtext" (RFC-1341), "enriched", "html", "xvcard", "vcal", "rtf", "xml" ],
    *   "audio"       / [ "basic" ],
    *   "video"       / [ "mpeg" ],
    *   "image"       / [ "jpeg", "gif" ],
    *   "message"     / [ "rfc822", "disposition-notification" ],
    *   "multipart"   / [ "mixed", "alternative", "digest", "parallel", "signed", "encrypted", "report", "form-data" ],
    *   "application" / [ "postscript", "octet-stream", "pgp-signature", "pgp-encrypted", "pgp-clearsigned",
    *                     "pkcs7-signature", "pkcs7-mime", "ms-tnef", "x-www-form-urlencoded" ]
    * ===================================================================================================================
    */

   UString getContentType()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getContentType()")

      U_ASSERT(empty() == false)

      UString content_type = getHeader(U_CONSTANT_TO_PARAM("Content-Type"));

      U_RETURN_STRING(content_type);
      }

   static bool isContentType(const UString& content_type, const char* type, uint32_t len, bool ignore_case = false)
      {
      U_TRACE(0, "UMimeHeader::isContentType(%V,%.*S,%u,%b)", content_type.rep, len, type, len, ignore_case)

      if (content_type)
         {
         if ((ignore_case ? u__strncasecmp(content_type.data(), type, len)
                          :        strncmp(content_type.data(), type, len)) == 0)
            {
            U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   static bool isType(const UString& content_type, const char* type, uint32_t len)
      {
      U_TRACE(0, "UMimeHeader::isType(%V,%.*S,%u)", content_type.rep, len, type, len)

      if (content_type)
         {
         uint32_t pos = content_type.find('/');

         if (pos                               != U_NOT_FOUND &&
             content_type.find(type, pos, len) != U_NOT_FOUND)
            {
            U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   static UString getCharSet(      const UString& content_type); // get charset/content-type info
   static UString shortContentType(const UString& content_type);

   static bool isMessage(    const UString& content_type) { return isContentType(content_type, U_CONSTANT_TO_PARAM("message")); }
   static bool isMultipart(  const UString& content_type) { return isContentType(content_type, U_CONSTANT_TO_PARAM("multipart")); }
   static bool isApplication(const UString& content_type) { return isContentType(content_type, U_CONSTANT_TO_PARAM("application")); }
   static bool isXML(        const UString& content_type) { return isContentType(content_type, U_CONSTANT_TO_PARAM("text/xml")); }
   static bool isText(       const UString& content_type) { return isContentType(content_type, U_STRING_TO_PARAM(*UString::str_txt_plain)); }
   static bool isRFC822(     const UString& content_type) { return isContentType(content_type, U_STRING_TO_PARAM(*UString::str_msg_rfc)); }

   static bool isPKCS7(            const UString& ctype)  { return isType(ctype, U_CONSTANT_TO_PARAM("pkcs7")); }
   static bool isURLEncoded(       const UString& ctype)  { return isType(ctype, U_CONSTANT_TO_PARAM("urlencoded")); }
   static bool isMultipartFormData(const UString& ctype)  { return isContentType(ctype, U_CONSTANT_TO_PARAM("multipart/form-data")); }

   static UString getBoundary(const UString& content_type)
      {
      U_TRACE(0, "UMimeHeader::getBoundary(%V)", content_type.rep)

      UString boundary = getValueAttributeFromKeyValue(content_type, U_CONSTANT_TO_PARAM("boundary"), false);

      U_RETURN_STRING(boundary);
      }

   UString getValueAttributeFromKey(const UString& key, const UString& name)
      {
      U_TRACE(0, "UMimeHeader::getValueAttributeFromKey(%V,%V)", key.rep, name.rep)

      U_ASSERT(empty() == false)

      UString value = getHeader(key);

      if (value) value = getValueAttributeFromKeyValue(value, name, false);

      U_RETURN_STRING(value);
      }

   static uint32_t getAttributeFromKeyValue(const UString& key_value, UVector<UString>& name_value);

   static UString getValueAttributeFromKeyValue(const UString& key_value, const UString& name_attr, bool ignore_case)
         { return getValueAttributeFromKeyValue(key_value, U_STRING_TO_PARAM(name_attr), ignore_case); }

   static UString getValueAttributeFromKeyValue(const UString& name_attr, UVector<UString>& name_value, bool ignore_case)
         { return getValueAttributeFromKeyValue(U_STRING_TO_PARAM(name_attr), name_value, ignore_case); }

   static UString getValueAttributeFromKeyValue(const UString& key_value, const char* name_attr, uint32_t name_attr_len,     bool ignore_case);
   static UString getValueAttributeFromKeyValue(const char* name_attr, uint32_t name_attr_len, UVector<UString>& name_value, bool ignore_case);

   // Disposition type (Content-Disposition header field, see RFC-1806): "inline", "attachment"

   UString getContentDisposition()
      {
      U_TRACE_NO_PARAM(0, "UMimeHeader::getContentDisposition()")

      U_ASSERT(empty() == false)

      UString content_disposition = getHeader(U_CONSTANT_TO_PARAM("Content-Disposition"));

      U_RETURN_STRING(content_disposition);
      }

   static bool getNames(const UString& cdisposition, UString& name, UString& filename);

   static UString getFileName(const UString& cdisposition) { return getValueAttributeFromKeyValue(cdisposition, U_CONSTANT_TO_PARAM("filename"), false); }

   // read from socket

   bool readHeader(USocket* socket, UString& data);

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, UMimeHeader& h);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UString header;
   UHashMap<UString> table;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UMimeHeader)

   friend class UHTTP;
   friend class UHTTP2;
};

#endif
