// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    transform.cpp - xml Digital SIGnature with libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/base64.h>
#include <ulib/utility/services.h>

#include "xpath.h"

// the allowed transforms usages
// the transform's name
// the transform's identification string (href)

int         UTranformBase64::_usage    = DSIG;
const char* UTranformBase64::_name     = "base64";
const char* UTranformBase64::_href     = "http://www.w3.org/2000/09/xmldsig#base64";

int         UTranformInputURI::_usage  = DSIG;
const char* UTranformInputURI::_name   = "input-uri";
const char* UTranformInputURI::_href   = "";

UVector<UIOCallback*>* UTranformInputURI::allIOCallbacks;

int         UTranformInclC14N::_usage  = DSIG|C14N;
const char* UTranformInclC14N::_name   = "c14n";
const char* UTranformInclC14N::_href   = "http://www.w3.org/2006/12/xml-c14n11"; // "http://www.w3.org/TR/2001/REC-xml-c14n-20010315";

int         UTranformXPointer::_usage  = DSIG;
const char* UTranformXPointer::_name   = "xpointer";
const char* UTranformXPointer::_href   = "http://www.w3.org/2001/04/xmldsig-more/xptr";

UXML2Document* UTranformXPointer::document;

int         UTranformSha1::_usage      = DIGEST;
const char* UTranformSha1::_name       = "sha1";
const char* UTranformSha1::_href       = "http://www.w3.org/2001/04/xmlenc#sha1";

int         UTranformSha256::_usage    = DIGEST;
const char* UTranformSha256::_name     = "sha256";
const char* UTranformSha256::_href     = "http://www.w3.org/2001/04/xmlenc#sha256";

int         UTranformRsaMd5::_usage    = SIGNATURE;
const char* UTranformRsaMd5::_name     = "rsa-md5";
const char* UTranformRsaMd5::_href     = "http://www.w3.org/2001/04/xmldsig-more#rsa-md5";

int         UTranformRsaSha1::_usage   = SIGNATURE;
const char* UTranformRsaSha1::_name    = "rsa-sha1";
const char* UTranformRsaSha1::_href    = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha1";

int         UTranformRsaSha256::_usage = SIGNATURE;
const char* UTranformRsaSha256::_name  = "rsa-sha256";
const char* UTranformRsaSha256::_href  = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";

UTranformXPointer::~UTranformXPointer()
{
   U_TRACE_UNREGISTER_OBJECT(0, UTranformXPointer)

   dataList.clear();
}

UBaseTransform::UBaseTransform()
{
   U_TRACE_REGISTER_OBJECT(0, UBaseTransform, "")

   status    = 0;
   hereNode  = 0;
   operation = 0;
}

UBaseTransform::~UBaseTransform()
{
   U_TRACE_UNREGISTER_OBJECT(0, UBaseTransform)
}

bool UTranformXPointer::setExpr(const char* expr, int nodeSetType, xmlNodePtr node)
{
   U_TRACE(0, "UTranformXPointer::setExpr(%S,%d,%p)", expr, nodeSetType, node)

   UBaseTransform::hereNode = node;

   UXPathData* data;

   U_NEW(UXPathData, data, UXPathData(UXPathData::XPOINTER, nodeSetType, expr));

   if (data->registerNamespaces(node))
      {
      dataList.push(data);

      U_RETURN(true);
      }

   delete data;

   U_RETURN(false);
}

// Opens the given @uri for reading

UTranformInputURI::UTranformInputURI(const char* uri)
{
   U_TRACE_REGISTER_OBJECT(0, UTranformInputURI, "%S", uri)

   clbks    = 0;
   clbksCtx = 0;

   /*
    * Try to find one of the input accept method accepting that scheme
    * Go in reverse to give precedence to user defined handlers.
    * try with an unescaped version of the uri
    */

   char* unescaped = U_SYSCALL(xmlURIUnescapeString, "%S,%d,%S", uri, 0, NULL);

   if (unescaped != 0)
      {
      clbks = find(unescaped);

      if (clbks) clbksCtx = clbks->opencallback(unescaped);

      U_SYSCALL_VOID(xmlFree, "%p", unescaped);
      }

   // If this failed try with a non-escaped uri this may be a strange filename

   if (clbks == 0)
      {
      clbks = find(uri);

      if (clbks) clbksCtx = clbks->opencallback(uri);
      }
}

