// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ipaddress.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_IPADDRES_H
#define ULIB_IPADDRES_H 1

#include <ulib/string.h>

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#endif

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN  16
#endif
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

#ifndef ENABLE_IPV6
#define U_INET_ADDRSTRLEN INET_ADDRSTRLEN
#else
#define U_INET_ADDRSTRLEN INET6_ADDRSTRLEN
#endif

union uusockaddr {
   struct sockaddr     psaGeneric;
   struct sockaddr_in  psaIP4Addr;
#ifdef ENABLE_IPV6
   struct sockaddr_in6 psaIP6Addr;
#endif
};

class UHTTP;
class USocket;
class USocketExt;
class USSLSocket;
class UNoCatPlugIn;
class UClient_Base;
class UServer_Base;
class URDBClientImage;
class UClientImage_Base;

template <class T> class UVector;

// Simple IP-based access-control system
// Interpret a "HOST/BITS" IP mask specification. (Ex. 192.168.1.64/28)

class U_EXPORT UIPAllow {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString spec, device, host;

   UIPAllow()
      {
      U_TRACE_REGISTER_OBJECT(0, UIPAllow, "", 0)

      mask = addr = network = 0;
      bnot = false;
      }

   ~UIPAllow()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UIPAllow)
      }

   UIPAllow& operator=(const UIPAllow& a)
      {
      U_TRACE(0, "UIPAllow::operator=(%p)", &a)

      U_MEMORY_TEST_COPY(a)

      device  = a.device;
      host    = a.host;
      addr    = a.addr;
      mask    = a.mask;
      network = a.network;
      bnot    = a.bnot;

      return *this;
      }

   // Interpret a "HOST/BITS" IP mask specification. (Ex. 192.168.1.64/28)

          bool     parseMask(const UString&  spec);
   static uint32_t parseMask(const UString& vspec, UVector<UIPAllow*>& vipallow);

   // Check whether a ip address client ought to be allowed (belong to the same network)...

   bool isAllowed(const char*    ip_client);
   bool isAllowed(const UString& ip_client);

   bool isAllowed(in_addr_t client) __pure;
   bool isAllowed(in_addr_t ifa_addr, in_addr_t ifa_netmask) __pure;

   // SERVICES

   bool isEmpty()
      {
      U_TRACE_NO_PARAM(0, "UIPAllow::isEmpty()")

      if (device.empty() ||
           host.empty())
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool getNetworkInterface(UVector<UIPAllow*>& vipallow);

   static uint32_t find(in_addr_t         client, UVector<UIPAllow*>& vipallow) __pure;
   static uint32_t find(const char*    ip_client, UVector<UIPAllow*>& vipallow) __pure;
   static uint32_t find(const UString& ip_client, UVector<UIPAllow*>& vipallow) __pure;

   static bool isAllowed(in_addr_t         client, UVector<UIPAllow*>& vipallow) { return (find(   client, vipallow) != U_NOT_FOUND); }
   static bool isAllowed(const char*    ip_client, UVector<UIPAllow*>& vipallow) { return (find(ip_client, vipallow) != U_NOT_FOUND); }
   static bool isAllowed(const UString& ip_client, UVector<UIPAllow*>& vipallow) { return (find(ip_client, vipallow) != U_NOT_FOUND); }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   in_addr_t addr, mask, network;
   bool bnot;
};

/****************************************************************************/
/* This class is used to provide transparent IPv4 and IPv6 address support  */
/* for the USocket classes within the library. The constructor creates a    */
/* default IPv4 address to localhost. The address can be assigned and parts */
/* of the address can be extracted for usage. This class is intended to be  */
/* used in conjunction with the USocket classes.                            */
/****************************************************************************/

union uupcAddress {
   uint32_t i;
#ifdef ENABLE_IPV6
   struct in6_addr s;
#endif
   char p[16];
};

#define U_ipaddress_HostNameUnresolved(obj)   (obj)->UIPAddress::flag[0]
#define U_ipaddress_StrAddressUnresolved(obj) (obj)->UIPAddress::flag[1]
#define U_ipaddress_unused1(obj)              (obj)->UIPAddress::flag[2]
#define U_ipaddress_unused2(obj)              (obj)->UIPAddress::flag[3]

class U_EXPORT UIPAddress {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UIPAddress()
      {
      U_TRACE_REGISTER_OBJECT(0, UIPAddress, "", 0)

      pcStrAddress[0] = '\0';
      iAddressLength  =
         iAddressType = 0;
      }

