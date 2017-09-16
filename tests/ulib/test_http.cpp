// test_http.cpp

#include <ulib/file.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/http.h>

// #define JOHN

int U_EXPORT main(int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   UString x, url(argv[1], strlen(argv[1]));

#ifdef U_ALIAS
   UClientImage_Base::init();

   (void) UClientImage_Base::request_uri->assign(U_CONSTANT_TO_PARAM("/questions/1657484/can-you-give-an-example-of-stack-overflow-in-c"));

   x = UHTTP::getPathComponent(0);

   U_ASSERT_EQUALS(x, "questions")

   x = UHTTP::getPathComponent(1);

   U_ASSERT_EQUALS(x, "1657484")

   x = UHTTP::getPathComponent(2);

   U_ASSERT_EQUALS(x, "can-you-give-an-example-of-stack-overflow-in-c")

   x = UHTTP::getPathComponent(3);

   U_ASSERT(x.empty())
#endif

   UHttpClient<UTCPSocket> http(U_NULLPTR);

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
   (void) http.setHostPort(url.getHost(), url.getPortNumber());

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
