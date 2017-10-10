// File: test_socket.cpp

// The sample code implements an Echo Server and client
// using both TCP and UDP sockets over IPv4 and IPv6 networks. Command line
// parameters indicate the functionality at run time.

#include <ulib/net/ping.h>
#include <ulib/net/udpsocket.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/container/vector.h>
#include <ulib/utility/socket_ext.h>

/******************************************************************************/
/* void PrintIPAddress(char *pcInformationString, UIPAddress& cAddr)          */
/*                                                                            */
/* This function will take an information string and IPAddress and output the */
/* the information string and details of the IPAddress to stdout.             */
/******************************************************************************/
static void PrintIPAddress(const char* pcInformationString, UIPAddress& cAddr)
{
   U_TRACE(5, "::PrintIPAddress(%S,%p)", pcInformationString, &cAddr)

   printf("%s\n", pcInformationString);
   printf(" - Name    : %s\n", (const char*) cAddr.getAddressString());
   printf(" - Family  : %s\n", (cAddr.getAddressFamily() == AF_INET6)?"IPv6":
                 ((cAddr.getAddressFamily() == AF_INET)?"IPv4":"Unknown"));
   printf(" - Address : %s\n", cAddr.getAddressString());
}

/******************************************************************************/
/* void TCPEchoClient(IPAddress &cEchoServer, unsigned int iPortNumber,       */
/*                    char *pcEchoString, bool bIPv6)                         */
/*                                                                            */
/* This function performs the task of the echo client operating using a TCP   */
/* socket.  Parameters include the IP Address and Port Number of the Echo     */
/* Server to use, the data string to send to the Echo Server and a flag that  */
/* indicates whether to use IPv6 or IPv4 sockets.  The actual functional code */
/* consists of creating a TCPClientSocket instance, calling the ConnectServer */
/* method to connect to the Echo Server, then calling Send followed by        */
/* Recv to perform the transaction.  The function then ends, closing the      */
/* TCP Socket when cClientSocket goes out of scope and returning to the       */
/* calling function.  If an error occurs, function execution stops and an     */
/* exception is thrown, this is not caught at this level.  Other code simply  */
/* prints details for debugging purposes.                                     */
/******************************************************************************/
static void TCPEchoClient(UIPAddress& cEchoServer, unsigned int iPortNumber, const char* pcEchoString, bool bIPv6)
{
   U_TRACE(5, "::TCPEchoClient(%p,%u,%S,%b)", &cEchoServer, iPortNumber, pcEchoString, bIPv6)

   int iBytesTransferred;
   char pcRecvBuffer[65535];
   UTCPSocket cClientSocket(bIPv6);

   pcRecvBuffer[0] = '\0';

   printf("TCP %s Echo Client\n\n", (bIPv6)?"IPv6":"IPv4");

   PrintIPAddress("Using Echo Server", cEchoServer);
   printf(" - Port    : %u\n\n", iPortNumber);

   cClientSocket.USocket::connectServer(cEchoServer, iPortNumber);

   printf("Socket Connected...\n");

   PrintIPAddress("Socket Details - Local Information", cClientSocket.localIPAddress());
   printf(" - Port    : %u\n\n", cClientSocket.localPortNumber());

   PrintIPAddress("Socket Details - Server Information", cClientSocket.remoteIPAddress());
   printf(" - Port    : %u\n\n", cClientSocket.remotePortNumber());

   printf("Send Buffer    : [%s]\n", pcEchoString);
   printf("Receive Buffer : [%s]\n\n", pcRecvBuffer);

   printf("Sending Echo Request...\n");
   iBytesTransferred = cClientSocket.send(pcEchoString, strlen(pcEchoString));
   printf("Sent %d bytes of data\n\n", iBytesTransferred);

   printf("Receiving Echo Response...\n");
   iBytesTransferred = cClientSocket.recv(pcRecvBuffer, 65535);
   printf("Received %d bytes of data\n", iBytesTransferred);
   pcRecvBuffer[iBytesTransferred > 0 ? iBytesTransferred : 0] = 0;

   printf("Send Buffer    : [%s]\n", pcEchoString);
   printf("Receive Buffer : [%s]\n", pcRecvBuffer);

   cClientSocket.close();
}

