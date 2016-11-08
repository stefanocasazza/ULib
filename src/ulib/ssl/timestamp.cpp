// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    timestamp.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/ssl/timestamp.h>
#include <ulib/base/ssl/dgst.h>
#include <ulib/net/client/http.h>
#include <ulib/utility/services.h>
#include <ulib/ssl/net/sslsocket.h>
#include <ulib/utility/string_ext.h>

TS_RESP* UTimeStamp::readTimeStampResponse(const UString& x)
{
   U_TRACE(1, "UTimeStamp::readTimeStamp(%V)", x.rep)

   BIO* in = (BIO*) U_SYSCALL(BIO_new_mem_buf, "%p,%d", U_STRING_TO_PARAM(x));

   TS_RESP* _response = (TS_RESP*) U_SYSCALL(d2i_TS_RESP_bio, "%p,%p", in, 0);

   (void) U_SYSCALL(BIO_free, "%p", in);

   U_RETURN_POINTER(_response,TS_RESP);
}

UTimeStamp::UTimeStamp(UString& request, const UString& TSA) : UPKCS7(0,0)
{
   U_TRACE_REGISTER_OBJECT(0, UTimeStamp, "%V,%V", request.rep, TSA.rep)

   UHttpClient<USSLSocket> client(0);

   response = (client.sendPost(TSA, request, "application/timestamp-query")
                     ? readTimeStampResponse(client.getContent())
                     : 0);

   if (response) pkcs7 = (PKCS7*) TS_RESP_get_token(response);
}

bool UTimeStamp::isTimeStampToken(PKCS7* p7)
{
   U_TRACE(1, "UTimeStamp::isTimeStampToken(%p)", p7)

   TS_TST_INFO* info = (TS_TST_INFO*) U_SYSCALL(PKCS7_to_TS_TST_INFO, "%p", p7);

   if (info)
      {
      U_SYSCALL_VOID(TS_TST_INFO_free, "%p", info);

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UTimeStamp::isTimeStampResponse(const UString& content)
{
   U_TRACE(1, "UTimeStamp::isTimeStampResponse(%V)", content.rep)

   BIO* in = (BIO*) U_SYSCALL(BIO_new_mem_buf, "%p,%d", U_STRING_TO_PARAM(content));

   TS_RESP* _response = (TS_RESP*) U_SYSCALL(d2i_TS_RESP_bio, "%p,%p", in, 0);

   (void) U_SYSCALL(BIO_free, "%p", in);

   if (_response)
      {
      U_SYSCALL_VOID(TS_RESP_free, "%p", _response);

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString UTimeStamp::createQuery(int alg, const UString& content, const char* policy, bool bnonce, bool bcert)
{
   U_TRACE(1, "UTimeStamp::createQuery(%d,%V,%S,%b,%b)", alg, content.rep, policy, bnonce, bcert)

   UServices::generateDigest(alg, content);

   // Creating request object

   TS_REQ* ts_req = (TS_REQ*) U_SYSCALL_NO_PARAM(TS_REQ_new);

   // Setting version

   (void) U_SYSCALL(TS_REQ_set_version, "%p,%d", ts_req, 1);

   // Creating and adding MSG_IMPRINT object

   TS_MSG_IMPRINT* msg_imprint = (TS_MSG_IMPRINT*) U_SYSCALL_NO_PARAM(TS_MSG_IMPRINT_new);

   // Adding algorithm

   X509_ALGOR* algo = (X509_ALGOR*) U_SYSCALL_NO_PARAM(X509_ALGOR_new);

   algo->algorithm       = (ASN1_OBJECT*) U_SYSCALL(OBJ_nid2obj, "%d", EVP_MD_type(u_md));
   algo->parameter       = (ASN1_TYPE*)   U_SYSCALL_NO_PARAM(ASN1_TYPE_new);
   algo->parameter->type = V_ASN1_NULL;

   (void) U_SYSCALL(TS_MSG_IMPRINT_set_algo, "%p,%p", msg_imprint, algo);

   // Adding message digest

   (void) U_SYSCALL(TS_MSG_IMPRINT_set_msg, "%p,%p,%u", msg_imprint, u_mdValue, u_mdLen);
   (void) U_SYSCALL(TS_REQ_set_msg_imprint, "%p,%p", ts_req, msg_imprint);

   // Setting policy if requested

   ASN1_OBJECT* policy_obj = 0;

   if (policy)
      {
      policy_obj = (ASN1_OBJECT*) U_SYSCALL(OBJ_txt2obj, "%S,%d", policy, 0);

      (void) U_SYSCALL(TS_REQ_set_policy_id, "%p,%p", ts_req, policy_obj);
      }

   // Setting nonce if requested

   if (bnonce)
      {
      static ASN1_INTEGER* nonce;

      if (nonce == 0)
         {
         static unsigned char buffer[8]; // Length of the nonce of the request in bits (must be a multiple of 8)

         nonce = (ASN1_INTEGER*) U_SYSCALL_NO_PARAM(ASN1_INTEGER_new);

         nonce->data = buffer;
         }

      nonce->length = u_num2str32(u_now->tv_sec, (char*)nonce->data) - (char*)nonce->data;

      (void) U_SYSCALL(TS_REQ_set_nonce, "%p,%p", ts_req, nonce);
      }

   // Setting certificate request flag if requested

   if (bcert) (void) U_SYSCALL(TS_REQ_set_cert_req, "%p,%d", ts_req, 1);

   // ASN.1 output

   BIO* bio = (BIO*) U_SYSCALL(BIO_new, "%p", BIO_s_mem());

   int res = U_SYSCALL(i2d_TS_REQ_bio, "%p,%p", bio, ts_req);

   U_SYSCALL_VOID(TS_REQ_free,         "%p", ts_req);
   U_SYSCALL_VOID(TS_MSG_IMPRINT_free, "%p", msg_imprint);
   U_SYSCALL_VOID(X509_ALGOR_free,     "%p", algo);

   if (policy_obj) U_SYSCALL_VOID(ASN1_OBJECT_free, "%p", policy_obj);

   if (res == -1) (void) U_SYSCALL(BIO_free, "%p", bio);
   else
      {
      UString req = UStringExt::BIOtoString(bio);

      U_RETURN_STRING(req);
      }

   return UString::getStringNull();
}

UString UTimeStamp::getTimeStampToken(int alg, const UString& data, const UString& url)
{
   U_TRACE(0, "UTimeStamp::getTimeStampToken(%d,%V,%V)", alg, data.rep, url.rep)

   U_INTERNAL_ASSERT(url)

   UString token, request = createQuery(alg, data, 0, false, false);

   UTimeStamp ts(request, url);

   if (ts.UPKCS7::isValid()) token = ts.UPKCS7::getEncoded("BASE64");

   U_RETURN_STRING(token);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UTimeStamp::dump(bool reset) const
{
   UPKCS7::dump(false);

   *UObjectIO::os << '\n'
                  << "response " << (void*)response;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
