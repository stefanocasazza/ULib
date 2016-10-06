// DocumentClassifier.cpp

#include "DocumentClassifier.h"

#include <ulib/file.h>
#include <ulib/zip/zip.h>
#include <ulib/ssl/crl.h>
#include <ulib/ssl/pkcs10.h>
#include <ulib/ssl/certificate.h>
#include <ulib/utility/services.h>
#include <ulib/ssl/mime/mime_pkcs7.h>

#ifdef HAVE_SSL_TS
#  include <ulib/ssl/timestamp.h>
#endif

// EXTENSION

bool                       DocumentClassifier::verify_result;
time_t                     DocumentClassifier::certsVerificationTime;
UString*                   DocumentClassifier::label_ko;
UCertificate*              DocumentClassifier::ca;
TreeCertificate*           DocumentClassifier::ptcert;
UVector<TreeCertificate*>* DocumentClassifier::vtcert;

void DocumentClassifier::addCertificate(UCertificate* cert)
{
   U_TRACE(5, "DocumentClassifier::addCertificate(%p)", cert)

   cert->duplicate();

   if (vtcert == 0) U_NEW(UVector<TreeCertificate*>, vtcert, UVector<TreeCertificate*>);

   if (ca == 0)
      {
      UCertificate* ca_root;
      unsigned num_tree = vtcert->size();

      for (unsigned i = 0; i < num_tree; ++i)
         {
         ptcert  = vtcert->at(i);
         ca_root = ptcert->elem();

         if (*ca_root == *cert) goto done;
         }

      U_NEW(TreeCertificate, ptcert, TreeCertificate(cert));

      vtcert->push(ptcert);
      }
   else
      {
      U_ASSERT(cert->isIssued(*ca))

      UCertificate* ca_node;
      TreeCertificate* pnode;
      unsigned numChild = ptcert->numChild();

      for (unsigned i = 0; i < numChild; ++i)
         {
         pnode   = ptcert->childAt(i);
         ca_node = pnode->elem();

         if (*ca_node == *cert)
            {
            ptcert = pnode;

            goto done;
            }
         }

      ptcert = ptcert->push(cert);
      }

done:
   ca = cert;
}

int DocumentClassifier::verifyCallback(int ok, X509_STORE_CTX* ctx) // callback
{
   U_TRACE(5, "DocumentClassifier::verifyCallback(%d,%p)", ok, ctx)

   static bool flag_ricorsione;

   if (flag_ricorsione || verify_result == false) U_RETURN(ok);

   if (ok == 0) U_RETURN(1);

   flag_ricorsione = true;

   // start time verification

   (void) UServices::X509Callback(ok, ctx);

   UCertificate* cert;

   U_NEW(UCertificate, cert, UCertificate(UServices::verify_current_cert));

   ok = cert->verify(0, certsVerificationTime);

   if (ok) addCertificate(cert);
   else
      {
      verify_result = false;

      if (label_ko == 0) U_NEW(UString, label_ko, UString(4000U));

      // TO DO

      delete cert;
      }

   // end time verification

   flag_ricorsione = false;

   U_RETURN(ok);
}

void DocumentClassifier::printCertificate(UTree<UCertificate*>* pnode)
{
   U_TRACE(5, "DocumentClassifier::printCertificate(%p)", pnode)

   UCertificate* cert = pnode->elem();

   U_INTERNAL_ASSERT_POINTER(cert)

   UString crl;
   bool crl_exist, cert_exist;
   long hash = cert->hashCode();

   crl      = UCertificate::getFileName(hash, true,  &crl_exist);
   filename = UCertificate::getFileName(hash, false, &cert_exist);
// filename = cert->getFileName();

   U_INTERNAL_DUMP("cert_exist = %b crl_exist = %b", cert_exist, crl_exist)

   U_INTERNAL_ASSERT(crl)
   U_INTERNAL_ASSERT(filename)

                  buffer += filename + '\n';
   if (crl_exist) buffer += crl      + '\n';

   unsigned numChild = pnode->numChild();

   for (unsigned i = 0; i < numChild; ++i) printCertificate(pnode->childAt(i));
}

UString DocumentClassifier::printCertificate()
{
   U_TRACE(5, "DocumentClassifier::printCertificate()")

   UString result;

   if (vtcert && verify_result)
      {
      buffer.setEmpty();

      uint32_t num_tree = vtcert->size();

      for (uint32_t i = 0; i < num_tree; ++i) printCertificate(vtcert->at(i));

      result = buffer;
      }

   U_RETURN_STRING(result);
}

