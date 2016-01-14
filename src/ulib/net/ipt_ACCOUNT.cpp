// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ipt_ACCOUNT.cpp - accessing information from ipt_ACCOUNT kernel module (Intra2net AG)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/ipt_ACCOUNT.h>
#include <ulib/utility/services.h>

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
#  include <netinet/in.h>
#  if defined(HAVE_NETPACKET_PACKET_H) && !defined(U_ALL_CPP)
#     include <linux/if.h>
#  endif
#  include <linux/netfilter_ipv4/ip_tables.h>

/**
 * Socket option interface shared between kernel (xt_ACCOUNT) and userspace library (libxt_ACCOUNT_cl).
 * Hopefully we are unique at least within our kernel & xtables-addons space. Turned out often enough we are not
 *
 *  64- 67 used by ip_tables, ip6_tables
 *  96-100 used by arp_tables
 * 128-131 used by ebtables
 */

#ifdef HAVE_ARCH64
#  define IPT_ACCOUNT_TWEAK_BASE_CTL_NUMBER_FOR_SETSOCKOPTS
#endif
#  ifndef IPT_ACCOUNT_TWEAK_BASE_CTL_NUMBER_FOR_SETSOCKOPTS
#     define IPT_SO_SET_ACCOUNT_HANDLE_FREE         (IPT_BASE_CTL + 3)
#     define IPT_SO_SET_ACCOUNT_HANDLE_FREE_ALL     (IPT_BASE_CTL + 4)

#     define IPT_SO_GET_ACCOUNT_PREPARE_READ        (IPT_BASE_CTL + 4)
#     define IPT_SO_GET_ACCOUNT_PREPARE_READ_FLUSH  (IPT_BASE_CTL + 5)
#     define IPT_SO_GET_ACCOUNT_GET_DATA            (IPT_BASE_CTL + 6)
#     define IPT_SO_GET_ACCOUNT_GET_HANDLE_USAGE    (IPT_BASE_CTL + 7)
#     define IPT_SO_GET_ACCOUNT_GET_TABLE_NAMES     (IPT_BASE_CTL + 8)
#  else
#     define SO_ACCOUNT_BASE_CTL 70
#     define IPT_SO_SET_ACCOUNT_HANDLE_FREE        (SO_ACCOUNT_BASE_CTL + 1)
#     define IPT_SO_SET_ACCOUNT_HANDLE_FREE_ALL    (SO_ACCOUNT_BASE_CTL + 2)
#     define IPT_SO_GET_ACCOUNT_PREPARE_READ       (SO_ACCOUNT_BASE_CTL + 4)
#     define IPT_SO_GET_ACCOUNT_PREPARE_READ_FLUSH (SO_ACCOUNT_BASE_CTL + 5)
#     define IPT_SO_GET_ACCOUNT_GET_DATA           (SO_ACCOUNT_BASE_CTL + 6)
#     define IPT_SO_GET_ACCOUNT_GET_HANDLE_USAGE   (SO_ACCOUNT_BASE_CTL + 7)
#     define IPT_SO_GET_ACCOUNT_GET_TABLE_NAMES    (SO_ACCOUNT_BASE_CTL + 8)
#  endif

#  define IPT_SO_SET_ACCOUNT_MAX IPT_SO_SET_ACCOUNT_HANDLE_FREE_ALL
#  define IPT_SO_GET_ACCOUNT_MAX IPT_SO_GET_ACCOUNT_GET_TABLE_NAMES
#endif

#define IPT_ACCOUNT_MIN_BUFSIZE 4096 // Don't set this below the size of struct ipt_account_handle_sockopt

UIptAccount::UIptAccount(bool bSocketIsIPv6) : USocket(bSocketIsIPv6)
{
   U_TRACE_REGISTER_OBJECT(0, UIptAccount, "%b", bSocketIsIPv6)

   pos       = 0;
   error_str = 0;
#ifndef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   data      = 0;
   data_size = 0;
   handle    = 0;
#else
   handle = U_MALLOC_TYPE(struct ipt_acc_handle_sockopt);

   (void) memset(handle, '\0', sizeof(struct ipt_acc_handle_sockopt));

   handle->handle_nr = (uint32_t)-1;

   USocket::_socket(SOCK_RAW, 0, IPPROTO_ICMP);

   if (USocket::isOpen() == false)
      {
      if (UServices::isSetuidRoot() == false) U_ERROR("Must run as root to create raw socket");

      U_ERROR("Can't open socket to kernel. Permission denied or ipt_ACCOUNT module not loaded");
      }

   // 4096 bytes default buffer should save us from reallocations as it fits 200 concurrent active clients

   data      = UMemoryPool::_malloc(IPT_ACCOUNT_MIN_BUFSIZE);
   data_size =                      IPT_ACCOUNT_MIN_BUFSIZE;
#endif
}

UIptAccount::~UIptAccount()
{
   U_TRACE_UNREGISTER_OBJECT(0, UIptAccount)

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   UMemoryPool::_free(data, data_size);

   U_FREE_TYPE(handle, struct ipt_acc_handle_sockopt);

   freeEntries();
#endif
}

