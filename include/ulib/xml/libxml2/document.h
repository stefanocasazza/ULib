// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    document.h - wrapping of libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_UXML2DOCUMENT_H
#define ULIB_UXML2DOCUMENT_H 1

#include <ulib/xml/libxml2/node.h>

/*
Represents an XML document in the DOM model.

struct _xmlDoc {
   void*             _private;   // application data
   xmlElementType    type;       // XML_DOCUMENT_NODE, must be second !
   char*             name;       // name/filename/URI of the document
   struct _xmlNode*  children;   // the document tree
   struct _xmlNode*  last;       // last child link
   struct _xmlNode*  parent;     // child->parent link
   struct _xmlNode*  next;       // next sibling link
   struct _xmlNode*  prev;       // previous sibling link
   struct _xmlDoc*   doc;        // autoreference to itself
   // End of common part
   int compression;              // level of zlib compression
   int standalone;               // standalone document (no external refs)
                                 //  1 if standalone = "yes"
                                 //  0 if standalone = "no"
                                 // -1 if there is no XML declaration
                                 // -2 if there is an XML declaration, but no standalone attribute was specified
   struct _xmlDtd*   intSubset;  // the document internal subset
   struct _xmlDtd*   extSubset;  // the document external subset
   struct _xmlNs*    oldNs;      // Global namespace, the old way
   const xmlChar*    version;    // the XML version string
   const xmlChar*    encoding;   // external initial encoding, if any
   void*             ids;        // Hash table for ID attributes if any
   void*             refs;       // Hash table for IDREFs attributes if any
   const xmlChar*    URL;        // The URI for that document
   int charset;                  // encoding of the in-memory content actually an xmlCharEncoding
   struct _xmlDict*  dict;       // dict used to allocate names or NULL
   void*             psvi;       // for type/PSVI informations
   int parseFlags;               // set of xmlParserOption used to parse the document
   int properties;               // set of xmlDocProperties for this document set at the end of parsing
};
*/

class UDSIGContext;

class U_EXPORT UXML2Document {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UXML2Document()
      {
      U_TRACE_REGISTER_OBJECT(0, UXML2Document, "", 0)

      impl_ = (xmlDocPtr) U_SYSCALL(xmlNewDoc, "%S", (xmlChar*)"1.0");
      }

   UXML2Document(const UString& data);

