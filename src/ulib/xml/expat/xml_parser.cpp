// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    xml_parser.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/expat/xml_parser.h>

void UXMLParser::initParser(bool ns, const char* encoding)
{
   U_TRACE(0, "UXMLParser::initParser(%b,%S)", ns, encoding)

   if (m_parser) XML_ParserFree(m_parser);

   /**
    * Constructs a new parser that has namespace processing in effect.
    * If encoding is non-null, it specifies a character encoding to use for the document.
    * This overrides the document encoding declaration. There are four built-in encodings:
    *   US-ASCII
    *   UTF-8
    *   UTF-16
    *   ISO-8859-1
    * Namespace expanded element names and attribute names are returned as a concatenation of the namespace URI, sep,
    * and the local part of the name. This means that you should pick a character for sep that can't be part of a legal URI
    */

   m_parser = (ns ? XML_ParserCreateNS(encoding, '#')
                  : XML_ParserCreate(encoding));

   XML_SetUserData(                 m_parser, this);
   XML_SetElementHandler(           m_parser, UXMLParser::_startElement,
                                              UXMLParser::_endElement);
   XML_SetCharacterDataHandler(     m_parser, UXMLParser::_characterData);

   if (ns)
      {
      XML_SetStartNamespaceDeclHandler(m_parser, UXMLParser::_startNamespace);
      XML_SetEndNamespaceDeclHandler(  m_parser, UXMLParser::_endNamespace);
      }
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UXMLParser::dump(bool reset) const
{
   *UObjectIO::os << "m_parser                                          " << (void*)m_parser;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