void DocumentClassifier::printLabel(UTree<UCertificate*>* pnode)
{
   U_TRACE(5, "DocumentClassifier::printLabel(%p)", pnode)

   UCertificate* cert = pnode->elem();

   U_INTERNAL_ASSERT_POINTER(cert)

   bool crl_exist, cert_exist;
   long hash = cert->hashCode();
   UString crl, inner(4000U);

   crl      = UCertificate::getFileName(hash, true,  &crl_exist);
   filename = UCertificate::getFileName(hash, false, &cert_exist);
// filename = cert->getFileName();

   U_INTERNAL_DUMP("cert_exist = %b crl_exist = %b", cert_exist, crl_exist)

   U_INTERNAL_ASSERT(crl)
   U_INTERNAL_ASSERT(filename)

   if (cert_exist == false) (void) UFile::writeTo(filename, cert->getEncoded());

   UString issuer = cert->getIssuerForLDAP();

   inner.snprintf(U_CONSTANT_TO_PARAM("<certificate issuer=\"%.*s\" serial=\"%d\" filename=\"%s\">\n"),
                  U_STRING_TO_TRACE(issuer), cert->getSerialNumber(), u_basename(filename.data()));

   buffer += inner;

   if (crl_exist)
      {
      inner.snprintf(U_CONSTANT_TO_PARAM("<crl filename=\"%s\"/>\n"), u_basename(crl.data()));

      buffer += inner;
      }

   uint32_t numChild = pnode->numChild();

   for (uint32_t i = 0; i < numChild; ++i) printLabel(pnode->childAt(i));

   buffer += U_STRING_FROM_CONSTANT("</certificate>\n");
}

UString DocumentClassifier::printLabel()
{
   U_TRACE(5, "DocumentClassifier::printLabel()")

   UString result;

   if (vtcert && verify_result)
      {
      buffer.setEmpty();

      for (uint32_t i = 0, num_tree = vtcert->size(); i < num_tree; ++i) printLabel(vtcert->at(i));

      result = buffer;
      }

   U_RETURN_STRING(result);
}

DocumentClassifier::~DocumentClassifier()
{
   U_TRACE(5, "DocumentClassifier::~DocumentClassifier()")

   if (vtcert)
      {
      for (uint32_t i = 0, num_tree = vtcert->size(); i < num_tree; ++i)
         {
         TreeCertificate* pnode = vtcert->at(i);

         pnode->clear();
         }

      vtcert->clear();
      }
}

// NORMAL

Element*         DocumentClassifier::current;
UTree<Element*>* DocumentClassifier::ptree;

DocumentClassifier::DocumentClassifier(const char* pathfile) : description(4000U), buffer(4000U)
{
   U_TRACE(5, "DocumentClassifier::DocumentClassifier(%S)", pathfile)

   binary  = false;
   content = UFile::contentOf(UString(pathfile));

   if (content.empty())
      {
      type = Element::EMPTY;

      return;
      }

   // EXTENSION

   verify_result = true;

   UServices::setVerifyCallback(DocumentClassifier::verifyCallback);

   // NORMAL

   type  = elementaryTypeOf();
   ptree = &tree;

   U_INTERNAL_DUMP("ptree = %p", ptree)

   if (type == Element::UNKNOWN)
      {
      if (binary == false) buildTree();
      }
   else if (type == Element::PKCS7)
      {
      addElement(Element::PKCS7);

      if (binary == false) buildTree();
      }
}

Element::~Element()
{
   U_TRACE_UNREGISTER_OBJECT(5, Element)

   if (node_to_delete)
      {
      U_INTERNAL_ASSERT(type == TBOTE ||
                        type == T_TOKEN)

      UTree<Element*>* node = node_to_delete->pop();

      if (type == T_TOKEN)
         {
         UTree<Element*>* ts  = node->pop();
         UTree<Element*>* zip = node->pop();
         UTree<Element*>* xml = node->pop();

         delete xml;

         delete ts->pop();
         delete ts;

         (void) zip->pop();
         delete zip;

         delete node;
         }

      delete node_to_delete;
      }
}

