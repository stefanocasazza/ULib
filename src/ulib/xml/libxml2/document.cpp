// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    document.cpp - wrapping of libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/string_ext.h>
#include <ulib/xml/libxml2/document.h>

#include <libxml/c14n.h>

bool UXML2Document::binit;

void UXML2Document::init()
{
   U_TRACE_NO_PARAM(1, "UXML2Document::init()")

   binit = true;

   /**
    * build an XML tree from a file; we need to add default attributes
    * and resolve all character and entities references: required for c14n!
    */

   xmlLoadExtDtdDefaultValue = XML_DETECT_IDS | XML_COMPLETE_ATTRS;

   U_SYSCALL_VOID(xmlSubstituteEntitiesDefault, "%d", 1);

   /* Do not fetch DTD over network */

// xmlExternalEntityLoader defaultEntityLoader = xmlNoNetExternalEntityLoader;

   U_SYSCALL_VOID(xmlSetExternalEntityLoader, "%p", xmlNoNetExternalEntityLoader);

   xmlLoadExtDtdDefaultValue = 0;
}

UXML2Document::UXML2Document(const UString& _data) : data(_data)
{
   U_TRACE_REGISTER_OBJECT(0, UXML2Document, "%V", _data.rep)

   if (binit == false) init();

// impl_ = (xmlDocPtr) U_SYSCALL(xmlParseMemory, "%p,%d", U_STRING_TO_PARAM(_data));

   /*
   Enum xmlParserOption {
      XML_PARSE_RECOVER = 1 : recover on errors
      XML_PARSE_NOENT = 2 : substitute entities
      XML_PARSE_DTDLOAD = 4 : load the external subset
      XML_PARSE_DTDATTR = 8 : default DTD attributes
      XML_PARSE_DTDVALID = 16 : validate with the DTD
      XML_PARSE_NOERROR = 32 : suppress error reports
      XML_PARSE_NOWARNING = 64 : suppress warning reports
      XML_PARSE_PEDANTIC = 128 : pedantic error reporting
      XML_PARSE_NOBLANKS = 256 : remove blank nodes
      XML_PARSE_SAX1 = 512 : use the SAX1 interface internally
      XML_PARSE_XINCLUDE = 1024 : Implement XInclude substitition
      XML_PARSE_NONET = 2048 : Forbid network access
      XML_PARSE_NODICT = 4096 : Do not reuse the context dictionnary
      XML_PARSE_NSCLEAN = 8192 : remove redundant namespaces declarations
      XML_PARSE_NOCDATA = 16384 : merge CDATA as text nodes
      XML_PARSE_NOXINCNODE = 32768 : do not generate XINCLUDE START/END nodes
      XML_PARSE_COMPACT = 65536 : compact small text nodes; no modification of the tree allowed afterwards (will possibly crash if you try to modify tree)
      XML_PARSE_OLD10 = 131072 : parse using XML-1.0 before update 5
      XML_PARSE_NOBASEFIX = 262144 : do not fixup XINCLUDE xml:base uris
      XML_PARSE_HUGE = 524288 : relax any hardcoded limit from the parser
      XML_PARSE_OLDSAX = 1048576 : parse using SAX2 interface from before 2.7.0
   }
   */

   impl_ = (xmlDocPtr) U_SYSCALL(xmlReadMemory, "%p,%d,%S,%S,%d", U_STRING_TO_PARAM(_data), 0, 0, XML_PARSE_COMPACT);

   if (impl_ == NULL)
      {
      U_ERROR("Unable to parse xml document");
      }

   /*
    * Check the document is of the right kind
    */

   if (getRootNode() == NULL) U_ERROR("Empty xml document");
}

uint32_t UXML2Document::getElement(UString& element, uint32_t pos, const char* tag, uint32_t tag_len)
{
   U_TRACE(0, "UXML2Document::getElement(%V,%u,%.*S,%u)", element.rep, pos, tag_len, tag, tag_len)

   U_INTERNAL_ASSERT_POINTER(tag)

   uint32_t start = data.find(tag, pos, tag_len);

   if (start == U_NOT_FOUND ||
       data.c_char(start-1) != '<')
      {
      U_RETURN(U_NOT_FOUND);
      }

   uint32_t end = data.find(tag, start + tag_len, tag_len);

   if (end == U_NOT_FOUND        ||
       data.c_char(end-1) != '/' ||
       data.c_char(end-2) != '<')
      {
      U_RETURN(U_NOT_FOUND);
      }

   element = data.substr(start - 1, end + tag_len - start + 2);

   end += tag_len;

   U_RETURN(end);
}

