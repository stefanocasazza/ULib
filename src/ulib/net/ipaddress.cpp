// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ipaddress.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/ipaddress.h>
#include <ulib/container/vector.h>

#ifdef HAVE_IFADDRS_H
#  include <ifaddrs.h>
#endif

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#endif

#include "socket_address.cpp"

/**
 * Platform specific code
 *
 * These macros allow different implementations for functionality on the
 * supported platforms.  The macros define the following tasks, details on
 * specific implementations are detailed in the specific platform support sections
 *
 * IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType)
 *
 * Resolves the IP Address details of the provided binary IP address. The macro variables are:
 * pheDetails     (struct hostent*) - Host details returned in this structure. If the name lookup is unresolved, must be set to 0
 * pcAddress      (pointer)         - Pointer to binary IP Address
 * iAddressLength (int)             - Size of binary IP Address
 * iAddressType   (int)             - Request an AF_INET or AF_INET6 lookup
 *
 * IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError)
 *
 * Resolves the IP Address details of the provided host name. The macro variables are:
 * pheDetails     (struct hostent*) - Host details returned in this structure. If the address lookup is unresolved, must be set to 0
 * pcHostName     (char*)           - String representation of the host name
 * iAddressType   (int)             - Request an AF_INET or AF_INET6 lookup
 * iError         (int)             - Error details in the operation are returned in this variable
 */

#ifdef SOLARIS
/************************************************************************/
/* IPADDR_TO_HOST is implemented using the getipnodebyaddr() function */
/* IPNAME_TO_HOST is implemented using the getipnodebyname() function */
/************************************************************************/

#  define IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType) \
{ int rc = 0; pheDetails = (struct hostent*) U_SYSCALL(getipnodebyaddr, "%p,%d,%d,%p", pcAddress, iAddressLength, iAddressType, &rc); }

#  define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError) \
{ int rc = 0; pheDetails = (struct hostent*) U_SYSCALL(getipnodebyname, "%S,%d,%d,%p", pcHostName, iAddressType, AI_DEFAULT, &rc); iError = rc; }

#else
/***********************************************************************/
/* IPADDR_TO_HOST is implemented using the gethostbyaddr() function  */
/* IPNAME_TO_HOST is implemented using the gethostbyname2() function */
/***********************************************************************/

#  define IPADDR_TO_HOST(pheDetails, pcAddress, iAddressLength, iAddressType) \
{ pheDetails = (struct hostent*) U_SYSCALL(gethostbyaddr, "%p,%d,%d", pcAddress, iAddressLength, iAddressType); }

#  if defined(ENABLE_IPV6) && !defined(_MSWINDOWS_)
#     define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError) \
         { pheDetails = (struct hostent*) U_SYSCALL(gethostbyname2, "%S,%d", pcHostName, iAddressType); iError = h_errno; }
#  else
#     define IPNAME_TO_HOST(pheDetails, pcHostName, iAddressType, iError) \
         { pheDetails = (struct hostent*) U_SYSCALL(gethostbyname, "%S", pcHostName); iError = h_errno; }
#  endif
#endif

// gcc: call is unlikely and code size would grow

void UIPAddress::setAddress(void* address, bool bIPv6)
{
   U_TRACE(1, "UIPAddress::setAddress(%p,%b)", address, bIPv6)

   U_CHECK_MEMORY

#ifdef ENABLE_IPV6
   if (bIPv6)
      {
      iAddressType   = AF_INET6;
      iAddressLength = sizeof(in6_addr);

      U_MEMCPY(pcAddress.p, address, iAddressLength);
      }
   else
#endif
      {
      iAddressType   = AF_INET;
      pcAddress.i    = *(uint32_t*)address;
      iAddressLength = sizeof(in_addr);
      }

   U_ipaddress_HostNameUnresolved(this)   =
   U_ipaddress_StrAddressUnresolved(this) = true;

// U_INTERNAL_DUMP("addr = %u", getInAddr())
}