void DocumentClassifier::classifyEntry(void* e, void* pnode)
{
   U_TRACE(5, "DocumentClassifier::classifyEntry(%p,%p)", e, pnode)

   Element::Type type    = ((Element*)e)->type;
   UTree<Element*>* node = (UTree<Element*>*)pnode;

   U_DUMP("e->description = %S depth() = %u", ((Element*)e)->description.data(), node->depth())

   if (type == Element::RFC822)
      {
      node = node->childAt(0);
      type = node->elem()->type;
      }

   if (type == Element::SMIME &&
       node->numChild() == 1)
      {
      UTree<Element*>* child = node->childAt(0);

      // CHECK FOR TBOTE OR TARCHIVE TOKEN

      if (child->elem()->type == Element::MULTIPART)
         {
         bool tarchive = (child->numChild() == 3                                                            &&
                          child->childAt(0)->elem()->content_type     == U_STRING_FROM_CONSTANT("text/xml") &&
                          child->childAt(1)->numChild()               == 1                                  &&
                          child->childAt(1)->childAt(0)->elem()->type == Element::ZIP                       &&
                          child->childAt(2)->numChild()               == 1                                  &&
                          child->childAt(2)->childAt(0)->elem()->type == Element::TIMESTAMP_RESPONSE);

         UTree<Element*>* parent = node->parent();

         if (tarchive) child = child->childAt(1)->childAt(0); // Zip node

         U_INTERNAL_DUMP("tarchive = %b child = %p", tarchive, child)

         child->setParent(parent);

         if (parent == 0) ptree = child;
         else
            {
            parent->replace(0, child);

            child->elem()->node_to_delete = node;
            }

         child->elem()->type = (tarchive ? Element::T_TOKEN
                                         : Element::TBOTE);
         }
      }
}

int DocumentClassifier::elementaryTypeOf()
{
   U_TRACE(5, "DocumentClassifier::elementaryTypeOf()")

   U_INTERNAL_ASSERT(content)

   binary = content.isBinary();

   UPKCS7 p7(content, (binary ? "DER" : "PEM"));

   if (p7.isValid())
      {
#  ifdef HAVE_SSL_TS
      if (UTimeStamp::isTimeStampToken(p7.getPKCS7())) U_RETURN(Element::TIMESTAMP);
#  endif

      verify(p7); // EXTENSION

      content = p7.getContent();
      binary  = content.isBinary();

      U_RETURN(Element::PKCS7);
      }

   if (UCertificate(content).isValid())   U_RETURN(Element::X509);
   if (UPKCS10(content).isValid())        U_RETURN(Element::PKCS10);
   if (UCrl(content).isValid())           U_RETURN(Element::CRL);

   U_RETURN(Element::UNKNOWN);
}

void DocumentClassifier::setType(UTree<Element*>* pnode)
{
   U_TRACE(5, "DocumentClassifier::setType(%p)", pnode)

   if (pnode->elem()->type == Element::TBOTE)
      {
      type = Element::TBOTE;

      return;
      }

   if (pnode->elem()->type == Element::T_TOKEN) type = Element::T_TOKEN;

   uint32_t numChild = pnode->numChild();

   if (numChild)
      {
      for (uint32_t i = 0; (i < numChild) && (type != Element::TBOTE); ++i)
         {
         setType(pnode->childAt(i));
         }
      }
}

void DocumentClassifier::buildTree()
{
   U_TRACE(5, "DocumentClassifier::buildTree()")

   U_INTERNAL_ASSERT(content)
   U_INTERNAL_ASSERT(binary == false)

   parse();

   ptree = &tree;

   if (tree.empty() == false)
      {
      if (tree.numChild()) classify();

      type = ptree->elem()->type;

      setType(ptree);
      }

   U_INTERNAL_DUMP("ptree = %p", ptree)
}

void DocumentClassifier::parse()
{
   U_TRACE(5, "DocumentClassifier::parse()")

   U_INTERNAL_ASSERT(content)

   binary = content.isBinary();

   if (binary)
      {
      UZIP zip(content);

      if (zip.isValid() &&
          zip.readContent())
         {
         UString namefile;
         uint32_t count = zip.getFilesCount();

         description.snprintf(U_CONSTANT_TO_PARAM(" - %d parts"), count);

         addElement(Element::ZIP, description);

         for (uint32_t i = 0; i < count; ++i)
            {
            namefile = zip.getFilenameAt(i);

            description.snprintf(U_CONSTANT_TO_PARAM(" %d: Filename='%.*s'"), i+1, U_STRING_TO_TRACE(namefile));

            addElement(Element::PART, description);

            current->setFileName(namefile);

            content = zip.getFileContentAt(i);

            parse();

            up();
            }

         up();

         return;
         }
      }
   else if (content.isWhiteSpace() == false)
      {
      UMimeEntity entity(content);

      if (entity.isMime())
         {
         parseMime(entity);

         return;
         }
      }

   UPKCS7 p7(content, (binary ? "DER" : "PEM"));

   if (p7.isValid())
      {
      addElement(Element::PKCS7);

      verify(p7); // EXTENSION

      return;
      }
}

