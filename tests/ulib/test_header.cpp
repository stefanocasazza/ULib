// test_header.cpp

#include <ulib/mime/header.h>

#define HEADER_1                                               \
   "Host: dummy\r\n"                                           \
   "Content-Length: 1200\r\n"                                  \
   "Authorization: Digest Name1=Value1\r\n"                    \
   "Cookie: $Version=1; NameA=valueA;\r\n"                     \
   "  NameB = ValueB;$Path=\"/\" ; $Domain=domain1;\r\n"       \
   "  NameC = ValueC;$Domain=\"/\" ; $Path=domain1;\r\n"       \
   "  $Port=\"123\"\r\n"                                       \
   "\r\n"

#define HEADER_2                                               \
   "Date: Fri, 14 Nov 2003 10:57:35 GMT\r\n"                   \
   "Server: Apache/1.3.23 (Unix)  (Red-Hat/Linux) mod_ssl/2.8.7 OpenSSL/0.9.6b DAV/1.0.3 PHP/4.1.2 mod_perl/1.26\r\n" \
   "Last-Modified: Thu, 07 Nov 2002 08:40:12 GMT\r\n"          \
   "ETag: \"13b94-15f9-3dca26ec\"\r\n"                         \
   "Accept-Ranges: bytes\r\n"                                  \
   "Content-Type: text/html\r\n"                               \
   "WWW-Authenticate: Basic realm=\"WallyWorld\"\r\n"          \
   "Location: https://user:passwd@dummy/path?query=value\r\n"  \
   "Set-Cookie: Version=1; NameA=valueA;\r\n"                  \
   "  NameB = ValueB;Path=\"/\" ; Domain=domain1;\r\n"         \
   "  NameC = ValueC;Domain=\"/\" ; Path=domain1;\r\n"         \
   "  Port=\"123\"\r\n"                                        \
   "Set-Cookie2: Version=1; NameA=valueA;\r\n"                 \
   "  NameB = ValueB;Path=\"/\" ; Domain=domain1;\r\n"         \
   "  NameC = ValueC;Domain=\"/\" ; Path=domain1;\r\n"         \
   "  Port=\"123\"\r\n"                                        \
   "\r\n"

int
U_EXPORT main (int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString h1 = U_STRING_FROM_CONSTANT(HEADER_1),
           h2 = U_STRING_FROM_CONSTANT(HEADER_2);

   UMimeHeader h;
   h.setIgnoreCase(true);
   unsigned n = h.parse(h1);

   U_ASSERT( n == 4 )

   n = h.parse(h2);

   U_ASSERT( n == 14 )

   UString x = h.getHeader(U_STRING_FROM_CONSTANT("Set-COOkie2"));

   U_ASSERT( x == U_STRING_FROM_CONSTANT("Version=1; NameA=valueA;NameB = ValueB;Path=\"/\" ; Domain=domain1;NameC = ValueC;Domain=\"/\" ; Path=domain1;Port=\"123\"") )

   UString y = h.getHeader(U_STRING_FROM_CONSTANT("WWW-AutHENTIcate"));

   U_ASSERT( y == U_STRING_FROM_CONSTANT("Basic realm=\"WallyWorld\"") )

   cout << h << endl;
}
