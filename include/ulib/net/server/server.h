// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    server.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_SERVER_H
#define U_SERVER_H 1

#include <ulib/log.h>
#include <ulib/process.h>
#include <ulib/command.h>
#include <ulib/notifier.h>
#include <ulib/file_config.h>
#include <ulib/utility/interrupt.h>
#include <ulib/utility/socket_ext.h>
#include <ulib/net/server/client_image.h>
#include <ulib/net/server/server_plugin.h>

#ifndef SIGWINCH
#define SIGWINCH 28
#endif

/**
 * @class UServer
 *
 * @brief Handles incoming connections.
 *
 * The UServer class contains the methods needed to write a portable server. In general, a server listens for incoming network requests on a well-known
 * IP address and port number. When a connection request is received, the UServer makes this connection available to the server program as a socket.
 * The socket represents a two-way (full-duplex) connection with the client.
 *
 * In common with normal socket programming, the life-cycle of a UServer follows this basic course:
 *
 * 1) bind() to an IP-address/port number and listen for incoming connections
 * 2) accept() a connection request
 * 3) deal with the request, or pass the created socket to another thread or process to be dealt with
 * 4) return to step 2 for the next client connection request
 */

// ---------------------------------------------------------------------------------------------
// For example: U_MACROSERVER(UServerExample, UClientExample, UTCPSocket);
// ---------------------------------------------------------------------------------------------
#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
#  define U_MACROSERVER(server_class,client_type,socket_type) \
class server_class : public UServer<socket_type> { \
public: \
 server_class(UFileConfig* pcfg) : UServer<socket_type>(pcfg) { U_TRACE_REGISTER_OBJECT(  5, server_class, "%p", pcfg) } \
~server_class()                                               { U_TRACE_UNREGISTER_OBJECT(5, server_class) } \
const char* dump(bool reset) const { return UServer<socket_type>::dump(reset); } \
protected: \
virtual void preallocate() U_DECL_FINAL { \
U_TRACE_NO_PARAM(5+256, #server_class "::preallocate()") \
vClientImage = new client_type[UNotifier::max_connection]; } \
virtual void deallocate() U_DECL_FINAL { \
U_TRACE_NO_PARAM(5+256, #server_class "::deallocate()") \
delete[] (client_type*)vClientImage; }  \
virtual bool check_memory() U_DECL_FINAL { return u_check_memory_vector<client_type>((client_type*)vClientImage, UNotifier::max_connection); } }
#else
#  define U_MACROSERVER(server_class,client_type,socket_type) \
class server_class : public UServer<socket_type> { \
public: \
 server_class(UFileConfig* pcfg) : UServer<socket_type>(pcfg) {} \
~server_class()                                               {} \
protected: \
virtual void preallocate() U_DECL_FINAL { \
vClientImage = new client_type[UNotifier::max_connection]; } }
#endif
// ---------------------------------------------------------------------------------------------

// manage server write to log

#define U_SERVER_LOG_PREFIX "(pid %P)> "

#ifdef U_LOG_DISABLE
#  define U_RESET_MODULE_NAME
#  define   U_SET_MODULE_NAME(name)

#  define U_SRV_LOG(          fmt,args...) {}
#  define U_SRV_LOG_WITH_ADDR(fmt,args...) {}
#else
#  define U_RESET_MODULE_NAME              { if (UServer_Base::isLog())   (void) strcpy(UServer_Base::mod_name[0], UServer_Base::mod_name[1]); }
#  define   U_SET_MODULE_NAME(name)        { if (UServer_Base::isLog()) { (void) strcpy(UServer_Base::mod_name[1], UServer_Base::mod_name[0]); \
                                                                          (void) strcpy(UServer_Base::mod_name[0], "["#name"] "); } }

#  define U_SRV_LOG(          fmt,args...) { if (UServer_Base::isLog()) ULog::log(U_CONSTANT_TO_PARAM("%s" fmt),       UServer_Base::mod_name[0] , ##args); }
#  define U_SRV_LOG_WITH_ADDR(fmt,args...) { if (UServer_Base::isLog()) ULog::log(U_CONSTANT_TO_PARAM("%s" fmt " %v"), UServer_Base::mod_name[0] , ##args, \
                                                                                  UServer_Base::pClientImage->logbuf->rep); }
#endif

class UHTTP;
class UHTTP2;
class UCommand;
class UDayLight;
class UTimeStat;
class USSLSocket;
class USSIPlugIn;
class UWebSocket;
class USocketExt;
class Application;
class UTimeThread;
class UFileConfig;
class UHttpPlugIn;
class UFCGIPlugIn;
class USCGIPlugIn;
class UThrottling;
class UNoCatPlugIn;
class UGeoIPPlugIn;
class UClient_Base;
class UProxyPlugIn;
class UDataStorage;
class UStreamPlugIn;
class UModNoCatPeer;
class UClientThread;
class UHttpClient_Base;
class UWebSocketPlugIn;
class UModProxyService;
class UTimeoutConnection;

template <class T> class URDBObjectHandler;

class U_EXPORT UServer_Base : public UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // -----------------------------------------------------------------------------------------------------------------------------
   // UServer - configuration parameters
   // -----------------------------------------------------------------------------------------------------------------------------
   // ENABLE_IPV6  flag indicating the use of ipv6
   // SERVER       host name or ip address for the listening socket
   // PORT         port number             for the listening socket
   // SOCKET_NAME  file name               for the listening socket
   // IP_ADDRESS   ip address of host for the interface connected to the Internet (autodetected if not specified)
   //
   // ALLOWED_IP            list of comma separated client         address for IP-based access control (IPADDR[/MASK])
   // ALLOWED_IP_PRIVATE    list of comma separated client private address for IP-based access control (IPADDR[/MASK]) for public server
   // ENABLE_RFC1918_FILTER reject request from private IP to public server address
   // MIN_SIZE_FOR_SENDFILE for major size it is better to use sendfile() to serve static content
   //
   // LISTEN_BACKLOG             max number of ready to be delivered connections to accept()
   // SET_REALTIME_PRIORITY      flag indicating that the preforked processes will be scheduled under the real-time policies SCHED_FIFO
   //
   // CLIENT_THRESHOLD           min number of clients to active polling
   // CLIENT_FOR_PARALLELIZATION min number of clients to active parallelization 
   //
   // PID_FILE      write pid on file indicated
   // WELCOME_MSG   message of welcome to send initially to client
   // RUN_AS_USER   downgrade the privileges to that of user account
   // DOCUMENT_ROOT The directory out of which you will serve your documents
   //
   // LOG_FILE      locations for file log
   // LOG_FILE_SZ   memory size for file log
   // LOG_MSG_SIZE  limit length of print network message to LOG_MSG_SIZE chars (default 128)
   //
   // PLUGIN        list of plugins to load, a flexible way to add specific functionality to the server
   // PLUGIN_DIR    directory where there are plugins to load
   //
   // ORM_DRIVER     list of ORM drivers to load, a flexible way to add specific functionality to the ORM
   // ORM_DRIVER_DIR directory where there are ORM drivers to load
   //
   // REQ_TIMEOUT    timeout for request from client
   // MAX_KEEP_ALIVE Specifies the maximum number of requests that can be served through a Keep-Alive (Persistent) session.
   //                (Value <= 0 will disable Keep-Alive)
   //
   // DH_FILE       dh param (these are the bit DH parameters from "Assigned Number for SKIP Protocols")
   // CERT_FILE     server certificate
   // KEY_FILE      server private key
   // PASSWORD      password for server private key
   // CA_FILE       locations of trusted CA certificates used in the verification
   // CA_PATH       locations of trusted CA certificates used in the verification
   // VERIFY_MODE   mode of verification (SSL_VERIFY_NONE=0, SSL_VERIFY_PEER=1, SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2, SSL_VERIFY_CLIENT_ONCE=4)
   //
   // PREFORK_CHILD number of child server processes created at startup ( 0 - serialize, no forking
   //                                                                     1 - classic, forking after client accept
   //                                                                    >1 - pool of serialized processes plus monitoring process)
   // -----------------------------------------------------------------------------------------------------------------------------

