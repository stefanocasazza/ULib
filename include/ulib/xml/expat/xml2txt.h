// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    xml2txt.h - parser XML based on Expat to use plain text
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_XML2TXT_H
#define ULIB_XML2TXT_H 1

#include <ulib/container/vector.h>
#include <ulib/xml/expat/xml_parser.h>

class U_EXPORT UXml2Txt : public UXMLParser {
public:

            UXml2Txt(const UString& tag_list, bool _tag_to_exclude, bool _tag_output_also);
   virtual ~UXml2Txt();

   // SERVICES

   bool parse(const UString& data)
      {
      U_TRACE(0, "UXml2Txt::parse(%V)", data.rep)

      tag_pos = U_NOT_FOUND;

      output.setBuffer(U_CAPACITY);

      bool result = UXMLParser::parse(data);

      (void) output.shrink();

      U_RETURN(result);
      }

   UString getText() const { return output; }

   // VIRTUAL METHOD redefined

   virtual void startElement(const XML_Char* name, const XML_Char** attrs) U_DECL_OVERRIDE;

   virtual void characterData(const XML_Char* str, int len) U_DECL_OVERRIDE;

   virtual void endElement(const XML_Char* name) U_DECL_OVERRIDE;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString output;
   UVector<UString> taglist;
   uint32_t tag_pos;
   bool tag_match, tag_to_exclude, tag_output_also;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UXml2Txt)
};

#endif
