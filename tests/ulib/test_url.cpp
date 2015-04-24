// test_url.cpp

#include <ulib/url.h>
#include <ulib/file.h>
#include <ulib/utility/des3.h>
#include <ulib/utility/base64.h>
#include <ulib/utility/string_ext.h>

#include <iostream>

static void check(const UString& dati, const UString& file)
{
   U_TRACE(5,"check(%p,%p)", &dati, &file)

   uint32_t sz = dati.size() * 4;

   UString buffer1(sz), buffer2(sz);

   Url::encode(dati,    buffer1);
   Url::decode(buffer1, buffer2);

   U_INTERNAL_DUMP("buffer1 = %#.*S", U_STRING_TO_TRACE(buffer1))
   U_INTERNAL_DUMP("dati    = %#.*S", U_STRING_TO_TRACE(dati))
   U_INTERNAL_DUMP("buffer2 = %#.*S", U_STRING_TO_TRACE(buffer2))

// (void) UFile::writeToTmp(U_STRING_TO_PARAM(buffer1), false, "url.encode", 0);
// (void) UFile::writeToTmp(U_STRING_TO_PARAM(buffer2), false, "url.decode", 0);
 
   U_ASSERT( dati == buffer2 )
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString filename;

   while (cin >> filename)
      {
      UString dati = UFile::contentOf(filename);

      check(dati, filename);
      }

    Url url[] = {
       Url(U_STRING_FROM_CONSTANT("http://www.cs.wustl.edu/")),
       Url(U_STRING_FROM_CONSTANT("http://www.cs.wustl.edu/index.html")),
       Url(U_STRING_FROM_CONSTANT("http://www.cs.wustl.edu/form?var=foo")),
       Url(U_STRING_FROM_CONSTANT("http://www.notexist.com:8080/index.html")),
       Url(U_STRING_FROM_CONSTANT("http://www.notexist.com:80/index.html")),
       Url(U_STRING_FROM_CONSTANT("http://www.notexist.com:80?var=foo")),
       Url(U_STRING_FROM_CONSTANT("ftp://foo")),
       Url(U_STRING_FROM_CONSTANT("http://www/?kkk//")),
       Url(U_STRING_FROM_CONSTANT("ftp://www.cs.wustl.edu/")),
       Url(U_STRING_FROM_CONSTANT("ftp://user@www.cs.wustl.edu/")),
       Url(U_STRING_FROM_CONSTANT("ftp://user:pass@www.cs.wustl.edu/")),
       Url(U_STRING_FROM_CONSTANT("ftp://user:pass@www.cs.wustl.edu/path")),
       Url(U_STRING_FROM_CONSTANT("ftp://www.cs.wustl.edu")),
       Url(U_STRING_FROM_CONSTANT("http://www.cs.wustl.edu/index.html")),
       Url(U_STRING_FROM_CONSTANT("mailto:ace-users@cs.wustl.edu")),
       Url(U_STRING_FROM_CONSTANT("mailto:majordomo@cs.wustl.edu?Subject: subscribe ace-users")),
       Url(U_STRING_FROM_CONSTANT("mailto:nobody")),
       Url(U_STRING_FROM_CONSTANT("http://www.cs.wustl.edu")),
       Url(U_STRING_FROM_CONSTANT("file:/etc/passwd")),
       Url(U_STRING_FROM_CONSTANT("http://www.cs.wustl.edu/form?var=foo&url=http%3a//www/%3fkkk//")) };

   int i, nurl = sizeof(url)/sizeof(url[0]);

   for (i = 0; i < nurl; ++i)
      {
      cout  << '"' << url[i]              << "\" "
            << '"' << url[i].getService() << "\" "
            << '"' << url[i].getUser()    << "\" "
            << '"' << url[i].getHost()    << "\" "
            << '"' << url[i].getPort()    << "\" " 
            << '"' << url[i].getPath()    << "\" "
            << '"' << url[i].getQuery()   << "\"\n";
      }

   for (i = 0; i < nurl; ++i)
      {
      url[i].eraseUser();
      url[i].eraseQuery();
      }

   Url u = url[2];

   u.setService(U_CONSTANT_TO_PARAM("http"));
   u.setUser(U_CONSTANT_TO_PARAM("pippo"));
   u.setHost(U_CONSTANT_TO_PARAM("www.unirel.com"));
   u.setPort(8080);
   u.setPath(U_CONSTANT_TO_PARAM("/usr/src"));
   u.setQuery(U_CONSTANT_TO_PARAM("var1=foo&var2=bar"));

   cout  << '"' << u              << "\" "
         << '"' << u.getService() << "\" "
         << '"' << u.getUser()    << "\" "
         << '"' << u.getHost()    << "\" "
         << '"' << u.getPort()    << "\" " 
         << '"' << u.getPath()    << "\" "
         << '"' << u.getQuery()   << "\"\n";

   U_ASSERT( u.isQuery() ) 

   u.setPath(U_CONSTANT_TO_PARAM("/info"));

   u.addQuery(U_CONSTANT_TO_PARAM("ip"), U_CONSTANT_TO_PARAM("10.30.1.130"));
   u.addQuery(U_CONSTANT_TO_PARAM("ap"), U_CONSTANT_TO_PARAM("ap@palazzoVecchio"));

   u.setPath(U_CONSTANT_TO_PARAM("/info"));

   u.addQuery(U_CONSTANT_TO_PARAM("ip"), U_CONSTANT_TO_PARAM("10.30.1.131"));
   u.addQuery(U_CONSTANT_TO_PARAM("ap"), U_CONSTANT_TO_PARAM("ap@palazzoNuovo"));

   U_ASSERT( u.isQuery() ) 

   UString info = u.getPathAndQuery();

   U_INTERNAL_ASSERT(info)

   cout  << '"' << info << "\"\n";

#define URL_BASE64    "U2FsdGVkX18CtybN+EswQN4oGqmRmH7OWMvKC+ilxpxcjzTKOGEbkZfq+UlZXtX/+IZx6d3nf/MXiDX6Exp6V5AAQLRquwujP6ZqOVNqJYxc8weXv8X1e0z1rykGr75k1AAjhl411QzESBTLxW7r+1V59mD6r3LKGxdFQj3hJ7UxntSyVzkMW9wjAmc2mffbqsauh2s2TFClAO/gzLnOt5OCmQ/bWbsdITU+d+8H/AHXPVGSMKHFtg=="

#define URL_BASE64URL "U2FsdGVkX18CtybN-EswQN4oGqmRmH7OWMvKC-ilxpxcjzTKOGEbkZfq-UlZXtX_-IZx6d3nf_MXiDX6Exp6V5AAQLRquwujP6ZqOVNqJYxc8weXv8X1e0z1rykGr75k1AAjhl411QzESBTLxW7r-1V59mD6r3LKGxdFQj3hJ7UxntSyVzkMW9wjAmc2mffbqsauh2s2TFClAO_gzLnOt5OCmQ_bWbsdITU-d-8H_AHXPVGSMKHFtg"

#define URL_ENDCODED "U2FsdGVkX18CtybN%2BEswQN4oGqmRmH7OWMvKC%2BilxpxcjzTKOGEbkZfq%2BUlZXtX/%2BIZx6d3nf/MXiDX6Exp6V5AAQLRquwujP6ZqOVNqJYxc8weXv8X1e0z1rykGr75k1AAjhl411QzESBTLxW7r%2B1V59mD6r3LKGxdFQj3hJ7UxntSyVzkMW9wjAmc2mffbqsauh2s2TFClAO/gzLnOt5OCmQ/bWbsdITU%2Bd%2B8H/AHXPVGSMKHFtg%3D%3D"

#define URL_ENDCODED_BASE64URL "U2FsdGVkX1808rcrEGMANm0PQYLvNaoTBgfLjgnPI68jOshyAcHRgBBe9OcJ-2sU-PoM_mFzlkMECF9fiXdbVf0CgVYc3AWmS43m9_tSPF7eFUlsWApYKv-LvPjbcNXLJltNgAXVc71XuVKqPqB6mSjIhmcJAtFZ86TB3FXLkNVz0QcQLcSCQ_UFQ0kNOe89TJ3dHvQ-Wkx8JfPu8qEotnxUourW6xVq3Vrp_9ArkB_1dK9Ag8Okz7AEv9v-AZw9011syWIY57E_jx_IkGNEWA"

   bool result = u_isBase64(U_CONSTANT_TO_PARAM(URL_BASE64));

   U_INTERNAL_ASSERT(result)

   result = u_isUrlEncodeNeeded(U_CONSTANT_TO_PARAM(URL_BASE64));

   U_INTERNAL_ASSERT(result)

   result = u_isUrlEncodeNeeded(U_CONSTANT_TO_PARAM(URL_BASE64URL));

   U_INTERNAL_ASSERT_EQUALS(result, false)

   result = u_isUrlEncoded(U_CONSTANT_TO_PARAM(URL_ENDCODED), false);

   U_INTERNAL_ASSERT(result)

   result = u_isBase64(U_CONSTANT_TO_PARAM(URL_ENDCODED));

   U_INTERNAL_ASSERT_EQUALS(result, false)

   result = u_isBase64(U_CONSTANT_TO_PARAM("name=stefano"));

   U_INTERNAL_ASSERT(result)

   result = u_isUrlEncoded(U_CONSTANT_TO_PARAM("name=stefano"), false);

   U_INTERNAL_ASSERT_EQUALS(result, false)

   UString value_decoded(U_CAPACITY), output(U_CAPACITY);

   Url::decode(U_CONSTANT_TO_PARAM(URL_ENDCODED), value_decoded);

   U_INTERNAL_ASSERT(value_decoded.equal(U_CONSTANT_TO_PARAM(URL_BASE64)))

   result = u_isUrlEncoded(U_CONSTANT_TO_PARAM(URL_BASE64), false);

   U_INTERNAL_ASSERT_EQUALS(result, false)

   result = u_isUrlEncoded(U_CONSTANT_TO_PARAM(URL_BASE64URL), false);

   U_INTERNAL_ASSERT_EQUALS(result, false)

   result = u_isUrlEncoded(U_CONSTANT_TO_PARAM("address+space+usage%3A12.81+MBytes+-+rss+usage%3A3.59+MBytes"), false);

   U_INTERNAL_ASSERT(result)

   result = u_isUrlEncodeNeeded(U_CONSTANT_TO_PARAM(URL_ENDCODED));

   U_INTERNAL_ASSERT_EQUALS(result, false)

   result = UBase64::decode(U_CONSTANT_TO_PARAM(URL_BASE64), value_decoded);

   U_INTERNAL_ASSERT(result)

   result = UBase64::decodeUrl(U_CONSTANT_TO_PARAM(URL_BASE64URL), output);

   U_INTERNAL_ASSERT(result)
   U_INTERNAL_ASSERT_EQUALS(value_decoded, output)

   result = UBase64::decodeAll(U_CONSTANT_TO_PARAM(URL_BASE64), output);

   U_INTERNAL_ASSERT(result)
   U_INTERNAL_ASSERT_EQUALS(value_decoded, output)

   result = UBase64::decodeAll(U_CONSTANT_TO_PARAM(URL_BASE64URL), output);

   U_INTERNAL_ASSERT(result)
   U_INTERNAL_ASSERT_EQUALS(value_decoded, output)

   result = u_isUrlEncoded(U_CONSTANT_TO_PARAM(URL_ENDCODED_BASE64URL), false);

   U_INTERNAL_ASSERT_EQUALS(result, false)

   result = UBase64::decodeUrl(U_CONSTANT_TO_PARAM(URL_ENDCODED_BASE64URL), value_decoded);

   U_INTERNAL_ASSERT(result)

   UDES3::setPassword("vivalatopa");

   result = UDES3::decode(value_decoded, output);

   U_INTERNAL_ASSERT(result)

   result = u_isUrlEncoded(U_STRING_TO_PARAM(output), true);

   U_INTERNAL_DUMP("output = %#.*S", U_STRING_TO_TRACE(output))

   U_INTERNAL_ASSERT(result)

   UVector<UString> name_value(16);

   uint32_t sz = UStringExt::getNameValueFromData(output, name_value, U_CONSTANT_TO_PARAM("&"));

   U_INTERNAL_ASSERT_EQUALS(sz, 16)
   U_INTERNAL_ASSERT_EQUALS(name_value[0],  "uid")
   U_INTERNAL_ASSERT_EQUALS(name_value[1],  "00:14:a5:6e:9c:cb")
   U_INTERNAL_ASSERT_EQUALS(name_value[14], "redir_to")
   U_INTERNAL_ASSERT_EQUALS(name_value[15], "http%253A//www.mozilla.org/about/")

   result = u_isUrlEncodeNeeded(U_STRING_TO_PARAM(name_value[1]));

   U_INTERNAL_ASSERT(result)

   /*
   U_ASSERT( u.getService() == UString( u.getService(buffer, sizeof(buffer)) ) )
   U_ASSERT( u.getUser()    == UString( u.getUser(buffer, sizeof(buffer)) ) )
   U_ASSERT( u.getHost()    == UString( u.getHost(buffer, sizeof(buffer)) ) )
   U_ASSERT( u.getPort()    == 8080 )
   U_ASSERT( u.getPath()    == UString( u.getPath(buffer, sizeof(buffer)) ) )
   U_ASSERT( u.getQuery()   == UString( u.getQuery(buffer, sizeof(buffer)) ) )
   */

   /*
   UString entry(1000), value(1000);

   U_ASSERT( u.firstQuery(entry, value) == u.firstQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2)) )
   U_ASSERT( entry == UString(buffer1) )
   U_ASSERT( value == UString(buffer2) )
   U_ASSERT( entry == UString(U_CONSTANT_TO_PARAM("var1")) )
   U_ASSERT( value == UString(U_CONSTANT_TO_PARAM("foo")) )

   u.firstQuery(entry, value);
   u.nextQuery(entry, value);

   u.firstQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2));
   u.nextQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2));
   U_ASSERT( entry == UString(buffer1) )
   U_ASSERT( value == UString(buffer2) )
   U_ASSERT( entry == U_STRING_FROM_CONSTANT("var2") )
   U_ASSERT( value == U_STRING_FROM_CONSTANT("ba") )

   u.addQuery(U_CONSTANT_TO_PARAM("var3"), U_CONSTANT_TO_PARAM("co"));

   u.firstQuery(entry, value);
   u.nextQuery(entry, value);
   u.nextQuery(entry, value);

   U_ASSERT( u.firstQuery(entry, value) == u.firstQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2)) )
   u.nextQuery(entry, value);
   u.nextQuery(entry, value);
   u.firstQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2));
   u.nextQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2));
   u.nextQuery(buffer1, sizeof(buffer1), buffer2, sizeof(buffer2));

   U_ASSERT( entry == UString(buffer1) )
   U_ASSERT( value == UString(buffer2) )
   U_ASSERT( entry == U_STRING_FROM_CONSTANT("var3") )
   U_ASSERT( value == U_STRING_FROM_CONSTANT("co") )

   entry = U_STRING_FROM_CONSTANT("var2");

   U_ASSERT( u.findQuery(entry, value) == true )
   U_ASSERT( entry == U_STRING_FROM_CONSTANT("var2") )
   U_ASSERT( value == U_STRING_FROM_CONSTANT("ba") )

   entry.clear();
   value = U_STRING_FROM_CONSTANT("foo");

   U_ASSERT( u.findQuery(entry, value) == true )
   U_ASSERT( entry == U_STRING_FROM_CONSTANT("var1") )
   U_ASSERT( value == U_STRING_FROM_CONSTANT("foo") )
   */
}
