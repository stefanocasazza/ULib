/* ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cdes3.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#include <ulib/base/utility.h>
#include <ulib/base/ssl/des3.h>

#include <openssl/evp.h>
#include <openssl/des.h>
#include <openssl/rand.h>

#if defined(__NetBSD__) || defined(__UNIKERNEL__)
#  include <des.h>
#endif

#define U_ENCRYPT   1
#define U_DECRYPT   0
#define U_STR_MAGIC "Salted__"

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#  define des_cblock         DES_cblock
#  define des_key_schedule   DES_key_schedule
#  define des_string_to_key  DES_string_to_key
#  define RAND_pseudo_bytes  RAND_bytes
#  define des_set_odd_parity DES_set_odd_parity
#endif

static const char* password;

static const EVP_MD* md;
static const EVP_CIPHER* cipher;
static unsigned char salt[PKCS5_SALT_LEN];
static unsigned char iv[EVP_MAX_IV_LENGTH];
static unsigned char xkey[EVP_MAX_KEY_LENGTH];

static int inp_num, out_num;
static des_cblock key, inp_ivec, out_ivec;
static des_key_schedule inp1_sched, inp2_sched, inp3_sched, out1_sched, out2_sched, out3_sched;

void u_des_init(void)
{
   U_INTERNAL_TRACE("u_des_init()")

   if (password) des_string_to_key(password, &key);
   else          des_set_odd_parity(&key);

   u__memcpy(&inp_ivec, &key, sizeof(inp_ivec), __PRETTY_FUNCTION__);
   u__memcpy(&out_ivec, &key, sizeof(out_ivec), __PRETTY_FUNCTION__);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   (void) des_set_key(&inp_ivec, inp1_sched);
   (void) des_set_key(&inp_ivec, inp2_sched);
   (void) des_set_key(&inp_ivec, inp3_sched);
   (void) des_set_key(&out_ivec, out1_sched);
   (void) des_set_key(&out_ivec, out2_sched);
   (void) des_set_key(&out_ivec, out3_sched);
#else
   (void) DES_set_key(&inp_ivec, &inp1_sched);
   (void) DES_set_key(&inp_ivec, &inp2_sched);
   (void) DES_set_key(&inp_ivec, &inp3_sched);
   (void) DES_set_key(&out_ivec, &out1_sched);
   (void) DES_set_key(&out_ivec, &out2_sched);
   (void) DES_set_key(&out_ivec, &out3_sched);
#endif
}

void u_des3_init(void)
{
   U_INTERNAL_TRACE("u_des3_init()")

/* OpenSSL_add_all_algorithms(); // called in ULib::init() */

   md     = EVP_md5();
   cipher = EVP_des_ede3_cbc(); /* EVP_get_cipherbyname("SN_des_ede3_cbc"); // des3 */

   U_INTERNAL_ASSERT_POINTER(md)
   U_INTERNAL_ASSERT_POINTER(cipher)
}

void u_des_reset(void)
{
   U_INTERNAL_TRACE("u_des_reset()")

   inp_num = out_num = 0;

   (void) memset(&key,      0, sizeof(des_cblock));
   (void) memset(&inp_ivec, 0, sizeof(des_cblock));
   (void) memset(&out_ivec, 0, sizeof(des_cblock));

   (void) memset(&inp1_sched, 0, sizeof(des_key_schedule));
   (void) memset(&inp2_sched, 0, sizeof(des_key_schedule));
   (void) memset(&inp3_sched, 0, sizeof(des_key_schedule));
   (void) memset(&out1_sched, 0, sizeof(des_key_schedule));
   (void) memset(&out2_sched, 0, sizeof(des_key_schedule));
   (void) memset(&out3_sched, 0, sizeof(des_key_schedule));

   u_des3_init();
}

void u_des3_reset(void)
{
   U_INTERNAL_TRACE("u_des3_reset()")

   u_des3_init();
}

void u_des_key(const char* restrict str)
{
   U_INTERNAL_TRACE("u_des_key(%s)", str)

   password = str;

   if (inp_num || out_num) u_des3_reset();
   else

   u_des3_init();
}

void u_des3_key(const char* restrict str)
{
   U_INTERNAL_TRACE("u_des3_key(%s)", str)

   password = str;

   u_des3_init();
}

