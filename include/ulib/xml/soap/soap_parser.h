// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_parser.h - SOAP parser based on Expat
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOAP_PARSER_H
#define ULIB_SOAP_PARSER_H 1

#include <ulib/container/tree.h>
#include <ulib/xml/expat/element.h>
#include <ulib/net/rpc/rpc_parser.h>
#include <ulib/xml/expat/xml_parser.h>

#ifdef U_SOAP_NAMESPACE
#  include <ulib/container/hash_map.h>
#endif

/**
 * @class USOAPParser
 *
 *  @brief USOAPParser is a parser SOAP based on Expat (Expat is a stream-oriented parser)
 *
 *  SOAP (Simple Object Access Protocol) is a simple XML based protocol to let applications exchange information over HTTP.
 *  SOAP is fundamentally a stateless, one-way message exchange paradigm.
 *
 *  The following SOAP 1.2 message contains a number of elements, attributes, and values:
 *  -------------------------------------------------------------------------------------
 *  <?xml version='1.0' ?>
 *  <soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope"> 
 *   <soap:Header>
 *     <m:reservation xmlns:m="http://travelcompany.example.org/reservation" 
 *             soap:role="http://www.w3.org/2003/05/soap-envelope/role/next"
 *             soap:mustUnderstand="true">
 *      <m:reference>uuid:093a2da1-q345-739r-ba5d-pqff98fe8j7d</m:reference>
 *      <m:dateAndTime>2001-11-29T13:20:00.000-05:00</m:dateAndTime>
 *     </m:reservation>
 *     <n:passenger xmlns:n="http://mycompany.example.com/employees"
 *             soap:role="http://www.w3.org/2003/05/soap-envelope/role/next"
 *             soap:mustUnderstand="true">
 *      <n:name>Åke Jógvan Øyvind</n:name>
 *     </n:passenger>
 *    </soap:Header>
 *    <soap:Body>
 *     <p:itinerary xmlns:p="http://travelcompany.example.org/reservation/travel">
 *      <p:departure>
 *        <p:departing>New York</p:departing>
 *        <p:arriving>Los Angeles</p:arriving>
 *        <p:departureDate>2001-12-14</p:departureDate>
 *        <p:departureTime>late afternoon</p:departureTime>
 *        <p:seatPreference>aisle</p:seatPreference>
 *      </p:departure>
 *      <p:return>
 *        <p:departing>Los Angeles</p:departing>
 *        <p:arriving>New York</p:arriving>
 *        <p:departureDate>2001-12-20</p:departureDate>
 *        <p:departureTime>mid-morning</p:departureTime>
 *        <p:seatPreference/>
 *      </p:return>
 *     </p:itinerary>
 *     <q:lodging xmlns:q="http://travelcompany.example.org/reservation/hotels">
 *      <q:preference>none</q:preference>
 *     </q:lodging>
 *    </soap:Body>
 *  </soap:Envelope>
 *  -------------------------------------------------------------------------------------
 *
 *  SOAP envelope: The outermost element information item of a SOAP message.
 *  ------------------------------------------------------------------------
 *  The SOAP Envelope element information item has:
 *   - A local name of Envelope.
 *   - A namespace name of "http://www.w3.org/2003/05/soap-envelope".
 *   - Zero or more namespace qualified attribute information items amongst its attributes property.
 *   - One or two element information items in its [children] property in order as follows:
 *   -  1. An optional Header element information item.
 *   -  2. A mandatory Body   element information item.
 *  ------------------------------------------------------------------------
 *
 *  The encodingStyle attribute information item indicates the encoding rules used to serialize parts of a SOAP message.
 *  --------------------------------------------------------------------------------------------------------------------
 *   The encodingStyle attribute information item has:
 *   - A local name of encodingStyle.
 *   - A namespace name of "http://www.w3.org/2003/05/soap-envelope".
 *   - The encodingStyle attribute information item is of type xs:anyURI.
 *     Its value identifies a set of serialization rules that can be used to deserialize the SOAP message.
 *   - The encodingStyle attribute information item MAY appear on the following:
 *   -  1. A SOAP header block.
 *   -  2. A child element information item of the SOAP Body element information item if that child is not a SOAP
 *         Fault element information.
 *   -  3. A child element information item of the SOAP Detail element information item.
 *   -  4. Any descendent of 1, 2, and 3 above.
 *  --------------------------------------------------------------------------------------------------------------------
 *
 *  --------------------------------------------------------------------------------------------------------------------
 *  SOAP header: A collection of zero or more SOAP header blocks each of which might be targeted at any SOAP receiver
 *               within the SOAP message path.
 *  SOAP header block: An element information item used to delimit data that logically constitutes a single computational
 *                     unit within the SOAP header.
 *
 *  The type of a SOAP header block is identified by the XML expanded name of the header block element information item.
 *  The Header element information item has:
 *   - A local name of Header.
 *   - A namespace name of "http://www.w3.org/2003/05/soap-envelope".
 *   - Zero or more namespace qualified attribute information items in its attributes property.
 *   - Zero or more namespace qualified element information items in its children property.
 *   - Each child element information item of the SOAP Header is called a SOAP header block.
 *
 *  Each SOAP header block element information item:
 *   . MUST have a namespace name property which has a value, that is the name of the element MUST be namespace qualified.
 *   . MAY have any number of character information item children. Child character information items whose character code
 *         is amongst the white space characters as defined by XML 1.0 are considered significant.
 *   . MAY have any number of element information item children. Such element information items MAY be namespace qualified.
 *   . MAY have zero or more attribute information items in its attributes property. Among these MAY be any or all of the
 *         following, which have special significance for SOAP processing:
 *   .     - encodingStyle attribute information item.
 *   .     - role attribute information item.
 *   .     - mustUnderstand attribute information item.
 *   .     - relay attribute information item.
 *
 *  A SOAP header is an extension mechanism that provides a way to pass information in SOAP messages that is not
 *  application payload. Such "control" information includes, for example, passing directives or contextual
 *  information related to the processing of the message. This allows a SOAP message to be extended in an
 *  application-specific manner. The immediate child elements of the soap:Header element are called header
 *  blocks, and represent a logical grouping of data which, as shown later, can individually be targeted at SOAP nodes that
 *  might be encountered in the path of a message from a sender to an ultimate receiver.
 *
 *  SOAP headers have been designed in anticipation of various uses for SOAP, many of which will involve the
 *  participation of other SOAP processing nodes - called SOAP intermediaries - along a message's path from an
 *  initial SOAP sender to an ultimate SOAP receiver. This allows SOAP intermediaries to provide value-added
 *  services. Headers, as shown later, may be inspected, inserted, deleted or forwarded by SOAP nodes encountered
 *  along a SOAP message path.
 *  --------------------------------------------------------------------------------------------------------------------
 *
 *  --------------------------------------------------------------------------------------------------------------------
 *  SOAP body: A collection of zero or more element information items targeted at an ultimate SOAP receiver in the SOAP
 *             message path.
 *
 *  The Body element information item has:
 *      - A local name of Body.
 *      - A namespace name of "http://www.w3.org/2003/05/soap-envelope".
 *      - Zero or more namespace qualified attribute information items in its attributes property.
 *      - Zero or more namespace qualified element information items in its children property.
 *      - The Body element information item MAY have any number of character information item children whose character
 *        code is amongst the white space characters as defined by XML 1.0. These are considered significant.
 *
 *  All child element information items of the SOAP Body element information item:
 *      - SHOULD have a namespace name property which has a value, that is the name of the element SHOULD be namespace
 *               qualified. Note: Namespace qualified elements tend to produce messages whose interpretation is less
 *               ambiguous than those with unqualified elements. The use of unqualified elements is therefore discouraged.
 *      - MAY have any number of character information item children. Child character information items whose character
 *            code is amongst the white space characters as defined by XML 1.0 are considered significant.
 *      - MAY have any number of element information item children. Such element information items MAY be namespace qualified.
 *      - MAY have zero or more attribute information items in its attributes property. Among these MAY be the following,
 *            which has special significance for SOAP processing: encodingStyle attribute information item.
 *
 *  SOAP defines one particular direct child of the SOAP body, the SOAP fault, which is used for reporting errors
 *  --------------------------------------------------------------------------------------------------------------------
 */