   ~UIPAddress()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UIPAddress)
      }

   // Sets an UIPAddress by providing a host name and a boolean
   // variable to indicate whether we want an IPv6 or IPv4 address

   void setLocalHost(                             bool bIPv6 = false);
   bool setHostName(const UString& pcNewHostName, bool bIPv6 = false);

   // Sets an UIPAddress by providing a pointer to an address
   // structure of the form in_addr or in6_addr. This pointer is cast to (void*).
   // A boolean value is used to indicate if this points to an IPv6 or IPv4 address

   void setAddress(void* address, bool bIPv6 = false);

   // Returns a constant integer of the address family represented by the UIPAddress

   u_short getAddressFamily() const { return iAddressType; }

   // Returns if it belongs to a private (non-routable) IP address space (RFC 1918) 

   static bool isPrivate(uint32_t i)
      {
      U_TRACE(0, "UIPAddress::isPrivate(0x%X)", i)

      if (((i >= 0x0A000000) && (i <= 0x0AFFFFFF)) ||
          ((i >= 0xAC100000) && (i <= 0xAC1FFFFF)) ||
          ((i >= 0xC0A80000) && (i <= 0xC0A8FFFF)))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isPrivate() __pure;
   bool isWildCard() __pure;

   static UString toString(uint8_t* paddr);
   static UString toString(in_addr_t addr) { return toString((uint8_t*)&addr); } 

#ifdef ENABLE_IPV6
   /********************************************************************************/
   /* This method converts the IPAddress instance to the specified type - either   */
   /* AF_INET or AF_INET6. If the address family is already of the specified       */
   /* type, then no changes are made. The following steps are for converting to:   */
   /*                                                                              */
   /* IPv4: If the existing IPv6 address is not an IPv4 Mapped IPv6 address the    */
   /*       conversion cannot take place. Otherwise,                               */
   /*       the last 32 bits of the IPv6 address form the IPv4 address and we      */
   /*       call setAddress() to set the address to these four bytes.              */
   /*                                                                              */
   /* IPv6: The 32 bits of the IPv4 address are copied to the last 32 bits of      */
   /*       the 128-bit IPv address.  This is then prepended with 80 zero bits     */
   /*       and 16 one bits to form an IPv4 Mapped IPv6 address.                   */
   /*                                                                              */
   /* Finally, the new address family is set along with both lazy evaluation flags */
   /********************************************************************************/

   void convertToAddressFamily(int iNewAddressFamily);
#endif

   int getInAddrLength() const { return iAddressLength; }

   // Returns a (void*) to the address represented by UIPAddress.
   // This must be cast to (in_addr*) or to (in6_addr*) for use

   void* get_in_addr() const { return (void*) (pcAddress.p + (iAddressType == AF_INET6 ? 12 : 0)); }

   // Returns the address represented by UIPAddress

   in_addr_t getInAddr() const
      {
      union uuaddr {
         void*      generic;
         in_addr_t* addr;
      };

      union uuaddr u = { get_in_addr() };

      return *(u.addr);
      }

   // Returns a string of the hostname of the represented IP Address

   UString& getHostName() { resolveHostName(); return strHostName; }

   // Returns a constant string pointer to the string
   // representation of the IP Address suitable for visual presentation

   const char* getAddressString() { if (U_ipaddress_StrAddressUnresolved(this)) resolveStrAddress(); return pcStrAddress; }

   // Check equality with an existing UIPAddress object

   bool operator==(const UIPAddress& cOtherAddr) const
      {
      U_TRACE(0, "UIPAddress::operator==(%p)", &cOtherAddr)

      if (iAddressType   == cOtherAddr.iAddressType   &&
          iAddressLength == cOtherAddr.iAddressLength &&
          (memcmp(pcAddress.p, cOtherAddr.pcAddress.p, iAddressLength) == 0))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool operator!=(const UIPAddress& cOtherAddr) const { return !operator==(cOtherAddr); }

   void set(const UIPAddress& cOtherAddr);

   UIPAddress(const UIPAddress& cOtherAddr)
      {
      U_TRACE_REGISTER_OBJECT(0, UIPAddress, "%p", &cOtherAddr)

      U_MEMORY_TEST_COPY(cOtherAddr)

      set(cOtherAddr);
      }

   UIPAddress& operator=(const UIPAddress& cOtherAddr)
      {
      U_TRACE(1, "UIPAddress::operator=(%p)", &cOtherAddr)

      U_MEMORY_TEST_COPY(cOtherAddr)

      set(cOtherAddr);

      return *this;
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString strHostName;
   int iAddressLength, iAddressType;
   unsigned char flag[4];
   union uupcAddress pcAddress;
   char pcStrAddress[U_INET_ADDRSTRLEN];

   void resolveHostName();
   void resolveStrAddress();

   static char* resolveStrAddress(int iAddressType, const void* addr, char* ip);

   /****************************************************************************/
   /* This method is used to set the contents of the iAddressLength and        */
   /* pcAddress member variables. Address Length bytes are copied from the     */
   /* source address to the pcAddress array. This array is 16 bytes long, long */
   /* enough to hold both IPv4 and IPv6 addresses                              */
   /****************************************************************************/

   void setAddress(const char* pcNewAddress, int iNewAddressLength);

private:
   friend class UHTTP;
   friend class USocket;
   friend class USocketExt;
   friend class USSLSocket;
   friend class UNoCatPlugIn;
   friend class UClient_Base;
   friend class UServer_Base;
   friend class URDBClientImage;
   friend class UClientImage_Base;
};

#endif
