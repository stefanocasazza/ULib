// DocumentClassifier.h

#ifndef DOCUMENT_CLASSIFIER_H
#define DOCUMENT_CLASSIFIER_H

#include <ulib/ssl/pkcs7.h>
#include <ulib/container/tree.h>

class Element {
public:
   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum Type { CRL                = 0,
               X509               = 1,
               PKCS7              = 2,
               PKCS10             = 3,
               TIMESTAMP          = 4,
               TIMESTAMP_RESPONSE = 5,
               ZIP                = 6,
               PART               = 7,
               MIME               = 8,
               RFC822             = 9,
               SMIME              = 10,
               MULTIPART          = 11,
               TBOTE              = 12,
               T_TOKEN            = 13,
               EMPTY              = 14,
               UNKNOWN            = 15 };

   Type type;
   UTree<Element*>* node_to_delete;
   UString description, content_type, filename;

   explicit Element(Type t) : type(t)
      {
      U_TRACE_REGISTER_OBJECT(5, Element, "%d", t)

      node_to_delete = U_NULLPTR;
      }

   Element(Type t, UString& d) : type(t), description(d)
      {
      U_TRACE_REGISTER_OBJECT(5, Element, "%d,%.*S", t, U_STRING_TO_TRACE(d))

      node_to_delete = U_NULLPTR;

      description.duplicate();

   // d.clear();
      }

   Element(Type t, UString& d, UString& c) : type(t), description(d), content_type(c)
      {
      U_TRACE_REGISTER_OBJECT(5, Element, "%d,%.*S,%.*S", t, U_STRING_TO_TRACE(d), U_STRING_TO_TRACE(c))

      node_to_delete = U_NULLPTR;

       description.duplicate();
      content_type.duplicate();

   // d.clear();
      c.clear();
      }

   ~Element();

   // SERVICES

   UString whatIs();

   void setFileName(UString& namefile)
      {
      U_TRACE(5, "Element::setFileName(%.*S)", U_STRING_TO_TRACE(namefile))

      filename = namefile.copy();

      namefile.clear();
      }

#ifdef DEBUG
   friend ostream& operator<<(ostream& os, const Element& e)
      {
      os.put('{');
      os.put(' ');
      os << e.type;
      os.put(' ');
      os << e.content_type;
      os.put(' ');
      os << e.filename;
      os.put(' ');
      os << e.description;
      os.put('}');

      return os;
      }

   const char* dump(bool reset) const;
#endif
};

class UMimeEntity;
class UCertificate;

typedef UTree<UCertificate*> TreeCertificate;

class DocumentClassifier {
public:

   static time_t certsVerificationTime;

   // COSTRUTTORE

   explicit DocumentClassifier(const char* filename);
           ~DocumentClassifier();

   // SERVICES

   UString print();
   UString whatIs();
   UString printLabel();
   UString printCertificate();

protected:
   UTree<Element*> tree;
   UString description, buffer, filename, content;
   int type;
   bool binary;

   static Element* current;
   static UTree<Element*>* ptree;

   void    parse();
   void    buildTree();
   int     elementaryTypeOf();
   void    parseMime(UMimeEntity& entity);
   void    setType(UTree<Element*>* pnode);
   void    print(const UString& prefix, UTree<Element*>* pnode);
   UString manageDescription(UMimeEntity* uMimeEntity, uint32_t i = 0);

   void up()       { ptree = ptree->parent(); }
   void classify() { ptree = &tree; tree.callForAllEntry(DocumentClassifier::classifyEntry); }

   void addElement(Element::Type t)                         { ptree = ptree->push_back(current = new Element(t)); }
   void addElement(Element::Type t, UString& d)             { ptree = ptree->push_back(current = new Element(t,d)); }
   void addElement(Element::Type t, UString& d, UString& c) { ptree = ptree->push_back(current = new Element(t,d,c));}

   static void classifyEntry(void* e, void* pnode);

   // EXTENSION

   static UCertificate* ca;
   static UString* label_ko;                 // unverified
   static bool verify_result;
   static TreeCertificate* ptcert;
   static UVector<TreeCertificate*>* vtcert; // for label ok

   void printLabel(UTree<UCertificate*>* pnode);
   void printCertificate(UTree<UCertificate*>* pnode);

   static void addCertificate(UCertificate* cert);
   static int  verifyCallback(int ok, X509_STORE_CTX* ctx); // callback

   static void verify(const UPKCS7& snode)
      {
      U_TRACE(5, "DocumentClassifier::verify(%p)", &snode)

      U_INTERNAL_DUMP("verify_result = %b", verify_result)

      if (verify_result)
         {
         ca = U_NULLPTR;

         (void) snode.verify(0);
         }
      }
};

#endif
