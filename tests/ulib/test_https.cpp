// test_https.cpp

#include <ulib/net/client/http.h>
#include <ulib/ssl/net/sslsocket.h>

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString url(argv[1], strlen(argv[1])); 

   UHttpClient<USSLSocket> http(0);
   http.setRequestPasswordAuthentication(U_STRING_FROM_CONSTANT("Aladdin"),
                                         U_STRING_FROM_CONSTANT("open sesame"));

   if (http.connectServer(url) &&
       http.sendRequest())
      {
      UString content = http.getContent();

      cout.write(content.data(), content.size());
      }
}