   static void run(); // loop waiting for connection

   static UFileConfig* cfg;
   static bool bssl, bipc, flag_loop;
   static unsigned int port; // the port number to bind to

   static int          getReqTimeout()  { return (ptime ? ptime->UTimeVal::tv_sec : 0); }
   static bool         isIPv6()         { return UClientImage_Base::bIPv6; }
   static UString      getHost()        { return *host; }
   static unsigned int getPort()        { return port; }

   static UCommand* loadConfigCommand() { return UCommand::loadConfigCommand(cfg); }

   // The directory out of which you will serve your documents...

   static UString*    document_root;
   static uint32_t    document_root_size;
   static const char* document_root_ptr;

   static UString getDocumentRoot()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::getDocumentRoot()")

      U_INTERNAL_ASSERT_POINTER(document_root)

      U_RETURN_STRING(*document_root);
      }

   static bool setDocumentRoot(const UString& dir);

#ifdef U_WELCOME_SUPPORT
   static void setMsgWelcome(const UString& msg);
#endif

#if defined(U_LINUX) && defined(ENABLE_THREAD)
#endif

   // -------------------------------------------------------------------
   // MANAGE PLUGIN MODULES
   // -------------------------------------------------------------------

   static char mod_name[2][16];
   static UEventFd* handler_other;
   static UEventFd* handler_inotify;

   static int loadPlugins(UString& plugin_dir, const UString& plugin_list); // load plugin modules and call server-wide hooks handlerConfig()...

   // ---------------------------------
   // Server-wide hooks
   // ---------------------------------
   static int pluginsHandlerInit();
   static int pluginsHandlerRun();
   static int pluginsHandlerFork();
   static int pluginsHandlerStop();
   // ---------------------------------
   // Connection-wide hooks
   // ---------------------------------
   static int pluginsHandlerREAD();
   static int pluginsHandlerRequest();
   // ---------------------------------
   // SigHUP hook
   // ---------------------------------
   static int pluginsHandlerSigHUP();
   // ---------------------------------

   // ----------------------------------------------------------------------------------------------------------------------------
   // Manage process server
   // ----------------------------------------------------------------------------------------------------------------------------
   // PREFORK_CHILD number of child server processes created at startup: -1 - thread approach (experimental)
   //                                                                     0 - serialize, no forking
   //                                                                     1 - classic, forking after client accept
   //                                                                    >1 - pool of serialized processes plus monitoring process
   // ----------------------------------------------------------------------------------------------------------------------------

   typedef struct shared_data {
   // ---------------------------------
      uint32_t cnt_usr1;
      uint32_t cnt_usr2;
      uint32_t cnt_usr3;
      uint32_t cnt_usr4;
      uint32_t cnt_usr5;
      uint32_t cnt_usr6;
      uint32_t cnt_usr7;
      uint32_t cnt_usr8;
      uint32_t cnt_usr9;
   // ---------------------------------
      char buffer1[512];
      char buffer2[512];
      char buffer3[512];
      char buffer4[512];
      char buffer5[512];
      char buffer6[512];
      char buffer7[512];
   // ---------------------------------
      sig_atomic_t cnt_connection;
      sig_atomic_t cnt_parallelization;
   // ---------------------------------
      sem_t lock_user1;
      sem_t lock_user2;
      sem_t lock_throttling;
      sem_t lock_rdb_server;
      sem_t lock_data_session;
      sem_t lock_db_not_found;
      char spinlock_user1[1];
      char spinlock_user2[1];
      char spinlock_throttling[1];
      char spinlock_rdb_server[1];
      char spinlock_data_session[1];
      char spinlock_db_not_found[1];
#  ifdef USE_LIBSSL
      sem_t    lock_ssl_session;
      char spinlock_ssl_session[1];
#    if defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB) && !defined(_MSWINDOWS_)
      sem_t    lock_ocsp_staple;
      char spinlock_ocsp_staple[1];
#    endif
#  endif
   // ------------------------------------------------------------------------------
#  if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
      pthread_rwlock_t rwlock;
      struct timeval now_shared; // => u_now
      ULog::log_date log_date_shared;
#  endif
      ULog::log_data log_data_shared;
   // -> maybe unnamed array of char for gzip compression (log rotate)
   // --------------------------------------------------------------------------------
   } shared_data;

