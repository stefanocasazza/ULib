// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ipt_ACCOUNT.h - accessing information from ipt_ACCOUNT kernel module (Intra2net AG)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_IPT_ACCOUNT_H
#define ULIB_IPT_ACCOUNT_H 1

#include <ulib/net/socket.h>

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
#  include <linux/netfilter_ipv4/ipt_ACCOUNT.h>
#else
struct ipt_acc_handle_ip;
struct ipt_acc_handle_sockopt;
#endif

/*
struct ipt_acc_handle_ip {
   uint32_t ip;
   uint32_t src_packets;
   uint32_t src_bytes;
   uint32_t dst_packets;
   uint32_t dst_bytes;
};
*/

class U_EXPORT UIptAccount : public USocket {
public:

   // COSTRUTTORI

            UIptAccount(bool bSocketIsIPv6 = false);
   virtual ~UIptAccount();

   // SERVICES

   void freeEntries();
   bool getTableNames();
   bool freeAllHandles();
   int  getHandleUsage();
   bool readEntries(const char* table, bool dont_flush);

   const char*               getError() { return error_str; }
   const char*               getNextName();
#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   struct ipt_acc_handle_ip* getNextEntry();
#endif

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
    void* data;
    const char* error_str;
    unsigned int pos, data_size;
    struct ipt_acc_handle_sockopt* handle;

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UIptAccount(const UIptAccount&) = delete;
   UIptAccount& operator=(const UIptAccount&) = delete;
#else
   UIptAccount(const UIptAccount&) : USocket(false) {}
   UIptAccount& operator=(const UIptAccount&)       { return *this; }
#endif
};

#endif