long u_des_encode(const unsigned char* restrict inp, long len, unsigned char* restrict out)
{
   U_INTERNAL_TRACE("u_des_encode(%.*s,%ld,%p)", U_min(len,128), inp, len, out)

   U_INTERNAL_PRINT("inp_num = %d", inp_num)

   /**
    * The input and output encrypted as though 64bit cfb mode is being used.
    * The extra state information to record how much of the 64bit block we have used is contained in inp_num
    */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   des_ede3_cfb64_encrypt(inp, out, len,  out1_sched,  out2_sched,  out3_sched, &out_ivec, &inp_num, DES_ENCRYPT);
#else
   DES_ede3_cfb64_encrypt(inp, out, len, &out1_sched, &out2_sched, &out3_sched, &out_ivec, &inp_num, DES_ENCRYPT);
#endif

   U_INTERNAL_PRINT("inp_num = %d", inp_num)

   return len;
}

long u_des3_encode(const unsigned char* restrict inp, long len, unsigned char* restrict out)
{
   BIO* wbio;
   BIO* benc;

   U_INTERNAL_TRACE("u_des3_encode(%.*s,%ld,%p)", U_min(len,128), inp, len, out)

   RAND_pseudo_bytes(salt, sizeof(salt));

   U_INTERNAL_PRINT("cipher = %p, md = %p", cipher, md)

   (void) EVP_BytesToKey(cipher, md, salt, (unsigned char*)password, u__strlen(password, __PRETTY_FUNCTION__), 1, xkey, iv);

   wbio = BIO_new(BIO_s_mem());
   benc = BIO_new(BIO_f_cipher());

   BIO_set_cipher(benc, cipher, xkey, iv, U_ENCRYPT);

   BIO_write(wbio, U_CONSTANT_TO_PARAM(U_STR_MAGIC));
   BIO_write(wbio, (char*)salt, sizeof(salt));

   benc = BIO_push(benc, wbio);

   BIO_write(benc, (char*)inp, len);

   (void) BIO_flush(benc);

   len = BIO_read(wbio, (char*)out, len + 64);

   BIO_free(wbio);
   BIO_free(benc);

   return len;
}

long u_des_decode(const unsigned char* restrict inp, long len, unsigned char* restrict out)
{
   U_INTERNAL_TRACE("u_des_decode(%.*s,%ld,%p)", U_min(len,128), inp, len, out)

   U_INTERNAL_PRINT("out_num = %d", out_num)

   /**
    * The input and output encrypted as though 64bit cfb mode is being used.
    * The extra state information to record how much of the 64bit block we have used is contained in out_num
    */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   des_ede3_cfb64_encrypt(inp, out, len,  inp1_sched,  inp2_sched,  inp3_sched, &inp_ivec, &out_num, DES_DECRYPT);
#else
   DES_ede3_cfb64_encrypt(inp, out, len, &inp1_sched, &inp2_sched, &inp3_sched, &inp_ivec, &out_num, DES_DECRYPT);
#endif

   U_INTERNAL_PRINT("out_num = %d", out_num)

   return len;
}

long u_des3_decode(const unsigned char* restrict inp, long len, unsigned char* restrict out)
{
   BIO* rbio;
   BIO* wbio;
   BIO* benc;
   char mbuf[sizeof(U_STR_MAGIC)-1];
   unsigned int magic_len = sizeof(U_STR_MAGIC)-1;

   U_INTERNAL_TRACE("u_des3_decode(%.*s,%ld,%p)", U_min(len,128), inp, len, out)

   rbio = BIO_new(BIO_s_mem());
   wbio = BIO_new(BIO_s_mem());
   benc = BIO_new(BIO_f_cipher());

   BIO_write(rbio, (char*)inp, len);

   BIO_read(rbio, mbuf, sizeof(mbuf));

   if (strncmp(mbuf, U_STR_MAGIC, magic_len)) return 0;

   BIO_read(rbio, (unsigned char*)salt, sizeof(salt));

   (void) EVP_BytesToKey(cipher, md, salt, (unsigned char*)password, u__strlen(password, __PRETTY_FUNCTION__), 1, xkey, iv);

   BIO_set_cipher(benc, cipher, xkey, iv, U_DECRYPT);

   benc = BIO_push(benc, wbio);

   inp += magic_len + sizeof(salt);
   len -= magic_len + sizeof(salt);

   BIO_write(benc, (char*)inp, len);

   (void) BIO_flush(benc);

   len = BIO_read(wbio, (char*)out, len + 64);

   BIO_free(rbio);
   BIO_free(wbio);
   BIO_free(benc);

   return (len > 0 ? len : 0);
}