#define U_SRV_BUF1                  UServer_Base::ptr_shared_data->buffer1
#define U_SRV_BUF2                  UServer_Base::ptr_shared_data->buffer2
#define U_SRV_BUF3                  UServer_Base::ptr_shared_data->buffer3
#define U_SRV_BUF4                  UServer_Base::ptr_shared_data->buffer4
#define U_SRV_BUF5                  UServer_Base::ptr_shared_data->buffer5
#define U_SRV_BUF6                  UServer_Base::ptr_shared_data->buffer6
#define U_SRV_BUF7                  UServer_Base::ptr_shared_data->buffer7
#define U_SRV_CNT_USR1              UServer_Base::ptr_shared_data->cnt_usr1
#define U_SRV_CNT_USR2              UServer_Base::ptr_shared_data->cnt_usr2
#define U_SRV_CNT_USR3              UServer_Base::ptr_shared_data->cnt_usr3
#define U_SRV_CNT_USR4              UServer_Base::ptr_shared_data->cnt_usr4
#define U_SRV_CNT_USR5              UServer_Base::ptr_shared_data->cnt_usr5
#define U_SRV_CNT_USR6              UServer_Base::ptr_shared_data->cnt_usr6
#define U_SRV_CNT_USR7              UServer_Base::ptr_shared_data->cnt_usr7
#define U_SRV_CNT_USR8              UServer_Base::ptr_shared_data->cnt_usr8
#define U_SRV_CNT_USR9              UServer_Base::ptr_shared_data->cnt_usr9
#define U_SRV_TOT_CONNECTION        UServer_Base::ptr_shared_data->cnt_connection
#define U_SRV_CNT_PARALLELIZATION   UServer_Base::ptr_shared_data->cnt_parallelization
#define U_SRV_LOCK_USER1          &(UServer_Base::ptr_shared_data->lock_user1)
#define U_SRV_LOCK_USER2          &(UServer_Base::ptr_shared_data->lock_user2)
#define U_SRV_LOCK_THROTTLING     &(UServer_Base::ptr_shared_data->lock_throttling)
#define U_SRV_LOCK_RDB_SERVER     &(UServer_Base::ptr_shared_data->lock_rdb_server)
#define U_SRV_LOCK_SSL_SESSION    &(UServer_Base::ptr_shared_data->lock_ssl_session)
#define U_SRV_LOCK_DATA_SESSION   &(UServer_Base::ptr_shared_data->lock_data_session)
#define U_SRV_LOCK_DB_NOT_FOUND   &(UServer_Base::ptr_shared_data->lock_db_not_found)
#define U_SRV_SPINLOCK_USER1        UServer_Base::ptr_shared_data->spinlock_user1
#define U_SRV_SPINLOCK_USER2        UServer_Base::ptr_shared_data->spinlock_user2
#define U_SRV_SPINLOCK_THROTTLING   UServer_Base::ptr_shared_data->spinlock_throttling
#define U_SRV_SPINLOCK_RDB_SERVER   UServer_Base::ptr_shared_data->spinlock_rdb_server
#define U_SRV_SPINLOCK_SSL_SESSION  UServer_Base::ptr_shared_data->spinlock_ssl_session
#define U_SRV_SPINLOCK_DATA_SESSION UServer_Base::ptr_shared_data->spinlock_data_session
#define U_SRV_SPINLOCK_DB_NOT_FOUND UServer_Base::ptr_shared_data->spinlock_db_not_found

   static ULock* lock_user1;
   static ULock* lock_user2;
   static int preforked_num_kids; // keeping a pool of children and that they accept connections themselves
   static shared_data* ptr_shared_data;
   static uint32_t shared_data_add, map_size;
   static bool update_date, update_date1, update_date2, update_date3;

   static void setLockUser1()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockUser1()")

      U_INTERNAL_ASSERT_EQUALS(lock_user1, 0)

      U_NEW(ULock, lock_user1, ULock);

      lock_user1->init(&(ptr_shared_data->lock_user1), ptr_shared_data->spinlock_user1);
      }

   static void setLockUser2()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockUser2()")

      U_INTERNAL_ASSERT_EQUALS(lock_user2, 0)

      U_NEW(ULock, lock_user2, ULock);

      lock_user2->init(&(ptr_shared_data->lock_user2), ptr_shared_data->spinlock_user2);
      }

   // NB: two step shared memory acquisition - first we get the offset, after the pointer...

   static void* getOffsetToDataShare(uint32_t shared_data_size)
      {
      U_TRACE(0, "UServer_Base::getOffsetToDataShare(%u)", shared_data_size)

      long offset = sizeof(shared_data) + shared_data_add;
                                          shared_data_add += shared_data_size;

      U_RETURN_POINTER(offset, void);
      }

   static void* getPointerToDataShare(void* shared_data_ptr)
      {
      U_TRACE(0, "UServer_Base::getPointerToDataShare(%p)", shared_data_ptr)

      U_INTERNAL_ASSERT_POINTER(ptr_shared_data)

      shared_data_ptr = (void*)((ptrdiff_t)ptr_shared_data + (ptrdiff_t)shared_data_ptr);

      U_RETURN_POINTER(shared_data_ptr, void);
      }

   static uint32_t           nClientIndex;
   static UClientImage_Base* vClientImage;
   static UClientImage_Base* pClientImage;
   static UClientImage_Base* eClientImage;

   static bool isPreForked()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isPreForked()")

      U_INTERNAL_DUMP("preforked_num_kids = %d", preforked_num_kids)

      if (preforked_num_kids > 1) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isClassic()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isClassic()")

      U_INTERNAL_DUMP("preforked_num_kids = %d", preforked_num_kids)

      if (preforked_num_kids == 1) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isChild()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isChild()")

      U_INTERNAL_DUMP("preforked_num_kids = %d", preforked_num_kids)

      if (preforked_num_kids >= 1 &&
          proc->child())
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void removeZombies();

   // PARALLELIZATION

   static uint32_t num_client_for_parallelization, num_client_threshold;

   static bool isParallelizationChild()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isParallelizationChild()")

      U_INTERNAL_DUMP("U_ClientImage_parallelization = %d proc->child() = %b",
                       U_ClientImage_parallelization,     proc->child())

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isParallelizationParent()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isParallelizationParent()")

      U_INTERNAL_DUMP("U_ClientImage_parallelization = %d proc->parent() = %b",
                       U_ClientImage_parallelization,     proc->parent())

      if (U_ClientImage_parallelization == U_PARALLELIZATION_PARENT) U_RETURN(true);

      U_RETURN(false);
      }

   static void    endNewChild() __noreturn;
   static pid_t startNewChild();

   static bool startParallelization(            uint32_t nclient = 1); // it can creates a copy of itself, return true if parent...
   static bool    isParallelizationGoingToStart(uint32_t nclient = 1) __pure;

   // manage log server...

   typedef struct file_LOG {
      UFile* LOG;
      int    flags;
   } file_LOG;

   static ULog* log;
   static ULog* apache_like_log;
   static UVector<file_LOG*>* vlog;

   static void  closeLog();
   static void reopenLog();

   static bool isLog()      { return (log != 0); }
   static bool isOtherLog() { return (vlog->empty() == false); }

   static bool addLog(UFile* log, int flags = O_APPEND | O_WRONLY);

   static void logCommandMsgError(const char* cmd, bool balways);

   // NETWORK CTX

   static char* client_address;
   static int iAddressType, socket_flags, tcp_linger_set;
   static uint32_t client_address_len, min_size_for_sendfile;