class U_EXPORT USOAPParser : public UXMLParser, public URPCParser {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   USOAPParser(UVector<UString>* arg = 0) : URPCParser(arg), tree(0,0,2)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPParser, "", 0)

#  ifdef U_SOAP_NAMESPACE
      XMLNStoURN.allocate();
#  endif

      zero();

      ptree = 0;
      }

   void clearData()
      {
      U_TRACE_NO_PARAM(0, "USOAPParser::clearData()")

      URPCParser::clearData();

#  ifdef U_SOAP_NAMESPACE
      XMLNStoURN.clear();
#  endif

      tree.clear();
      }

   virtual ~USOAPParser()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPParser)

      clearData();
      }

   // SERVICES

   void zero()
      {
      U_TRACE_NO_PARAM(0, "USOAPParser::zero()")

      body                    = 0;
      header                  = 0;
      method                  = 0;
      current                 = 0;
      flag_state              = 0;
      envelope.mustUnderstand = false;
      }

   void initParser(const char* charset = 0)
      {
      U_TRACE(0, "USOAPParser::initParser(%S)", charset)

      zero();

      ptree = &tree;

      UXMLParser::initParser(false, charset);
      }

   bool parse(const UString& msg);

#ifdef U_SOAP_NAMESPACE
   UHashMap<UString>& getNamespacesInUse() { return XMLNStoURN; } // Returns a reference to the internal namespace to URN mapping