   ~UXML2Document()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UXML2Document)

      U_SYSCALL_VOID(xmlFreeDoc, "%p", impl_);
      }

   // SERVICES

   uint32_t getElement(UVector<UString>& velement,     const char* tag, uint32_t tag_len);
   uint32_t getElement(UString& element, uint32_t pos, const char* tag, uint32_t tag_len);
   UString  getElementData(              uint32_t pos, const char* tag, uint32_t tag_len);

   /**
    * getEncoding()
    *
    * @return The encoding used in the source from which the document has been loaded
    */

   const char* getEncoding() const { return (const char*)impl_->encoding; }

   xmlDtdPtr getInternalSubset() const
      {
      U_TRACE_NO_PARAM(1, "UXML2Document::getInternalSubset()")

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlDtdPtr dtd = (xmlDtdPtr) U_SYSCALL(xmlGetIntSubset, "%p", impl_);

      U_RETURN_POINTER(dtd,_xmlDtd);
      }

   xmlDtdPtr setInternalSubset(const char* name, const char* external_id = 0, const char* system_id = 0)
      {
      U_TRACE(1, "UXML2Document::setInternalSubset(%S,%S,%S)", name, external_id, system_id)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlDtdPtr dtd = (xmlDtdPtr) U_SYSCALL(xmlCreateIntSubset, "%p,%S,%S,%S", impl_, (const xmlChar*)name,
                                                                                      (const xmlChar*)external_id,
                                                                                      (const xmlChar*)system_id);

      U_RETURN_POINTER(dtd,_xmlDtd);
      }

   /**
    * Return the root node.
    *
    * This function does _not_ create a default root node if it doesn't exist.
    *
    * @return A pointer to the root node if it exists, 0 otherwise
    */

   xmlNodePtr getRootNode() const
      {
      U_TRACE(1, "UXML2Document::getRootNode()")

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlNodePtr root = (xmlNodePtr) U_SYSCALL(xmlDocGetRootElement, "%p", impl_);

      U_RETURN_POINTER(root,_xmlNode);
      }

   /**
    * Creates the root node.
    *
    * @param name      The node's name.
    * @param ns_uri    The namespace URI. A namspace declaration will be added to this node, because it could not have been declared before.
    * @param ns_prefix The namespace prefix to associate with the namespace. If no namespace prefix is specified then the namespace URI will
    *                  be the default namespace.
    *
    * @return A pointer to the new root node
    */

   xmlNodePtr createRootNode(const char* name, const char* ns_uri = 0, const char* ns_prefix = 0)
      {
      U_TRACE(1, "UXML2Document::setInternalSubset(%S,%S,%S)", name, ns_uri, ns_prefix)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlNodePtr node = (xmlNodePtr) U_SYSCALL(xmlNewDocNode, "%p,%p,%S,%S", impl_, 0, (const xmlChar*)name, 0);

      U_SYSCALL_VOID(xmlDocSetRootElement, "%p,%p", impl_, node);

      xmlNodePtr root = getRootNode();

      if (ns_uri) UXML2Node(root).setNameSpaceDeclaration(ns_uri, ns_prefix);

      U_RETURN_POINTER(root,_xmlNode);
      }

   /**
    * Creates a root node by importing the node from another document, without affecting the source node.
    *
    * @param node      The node to copy and insert as the root node of the document
    * @param recursive Whether to import the child nodes also. Defaults to true.
    *
    * @return A pointer to the new root node
    */

   xmlNodePtr createRootNodeByImport(const xmlNodePtr node, bool recursive = true)
      {
      U_TRACE(1, "UXML2Document::createRootNodeByImport(%p,%b)", node, recursive)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlNodePtr imported_node = (xmlNodePtr) U_SYSCALL(xmlDocCopyNode, "%p,%p,%b", node, impl_, recursive);

      if (imported_node) U_SYSCALL_VOID(xmlDocSetRootElement, "%p,%p", impl_, node);

      xmlNodePtr root = getRootNode();

      U_RETURN_POINTER(root,_xmlNode);
      }

   /**
    * Append a new comment node.
    *
    * @param content The text. This should be unescaped
    *
    * @returns The new comment node
    */

   xmlNodePtr addComment(const char* content)
      {
      U_TRACE(1, "UXML2Document::addComment(%S)", content)

      U_INTERNAL_ASSERT_POINTER(impl_)
      U_INTERNAL_ASSERT_POINTER(impl_->doc)

      xmlNodePtr node = (xmlNodePtr) U_SYSCALL(xmlNewComment, "%S", (const xmlChar*)content);

      // Use the result, because node can be freed when merging text nodes:

      if (node) node = (xmlNodePtr) U_SYSCALL(xmlAddChild, "%p,%p", (xmlNodePtr)impl_, node);

      U_RETURN_POINTER(node,_xmlNode);
      }

   /**
    * Add an Entity declaration to the document.
    *
    * @param name     The name of the entity that will be used in an entity reference.
    * @param type     The type of entity.
    * @param publicId The public ID of the subset.
    * @param systemId The system ID of the subset.
    * @param content  The value of the Entity. In entity reference substitutions, this is the replacement value
    */

   xmlEntityPtr setEntityDeclaration(const char* name, xmlEntityType type, const char* publicId, const char* systemId, const char* content)
      {
      U_TRACE(1, "UXML2Document::setEntityDeclaration(%S,%d,%S,%S,%S)", name, type, publicId, systemId, content)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlEntityPtr entity = (xmlEntityPtr) U_SYSCALL(xmlAddDocEntity, "%p,%S,%d,%S,%S,%S", impl_, (const xmlChar*)name, type,
                                                                                                  (const xmlChar*)publicId,
                                                                                                  (const xmlChar*)systemId,
                                                                                                  (const xmlChar*)content);

      U_RETURN_POINTER(entity,_xmlEntity);
      }

   /**
    * Searches all children of the @parent node having given name and namespace href.
    *
    * @param parent  the pointer to XML node.
    * @param name    the name.
    * @param ns      the namespace href (may be NULL).
    *
    * @return        the pointer to the found node or NULL if an error occurs or node is not found
    */

   xmlNodePtr findNode(const xmlNodePtr parent, const xmlChar* name, const xmlChar* ns);

   /**
    * Searches a direct child of the @parent node having given name and namespace href.
    *
    * @param parent  the pointer to XML node.
    * @param name    the name.
    * @param ns      the namespace href (may be NULL).
    *
    * @return        the pointer to the found node or NULL if an error occurs or node is not found
    */

   xmlNodePtr findChild(const xmlNodePtr parent, const xmlChar* name, const xmlChar* ns);

   /**
    * Searches the ancestors axis of the @cur node for a node having given name and namespace href.
    *
    * @param cur     the pointer to an XML node.
    * @param name    the name.
    * @param ns      the namespace href (may be NULL).
    *
    * @return        the pointer to the found node or NULL if an error occurs or node is not found
    */

   xmlNodePtr findParent(const xmlNodePtr cur, const xmlChar* name, const xmlChar* ns);

   /**
    * Write the document to a file.
    *
    * @param filename
    * @param encoding  If not provided, UTF-8 is used
    * @param formatted The output is formatted by inserting whitespaces, which is easier to read for a human,
    *                  but may insert unwanted significant whitespaces. Use with care !
    */

   bool writeToFile(const char* filename, const char* encoding = 0, bool formatted = false);

   /**
    * Write the document to the memory.
    *
    * @param encoding  If not provided, UTF-8 is used
    * @param formatted The output is formatted by inserting whitespaces, which is easier to read for a human,
    *                  but may insert unwanted significant whitespaces. Use with care !
    *
    * @return The written document
    */

   xmlChar* writeToString(int& length, const char* encoding = 0, bool formatted = false);

   /**
    * Canonical XML implementation (http://www.w3.org/TR/2001/REC-xml-c14n-20010315)
    *
    * Enum xmlC14NMode {
    *    XML_C14N_1_0           = 0 : Origianal C14N 1.0 spec
    *    XML_C14N_EXCLUSIVE_1_0 = 1 : Exclusive C14N 1.0 spec
    *    XML_C14N_1_1           = 2 : C14N 1.1 spec
    * }
    */

          UString xmlC14N(                     int mode = 2, int with_comments = 0, unsigned char** inclusive_namespaces = 0);
   static UString xmlC14N(const UString& data, int mode = 2, int with_comments = 0, unsigned char** inclusive_namespaces = 0);

   // Access the underlying libxml2 implementation.

   xmlDocPtr cobj() { return impl_; }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString data;
   xmlDocPtr impl_;

   static bool binit;

   static void init();

   UXML2Document(xmlDocPtr doc) : impl_(doc)
      {
      U_TRACE_REGISTER_OBJECT(0, UXML2Document, "%p", doc)
      }

   /**
    * Retrieve an Entity.
    *
    * The entity can be from an external subset or internally declared.
    *
    * @param name Then name of the entity to get.
    *
    * @returns A pointer to the libxml2 entity structure
    */

   xmlEntityPtr getEntity(const char* name)
      {
      U_TRACE(1, "UXML2Document::getEntity(%S)", name)

      U_INTERNAL_ASSERT_POINTER(impl_)

      xmlEntityPtr node = (xmlEntityPtr) U_SYSCALL(xmlGetDocEntity, "%p,%S", impl_, (const xmlChar*)name);

      U_RETURN_POINTER(node,_xmlEntity);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UXML2Document)

   friend class UDSIGContext;
};

#endif
