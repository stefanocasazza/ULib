// ============================================================================
//
// = LIBRARY
//    ulib - c++ library
//
// = FILENAME
//    transforms.h - xml Digital SIGnature with libxml2
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DSIG_TRANSFORM_H
#define ULIB_DSIG_TRANSFORM_H 1

#include <ulib/container/vector.h>
#include <ulib/xml/libxml2/document.h>

#include "xpath.h"

#include <libxml/uri.h>
#include <libxml/xmlIO.h>

class UXPathData;
class UDSIGContext;
class UReferenceCtx;
class UTransformCtx;

class UBaseTransform {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // The transform usage bit mask

   enum Usage {
   // NONE       = 0x0000, // usage is unknown or undefined
      DSIG       = 0x0001, // Transform could be used in <dsig:Transform>
      C14N       = 0x0002, // Transform could be used in <dsig:CanonicalizationMethod>
      DIGEST     = 0x0004, // Transform could be used in <dsig:DigestMethod>
      SIGNATURE  = 0x0008, // Transform could be used in <dsig:SignatureMethod>
      ENCRYPTION = 0x0010, // Transform could be used in <enc:EncryptionMethod>
      ANY        = 0xFFFF  // Transform could be used for operation
   };

   // The transform execution status

   enum State {
   // NONE     = 0,  // status unknown
      WORKING  = 1,  // transform is executed
      FINISHED = 2,  // transform finished
      OK       = 3,  // transform succeeded
      FAIL     = 4   // transform failed (an error occur)
   };

   // The transform operation

   enum Operation {
   // NONE     = 0,  // operation is unknown
      ENCODE   = 1,  // encode operation (for base64 transform)
      DECODE   = 2,  // decode operation (for base64 transform)
      SIGN     = 3,  // sign or digest operation
      VERIFY   = 4,  // verification of signature or digest operation
      ENCRYPT  = 5,  // encryption operation
      DECRYPT  = 6,  // decryption operation
   };

   // The transform operation mode

   enum Mode {
   // NONE = 0, // mode unknown
      PUSH = 1, // pushing data thru transform
      POP  = 2  // popping data from transform
   };

   enum DataType {
   // NONE   = 0x0000, // transform data type is unknown or nor data expected
      BINARY = 0x0001, // binary transform data
      XML    = 0x0002  // xml transform data
   };

   // COSTRUTTORI

            UBaseTransform();
   virtual ~UBaseTransform();

   // method VIRTUAL to define

   virtual int         usage() { return 0; } // the allowed transforms usages
   virtual const char* name()  { return 0; } // the transform's name
   virtual const char* href()  { return 0; } // the transform's identification string (href)

   virtual bool readNode(xmlNodePtr node) { return true; } // the XML node read method

   /**
    * Process binary @data by calling transform's execute method and pushes 
    * results to next transform.
    *
    * Returns: true on success or a false value if an error occurs.
    */