/******************************************************************************/
/* void TCPEchoServer(unsigned int iPortNumber, bool bIPv6)                   */
/*                                                                            */
/* This function performs the task of the echo server operating using a TCP   */
/* socket.  Parameters include Port Number the server will set up for the     */
/* listening socket to use and a flag that indicates whether to use IPv6 or   */
/* IPv4 sockets.  The actual functional code consists of creating a           */
/* TCPServerSocket instance bound to the provided Port Number.  We then go    */
/* into an infinite loop where we call AcceptClient, waiting for a connection */
/* from an Echo client.  The connection is assigned to pcClientSocket.  We    */
/* then call Recv followed by Send to perform the Echo transaction.           */
/* Finally, pcClientSocket is deleted to close the TCP Connection, we then    */
/* jump to the top of the loop to wait for the next connection.  If an error  */
/* occurs, function execution stops and an exception is thrown, this is not   */
/* caught at this level.  Other code prints details for debugging purposes.   */
/******************************************************************************/
static void TCPEchoServer(unsigned int iPortNumber, bool bIPv6)
{
   U_TRACE(5, "::TCPEchoServer(%u,%b)", iPortNumber, bIPv6)

   char pcBuffer[65535];
   int iBytesTransferred;
   UTCPSocket* pcClientSocket;

   UTCPSocket cServerSocket(bIPv6);

   cServerSocket.setServer(iPortNumber);
   cServerSocket.reusePort(O_RDWR | O_CLOEXEC);
   cServerSocket.setLocal();

   pcBuffer[0] = '\0';

   printf("TCP %s Echo Server\n\n", (bIPv6)?"IPv6":"IPv4");

   PrintIPAddress("Created Server Socket - Socket Details", cServerSocket.localIPAddress());
   printf(" - Port    : %u\n\n", cServerSocket.localPortNumber());

   int fd = cServerSocket.getFd();

   /*
   printf("ARP cache\n");

   UString arp_cache;
   UVector<UString> varp_cache;

   if (USocketExt::getARPCache(arp_cache, varp_cache)) cout << varp_cache << "\n\n";
   */

   for (;;)
      {
      printf("Waiting for Connection...\n");

      fflush(stdout);
      pcClientSocket = new UTCPSocket();
      cServerSocket.acceptClient(pcClientSocket);
      pcClientSocket->setLocal();

      printf("Connection Made...\n\n");

      PrintIPAddress("Listening Server Socket - Socket Details", cServerSocket.localIPAddress());
      printf(        " - Port    : %u\n\n",                      cServerSocket.localPortNumber());

      PrintIPAddress("Client Communication Socket on", pcClientSocket->localIPAddress());
      printf(        " - Port    : %u\n\n",            pcClientSocket->localPortNumber());
      PrintIPAddress("Is Connected to",                pcClientSocket->remoteIPAddress());
      printf(        " - Port    : %u\n\n",            pcClientSocket->remotePortNumber());

      cout << "Host name       = " << cServerSocket.localIPAddress().getHostName()                         << '\n'
           << "Host address    = " << cServerSocket.localIPAddress().getAddressString()                    << '\n'
           << "Node name       = " << USocketExt::getNodeName()                                            << '\n'
           << "MAC address     = " << pcClientSocket->getMacAddress("eth0")                                << '\n'
           << "Network device  = " << USocketExt::getNetworkDevice("eth0")                                 << '\n'
           << "Gateway address = " << USocketExt::getGatewayAddress(U_CONSTANT_TO_PARAM("192.168.1.0/24")) << '\n'
           << "Network address = " << USocketExt::getNetworkAddress(fd, "eth0")                            << "\n\n";

      iBytesTransferred = pcClientSocket->recv(pcBuffer, 65535);
      printf("Received %d bytes of data\n", iBytesTransferred);
      printf("Buffer : [");

      for (int i = 0; i < iBytesTransferred; i++) printf("%c", pcBuffer[i]);

      printf("]\n\n");

      printf("Echoing data to client...\n\n");

      iBytesTransferred = pcClientSocket->send(pcBuffer, iBytesTransferred);
      printf("Sent %d bytes of data\n\n", iBytesTransferred);

      printf("Closing Client Socket...\n\n");

      pcClientSocket->close();

      delete pcClientSocket;
      }

   cServerSocket.close();
}

