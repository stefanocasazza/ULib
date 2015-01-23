/* client.c */

#include "CSP.nsmap"

#include <unistd.h>     /* defines _POSIX_THREADS if pthreads are available */
#ifdef _POSIX_THREADS
# include <pthread.h>
#endif
#include <signal.h>     /* defines SIGPIPE */

void sigpipe_handle(int);
int  CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();

/*
int ns__CSP_LIST_CA(char** response);
int ns__CSP_CA(const char* dir, unsigned days, const char* cnf, char** response);
int ns__CSP_SIGN_P10(const char* ca, const char* policy, const char* pkcs10, char** response);
int ns__CSP_SIGN_SPACK(const char* ca, const char* policy, const char* spack, char** response);
int ns__CSP_LIST_CERTS(const char* ca, unsigned compress, char** response);
int ns__CSP_REMOVE_CERT(const char* ca, const char* serial, char** response);
int ns__CSP_ZERO_CERTS(const char* ca, char** response);
*/

int main(int argc, char** argv)
{
   struct soap soap;
   const char* url = argv[1];
   int method      = atoi(argv[2]);

   if (CRYPTO_thread_setup())
      {
      fprintf(stderr, "Cannot setup thread mutex\n");

      exit(1);
      }

   soap_init(&soap);

   /*
   The supplied server certificate "server.pem" assumes that the server is
   running on '10.30.1.131', so clients can only connect from the same host when
   verifying the server's certificate. Use SOAP_SSL_NO_AUTHENTICATION to omit
   the authentication of the server and use encryption directly from any site.
   To verify the certificates of third-party services, they must provide a
   certificate issued by Verisign or another trusted CA. At the client-side,
   the capath parameter should point to a directory that contains these
   trusted (root) certificates or the cafile parameter should refer to one
   file will all certificates. To help you out, the supplied "cacerts.pem"
   file contains the certificates issued by various CAs. You should use this
   file for the cafile parameter instead of "cacert.pem" to connect to trusted
   servers.  Note that the client may fail to connect if the server's
   credentials have problems (e.g. expired). Use SOAP_SSL_NO_AUTHENTICATION
   and set cacert to NULL to encrypt messages if you don't care about the
   trustworthyness of the server.  Note: setting capath may not work on Windows.
   */

   if (soap_ssl_client_context(&soap,
         SOAP_SSL_NO_AUTHENTICATION, /* use SOAP_SSL_DEFAULT in production code */
         NULL,                       /* keyfile: required only when client must authenticate to server */
         NULL,                       /* password to read the keyfile */
         "cacert.pem",               /* optional cacert file to store trusted certificates */
         NULL,                       /* optional capath to directory with trusted certificates */
         NULL))                      /* if randfile!=NULL: use a file with random data to seed randomness */ 
      {
      soap_print_fault(&soap, stderr);

      exit(1);
      }

   switch (method)
      {
      case 1:
         {
         struct _ns__CSP_CA data;
         struct _ns__CSP_CAResponse res;

         data.dir  = argv[3];
         data.days = atoi(argv[4]);
         data.cnf  = argv[5];

         if (soap_call___ns__CSP_CA(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_CA() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;

      case 2:
         {
         struct _ns__CSP_LIST_CA data;
         struct _ns__CSP_LIST_CAResponse res;

         if (soap_call___ns__CSP_LIST_CA(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_LIST_CA() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;

      case 3:
         {
         struct _ns__CSP_SIGN_P10 data;
         struct _ns__CSP_SIGN_P10Response res;

         data.ca     = argv[3];
         data.policy = argv[4];
         data.pkcs10 = argv[5];

         if (soap_call___ns__CSP_SIGN_P10(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_SIGN_P10() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;

      case 4:
         {
         struct _ns__CSP_SIGN_SPACK data;
         struct _ns__CSP_SIGN_SPACKResponse res;

         data.ca     = argv[3];
         data.policy = argv[4];
         data.spack  = argv[5];

         if (soap_call___ns__CSP_SIGN_SPACK(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_SIGN_SPACK() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;

      case 5:
         {
         struct _ns__CSP_LIST_CERTS data;
         struct _ns__CSP_LIST_CERTSResponse res;

         data.ca       = argv[3];
         data.compress = atoi(argv[4]);

         if (soap_call___ns__CSP_LIST_CERTS(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_LIST_CERTS() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;

      case 6:
         {
         struct _ns__CSP_REMOVE_CERT data;
         struct _ns__CSP_REMOVE_CERTResponse res;

         data.ca     = argv[3];
         data.serial = argv[4];

         if (soap_call___ns__CSP_REMOVE_CERT(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_REMOVE_CERT() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;

      case 7:
         {
         struct _ns__CSP_ZERO_CERTS data;
         struct _ns__CSP_ZERO_CERTSResponse res;

         data.ca = argv[3];

         if (soap_call___ns__CSP_ZERO_CERTS(&soap, url, NULL, &data, &res) == 0)
            printf("CSP_ZERO_CERTS() = %s\n", res.response);
         else
            soap_print_fault(&soap, stderr);
         }
      break;
      }

   soap_destroy(&soap); /* C++ */
   soap_end(&soap);
   soap_done(&soap);
   CRYPTO_thread_cleanup();

   return 0;
}

/******************************************************************************\
 *
 * OpenSSL
 *
\******************************************************************************/

#ifdef WITH_OPENSSL

#if defined(WIN32)
# define MUTEX_TYPE     HANDLE
# define MUTEX_SETUP(x)    (x) = CreateMutex(NULL, FALSE, NULL)
# define MUTEX_CLEANUP(x)  CloseHandle(x)
# define MUTEX_LOCK(x)     WaitForSingleObject((x), INFINITE)
# define MUTEX_UNLOCK(x)   ReleaseMutex(x)
# define THREAD_ID      GetCurrentThreadId()
#elif defined(_POSIX_THREADS)
# define MUTEX_TYPE     pthread_mutex_t
# define MUTEX_SETUP(x)    pthread_mutex_init(&(x), NULL)
# define MUTEX_CLEANUP(x)  pthread_mutex_destroy(&(x))
# define MUTEX_LOCK(x)     pthread_mutex_lock(&(x))
# define MUTEX_UNLOCK(x)   pthread_mutex_unlock(&(x))
# define THREAD_ID      pthread_self()
#else
# error "You must define mutex operations appropriate for your platform"
# error  "See OpenSSL /threads/th-lock.c on how to implement mutex on your platform"
#endif

struct CRYPTO_dynlock_value
{ MUTEX_TYPE mutex;
};

static MUTEX_TYPE *mutex_buf;

static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line)
{ struct CRYPTO_dynlock_value *value;
  value = (struct CRYPTO_dynlock_value*)malloc(sizeof(struct CRYPTO_dynlock_value));
  if (value)
    MUTEX_SETUP(value->mutex);
  return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(l->mutex);
  else
    MUTEX_UNLOCK(l->mutex);
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line)
{ MUTEX_CLEANUP(l->mutex);
  free(l);
}

void locking_function(int mode, int n, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(mutex_buf[n]);
  else
    MUTEX_UNLOCK(mutex_buf[n]);
}

unsigned long id_function()
{ return (unsigned long)THREAD_ID;
}

int CRYPTO_thread_setup()
{ int i;
  mutex_buf = (MUTEX_TYPE*)malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
  if (!mutex_buf)
    return SOAP_EOM;
  for (i = 0; i < CRYPTO_num_locks(); i++)
    MUTEX_SETUP(mutex_buf[i]);
  CRYPTO_set_id_callback(id_function);
  CRYPTO_set_locking_callback(locking_function);
  CRYPTO_set_dynlock_create_callback(dyn_create_function);
  CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
  CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
  return SOAP_OK;
}

void CRYPTO_thread_cleanup()
{ int i;
  if (!mutex_buf)
    return;
  CRYPTO_set_id_callback(NULL);
  CRYPTO_set_locking_callback(NULL);
  CRYPTO_set_dynlock_create_callback(NULL);
  CRYPTO_set_dynlock_lock_callback(NULL);
  CRYPTO_set_dynlock_destroy_callback(NULL);
  for (i = 0; i < CRYPTO_num_locks(); i++)
    MUTEX_CLEANUP(mutex_buf[i]);
  free(mutex_buf);
  mutex_buf = NULL;
}

#endif

/******************************************************************************\
 *
 * SIGPIPE
 *
\******************************************************************************/

void sigpipe_handle(int x) { }

