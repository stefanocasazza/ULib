// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    services.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/date.h>
#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/base64.h>
#include <ulib/utility/hexdump.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>

unsigned char UServices::key[16];

/* coverity[+alloc] */
int UServices::getDevNull(const char* file)
{
   U_TRACE(0, "UServices::getDevNull(%S)", file)

#ifdef DEBUG
          int fd_stderr = UFile::creat(file, O_WRONLY | O_APPEND, PERM_FILE);
#else
   static int fd_stderr;

   if (fd_stderr == 0) fd_stderr = UFile::open("/dev/null", O_WRONLY, PERM_FILE); // return open(/dev/null)
#endif

   U_RETURN(fd_stderr);
}

/* UID handling: are we setuid-root...? */

bool UServices::isSetuidRoot()
{
   U_TRACE(1, "UServices::isSetuidRoot()")

   if (u_user_name_len == 0) u_init_ulib_username();

   U_INTERNAL_DUMP("u_user_name(%u) = %.*S", u_user_name_len, u_user_name_len, u_user_name)

   bool i_am_root = (u_user_name_len == 4 && (uid_t) U_SYSCALL_NO_PARAM(getuid) == 0);

   if (i_am_root ||
       ((uid_t) U_SYSCALL_NO_PARAM(geteuid) == 0 ||
        (uid_t) U_SYSCALL_NO_PARAM(getegid) == 0))
      {
      U_RETURN(true); /* we are setuid-root */
      }

   U_RETURN(false);
}

/* close stdin and stdout, as they are not needed */

void UServices::closeStdInputOutput()
{
   U_TRACE(1, "UServices::closeStdInputOutput()")

   int fd = UFile::open("/dev/null", O_RDONLY, 0);

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

   // move stdin to /dev/null

   UFile::close(STDIN_FILENO);

#ifndef HAVE_DUP3
   (void) U_SYSCALL(dup2, "%d,%d",    fd, STDIN_FILENO);
#else
   (void) U_SYSCALL(dup3, "%d,%d,%d", fd, STDIN_FILENO, O_CLOEXEC);
#endif

   // move stdout to /dev/null

   UFile::close(STDOUT_FILENO);

#ifndef HAVE_DUP3
   (void) U_SYSCALL(dup2, "%d,%d",    fd, STDOUT_FILENO);
#else
   (void) U_SYSCALL(dup3, "%d,%d,%d", fd, STDOUT_FILENO, O_CLOEXEC);
#endif

#ifdef U_COVERITY_FALSE_POSITIVE
   if (fd != -1)
#endif
   UFile::close(fd);
}

/**
 * I/O - read while not received almost count data
 *
 * timeoutMS specified the timeout value, in milliseconds.
 *           A negative value indicates no timeout, i.e. an infinite wait.
 */

bool UServices::read(int fd, UString& buffer, uint32_t count, int timeoutMS)
{
   U_TRACE(0, "UServices::read(%d,%.*S,%u,%d)", fd, U_STRING_TO_TRACE(buffer), count, timeoutMS)

   ssize_t value;
   int byte_read = 0;
   bool result = true;
   uint32_t start  = buffer.size(), // NB: buffer read can start with previous data...
            ncount = buffer.space(),
            chunk  = start;

   if (chunk < U_CAPACITY) chunk = U_CAPACITY;

   if (ncount < chunk)
      {
      UString::_reserve(buffer, chunk);

      ncount = buffer.space();
      }

   char* ptr = buffer.c_pointer(start);

read:
   value = UNotifier::read(fd, ptr + byte_read, ncount, timeoutMS);

   if (value <= 0)
      {
      if (byte_read &&
          value != 0)
         {
         // NB: we have failed to read more bytes...

         U_INTERNAL_ASSERT(buffer.invariant())

         U_RETURN(true);
         }

      result = false;

      goto done;
      }

   byte_read += value;

   U_INTERNAL_DUMP("byte_read = %d", byte_read)

   if (byte_read < (int)count)
      {
      U_INTERNAL_ASSERT_DIFFERS(count, U_SINGLE_READ)

      ncount -= value;

      goto read;
      }

   if (value == (ssize_t)ncount)
      {
#  ifdef DEBUG
      U_MESSAGE("UServices::read(%u) ran out of buffer space(%u)", count, ncount);
#  endif

      buffer.size_adjust_force(start + byte_read); // NB: we force because string can be referenced...

      // NB: may be there are available more bytes to read...

      UString::_reserve(buffer, ncount * 2);
      
      ptr = buffer.c_pointer(start);

      ncount = buffer.space();

      timeoutMS = 500; // wait max for half second...

      goto read;
      }

done:
   buffer.size_adjust_force(start + byte_read); // NB: we force because string can be referenced...

   U_RETURN(result);
}