#define U_CLIENT_ADDRESS_TO_PARAM  UServer_Base::client_address, UServer_Base::client_address_len
#define U_CLIENT_ADDRESS_TO_TRACE  UServer_Base::client_address_len, UServer_Base::client_address

   static UString getIPAddress()                         { return *IP_address; }
   static UString getNetworkDevice( const char* exclude) { return USocketExt::getNetworkDevice(exclude); }
   static UString getNetworkAddress(const char* device)  { return USocketExt::getNetworkAddress(socket->getFd(), device); }

#if defined(USE_LIBSSL) && defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB) && !defined(_MSWINDOWS_)
   static UThread* pthread_ocsp;
   static ULock* lock_ocsp_staple;

   static void setLockOCSPStaple()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockOCSPStaple()")

      U_INTERNAL_ASSERT_EQUALS(lock_ocsp_staple, 0)

      U_NEW(ULock, lock_ocsp_staple, ULock);

      lock_ocsp_staple->init(&(ptr_shared_data->lock_ocsp_staple), ptr_shared_data->spinlock_ocsp_staple);
      }
#endif

   // DEBUG

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const;
#endif

   static USocket* socket;
   static USocket* csocket;
protected:
   static int timeoutMS,       // the time-out value in milliseconds for client request
              verify_mode;     // mode of verification ssl connection

   static UString* server;     // host name or ip address for the listening socket
   static UString* as_user;    // change the current working directory to the user's home dir, and downgrade security to that user account
   static UString* dh_file;    // These are the 1024 bit DH parameters from "Assigned Number for SKIP Protocols"
   static UString* cert_file;  // locations for certificate of server
   static UString* key_file;   // locations for private key of server
   static UString* password;   // password for private key of server
   static UString* ca_file;    // locations of trusted CA certificates used in the verification
   static UString* ca_path;    // locations of trusted CA certificates used in the verification
   static UString* name_sock;  // name file for the listening socket
   static UString* IP_address; // IP address of this server

   static int rkids;
   static UString* host;
   static sigset_t mask;
   static UProcess* proc;
   static UEventTime* ptime;
   static UServer_Base* pthis;
   static UString* cenvironment;
   static UString* senvironment;
   static bool flag_sigterm, monitoring_process, set_realtime_priority, public_address, binsert, set_tcp_keep_alive, called_from_handlerTime;

   static uint32_t                 vplugin_size;
   static UVector<UString>*        vplugin_name;
   static UVector<UString>*        vplugin_name_static;
   static UVector<UServerPlugIn*>* vplugin;
   static UVector<UServerPlugIn*>* vplugin_static;

   static void init();
   static void loadConfigParam();
   static void runLoop(const char* user);
   static bool handlerTimeoutConnection(void* cimg);

