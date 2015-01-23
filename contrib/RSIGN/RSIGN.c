/* RSIGN.c: Handle openssl engine */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
#define OPENSSL_NO_ERR
*/
#define ENGINE_DYNAMIC_SUPPORT

#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/engine.h>
#include <openssl/objects.h>

/*
#define FULL_DEBUG
*/

#ifdef DEBUG
#include <openssl/bio.h>

#  if OPENSSL_VERSION_NUMBER >= 0x0090800fL
#     define OPENSSL_d2i_TYPE const unsigned char**
#  else
#     define OPENSSL_d2i_TYPE unsigned char**
#  endif
#endif

/*****************************************************************************
 * Constants used when creating the ENGINE
 **/
static const char* engine_RSIGN_id   = "RSIGN";
static const char* engine_RSIGN_name = "RSIGN engine support";

#if defined(FULL_DEBUG) || defined(DEBUG)
static BIO* err;
static const char* routine;
#endif

#ifdef OPENSSL_NO_ERR
#  define RSIGN_R_UNKNOWN_FAULT           101
#  define RSIGN_R_COMMAND_NOT_IMPLEMENTED 102
#  define RSIGN_ERROR_INVALID_PARAMETER   103
#  define RSIGN_ERROR_NOT_ENOUGH_MEMORY   104
#  define RSIGN_R_UNKNOWN_PADDING_TYPE    105
#else
#  include "./RSIGN.err"
#endif

/*****************************************************************************
 *** Function declarations and global variable definitions                 ***
 *****************************************************************************/

/*****************************************************************************
 * Functions to handle the engine
 **/
static int RSIGN_destroy(ENGINE* e);
static int RSIGN_init(ENGINE* e);
static int RSIGN_finish(ENGINE* e);

/* static variables */

/*****************************************************************************
 * RSA functions
 **/

#ifndef OPENSSL_NO_RSA

static int RSIGN_rsa_sign(int flen, const unsigned char* from, unsigned char* to, RSA* rsa, int padding);

/*****************************************************************************
 * Our RSA method
 **/
static RSA_METHOD RSIGN_rsa = {
   "RSIGN RSA method",
   NULL,                /* rsa_pub_encrypt */
   NULL,                /* rsa_pub_decrypt */
   RSIGN_rsa_sign,      /* our RSIGN_rsa_sign is OpenSSL rsa_priv_encrypt */
   NULL,                /* rsa_priv_decrypt */
   NULL,                /* rsa_mod_exp */
   NULL,                /* mod_exp_mont */
   NULL,                /* init */
   NULL,                /* finish */
   0,                   /* flags */
   NULL,                /* app_data */
   NULL,                /* rsa_sign */
   NULL                 /* rsa_verify */
};

#endif

/*****************************************************************************
 * Symetric cipher and digest function registrars
 **/

/*****************************************************************************
 * DES functions
 **/

/*****************************************************************************
 * Our DES ciphers
 **/

/*****************************************************************************
 * MD functions
 **/

/*****************************************************************************
 * Our MD digests
 **/

/*****************************************************************************
 *** Function definitions                                                  ***
 *****************************************************************************/

/*****************************************************************************
 * Functions to handle the engine
 **/

static int bind_RSIGN(ENGINE* e)
{
   if (!ENGINE_set_id(e, engine_RSIGN_id)
    || !ENGINE_set_name(e, engine_RSIGN_name)
    || !ENGINE_set_RSA(e, &RSIGN_rsa)
    || !ENGINE_set_destroy_function(e, RSIGN_destroy)
    || !ENGINE_set_init_function(e, RSIGN_init)
    || !ENGINE_set_finish_function(e, RSIGN_finish)
   ) return 0;

   /* Ensure the rsaref error handling is set up */

#ifndef OPENSSL_NO_ERR
   ERR_load_RSIGN_strings();
#endif

   return 1;
}

#ifdef ENGINE_DYNAMIC_SUPPORT

static int bind_helper(ENGINE* e, const char* id)
{
   if (id && (strcmp(id, engine_RSIGN_id) != 0)) return 0;

   if (!bind_RSIGN(e)) return 0;

   return 1;
}       

IMPLEMENT_DYNAMIC_CHECK_FN()
IMPLEMENT_DYNAMIC_BIND_FN(bind_helper)

#else

static ENGINE* engine_RSIGN(void)
{
   ENGINE* ret = ENGINE_new();

   if (!ret) return NULL;

   if (!bind_RSIGN(ret))
      {
      ENGINE_free(ret);

      return NULL;
      }

   return ret;
}

