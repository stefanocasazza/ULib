// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mime_pkcs7.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MIME_PKCS7_H
#define U_MIME_PKCS7_H 1

#include <ulib/ssl/pkcs7.h>
#include <ulib/mime/entity.h>

class U_EXPORT UMimePKCS7 : public UMimeEntity {
public:

   UMimePKCS7(const UString& data);

   UMimePKCS7(UMimeEntity& item) : UMimeEntity(item), pkcs7(content, "DER")
      {
      U_TRACE_REGISTER_OBJECT(0, UMimePKCS7, "%p", &item)

      U_ASSERT(UMimeEntity::isPKCS7())

      valid_content = false;

      if (pkcs7.isValid()) content = pkcs7.getContent(&valid_content);
      }

   ~UMimePKCS7()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMimePKCS7)
      }

   UPKCS7& getPKCS7() { return pkcs7; }

   bool isValid() const
      {
      U_TRACE_NO_PARAM(0, "UMimePKCS7::isValid()")

      bool result = (pkcs7.isValid() && valid_content);

      U_RETURN(result);
      }

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UMimePKCS7& mp7);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UPKCS7 pkcs7;
   bool valid_content;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UMimePKCS7)
};

#endif
