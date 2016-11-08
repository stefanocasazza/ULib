// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mime_pkcs7.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/ssl/mime/mime_pkcs7.h>

UMimePKCS7::UMimePKCS7(const UString& _data)
{
   U_TRACE_REGISTER_OBJECT(0, UMimePKCS7, "%V", _data.rep)

   // OpenSSL_add_all_algorithms(); // called in ULib::init()

   BIO* indata; // indata is the signed data if the content is not present in pkcs7 (that is it is detached)

   BIO* in = (BIO*) BIO_new_mem_buf(U_STRING_TO_PARAM(_data));

   // SMIME reader: handle multipart/signed and opaque signing. In multipart case the content is placed
   // in a memory BIO pointed to by "indata". In opaque this is set to NULL

   PKCS7* p7 = (PKCS7*) SMIME_read_PKCS7(in, &indata);

   (void) BIO_free(in);

   pkcs7.set(p7, indata);

   content = pkcs7.getContent(&valid_content);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, const UMimePKCS7& mp7)
{
   U_TRACE(0, "UMimePKCS7::operator<<(%p,%p)", &os, &mp7)

   os.write(mp7.content.data(), mp7.content.size());

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UMimePKCS7::dump(bool reset) const
{
   UMimeEntity::dump(false);

   *UObjectIO::os << "\n"
               << "valid_content             " << valid_content << '\n'
               << "pkcs7             (UPKCS7 " << (void*)&pkcs7 << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
