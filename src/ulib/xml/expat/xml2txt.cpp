// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    xml2txt.cpp - parser XML based on Expat to use plain text
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/expat/xml2txt.h>

#define U_NOT_OUTPUT ((tag_match == false && tag_to_exclude == false) || \
                      (tag_match ==  true && tag_to_exclude ==  true))

UXml2Txt::UXml2Txt(const UString& tag_list, bool _tag_to_exclude, bool _tag_output_also) : taglist(tag_list, "<>, ")
{
   U_TRACE_REGISTER_OBJECT(0, UXml2Txt, "%V,%b,%b", tag_list.rep, _tag_to_exclude, _tag_output_also)

   tag_pos = 0;
   tag_match = false;

   tag_to_exclude  = (taglist.empty() == false ? _tag_to_exclude : false);
   tag_output_also = _tag_output_also;

   UXMLParser::initParser(false, 0);
}

UXml2Txt::~UXml2Txt()
{
   U_TRACE_UNREGISTER_OBJECT(0, UXml2Txt)
}

void UXml2Txt::startElement(const XML_Char* name, const XML_Char** attrs)
{
   U_TRACE(0, "UXml2Txt::startElement(%S,%p)", name, attrs)

   if (taglist.empty()) tag_match = true;
   else
      {
      tag_pos   = taglist.find(UString(name, u__strlen(name, __PRETTY_FUNCTION__)));
      tag_match = (tag_pos != U_NOT_FOUND);
      }

   U_INTERNAL_DUMP("U_NOT_OUTPUT = %b tag_match = %b", U_NOT_OUTPUT, tag_match)

   if (U_NOT_OUTPUT) return;

   if (tag_output_also)
      {
      (void) output.push('<');
      (void) output.append(name);
      (void) output.push('>');
      }
   else if (attrs)
      {
      U_DUMP_ATTRS(attrs)

      for (int i = 0; attrs[i]; ++i)
         {
         (void) output.push(' ');
         (void) output.append(attrs[i]);
         }
      }
}

void UXml2Txt::characterData(const XML_Char* str, int len)
{
   U_TRACE(5, "UXml2Txt::characterData(%.*S,%d)", len, str, len)

   U_INTERNAL_DUMP("U_NOT_OUTPUT = %b tag_match = %b", U_NOT_OUTPUT, tag_match)

   if (U_NOT_OUTPUT) return;

   /*
   while (u__isspace(*str) && len > 0)
      {
      ++str;
      --len;
      }

   if (len > 0)
   */

   (void) output.append(str, len);
}

void UXml2Txt::endElement(const XML_Char* name)
{
   U_TRACE(0, "UXml2Txt::endElement(%S)", name)

   U_INTERNAL_DUMP("U_NOT_OUTPUT = %b tag_match = %b", U_NOT_OUTPUT, tag_match)

   if (U_NOT_OUTPUT == false &&
       tag_output_also)
      {
      (void) output.append(U_CONSTANT_TO_PARAM("</"));
      (void) output.append(name);
      (void) output.push('>');
      }

   if (tag_match              &&
       tag_pos != U_NOT_FOUND &&
       taglist.at(tag_pos) == name)
      {
      tag_match = false;
      }

   U_INTERNAL_DUMP("tag_match = %b", tag_match)
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UXml2Txt::dump(bool reset) const
{
   *UObjectIO::os << "tag_pos                   " << tag_pos         << '\n'
                  << "tag_match                 " << tag_match       << '\n'
                  << "tag_to_exclude            " << tag_to_exclude  << '\n'
                  << "tag_output_also           " << tag_output_also << '\n'
                  << "output  (UString          " << (void*)&output  << ")\n"
                  << "taglist (UVector<UString> " << (void*)&taglist << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
