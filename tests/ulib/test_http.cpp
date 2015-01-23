// test_http.cpp

#include <ulib/file.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/http.h>

// #define JOHN

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   UString x, url(argv[1], strlen(argv[1]));

   UHttpClient<UTCPSocket> http(0);

#ifndef JOHN
   http.setRequestPasswordAuthentication(U_STRING_FROM_CONSTANT("Aladdin"),
                                         U_STRING_FROM_CONSTANT("open sesame"));

   if (http.connectServer(url) &&
       http.sendRequest())
      {
      UString content = http.getContent();

      cout.write(content.data(), content.size());
      }
#else
   (void) http.setHostPort(url.getHost(), url.getPort());

#  define AB_REQUEST(ver) "GET /usp/benchmarking.usp?name=stefano HTTP/1."#ver"\r\n" \
                          "Host: stefano\r\n" \
                          "User-Agent: ApacheBench/2.3\r\n" \
                          "Accept: */*\r\n" \
                          "\r\n"

/*
#  define HTTP_VER 1
*/
#  define HTTP_VER 0
   x = U_STRING_FROM_CONSTANT(AB_REQUEST(HTTP_VER));

#  if HTTP_VER == 1
   if (http.connect())
      {
      for (uint32_t i = 0 ; i < 100000 && http.sendRequest(x); ++i)
         {
         }
      }
#  else
   for (uint32_t i = 0 ; i < 100000; ++i)
      {
      if (http.connect())
         {
         if (http.sendRequest(x))
            {
            http.close();

            continue;
            }
         }

      break;
      }
#  endif
#endif
}