/******************************************************************************/
/* void UDPEchoClient(IPAddress &cEchoServer, unsigned int iPortNumber,       */
/*                    char *pcEchoString, bool bIPv6)                         */
/*                                                                            */
/* This function performs the task of the echo client operating using a UDP   */
/* socket.  Parameters include the IP Address and Port Number of the Echo     */
/* Server to use, the data string to send to the Echo Server and a flag that  */
/* indicates whether to use IPv6 or IPv4 sockets.  The actual functional code */
/* consists of creating a UDPSocket instance, calling the Connect             */
/* method to connect (using the concept of a connected UDP socket where data  */
/* can only be sent to or received from the connected remote socket) to the   */
/* Echo Server, then calling SendDatagram followed by ReceiveDatagram to      */
/* perform the transaction.  DisConnect is called to close the connected UDP  */
/* socket and we return to the calling function.  If there is an error,       */
/* function execution stops and an exception is thrown, this is not caught at */
/* this level.  Other code simply prints details for debugging purposes.      */
/******************************************************************************/
static void UDPEchoClient(UIPAddress& cEchoServer, unsigned int iPortNumber, const char* pcEchoString, bool bIPv6)
{
   U_TRACE(5, "::UDPEchoClient(%p,%u,%S,%b)", &cEchoServer, iPortNumber, pcEchoString, bIPv6)

   int iBytesTransferred;
   char pcRecvBuffer[65535];
   UUDPSocket cClientSocket(bIPv6);

   pcRecvBuffer[0] = '\0';

   printf("UDP %s Echo Client\n\n", (bIPv6)?"IPv6":"IPv4");

   PrintIPAddress("Using Echo Server", cEchoServer);
   printf(" - Port    : %u\n\n", iPortNumber);

   cClientSocket.USocket::connectServer(cEchoServer, iPortNumber);

   printf("Socket Connected (UDP connected socket)...\n");

   PrintIPAddress("Socket Details - Local Information", cClientSocket.localIPAddress());
   printf(" - Port    : %u\n\n", cClientSocket.localPortNumber());

   PrintIPAddress("Socket Details - Server Information", cClientSocket.remoteIPAddress());
   printf(" - Port    : %u\n\n", cClientSocket.remotePortNumber());

   printf("Send Buffer    : [%s]\n", pcEchoString);
   printf("Receive Buffer : [%s]\n\n", pcRecvBuffer);

   printf("Sending Echo Request...\n");
   iBytesTransferred = cClientSocket.send(pcEchoString, (int)strlen(pcEchoString));
   printf("Sent %d bytes of data\n\n", iBytesTransferred);

   printf("Receiving Echo Response...\n");
   iBytesTransferred = cClientSocket.recv(pcRecvBuffer, 65535);
   printf("Received %d bytes of data\n", iBytesTransferred);
   pcRecvBuffer[iBytesTransferred > 0 ? iBytesTransferred : 0] = 0;

   printf("Send Buffer    : [%s]\n", pcEchoString);
   printf("Receive Buffer : [%s]\n", pcRecvBuffer);

   cClientSocket.close();
}

