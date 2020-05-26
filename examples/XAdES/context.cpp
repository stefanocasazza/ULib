// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    context.cpp - xml Digital SIGnature with libxml2 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/base64.h>

#include "xpath.h"
#include "context.h"

#ifdef LIBXML_FTP_ENABLED 
#  include <libxml/nanoftp.h>
#endif
#ifdef LIBXML_HTTP_ENABLED
#  include <libxml/nanohttp.h>
#endif

UDSIGContext*     UDSIGContext::pthis;
UVector<UString>* UTransformCtx::enabledTransforms;

void UTransformCtx::registerDefault()
{
   U_TRACE(0, "UTransformCtx::registerDefault()")

   U_INTERNAL_ASSERT_POINTER(enabledTransforms)

   enabledTransforms->push_back(UString(UTranformBase64::_name, strlen(UTranformBase64::_name)));               // "base64"
   enabledTransforms->push_back(UString(UTranformBase64::_href, strlen(UTranformBase64::_href)));

   enabledTransforms->push_back(UString(UTranformInclC14N::_name, strlen(UTranformInclC14N::_name)));             // "c14n"
   enabledTransforms->push_back(UString(UTranformInclC14N::_href, strlen(UTranformInclC14N::_href)));

   enabledTransforms->push_back(UString(UTranformXPointer::_name, strlen(UTranformXPointer::_name)));             // "xpointer"
   enabledTransforms->push_back(UString(UTranformXPointer::_href, strlen(UTranformXPointer::_href)));

   /*
   enabledTransforms->push_back(UString(UTranformEnveloped::_name));            // "enveloped-signature"
   enabledTransforms->push_back(UString(UTranformInclC14NWithComment::_name));  // "c14n-with-comments"
   enabledTransforms->push_back(UString(UTranformInclC14N11::_name));           // "c14n11"
   enabledTransforms->push_back(UString(UTranformInclC14N11WithComment::_name));// "c14n11-with-comments"
   enabledTransforms->push_back(UString(UTranformExclC14N::_name));             // "exc-c14n"
   enabledTransforms->push_back(UString(UTranformExclC14NWithComment::_name));  // "exc-c14n-with-comments"
   enabledTransforms->push_back(UString(UTranformXPath::_name));                // "xpath"
   enabledTransforms->push_back(UString(UTranformXPath2::_name));               // "xpath2"
   enabledTransforms->push_back(UString(UTranformXslt::_name));                 // "xslt"
   */

   enabledTransforms->push_back(UString(UTranformSha1::_name, strlen(UTranformSha1::_name)));                 // "sha1"
   enabledTransforms->push_back(UString(UTranformSha1::_href, strlen(UTranformSha1::_href)));  

   enabledTransforms->push_back(UString(UTranformSha256::_name, strlen(UTranformSha256::_name)));               // "sha256"
   enabledTransforms->push_back(UString(UTranformSha256::_href, strlen(UTranformSha256::_href)));  

   enabledTransforms->push_back(UString(UTranformRsaMd5::_name, strlen(UTranformRsaMd5::_name)));               // "rsa-md5"
   enabledTransforms->push_back(UString(UTranformRsaMd5::_href, strlen(UTranformRsaMd5::_href)));

   enabledTransforms->push_back(UString(UTranformRsaSha1::_name, strlen(UTranformRsaSha1::_name)));              // "rsa-sha1"
   enabledTransforms->push_back(UString(UTranformRsaSha1::_href, strlen(UTranformRsaSha1::_href)));

   enabledTransforms->push_back(UString(UTranformRsaSha256::_name, strlen(UTranformRsaSha256::_name)));            // "rsa-sha256"
   enabledTransforms->push_back(UString(UTranformRsaSha256::_href, strlen(UTranformRsaSha256::_href)));

   /*
   enabledTransforms->push_back(UString(UTranform::_name));                     // "aes128-cbc"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "aes192-cbc"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "aes256-cbc"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "kw-aes128"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "kw-aes192"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "kw-aes256"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "tripledes-cbc"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "kw-tripledes"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "dsa-sha1"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-md5"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-ripemd160"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-sha1"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-sha224"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-sha256"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-sha384"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "hmac-sha512"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "md5"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "ripemd160"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "rsa-ripemd160"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "rsa-sha224"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "rsa-sha384"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "rsa-sha512"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "rsa-1_5"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "rsa-oaep-mgf1p"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "sha224"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "sha384"
   enabledTransforms->push_back(UString(UTranform::_name));                     // "sha512"
   */
}

