// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    node.h - wrapping of libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_UXML2NODE_H
#define ULIB_UXML2NODE_H 1

#include <ulib/container/vector.h>

#ifdef _MSWINDOWS_
#  include <libxml/xmlversion.h>
#  undef  XMLPUBVAR
#  define XMLPUBVAR __declspec(dllimport) extern
#endif

#include <libxml/tree.h>
#include <libxml/xpathInternals.h>

/*
Represents XML Nodes.
You should never new or delete Nodes. The Parser will create and manage them for you.

struct _xmlNode {
   void*             _private;   // application data
   xmlElementType    type;       // type number, must be second !
   const xmlChar*    name;       // the name of the node, or the entity
   struct _xmlNode*  children;   // parent->childs link
   struct _xmlNode*  last;       // last child link
   struct _xmlNode*  parent;     // child->parent link
   struct _xmlNode*  next;       // next sibling link
   struct _xmlNode*  prev;       // previous sibling link
   struct _xmlDoc*   doc;        // the containing document
   // End of common part
   xmlNs*            ns;         // pointer to the associated namespace
   xmlChar*          content;    // the content
   struct _xmlAttr*  properties; // properties list
   xmlNs*            nsDef;      // namespace definitions on this node
   void*             psvi;       // for type/PSVI informations
   unsigned short    line;       // line number
   unsigned short    extra;      // extra data for XPath/XSLT
};
*/

class U_EXPORT UXML2Node {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UXML2Node(xmlNodePtr node) : impl_(node)
      {
      U_TRACE_REGISTER_OBJECT(0, UXML2Node, "%p", node)
      }