/******************************************************************************/
/* void UDPEchoServer(unsigned int iPortNumber, bool bIPv6)                   */
/*                                                                            */
/* This function performs the task of the echo server operating using a UDP   */
/* socket.  Parameters include Port Number the server will set up to listen   */
/* for datagrams and a flag that indicates whether to use IPv6 or IPv4        */
/* sockets.  The actual functional code consists of creating a UDPSocket      */
/* instance bound to the provided Port Number.  We then go into an infinite   */
/* waiting for a datagram to arrive by calling ReceiveDatagram, this returns  */
/* the IP Address and Port Number of the sender.  We call the SendDatagram    */
/* method to echo the datagram back to the sender.  Finally, we jump to the   */
/* top of the loop to wait for the next datagram.  If an error occurs,        */
/* function execution stops and an exception is thrown, this is not caught at */
/* this level.  Other code prints details for debugging purposes.             */
/******************************************************************************/
static void UDPEchoServer(unsigned int iPortNumber, bool bIPv6)
{
   U_TRACE(5, "::UDPEchoServer(%u,%b)", iPortNumber, bIPv6)

   char pcBuffer[65535];
   UIPAddress cIPSource;
   int iBytesTransferred;
   unsigned int iPortSource;

   UUDPSocket cServerSocket(bIPv6);

   cServerSocket.setServer(iPortNumber, U_NULLPTR);
   cServerSocket.reusePort(O_RDWR | O_CLOEXEC);
   cServerSocket.setLocal();

   pcBuffer[0] = '\0';

   printf("UDP %s Echo Server\n\n", (bIPv6)?"IPv6":"IPv4");

   PrintIPAddress("Created Server Socket - Socket Details", cServerSocket.localIPAddress());
   printf(" - Port    : %u\n\n", cServerSocket.localPortNumber());

   for (;;)
      {
      PrintIPAddress("Waiting for data on :", cServerSocket.localIPAddress());
      printf(" - Port    : %u\n\n", cServerSocket.localPortNumber());

      fflush(stdout);
      iBytesTransferred = cServerSocket.recvFrom(pcBuffer, 65535, 0, cIPSource, iPortSource);
      PrintIPAddress("Received datagram from :", cIPSource);
      printf(" - Port    : %u\n", iPortSource);
      printf(" - Size    : %d bytes\n", iBytesTransferred);
      printf(" - Buffer  : [");
      for (int i = 0; i < iBytesTransferred; i++) printf("%c", pcBuffer[i]);
      printf("]\n\n");

      printf("Echoing datagram to client...\n\n");

      iBytesTransferred = cServerSocket.sendTo(pcBuffer, iBytesTransferred, 0, cIPSource, iPortSource);
      printf("Sent %d bytes of data\n\n", iBytesTransferred);
      }

   cServerSocket.close();
}

/******************************************************************************/
/* bool SetFlag(char *pcArg, char *pcTrueString, char *pcFalseString,         */
/*              bool &bFlag, bool bDefault)                                   */
/*                                                                            */
/* This function will take an argument presented as a string, as well as two  */
/* strings to compare the argument to, the first of these two strings will    */
/* represent setting the flag to true, the second to false.  The fourth       */
/* parameter is the flag that will be set based on the value of the argument  */
/* while the final parameter signifies the default value for the flag if      */
/* neither of the two values match.  The function returns true if one of the  */
/* arguments matched and the flag was set.  The function returns false if     */
/* neither argument matched and the flag was set to the default value.  This  */
/* function is used to help parse the command line looking for flags.  The    */
/* return value will say if the parameter was parsed or a default value was   */
/* applied.                                                                   */
/******************************************************************************/
static bool SetFlag(const char* pcArg, const char* pcTrueString, const char* pcFalseString, bool& bFlag, bool bDefault)
{
   U_TRACE(5, "::SetFlag(%S,%S,%S,%p,%b)", pcArg, pcTrueString, pcFalseString, &bFlag, bDefault)

   bFlag = false;

   if (strcasecmp(pcArg, pcTrueString))
      {
      if (strcasecmp(pcArg, pcFalseString))
         {
         bFlag = bDefault;

         return false;
         }
      }
   else
      {
      bFlag = true;
      }

   return true;
}