UString DocumentClassifier::manageDescription(UMimeEntity* uMimeEntity, uint32_t i)
{
   U_TRACE(5, "DocumentClassifier::manageDescription(%p,%u)", uMimeEntity, i)

   UString number(10U),
           stype = uMimeEntity->shortContentType();

   if (i) number.snprintf(U_CONSTANT_TO_PARAM(" %u"), i);

   if (uMimeEntity->isBodyMessage())
      {
      filename.clear();

      description.snprintf(U_CONSTANT_TO_PARAM("%.*s: Content-type='%.*s' - BODY MESSAGE"),
                           U_STRING_TO_TRACE(number), U_STRING_TO_TRACE(stype));
      }
   else
      {
      filename = uMimeEntity->getFileName();

      description.snprintf(U_CONSTANT_TO_PARAM("%.*s: Content-type='%.*s' - Filename='%.*s'"),
                           U_STRING_TO_TRACE(number), U_STRING_TO_TRACE(stype), U_STRING_TO_TRACE(filename));
      }

   U_RETURN_STRING(stype);
}

void DocumentClassifier::parseMime(UMimeEntity& entity)
{
   U_TRACE(5, "DocumentClassifier::parseMime(%p)", &entity)

   if (entity.isRFC822())
      {
      addElement(Element::RFC822);

      UMimeMessage tmp(entity);
      UMimeEntity& rfc822 = tmp.getRFC822();

      parseMime(rfc822);

      up();
      }
   else if (entity.isPKCS7())
      {
      addElement(Element::SMIME);

      UMimePKCS7 tmp(entity);

      verify(tmp.getPKCS7()); // EXTENSION

      content = tmp.getContent();

      parse();

      up();
      }
   else if (entity.isMultipart())
      {
      UString stype;
      UMimeEntity* uMimeEntity;
      UMimeMultipart tmp(entity);
      uint32_t i = 0, bodyPartCount = tmp.getNumBodyPart();

      description.snprintf(U_CONSTANT_TO_PARAM(" - %u parts"), bodyPartCount);

      addElement(Element::MULTIPART, description);

      while (i < bodyPartCount)
         {
         uMimeEntity = tmp[i];

         stype = manageDescription(uMimeEntity, ++i);

         addElement(Element::PART, description, stype);

         current->setFileName(filename);

         content = (uMimeEntity->isMultipart() ? uMimeEntity->getData() : uMimeEntity->getContent());

#     ifdef HAVE_SSL_TS
         if (stype == U_STRING_FROM_CONSTANT("application/timestamp-reply") && UTimeStamp::isTimeStampResponse(content))
#     else
         if (stype == U_STRING_FROM_CONSTANT("application/timestamp-reply"))
#     endif
            {
            addElement(Element::TIMESTAMP_RESPONSE);

            up();
            }
         else
            {
            parse();
            }

         up();
         }

      up();

      content.clear();
      }
   else
      {
      UString stype = manageDescription(&entity, 0);

      addElement(Element::MIME, description, stype);

      current->setFileName(filename);
      }
}

void DocumentClassifier::print(const UString& prefix, UTree<Element*>* pnode)
{
   U_TRACE(5, "DocumentClassifier::print(%.*S,%p)", U_STRING_TO_TRACE(prefix), pnode)

   Element* elem = pnode->elem();

   U_INTERNAL_ASSERT_POINTER(elem)

   buffer += prefix + ' ' + elem->whatIs() + elem->description + '\n';

   uint32_t numChild = pnode->numChild();

   if (numChild)
      {
      UString innerPrefix(100);

      for (uint32_t i = 0; i < numChild; ++i)
         {
         innerPrefix.snprintf(U_CONSTANT_TO_PARAM("%.*s.%u"), U_STRING_TO_TRACE(prefix), i + 1);

         print(innerPrefix, pnode->childAt(i));
         }
      }
}