UBaseTransform* UTransformCtx::findByHref(const char* href)
{
   U_TRACE(0, "UTransformCtx::findByHref(%S)", href)

   /* check with enabled transforms list */

   uint32_t i = enabledTransforms->find(href, u__strlen(href, __PRETTY_FUNCTION__));

   if (i == U_NOT_FOUND) U_RETURN_POINTER(U_NULLPTR, UBaseTransform);

   UBaseTransform* ptr;

   switch (i)
      {
      case  1: U_NEW(UTranformBase64,    ptr, UTranformBase64);    break; // "base64"
      case  3: U_NEW(UTranformInclC14N,  ptr, UTranformInclC14N);  break; // "c14n"
      case  5: U_NEW(UTranformXPointer,  ptr, UTranformXPointer);  break; // "xpointer"
      case  7: U_NEW(UTranformSha1,      ptr, UTranformSha1);      break; // "sha1"
      case  9: U_NEW(UTranformSha256,    ptr, UTranformSha256);    break; // "sha256"
      case 11: U_NEW(UTranformRsaMd5,    ptr, UTranformRsaMd5);    break; // "rsa-md5"
      case 13: U_NEW(UTranformRsaSha1,   ptr, UTranformRsaSha1);   break; // "rsa-sha1"
      case 15: U_NEW(UTranformRsaSha256, ptr, UTranformRsaSha256); break; // "rsa-sha256"
      default:                           ptr = U_NULLPTR;          break;
      }

   U_INTERNAL_ASSERT_POINTER(ptr)
// U_INTERNAL_ASSERT_EQUALS(strcmp(ptr->href(), href), 0)

   U_RETURN_POINTER(ptr, UBaseTransform);
}

/**
 * @node:  the pointer to the transform's node.
 * @usage: the transform usage (signature, encryption, ...).
 *
 * Reads transform from the @node as follows:
 *
 *    1) reads "Algorithm" attribute;
 *
 *    2) checks the lists of known and allowed transforms;
 *
 *    3) calls transform's read transform node method.
 *
 * Returns: pointer to newly created transform or NULL if an error occurs.
 */

UBaseTransform* UTransformCtx::nodeRead(xmlNodePtr node, int usage)
{
   U_TRACE(0, "UTransformCtx::nodeRead(%p,%d)", node, usage)

   U_INTERNAL_ASSERT_POINTER(node)
   U_INTERNAL_ASSERT_POINTER(UDSIGContext::pthis)
   U_INTERNAL_ASSERT_EQUALS(UDSIGContext::pthis->status, UReferenceCtx::UNKNOWN)

   const char* href = UXML2Node::getProp(node, "Algorithm");

   if (href)
      {
      UBaseTransform* id = findByHref(href);

      if (id)
         {
         if ((id->usage() & usage) != 0 &&
              id->readNode(node))
            {
            id->hereNode = node;

            U_RETURN_POINTER(id, UBaseTransform);
            }

         U_DELETE(id)
         }
      }

   U_RETURN_POINTER(U_NULLPTR, UBaseTransform);
}

// Reads transforms from the <dsig:Transform/> children of the @node and 
// appends them to the current transforms chain in ctx object.

bool UTransformCtx::nodesListRead(xmlNodePtr node, int usage)
{
   U_TRACE(0, "UTransformCtx::nodesListRead(%p,%d)", node, usage)

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"Transforms",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      UBaseTransform* id = nodeRead(cur, usage);

      if (id == U_NULLPTR) U_RETURN(false);

      chain.push_back(id);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   if (cur) U_RETURN(false);

   U_RETURN(true);
}