UIOCallback* UTranformInputURI::find(const char* uri)
{
   U_TRACE(0, "UTranformInputURI::find(%S)", uri)

   U_INTERNAL_ASSERT_POINTER(allIOCallbacks)

   UIOCallback* callbacks;

   for (uint32_t i = 0, n = allIOCallbacks->size(); i < n; ++i)
      {
      callbacks = (*allIOCallbacks)[i];

      if (callbacks->matchcallback(uri)) U_RETURN_POINTER(callbacks, UIOCallback);
      }

   U_RETURN_POINTER(0, UIOCallback);
}

/**
 * Process binary @data by calling transform's execute method and pushes 
 * results to next transform.
 *
 * Returns: true on success or a false value if an error occurs.
 */

bool UTranformBase64::execute(UString& data)
{
   U_TRACE(0, "UTranformBase64::execute(%.*S)", U_STRING_TO_TRACE(data))

   UString buffer(data.size());

   UBase64::decode(data, buffer);

   if (buffer)
      {
      data = buffer;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UTranformInclC14N::execute(UString& data)
{
   U_TRACE(0, "UTranformInclC14N::execute(%.*S)", U_STRING_TO_TRACE(data))

   data = UXML2Document::xmlC14N(data);

   U_RETURN(true);
}

bool UTranformSha1::execute(UString& data)
{
   U_TRACE(0, "UTranformSha1::execute(%.*S)", U_STRING_TO_TRACE(data))

   UString ObjectDigestValue(200U);

   UServices::generateDigest(U_HASH_SHA1, 0, data, ObjectDigestValue, true);

   data = ObjectDigestValue;

   U_RETURN(true);
}

bool UTranformSha256::execute(UString& data)
{
   U_TRACE(0, "UTranformSha256::execute(%.*S)", U_STRING_TO_TRACE(data))

   UString ObjectDigestValue(200U);

   UServices::generateDigest(U_HASH_SHA256, 0, data, ObjectDigestValue, true);

   data = ObjectDigestValue;

   U_RETURN(true);
}

bool UTranformRsaMd5::execute(UString& data)
{
   U_TRACE(0, "UTranformRsaMd5::execute(%.*S)", U_STRING_TO_TRACE(data))

   U_RETURN(false);
}

bool UTranformRsaSha1::execute(UString& data)
{
   U_TRACE(0, "UTranformRsaSha1::execute(%.*S)", U_STRING_TO_TRACE(data))

   U_RETURN(false);
}

bool UTranformRsaSha256::execute(UString& data)
{
   U_TRACE(0, "UTranformRsaSha256::execute(%.*S)", U_STRING_TO_TRACE(data))

   U_RETURN(false);
}

bool UTranformXPointer::execute(UString& data)
{
   U_TRACE(0, "UTranformXPointer::execute(%.*S)", U_STRING_TO_TRACE(data))

   U_INTERNAL_DUMP("dataList.size() = %u", dataList.size())

   if (tag.empty())
      {
      UXPathData* pdata = dataList[0];

      xmlXPathObjectPtr xpathObj = (xmlXPathObjectPtr) U_SYSCALL(xmlXPtrEval, "%S,%p", (const xmlChar*)pdata->expr, pdata->ctx);

      if (xpathObj == 0) U_RETURN(false);

      // ...
      }
   else
      {
      U_INTERNAL_ASSERT_POINTER(document)

      (void) document->getElement(data, 128, U_STRING_TO_PARAM(tag));
      }

   U_RETURN(true);
}

bool UTranformInputURI::execute(UString& data)
{
   U_TRACE(0, "UTranformInputURI::execute(%.*S)", U_STRING_TO_TRACE(data))

   U_INTERNAL_DUMP("clbksCtx = %p clbks = %p", clbksCtx, clbks)

   if (clbksCtx &&
       clbks)
      {
      data.setBuffer(128 * 1024);

      int ret = (clbks->readcallback)( clbksCtx, (char*)data.data(), (int)data.capacity());
                (clbks->closecallback)(clbksCtx);

      if (ret > 0)
         {
         data.size_adjust(ret);

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UBaseTransform::dump(bool reset) const
{
   *UObjectIO::os << "status    " << status   << '\n'
                  << "hereNode  " << hereNode << '\n'
                  << "operation " << operation;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UIOCallback::dump(bool reset) const
{
   *UObjectIO::os << "opencallback  " << (void*)opencallback  << '\n'
                  << "readcallback  " << (void*)readcallback  << '\n'
                  << "closecallback " << (void*)closecallback << '\n'
                  << "matchcallback " << (void*)matchcallback;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