   ~UXML2Node()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UXML2Node)
      }

   /**
    * Get the name of this node.
    *
    * @returns The node's name
    */

   const char* getName() const
      {
      U_TRACE_NO_PARAM(0, "UXML2Node::getName()")

      U_INTERNAL_ASSERT_POINTER(impl_)

      const char* result = (impl_->name ? (const char*)impl_->name : "");

      U_RETURN(result);
      }

   /**
    * Set the name of this node.
    *
    * @param name The new name for the node
    */

   void setName(const xmlChar* name)
      {
      U_TRACE(1, "UXML2Node::setName(%S)", name)

      U_INTERNAL_ASSERT_POINTER(impl_)

      U_SYSCALL_VOID(xmlNodeSetName, "%p,%S", impl_, name);
      }

   // Look for existing namespace to use

   xmlNsPtr getNamespace(const xmlChar* ns_prefix)
      {
      U_TRACE(1, "UXML2Node::getNamespace(%S)", ns_prefix)

      U_INTERNAL_ASSERT_POINTER(impl_)
      U_INTERNAL_ASSERT_POINTER(impl_->doc)

      // Look for the existing namespace to use

      xmlNsPtr ns = (xmlNsPtr) U_SYSCALL(xmlSearchNs, "%p,%p,%S", impl_->doc, impl_, ns_prefix);

      U_RETURN_POINTER(ns,xmlNs);
      }

   void setNameSpace(xmlNsPtr ns)
      {
      U_TRACE(1, "UXML2Node::setNameSpace(%p)", ns)

      U_INTERNAL_ASSERT_POINTER(impl_)

      U_SYSCALL_VOID(xmlSetNs, "%p,%S", impl_, ns); // Use it for this element
      }

   /**
    * Set the namespace prefix used by the node.
    *
    * If no such namespace prefix has been declared then this method return false.
    *
    * @param ns_prefix The namespace prefix
    */

   bool setNameSpace(const xmlChar* ns_prefix = 0)
      {
      U_TRACE(1, "UXML2Node::setNameSpace(%S)", ns_prefix)

      xmlNsPtr ns = getNamespace(ns_prefix);

      if (ns)
         {
         setNameSpace(ns);

         U_RETURN(true);
         }

      U_RETURN(false);
      }

  /**
   * This adds a namespace declaration to this node which will apply to this node and all children.
   *
   * @param ns_uri    The namespace to associate with the prefix, or to use as the default namespace if no prefix is specified.
   * @param ns_prefix The namespace prefix. If no prefix is specified then the namespace URI will be the default namespace
   */

   void setNameSpaceDeclaration(const char* ns_uri, const char* ns_prefix = 0)
      {
      U_TRACE(1, "UXML2Node::setNameSpaceDeclaration(%S,%S)", ns_uri, ns_prefix)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlNsPtr ns = (xmlNsPtr) U_SYSCALL(xmlNewNs, "%p,%S,%S", impl_, (const xmlChar*)ns_uri, (const xmlChar*)ns_prefix);

      setNameSpace(ns);
      }

   const char* getNameSpacePrefix() const
      {
      U_TRACE_NO_PARAM(0, "UXML2Node::getNameSpacePrefix()")

      U_INTERNAL_ASSERT_POINTER(impl_)

      // check for impl_ if is actually of type xmlDoc, instead of just xmlNode.
      // This can be an issue when calling this method on a UXML2Node returned by find().
      // Therefore, a call to impl_->ns would be invalid

      const char* result = (impl_->type != XML_DOCUMENT_NODE && impl_->ns && impl_->ns->prefix ? (const char*)impl_->ns->prefix : "");

      U_RETURN(result);
      }

   const xmlChar* getNameSpaceUri();

   /**
    * Checks that the node has a given name and a given namespace href.
    *
    * @param cur   the pointer to an XML node
    * @param name  the name
    * @param ns    the namespace href
    *
    * @returns     true if the node matches or false otherwise
    */

   bool checkNodeName(const xmlChar* name, const xmlChar* ns)
      {
      U_TRACE(0, "UXML2Node::checkNodeName(%S,%S)", name, ns)

      U_INTERNAL_ASSERT_POINTER(impl_)

      bool result = (xmlStrEqual(impl_->name, name) && xmlStrEqual(getNameSpaceUri(), ns));

      U_RETURN(result);
      }

   static bool checkNodeName(xmlNodePtr node, const xmlChar* name, const xmlChar* ns);

   /**
    * Discover at what line number this node occurs in the XML file.
    *
    * @returns The line number
    */

   int getLine() const { return XML_GET_LINE(impl_); }

   /**
    * Get the parent element for this node.
    *
    * @returns The parent node
    */

   xmlNodePtr getParent() const { return (impl_->parent && impl_->parent->type == XML_ELEMENT_NODE ? impl_->parent : 0); }

   /**
    * Get the next sibling for this node.
    *
    * @returns The next sibling
    */

   static __pure xmlNodePtr getNextSibling(xmlNodePtr node)
      {
      U_TRACE(0, "UXML2Node::getNextSibling(%p)", node)

      while (node && (node->type != XML_ELEMENT_NODE)) node = node->next;

      U_RETURN_POINTER(node, xmlNode);
      }

   xmlNodePtr getNextSibling() const { return getNextSibling(impl_->next); }

   /**
    * Get the previous sibling for this node.
    *
    * @returns The previous sibling
    */

   xmlNodePtr getPreviousSibling() const { return impl_->prev; }

   /**
    * Obtain the vector of child nodes. You may optionally obtain a vector of only the child nodes which have a certain name.
    *
    * @param children The vector of child nodes.
    * @param name     The names of the child nodes to get. If you do not specify a name, then the vector will contain all nodes, regardless of their names
    */

   uint32_t getChildren(UVector<xmlNodePtr>& children, const xmlChar* name) const;

   /**
    * Add a child element to this node.
    *
    * @param name      The new node name
    * @param ns_prefix The namespace prefix.
    *
    * @returns The newly-created element
    */

   xmlNodePtr addChild(const xmlChar* name, const xmlChar* ns_prefix = 0)
      {
      U_TRACE(1, "UXML2Node::addChild(%S,%S)", name, ns_prefix)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlNodePtr node  = 0;
      xmlNodePtr child = createNewChildNode(name, ns_prefix);

      if (child) node = (xmlNodePtr) U_SYSCALL(xmlAddChild, "%p,%p", impl_, child);

      U_RETURN_POINTER(node,_xmlNode);
      }

   /**
    * Add a child element to this node after the specified existing child node.
    *
    * @param previous_sibling  An existing child node.
    * @param name              The new node name
    * @param ns_prefix         The namespace prefix. If the prefix has not been declared then this method will throw an exception.
    *
    * @returns The newly-created element
    */

   xmlNodePtr addChild(xmlNodePtr previous_sibling, const xmlChar* name, const xmlChar* ns_prefix = 0)
      {
      U_TRACE(1, "UXML2Node::addChild(%p,%S,%S)", previous_sibling, name, ns_prefix)

      U_INTERNAL_ASSERT_POINTER(previous_sibling)

      xmlNodePtr node  = 0;
      xmlNodePtr child = createNewChildNode(name, ns_prefix);

      if (child) node = (xmlNodePtr) U_SYSCALL(xmlAddNextSibling, "%p,%p", previous_sibling, child);

      U_RETURN_POINTER(node,_xmlNode);
      }

   /**
    * Add a child element to this node before the specified existing child node.
    *
    * @param next_sibling An existing child node.
    * @param name         The new node name
    * @param ns_prefix    The namespace prefix. If the prefix has not been declared then this method will throw an exception.
    *
    * @returns The newly-created element
    */

   xmlNodePtr addChildBefore(xmlNodePtr next_sibling, const xmlChar* name, const xmlChar* ns_prefix = 0)
      {
      U_TRACE(1, "UXML2Node::addChildBefore(%p,%S,%S)", next_sibling, name, ns_prefix)

      U_INTERNAL_ASSERT_POINTER(next_sibling)

      xmlNodePtr node  = 0;
      xmlNodePtr child = createNewChildNode(name, ns_prefix);

      if (child) node = (xmlNodePtr) U_SYSCALL(xmlAddPrevSibling, "%p,%p", next_sibling, child);

      U_RETURN_POINTER(node,_xmlNode);
      }

   /** Remove the child node.
    *
    * @param node The child node to remove. This Node will be deleted and therefore unusable after calling this method
    */

   static void removeChild(xmlNodePtr node)
      {
      U_TRACE(1, "UXML2Node::removeChild(%p)", node)

      U_SYSCALL_VOID(xmlUnlinkNode, "%p", node);
      U_SYSCALL_VOID(xmlFreeNode,   "%p", node);
      }

   /**
    * Search and get the value of an attribute associated to a node
    */

   static const char* getProp(xmlNodePtr node, const char* name)
      {
      U_TRACE(1, "UXML2Node::getProp(%p,%S)", node, name)

      const char* prop = (const char*) U_SYSCALL(xmlGetProp, "%p,%S", node, (const xmlChar*)name);

      U_RETURN(prop);
      }

   /**
    * Get the value of an content associated to a node
    */

   static xmlChar* getContent(xmlNodePtr node)
      {
      U_TRACE(1, "UXML2Node::getContent(%p)", node)

      xmlChar* content = (xmlChar*) U_SYSCALL(xmlNodeGetContent, "%p", node);

      U_RETURN_POINTER(content, xmlChar);
      }

   /**
    * Import node(s) from another document under this node, without affecting the source node.
    *
    * @param node      The node to copy and insert under the current node.
    * @param recursive Whether to import the child nodes also. Defaults to true.
    *
    * @returns The newly-created node
    */

   xmlNodePtr importNode(const xmlNodePtr node, bool recursive = true);

   /**
    * Return the XPath of this node.
    *
    * @result The XPath of the node
    */

   const char* getPath() const
      {
      U_TRACE_NO_PARAM(1, "UXML2Node::getPath()")

      U_INTERNAL_ASSERT_POINTER(impl_)

      const char* path = (const char*) U_SYSCALL(xmlGetNodePath, "%p", impl_);

      U_RETURN(path);
      }

   /**
    * Find nodes from a XPath expression.
    *
    * @param xpath The XPath of the nodes
    */

   uint32_t find(UVector<xmlNodePtr>& vec, const xmlChar* xpath) const
      {
      U_TRACE(1, "UXML2Node::find(%p,%S)", &vec, xpath)

      U_INTERNAL_ASSERT_POINTER(impl_)
      U_INTERNAL_ASSERT_POINTER(impl_->doc)

      xmlXPathContextPtr ctxt = (xmlXPathContextPtr) U_SYSCALL(xmlXPathNewContext, "%p", impl_->doc);

      ctxt->node = impl_;

      return find_impl(vec, ctxt, xpath);
      }

   /**
    * Find nodes from a XPath expression.
    *
    * @param xpath       The XPath of the nodes.
    * @param namespaces  A vector of namespace prefixes to namespace URIs to be used while finding
    */

   uint32_t find(UVector<xmlNodePtr>& vec, const xmlChar* xpath, UVector<UString>& namespaces) const;

   // Access the underlying libxml2 implementation

   xmlNodePtr cobj() { return impl_; }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   xmlNodePtr impl_;

   static uint32_t find_impl(UVector<xmlNodePtr>& vec, xmlXPathContext* ctxt, const xmlChar* xpath);

   // Create the C instance ready to be added to the parent node

   xmlNodePtr createNewChildNode(const xmlChar* name, const xmlChar* ns_prefix = 0);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UXML2Node)
};

#endif