#endif

   UString getResponse()
      {
      U_TRACE_NO_PARAM(0, "USOAPParser::getResponse()")
      
      U_INTERNAL_ASSERT_POINTER(method)

      UString res = method->childAt(0)->elem()->getValue();

      U_RETURN_STRING(res);
      }

   UString getFaultResponse();
 
   // -----------------------------------------------------------------------
   // Given a USOAPObject this calls the appropriate URPCMethod.
   // If this fails, ask for a USOAPFault. Returns a SOAP send-ready response
   // -----------------------------------------------------------------------
   // bContainsFault: Indicates if the returned string contains a fault
   // -----------------------------------------------------------------------

   UString processMessage(const UString& msg, URPCObject& object, bool& bContainsFault);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UTree<UXMLElement*> tree;     // Container of UXMLElement
   UTree<UXMLElement*>* body;
   UTree<UXMLElement*>* header;
   UTree<UXMLElement*>* method;

#ifdef U_SOAP_NAMESPACE
   UHashMap<UString> XMLNStoURN; // Contains the map of namespace name to URN
#endif

   // VIRTUAL METHOD

   virtual void startElement(const XML_Char* name, const XML_Char** attrs) U_DECL_OVERRIDE;

   virtual void characterData(const XML_Char* str, int len) U_DECL_OVERRIDE
      {
      U_TRACE(0+256, "USOAPParser::characterData(%.*S,%d)", len, str, len)

      current->getValue().append(str, len);
      }

   virtual void endElement(const XML_Char* name) U_DECL_OVERRIDE
      {
      U_TRACE(0, "USOAPParser::endElement(%S)", name)

      U_DUMP("ptree = %p ptree->parent() = %p ptree->numChild() = %u ptree->depth() = %u",
                  ptree, ptree->parent(), ptree->numChild(), ptree->depth())

      ptree = ptree->parent();
      }

private:
   int flag_state;
   UXMLElement* current;
   UTree<UXMLElement*>* ptree;

   U_DISALLOW_COPY_AND_ASSIGN(USOAPParser)
};

#endif