UString DocumentClassifier::print()
{
   U_TRACE(5, "DocumentClassifier::print()")

   UString result;

   switch (type)
      {
      case Element::CRL:       result = U_STRING_FROM_CONSTANT("1 CRL\n");                      break;
      case Element::PKCS10:    result = U_STRING_FROM_CONSTANT("1 PKCS10\n");                   break;
      case Element::X509:      result = U_STRING_FROM_CONSTANT("1 CERTIFICATE\n");              break;
      case Element::TIMESTAMP: result = U_STRING_FROM_CONSTANT("1 TIMESTAMP\n");                break;
      case Element::PKCS7:     result = U_STRING_FROM_CONSTANT("1 PKCS7\n");                    break;
      case Element::SMIME:     result = U_STRING_FROM_CONSTANT("1 MIME: smime signed\n");       break;
      case Element::UNKNOWN:   result = U_STRING_FROM_CONSTANT("UNKNOWN OBJECT\n");             break;
      case Element::EMPTY:     result = U_STRING_FROM_CONSTANT("EMPTY OR UNEXISTANT FILE\n");   break;

      default:
         {
         buffer.setEmpty();

         print(U_STRING_FROM_CONSTANT("1"), ptree);

         result = buffer;
         }
      break;
      }

   U_RETURN_STRING(result);
}

UString Element::whatIs()
{
   U_TRACE(5, "Element::whatIs()")

   UString result;

   switch (type)
      {
      case CRL:                result = U_STRING_FROM_CONSTANT("CRL");                 break;
      case X509:               result = U_STRING_FROM_CONSTANT("X509");                break;
      case PKCS7:              result = U_STRING_FROM_CONSTANT("PKCS7");               break;
      case PKCS10:             result = U_STRING_FROM_CONSTANT("PKCS10");              break;
      case TIMESTAMP:          result = U_STRING_FROM_CONSTANT("TIMESTAMP");           break;
      case TIMESTAMP_RESPONSE: result = U_STRING_FROM_CONSTANT("TIMESTAMP RESPONSE");  break;

      case ZIP:                result = U_STRING_FROM_CONSTANT("ZIP");                 break;
      case PART:               result = U_STRING_FROM_CONSTANT("Part");                break;

      case MIME:               result = U_STRING_FROM_CONSTANT("MIME");                break;
      case RFC822:             result = U_STRING_FROM_CONSTANT("MIME: rfc822");        break;
      case SMIME:              result = U_STRING_FROM_CONSTANT("MIME: smime signed");  break;
      case MULTIPART:          result = U_STRING_FROM_CONSTANT("MIME: multipart");     break;

      case TBOTE:              result = U_STRING_FROM_CONSTANT("TBOTE: signed");       break;
      case T_TOKEN:            result = U_STRING_FROM_CONSTANT("TARCHIVE TOKEN");      break;

      case EMPTY:              result = U_STRING_FROM_CONSTANT("EMPTY");               break;
      case UNKNOWN:
      default:                 result = U_STRING_FROM_CONSTANT("UNKNOWN");
      }

   U_RETURN_STRING(result);
}

UString DocumentClassifier::whatIs()
{
   U_TRACE(5, "DocumentClassifier::whatIs()")

   UString result;

   switch (type)
      {
      case Element::CRL:       result = U_STRING_FROM_CONSTANT("CRL");                   break;
      case Element::PKCS10:    result = U_STRING_FROM_CONSTANT("PKCS10");                break;
      case Element::TIMESTAMP: result = U_STRING_FROM_CONSTANT("TIMESTAMP");             break;
      case Element::X509:      result = U_STRING_FROM_CONSTANT("CERTIFICATE");           break;

      case Element::SMIME:
      case Element::PKCS7:     result = U_STRING_FROM_CONSTANT("SIMPLE SIGNED OBJECT");  break;
      case Element::TBOTE:     result = U_STRING_FROM_CONSTANT("TBOTE SIGNED");          break;
      case Element::T_TOKEN:   result = U_STRING_FROM_CONSTANT("TARCHIVE TOKEN");        break;

      default:                 result = U_STRING_FROM_CONSTANT("UNKNOWN OBJECT");
      }

   U_RETURN_STRING(result);
}

// DEBUG

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>

const char* Element::dump(bool reset) const
{
   *UObjectIO::os << "type                  " << type                  << '\n'
                  << "node_to_delete        " << (void*)node_to_delete << '\n'
                  << "filename     (UString " << (void*)&filename      << ")\n"
                  << "description  (UString " << (void*)&description   << ")\n"
                  << "content_type (UString " << (void*)&content_type  << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#endif
