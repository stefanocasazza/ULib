// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    socket_address.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOCKET_ADDRESS_H
#define ULIB_SOCKET_ADDRESS_H 1

/*****************************************************************************/
/* Helper Class SocketAddress                                                */
/*                                                                           */
/* This class provides encapsulation of the sockaddr_in and sockadd_in6      */
/* structures used by many IP socket commands. The class allows us to set    */
/* parts of the structures and retrieve their contents. This class is mainly */
/* used for convenience and to minimise rewriting of code to support both    */
/* IPv4 and IPv6                                                             */
/*****************************************************************************/

// ------------------------------------
// struct sockaddr {
//    u_short sa_family;
//    char    sa_data[14];
// };
//
// struct sockaddr_in {
//    short          sin_family;
//    u_short        sin_port;
//    struct in_addr sin_addr;
//    char           sin_zero[8];
// };
//
// struct sockaddr_in6 {
//    u_short         sin_family;
//    in_port_t       sin6_port;       /* Transport layer port # */
//    uint32_t        sin6_flowinfo;   /* IPv6 flow information */
//    struct in6_addr sin6_addr;       /* IPv6 address */
//    uint32_t        sin6_scope_id;   /* IPv6 scope-id */
// };
//
// struct in_addr {
//    u_long s_addr;
// };
// ------------------------------------

class U_NO_EXPORT SocketAddress {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // Zeroes the contents of the internal sockaddr_in6 structure and then sets
   // all three structure pointers to point to the internal structure. The
   // three pointers allow us to reference the structure as being of all types.
   // The actual internal structure is of type sockaddr_in6 because it is the
   // largest of all three types

   SocketAddress() { (void) memset(&addr, 0, sizeof(addr)); }

   // If we want an IPv6 wildcard address, the sin6_addr value of the sockaddr
   // structure is set to in6addr_any, otherwise the sin_addr value of the
   // structure is set to INADDR_ANY

   void setIPAddressWildCard(bool bIPv6)
      {
#  ifdef ENABLE_IPV6
   // static const struct in6_addr in6addr_any = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; /* :: */

      if (bIPv6)
         {
         addr.psaIP6Addr.sin6_addr   = in6addr_any;
         addr.psaIP6Addr.sin6_family = AF_INET6;
         }
      else
#  else
      (void)bIPv6;
#  endif
         {
         addr.psaIP4Addr.sin_addr.s_addr = htonl(INADDR_ANY);
         addr.psaIP4Addr.sin_family      = AF_INET;
         }
      }

   // Sets the Address part of the sockaddr structure. First we set the family
   // type based on the family type of the provided IP Address. We then make a
   // binary copy of the address stored in cAddr to the appropriate field in the
   // sockaddr structure

   void setIPAddress(UIPAddress& cAddr)
      {
      U_TRACE(0, "SocketAddress::setIPAddress(%p)", &cAddr)

      addr.psaGeneric.sa_family = cAddr.getAddressFamily();

#  ifdef ENABLE_IPV6
      if (addr.psaGeneric.sa_family == AF_INET6)
         U_MEMCPY(&(addr.psaIP6Addr.sin6_addr), cAddr.get_in_addr(), cAddr.getInAddrLength());
      else
#  endif
         U_MEMCPY(&(addr.psaIP4Addr.sin_addr),  cAddr.get_in_addr(), cAddr.getInAddrLength());
      }

   // Sets the port number part of the sockaddr structure. Based on the value
   // of the family type, we set either the sin_port or sin6_port fields of the
   // sockaddr structure. If the family type is not set, we set the IP4 value

   void setPortNumber(int iPortNumber)
      {
#  ifdef ENABLE_IPV6
      if (addr.psaGeneric.sa_family == AF_INET6) addr.psaIP6Addr.sin6_port = htons(iPortNumber);
      else
#  endif
                                                 addr.psaIP4Addr.sin_port  = htons(iPortNumber);
      }

   // Returns the Address stored in the sockaddr structure. Based on the family
   // type value, we set the IPAddress parameter equal to either the sin6_addr
   // or sin_addr fields. If the family type is not set, we assume it is IPv4

   void getIPAddress(UIPAddress& cAddr)
      {
#  ifdef ENABLE_IPV6
      if (addr.psaGeneric.sa_family == AF_INET6) cAddr.setAddress(&(addr.psaIP6Addr.sin6_addr), true);
      else
#  endif
                                                 cAddr.setAddress(&(addr.psaIP4Addr.sin_addr), false);
      }

   // Returns the port number stored in the sockaddr structure. Based on the
   // family type value, we set the iPortNumber parameter equal to either the
   // sin6_port or sin_port fields. If the family type is not set, assume IPv4

   unsigned int getPortNumber()
      {
#  ifdef ENABLE_IPV6
      if (addr.psaGeneric.sa_family == AF_INET6) return ntohs(addr.psaIP6Addr.sin6_port);
      else
#  endif                                  
                                                 return ntohs(addr.psaIP4Addr.sin_port);
      }

   // Returns the size of the structure pointed to by the (sockaddr*) cast.
   // The value returned is based on the family type value. If the family type
   // is not set, we return the size of the larger sockaddr_in6 structure so
   // that both types are covered

   socklen_t sizeOf()
      {
#  ifdef ENABLE_IPV6
      if (addr.psaGeneric.sa_family == AF_INET6) return sizeof(sockaddr_in6);
      else
#  endif
                                                 return sizeof(sockaddr_in);
      }

   operator       sockaddr*()       { return &(addr.psaGeneric); }
   operator const sockaddr*() const { return &(addr.psaGeneric); }

#if defined(HAVE_GETADDRINFO) || defined(HAVE_GETNAMEINFO)
   /*
   struct addrinfo {
      int ai_flags;              // Input flags
      int ai_family;             // Protocol family for socket
      int ai_socktype;           // Socket type
      int ai_protocol;           // Protocol for socket
      socklen_t ai_addrlen;      // Length of socket address
      struct sockaddr* ai_addr;  // Socket address for socket
      char* ai_canonname;        // Canonical name for service location
      struct addrinfo* ai_next;  // Pointer to next in list
   };
   */
   void set(struct addrinfo* result)
      {
      U_TRACE(0, "SocketAddress::set(%p)", result)

      addr.psaGeneric.sa_family = result->ai_family;

#  ifdef ENABLE_IPV6
      if (addr.psaGeneric.sa_family == AF_INET6)
         U_MEMCPY(&(addr.psaIP6Addr.sin6_addr),
                         &((struct sockaddr_in6*)result->ai_addr)->sin6_addr, sizeof(in6_addr));
      else
#  endif
         U_MEMCPY(&(addr.psaIP4Addr.sin_addr),
                         &((struct sockaddr_in*)result->ai_addr)->sin_addr,   sizeof(in_addr));
      }
#endif

protected:
   union uusockaddr addr;

private:
   U_DISALLOW_COPY_AND_ASSIGN(SocketAddress)

   friend class USocket;
};

#endif
