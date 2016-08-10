// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    xml_parser.h - parser XML based on Expat
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_XMLPARSER_H
#define ULIB_XMLPARSER_H 1

#include <ulib/string.h>

#include <expat.h>

/**
 * @class UXMLParser 
 *
 * @brief UXMLParser is a parser XML based on Expat (Expat is a stream-oriented parser)
 */

class U_EXPORT UXMLParser {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UXMLParser()
      {
      U_TRACE_REGISTER_OBJECT(0, UXMLParser, "", 0)

      m_parser = 0;
      }

   virtual ~UXMLParser()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UXMLParser)

      if (m_parser)
         {
         // Free memory used by the parser. Your application is responsible for freeing any memory associated with UserData.

         XML_ParserFree(m_parser);

         m_parser = 0;
         }
      }

   // SERVICES

   void initParser(bool ns = true, const char* charset = 0);

   bool parse(const UString& data)
      {
      U_TRACE(0, "UXMLParser::parse(%V)", data.rep)

      U_INTERNAL_ASSERT_POINTER(m_parser)

      bool result = (XML_Parse(m_parser, U_STRING_TO_PARAM(data), 1) != 0);

      U_RETURN(result);
      }

   const char* getErrorMessage()
      {
      U_TRACE_NO_PARAM(0, "UXMLParser::getErrorMessage()")

      U_INTERNAL_ASSERT_POINTER(m_parser)

      const char* result = XML_ErrorString(XML_GetErrorCode(m_parser));

      U_RETURN(result);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   XML_ParserStruct* m_parser;

   void* getParseBuffer(int size)
      {
      U_TRACE(0, "UXMLParser::getParseBuffer(%d)", size)

      U_INTERNAL_ASSERT_POINTER(m_parser)

      void* result = XML_GetBuffer(m_parser, size);

      U_RETURN(result);
      }

   bool parseBuffer(int size)
      {
      U_TRACE(0, "UXMLParser::parseBuffer(%d)", size)

      U_INTERNAL_ASSERT_POINTER(m_parser)

      bool result = (XML_ParseBuffer(m_parser, size, (size == 0)) != 0);

      U_RETURN(result);
      }

   // VIRTUAL METHOD

   virtual void startNamespace(const XML_Char* prefix, const XML_Char* uri)
      {
      U_TRACE(0, "UXMLParser::startNamespace(%S,%S)", prefix, uri)
      }

   virtual void endNamespace(const XML_Char* prefix)
      {
      U_TRACE(0, "UXMLParser::endNamespace(%S)", prefix)
      }

   virtual void startElement(const XML_Char* name, const XML_Char** attrs)
      {
      U_TRACE(0, "UXMLParser::startElement(%S,%p)", name, attrs)

      U_DUMP_ATTRS(attrs)
      }

   virtual void endElement(const XML_Char* name)
      {
      U_TRACE(0, "UXMLParser::endElement(%S)", name)
      }

   virtual void characterData(const XML_Char* str, int len)
      {
      U_TRACE(0, "UXMLParser::characterData(%.*S,%d)", len, str, len)
      }

   // the C based XML parser calls through to these methods which pass them on to the C++ interface

   static void _startElement(void* userData, const XML_Char* name, const XML_Char** attrs)
      { ((UXMLParser*)userData)->startElement(name, attrs); }

   static void _characterData(void* userData, const XML_Char* str, int len)
      { ((UXMLParser*)userData)->characterData(str, len); }

   static void _endElement(void* userData, const XML_Char* name)
      { ((UXMLParser*)userData)->endElement(name); }

   static void _startNamespace(void* userData, const XML_Char* prefix, const XML_Char* uri)
      { ((UXMLParser*)userData)->startNamespace(prefix, uri); }

   static void _endNamespace(void* userData, const XML_Char* prefix) { ((UXMLParser*)userData)->endNamespace(prefix); }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UXMLParser)
};

#endif
