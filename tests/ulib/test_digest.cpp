// test_digest.cpp

#include <ulib/ssl/digest.h>

static const char* test[] = {
   "",
   "a",
   "abc",
   "message digest",
   "abcdefghijklmnopqrstuvwxyz",
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
   "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
   0,
};

static const char* ret[] = {
   "d41d8cd98f00b204e9800998ecf8427e",
   "0cc175b9c0f1b6a831c399e269772661",
   "900150983cd24fb0d6963f7d28e17f72",
   "f96b697d7cb7938d525a2f31aaf161d0",
   "c3fcd3d76192e4007dfb496cca67e13b",
   "d174ab98d277d9f5a5611c2c9f419d9f",
   "57edf4a22be3c955ac49da2e2107b67a",
};

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   int i;
   unsigned char* md;
   UString buffer(MD5_DIGEST_LENGTH * 2);

   buffer.size_adjust(MD5_DIGEST_LENGTH * 2);

   char* buf      = buffer.data();
   const char** P = test;
   const char** R = ret;

   while (*P)
      {
      UString data(*P), output = UDigest::md5(data);

      md = (unsigned char*)output.data();

      for (i = 0; i < MD5_DIGEST_LENGTH; ++i) (void)sprintf(&(buf[i*2]), "%02x", md[i]);

      cout << *P << " -> " << buffer << endl;

      U_ASSERT( buffer == *R )

      ++R;
      ++P;
      }
}