UString UXML2Document::getElementData(uint32_t pos, const char* tag, uint32_t tag_len)
{
   U_TRACE(0, "UXML2Document::getElementData(%u,%.*S,%u)", pos, tag_len, tag, tag_len)

   UString element;
   uint32_t end = getElement(element, pos, tag, tag_len);

   if (element &&
       end != U_NOT_FOUND)
      {
      uint32_t n    = tag_len + 2;                                                       // <...>
      UString _data = UStringExt::trim(element.substr(n, element.size() - (n * 2) - 1)); // </..>

      U_RETURN_STRING(_data);
      }

   U_RETURN_STRING(element);
}

uint32_t UXML2Document::getElement(UVector<UString>& velement, const char* tag, uint32_t tag_len)
{
   U_TRACE(0, "UXML2Document::getElement(%p,%.*S,%u)", &velement, tag_len, tag, tag_len)

   U_INTERNAL_ASSERT_POINTER(tag)

   UString element;
   uint32_t n = velement.size(), pos = 0;

   while (true)
      {
      pos = getElement(element, pos, tag, tag_len);

      if (pos == U_NOT_FOUND) break;

      velement.push(element);
      }

   uint32_t result = velement.size() - n;

   U_RETURN(result);
}

xmlNodePtr UXML2Document::findNode(const xmlNodePtr parent, const xmlChar* name, const xmlChar* ns)
{
   U_TRACE(0, "UXML2Document::findNode(%p,%S,%S)", parent, name, ns)

   U_INTERNAL_ASSERT_POINTER(name)

   xmlNodePtr ret;
   xmlNodePtr cur = parent;

   while (cur)
      {
      if (cur->type == XML_ELEMENT_NODE &&
          UXML2Node(cur).checkNodeName(name, ns))
         {
         U_RETURN_POINTER(cur, xmlNode);
         }

      if (cur->children)
         {
         ret = findNode(cur->children, name, ns);

         if (ret) U_RETURN_POINTER(ret, xmlNode);
         }

      cur = cur->next;
      }

   U_RETURN_POINTER(0, xmlNode);
}

xmlNodePtr UXML2Document::findChild(const xmlNodePtr parent, const xmlChar* name, const xmlChar* ns)
{
   U_TRACE(0, "UXML2Document::findChild(%p,%S,%S)", parent, name, ns)

   U_INTERNAL_ASSERT_POINTER(name)
   U_INTERNAL_ASSERT_POINTER(parent)

   xmlNodePtr cur = parent->children;

   while (cur)
      {
      if (cur->type == XML_ELEMENT_NODE &&
          UXML2Node(cur).checkNodeName(name, ns))
         {
         U_RETURN_POINTER(cur, xmlNode);
         }

      cur = cur->next;
      }

   U_RETURN_POINTER(0, xmlNode);
}

xmlNodePtr UXML2Document::findParent(const xmlNodePtr cur, const xmlChar* name, const xmlChar* ns)
{
   U_TRACE(0, "UXML2Document::findParent(%p,%S,%S)", cur, name, ns)

   U_INTERNAL_ASSERT_POINTER(cur)
   U_INTERNAL_ASSERT_POINTER(name)

   xmlNodePtr ret;

   if (cur->type == XML_ELEMENT_NODE &&
       UXML2Node(cur).checkNodeName(name, ns))
      {
      U_RETURN_POINTER(cur, xmlNode);
      }

   if (cur->parent)
      {
      ret = findParent(cur->parent, name, ns);

      if (ret) U_RETURN_POINTER(ret, xmlNode);
      }

   U_RETURN_POINTER(0, xmlNode);
}

bool UXML2Document::writeToFile(const char* filename, const char* encoding, bool formatted)
{
   U_TRACE(1, "UXML2Document::writeToFile(%S,%S,%b)", filename, encoding, formatted)

   U_INTERNAL_ASSERT_POINTER(impl_)

   int oldIndentTreeOutput = 0, oldKeepBlanksDefault = 0;

   if (formatted)
      {
      oldIndentTreeOutput  = xmlIndentTreeOutput;
      xmlIndentTreeOutput  = 1;
      oldKeepBlanksDefault = U_SYSCALL(xmlKeepBlanksDefault, "%d", 1);
      }

   bool result = (U_SYSCALL(xmlSaveFormatFileEnc, "%S,%p,%S,%d", filename, impl_, encoding, formatted) != -1);

   if (formatted)
      {
      xmlIndentTreeOutput = oldIndentTreeOutput;

      (void) U_SYSCALL(xmlKeepBlanksDefault, "%d", oldKeepBlanksDefault);
      }

   U_RETURN(result);
}