#ifdef U_WELCOME_SUPPORT
   static UString* msg_welcome;
#endif

#ifdef U_ACL_SUPPORT
   static UString* allow_IP;
   static UVector<UIPAllow*>* vallow_IP;
#endif

#ifdef U_RFC1918_SUPPORT
   static UString* allow_IP_prv;
   static bool enable_rfc1918_filter;
   static UVector<UIPAllow*>* vallow_IP_prv;
#endif

#ifdef DEBUG
# ifndef U_LOG_DISABLE
   static long last_event;
# endif
   static uint64_t stats_bytes;
   static uint32_t max_depth, wakeup_for_nothing, nread, nread_again, stats_connections, stats_simultaneous;

   static UString getStats();
#endif

#ifdef U_THROTTLING_SUPPORT
   static bool         throttling_chk;
   static UString*     throttling_mask;
   static UThrottling* throttling_rec;
   static URDBObjectHandler<UDataStorage*>* db_throttling;

   static void clearThrottling();
   static bool checkThrottling();
   static bool checkThrottlingBeforeSend(bool bwrite);

   static void initThrottlingClient();
   static void initThrottlingServer();
#endif

            UServer_Base(UFileConfig* pcfg = 0);
   virtual ~UServer_Base();

#ifndef U_LOG_DISABLE
   static uint32_t setNumConnection(char* buffer);