void ENGINE_load_RSIGN(void)
{
   ENGINE* toadd = engine_RSIGN(); /* Copied from eng_[openssl|dyn].c */

   if (!toadd) return;

   ENGINE_add(toadd);

   ENGINE_free(toadd);

#ifndef OPENSSL_NO_ERR
   ERR_clear_error();
#endif
}

#endif

/* Initiator which is only present to make sure this engine looks available */

static int RSIGN_init(ENGINE* e)
{
#if defined(FULL_DEBUG) || defined(DEBUG)
   err = BIO_new_fp(stderr, BIO_NOCLOSE);
#endif

#ifdef FULL_DEBUG
   BIO_printf(err, "Call RSIGN_init(%p)\n", e);
   ERR_print_errors(err);
#endif

   return 1;
}

/* Finisher which is only present to make sure this engine looks available */

static int RSIGN_finish(ENGINE* e)
{
#ifdef FULL_DEBUG
   BIO_printf(err, "Call RSIGN_finish(%p)\n", e);
   ERR_print_errors(err);
#endif

   return 1;
}

/* Destructor (complements the "ENGINE_ncipher()" constructor) */

static int RSIGN_destroy(ENGINE* e)
{
#ifdef FULL_DEBUG
   BIO_printf(err, "Call RSIGN_destroy(%p)\n", e);
   ERR_print_errors(err);
#endif

#ifndef OPENSSL_NO_ERR
   ERR_unload_RSIGN_strings();
#endif

   return 1;
}

/*******************************************************************************
 * Information function
 *******************************************************************************/

static void writeData(const void* data, unsigned size, const char* name)
{
   FILE* hOutputFile;

   hOutputFile = fopen(name, "wb");

   fwrite(data, size, 1, hOutputFile);

   fclose(hOutputFile);
}

/*****************************************************************************
 * RSA functions
 **/

/*
* Does what OpenSSL rsa_priv_enc does.
*/

static int RSIGN_rsa_sign(int flen, const unsigned char* from, unsigned char* to, RSA* rsa, int padding)
{
   int byte_read = 0, error;

#ifdef FULL_DEBUG
   BIO_printf(err, "Call RSIGN_rsa_sign(%d)\n", flen);

   routine = "RSIGN_rsa_sign";
#endif

   switch (padding)
      {
      case RSA_PKCS1_PADDING: /* do it in one shot */
         {
         FILE* fp;
         char* tmp;
         long keyLength;
         char* cmd = getenv("RSIGN_CMD");

         if (!cmd || !*cmd)
            {
            error = RSIGN_R_COMMAND_NOT_IMPLEMENTED;  

            goto error;
            }

         tmp = strrchr(cmd, ' ');

         if (!tmp++)
            {
            error = RSIGN_R_COMMAND_NOT_IMPLEMENTED;  

            goto error;
            }

         keyLength = RSA_size(rsa);

#     ifdef FULL_DEBUG
         BIO_printf(err, "cmd = \"%s\"\n", cmd);
         BIO_printf(err, "tmp = \"%s\"\n", tmp);
         BIO_printf(err, "keyLength = %ld\n", keyLength);
#     endif

         writeData((const void*)from, flen, tmp);

         (void) fflush(NULL);

         fp = popen(cmd, "r");

         if (fp)
            {
            ssize_t value;
            size_t start = 0;

            while (1)
               {
               value = read(fileno(fp), to + start, 8192);

               if (value <= 0) break;

               start     += value;
               byte_read += value;
               }

            (void) pclose(fp);
            }

         unlink(tmp);

         goto end;
         }
      break;

      case RSA_NO_PADDING:
      default:
         error = RSIGN_R_UNKNOWN_PADDING_TYPE;  
      break;
      }

error:

   byte_read = 0;

#ifndef OPENSSL_NO_ERR
   RSIGN_err(RSIGN_F_RSA_SIGN, error);
#endif

#if defined(FULL_DEBUG) || defined(DEBUG)
   ERR_print_errors(err);
#endif

end:

#ifdef FULL_DEBUG
   BIO_printf(err, "Return RSIGN_rsa_sign(%d)\n", byte_read);
#endif

   return byte_read;
}

/*****************************************************************************
 * Symetric cipher and digest function registrers
 **/

/*****************************************************************************
 * DES functions
 **/

/*****************************************************************************
 * MD functions
 **/
