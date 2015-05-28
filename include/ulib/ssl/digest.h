// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    digest.h - interface to MD5
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_DIGEST_H
#define U_DIGEST_H 1

#include <ulib/string.h>

#include <openssl/md5.h>

// MD5_DIGEST_LENGTH == 16

struct UDigest {

   static UString md5(const UString& data)
      {
      U_TRACE(1, "UDigest::md5(%V)", data.rep)

      MD5_CTX ctx;
      UString output(MD5_DIGEST_LENGTH);

      output.size_adjust(MD5_DIGEST_LENGTH);

#  ifdef HAVE_OPENSSL_97
      (void) U_SYSCALL(MD5_Init,   "%p",       &ctx);
      (void) U_SYSCALL(MD5_Update, "%p,%p,%d", &ctx, U_STRING_TO_PARAM(data));
      (void) U_SYSCALL(MD5_Final,  "%p,%p", (unsigned char*)output.data(), &ctx);
#  else
      U_SYSCALL_VOID(  MD5_Init,   "%p",       &ctx);
      U_SYSCALL_VOID(  MD5_Update, "%p,%p,%d", &ctx, U_STRING_TO_PARAM(data));
      U_SYSCALL_VOID(  MD5_Final,  "%p,%p", (unsigned char*)output.data(), &ctx);
#  endif

      U_RETURN_STRING(output);
      }

   static UString hmac(const UString& data, const UString& password)
      {
      U_TRACE(1, "UDigest::hmac(%V,%V)", data.rep, password.rep)

      U_INTERNAL_ASSERT_EQUALS(password.size(), MD5_DIGEST_LENGTH)

      MD5_CTX ctx;
      UString output(MD5_DIGEST_LENGTH);
      const void* ptr = password.data();

      output.size_adjust(MD5_DIGEST_LENGTH);

#  ifdef HAVE_OPENSSL_97
      (void) U_SYSCALL(MD5_Init,   "%p",       &ctx);
      (void) U_SYSCALL(MD5_Update, "%p,%p,%d", &ctx, ptr, MD5_DIGEST_LENGTH);
      (void) U_SYSCALL(MD5_Update, "%p,%p,%d", &ctx, U_STRING_TO_PARAM(data));
      (void) U_SYSCALL(MD5_Update, "%p,%p,%d", &ctx, ptr, MD5_DIGEST_LENGTH);
      (void) U_SYSCALL(MD5_Final,  "%p,%p", (unsigned char*)output.data(), &ctx);
#  else
      U_SYSCALL_VOID(  MD5_Init,   "%p",       &ctx);
      U_SYSCALL_VOID(  MD5_Update, "%p,%p,%d", &ctx, ptr, MD5_DIGEST_LENGTH);
      U_SYSCALL_VOID(  MD5_Update, "%p,%p,%d", &ctx, U_STRING_TO_PARAM(data));
      U_SYSCALL_VOID(  MD5_Update, "%p,%p,%d", &ctx, ptr, MD5_DIGEST_LENGTH);
      U_SYSCALL_VOID(  MD5_Final,  "%p,%p", (unsigned char*)output.data(), &ctx);
#  endif

      U_RETURN_STRING(output);
      }
};

#endif