xmlChar* UXML2Document::writeToString(int& length, const char* encoding, bool formatted)
{
   U_TRACE(1, "UXML2Document::writeToString(%S,%b)", encoding, formatted)

   U_INTERNAL_ASSERT_POINTER(impl_)

   int oldIndentTreeOutput, oldKeepBlanksDefault;

   if (formatted)
      {
      oldIndentTreeOutput  = xmlIndentTreeOutput;
      xmlIndentTreeOutput  = 1;
      oldKeepBlanksDefault = U_SYSCALL(xmlKeepBlanksDefault, "%d", 1);
      }

   xmlChar* buffer = 0;

   U_SYSCALL_VOID(xmlDocDumpFormatMemoryEnc, "%p,%p,%p,%S,%d", impl_, &buffer, &length, encoding, formatted);

   if (formatted)
      {
      xmlIndentTreeOutput = oldIndentTreeOutput;

      (void) U_SYSCALL(xmlKeepBlanksDefault, "%d", oldKeepBlanksDefault);
      }

   U_RETURN_POINTER(buffer, xmlChar);
}

// Canonical XML implementation (http://www.w3.org/TR/2001/REC-xml-c14n-20010315)

UString UXML2Document::xmlC14N(int mode, int with_comments, unsigned char** inclusive_namespaces)
{
   U_TRACE(1, "UXML2Document::xmlC14N(%d,%d,%d,%p)", mode, with_comments, inclusive_namespaces)

   U_INTERNAL_ASSERT_POINTER(impl_)

#ifndef LIBXML_C14N_ENABLED
   U_ERROR("XPath/Canonicalization support not compiled in libxml2");
#endif

   /*
    * Canonical form
    */

   UString output;

   xmlChar* result = NULL;

   int ret = U_SYSCALL(xmlC14NDocDumpMemory, "%p,%p,%d,%p,%d,%p", impl_, NULL, mode, inclusive_namespaces, with_comments, &result);

   if (ret < 0) U_WARNING("Failed to canonicalize buffer data (%d)", ret);

   if (result != NULL)
      {
      (void) output.replace((const char*)result);

      U_SYSCALL_VOID(xmlFree, "%p", result);
      }

   U_RETURN_STRING(output);
}

UString UXML2Document::xmlC14N(const UString& data, int mode, int with_comments, unsigned char** inclusive_namespaces)
{
   U_TRACE(0, "UXML2Document::xmlC14N(%V,%d,%d,%d,%p)", data.rep, mode, with_comments, inclusive_namespaces)

   UString output;
   UXML2Document tmp(data);

   if (tmp.cobj()) output = tmp.xmlC14N(mode, with_comments, inclusive_namespaces);

   U_RETURN_STRING(output);
}

/* helper function for converting a passed string in one character set to another

char* UXML2Document::convString(const char* string, const char* in_charset, const char* out_charset)
{
   U_TRACE(0, "UXML2Document::convString(%V,%S,%S)", string.rep in_charset, out_charset)

   U_INTERNAL_ASSERT_POINTER( in_charset)
   U_INTERNAL_ASSERT_POINTER(out_charset)

   char* ret = 0;

   xmlCharEncodingHandlerPtr in  = U_SYSCALL(xmlFindCharEncodingHandler, "%S",  in_charset);
   xmlCharEncodingHandlerPtr out = U_SYSCALL(xmlFindCharEncodingHandler, "%S", out_charset);

   if (in && out)
      {
      xmlBufferPtr orig = xmlBufferCreate();
      xmlBufferPtr utf8 = xmlBufferCreate();
      xmlBufferPtr conv = xmlBufferCreate();

      xmlBufferCCat(orig, string);

      if (xmlCharEncInFunc(in, utf8, orig) > 0)
         {
         xmlCharEncOutFunc(out, conv, NULL);

         if (xmlCharEncOutFunc(out, conv, utf8) >= 0) ret = strdup((const char*)xmlBufferContent(conv));
         }

      xmlBufferFree(orig);
      xmlBufferFree(utf8);
      xmlBufferFree(conv);
      }

   xmlCharEncCloseFunc(in);
   xmlCharEncCloseFunc(out);

   return ret;
}
*/

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UXML2Document::dump(bool reset) const
{
   *UObjectIO::os << "impl_ " << (void*)impl_;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