int UServices::askToLDAP(UString* pinput, UHashMap<UString>* ptable, const char* fmt, va_list argp)
{
   U_TRACE(0, "UServices::::askToLDAP(%p,%p,%S)", pinput, ptable, fmt)

   static int _fd_stderr;

   UString output, buffer(U_CAPACITY);

   buffer.vsnprintf(fmt, argp);

   UCommand cmd(buffer);

   if (_fd_stderr == 0) _fd_stderr = getDevNull("/tmp/askToLDAP.err");

   bool result = cmd.execute(pinput, &output, -1, _fd_stderr);

#ifndef U_LOG_DISABLE
   UServer_Base::logCommandMsgError(cmd.getCommand(), false);
#endif

   if (pinput == 0) ptable->clear();

   if (result)
      {
      if (output &&
          pinput == 0)
         {
         (void) UFileConfig::loadProperties(*ptable, output.data(), output.end());
         }

      U_RETURN(1);
      }

   if (UCommand::exit_value == 255) U_RETURN(-1); // Can't contact LDAP server (-1)

   U_RETURN(0);
}

uint64_t UServices::getUniqUID()
{
   U_TRACE(0, "UServices::getUniqUID()")

   static uint64_t unique_num;

   if (unique_num == 0) unique_num = (uint64_t)u_now->tv_usec;

   uint64_t _uid = (((uint64_t)u_pid) << 56)                               |
                  ((((uint64_t)u_now->tv_sec) & (0xfffffULL << 20)) << 16) |
                  (++unique_num & 0xfffffffffULL);

   U_RETURN(_uid);
}

#ifdef USE_LIBUUID
// creat a new unique UUID value - 16 bytes (128 bits) long
// return from the binary representation a 36-byte string (plus tailing '\0') of the form 1b4e28ba-2fa1-11d2-883f-0016d3cca427

uuid_t UServices::uuid; // typedef unsigned char uuid_t[16];

UString UServices::getUUID()
{
   U_TRACE(1, "UServices::getUUID()")

   UString id(37U);

   U_SYSCALL_VOID(uuid_generate, "%p", uuid);
   U_SYSCALL_VOID(uuid_unparse,  "%p", uuid, id.data());

   id.size_adjust(36U);

   U_RETURN_STRING(id);
}
#endif

#ifdef USE_LIBSSL
#  include <openssl/err.h>
#  include <openssl/engine.h>
#  include <ulib/base/ssl/dgst.h>
#  ifdef DEBUG
#     include <ulib/ssl/certificate.h>
#  endif

#  if !defined(HAVE_OPENSSL_97) && !defined(HAVE_OPENSSL_98)
#     warning "WARNING: I must to disable some function with this version of openssl... be aware"

#     define ENGINE_load_dynamic()            getpid()
#     define X509_STORE_set_flags(a,b)        getpid()
#     define ENGINE_load_public_key(a,b,c,d)  getpid()
#     define ENGINE_load_private_key(a,b,c,d) getpid()

#     ifdef DEBUG
#        define trace_sysreturn_type(a) trace_sysreturn_type(0)
#     endif
#  endif

