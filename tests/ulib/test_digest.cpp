// test_digest.cpp

#include <ulib/utility/services.h>

#include <ulib/internal/chttp.h>

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   U_INTERNAL_DUMP("argv[0] = %S argv[1] = %S argv[2] = %S argv[3] = %S argv[4] = %S argv[5] = %S argv[6] = %S", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6])

   UString ha1, nc, nonce, cnonce, uri, user, ha3(33U);

      ha1.assign(argv[1]);
       nc.assign(argv[2]);
    nonce.assign(argv[3]);
   cnonce.assign(argv[4]);
      uri.assign(argv[5]);
     user.assign(argv[6]);

   u_init_http_method_list();

   (void) UServices::setDigestCalcResponse(ha1, nc, nonce, cnonce, uri, user, ha3);

   cout.write(U_STRING_TO_PARAM(ha3));
   cout.put('\n');
}
