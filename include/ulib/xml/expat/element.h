// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    element.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_UXMLELEMENT_H
#define ULIB_UXMLELEMENT_H 1

#include <ulib/container/vector.h>
#include <ulib/xml/expat/attribute.h>

class U_EXPORT UXMLElement {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UXMLElement()
      {
      U_TRACE_REGISTER_OBJECT(0, UXMLElement, "", 0)
      }

   UXMLElement(const UString& s, const UString& a, const UString& n) : str(s), accessorName(a), namespaceName(n)
      {
      U_TRACE_REGISTER_OBJECT(0, UXMLElement, "%V,%V,%V", s.rep, a.rep, n.rep) }

   ~UXMLElement()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UXMLElement)
      }

   // SERVICES

   UString& getStr()           { return str; }
   UString& getValue()         { return value; }         // reference to the internal value of the element
   UString& getAccessorName()  { return accessorName; }  // reference to the internal accessor name
                                                         // (string used to access the element)
   UString& getNamespaceName() { return namespaceName; } // reference to the internal namespace name

   // Adds an attribute to the element

   void addAttribute(UXMLAttribute* attribute)
      {
      U_TRACE(0, "UXMLElement::addAttribute(%p)", attribute)

      attributeContainer.push_back(attribute);
      }

   // Tells the caller how many attributes this element contains

   uint32_t numAttributes() const { return attributeContainer.size(); }

   // Returns the UXMLAttribute at a specific index

   UXMLAttribute* attributeAt(uint32_t index) const { return attributeContainer[index]; }

   // Given an attribute name, this finds the named attribute.
   // If you are looking for a fully qualified attribute (namespace plus base name),
   // just pass it in. I'll find the colon (':')

   UXMLAttribute* getAttribute(const UString& attributeName);

   static void splitNamespaceAndName(const UString& str, UString& namespaceName, UString& accessorName);

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UXMLElement& e);

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UString str,
           value,         // Value of the element
           accessorName,  // Name  of the element (minus namespace name)
           namespaceName; // Name  of the namespace (minus accessor name)

   UVector<UXMLAttribute*> attributeContainer; // Retrieve attributes using attribute name

private:
   U_DISALLOW_ASSIGN(UXMLElement)
};

#endif
