// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ping.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/process.h>
#include <ulib/timeval.h>
#include <ulib/net/ping.h>
#include <ulib/notifier.h>
#include <ulib/utility/services.h>
#include <ulib/utility/interrupt.h>
#include <ulib/utility/socket_ext.h>

#ifdef _MSWINDOWS_
#  define ICMP_ECHO       8  /* Echo Request */
#  define ICMP_ECHOREPLY  0  /* Echo Reply   */
#else
#  include <netinet/ip.h>
#  include <netinet/in.h>
#  include <netinet/ip_icmp.h>
#  ifdef HAVE_NETPACKET_PACKET_H
#     include <net/if.h>
#     include <sys/ioctl.h>
#     include <arpa/inet.h>
#     include <net/if_arp.h>
#     include <netinet/ether.h>
#  endif
#endif

#ifndef HAVE_IPHDR
typedef struct iphdr {
   uint8_t  ip_vhl:4;   /* Length of the header in dwords */
   uint8_t  version:4;  /* Version of IP                  */
   uint8_t  tos;        /* Type of service                */
   uint16_t total_len;  /* Length of the packet in dwords */
   uint16_t ident;      /* unique identifier              */
   uint16_t flags;      /* Flags                          */
   uint8_t  ip_ttl;     /* Time to live                   */
   uint8_t  protocol;   /* Protocol number (TCP, UDP etc) */
   uint16_t checksum;   /* IP checksum                    */
   uint32_t source_ip, dest_ip;
   /* The options start here */
} iphdr;
#endif

/*
#ifndef HAVE_SOCKADDR_LL

// The sockaddr_ll is a device independent physical layer address

typedef struct sockaddr_ll {
   uint16_t sll_family;    // Always AF_PACKET
   uint16_t sll_protocol;  // Physical layer protocol
   int      sll_ifindex;   // Interface number
   uint16_t sll_hatype;    // Header type
   uint8_t  sll_pkttype;   // Packet type
   uint8_t  sll_halen;     // Length of address
   uint8_t  sll_addr[8];   // Physical layer address
};

// sll_protocol is the standard ethernet protocol type in network order as defined in the <linux/if_ether.h> include file.
// It defaults to the socket's protocol.
//
// sll_ifindex is the interface index of the interface (see netdevice(7)); 0 matches any interface (only legal for binding).
//
// sll_hatype is a ARP type as defined in the <linux/if_arp.h> include file.
//
// sll_pkttype contains the packet type.
//    Valid types are PACKET_HOST      for a packet addressed to the local host,
//                    PACKET_BROADCAST for a physical layer broadcast packet,
//                    PACKET_MULTICAST for a packet sent to a physical layer multicast address,
//                    PACKET_OTHERHOST for a packet to some other host that has been caught by a device driver in promiscuous mode,
//                and PACKET_OUTGOING  for a packet originated from the local host that is looped back to a packet socket.
//    These types make only sense for receiving.
//
// sll_addr and sll_halen contain the physical layer (e.g. IEEE 802.3) address and its length. The exact interpretation depends on the device.
//
// When you send packets it is enough to specify
// sll_family,
// sll_ifindex.
// sll_halen,
// sll_addr,
// The other fields should be 0.
//
// sll_hatype and sll_pkttype are set on received packets for your information.
// For bind only sll_protocol and sll_ifindex are used.

#endif
*/

/* See RFC 826 for protocol description. ARP packets are variable in size; the arphdr structure defines the fixed-length portion.
 * Protocol type values are the same as those for 10 Mb/s Ethernet. It is followed by the variable-sized fields ar_sha, arp_spa,
 * arp_tha and arp_tpa in that order, according to the lengths specified. Field names used correspond to RFC 826.

#ifndef HAVE_ARPHDR
typedef struct arphdr {
   uint16_t ar_hrd;              // Format of hardware address
   uint16_t ar_pro;              // Format of protocol address
   uint8_t  ar_hln;              // Length of hardware address
   uint8_t  ar_pln;              // Length of protocol address
   uint16_t ar_op;               // ARP opcode (command)
// ----------------------------------------------------------------
// Ethernet looks like this : This bit is variable sized however...
// ETH_ALEN == 6 (Octets in one ethernet addr)
// ----------------------------------------------------------------
// uint8_t __ar_sha[ETH_ALEN];   // Sender hardware address
// uint8_t __ar_sip[4];          // Sender IP address
// uint8_t __ar_tha[ETH_ALEN];   // Target hardware address
// uint8_t __ar_tip[4];          // Target IP address
// ----------------------------------------------------------------
} arphdr;
#endif
*/