UDSIGContext::UDSIGContext()
{
   U_TRACE_CTOR(0, UDSIGContext, "")

   U_INTERNAL_ASSERT_EQUALS(pthis, U_NULLPTR)

   id                   = U_NULLPTR;
   pthis                = this;
   status               = UReferenceCtx::UNKNOWN;  // the <dsig:Signature/> processing status.
   operation            = 0;                       // the operation: sign or verify.
   signMethod           = U_NULLPTR;               // the pointer to signature transform.
   c14nMethod           = U_NULLPTR;               // the pointer to c14n transform.
   keyInfoNode          = U_NULLPTR;               // the pointer to <dsig:keyInfo/> node.
   signValueNode        = U_NULLPTR;               // the pointer to <dsig:SignatureValue/> node.
   signedInfoNode       = U_NULLPTR;               // the pointer to <dsig:signedInfo/> node.
   enabledReferenceUris = 0;                       // the URI types allowed for <dsig:Reference/> node.

   // Registers the default transforms compiled-in handlers.

   U_INTERNAL_ASSERT_EQUALS(UTransformCtx::enabledTransforms, U_NULLPTR)

   U_NEW(UVector<UString>, UTransformCtx::enabledTransforms, UVector<UString>);

   UTransformCtx::registerDefault();

   // Registers the default compiled-in I/O handlers.

   U_INTERNAL_ASSERT_EQUALS(UTranformInputURI::allIOCallbacks, U_NULLPTR)

   U_NEW(UVector<UIOCallback*>, UTranformInputURI::allIOCallbacks, UVector<UIOCallback*>);

   UIOCallback* p;

#ifdef LIBXML_FTP_ENABLED       
   U_SYSCALL_VOID_NO_PARAM(xmlNanoFTPInit);

   U_NEW(UIOCallback, p, UIOCallback(xmlIOFTPMatch, xmlIOFTPOpen, xmlIOFTPRead, xmlIOFTPClose));

   UTranformInputURI::allIOCallbacks->push_back(p);
#endif

#ifdef LIBXML_HTTP_ENABLED
   U_SYSCALL_VOID_NO_PARAM(xmlNanoHTTPInit);

   U_NEW(UIOCallback, p, UIOCallback(xmlIOHTTPMatch, xmlIOHTTPOpen, xmlIOHTTPRead, xmlIOHTTPClose));

   UTranformInputURI::allIOCallbacks->push_back(p);
#endif

   U_NEW(UIOCallback, p, UIOCallback(xmlFileMatch, xmlFileOpen, xmlFileRead, xmlFileClose));

   UTranformInputURI::allIOCallbacks->push_back(p);
}

UDSIGContext::~UDSIGContext()
{
   U_TRACE_DTOR(0, UDSIGContext)

   U_DELETE(UTransformCtx::enabledTransforms)

#ifdef LIBXML_FTP_ENABLED       
   U_SYSCALL_VOID_NO_PARAM(xmlNanoFTPCleanup);
#endif
#ifdef LIBXML_HTTP_ENABLED
   U_SYSCALL_VOID_NO_PARAM(xmlNanoHTTPCleanup);
#endif

   U_DELETE(UTranformInputURI::allIOCallbacks)
}

/**
 * The Manifest Element (http://www.w3.org/TR/xmldsig-core/#sec-Manifest)
 *
 * The Manifest element provides a list of References. The difference from 
 * the list in SignedInfo is that it is application defined which, if any, of 
 * the digests are actually checked against the objects referenced and what to 
 * do if the object is inaccessible or the digest compare fails. If a Manifest 
 * is pointed to from SignedInfo, the digest over the Manifest itself will be 
 * checked by the core result validation behavior. The digests within such 
 * a Manifest are checked at the application's discretion. If a Manifest is 
 * referenced from another Manifest, even the overall digest of this two level 
 * deep Manifest might not be checked.
 *     
 * Schema Definition:
 *     
 * <element name="Manifest" type="ds:ManifestType"/> 
 * <complexType name="ManifestType">
 *   <sequence>
 *     <element ref="ds:Reference" maxOccurs="unbounded"/> 
 *   </sequence> 
 *   <attribute name="Id" type="ID" use="optional"/> 
 *  </complexType>
 * 
 * DTD:
 *
 * <!ELEMENT Manifest (Reference+)  >
 * <!ATTLIST Manifest Id ID  #IMPLIED >
 */

bool UDSIGContext::processManifestNode(xmlNodePtr node)
{
   U_TRACE(0, "UDSIGContext::processManifestNode(%p)", node)

   UReferenceCtx* ref;

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Reference",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      /* create reference */

      U_NEW(UReferenceCtx, ref, UReferenceCtx(UReferenceCtx::MANIFEST));

      /* add to the list */

      manifestReferences.push_back(ref);

      /* process */

      if (ref->processNode(cur) == false) U_RETURN(false);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // if there is something left than it's an error

   if (cur) U_RETURN(false);

   U_RETURN(true);
}