   virtual bool execute(UString& data) { return false; }

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   int status;          // the current status
   int operation;       // the transform's operation
   xmlNodePtr hereNode; // the pointer to transform's <dsig:Transform /> node

private:
   UBaseTransform& operator=(const UBaseTransform&) { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

/*
 * The Base64 transform klass (http://www.w3.org/TR/xmldsig-core/#sec-Base-64).
 * The normative specification for base64 decoding transforms is RFC 2045
 * (http://www.ietf.org/rfc/rfc2045.txt). The base64 Transform element has 
 * no content. The input is decoded by the algorithms. This transform is 
 * useful if an application needs to sign the raw data associated with 
 * the encoded content of an element.
 */

class UTranformBase64 : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformBase64()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformBase64, "")
      }

   virtual ~UTranformBase64()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformBase64)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; } // the allowed transforms usages
   virtual const char* name()  { return _name;  } // the transform's name
   virtual const char* href()  { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformBase64(const UTranformBase64&) : UBaseTransform() {}
   UTranformBase64& operator=(const UTranformBase64&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

// Inclusive (regular) canonicalization that omits comments transform class
// (http://www.w3.org/TR/xmldsig-core/#sec-c14nAlg and http://www.w3.org/TR/2001/REC-xml-c14n-20010315)

class U_EXPORT UTranformInclC14N : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformInclC14N()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformInclC14N, "")
      }

   virtual ~UTranformInclC14N()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformInclC14N)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; }  // the allowed transforms usages
   virtual const char* name()  { return _name;  }  // the transform's name
   virtual const char* href()  { return _href;  }  // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformInclC14N(const UTranformInclC14N&) : UBaseTransform() {}
   UTranformInclC14N& operator=(const UTranformInclC14N&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

class UTranformSha1 : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformSha1()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformSha1, "")
      }

   virtual ~UTranformSha1()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformSha1)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; } // the allowed transforms usages
   virtual const char* name()  { return _name;  } // the transform's name
   virtual const char* href()  { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformSha1(const UTranformSha1&) : UBaseTransform() {}
   UTranformSha1& operator=(const UTranformSha1&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

class UTranformSha256 : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformSha256()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformSha256, "")
      }

   virtual ~UTranformSha256()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformSha256)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; } // the allowed transforms usages
   virtual const char* name()  { return _name;  } // the transform's name
   virtual const char* href()  { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformSha256(const UTranformSha256&) : UBaseTransform() {}
   UTranformSha256& operator=(const UTranformSha256&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

class U_EXPORT UTranformRsaMd5 : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformRsaMd5()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformRsaMd5, "")
      }

   virtual ~UTranformRsaMd5()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformRsaMd5)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; } // the allowed transforms usages
   virtual const char* name()  { return _name;  } // the transform's name
   virtual const char* href()  { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformRsaMd5(const UTranformRsaMd5&) : UBaseTransform() {}
   UTranformRsaMd5& operator=(const UTranformRsaMd5&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

class UTranformRsaSha1 : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformRsaSha1()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformRsaSha1, "")
      }

   virtual ~UTranformRsaSha1()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformRsaSha1)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; } // the allowed transforms usages
   virtual const char* name()  { return _name;  } // the transform's name
   virtual const char* href()  { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformRsaSha1(const UTranformRsaSha1&) : UBaseTransform() {}
   UTranformRsaSha1& operator=(const UTranformRsaSha1&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

class UTranformRsaSha256 : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformRsaSha256()
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformRsaSha256, "")
      }

   virtual ~UTranformRsaSha256()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformRsaSha256)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; } // the allowed transforms usages
   virtual const char* name()  { return _name;  } // the transform's name
   virtual const char* href()  { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

private:
   UTranformRsaSha256(const UTranformRsaSha256&) : UBaseTransform() {}
   UTranformRsaSha256& operator=(const UTranformRsaSha256&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

class UTranformXPointer : public UBaseTransform {
public:

   // COSTRUTTORI

   UTranformXPointer() : dataList(5)
      {
      U_TRACE_REGISTER_OBJECT(0, UTranformXPointer, "")
      }

   virtual ~UTranformXPointer();

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage() { return _usage; }  // the allowed transforms usages
   virtual const char* name()  { return _name;  }  // the transform's name
   virtual const char* href()  { return _href;  }  // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

   static UXML2Document* document;

   UString tag;
   UVector<UXPathData*> dataList;

   bool setExpr(const char* expr, int nodeSetType, xmlNodePtr node);

private:
   UTranformXPointer(const UTranformXPointer&) : UBaseTransform() {}
   UTranformXPointer& operator=(const UTranformXPointer&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

/*
 * Input URI transform
 */

class UIOCallback;

class UTranformInputURI : public UBaseTransform {
public:

   // COSTRUTTORI

   explicit UTranformInputURI(const char* uri);

   virtual ~UTranformInputURI()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTranformInputURI)
      }

   // define method VIRTUAL of class UBaseTransform

   virtual int         usage()  { return _usage; } // the allowed transforms usages
   virtual const char* name()   { return _name;  } // the transform's name
   virtual const char* href()   { return _href;  } // the transform's identification string (href)

   virtual bool execute(UString& data);

#ifdef DEBUG
   const char* dump(bool reset) const { return UBaseTransform::dump(reset); }
#endif

protected:
   void* clbksCtx;
   UIOCallback* clbks;

   static int _usage;        // the allowed transforms usages
   static const char* _name; // the transform's name
   static const char* _href; // the transform's identification string (href)

   static UVector<UIOCallback*>* allIOCallbacks;

   // SERVICES

   static UIOCallback* find(const char* uri);

private:
   UTranformInputURI(const UTranformInputURI&) : UBaseTransform() {}
   UTranformInputURI& operator=(const UTranformInputURI&)         { return *this; }

   friend class UDSIGContext;
   friend class UTransformCtx;
   friend class UReferenceCtx;
};

/*
 * Input I/O callback list
 */

class UIOCallback {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

   UIOCallback(xmlInputMatchCallback matchFunc,
               xmlInputOpenCallback  openFunc,
               xmlInputReadCallback  readFunc,
               xmlInputCloseCallback closeFunc)
      {
      U_TRACE_REGISTER_OBJECT(0, UIOCallback, "%p,%p,%p,%p", matchFunc, openFunc, readFunc, closeFunc)

      matchcallback = matchFunc;
      opencallback  = openFunc;
      readcallback  = readFunc;
      closecallback = closeFunc;
      }

   ~UIOCallback()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UIOCallback)
      }

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

protected:
   xmlInputMatchCallback matchcallback;
   xmlInputOpenCallback  opencallback;
   xmlInputReadCallback  readcallback;
   xmlInputCloseCallback closecallback;

private:
   UIOCallback& operator=(const UIOCallback&) { return *this; }

   friend class UTranformInputURI;
};

#endif
