// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    element.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/escape.h>
#include <ulib/xml/expat/element.h>

void UXMLElement::splitNamespaceAndName(const UString& _str, UString& _namespaceName, UString& _accessorName)
{
   U_TRACE(0+256, "UXMLElement::splitNamespaceAndName(%V,%p,%p)", _str.rep, &_namespaceName, &_accessorName)

   U_INTERNAL_ASSERT(_str)

   uint32_t nColonPos = _str.find(':');

   if (nColonPos != U_NOT_FOUND)
      {
      _namespaceName = _str.substr(0U, nColonPos);
       _accessorName  = _str.substr(nColonPos + 1, _str.length() - nColonPos - 1);
      }
   else
      {
      _namespaceName.clear();
       _accessorName = _str;
      }
}

UXMLAttribute* UXMLElement::getAttribute(const UString& attributeName)
{
   U_TRACE(0, "UXMLElement::getAttribute(%V)", attributeName.rep)

   uint32_t n = numAttributes(); 

   if (n)
      {
      UString szAccessor, szNamespace;

      splitNamespaceAndName(attributeName, szNamespace, szAccessor);

      for (uint32_t i = 0; i < n; ++i)
         {
         UXMLAttribute* entry = attributeAt(i);

         if ((entry->getAccessor() == attributeName) &&
             (szNamespace.empty() || (entry->getNamespaceName() == szNamespace)))
            {
            // We found the attribute

            U_RETURN_POINTER(entry, UXMLAttribute);
            }
         }
      }

   U_RETURN_POINTER(0, UXMLAttribute);
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, const UXMLElement& e)
{
   U_TRACE(0+256, "UXMLElement::operator<<(%p,%p)", &os, &e)

   // encode the element name

   os.put('<');

   if (e.namespaceName)
      {
      os << e.namespaceName;

      os.put(':');
      }

   os << e.accessorName;

   os.put(' ');

   // encode the element's attributes

   os << e.attributeContainer;

   // terminate the element declaration

   os.put(' ');
   os.put('>');

   if (e.value)
      {
      (void) os.write(U_CONSTANT_TO_PARAM(" VALUE = "));

      char buffer[4096];
      uint32_t len = u_escape_encode((const unsigned char*)U_STRING_TO_PARAM(e.value), buffer, sizeof(buffer));

      (void) os.write(buffer, len);
      }

   return os;
}

#  ifdef DEBUG
const char* UXMLElement::dump(bool reset) const
{
   *UObjectIO::os << "str                (UString                 " << (void*)&str                  << ")\n"
                  << "value              (UString                 " << (void*)&value                << ")\n"
                  << "accessorName       (UString                 " << (void*)&accessorName         << ")\n"
                  << "namespaceName      (UString                 " << (void*)&namespaceName        << ")\n"
                  << "attributeContainer (UVector<UXMLAttribute*> " << (void*)&attributeContainer   << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