#ifndef ETH_P_IP
#define ETH_P_IP  0x0800 /* Internet Protocol packet */
#endif
#ifndef ETH_P_ARP
#define ETH_P_ARP 0x0806 /* Address Resolution packet */
#endif

#ifndef ICMP_ECHOREPLY
#define ICMP_ECHOREPLY 0
#endif
#ifndef ICMP_ECHO
#define ICMP_ECHO      8
#endif

int       UPing::timeoutMS;
fd_set*   UPing::addrmask;
uint32_t  UPing::map_size;
UProcess* UPing::proc;

UPing::UPing(int _timeoutMS, bool bSocketIsIPv6) : USocket(bSocketIsIPv6)
{
   U_TRACE_REGISTER_OBJECT(0, UPing, "%d,%b", _timeoutMS, bSocketIsIPv6)

   rep       = 0;
   timeoutMS = _timeoutMS;

   (void) memset(&req, 0, sizeof(reqhdr));
#ifdef HAVE_NETPACKET_PACKET_H
   (void) memset(&arp, 0, sizeof(arpmsg));
#endif

   if (proc     == 0) U_NEW(UProcess, proc, UProcess);
   if (addrmask == 0)
      {
      map_size = sizeof(fd_set) + sizeof(uint32_t);
      addrmask = (fd_set*) UFile::mmap(&map_size);
      }
}

UPing::~UPing()
{
   U_TRACE_UNREGISTER_OBJECT(0, UPing)

   if (proc)
      {
      delete proc;
             proc= 0;
      }

   if (addrmask &&
       map_size)
      {
      UFile::munmap(addrmask, map_size);
                    addrmask = 0;
      }
}

// Checksum (16 bits), calculated with the ICMP part of the packet (the IP header is not used)

inline void UPing::cksum(void* hdr, int len)
{
   U_TRACE(0, "UPing::cksum(%p,%d)", hdr, len)

   int i = 0, b1 = 0, b2 = 0;
   uint8_t* cb = (uint8_t*)hdr;

   ((icmphdr*)hdr)->checksum = 0;

   for (; i < (len & ~1); i += 2)
      {
      b1 += cb[i];
      b2 += cb[i + 1];
      }

   if (i & 1) b1 += cb[len - 1];

   while (true)
      {
      if (b1 >= 256)
         {
         b2 += b1 >> 8;
         b1 &= 0xff;

         continue;
         }

      if (b2 >= 256)
         {
         b1 += b2 >> 8;
         b2 &= 0xff;

         continue;
         }

      break;
      }

   cb    = (uint8_t*)&(((icmphdr*)hdr)->checksum);
   cb[0] = ~(uint8_t)b1;
   cb[1] = ~(uint8_t)b2;

   U_INTERNAL_DUMP("b1 = %ld b2 = %ld", b1, b2)
}

// This method is called to test whether a particular host is reachable across an IP network; it is also used to self test the network interface card
// of the computer, or as a latency test. It works by sending ICMP echo request packets to the target host and listening for ICMP echo response replies.
// Note that ICMP (and therefore ping) resides on the Network layer (level 3) of the OSI (Open Systems Interconnection) model. This is the same layer as
// IP (Internet Protocol). Consequently, ping does not use a port for communication

bool UPing::initPing()
{
   U_TRACE_NO_PARAM(0, "UPing::initPing()")

   USocket::_socket(SOCK_RAW, 0, IPPROTO_ICMP);

   if (USocket::isOpen())
      {
      (void) USocket::setTimeoutSND(timeoutMS);
      (void) USocket::setTimeoutRCV(timeoutMS);

      U_RETURN(true);
      }

   U_WARNING("Sorry, I could not create raw socket%s", UServices::isSetuidRoot() ? "" : " - you must run as root to create raw socket...");

   U_RETURN(false);
}