void UIPAddress::set(const UIPAddress& cOtherAddr)
{
   U_TRACE(1, "UIPAddress::set(%p)", &cOtherAddr)

   strHostName    = cOtherAddr.strHostName;
   iAddressType   = cOtherAddr.iAddressType;
   iAddressLength = cOtherAddr.iAddressLength;

#ifdef ENABLE_IPV6
   if (iAddressType == AF_INET6)
      {
      U_INTERNAL_ASSERT_EQUALS(iAddressLength, sizeof(in6_addr))

      U_MEMCPY(pcAddress.p, cOtherAddr.pcAddress.p, sizeof(in6_addr));
      }
   else
#endif
      {
      U_INTERNAL_ASSERT_EQUALS(iAddressType, AF_INET)
      U_INTERNAL_ASSERT_EQUALS(iAddressLength, sizeof(in_addr))

      pcAddress.i = cOtherAddr.pcAddress.i;
      }

   uint32_t n = u__strlen(cOtherAddr.pcStrAddress, __PRETTY_FUNCTION__);

   if (n) U_MEMCPY(pcStrAddress, cOtherAddr.pcStrAddress, n);

   pcStrAddress[n] = '\0';

   U_ipaddress_HostNameUnresolved(this)   = U_ipaddress_HostNameUnresolved(&cOtherAddr);
   U_ipaddress_StrAddressUnresolved(this) = U_ipaddress_StrAddressUnresolved(&cOtherAddr);

   U_INTERNAL_DUMP("addr = %u strHostName = %V", getInAddr(), strHostName.rep)

   U_INTERNAL_ASSERT_EQUALS(u_isIPAddr(iAddressType == AF_INET6, U_STRING_TO_PARAM(strHostName)), false)
}

void UIPAddress::setLocalHost(bool bIPv6)
{
   U_TRACE(0, "UIPAddress::setLocalHost(%b)", bIPv6)

   U_INTERNAL_ASSERT_POINTER(UString::str_localhost)

   strHostName = *UString::str_localhost;

#ifdef ENABLE_IPV6
   if (bIPv6)
      {
      pcAddress.p[0] =
      pcAddress.p[1] = 
      pcAddress.p[2] =
      pcAddress.p[3] =
      pcAddress.p[4] =
      pcAddress.p[5] = 0;

      iAddressType   = AF_INET6;
      iAddressLength = sizeof(in6_addr);

      u__strcpy(pcStrAddress, "::1");
      }
   else
#endif
      {
      pcAddress.p[0] = 127;
      pcAddress.p[1] =
      pcAddress.p[2] = 0;
      pcAddress.p[3] = 1;

      iAddressType   = AF_INET;
      iAddressLength = sizeof(in_addr);

      u__strcpy(pcStrAddress, "127.0.0.1");
      }

   U_ipaddress_HostNameUnresolved(this)   =
   U_ipaddress_StrAddressUnresolved(this) = false;
}

void UIPAddress::setAddress(const char* pcNewAddress, int iNewAddressLength)
{
   U_TRACE(1, "UIPAddress::setAddress(%S,%d)", pcNewAddress, iNewAddressLength)

   U_CHECK_MEMORY

   iAddressLength = iNewAddressLength;

   U_MEMCPY(pcAddress.p, pcNewAddress, iAddressLength);

   U_INTERNAL_DUMP("addr = %u", getInAddr())
}

