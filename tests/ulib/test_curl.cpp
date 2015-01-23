// test_curl.cpp

#include <ulib/curl/curl.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString filename;

   while (cin >> filename)
      {
      UCURL curl;

      curl.setOption(CURLOPT_SSL_VERIFYPEER, 0L);
      curl.setOption(CURLOPT_SSL_VERIFYHOST, 0L);

      curl.setURL(filename.c_str());

      cout << "URL: " << filename << endl;
      cerr << "URL: " << filename << endl;

      if (curl.performWait()) cout << curl.getResponse();
      }

// exit(0);
}
