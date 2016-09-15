// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    header.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/mime/header.h>
#include <ulib/utility/uhttp.h>

//=============================================================================
// parsing an UString for MIME headers
// 
// Mime-type headers comprise a keyword followed by ":" followed
// by whitespace and the value. If a line begins with witespace it is
// a continuation of the preceeding line
//
// MIME headers are delimited by an empty line
//=============================================================================

uint32_t UMimeHeader::parse(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UMimeHeader::parse(%.*S,%u)", len, ptr, len)

   if ( len == 0 ||
       *ptr == '\n') // line empty: we have reached the MIME headers delimiter
      {
      U_RETURN(0);
      }

   int32_t sz;
   uint32_t n;
   const char* pkv;
   UString key, value;

   (void) header.replace(ptr, len);

   const char* prev = ptr = header.data();
   const char* _end = ptr + len;

   // Until we reach the data part of the response we know we are dealing with iso-8859-1 characters

   while (prev < _end)
      {
      U_INTERNAL_DUMP("prev = %.80S", prev)

      ptr = (const char*) memchr(prev, '\n', _end - prev);

      U_INTERNAL_DUMP("ptr = %.80S", ptr)

      if (ptr == 0) ptr = _end; // we have reached the MIME headers end without line empty...
      else
         {
         len = (ptr - prev);

         U_INTERNAL_DUMP("len = %u", len)

         if (len == 0 || u_isWhiteSpace(prev, len)) break; // line empty: we have reached the MIME headers delimiter...
         }

      bool cr = (ptr[-1] == '\r');

      U_INTERNAL_DUMP("cr = %b", cr)

      // Check for continuation of preceding header

      if (u__isspace(*prev))
         {
         do { ++prev; } while (prev < _end && u__isspace(*prev));

         U_INTERNAL_ASSERT_MINOR(prev,ptr)

         if (value)
            {
            value.append(prev, ptr - prev - cr);

            table.replaceAfterFind(value);
            }
         }
      else
         {
         pkv = (const char*) memchr(prev, ':', _end - prev);

         if (pkv == 0   ||
             pkv >= ptr ||
             pkv == prev)
            {
            break;
            }

         (void) key.assign(prev, pkv - prev);

         do { ++pkv; } while (pkv < _end && u__isspace(*pkv));

         sz = ptr - pkv;

         U_INTERNAL_DUMP("sz = %d pkv = %.80S", sz, pkv)

         if (sz > 0) (void) value.assign(pkv, sz - cr);
         else               value.clear();

         // Check for duplication of header

         if (containsHeader(key) == false) table.insertAfterFind(key, value);
         else
            {
            UStringRep* rep = table.elem();

            if (value.equal(rep) == false)
               {
               UString duplicate(rep->size() + 4U + key.size() + 2U);

               duplicate.snprintf(U_CONSTANT_TO_PARAM("%v%s%v: "), rep, (cr ? U_CRLF : U_LF), key.rep);

               U_INTERNAL_DUMP("duplicate = %V", duplicate.rep)

               table.replaceAfterFind(duplicate);
               }
            }
         }

      prev = ptr + 1;
      }

   n = table.size();

   U_RETURN(n);
}

// inlining failed in call to 'UMimeHeader::removeHeader(UString const&)': call is unlikely and code size would grow

void UMimeHeader::removeHeader(const char* key, uint32_t keylen)
{
   U_TRACE(0, "UMimeHeader::removeHeader(%.*S,%u)", keylen, key, keylen)

   if (containsHeader(key, keylen)) table.eraseAfterFind();
}

uint32_t UMimeHeader::getAttributeFromKeyValue(const UString& key_value, UVector<UString>& name_value)
{
   U_TRACE(0, "UMimeHeader::getAttributeFromKeyValue(%V,%p)", key_value.rep, &name_value)

   uint32_t n = 0;

   if (key_value)
      {
      uint32_t pos = key_value.find(';'); // Content-Disposition: form-data; name="input_file"; filename="/tmp/4dcd39e8-2a84-4242-b7bc-ca74922d26e1"

      if (pos != U_NOT_FOUND)
         {
         // NB: we must use substr() because of the possibility of unQuote()...

         n = name_value.split(key_value.substr(pos + 1), " =;"); // NB: I can't use also '"' char...

         if ((n & 1)) // NB: it is even...
            {
            (void) name_value.pop();

            n -= 1;
            }
         }
      }

   U_RETURN(n);
}

UString UMimeHeader::getValueAttributeFromKeyValue(const char* name_attr, uint32_t name_attr_len, UVector<UString>& name_value, bool ignore_case)
{
   U_TRACE(0, "UMimeHeader::getValueAttributeFromKeyValue(%.*S,%u,%p,%b)", name_attr_len, name_attr, name_attr_len, &name_value, ignore_case)

   UString name, value;

   for (int32_t i = 0, n = name_value.size(); i < n; i += 2)
      {
      name = name_value[i];

      U_INTERNAL_DUMP("name = %V", name.rep)

      if (UStringRep::equal_lookup(name.rep, name_attr, name_attr_len, ignore_case))
         {
         value = name_value[i+1];

         if (value.isQuoted()) value.rep->unQuote();

         break;
         }
      }

   U_INTERNAL_DUMP("value = %V", value.rep)

   U_RETURN_STRING(value);
}