bool UPing::ping(UIPAddress& addr)
{
   U_TRACE(0, "UPing::ping(%p)", &addr)

   U_CHECK_MEMORY

   if (USocket::isOpen() ||
       initPing())
      {
      union uuiphdr {
         struct iphdr* ph;
         unsigned char* pc;
      };

      union uuiphdr u;
      uint16_t seq = 0;
      int iphdrlen, ret;
      UIPAddress cResponseIP;
      unsigned char buf[4096];
      unsigned int iSourcePortNumber;

      (void) U_SYSCALL(memset, "%p,%d,%u", &req, 0, sizeof(req));

      req.id   = htons((short)u_pid); //           identifier (16 bits)
      req.type = ICMP_ECHO;           // Type of ICMP message ( 8 bits) -> 8

      for (int i = 0; i < 3; ++i)
         {
         req.seq = htons(++seq);

         // Checksum (16 bits), calculated with the ICMP part of the packet (the IP header is not used)

         cksum(&req, sizeof(req));

         U_INTERNAL_DUMP("id = %hd seq = %hd", ntohs(req.id), ntohs(req.seq))

         // send icmp echo request

         ret = USocket::sendTo((void*)&req, sizeof(req), 0, addr, 0);

         if (ret <= 0) U_RETURN(false);

loop:    ret = USocket::recvFrom(buf, sizeof(buf), 0, cResponseIP, iSourcePortNumber); // wait for response

         if (ret <= 0)
            {
            if (errno == EAGAIN) continue;

            U_RETURN(false);
            }

         if (cResponseIP != addr) goto loop;

         if (U_socket_IPv6(this))
            {
            if (ret < (int)sizeof(rephdr)) goto loop; 

            iphdrlen = 0;
            }
         else
            {
            if (ret < (int)(sizeof(struct iphdr) + sizeof(rephdr) - 4)) goto loop;

            iphdrlen = sizeof(struct iphdr);

            u.pc = buf;

            U_INTERNAL_DUMP("protocol = %d", u.ph->protocol)

            if (u.ph->protocol != IPPROTO_ICMP) goto loop; // IPPROTO_ICMP -> 1
            }

         rep = (rephdr*)(buf + iphdrlen);

         U_INTERNAL_DUMP("iphdrlen = %d type = %d id = %hd seq = %hd", iphdrlen, rep->type, ntohs(rep->id), ntohs(rep->seq))

         if ((rep->id   != req.id)  ||
             (rep->seq  != req.seq) ||
             (rep->type != ICMP_ECHOREPLY)) // ICMP_ECHOREPLY -> 0
            {
            goto loop;
            }

         U_INTERNAL_DUMP("TTL = %d", ntohl(rep->ttl))

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

// parallel PING/ARPING

#define SHM_counter (*(uint32_t*)(((char*)addrmask)+sizeof(fd_set)))

fd_set* UPing::pingAsyncCompletion()
{
   U_TRACE_NO_PARAM(0, "UPing::pingAsyncCompletion()")

   U_INTERNAL_ASSERT_POINTER(proc)

   proc->waitAll();

   U_INTERNAL_DUMP("SHM_counter = %u addrmask = %B", SHM_counter, __FDS_BITS(addrmask)[0])

   U_RETURN_POINTER(addrmask, fd_set);
}

fd_set* UPing::checkForPingAsyncCompletion(uint32_t nfds)
{
   U_TRACE(0, "UPing::checkForPingAsyncCompletion(%u)", nfds)

   if (nfds &&
       SHM_counter < nfds)
      {
      UTimeVal::nanosleep(1L);

      U_INTERNAL_DUMP("SHM_counter = %u addrmask = %B", SHM_counter, __FDS_BITS(addrmask)[0])

      // check if pending...

      if (SHM_counter < nfds) U_RETURN_POINTER(0, fd_set);
      }

   return pingAsyncCompletion();
}

void UPing::pingAsync(uint32_t nfd, UIPAddress* paddr, const char* device)
{
   U_TRACE(0, "UPing::pingAsync(%u,%p,%S)", nfd, paddr, device)

   if (proc->fork() &&
       proc->child())
      {
#  ifdef HAVE_NETPACKET_PACKET_H
      bool bresp = (device ? arping(*paddr, device) : ping(*paddr));
#  else
      bool bresp = (device ? false                  : ping(*paddr));
#  endif

      U_INTERNAL_DUMP("bresp = %b", bresp)

      if (bresp) FD_SET(nfd, addrmask);

      SHM_counter++;

      U_EXIT(0);
      }
}

fd_set* UPing::ping(UVector<UIPAddress*>& vaddr, bool async, const char* device)
{
   U_TRACE(0, "UPing::ping(%p,%b,%S)", &vaddr, async, device)

   SHM_counter = 0;
   FD_ZERO(addrmask);

   U_INTERNAL_DUMP("SHM_counter = %d addrmask = %B", SHM_counter, __FDS_BITS(addrmask)[0])

   if (USocket::isClosed())
      {
#  ifdef HAVE_NETPACKET_PACKET_H
      if (device) initArpPing(device);
      else
#  endif
      if (initPing() == false) U_RETURN_POINTER(addrmask, fd_set);
      }

   uint32_t n = vaddr.size();

   for (uint32_t i = 0; i < n; ++i) pingAsync(i, vaddr[i], device);

   return checkForPingAsyncCompletion(async ? n : 0);
}

#ifdef HAVE_NETPACKET_PACKET_H
int UPing::recvArpPing()
{
   U_TRACE_NO_PARAM(0, "UPing::recvArpPing()")

   int ret;
   UPing::arpmsg reply;

   (void) U_SYSCALL(memset, "%p,%d,%u", &reply, '\0', sizeof(UPing::arpmsg));

#ifndef U_ARP_WITH_BROADCAST
   union uuarphdr {
      unsigned char* pc;
      struct arphdr* ph;
   };

   socklen_t alen;
   union uuarphdr ah;
   union uusockaddr_ll from;

   ah.pc = (unsigned char*)&(reply.htype);
#endif

loop: // wait for ARP reply

   if (UNotifier::waitForRead(USocket::iSockDesc, timeoutMS) != 1) U_RETURN(2);

#ifdef U_ARP_WITH_BROADCAST
   ret = U_SYSCALL(recv,      "%d,%p,%d,%u,%p,%p", USocket::iSockDesc, &reply, sizeof(reply),    0);
#else
   alen = sizeof(struct sockaddr);
   ret  = U_SYSCALL(recvfrom, "%d,%p,%d,%u,%p,%p", USocket::iSockDesc,  ah.pc, sizeof(reply)-14, 0, &(from.s), &alen);
#endif

   if (ret <= 0)
      {
      if (USocket::checkErrno()) U_RETURN(2); // TIMEOUT

      U_RETURN(0);
      }

   if (ret < 42                              && // ARP_MSG_SIZE
       reply.operation != htons(ARPOP_REPLY) &&
       reply.operation != htons(ARPOP_REQUEST))
      {
      goto loop;
      }

#ifndef U_ARP_WITH_BROADCAST
   // Filter out wild packets

   U_INTERNAL_DUMP("sll_pkttype = %u", from.l.sll_pkttype)

   if (from.l.sll_pkttype != PACKET_HOST      &&
       from.l.sll_pkttype != PACKET_BROADCAST &&
       from.l.sll_pkttype != PACKET_MULTICAST)
      {
      goto loop;
      }

   // Protocol must be IP

   if (ah.ph->ar_pro != htons(ETH_P_IP) ||
       ah.ph->ar_pln != 4               ||
       ah.ph->ar_hln != 6               ||
       (ret < (int)(sizeof(ah) + 2 * (4 + 6))))
      {
      goto loop;
      }
#endif

   U_DUMP("ARP re%s from %s (%02x:%02x:%02x:%02x:%02x:%02x) to %s (%02x:%02x:%02x:%02x:%02x:%02x)",
           (reply.operation == htons(ARPOP_REPLY) ? "ply" : "quest"),
            UIPAddress::toString(reply.sInaddr).data(),
            reply.sHaddr[0] & 0xFF, reply.sHaddr[1] & 0xFF, reply.sHaddr[2] & 0xFF,
            reply.sHaddr[3] & 0xFF, reply.sHaddr[4] & 0xFF, reply.sHaddr[5] & 0xFF,
            UIPAddress::toString(reply.tInaddr).data(),
            reply.tHaddr[0] & 0xFF, reply.tHaddr[1] & 0xFF, reply.tHaddr[2] & 0xFF,
            reply.tHaddr[3] & 0xFF, reply.tHaddr[4] & 0xFF, reply.tHaddr[5] & 0xFF)

   if (memcmp(reply.sInaddr, arp.tInaddr, 4) ||
       memcmp(reply.tHaddr,  arp.sHaddr,  6)) // don't check tHaddr: Linux doesn't return proper target's hardware address (fixed in 2.6.24?)
      {
      goto loop;
      }

   U_RETURN(1);
}

void UPing::initArpPing(const char* device)
{
   U_TRACE(0, "UPing::initArpPing(%S)", device)

#ifdef U_ARP_WITH_BROADCAST
   USocket::iSockDesc = USocket::socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
#else
   USocket::iSockDesc = USocket::socket(PF_PACKET, SOCK_DGRAM,                 0);
#endif

   if (USocket::isOpen())
      {
#  ifdef U_ARP_WITH_BROADCAST
      uint32_t value = 1;

      if (USocket::setSockOpt(SOL_SOCKET, SO_BROADCAST, (const void*)&value) == false) U_ERROR("Can't enable bcast on packet socket");
#  endif

      struct ifreq ifr;

      (void) U_SYSCALL(memset, "%p,%d,%u", &arp, '\0', sizeof(arp));
      (void) U_SYSCALL(memset, "%p,%d,%u", &ifr, '\0', sizeof(struct ifreq));

      (void) u__strncpy(ifr.ifr_name, device, IFNAMSIZ-1);

      if (U_SYSCALL(ioctl, "%d,%d,%p", USocket::iSockDesc, SIOCGIFINDEX, (char*)&ifr) == -1) U_ERROR("Unknown iface for interface %S", device);

      U_INTERNAL_DUMP("ifr_ifindex = %u", ifr.ifr_ifindex)

#  ifdef U_ARP_WITH_BROADCAST
      arp.h_proto = htons(ETH_P_ARP); // protocol type (Ethernet)
#  else
      (void) U_SYSCALL(memset, "%p,%d,%u", &he, '\0', sizeof(he));

      he.l.sll_protocol = htons(ETH_P_ARP); // protocol type (Ethernet)
      he.l.sll_family   = AF_PACKET;
      he.l.sll_ifindex  = ifr.ifr_ifindex;

      socklen_t alen = sizeof(struct sockaddr_ll);

      U_INTERNAL_DUMP("alen = %u", alen)

      if (U_SYSCALL(bind,        "%d,%p,%d", USocket::iSockDesc, &(he.s),  alen) == -1) U_ERROR_SYSCALL("bind");
      if (U_SYSCALL(getsockname, "%d,%p,%p", USocket::iSockDesc, &(he.s), &alen) == -1) U_ERROR_SYSCALL("getsockname");

      U_INTERNAL_DUMP("alen = %d he.sll_halen = %u", alen, he.l.sll_halen) // 6

      if (he.l.sll_halen == 0) U_ERROR("Interface '%s' is not ARPable (no ll address)", device);

      U_MEMCPY(arp.sHaddr, he.l.sll_addr, 6); // source hardware address

      (void) U_SYSCALL(memset, "%p,%d,%u", he.l.sll_addr, 0xff, 6);
#  endif

      if (U_SYSCALL(ioctl, "%d,%d,%p", USocket::iSockDesc, SIOCGIFFLAGS, (char*)&ifr) == -1) U_ERROR("ioctl(SIOCGIFFLAGS) failed for interface %S", device);

      if (!(ifr.ifr_flags &  IFF_UP))                                                        U_ERROR("Interface %S is down", device);
      if (  ifr.ifr_flags & (IFF_NOARP | IFF_LOOPBACK))                                      U_ERROR("Interface %S is not ARPable", device);

      if (U_SYSCALL(ioctl, "%d,%d,%p", USocket::iSockDesc, SIOCGIFADDR, (char*)&ifr)  == -1) U_ERROR("ioctl(SIOCGIFADDR) failed for interface %S", device);

      U_MEMCPY(arp.sInaddr, ifr.ifr_addr.sa_data + sizeof(short), 4); // source IP address

      // struct arphdr {
      //  unsigned short int ar_hrd; // Format of hardware address
      //  unsigned short int ar_pro; // Format of protocol address
      //  unsigned char ar_hln;      // Length of hardware address
      //  unsigned char ar_pln;      // Length of protocol address
      //  unsigned short int ar_op;  // ARP opcode (command) };

      arp.htype     = htons(ARPHRD_ETHER);   // hardware type
      arp.ptype     = htons(ETH_P_IP);       // protocol type (ARP message)
      arp.hlen      = 6;                     // hardware address length
      arp.plen      = 4;                     // protocol address length
      arp.operation = htons(ARPOP_REQUEST);  // ARP op code

      U_INTERNAL_DUMP("ar_op = %u", arp.operation)

#  ifdef U_ARP_WITH_BROADCAST
      if (U_SYSCALL(ioctl, "%d,%d,%p", USocket::iSockDesc, SIOCGIFHWADDR, (char*)&ifr) == -1)
         {
         U_ERROR("ioctl(SIOCGIFHWADDR) failed for interface %S", device);
         }

      (void) U_SYSCALL(memset, "%p,%d,%u",arp.h_dest, 0xff, 6);                     // MAC DA
                                 U_MEMCPY(arp.h_source, ifr.ifr_hwaddr.sa_data, 6); // MAC SA
                                 U_MEMCPY(arp.sHaddr,   ifr.ifr_hwaddr.sa_data, 6); // source hardware address
#  endif

   // --------------------------------------------------------------------------------------------
   // Target address - TODO...
   // --------------------------------------------------------------------------------------------
   //          arp.tHaddr is zero-filled            // target hardware address
   // U_MEMCPY(arp.tInaddr, addr.get_in_addr(), 4); // target IP address
   // --------------------------------------------------------------------------------------------

      U_DUMP("SOURCE = %s (%02x:%02x:%02x:%02x:%02x:%02x)",
                UIPAddress::toString(arp.sInaddr).data(),
                arp.sHaddr[0] & 0xFF,
                arp.sHaddr[1] & 0xFF,
                arp.sHaddr[2] & 0xFF,
                arp.sHaddr[3] & 0xFF,
                arp.sHaddr[4] & 0xFF,
                arp.sHaddr[5] & 0xFF)

      (void) USocket::setTimeoutSND(timeoutMS);
      (void) USocket::setTimeoutRCV(timeoutMS);
      }
   else
      {
      const char* msg = (UServices::isSetuidRoot()
                              ? "Sorry, could not create packet socket"
                              : "Must be run as root to create packet socket, exiting");

      U_ERROR("%s", msg);
      }
}

bool UPing::arping(UIPAddress& addr, const char* device)
{
   U_TRACE(0, "UPing::arping(%p,%S)", &addr, device)

   U_CHECK_MEMORY

   int i, ret;
   bool restarted = false;

   if (USocket::isClosed())
      {
retry:
      initArpPing(device);
      }

   // -----------------------------------------------------------------------------------------------------------------------
   // Target address
   // -----------------------------------------------------------------------------------------------------------------------
   //       arp.tHaddr is zero-filled            // target hardware address
   U_MEMCPY(arp.tInaddr, addr.get_in_addr(), 4); // target IP address
   // -----------------------------------------------------------------------------------------------------------------------

#ifdef U_ARP_WITH_BROADCAST
   struct sockaddr _saddr;

   (void) U_SYSCALL(memset, "%p,%d,%u", &_saddr, '\0', sizeof(struct sockaddr));
   (void) u__strncpy(_saddr.sa_data, device, sizeof(_saddr.sa_data));

   size_t len             = sizeof(arp);
   socklen_t alen         = sizeof(struct sockaddr);
   const void* buf        = &arp;
   struct sockaddr* saddr = &_saddr;
#else
   size_t len             = 28;
   socklen_t alen         = sizeof(struct sockaddr_ll);
   const void* buf        = &(arp.htype);
   struct sockaddr* saddr = &(he.s);
#endif

   U_DUMP("ARP request from %s (%02x:%02x:%02x:%02x:%02x:%02x) to %s (%02x:%02x:%02x:%02x:%02x:%02x)",
            UIPAddress::toString(arp.sInaddr).data(),
            arp.sHaddr[0] & 0xFF, arp.sHaddr[1] & 0xFF, arp.sHaddr[2] & 0xFF,
            arp.sHaddr[3] & 0xFF, arp.sHaddr[4] & 0xFF, arp.sHaddr[5] & 0xFF,
            UIPAddress::toString(arp.tInaddr).data(),
            arp.tHaddr[0] & 0xFF, arp.tHaddr[1] & 0xFF, arp.tHaddr[2] & 0xFF,
            arp.tHaddr[3] & 0xFF, arp.tHaddr[4] & 0xFF, arp.tHaddr[5] & 0xFF)

   for (i = 0; i < 3; ++i)
      {
      ret = U_SYSCALL(sendto, "%d,%p,%d,%u,%p,%d", USocket::iSockDesc, buf, len, 0, saddr, alen);

      if (USocket::checkIO(ret) == false) U_RETURN(false);

      ret = recvArpPing();

      if (ret == 2) continue;
      if (ret == 0) U_RETURN(false);

      if (i         == 0 &&
          restarted == false)
         {
         USocket::close();

         restarted = true;

         goto retry;
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

fd_set* UPing::arping(UPing** sockp, UVector<UIPAddress*>** vaddr, uint32_t n, bool async, UVector<UString>& vdev)
{
   U_TRACE(0, "UPing::arping(%p,%p,%u,%b,%p)", sockp, vaddr, n, async, &vdev)

   SHM_counter = 0;
   FD_ZERO(addrmask);

   uint32_t j, nfds = 0;

   for (uint32_t i = 0; i < n; ++i)
      {
      uint32_t k = vaddr[i]->size();

      for (j = 0; j < k; ++j) sockp[i]->pingAsync(nfds++, vaddr[i]->at(j), vdev[i].data());
      }

   U_INTERNAL_DUMP("nfds = %u", nfds)

   return checkForPingAsyncCompletion(async ? nfds : 0);
}

#endif

fd_set* UPing::checkARPCache(UVector<UString>& varp_cache, UVector<UIPAddress*>** vaddr, uint32_t n)
{
   U_TRACE(0, "UPing::checkARPCache(%p,%p,%u)", &varp_cache, vaddr, n)

   SHM_counter = 0;
   FD_ZERO(addrmask);

   UString ip;
   UIPAddress* paddr;

   for (uint32_t i = 0; i < n; ++i)
      {
      uint32_t k = vaddr[i]->size();

      for (uint32_t j = 0; j < k; ++j)
         {
         paddr = vaddr[i]->at(j);
         ip    = paddr->getAddressString();

         U_INTERNAL_DUMP("ip = %V", ip.rep)

         if (varp_cache.find(ip) != U_NOT_FOUND) FD_SET(SHM_counter, addrmask);

         SHM_counter++;
         }
      }

   U_INTERNAL_DUMP("SHM_counter = %u addrmask = %B", SHM_counter, __FDS_BITS(addrmask)[0])

   U_RETURN_POINTER(addrmask, fd_set);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UPing::dump(bool reset) const
{
   USocket::dump(false);

   *UObjectIO::os << '\n'
                  << "rep                           " << rep            << '\n'
                  << "timeoutMS                     " << timeoutMS      << '\n'
                  << "proc            (UProcess     " << (void*)proc    << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
