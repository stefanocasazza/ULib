// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    context.h - xml Digital SIGnature with libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DSIG_CONTEXT_H
#define ULIB_DSIG_CONTEXT_H 1

#include "transforms.h"

class UDSIGContext;
class UReferenceCtx;

// xml Digital SIGnature processing context

class UTransformCtx {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // URI transform type bit mask

   enum UriType {
   // NONE          = 0x0000, // URI type is unknown or not set
      EMPTY         = 0x0001, // empty URI ("") type
      SAME_DOCUMENT = 0x0002, // same document ("#...") but not empty ("") URI type
      TYPE_LOCAL    = 0x0004, // local URI ("file:///....") type
      TYPE_REMOTE   = 0x0008, // remote URI type
      TYPE_ANY      = 0xFFFF  // Any URI type
   };

   // COSTRUTTORI

   UTransformCtx() : chain(5)
      {
      U_TRACE_CTOR(0, UTransformCtx, "")

      uri         = U_NULLPTR;
      status      = 0;
      xptrExpr    = U_NULLPTR;
      enabledUris = TYPE_ANY;
      }

   ~UTransformCtx()
      {
      U_TRACE_DTOR(0, UTransformCtx)

      if (uri)
         {
         U_SYSCALL_FREE((void*)uri);
         U_SYSCALL_FREE((void*)xptrExpr);
         }
      }

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   int status;           // the transforms chain processing status
   int enabledUris;      // the allowed transform data source uri types
   const char* uri;      // the data source URI without xpointer expression
   const char* xptrExpr; // the xpointer expression from data source URI (if any)

   UVector<UBaseTransform*> chain;

   static UVector<UString>* enabledTransforms;

   // SERVICES

   bool execute(UString& data);
   bool setURI(const char* uri, xmlNodePtr node);
   bool nodesListRead(xmlNodePtr node, int usage);
   bool verifyNodeContent(xmlNodePtr node, UString& signature_value);

   static void            registerDefault();
   static UBaseTransform* findByHref(const char* href);
   static UBaseTransform* nodeRead(xmlNodePtr node, int usage);

private:
   U_DISALLOW_ASSIGN(UTransformCtx)

   friend class UDSIGContext;
   friend class UReferenceCtx;
};

class U_EXPORT UDSIGContext {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

    UDSIGContext();
   ~UDSIGContext();

   // SERVICES

   bool verify(UXML2Document& document, const char*& alg, UString& data, UString& signature);

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   int                     status;               // the <dsig:Signature/> processing status.
   int                     operation;            // the operation: sign or verify.
   int                     enabledReferenceUris; // the URI types allowed for <dsig:Reference/> node.
   xmlNodePtr              keyInfoNode;          // the pointer to <dsig:keyInfo/> node.
   xmlNodePtr              signValueNode;        // the pointer to <dsig:SignatureValue/> node.
   xmlNodePtr              signedInfoNode;       // the pointer to <dsig:signedInfo/> node.
   const char*             id;                   // the pointer to Id attribute of <dsig:Signature/> node.
   UTransformCtx           transformCtx;         // the <dsig:SignedInfo/> node processing context.
   UBaseTransform*         signMethod;           // the pointer to signature transform.
   UBaseTransform*         c14nMethod;           // the pointer to c14n transform.
   UVector<UReferenceCtx*> manifestReferences;   // the list of references in <dsig:Manifest/> nodes.
   UVector<UReferenceCtx*> signedInfoReferences; // the list of references in <dsig:SignedInfo/> node.      

   static UDSIGContext* pthis;

   // SERVICES

   bool processKeyInfoNode();
   bool processObjectNode(xmlNodePtr objectNode);
   bool processManifestNode(xmlNodePtr manifestNode);
   bool processSignedInfoNode(const char*& alg, UString& data);
   bool processSignatureNode(xmlNodePtr signature, const char*& alg, UString& data);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UDSIGContext)
   
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

/* The <dsig:Reference/> processing context
 *
 * Reference is an element that may occur one or more times. It specifies
 * a digest algorithm and digest value, and optionally an identifier of the 
 * object being signed, the type of the object, and/or a list of transforms 
 * to be applied prior to digesting. The identification (URI) and transforms 
 * describe how the digested content (i.e., the input to the digest method) 
 * was created. The Type attribute facilitates the processing of referenced 
 * data. For example, while this specification makes no requirements over 
 * external data, an application may wish to signal that the referent is a 
 * Manifest. An optional ID attribute permits a Reference to be referenced 
 * from elsewhere
 */

class UReferenceCtx {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum Origin {
      MANIFEST,   // reference in <dsig:Manifest> node
      SIGNED_INFO // reference in <dsig:SignedInfo> node
   };

   enum Status {
      UNKNOWN   = 0, // the status is unknow
      SUCCEEDED = 1, // the processing succeeded
      INVALID   = 2  // the processing failed
   };

   // COSTRUTTORI

   UReferenceCtx(int org)
      {
      U_TRACE_CTOR(0, UReferenceCtx, "%d", org)

      id     = U_NULLPTR;
      uri    = U_NULLPTR;
      type   = U_NULLPTR;
      status = UNKNOWN;
      origin = org;
      }

   ~UReferenceCtx()
      {
      U_TRACE_DTOR(0, UReferenceCtx)
      }

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   int status;                 // the reference processing status
   int origin;                 // the reference processing transforms context
   const char* id;             // the <dsig:Reference/> node ID attribute
   const char* uri;            // the <dsig:Reference/> node URI attribute
   const char* type;           // the <dsig:Reference/> node Type attribute
   UTransformCtx transformCtx; // the reference processing transforms context

   // SERVICES

   bool processNode(xmlNodePtr node);

private:
   U_DISALLOW_ASSIGN(UReferenceCtx)
   
   friend class UDSIGContext;
};

#endif