/******************************************************************************/
/* int U_EXPORT main(int iArgC, char** ppcArgV)                               */
/*                                                                            */
/* Program code, first some long winded code to parse all of the command line */
/* parameters, set variables based on these paramters and print a command     */
/* usage description if the parameters are incorrectly formatted.  The        */
/* command line format is:                                                    */
/*                                                                            */
/* test_socket serv [ip4 | ip6] [tcp | udp] portnum                           */
/* test_socket [client] [ip4 | ip6] [tcp | udp] servname portnum echostring   */
/*                                                                            */
/* For the echo server, the second two parameters are optional, for the echo  */
/* client, the first three parameters are optional.  The default values       */
/* signify a IPv4 TCP Echo Client.  A port number is always specified, the    */
/* echo client also requires the name of the Echo Server and the string to    */
/* send to the server for processing.                                         */
/*                                                                            */
/* Following this, we call one of the four echo functions based on the values */
/* of the variables.                                                          */ 
/******************************************************************************/
int U_EXPORT main(int iArgC, char** ppcArgV)
{
   U_ULIB_INIT(ppcArgV);

   U_TRACE(5, "::main(%d,%p)", iArgC, ppcArgV)

   u_init_ulib_hostname();

   int iCurrentArg;
   char* pcEchoString;
   UIPAddress cEchoServerAddress;
   unsigned int iServerPortNumber;
   bool bUseIPv6, bUseTCP, bServer;

   iCurrentArg = 1;

   if (SetFlag(ppcArgV[iCurrentArg], "serv", "client", bServer,  false)) iCurrentArg++;
   if (SetFlag(ppcArgV[iCurrentArg],  "ip6",    "ip4", bUseIPv6, false)) iCurrentArg++;
   if (SetFlag(ppcArgV[iCurrentArg],  "tcp",    "udp", bUseTCP,   true)) iCurrentArg++;

#ifdef USE_C_ARES
   USocketExt::startResolv("google.com");
#endif

   if (!bServer)
      {
      cEchoServerAddress.setHostName(UString(ppcArgV[iCurrentArg]), bUseIPv6);

      iCurrentArg++;
      }

   char* pcEndNumber;

   iServerPortNumber = strtol((const char *)ppcArgV[iCurrentArg], &pcEndNumber, 10);

   iCurrentArg++;

   if (!bServer)
      {
      pcEchoString = ppcArgV[iCurrentArg];

      UString hostname(    ppcArgV[++iCurrentArg]);
      const char* device = ppcArgV[++iCurrentArg];

      UIPAddress* paddr;
      UVector<UIPAddress*> vaddr;

      U_NEW(UIPAddress, paddr, UIPAddress);

      (void) paddr->setHostName(hostname);

      vaddr.push(paddr);

      UPing sockp(2000, bUseIPv6);

#  ifdef HAVE_NETPACKET_PACKET_H
      if (device) sockp.initArpPing(device);
      else
#  endif
      (void) sockp.initPing();

      fd_set* mask = sockp.ping(vaddr, false, device);

      U_INTERNAL_DUMP("ping(%s) = %b", hostname.data(), FD_ISSET(0, mask))
      }

   if (bServer)
      {
      if (bUseTCP)
         {
         TCPEchoServer(iServerPortNumber, bUseIPv6);
         }
      else
         {
         UDPEchoServer(iServerPortNumber, bUseIPv6);
         }
      }
   else
      {
      if (bUseTCP)
         {
         TCPEchoClient(cEchoServerAddress, iServerPortNumber, pcEchoString, bUseIPv6);
         }
      else
         {
         UDPEchoClient(cEchoServerAddress, iServerPortNumber, pcEchoString, bUseIPv6);
         }
      }

#ifdef USE_C_ARES
   USocketExt::waitResolv();

   printf("Found address google.com: %s\n", USocketExt::endResolv());
#endif
}