void UIptAccount::freeEntries()
{
   U_TRACE_NO_PARAM(0, "UIptAccount::freeEntries()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   U_INTERNAL_ASSERT_POINTER(handle)

   if (handle->handle_nr != (uint32_t)-1)
      {
      (void) USocket::setSockOpt(IPPROTO_IP, IPT_SO_SET_ACCOUNT_HANDLE_FREE, handle, sizeof(struct ipt_acc_handle_sockopt));

      handle->handle_nr = (uint32_t)-1;
      }

   pos = 0;

   handle->itemcount = 0;
#endif
}

bool UIptAccount::readEntries(const char* table, bool bflush)
{
   U_TRACE(0, "UIptAccount::readEntries(%S,%b)", table, bflush)

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   uint32_t s = sizeof(struct ipt_acc_handle_sockopt);

   (void) u__strncpy(handle->name, table, ACCOUNT_TABLE_NAME_LEN-1);

   // Get table information

   if (USocket::getSockOpt(IPPROTO_IP, (bflush ? IPT_SO_GET_ACCOUNT_PREPARE_READ_FLUSH : IPT_SO_GET_ACCOUNT_PREPARE_READ), handle, s) == false)
      {
      error_str = "Can't get table information from kernel. Is the table existing ?";

      U_RETURN(false);
      }

   // Check data buffer size

   pos = 0;

   uint32_t new_size = handle->itemcount * sizeof(struct ipt_acc_handle_ip);

   // We want to prevent reallocations all the time

   if (new_size < IPT_ACCOUNT_MIN_BUFSIZE) new_size = IPT_ACCOUNT_MIN_BUFSIZE;

   // Reallocate if it's too small or twice as big

   if (data_size <  new_size ||
       data_size > (new_size * 2))
      {
      // Free old buffer

      UMemoryPool::_free(data, data_size);

      data = UMemoryPool::_malloc(new_size);

      data_size = new_size;
      }

   // Copy data from kernel

   U_MEMCPY(data, handle, sizeof(struct ipt_acc_handle_sockopt));

   if (USocket::getSockOpt(IPPROTO_IP, IPT_SO_GET_ACCOUNT_GET_DATA, data, data_size) == false)
      {
      error_str = "Can't get data from kernel. Check /var/log/messages for details";

      freeEntries();

      U_RETURN(false);
      }

   // Free kernel handle but don't reset pos/itemcount

   (void) USocket::setSockOpt(IPPROTO_IP, IPT_SO_SET_ACCOUNT_HANDLE_FREE, handle, sizeof(struct ipt_acc_handle_sockopt));

   handle->handle_nr = (uint32_t)-1;
#endif

   U_RETURN(true);
}

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
struct ipt_acc_handle_ip* UIptAccount::getNextEntry()
{
   U_TRACE_NO_PARAM(0, "UIptAccount::getNextEntry()")

   // Empty or no more items left to return ?

   if (!handle->itemcount || pos >= handle->itemcount) U_RETURN_POINTER(0, struct ipt_acc_handle_ip);

   // Get next entry

   struct ipt_acc_handle_ip* rtn = (struct ipt_acc_handle_ip*)((char*)data + pos * sizeof(struct ipt_acc_handle_ip));

   ++pos;

   U_RETURN_POINTER(rtn, struct ipt_acc_handle_ip);
}
#endif

int UIptAccount::getHandleUsage()
{
   U_TRACE_NO_PARAM(0, "UIptAccount::getHandleUsage()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   uint32_t s = sizeof(struct ipt_acc_handle_sockopt);

   if (USocket::getSockOpt(IPPROTO_IP, IPT_SO_GET_ACCOUNT_GET_HANDLE_USAGE, handle, s))
      {
      handle->handle_nr = (uint32_t)-1;

      U_RETURN(handle->itemcount);
      }

   error_str = "Can't get handle usage information from kernel";
#endif

   U_RETURN(-1);
}

bool UIptAccount::freeAllHandles()
{
   U_TRACE_NO_PARAM(0, "UIptAccount::freeAllHandles()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   if (USocket::setSockOpt(IPPROTO_IP, IPT_SO_SET_ACCOUNT_HANDLE_FREE_ALL, 0, 0)) U_RETURN(true);

   error_str = "Can't free all kernel handles";
#endif
   U_RETURN(false);
}
    
bool UIptAccount::getTableNames()
{
   U_TRACE_NO_PARAM(0, "UIptAccount::getTableNames()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   if (USocket::getSockOpt(IPPROTO_IP, IPT_SO_GET_ACCOUNT_GET_TABLE_NAMES, data, data_size))
      {
      pos = 0;

      U_RETURN(true);
      }

   error_str = "Can't get table names from kernel. Out of memory, MINBUFISZE too small ?";
#endif
   U_RETURN(false);
}

const char* UIptAccount::getNextName()
{
   U_TRACE_NO_PARAM(0, "UIptAccount::getNextName()")

   const char* rtn = 0;

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   if (((char*)data)[pos] == '\0') U_RETURN((const char*)0);

   rtn = (const char*)data + pos;

   pos += u__strlen((char*)data + pos, __PRETTY_FUNCTION__) + 1;
#endif

   U_RETURN(rtn);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UIptAccount::dump(bool reset) const
{
   USocket::dump(false);

   *UObjectIO::os << '\n'
                  << "pos                           " << pos            << '\n'
                  << "data                          " << data           << '\n'
                  << "handle                        " << (void*)handle  << '\n'
                  << "data_size                     " << data_size      << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
