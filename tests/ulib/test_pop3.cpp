// test_pop3.cpp

#include <ulib/net/client/pop3.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UPop3Client pop3;
   UVector<UString> vec, vec1;
   UString tmp(argv[1], strlen(argv[1]));

   if (pop3._connectServer(tmp))
      {
      if (pop3.getCapabilities(vec1) && pop3.isSTLS(vec1)) (void) pop3.startTLS();

      if (pop3.login(argv[2], argv[3]))
         {
         cerr << pop3.getUIDL(vec) << endl;
         cerr << vec << endl;

         cerr << pop3.getHeader(1);
         cerr << pop3.getMessage(1);

         vec.clear();
         cerr << pop3.getAllHeader(vec) << endl;
         cerr << vec << endl;

         vec.clear();
         cerr << pop3.getAllMessage(vec) << endl;
         cerr << vec << endl;

         cerr << pop3.deleteAllMessage();
         cerr << pop3.reset();
         cerr << pop3.deleteMessage(1);
         cerr << pop3.reset();
         cerr << pop3.quit();

         cout << vec1 << endl;
         }

   // exit(0);
      }
}
