// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    node.cpp - wrapping of libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/libxml2/node.h>

const xmlChar* UXML2Node::getNameSpaceUri()
{
   U_TRACE_NO_PARAM(0, "UXML2Node::getNameSpaceUri()")

   U_INTERNAL_ASSERT_POINTER(impl_)

   // check for impl_ if is actually of type xmlDoc, instead of just xmlNode.
   // This can be an issue when calling this method on a UXML2Node returned by find().
   // Therefore, a call to impl_->ns would be invalid

   const xmlChar* result = (impl_->type != XML_DOCUMENT_NODE && impl_->ns && impl_->ns->href ? impl_->ns->href : 0);

   // do we have a namespace in the node ?

   if (result == 0)
      {
      // search for default namespace

      xmlNsPtr ns = getNamespace(0);

      if (ns) result = ns->href;
      }

   U_RETURN_POINTER(result,xmlChar);
}

xmlNodePtr UXML2Node::importNode(const xmlNodePtr node, bool recursive)
{
   U_TRACE(1, "UXML2Node::importNode(%p,%b)", node, recursive)

   U_INTERNAL_ASSERT_POINTER(impl_)
   U_INTERNAL_ASSERT_POINTER(impl_->doc)

   // Create the node, by copying:

   xmlNodePtr imported_node = (xmlNodePtr) U_SYSCALL(xmlDocCopyNode, "%p,%p,%d", node, impl_->doc, recursive);

   if (imported_node)
      {
      // Add the node:

      xmlNodePtr added_node = (xmlNodePtr) U_SYSCALL(xmlAddChild, "%p,%p", impl_, imported_node);

      if (!added_node)
         {
         U_SYSCALL_VOID(xmlFreeNode, "%p", imported_node);

         imported_node = 0;
         }
      }

   U_RETURN_POINTER(imported_node,_xmlNode);
}

uint32_t UXML2Node::find(UVector<xmlNodePtr>& vec, const xmlChar* xpath, UVector<UString>& namespaces) const
{
   U_TRACE(1, "UXML2Node::find(%p,%S,%p)", &vec, xpath, &namespaces)

   U_INTERNAL_ASSERT_POINTER(impl_)
   U_INTERNAL_ASSERT_POINTER(impl_->doc)

   xmlXPathContextPtr ctxt = (xmlXPathContextPtr) U_SYSCALL(xmlXPathNewContext, "%p", impl_->doc);

   ctxt->node = impl_;

   for (int32_t i = 0, n = namespaces.size(); i < n; i += 2)
      {
      U_SYSCALL_VOID(xmlXPathRegisterNs, "%p,%S,%S", ctxt, (const xmlChar*)namespaces[i].c_str(),
                                                           (const xmlChar*)namespaces[i+1].c_str());
      }

   return find_impl(vec, ctxt, xpath);
}

uint32_t UXML2Node::getChildren(UVector<xmlNodePtr>& children, const xmlChar* name) const
{
   U_TRACE(0, "UXML2Node::getChildren(%p,%S)", &children, name)

   U_INTERNAL_ASSERT_POINTER(impl_)

   xmlNodePtr child = impl_->children;

   if (child == 0) U_RETURN(0);

   uint32_t n = children.size();

   do {
      if ((name == 0 || strcmp((const char*)name, (const char*)child->name) == 0)) children.push_back(child);
      }
   while ((child = child->next));

   U_RETURN(children.size() - n);
}

xmlNodePtr UXML2Node::createNewChildNode(const xmlChar* name, const xmlChar* ns_prefix)
{
   U_TRACE(1, "UXML2Node::createNewChildNode(%S,%S)", name, ns_prefix)

   U_INTERNAL_ASSERT_POINTER(impl_)
   U_ASSERT_DIFFERS(impl_->type, XML_ELEMENT_NODE)

   xmlNsPtr ns = 0;

   // Ignore the namespace if none was specified

   if (ns_prefix)
      {
      // Use the existing namespace if one exists

      ns = getNamespace(ns_prefix);

      U_INTERNAL_ASSERT_POINTER(ns)
      }

   xmlNodePtr node = (xmlNodePtr) U_SYSCALL(xmlNewNode, "%p,%S", ns, name);

   U_RETURN_POINTER(node,_xmlNode);
}

uint32_t UXML2Node::find_impl(UVector<xmlNodePtr>& vec, xmlXPathContext* ctxt, const xmlChar* xpath)
{
   U_TRACE(1, "UXML2Node::find_impl(%p,%p,%S)", &vec, ctxt, xpath)

   xmlNodeSetPtr nodeset;
   uint32_t n = vec.size();
   xmlXPathObjectPtr result = (xmlXPathObjectPtr) U_SYSCALL(xmlXPathEval, "%S,%p", xpath, ctxt);

   if (result       == 0)             goto ctx;
   if (result->type != XPATH_NODESET) goto path;

   nodeset = result->nodesetval;

   /*
   struct _xmlNodeSet {
      int nodeNr;          // number of nodes in the set
      int nodeMax;         // size of the array as allocated
      xmlNodePtr* nodeTab; // array of nodes in no particular order
   };
   */

   if (nodeset)
      {
      vec.reserve(nodeset->nodeNr);

      for (int i = 0; i < nodeset->nodeNr; ++i) vec.push_back(nodeset->nodeTab[i]);
      }

path:
   U_SYSCALL_VOID(xmlXPathFreeObject,  "%p", result);
ctx:
   U_SYSCALL_VOID(xmlXPathFreeContext, "%p", ctxt);

   U_RETURN(vec.size() - n);
}

bool UXML2Node::checkNodeName(xmlNodePtr node, const xmlChar* name, const xmlChar* ns)
{
   U_TRACE(0, "UXML2Node::checkNodeName(%p,%S,%S)", node, name, ns)

   bool result = (node ? UXML2Node(node).checkNodeName(name, ns) : false);

   U_RETURN(result);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UXML2Node::dump(bool reset) const
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