bool UIPAddress::setHostName(const UString& pcNewHostName, bool bIPv6)
{
   U_TRACE(1, "UIPAddress::setHostName(%V,%b)", pcNewHostName.rep, bIPv6)

   U_CHECK_MEMORY

   if (pcNewHostName.empty()) U_RETURN(false);

   U_INTERNAL_DUMP("strHostName = %V U_ipaddress_HostNameUnresolved = %b", strHostName.rep, U_ipaddress_HostNameUnresolved(this))

   U_INTERNAL_ASSERT_EQUALS(u_isIPAddr(bIPv6, U_STRING_TO_PARAM(strHostName)), false)

   if (strHostName.equal(pcNewHostName) &&
       U_ipaddress_HostNameUnresolved(this) == false)
      {
      U_RETURN(true);
      }

   U_INTERNAL_ASSERT(pcNewHostName.isNullTerminated())

   const char* name = pcNewHostName.data();

   if (u_isIPAddr(bIPv6, name, pcNewHostName.size()))
      {
      struct in_addr ia;

      if (inet_aton(name, &ia) == 0) U_RETURN(false);

      strHostName.clear();

      setAddress(&ia, false);

      U_ipaddress_StrAddressUnresolved(this) = false;

      U_RETURN(true);
      }

   uint32_t len;
   const char* ptr;

#ifdef HAVE_GETADDRINFO
   int gai_err;
   struct addrinfo hints;
   struct addrinfo* result = 0;

   // setup hints structure
   // -----------------------------------------------------------------
   // struct addrinfo {
   //    int ai_flags;              // Input flags
   //    int ai_family;             // Protocol family for socket
   //    int ai_socktype;           // Socket type
   //    int ai_protocol;           // Protocol for socket
   //    socklen_t ai_addrlen;      // Length of socket address
   //    struct sockaddr* ai_addr;  // Socket address for socket
   //    char* ai_canonname;        // Canonical name for service location
   //    struct addrinfo* ai_next;  // Pointer to next in list
   // };
   // -----------------------------------------------------------------

   (void) U_SYSCALL(memset, "%p,%d,%u", &hints, 0, sizeof(hints));

   hints.ai_flags    = AI_CANONNAME;
   hints.ai_family   = (bIPv6 ? AF_INET6 : AF_INET);
   hints.ai_socktype = SOCK_STREAM;

   // get our address

   gai_err = U_SYSCALL(getaddrinfo, "%S,%p,%p,%p", name, 0, &hints, &result);

   if (gai_err != 0)
      {
      U_WARNING("getaddrinfo() error on host %S: %s", name, gai_strerror(gai_err));

      if (result) U_SYSCALL_VOID(freeaddrinfo, "%p", result);

      U_RETURN(false);
      }

   // copy the address into our struct

   U_INTERNAL_DUMP("result = %p ai_family = %d ai_canonname = %S ai_addr = %p ai_addrlen = %d", result,
                    result->ai_family, result->ai_canonname, result->ai_addr, result->ai_addrlen)

   SocketAddress sockadd;

   sockadd.set(result);
   sockadd.getIPAddress(*this);

   ptr          = result->ai_canonname;
   iAddressType = result->ai_family;
#else
   int iError;
   struct hostent* pheDetails;

   // struct hostent {
   //    char*  h_name;      // official name of host
   //    char** h_aliases;   // alias list
   //    int    h_addrtype;  // host address type
   //    int    h_length;    // length of address
   //    char** h_addr_list; // list of addresses
   // };

   IPNAME_TO_HOST(pheDetails, name, (bIPv6 ? AF_INET6 : AF_INET), iError);

   if (pheDetails == NULL)
      {
      const char* msg[2];

      switch (iError)
         {
         case HOST_NOT_FOUND:
            msg[0] = "HOST_NOT_FOUND";
            msg[1] = "The specified host is unknown";
         break;

         case NO_ADDRESS:
            msg[0] = "NO_ADDRESS";
            msg[1] = "The requested name is valid but does not have an IP address";
         break;

         case NO_RECOVERY:
            msg[0] = "NO_RECOVERY";
            msg[1] = "A non-recoverable name server error occurred";
         break;

         case TRY_AGAIN:
            msg[0] = "TRY_AGAIN";
            msg[1] = "A temporary error occurred on an authoritative name server. Try again later";
         break;

         default:
            msg[0] = "???";
            msg[1] = "unknown error";
         }

      U_WARNING("IPNAME_TO_HOST(...) - %s (%d, %s)", msg[0], iError, msg[1]);

      U_RETURN(false);
      }

   ptr          = pheDetails->h_name;
   iAddressType = pheDetails->h_addrtype;

   setAddress(pheDetails->h_addr_list[0], pheDetails->h_length);
#endif

   len = u__strlen(ptr, __PRETTY_FUNCTION__);

   if (u_isIPAddr(bIPv6, ptr, len))
      {
      U_MEMCPY(pcStrAddress, ptr, len);

      pcStrAddress[len] = '\0';

      U_ipaddress_StrAddressUnresolved(this) = false;
      }

#ifdef HAVE_GETADDRINFO
   U_SYSCALL_VOID(freeaddrinfo, "%p", result);
#endif

   strHostName = pcNewHostName;

   U_ipaddress_HostNameUnresolved(this) = false;

   U_INTERNAL_DUMP("strHostName = %V", strHostName.rep)

   U_INTERNAL_ASSERT_EQUALS(u_isIPAddr(bIPv6, U_STRING_TO_PARAM(strHostName)), false)

   U_RETURN(true);
}