/**
 * The Object Element (http://www.w3.org/TR/xmldsig-core/#sec-Object)
 * 
 * Object is an optional element that may occur one or more times. When 
 * present, this element may contain any data. The Object element may include 
 * optional MIME type, ID, and encoding attributes.
 *     
 * Schema Definition:
 *     
 * <element name="Object" type="ds:ObjectType"/> 
 * <complexType name="ObjectType" mixed="true">
 *   <sequence minOccurs="0" maxOccurs="unbounded">
 *     <any namespace="##any" processContents="lax"/>
 *   </sequence>
 *   <attribute name="Id" type="ID" use="optional"/> 
 *   <attribute name="MimeType" type="string" use="optional"/>
 *   <attribute name="Encoding" type="anyURI" use="optional"/> 
 * </complexType>
 * 
 * DTD:
 * 
 * <!ELEMENT Object (#PCDATA|Signature|SignatureProperties|Manifest %Object.ANY;)* >
 * <!ATTLIST Object  Id  ID  #IMPLIED 
 *                   MimeType    CDATA   #IMPLIED 
 *                   Encoding    CDATA   #IMPLIED >
 */

bool UDSIGContext::processObjectNode(xmlNodePtr node)
{
   U_TRACE(0, "UDSIGContext::processObjectNode(%p)", node)

   /* we care about Manifest nodes only; ignore everything else */

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Manifest",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      if (processManifestNode(cur) == false) U_RETURN(false);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   U_RETURN(true);
}

bool UDSIGContext::processKeyInfoNode()
{
   U_TRACE(0, "UDSIGContext::processKeyInfoNode()")

   U_RETURN(true);
}

/**
 * The Signature element (http://www.w3.org/TR/xmldsig-core/#sec-Signature)
 *
 * The Signature element is the root element of an XML Signature. 
 * Implementation MUST generate laxly schema valid [XML-schema] Signature 
 * elements as specified by the following schema:
 * The way in which the SignedInfo element is presented to the 
 * canonicalization method is dependent on that method. The following 
 * applies to algorithms which process XML as nodes or characters:
 *
 *  - XML based canonicalization implementations MUST be provided with 
 *  a [XPath] node-set originally formed from the document containing 
 *  the SignedInfo and currently indicating the SignedInfo, its descendants,
 *  and the attribute and namespace nodes of SignedInfo and its descendant 
 *  elements.
 *
 *  - Text based canonicalization algorithms (such as CRLF and charset 
 *  normalization) should be provided with the UTF-8 octets that represent 
 *  the well-formed SignedInfo element, from the first character to the 
 *  last character of the XML representation, inclusive. This includes 
 *  the entire text of the start and end tags of the SignedInfo element 
 *  as well as all descendant markup and character data (i.e., the text) 
 *  between those tags. Use of text based canonicalization of SignedInfo 
 *  is NOT RECOMMENDED.         
 *
 *  =================================
 *  we do not support any non XML based C14N 
 *
 * Schema Definition:
 *
 *  <element name="Signature" type="ds:SignatureType"/>
 *  <complexType name="SignatureType">
 *  <sequence> 
 *     <element ref="ds:SignedInfo"/> 
 *     <element ref="ds:SignatureValue"/> 
 *     <element ref="ds:KeyInfo" minOccurs="0"/> 
 *     <element ref="ds:Object" minOccurs="0" maxOccurs="unbounded"/> 
 *     </sequence> <attribute name="Id" type="ID" use="optional"/>
 *  </complexType>
 *
 * DTD:
 *
 *  <!ELEMENT Signature (SignedInfo, SignatureValue, KeyInfo?, Object*)  >
 *  <!ATTLIST Signature
 *      xmlns   CDATA   #FIXED 'http://www.w3.org/2000/09/xmldsig#'
 *      Id      ID  #IMPLIED >
 */