int         UServices::verify_depth;
int         UServices::verify_error;
X509*       UServices::verify_current_cert;
UString*    UServices::CApath;
X509_STORE* UServices::store;

void UServices::setOpenSSLError()
{
   U_TRACE(0, "UServices::setOpenSSLError()")

   long i;
   uint32_t sz;

   while ((i = ERR_get_error()))
      {
      char buf[1024];

      (void) ERR_error_string_n(i, buf, sizeof(buf));

      sz = u__strlen(buf, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("buf = %.*S", sz, buf)

      u_buffer_len += u__snprintf(u_buffer + u_buffer_len, U_BUFFER_SIZE - u_buffer_len, " (%ld, %.*s)", i, sz, buf);
      }
}

// setup OPENSSL standard certificate directory

void UServices::setCApath(const char* _CApath)
{
   U_TRACE(0, "UServices::setCApath(%S)", _CApath)

   U_INTERNAL_ASSERT(_CApath && *_CApath)

   if (CApath == 0) U_NEW_ULIB_OBJECT(CApath, UString);

   *CApath = UFile::getRealPath(_CApath);
}

void UServices::setVerifyStatus(long result)
{
   U_TRACE(0, "UServices::setVerifyStatus(%ld)", result)

   const char* descr = 0;

   switch (result)
      {
      case X509_V_OK:                                       descr = "X509_V_OK";                                     break;
      case X509_V_ERR_OUT_OF_MEM:                           descr = "X509_V_ERR_OUT_OF_MEM";                         break;
      case X509_V_ERR_INVALID_CA:                           descr = "X509_V_ERR_INVALID_CA";                         break;
      case X509_V_ERR_CERT_REVOKED:                         descr = "X509_V_ERR_CERT_REVOKED";                       break;
      case X509_V_ERR_CERT_UNTRUSTED:                       descr = "X509_V_ERR_CERT_UNTRUSTED";                     break;
      case X509_V_ERR_CERT_REJECTED:                        descr = "X509_V_ERR_CERT_REJECTED";                      break;
      case X509_V_ERR_INVALID_PURPOSE:                      descr = "X509_V_ERR_INVALID_PURPOSE";                    break;
      case X509_V_ERR_CRL_HAS_EXPIRED:                      descr = "X509_V_ERR_CRL_HAS_EXPIRED";                    break;
      case X509_V_ERR_CERT_HAS_EXPIRED:                     descr = "X509_V_ERR_CERT_HAS_EXPIRED";                   break;
      case X509_V_ERR_UNABLE_TO_GET_CRL:                    descr = "X509_V_ERR_UNABLE_TO_GET_CRL";                  break;
      case X509_V_ERR_CRL_NOT_YET_VALID:                    descr = "X509_V_ERR_CRL_NOT_YET_VALID";                  break;
      case X509_V_ERR_CERT_NOT_YET_VALID:                   descr = "X509_V_ERR_CERT_NOT_YET_VALID";                 break;
      case X509_V_ERR_CERT_CHAIN_TOO_LONG:                  descr = "X509_V_ERR_CERT_CHAIN_TOO_LONG";                break;
      case X509_V_ERR_PATH_LENGTH_EXCEEDED:                 descr = "X509_V_ERR_PATH_LENGTH_EXCEEDED";               break;
      case X509_V_ERR_CRL_SIGNATURE_FAILURE:                descr = "X509_V_ERR_CRL_SIGNATURE_FAILURE";              break;
      case X509_V_ERR_CERT_SIGNATURE_FAILURE:               descr = "X509_V_ERR_CERT_SIGNATURE_FAILURE";             break;
      case X509_V_ERR_APPLICATION_VERIFICATION:             descr = "X509_V_ERR_APPLICATION_VERIFICATION";           break;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:            descr = "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT";          break;
      case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:            descr = "X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN";          break;
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:          descr = "X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT";        break;
      case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:        descr = "X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD";      break;
      case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:       descr = "X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD";     break;
      case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:       descr = "X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD";     break;
      case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:       descr = "X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD";     break;
      case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:      descr = "X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE";    break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:      descr = "X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE";    break;
      case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:     descr = "X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE";   break;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:    descr = "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY";  break;
      case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:   descr = "X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY"; break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, "(%ld, %s) - %s", result, descr, X509_verify_cert_error_string(result));
}

int UServices::X509Callback(int ok, X509_STORE_CTX* ctx)
{
   U_TRACE(0, "UServices::X509Callback(%d,%p)", ok, ctx)

   verify_error        =         U_SYSCALL(X509_STORE_CTX_get_error,        "%p", ctx),
   verify_depth        =         U_SYSCALL(X509_STORE_CTX_get_error_depth,  "%p", ctx);
   verify_current_cert = (X509*) U_SYSCALL(X509_STORE_CTX_get_current_cert, "%p", ctx);

#ifdef DEBUG
   setVerifyStatus(verify_error);

   U_INTERNAL_DUMP("verify_error = %d verify_depth = %d status = %.*S", verify_error, verify_depth, u_buffer_len, u_buffer)

   u_buffer_len = 0;

   if (verify_current_cert)
      {
      UString fname_cert   = UCertificate::getFileName(verify_current_cert),
              issuer_cert  = UCertificate::getIssuer(verify_current_cert),
              subject_cert = UCertificate::getSubject(verify_current_cert);

      U_INTERNAL_DUMP("fname_cert   = %.*S", U_STRING_TO_TRACE(fname_cert))
      U_INTERNAL_DUMP("issuer_cert  = %.*S", U_STRING_TO_TRACE(issuer_cert))
      U_INTERNAL_DUMP("subject_cert = %.*S", U_STRING_TO_TRACE(subject_cert))
      }
#endif

   /*
   if (ok == 0)
      {
      // We put "ok = 1;" in any case that we don't consider to be an error. In that case, it will return OK
      // for the certificate check as long as there are no other critical errors. Don't forget that there can
      // be multiple errors

      switch (ctx->error)
         {
         case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT: ok = 1; break;

         case X509_V_ERR_OUT_OF_MEM:
         case X509_V_ERR_INVALID_CA:
         case X509_V_ERR_CERT_REVOKED:
         case X509_V_ERR_CERT_UNTRUSTED:
         case X509_V_ERR_CERT_REJECTED:
         case X509_V_ERR_INVALID_PURPOSE:
         case X509_V_ERR_CRL_HAS_EXPIRED:
         case X509_V_ERR_CERT_HAS_EXPIRED:
         case X509_V_ERR_UNABLE_TO_GET_CRL:
         case X509_V_ERR_CRL_NOT_YET_VALID:
         case X509_V_ERR_CERT_NOT_YET_VALID:
         case X509_V_ERR_CERT_CHAIN_TOO_LONG:
         case X509_V_ERR_PATH_LENGTH_EXCEEDED:
         case X509_V_ERR_CRL_SIGNATURE_FAILURE:
         case X509_V_ERR_CERT_SIGNATURE_FAILURE:
         case X509_V_ERR_APPLICATION_VERIFICATION:
         case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
         case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
         case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
         case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
         case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
         case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
         case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
         case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
         case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
         case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
         case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
         break;
         }
      }
   */

   U_RETURN(ok);
}

bool UServices::setupOpenSSLStore(const char* CAfile, const char* _CApath, int store_flags)
{
   U_TRACE(1, "UServices::setupOpenSSLStore(%S,%S,%d)", CAfile, _CApath, store_flags)

   U_INTERNAL_ASSERT((CAfile && *CAfile) || (_CApath && *_CApath))

   if (_CApath)
      {
      setCApath(_CApath);

      U_INTERNAL_ASSERT(CApath->isNullTerminated())

      _CApath = CApath->data();
      }

   if (store) U_SYSCALL_VOID(X509_STORE_free, "%p", store);

   store = (X509_STORE*) U_SYSCALL_NO_PARAM(X509_STORE_new);

   if (U_SYSCALL(X509_STORE_set_default_paths, "%p", store) == 0 || // add a lookup file/method to an X509 store
       U_SYSCALL(X509_STORE_load_locations, "%p,%S,%S", store, CAfile, _CApath) == 0)
      {
      U_RETURN(false);
      }

   if (store_flags) U_SYSCALL_VOID(X509_STORE_set_flags, "%p,%d", store, store_flags); // X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL

#ifdef DEBUG
   setVerifyCallback(UServices::X509Callback);
#endif

   U_RETURN(true);
}

ENGINE* UServices::loadEngine(const char* id, unsigned int flags)
{
   U_TRACE(1, "UServices::loadEngine(%S,%u)", id, flags)

   U_SYSCALL_VOID_NO_PARAM(ENGINE_load_dynamic);

   ENGINE* e = (ENGINE*) U_SYSCALL(ENGINE_by_id, "%S", id);

   if (e &&
       (U_SYSCALL(ENGINE_init, "%p", e)                  == 0 ||
        U_SYSCALL(ENGINE_set_default, "%p,%u", e, flags) == 0))
      {
      (void) U_SYSCALL(ENGINE_free, "%p", e);

      e = 0;
      }

   U_RETURN_POINTER(e,ENGINE);
}

void UServices::releaseEngine(ENGINE* e, bool bkey)
{
   U_TRACE(1, "UServices::releaseEngine(%p,%b)", e, bkey)

   U_INTERNAL_ASSERT_POINTER(e)

   (void) U_SYSCALL(ENGINE_finish, "%p", e);
   (void) U_SYSCALL(ENGINE_free,   "%p", e);

   if (bkey &&
       u_pkey)
      {
      U_SYSCALL_VOID(EVP_PKEY_free, "%p", u_pkey);

      u_pkey = 0;
      }
}

EVP_PKEY* UServices::loadKey(const UString& x, const char* format, bool _private, const char* password, ENGINE* e)
{
   U_TRACE(0, "UServices::loadKey(%.*S,%S,%b,%S,%p)", U_STRING_TO_TRACE(x), format, _private, password, e)

   BIO* in;
   UString tmp = x;
   EVP_PKEY* pkey = 0;

   if (e)
      {
      const char* filename = x.c_str();
   // PW_CB_DATA cb_data   = { password, filename };

      pkey = (EVP_PKEY*) (_private ? U_SYSCALL(ENGINE_load_private_key, "%p,%S,%p,%p", e, filename, 0, 0)   // &cb_data
                                   : U_SYSCALL(ENGINE_load_public_key,  "%p,%S,%p,%p", e, filename, 0, 0)); // &cb_data

      goto done;
      }

   if (format == 0) format = (x.isBinary() ? "DER" : "PEM");

   if (strncmp(format, U_CONSTANT_TO_PARAM("PEM")) == 0 &&
       strncmp(x.data(), U_CONSTANT_TO_PARAM("-----BEGIN RSA PRIVATE KEY-----")) != 0)
      {
      unsigned length = x.size();

      UString buffer(length);

      if (UBase64::decode(x.data(), length, buffer) == false) goto next;

      tmp    = buffer;
      format = "DER";
      }

next:
   in = (BIO*) U_SYSCALL(BIO_new_mem_buf, "%p,%d", U_STRING_TO_PARAM(tmp));

   pkey = (EVP_PKEY*) (strncmp(format, U_CONSTANT_TO_PARAM("PEM")) == 0
                        ? (_private ? U_SYSCALL(PEM_read_bio_PrivateKey, "%p,%p,%p,%p", in, 0, (password ? u_passwd_cb : 0), (void*)password)
                                    : U_SYSCALL(PEM_read_bio_PUBKEY,     "%p,%p,%p,%p", in, 0, (password ? u_passwd_cb : 0), (void*)password))
                        : (_private ? U_SYSCALL(d2i_PrivateKey_bio,      "%p,%p",       in, 0)
                                    : U_SYSCALL(d2i_PUBKEY_bio,          "%p,%p",       in, 0)));

   (void) U_SYSCALL(BIO_free, "%p", in);

done:
   U_RETURN_POINTER(pkey,EVP_PKEY);
}

/*
data   is the data to be signed
pkey   is the corresponding private key
passwd is the corresponding password for the private key
*/

UString UServices::getSignatureValue(int alg, const UString& data, const UString& pkey, const UString& passwd, int base64, ENGINE* e)
{
   U_TRACE(0,"UServices::getSignatureValue(%d,%.*S,%.*S,%.*S,%d,%p)",alg,U_STRING_TO_TRACE(data),U_STRING_TO_TRACE(pkey),U_STRING_TO_TRACE(passwd),base64,e)

   u_dgst_sign_init(alg, 0);

   u_dgst_sign_hash((unsigned char*)U_STRING_TO_PARAM(data));

   if (pkey)
      {
      u_pkey = loadKey(pkey, 0, true, passwd.c_str(), e);

      U_INTERNAL_ASSERT_POINTER(u_pkey)
      }

   UString output(U_CAPACITY);

   if (base64 == -2)
      {
      if (u_dgst_sign_finish(0, 0) > 0) output.setConstant((const char*)u_mdValue, u_mdLen);
      }
   else
      {
      if (u_dgst_sign_finish((unsigned char*)output.data(), base64) > 0) output.size_adjust(u_mdLen);
      }

   U_INTERNAL_DUMP("u_mdLen = %d output = %.*S", u_mdLen, U_STRING_TO_TRACE(output))

   if (pkey &&
       u_pkey)
      {
      U_SYSCALL_VOID(EVP_PKEY_free, "%p", u_pkey);

      u_pkey = 0;
      }

   U_RETURN_STRING(output);
}

bool UServices::verifySignature(int alg, const UString& data, const UString& signature, const UString& pkey, ENGINE* e)
{
   U_TRACE(0, "UServices::verifySignature(%d,%.*S,%.*S,%.*S,%p)", alg, U_STRING_TO_TRACE(data), U_STRING_TO_TRACE(signature), U_STRING_TO_TRACE(pkey), e)

   u_dgst_verify_init(alg, e);

   u_dgst_verify_hash((unsigned char*)U_STRING_TO_PARAM(data));

   if (pkey)
      {
      u_pkey = loadKey(pkey, 0, false, 0, e);

      U_INTERNAL_ASSERT_POINTER(u_pkey)
      }

   int result = u_dgst_verify_finish((unsigned char*)U_STRING_TO_PARAM(signature));

   if (pkey &&
       u_pkey)
      {
      U_SYSCALL_VOID(EVP_PKEY_free, "%p", u_pkey);

      u_pkey = 0;
      }

   if (result == 1) U_RETURN(true);

   U_RETURN(false);
}

void UServices::generateDigest(int alg, unsigned char* data, uint32_t size)
{
   U_TRACE(0, "UServices::generateDigest(%d,%.*S,%u)", alg, size, data, size)

   u_dgst_init(alg, 0, 0);

   u_dgst_hash(data, size);

   (void) u_dgst_finish(0, 0);

   U_INTERNAL_DUMP("u_mdLen = %d", u_mdLen)
}
#endif // USE_LIBSSL

void UServices::generateKey(unsigned char* pkey, unsigned char* hexdump)
{
   U_TRACE(1, "UServices::generateKey(%p,%p)", pkey, hexdump)

   if (pkey == 0) pkey = key;

#ifdef USE_LIBUUID
   U_SYSCALL_VOID(uuid_generate, "%p", pkey);
#else
   *(uint64_t*) pkey                     = getUniqUID();
   *(uint64_t*)(pkey + sizeof(uint64_t)) = getUniqUID();
#endif

   if (hexdump) (void) u_hexdump_encode(pkey, 16, hexdump);
}

void UServices::generateDigest(int alg, uint32_t keylen, unsigned char* data, uint32_t size, UString& output, int base64)
{
   U_TRACE(0, "UServices::generateDigest(%d,%u,%.*S,%u,%.*S,%d)", alg, keylen, size, data, size, U_STRING_TO_TRACE(output), base64)

#ifdef USE_LIBSSL
   u_dgst_init(alg, (const char*)key, keylen);

   u_dgst_hash(data, size);

   if (base64 == -2)
      {
      (void) u_dgst_finish(0, 0);

      output.setConstant((const char*)u_mdValue, u_mdLen);
      }
   else
      {
      uint32_t bytes_written = u_dgst_finish((unsigned char*)output.end(), base64);

      output.size_adjust(output.size() + bytes_written);
      }

   U_INTERNAL_DUMP("u_mdLen = %d output = %.*S", u_mdLen, U_STRING_TO_TRACE(output))
#endif
}

#define U_HMAC_SIZE  16U                           // MD5 output len
#define U_TOKEN_SIZE (1U + 10U + 1U + U_HMAC_SIZE) // ... '&' time '&' hmac

UString UServices::generateToken(const UString& data, time_t expire)
{
   U_TRACE(0, "UServices::generateToken(%.*S,%ld)", U_STRING_TO_TRACE(data), expire)

   UString token(data.size() + U_TOKEN_SIZE);

   token.snprintf("%.*s&%010ld&", U_STRING_TO_TRACE(data), expire); // NB: expire time must be of size 10...

   U_INTERNAL_DUMP("token = %.*S", U_STRING_TO_TRACE(token))

   // HMAC-MD5(data&expire&)

   generateDigest(U_HASH_MD5, 16, (unsigned char*)U_STRING_TO_PARAM(token), token, -1);

   UString output(token.size() * 2);

   UHexDump::encode(token, output);

   U_RETURN_STRING(output);
}

bool UServices::getTokenData(UString& data, const UString& value, time_t& expire)
{
   U_TRACE(0, "UServices::getTokenData(%.*S,%.*S,%p)", U_STRING_TO_TRACE(data), U_STRING_TO_TRACE(value), &expire)

   uint32_t sz = value.size();

   if (sz > U_TOKEN_SIZE)
      {
      UString token(sz);

      UHexDump::decode(value, token);

      U_INTERNAL_DUMP("token = %.*S", U_STRING_TO_TRACE(token))

      U_ASSERT_MAJOR(data.capacity(), token.size() - U_TOKEN_SIZE)

      // token parsing (data '&' time '&' hmac)

      expire = 0;

      const char* ptr = token.data();

#  if SIZEOF_TIME_T == SIZEOF_LONG
      int scanned = U_SYSCALL(sscanf, "%p,%S,%p,%p", ptr, "%[^&]&%ld&",  data.data(), &expire);
#  else
      int scanned = U_SYSCALL(sscanf, "%p,%S,%p,%p", ptr, "%[^&]&%lld&", data.data(), &expire);
#  endif

      U_INTERNAL_DUMP("scanned = %d", scanned)

      if (scanned == 2)
         {
         UString hmac;

         data.size_adjust();

         U_INTERNAL_DUMP("data = %.*S u_now = %ld expire = %T", U_STRING_TO_TRACE(data), u_now->tv_sec, expire)

         U_ASSERT_EQUALS(token.size(), data.size() + U_TOKEN_SIZE)

         sz = data.size() + 1U + 10U + 1U;

         generateDigest(U_HASH_MD5, 16, (unsigned char*)ptr, sz, hmac, -2);

         if (hmac.equal(ptr + sz, U_HMAC_SIZE))
            {
            U_INTERNAL_DUMP("(u_now < expire) = %b", (u_now->tv_sec < expire))

            if (expire == 0 || u_now->tv_sec < expire) U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

#ifdef USE_LIBSSL
#  if !defined(HAVE_OPENSSL_97) && !defined(HAVE_OPENSSL_98)
#     ifdef DEBUG
#        undef trace_sysreturn_type
#     endif
#  endif
#endif