#endif

   // define method VIRTUAL of class UEventFd

   virtual int  handlerRead()   U_DECL_FINAL; // This method is called to accept a new connection on the server socket
   virtual void handlerDelete() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::handlerDelete()")

      U_INTERNAL_DUMP("UEventFd::fd = %d", UEventFd::fd)

      UEventFd::fd = -1;
      }

   // method VIRTUAL to redefine

   virtual void handlerSignal(int signo)
      {
      U_TRACE(0, "UServer_Base::handlerSignal(%d)", signo)
      }

   virtual void preallocate()  = 0;
#ifdef DEBUG
   virtual void  deallocate()  = 0;
   virtual bool check_memory() = 0;
#endif

   // SERVICES

   static void _preallocate()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::_preallocate()")

      U_INTERNAL_ASSERT_POINTER(pthis)

      pthis->preallocate();
      }

   static RETSIGTYPE handlerForSigHUP(  int signo);
   static RETSIGTYPE handlerForSigTERM( int signo);
   static RETSIGTYPE handlerForSigCHLD( int signo);
   static RETSIGTYPE handlerForSigWINCH(int signo);

   static void manageChangeOfSystemTime();
   static void sendSignalToAllChildren(int signo, sighandler_t handler);

private:
   static void manageSigHUP() U_NO_EXPORT;
   static bool clientImageHandlerRead() U_NO_EXPORT;
   static void logMemUsage(const char* signame) U_NO_EXPORT;
   static void loadStaticLinkedModules(const char* name) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UServer_Base)

   friend class UHTTP;
   friend class UHTTP2;
   friend class UDayLight;
   friend class UTimeStat;
   friend class USSLSocket;
   friend class USSIPlugIn;
   friend class UWebSocket;
   friend class USocketExt;
   friend class Application;
   friend class UTimeThread;
   friend class UHttpPlugIn;
   friend class USCGIPlugIn;
   friend class UFCGIPlugIn;
   friend class UThrottling;
   friend class UApplication;
   friend class UProxyPlugIn;
   friend class UNoCatPlugIn;
   friend class UGeoIPPlugIn;
   friend class UClient_Base;
   friend class UStreamPlugIn;
   friend class UClientThread;
   friend class UModNoCatPeer;
   friend class UHttpClient_Base;
   friend class UWebSocketPlugIn;
   friend class UModProxyService;
   friend class UClientImage_Base;
   friend class UTimeoutConnection;
   friend class UBandWidthThrottling;
};