bool UDSIGContext::processSignatureNode(xmlNodePtr signature, const char*& alg, UString& data)
{
   U_TRACE(0, "UDSIGContext::processSignatureNode(%p,%p,%.*S)", signature, alg, U_STRING_TO_TRACE(data))

   U_INTERNAL_ASSERT_POINTER(signature)
   U_INTERNAL_ASSERT_EQUALS(signValueNode, U_NULLPTR)
   U_INTERNAL_ASSERT(operation == UBaseTransform::VERIFY)
   U_INTERNAL_ASSERT_EQUALS(status, UReferenceCtx::UNKNOWN)

   xmlNodePtr cur;

   /* read node data */

   id = UXML2Node::getProp(signature, "Id");

   // first node is required SignedInfo

   signedInfoNode = UXML2Node::getNextSibling(signature->children);

   if (UXML2Node::checkNodeName(signedInfoNode, (const xmlChar*)"SignedInfo",
                                                (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) U_RETURN(false);

   // next node is required SignatureValue

   signValueNode = UXML2Node::getNextSibling(signedInfoNode->next);

   if (UXML2Node::checkNodeName(signValueNode, (const xmlChar*)"SignatureValue",
                                               (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) U_RETURN(false);

   // next node is optional KeyInfo

   keyInfoNode = UXML2Node::getNextSibling(signValueNode->next);

   if (UXML2Node::checkNodeName(keyInfoNode, (const xmlChar*)"KeyInfo",
                                             (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      cur = UXML2Node::getNextSibling(keyInfoNode->next);
      }
   else
      {
      cur         = keyInfoNode;
      keyInfoNode = U_NULLPTR;
      }

   // next nodes are optional Object nodes

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Object",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      /* read manifests from objects */

      if (processObjectNode(cur) == false) U_RETURN(false);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // if there is something left than it's an error

   if (cur) U_RETURN(false);

   // now validated all the references and prepare transform

   if (processSignedInfoNode(alg, data) == false) U_RETURN(false);

   /* references processing might change the status */

   if (status != UReferenceCtx::UNKNOWN) U_RETURN(true);

   /* as the result, we should have sign and c14n methods set */

   U_INTERNAL_ASSERT_POINTER(c14nMethod)
   U_INTERNAL_ASSERT_POINTER(signMethod)

   if (processKeyInfoNode() == false) U_RETURN(false);

   U_RETURN(true);
}

/**
 * @node: the pointer to node.
 *
 * Gets the @node content, base64 decodes it and calls transform->verify()
 * function to verify binary results.
 *
 * Returns: true on success or false if an error occurs.
 */

bool UTransformCtx::verifyNodeContent(xmlNodePtr node, UString& signature_value)
{
   U_TRACE(0, "UTransformCtx::verifyNodeContent(%p,%.*S)", node, U_STRING_TO_TRACE(signature_value))

   U_INTERNAL_ASSERT_POINTER(node)

   const char* content = (const char*) UXML2Node::getContent(node);

   uint32_t size = u__strlen(content, __PRETTY_FUNCTION__);

   (void) signature_value.reserve(size);

   UBase64::decode(content, size, signature_value);

   if (signature_value) U_RETURN(true);

   U_RETURN(false);
}

/**
 * Executes transforms chain in ctx.
 *
 * Returns: true on success or false otherwise.
 */

bool UTransformCtx::execute(UString& data)
{
   U_TRACE(0, "UTransformCtx::execute(%.*S)", U_STRING_TO_TRACE(data))

   U_INTERNAL_ASSERT_EQUALS(status, 0)

   U_INTERNAL_DUMP("uri = %S xptrExpr = %S status = %d enabledUris = %d chain.size() = %u", uri, xptrExpr, status, enabledUris, chain.size())

   UBaseTransform* elem;

   for (uint32_t i = 0, n = chain.size(); i < n; ++i)
      {
      elem = chain[i];

      if (elem->execute(data) == false) U_RETURN(false);
      }

   U_RETURN(true);
}

/**
 * Validates signature in the document.
 *
 * The verification result is returned in #status member of the object.
 *
 * Returns: true on success
 */

bool UDSIGContext::verify(UXML2Document& document, const char*& alg, UString& data, UString& signature_value)
{
   U_TRACE(0, "UDSIGContext::verify(%p,%p,%.*S,%.*S)", &document, alg, U_STRING_TO_TRACE(data), U_STRING_TO_TRACE(signature_value))

   operation                   = UBaseTransform::VERIFY;
   UTranformXPointer::document = &document;

   // find signature node (the Signature element is the root element of an XML Signature)

   xmlNodePtr signature = document.findNode(document.getRootNode(),
                                            (const xmlChar*)"Signature",
                                            (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#");

   /* read signature info */

   if (signature == U_NULLPTR ||
       processSignatureNode(signature, alg, data) == false) U_RETURN(false);

   /* references processing might change the status */

   if (status != UReferenceCtx::UNKNOWN) U_RETURN(true);

   /* verify SignatureValue node content */

   if (transformCtx.verifyNodeContent(signValueNode, signature_value) == false) U_RETURN(false);

   /* set status and we are done */

   status = (signMethod->status == UBaseTransform::OK ? UReferenceCtx::SUCCEEDED : UReferenceCtx::INVALID);

   U_RETURN(true);
}

/**
 * @node: the pointer to <dsig:Reference/> node.

 * The Reference Element (http://www.w3.org/TR/xmldsig-core/#sec-Reference)
 * 
 * Reference is an element that may occur one or more times. It specifies 
 * a digest algorithm and digest value, and optionally an identifier of the 
 * object being signed, the type of the object, and/or a list of transforms 
 * to be applied prior to digesting. The identification (URI) and transforms 
 * describe how the digested content (i.e., the input to the digest method) 
 * was created. The Type attribute facilitates the processing of referenced 
 * data. For example, while this specification makes no requirements over 
 * external data, an application may wish to signal that the referent is a 
 * Manifest. An optional ID attribute permits a Reference to be referenced 
 * from elsewhere.
 *
 * Returns: true on succes or false value otherwise.
 */

bool UReferenceCtx::processNode(xmlNodePtr node)
{
   U_TRACE(0, "UReferenceCtx::processNode(%p)", node)

   U_INTERNAL_ASSERT_POINTER(node)

   // read attributes first

   id   = UXML2Node::getProp(node, "Id");
   uri  = UXML2Node::getProp(node, "URI");
   type = UXML2Node::getProp(node, "Type");

   /* set start URI (and check that it is enabled!) */

   if (transformCtx.setURI(uri, node) == false) U_RETURN(false);

   // first is optional Transforms node

   xmlNodePtr cur = UXML2Node::getNextSibling(node->children);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"Transforms",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      // Reads transforms from the <dsig:Transform/> children of the @node and 
      // appends them to the current transforms chain in @ctx object.

      if (transformCtx.nodesListRead(cur, UBaseTransform::DSIG) == false) U_RETURN(false);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // next node is required DigestMethod

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"DigestMethod",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      UBaseTransform* digestMethod = UTransformCtx::nodeRead(cur, UBaseTransform::DIGEST);

      if (digestMethod == U_NULLPTR) U_RETURN(false);

      transformCtx.chain.push_back(digestMethod);

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // last node is required DigestValue

   xmlNodePtr digestValueNode = U_NULLPTR;

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"DigestValue",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      digestValueNode = cur;

      cur = UXML2Node::getNextSibling(cur->next);
      }

   // if there is something left than it's an error

   if (cur) U_RETURN(false);

   if (digestValueNode == U_NULLPTR) U_RETURN(false);

   // verify Reference node content

   UString data;
   const char* content = (const char*) UXML2Node::getContent(digestValueNode);

   if (transformCtx.execute(data) == false || data != content) U_RETURN(false);

   // set status and we are done

   status = SUCCEEDED;

   U_RETURN(true);
}

/** 
 * The SignedInfo Element (http://www.w3.org/TR/xmldsig-core/#sec-SignedInfo)
 * 
 * The structure of SignedInfo includes the canonicalization algorithm, 
 * a result algorithm, and one or more references. The SignedInfo element 
 * may contain an optional ID attribute that will allow it to be referenced by 
 * other signatures and objects.
 *
 * SignedInfo does not include explicit result or digest properties (such as
 * calculation time, cryptographic device serial number, etc.). If an 
 * application needs to associate properties with the result or digest, 
 * it may include such information in a SignatureProperties element within 
 * an Object element.
 *
 * Schema Definition:
 *
 *  <element name="SignedInfo" type="ds:SignedInfoType"/> 
 *  <complexType name="SignedInfoType">
 *    <sequence> 
 *      <element ref="ds:CanonicalizationMethod"/>
 *      <element ref="ds:SignatureMethod"/> 
 *      <element ref="ds:Reference" maxOccurs="unbounded"/> 
 *    </sequence> 
 *    <attribute name="Id" type="ID" use="optional"/> 
 *  </complexType>
 *    
 * DTD:
 *    
 *  <!ELEMENT SignedInfo (CanonicalizationMethod, SignatureMethod,  Reference+) >
 *  <!ATTLIST SignedInfo  Id   ID      #IMPLIED>
 */

bool UDSIGContext::processSignedInfoNode(const char*& alg, UString& data)
{
   U_TRACE(0, "UDSIGContext::processSignedInfoNode(%p,%.*S)", alg, U_STRING_TO_TRACE(data))

   U_INTERNAL_ASSERT(operation == UBaseTransform::VERIFY)
   U_ASSERT_EQUALS(pthis->signedInfoReferences.size(), 0)
   U_INTERNAL_ASSERT_EQUALS(status, UReferenceCtx::UNKNOWN)

   // first node is required CanonicalizationMethod

   xmlNodePtr cur = UXML2Node::getNextSibling(signedInfoNode->children);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"CanonicalizationMethod",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) U_RETURN(false);

   c14nMethod = UTransformCtx::nodeRead(cur, UBaseTransform::C14N);

   if (c14nMethod == U_NULLPTR) U_RETURN(false);

   transformCtx.chain.push_back(c14nMethod);

   alg = UXML2Node::getProp(cur, "Algorithm");

   unsigned char** inclusive_namespaces = U_NULLPTR;

   int mode          = (strncmp(alg, U_CONSTANT_TO_PARAM("http://www.w3.org/TR/2001/REC-xml-c14n-20010315")) == 0 ? 0 : 2),
       with_comments = (mode == 2 && strstr(alg, "#WithComments") != U_NULLPTR);

   // next node is required SignatureMethod

   cur = UXML2Node::getNextSibling(cur->next);

   if (UXML2Node::checkNodeName(cur, (const xmlChar*)"SignatureMethod",
                                     (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#") == false) U_RETURN(false);

   signMethod = UTransformCtx::nodeRead(cur, UBaseTransform::SIGNATURE);

   if (signMethod == U_NULLPTR) U_RETURN(false);

   alg = UXML2Node::getProp(cur, "Algorithm");
   alg = strchr(alg, '-') + 1;
   alg = strchr(alg, '-') + 1;

   transformCtx.chain.push_back(signMethod);

   signMethod->operation = UDSIGContext::pthis->operation;

   // calculate references

   cur = UXML2Node::getNextSibling(cur->next);

   while (UXML2Node::checkNodeName(cur, (const xmlChar*)"Reference",
                                        (const xmlChar*)"http://www.w3.org/2000/09/xmldsig#"))
      {
      /* create reference */

      UReferenceCtx* ref;

      U_NEW(UReferenceCtx, ref, UReferenceCtx(UReferenceCtx::SIGNED_INFO));

      /* add to the list */

      signedInfoReferences.push_back(ref);

      /* process */

      if (ref->processNode(cur) == false) U_RETURN(false);

      /* bail out if next Reference processing failed */

      if (ref->status != UReferenceCtx::SUCCEEDED)
         {
         status = UReferenceCtx::INVALID;

         U_RETURN(false);
         }

      cur = UXML2Node::getNextSibling(cur->next);
      }

   /* check that we have at least one Reference */

   if (signedInfoReferences.empty()) U_RETURN(false);

   // if there is something left than it's an error

   if (cur) U_RETURN(false);

   (void) UTranformXPointer::document->getElement(data, 0, U_CONSTANT_TO_PARAM("ds:SignedInfo"));

   data = UXML2Document::xmlC14N(data, mode, with_comments, inclusive_namespaces);

   U_RETURN(true);
}

/* Parses uri and adds xpointer transforms if required.
 *
 * The following examples demonstrate what the URI attribute identifies
 * and how it is dereferenced 
 *
 * (http://www.w3.org/TR/xmldsig-core/#sec-ReferenceProcessingModel):
 *
 * - URI="http://example.com/bar.xml"
 *
 * identifies the octets that represent the external resource 
 * 'http://example.com/bar.xml', that is probably an XML document given 
 * its file extension.
 *
 * - URI="http://example.com/bar.xml#chapter1"
 *
 * identifies the element with ID attribute value 'chapter1' of the 
 * external XML resource 'http://example.com/bar.xml', provided as an 
 * octet stream. Again, for the sake of interoperability, the element 
 * identified as 'chapter1' should be obtained using an XPath transform 
 * rather than a URI fragment (barename XPointer resolution in external 
 * resources is not REQUIRED in this specification). 
 *
 * - URI=""
 *
 * identifies the node-set (minus any comment nodes) of the XML resource 
 * containing the signature
 *
 * - URI="#chapter1"
 *
 * identifies a node-set containing the element with ID attribute value 
 * 'chapter1' of the XML resource containing the signature. XML Signature 
 * (and its applications) modify this node-set to include the element plus 
 * all descendents including namespaces and attributes -- but not comments.
 */

bool UTransformCtx::setURI(const char* _uri, xmlNodePtr node)
{
   U_TRACE(0, "UTransformCtx::setURI(%S,%p)", _uri, node)

   /* check uri */

   int uriType = 0;

   if (          _uri  == U_NULLPTR ||
       u__strlen(_uri, __PRETTY_FUNCTION__) == 0)
      {
      uriType = EMPTY;
      }
   else if (_uri[0] == '#')
      {
      uriType = SAME_DOCUMENT;
      }
   else if (strncmp(_uri, U_CONSTANT_TO_PARAM("file://")) == 0)
      {
      uriType = TYPE_LOCAL;
      }
   else
      {
      uriType = TYPE_REMOTE;
      }

   if ((uriType & enabledUris) == 0) U_RETURN(false);

   /* is it an empty uri? */

   if (uriType == EMPTY) U_RETURN(true);

   /* do we have barename or full xpointer? */

   const char* xptr = strchr(_uri, '#');

   if (xptr == U_NULLPTR)
      {
      UBaseTransform* uriTransform;

      U_NEW(UTranformInputURI, uriTransform, UTranformInputURI(_uri));

      chain.insert(0, uriTransform);

      U_RETURN(true);
      }

   if (strncmp(_uri, U_CONSTANT_TO_PARAM("#xpointer(/)")) == 0)
      {
      xptrExpr = _uri;

      U_RETURN(true);
      }

   xptrExpr  = U_SYSCALL_STRDUP(xptr);
   this->uri = U_SYSCALL_STRNDUP(_uri, xptr - _uri);

   /* do we have barename or full xpointer? */

   int nodeSetType = UNodeSet::TREE;

   if (strncmp(xptr, U_CONSTANT_TO_PARAM("#xmlns("))    == 0 ||
       strncmp(xptr, U_CONSTANT_TO_PARAM("#xpointer(")) == 0)
      {
      ++xptr;
      }
   else
      {
      /* we need to add "xpointer(id('..')) because otherwise we have problems with numeric ("111" and so on) and other "strange" ids */

      static char buf[128];

      (void) u__snprintf(buf, sizeof(buf), U_CONSTANT_TO_PARAM("xpointer(id(\'%s\'))"), xptr + 1);

      xptr = buf;

      nodeSetType = UNodeSet::TREE_WITHOUT_COMMENTS;
      }

   U_INTERNAL_DUMP("this->uri = %S xptr = %S", this->uri, xptr)

   // we need to create XPointer transform to execute expr

   UTranformXPointer* transform;

   U_NEW(UTranformXPointer, transform, UTranformXPointer);

   if (transform->setExpr(xptr, nodeSetType, node))
      {
      // check for XADES

      if (u_find(xptr, u__strlen(xptr, __PRETTY_FUNCTION__), U_CONSTANT_TO_PARAM("idPackageSignature-SignedProperties")))
         {
         transform->tag = U_STRING_FROM_CONSTANT("xades:SignedProperties");

         UTranformInclC14N* p;

         U_NEW(UTranformInclC14N, p, UTranformInclC14N);

         chain.insert(0, p);
         }

      chain.insert(0, transform);

      U_RETURN(true);
      }

   U_RETURN(false);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* UTransformCtx::dump(bool reset) const
{
   *UObjectIO::os << "uri                             " << (     uri ?      uri : "") << '\n'
                  << "status                          " << status                     << '\n'
                  << "xptrExpr                        " << (xptrExpr ? xptrExpr : "") << '\n'
                  << "enabledUris                     " << enabledUris                << '\n'
                  << "chain (UVector<UBaseTransform*> " << &chain                     << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UReferenceCtx::dump(bool reset) const
{
   *UObjectIO::os << "status    " << status << '\n'
                  << "origin    " << origin;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UDSIGContext::dump(bool reset) const
{
   *UObjectIO::os << "status    " << status << '\n'
                  << "operation " << operation;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

#endif
