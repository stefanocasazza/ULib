// test_smtp.cpp

#include <ulib/net/client/smtp.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   USmtpClient smtp;
   UString tmp(argv[1], strlen(argv[1]));

   if (smtp._connectServer(tmp))
      {
      UString rcpt(argv[2], strlen(argv[2]));
#  ifdef USE_LIBSSL
      bool secure = (argc == 4);
#  else
      bool secure = false;
#  endif

      smtp.setSenderAddress(U_STRING_FROM_CONSTANT("< stefano.casazza@unirel.com >"));
      smtp.setRecipientAddress(rcpt);

      cout << (smtp.sendMessage(secure) ? "mail     send"
                                        : "mail NOT send") << '\n';

   // exit(0);
      }
}
