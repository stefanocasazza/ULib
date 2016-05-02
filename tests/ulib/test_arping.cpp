// test_arping.cpp

#include <ulib/net/ping.h>
#include <ulib/container/vector.h>

int U_EXPORT main(int argc, char** argv)
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "::main(%d,%p)", argc, argv)

   u_init_ulib_hostname();

   UString x;
   fd_set* addrmask;
   UIPAddress* item;
   const char* device;
   const char* hostname;
   UVector<UString> vInternalDevice;
   int i, j, num_radio = (argc - 1) / 3, nfds = num_radio * 2;

   UPing* sockp[128];
   UVector<UIPAddress*>* vaddr[128];

   // -----------------------------------------------------------------------------------------
   // eth0 10.10.100.123 10.1.1.1 eth0 10.30.1.110 10.30.1.111 eth0 10.10.100.124 10.10.100.125
   // -----------------------------------------------------------------------------------------
   // 1    2             3        4    5           6           7    8             9
   // -----------------------------------------------------------------------------------------

   for (i = 0; i < num_radio; ++i)
      {
      device = argv[1+(i*3)];

      (void) x.assign(device);

      vInternalDevice.push(x);

      U_NEW(UPing, sockp[i], UPing(3000, false));

      sockp[i]->initArpPing(device);

      U_NEW(UVector<UIPAddress*>, vaddr[i], UVector<UIPAddress*>);

      for (j = 0; j < 2; ++j)
         {
         U_NEW(UIPAddress, item, UIPAddress);

         hostname = argv[2+(i*3)+j];

         (void) x.assign(hostname);

         item->setHostName(x);

         vaddr[i]->push(item);
         }
      }

   addrmask = UPing::arping(sockp, vaddr, num_radio, true, vInternalDevice);

   while (addrmask == 0) addrmask = UPing::checkForPingAsyncCompletion(num_radio);

   if (addrmask)
      {
      for (i = 0; i < nfds; ++i)
         {
         if (FD_ISSET(i, addrmask))
            {
            item = vaddr[i / 2]->at(i % 2);

            cout << item->getAddressString() << endl;
            }
         }
      }

   for (i = 0; i < num_radio; ++i)
      {
      delete vaddr[i];
      delete sockp[i];
      }
}