/******************************************************************************/
/* void resolveStrAddress()                                                   */
/*                                                                            */
/* This method is used to resolve the string representation of the ip         */
/* address stored by the class, this need be performed only if the lazy       */
/* evaluation flag is set. The flag is reset at completion of the method to   */
/* indicate that the address string has been resolved - the string is         */
/* generated via a call to inet_ntop()                                        */
/******************************************************************************/

char* UIPAddress::resolveStrAddress(int iAddressType, const void* src, char* ip)
{
   U_TRACE(1, "UIPAddress::resolveStrAddress(%d,%p,%p)", iAddressType, src, ip)

   char* result = 0;

#ifdef HAVE_INET_NTOP
   result = (char*) U_SYSCALL(inet_ntop, "%d,%p,%p,%u", iAddressType, (void*)src, ip, U_INET_ADDRSTRLEN);
#else
   result = U_SYSCALL(inet_ntoa, "%u", *((struct in_addr*)src));

   if (result) u__strcpy(ip, result);
#endif

   return result;
}

void UIPAddress::resolveStrAddress()
{
   U_TRACE_NO_PARAM(0, "UIPAddress::resolveStrAddress()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(U_ipaddress_StrAddressUnresolved(this))

   if (resolveStrAddress(iAddressType, pcAddress.p, pcStrAddress)) U_ipaddress_StrAddressUnresolved(this) = false;
}

/****************************************************************************/
/* void resolveHostName()                                                   */
/*                                                                          */
/* This method is used to resolve the hostname using a reverse DNS          */
/* lookup, this need be performed only if the lazy evaluation flag is set.  */
/* The flag is reset at completion of the method to indicate that the host  */
/* name has been resolved. The reverse DNS lookup is performed via the      */
/* IPADDR_TO_HOST() macro. If the IP details lookup                         */
/* fails, pheDetails is set to NULL and we cannot obtain the host name, we  */
/* instead use the string representation of the ip address as a hostname -  */
/* calling resolveStrAddress() first to force the evaluation of this        */
/* string.  Otherwise we set the hostname from the values returned by the   */
/* function call                                                            */
/****************************************************************************/

void UIPAddress::resolveHostName()
{
   U_TRACE_NO_PARAM(1, "UIPAddress::resolveHostName()")

   U_CHECK_MEMORY

   if (U_ipaddress_HostNameUnresolved(this))
      {
#  ifdef HAVE_GETNAMEINFO
      char hbuf[NI_MAXHOST];
      SocketAddress sockadd;

      sockadd.setIPAddress(*this);

      int gai_err = U_SYSCALL(getnameinfo, "%p,%d,%p,%d,%p,%d,%d", (const sockaddr*)sockadd, sockadd.sizeOf(), hbuf, sizeof(hbuf), 0, 0, 0);

      if (gai_err)
         {
         U_WARNING("getnameinfo() error: %s", gai_strerror(gai_err));

         if (U_ipaddress_StrAddressUnresolved(this)) resolveStrAddress();

         strHostName.clear();
         }
      else
         {
         uint32_t len = u__strlen(hbuf, __PRETTY_FUNCTION__);

         if (u_isIPAddr(iAddressType == AF_INET6, hbuf, len)) strHostName.clear();
         else                                          (void) strHostName.replace(hbuf, len);
         }
#  else
      /*
      struct hostent {
         char*  h_name;      // official name of host
         char** h_aliases;   // alias list
         int    h_addrtype;  // host address type
         int    h_length;    // length of address
         char** h_addr_list; // list of addresses
      }
      */

      struct hostent* pheDetails;

      IPADDR_TO_HOST(pheDetails, pcAddress.p, iAddressLength, iAddressType);

      if (pheDetails == 0)
         {
         if (U_ipaddress_StrAddressUnresolved(this)) resolveStrAddress();

         strHostName.clear();
         }
      else
         {
         uint32_t len = u__strlen(pheDetails->h_name, __PRETTY_FUNCTION__);

         if (u_isIPAddr(iAddressType == AF_INET6, pheDetails->h_name, len)) strHostName.clear();
         else                                                        (void) strHostName.replace(pheDetails->h_name, len);
         }

#  endif

      U_ipaddress_HostNameUnresolved(this) = false;
      }

   U_INTERNAL_DUMP("strHostName = %V", strHostName.rep)

   U_INTERNAL_ASSERT_EQUALS(u_isIPAddr(iAddressType == AF_INET6, U_STRING_TO_PARAM(strHostName)), false)
}

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

#ifdef ENABLE_IPV6
void UIPAddress::convertToAddressFamily(int iNewAddressFamily)
{
   U_TRACE(1, "UIPAddress::convertToAddressFamily(%d)", iNewAddressFamily)

   U_CHECK_MEMORY

   if (iAddressType != iNewAddressFamily)
      {
      if  (iNewAddressFamily == AF_INET:)
         {
         if (IN6_IS_ADDR_V4MAPPED(&(pcAddress.s))) setAddress(pcAddress.p + 12, sizeof(in_addr));
         }
      else  
         {
         U_INTERNAL_ASSERT_EQUALS(iNewAddressFamily, AF_INET6)

         iAddressLength = sizeof(in6_addr);

         (void) memset(pcAddress.p,                0, 10);
         (void) memset(pcAddress.p + 10,        0xff,  2);
              U_MEMCPY(pcAddress.p + 12, pcAddress.p,  4);
         }

      iAddressType = iNewAddressFamily;

      U_ipaddress_HostNameUnresolved(this)   =
      U_ipaddress_StrAddressUnresolved(this) = true;
      }
}
#endif

/**
 * In the Internet addressing architecture, a private network is a network that uses private IP address space,
 * following the standards set by RFC 1918 and RFC 4193. These addresses are commonly used for home, office, and
 * enterprise local area networks (LANs), when globally routable addresses are not mandatory, or are not available
 * for the intended network applications. Under Internet Protocol IPv4, private IP address spaces were originally
 * defined in an effort to delay IPv4 address exhaustion[citation needed], but they are also a feature of the next
 * generation Internet Protocol, IPv6.
 *
 * These addresses are characterized as private because they are not globally delegated, meaning they are not allocated
 * to any specific organization, and IP packets addressed by them cannot be transmitted onto the public Internet.
 * Anyone may use these addresses without approval from a regional Internet registry (RIR). If such a private network
 * needs to connect to the Internet, it must use either a network address translator (NAT) gateway, or a proxy server
 */

__pure bool UIPAddress::isPrivate()
{
   U_TRACE_NO_PARAM(0, "UIPAddress::isPrivate()")

   U_CHECK_MEMORY

#ifdef ENABLE_IPV6
   if (iAddressType == AF_INET6)
      {
      // TODO: private address for IPv6 are in RFC 4862

      U_RETURN(false);
      }
   else
#endif
      {
      U_INTERNAL_ASSERT_EQUALS(iAddressType, AF_INET)
      U_INTERNAL_ASSERT_EQUALS(iAddressLength, sizeof(in_addr))

      if (isPrivate(htonl(pcAddress.i))) U_RETURN(true);
      }

   U_RETURN(false);
}

__pure bool UIPAddress::isWildCard()
{
   U_TRACE_NO_PARAM(0, "UIPAddress::isWildCard()")

   U_CHECK_MEMORY

#ifdef ENABLE_IPV6
   if (iAddressType == AF_INET6)
      {
   // static const struct in6_addr in6addr_any = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* :: */

      if (memcmp(pcAddress.p, in6addr_any, sizeof(in6_addr)) == 0) U_RETURN(true);

      U_RETURN(false);
      }
   else
#endif
      {
      U_INTERNAL_ASSERT_EQUALS(iAddressType, AF_INET)
      U_INTERNAL_ASSERT_EQUALS(iAddressLength, sizeof(in_addr))

      U_DUMP("htonl(pcAddress.i) = 0x%X %S", htonl(pcAddress.i), UIPAddress::toString(getInAddr()).data())

      if (htonl(pcAddress.i) == 0x00000000) U_RETURN(true);

      U_RETURN(false);
      }

   U_RETURN(false);
}

UString UIPAddress::toString(uint8_t* addr)
{
   U_TRACE(0, "UIPAddress::toString(%p)", addr)

   union uuaddr {
      uint8_t*        p;
      struct in_addr* paddr;
   };

   union uuaddr u = { addr };

   /* The inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation.
    * The string is returned in a statically allocated buffer, which subsequent calls will overwrite
    */

   char* result = inet_ntoa(*(u.paddr));

   UString x((void*)result, u__strlen(result, __PRETTY_FUNCTION__));

   U_RETURN_STRING(x);
}

// Simple IP-based access-control system
// Interpret a "HOST/BITS" IP mask specification. (Ex. 192.168.1.64/28)

bool UIPAllow::parseMask(const UString& _spec)
{
   U_TRACE(1, "UIPAllow::parseMask(%V)", _spec.rep)

   // get bit before slash

   struct in_addr ia;
   char addr_str[U_INET_ADDRSTRLEN];
   uint32_t start, addr_len = _spec.find('/');

   // extract and parse addr part

   if (_spec.c_char(0) == '!')
      {
      bnot  = true;
      start = 1;
      spec  = _spec.substr(1).copy();
      }
   else
      {
      bnot  = false;
      start = 0;
      spec  = _spec.copy();
      }

   if (addr_len == U_NOT_FOUND)
      {
      mask = 0xffffffff;

      _spec.copy(addr_str, addr_len, start);
      }
   else
      {
      mask = network = 0;

      _spec.copy(addr_str, addr_len - start, start);
      }

   U_INTERNAL_DUMP("u_isIPv4Addr(%S) = %b", addr_str, u_isIPv4Addr(addr_str, u__strlen(addr_str, __PRETTY_FUNCTION__)))

   if (u_isIPv4Addr(addr_str, u__strlen(addr_str, __PRETTY_FUNCTION__)) == false) U_RETURN(false);

   // converts the internet address from the IPv4 numbers-and-dots notation into binary form
   // (in network byte order) and stores it in the structure that points to

   int result = inet_aton(addr_str, &ia);

   U_INTERNAL_DUMP("addr_str = %S result = %d", addr_str, result)

   if (result == 0) U_RETURN(false);

   addr = ia.s_addr;

   // ------------------------------------
   // find mask length as a number of bits
   // ------------------------------------
   // /8  ..... 255.0.0.0       0xff000000
   // /16 65536 255.255.0.0     0xffff0000
   // /24   256 255.255.255.0   0xffffff00
   // /30     4 255.255.255.252 0xfffffffc
   // ------------------------------------
   int mask_bits = atoi(_spec.c_pointer(addr_len + 1));

   U_INTERNAL_DUMP("mask_bits = %d", mask_bits)

   if (addr_len == U_NOT_FOUND)
      {
      network = addr;
      }
   else if (mask_bits &&
            mask_bits != 32)
      {
      if (mask_bits < 0 ||
          mask_bits > 32)
         {
         U_RETURN(false);
         }

      // --------------------------------------------------------------------------------------------
      // Make a network-endian mask with the top mask_bits set
      // --------------------------------------------------------------------------------------------
      //      mask = 192.168.1.64/28 255.255.255.240
      // mask_bits = 28
      // addr      = <00000011 00010101 10000000 00000010>            192.168.1.64
      // mask      = <11111111 11111111 11111111 00001111> 0xf0ffffff 192.168.1.64/28 255.255.255.240
      // network   = <00000011 00010101 10000000 00000010> 0x4001a8c0 192.168.1.64
      // --------------------------------------------------------------------------------------------

      mask = htonl(~(0xffffffff >> mask_bits));

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      // -------------------------------------------------------------------
      //
      // -------------------------------------------------------------------
#  else
      // -------------------------------------------------------------------
      // -------------------------------------------------------------------
#  endif

      network = addr & mask;

      U_DUMP("addr = %S mask = %S network = %S",
               UIPAddress::toString(addr).data(),
               UIPAddress::toString(mask).data(),
               UIPAddress::toString(network).data())
      }

   U_INTERNAL_DUMP("addr = 0x%08x %B mask = 0x%08x %B network = 0x%08x %B", addr, addr, mask, mask, network, network)

   U_RETURN(true);
}

uint32_t UIPAllow::parseMask(const UString& vspec, UVector<UIPAllow*>& vipallow)
{
   U_TRACE(0, "UIPAllow::parseMask(%V,%p)", vspec.rep, &vipallow)

   UIPAllow* elem;
   UVector<UString> vec(vspec, ", \t");
   uint32_t result, n = vipallow.size();

   for (uint32_t i = 0, vlen = vec.size(); i < vlen; ++i)
      {
      U_NEW(UIPAllow, elem, UIPAllow);

      if (elem->parseMask(vec[i])) vipallow.push_back(elem);
      else                         delete elem;
      }

   result = (vipallow.size() - n);

   U_RETURN(result);
}

__pure bool UIPAllow::isAllowed(in_addr_t client)
{
   U_TRACE(0, "UIPAllow::isAllowed(%u)", client)

   U_DUMP("addr   = %S mask = %S network = %S",
            UIPAddress::toString(addr).data(),
            UIPAddress::toString(mask).data(),
            UIPAddress::toString(network).data())

   U_INTERNAL_ASSERT_EQUALS(network, addr & mask)

   bool result = ((client & mask) == network);

   U_DUMP("client = %S mask = %S network = %S",
            UIPAddress::toString(client).data(),
            UIPAddress::toString(mask).data(),
            UIPAddress::toString(client & mask).data())

   if (bnot) U_RETURN(!result);

   U_RETURN(result);
}

__pure bool UIPAllow::isAllowed(in_addr_t ifa_addr, in_addr_t ifa_netmask)
{
   U_TRACE(0, "UIPAllow::isAllowed(%u,%u)", ifa_addr, ifa_netmask)

   U_DUMP("addr     = %S mask        = %S network = %S",
            UIPAddress::toString(addr).data(),
            UIPAddress::toString(mask).data(),
            UIPAddress::toString(network).data())

   U_INTERNAL_ASSERT_EQUALS(network, addr & mask)

   bool result = ((ifa_addr & ifa_netmask) == network);

   U_DUMP("ifa_addr = %S ifa_netmask = %S network = %S",
            UIPAddress::toString(ifa_addr).data(),
            UIPAddress::toString(ifa_netmask).data(),
            UIPAddress::toString(ifa_addr & ifa_netmask).data())

   U_RETURN(result);
}

bool UIPAllow::isAllowed(const char* ip_client)
{
   U_TRACE(0, "UIPAllow::isAllowed(%S)", ip_client)

   U_INTERNAL_ASSERT(u_isIPv4Addr(ip_client, u__strlen(ip_client, __PRETTY_FUNCTION__)))

   struct in_addr ia;

   bool result = (inet_aton(ip_client, &ia) && isAllowed(ia.s_addr));

   U_RETURN(result);
}

bool UIPAllow::isAllowed(const UString& ip_client)
{
   U_TRACE(0, "UIPAllow::isAllowed(%V)", ip_client.rep)

   U_INTERNAL_ASSERT(u_isIPv4Addr(U_STRING_TO_PARAM(ip_client)))

   struct in_addr ia;

   bool result = (inet_aton(ip_client.c_str(), &ia) && isAllowed(ia.s_addr));

   U_RETURN(result);
}

__pure uint32_t UIPAllow::find(in_addr_t client, UVector<UIPAllow*>& vipallow)
{
   U_TRACE(0, "UIPAllow::find(%u,%p)", client, &vipallow)

   for (uint32_t i = 0, vlen = vipallow.size(); i < vlen; ++i)
      {
      if (vipallow[i]->isAllowed(client)) U_RETURN(i);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UIPAllow::find(const char* ip_client, UVector<UIPAllow*>& vipallow)
{
   U_TRACE(0, "UIPAllow::find(%S,%p)", ip_client, &vipallow)

   for (uint32_t i = 0, vlen = vipallow.size(); i < vlen; ++i)
      {
      if (vipallow[i]->isAllowed(ip_client)) U_RETURN(i);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UIPAllow::find(const UString& ip_client, UVector<UIPAllow*>& vipallow)
{
   U_TRACE(0, "UIPAllow::find(%V,%p)", ip_client.rep, &vipallow)

   for (uint32_t i = 0, vlen = vipallow.size(); i < vlen; ++i)
      {
      if (vipallow[i]->isAllowed(ip_client)) U_RETURN(i);
      }

   U_RETURN(U_NOT_FOUND);
}

bool UIPAllow::getNetworkInterface(UVector<UIPAllow*>& vipallow)
{
   U_TRACE(0, "UIPAllow::getNetworkInterface(%p)", &vipallow)

   bool result = false;

#if defined(HAVE_IFADDRS_H) && defined(HAVE_GETNAMEINFO)
   struct ifaddrs* ifaddr;

   if (U_SYSCALL(getifaddrs, "%p", &ifaddr) == 0)
      {
      UIPAllow* pallow;
      int family, gai_err;
      char host[NI_MAXHOST];
      uusockaddr ifa_addr, ifa_netmask;
      uint32_t i, vlen = vipallow.size();

      for (struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next)
         {
         if (ifa->ifa_addr == 0) continue;

         family = ifa->ifa_addr->sa_family;

#     ifdef U_LINUX
         U_INTERNAL_DUMP("%s => family: %d%s", ifa->ifa_name, family,
                                 (family == AF_PACKET) ? " (AF_PACKET)" :
                                 (family == AF_INET)   ? " (AF_INET)"   :
                                 (family == AF_INET6)  ? " (AF_INET6)"  : "")
#     endif

         if (family == AF_INET &&
             u_get_unalignedp16(ifa->ifa_name) != U_MULTICHAR_CONSTANT16('l','o')) // Name of interface
            {
            gai_err = U_SYSCALL(getnameinfo, "%p,%d,%p,%d,%p,%d,%d", ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, 0, 0, NI_NUMERICHOST);

            if (gai_err)
               {
               U_WARNING("getnameinfo() error: %s", gai_strerror(gai_err));
               }
            else
               {
               U_INTERNAL_DUMP("%s => address: <%s>", ifa->ifa_name, host)

               U_MEMCPY(&ifa_addr,     ifa->ifa_addr,    sizeof(struct sockaddr)); // Address of interface
               U_MEMCPY(&ifa_netmask, ifa->ifa_netmask, sizeof(struct sockaddr)); // Netmask of interface

               U_DUMP("ifa_addr = %S ifa_netmask = %S ifa_network = %S",
                     UIPAddress::toString(ifa_addr.psaIP4Addr.sin_addr.s_addr).data(),
                     UIPAddress::toString(ifa_netmask.psaIP4Addr.sin_addr.s_addr).data(),
                     UIPAddress::toString(ifa_addr.psaIP4Addr.sin_addr.s_addr & ifa_netmask.psaIP4Addr.sin_addr.s_addr).data())

               U_INTERNAL_ASSERT_EQUALS(ifa_addr.psaIP4Addr.sin_family,    AF_INET)
               U_INTERNAL_ASSERT_EQUALS(ifa_netmask.psaIP4Addr.sin_family, AF_INET)

               for (i = 0; i < vlen; ++i)
                  {
                  pallow = vipallow[i];

                  if (pallow->isAllowed(ifa_addr.psaIP4Addr.sin_addr.s_addr, ifa_netmask.psaIP4Addr.sin_addr.s_addr))
                     {
                     result = true;

                     (void) pallow->device.replace(ifa->ifa_name);
                     (void) pallow->host.replace(host);

                     break;
                     }
                  }
               }
            }
         }

      U_SYSCALL_VOID(freeifaddrs, "%p", ifaddr);
      }
#endif

   U_RETURN(result);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UIPAllow::dump(bool reset) const
{
   *UObjectIO::os << "addr            " << addr           << '\n'
                  << "mask            " << mask           << '\n'
                  << "bnot            " << bnot           << '\n'
                  << "network         " << mask           << '\n' 
                  << "host   (UString " << (void*)&host   << ")\n"
                  << "device (UString " << (void*)&device << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UIPAddress::dump(bool reset) const
{
   *UObjectIO::os << "pcAddress            " << (void*)pcAddress.p << '\n'
                  << "pcStrAddress         ";

   char buffer[128];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%S"), pcStrAddress));

   *UObjectIO::os << '\n'
                  << "iAddressType         " << iAddressType
                                             << " (" << (iAddressType == AF_INET6 ? "IPv6" : "IPv4")
                                             << ")\n"
                  << "iAddressLength       " << iAddressLength      << '\n'
                  << "strHostName (UString " << (void*)&strHostName << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