template <class Socket> class U_EXPORT UServer : public UServer_Base {
public:

   typedef UClientImage<Socket> client_type;

   UServer(UFileConfig* pcfg = 0) : UServer_Base(pcfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UServer, "%p", pcfg)

      U_NEW(Socket, socket, Socket(UClientImage_Base::bIPv6));
      }

   virtual ~UServer()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UServer)
      }

   // DEBUG

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const { return UServer_Base::dump(reset); }
#endif

protected:

   // ---------------------------------------------------------------------------------------------------------------
   // method VIRTUAL to redefine
   // ---------------------------------------------------------------------------------------------------------------
   // Create a new UClientImage object representing a client connection to the server.
   // Derived classes that have overridden UClientImage object may call this function to implement the creation logic
   // ---------------------------------------------------------------------------------------------------------------

   virtual void preallocate() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0+256, "UServer<Socket>::preallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      vClientImage = new client_type[UNotifier::max_connection];

      U_INTERNAL_DUMP("vClientImage = %p pClientImage = %p", vClientImage, pClientImage)

      U_INTERNAL_ASSERT_EQUALS(vClientImage, pClientImage)
      }

#ifdef DEBUG
   virtual void deallocate() U_DECL_OVERRIDE
      {
      U_TRACE(0+256, "UServer<Socket>::deallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      delete[] (client_type*)vClientImage;
      }

   virtual bool check_memory() U_DECL_OVERRIDE { return u_check_memory_vector<client_type>((client_type*)vClientImage, UNotifier::max_connection); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UServer)
};

#ifdef USE_LIBSSL
template <> class U_EXPORT UServer<USSLSocket> : public UServer_Base {
public:

   typedef UClientImage<USSLSocket> client_type;

   UServer(UFileConfig* pcfg) : UServer_Base(pcfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UServer<USSLSocket>, "%p", pcfg)

#  ifdef DEBUG
      if (pcfg &&
          bssl == false)
         {
         U_ERROR("You need to set bssl var before loading the configuration");
         }
#  endif

      U_NEW(USSLSocket, socket, USSLSocket(UClientImage_Base::bIPv6, 0, true));
      }

   virtual ~UServer()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UServer<USSLSocket>)
      }

   // DEBUG

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const { return UServer_Base::dump(reset); }
#endif

protected:

   // ---------------------------------------------------------------------------------------------------------------
   // method VIRTUAL to redefine
   // ---------------------------------------------------------------------------------------------------------------
   // Create a new UClientImage object representing a client connection to the server.
   // Derived classes that have overridden UClientImage object may call this function to implement the creation logic
   // ---------------------------------------------------------------------------------------------------------------

   virtual void preallocate() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0+256, "UServer<USSLSocket>::preallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      vClientImage = new client_type[UNotifier::max_connection];
      }

#ifdef DEBUG
   virtual void deallocate() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0+256, "UServer<USSLSocket>::deallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      delete[] (client_type*)vClientImage;
      }

   virtual bool check_memory() U_DECL_OVERRIDE { return u_check_memory_vector<client_type>((client_type*)vClientImage, UNotifier::max_connection); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UServer<USSLSocket>)
};
#endif
#endif