UString UMimeHeader::getValueAttributeFromKeyValue(const UString& value, const char* name_attr, uint32_t name_attr_len, bool ignore_case)
{
   U_TRACE(0, "UMimeHeader::getValueAttributeFromKeyValue(%V,%.*S,%u,%b)", value.rep, name_attr_len, name_attr, name_attr_len, ignore_case)

   UString result;
   UVector<UString> name_value;

   if (getAttributeFromKeyValue(value, name_value)) result = getValueAttributeFromKeyValue(name_attr, name_attr_len, name_value, ignore_case);

   U_RETURN_STRING(result);
}

bool UMimeHeader::getNames(const UString& cdisposition, UString& name, UString& filename)
{
   U_TRACE(0, "UMimeHeader::getNames(%V,%p,%p)", cdisposition.rep, &name, &filename)

   bool result = false;
   UVector<UString> name_value;

   // Content-Disposition: form-data; name="input_file"; filename="/tmp/4dcd39e8-2a84-4242-b7bc-ca74922d26e1"

   if (getAttributeFromKeyValue(cdisposition, name_value))
      {
      for (int32_t i = 0, n = name_value.size(); i < n; i += 2)
         {
         if (name_value[i].equal(U_CONSTANT_TO_PARAM("name")))
            {
            name = name_value[i+1];

            U_INTERNAL_DUMP("name_value[%d] = %V", i+1, name.rep)

            if (name.isQuoted()) name.rep->unQuote();

            U_INTERNAL_DUMP("name = %V", name.rep)
            }
         else if (name_value[i].equal(U_CONSTANT_TO_PARAM("filename")))
            {
            filename = name_value[i+1];

            U_INTERNAL_DUMP("name_value[%d] = %V", i+1, filename.rep)

            if (filename)
               {
               result = true;

               if (filename.isQuoted()) filename.rep->unQuote();

               uint32_t len    = filename.size();
               const char* ptr = filename.data();

               // This is hairy: Netscape and IE don't encode the filenames
               // The RFC says they should be encoded, so I will assume they are

               if (u_isUrlEncoded(ptr, len, false) &&
                       u_isBase64(ptr, len) == false)
                  {
                  UString filename_decoded(len);

                  Url::decode(ptr, len, filename_decoded);

                  filename = filename_decoded;
                  }

               U_INTERNAL_DUMP("filename = %V", filename.rep)
               }
            }
         }
      }

   U_RETURN(result);
}

UString UMimeHeader::getCharSet(const UString& content_type)
{
   U_TRACE(0, "UMimeHeader::getCharSet(%V)", content_type.rep)

   U_INTERNAL_ASSERT(content_type)

   UString charset = getValueAttributeFromKeyValue(content_type, U_CONSTANT_TO_PARAM("charset"), false);

   // For some RFC, encoding is us-ascii if it's not specifief in header

   if (charset.empty()) (void) charset.assign(U_CONSTANT_TO_PARAM("us-ascii"));

   U_INTERNAL_DUMP("charset = %V", charset.rep)

   U_RETURN_STRING(charset);
}

UString UMimeHeader::shortContentType(const UString& content_type)
{
   U_TRACE(0, "UMimeHeader::shortContentType(%V)", content_type.rep)

   U_INTERNAL_ASSERT(content_type)

   uint32_t pos = content_type.find(';');

   if (pos != U_NOT_FOUND)
      {
      UString result = content_type.substr(0U, pos);

      U_RETURN_STRING(result);
      }

   U_RETURN_STRING(content_type);
}

void UMimeHeader::writeHeaders(UString& buffer)
{
   U_TRACE(0, "UMimeHeader::writeHeaders(%V)", buffer.rep)

   UString tmp(U_CAPACITY);

   if (table.first())
      {
      do {
         const UStringRep* key = table.key();
         UStringRep* value     = table.elem();

         tmp.setBuffer(key->size() + 2 + value->size() + 2);

         tmp.snprintf(U_CONSTANT_TO_PARAM("%v: %v\r\n"), key, value);

         buffer += tmp;
         }
      while (table.next());
      }

   U_INTERNAL_DUMP("buffer = %V", buffer.rep)
}

UString UMimeHeader::getHeaders()
{
   U_TRACE_NO_PARAM(0, "UMimeHeader::getHeaders()")

   if (empty() == false)
      {
      UString buffer(U_CAPACITY);

      writeHeaders(buffer);

      (void) buffer.shrink();

      U_RETURN_STRING(buffer);
      }

   return UString::getStringNull();
}

// read from socket

bool UMimeHeader::readHeader(USocket* socket, UString& data)
{
   U_TRACE(0, "UMimeHeader::readHeader(%p,%p)", socket, &data)

   U_ASSERT(empty())

   if (UHTTP::readHeaderResponse(socket, data) &&
       (parse(data.c_pointer(U_http_info.startHeader), U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF2) - U_http_info.startHeader) > 0))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, UMimeHeader& h)
{
   U_TRACE(0+256, "UMimeHeader::operator<<(%p,%p)", &os, &h)

   if (h.table.first())
      {
      do {
         const UStringRep* key = h.table.key();
         UStringRep* value     = h.table.elem();

         os.write(key->data(), key->size());
         os.write(": ", 2);
         os.write(value->data(), value->size());
         os.write(U_CONSTANT_TO_PARAM(U_CRLF));
         }
      while (h.table.next());
      }

   os.write(U_CONSTANT_TO_PARAM(U_CRLF));

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UMimeHeader::dump(bool reset) const
{
   *UObjectIO::os << "table     (UHashMap " << (void*)&table  << ")\n"
                  << "header    (UString  " << (void*)&header << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
