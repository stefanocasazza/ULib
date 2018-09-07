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
#include <ulib/net/udpsocket.h>
#include <ulib/net/unixsocket.h>
#include <ulib/utility/interrupt.h>
#include <ulib/utility/socket_ext.h>
#include <ulib/net/server/client_image.h>

#ifndef SIGWINCH
#define SIGWINCH 28
#endif

#ifdef U_STATIC_ORM_DRIVER_PGSQL
#define U_DB_BUSY_ARRAY_SIZE 256
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
 server_class(UFileConfig* cfg) : UServer<socket_type>(cfg) { U_TRACE_CTOR(5, server_class, "%p", cfg) } \
~server_class()                                             { U_TRACE_DTOR(5, server_class) } \
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
 server_class(UFileConfig* cfg) : UServer<socket_type>(cfg) {} \
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

#  define U_SRV_LOG_CMD_MSG_ERR(cmd,balways) {}
#else
#  define U_SRV_LOG_CMD_MSG_ERR(cmd,balways) { if (UServer_Base::isLog()) UServer_Base::logCommandMsgError((cmd).getCommand(),balways); }

#  define U_RESET_MODULE_NAME              { if (UServer_Base::isLog())   (void) strcpy(UServer_Base::mod_name[0], UServer_Base::mod_name[1]); }
#  define   U_SET_MODULE_NAME(name)        { if (UServer_Base::isLog()) { (void) strcpy(UServer_Base::mod_name[1], UServer_Base::mod_name[0]); \
                                                                          (void) strcpy(UServer_Base::mod_name[0], "["#name"] "); } }

#  define U_SRV_LOG(          fmt,args...) { if (UServer_Base::isLog()) UServer_Base::log->log(U_CONSTANT_TO_PARAM("%s" fmt),       UServer_Base::mod_name[0] , ##args); }
#  define U_SRV_LOG_WITH_ADDR(fmt,args...) { if (UServer_Base::isLog()) UServer_Base::log->log(U_CONSTANT_TO_PARAM("%s" fmt " %v"), UServer_Base::mod_name[0] , ##args, \
                                                                                               UServer_Base::pClientImage->logbuf->rep); }

#endif

class UHTTP;
class UHTTP2;
class UEventDB;
class UCommand;
class UTimeStat;
class UDayLight;
class UTimeStat;
class USSLSocket;
class USSIPlugIn;
class UWebSocket;
class USocketExt;
class USSEClient;
class USSEThread;
class Application;
class UTimeThread;
class UFileConfig;
class UHttpPlugIn;
class UFCGIPlugIn;
class USCGIPlugIn;
class USmtpClient;
class UNoCatPlugIn;
class UNoDogPlugIn;
class UGeoIPPlugIn;
class UClient_Base;
class UProxyPlugIn;
class UDataStorage;
class UStreamPlugIn;
class UModNoCatPeer;
class UModNoDogPeer;
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

   // --------------------------------------------------------------------------------------------------------------------------------------
   // UServer - configuration parameters
   // --------------------------------------------------------------------------------------------------------------------------------------
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
   // LOAD_BALANCE_CLUSTER           list of comma separated IP address (IPADDR[/MASK]) to define the load balance cluster
   // LOAD_BALANCE_DEVICE_NETWORK    network interface name of the cluster of physical server
   // LOAD_BALANCE_LOADAVG_THRESHOLD system load threshold to proxies the request on other userver on the network cluster ([0-9].[0-9])
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
   //
   // CRASH_COUNT         this is the threshold for the number of crash of child server processes
   // CRASH_EMAIL_NOTIFY  the email address to send a message whenever the number of crash > CRASH_COUNT
   // --------------------------------------------------------------------------------------------------------------------------------------
   // This directive are for evasive action in the event of an HTTP DoS or DDoS attack or brute force attack
   // --------------------------------------------------------------------------------------------------------------------------------------
   // DOS_PAGE_COUNT      this is the threshold for the number of requests for the same page (or URI) per page interval
   // DOS_PAGE_INTERVAL   the interval for the page count threshold; defaults to 1 second intervals
   // DOS_SITE_COUNT      this is the threshold for the total number of requests for any object by the same client per site interval
   // DOS_SITE_INTERVAL   the interval for the site count threshold; defaults to 1 second intervals
   // DOS_BLOCKING_PERIOD the blocking period is the amount of time (in seconds) that a client will be blocked for if they are added to the blocking list (defaults to 10)
   // DOS_WHITE_LIST      list of comma separated IP addresses of trusted clients can be whitelisted to insure they are never denied (IPADDR[/MASK])
   // DOS_EMAIL_NOTIFY    the email address to send a message whenever an IP address becomes blacklisted
   // DOS_SYSTEM_COMMAND  the system command specified will be executed whenever an IP address becomes blacklisted. Use %.*s to denote the IP address of the blacklisted IP
   // DOS_LOGFILE         the file to write DOS event
   // --------------------------------------------------------------------------------------------------------------------------------------

   static void run(); // loop waiting for connection

#ifdef USE_FSTACK
   static int ff_run(void* arg)
      {
      U_TRACE(0, "UServer_Base::ff_run(%p)", arg)

      run();

      U_RETURN(0);
      }
#endif

   static UFileConfig* pcfg;
   static bool bssl, bipc, budp, flag_loop;
   static unsigned int port; // the port number to bind to

   static int          getReqTimeout()  { return (ptime ? ptime->UTimeVal::tv_sec : 0); }
   static bool         isIPv6()         { return UClientImage_Base::bIPv6; }
   static UString      getHost()        { return *host; }
   static unsigned int getPort()        { return port; }

   static UCommand* loadConfigCommand() { return UCommand::loadConfigCommand(pcfg); }

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

   // -------------------------------------------------------------------
   // MANAGE PLUGIN MODULES
   // -------------------------------------------------------------------

   static char mod_name[2][32];
   static UEventDB* handler_db1;
   static UEventDB* handler_db2;
   static UEventFd* handler_inotify;
   static UVector<UEventFd*>* handler_other;

   static void addHandlerEvent(UEventFd* item)
      {
      U_TRACE(0, "UServer_Base::addHandlerEvent(%p)", item)

      if (handler_other == U_NULLPTR) U_NEW(UVector<UEventFd*>, handler_other, UVector<UEventFd*>);

      handler_other->push_back(item);
      }

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
   static int  pluginsHandlerREAD();
   static void pluginsHandlerRequest();
   // ---------------------------------
   // SIGHUP hook
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
   // ---------------------------------
      uint8_t flag_sigterm;
   // ---------------------------------
      uint8_t   my_load;
      uint8_t  min_load_remote;
      uint32_t min_load_remote_ip;
   // ---------------------------------
      sig_atomic_t tot_connection;
      sig_atomic_t cnt_parallelization;
   // ---------------------------------
      sem_t lock_user1;
      sem_t lock_user2;
      sem_t lock_throttling;
      sem_t lock_rdb_server;
      sem_t lock_data_session;
#  ifdef USE_LIBSSL
      sem_t lock_ssl_session;
   // ------------------------------------------------------------------------------
#    if defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
      uint32_t   len_ocsp_staple;
      uint32_t valid_ocsp_staple;
      sem_t     lock_ocsp_staple;
#    endif
   // ------------------------------------------------------------------------------
#  endif
   // ------------------------------------------------------------------------------
#  ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
      sem_t lock_sse;
#   ifdef USE_LIBSSL
      sem_t lock_sse_ssl;
#   endif
#  endif
   // ------------------------------------------------------------------------------
#  if defined(U_LINUX) && defined(ENABLE_THREAD)
   // ---------------------------------
#   ifdef U_STATIC_ORM_DRIVER_PGSQL
      uint8_t busy_db1[U_DB_BUSY_ARRAY_SIZE];
      uint8_t busy_db2[U_DB_BUSY_ARRAY_SIZE];
#   endif
   // ---------------------------------
      ULog::log_date log_date_shared;
      struct timeval now_shared; // => u_now
      pthread_rwlock_t rwlock;
      int now_adjust_shared;
      bool daylight_shared;
#  endif
      ULog::log_data log_data_shared;
   // ---------------------------------
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
#define U_SRV_MY_LOAD               UServer_Base::ptr_shared_data->my_load
#define U_SRV_CNT_USR1              UServer_Base::ptr_shared_data->cnt_usr1
#define U_SRV_CNT_USR2              UServer_Base::ptr_shared_data->cnt_usr2
#define U_SRV_CNT_USR3              UServer_Base::ptr_shared_data->cnt_usr3
#define U_SRV_CNT_USR4              UServer_Base::ptr_shared_data->cnt_usr4
#define U_SRV_CNT_USR5              UServer_Base::ptr_shared_data->cnt_usr5
#define U_SRV_CNT_USR6              UServer_Base::ptr_shared_data->cnt_usr6
#define U_SRV_CNT_USR7              UServer_Base::ptr_shared_data->cnt_usr7
#define U_SRV_CNT_USR8              UServer_Base::ptr_shared_data->cnt_usr8
#define U_SRV_CNT_USR9              UServer_Base::ptr_shared_data->cnt_usr9
#define U_SRV_SSE_CNT1              UServer_Base::ptr_shared_data->cnt_usr8
#define U_SRV_SSE_CNT2              UServer_Base::ptr_shared_data->cnt_usr9
#define U_SRV_CNT_NOCAT             UServer_Base::ptr_shared_data->cnt_usr7
#define U_SRV_CNT_WIAUTH            UServer_Base::ptr_shared_data->cnt_usr7
#define U_SRV_FLAG_SIGTERM          UServer_Base::ptr_shared_data->flag_sigterm
#define U_SRV_LEN_OCSP_STAPLE       UServer_Base::ptr_shared_data->len_ocsp_staple
#define U_SRV_VALID_OCSP_STAPLE     UServer_Base::ptr_shared_data->valid_ocsp_staple
#define U_SRV_MIN_LOAD_REMOTE       UServer_Base::ptr_shared_data->min_load_remote
#define U_SRV_MIN_LOAD_REMOTE_IP    UServer_Base::ptr_shared_data->min_load_remote_ip
#define U_SRV_TOT_CONNECTION        UServer_Base::ptr_shared_data->tot_connection
#define U_SRV_CNT_PARALLELIZATION   UServer_Base::ptr_shared_data->cnt_parallelization
#define U_SRV_DB1_BUSY              UServer_Base::ptr_shared_data->busy_db1
#define U_SRV_DB2_BUSY              UServer_Base::ptr_shared_data->busy_db2
#define U_SRV_LOCK_USER1          &(UServer_Base::ptr_shared_data->lock_user1)
#define U_SRV_LOCK_USER2          &(UServer_Base::ptr_shared_data->lock_user2)
#define U_SRV_LOCK_SSE            &(UServer_Base::ptr_shared_data->lock_sse)
#define U_SRV_LOCK_SSE_SSL        &(UServer_Base::ptr_shared_data->lock_sse_ssl)
#define U_SRV_LOCK_THROTTLING     &(UServer_Base::ptr_shared_data->lock_throttling)
#define U_SRV_LOCK_RDB_SERVER     &(UServer_Base::ptr_shared_data->lock_rdb_server)
#define U_SRV_LOCK_SSL_SESSION    &(UServer_Base::ptr_shared_data->lock_ssl_session)
#define U_SRV_LOCK_DATA_SESSION   &(UServer_Base::ptr_shared_data->lock_data_session)

   static ULock* lock_user1;
   static ULock* lock_user2;
   static int preforked_num_kids; // keeping a pool of children and that they accept connections themselves
   static shared_data* ptr_shared_data;
   static uint32_t shared_data_add, map_size;
   static bool update_date, update_date1, update_date2, update_date3;

#define U_SHM_LOCK_NENTRY 512

   typedef struct shm_data {
   // ---------------------------------
      long last_time_email_dos;
   // ---------------------------------
      sem_t lock_user1;
      sem_t lock_user2;
      sem_t lock_evasive;
      sem_t lock_websock;
      sem_t lock_db_not_found;
   // ---------------------------------
      sem_t lock_base[U_SHM_LOCK_NENTRY];
   // ---------------------------------
      ULog::log_data log_data_shared;
   // ---------------------------------
   // -> maybe unnamed array of char for gzip compression (apache log like rotate)
   } shm_data;

   static shm_data* ptr_shm_data;
   static uint32_t shm_data_add, shm_size;

#define U_SHM_LOCK_USER1          &(UServer_Base::ptr_shm_data->lock_user1)
#define U_SHM_LOCK_USER2          &(UServer_Base::ptr_shm_data->lock_user2)
#define U_SHM_LOCK_EVASIVE        &(UServer_Base::ptr_shm_data->lock_evasive)
#define U_SHM_LOCK_WEBSOCK        &(UServer_Base::ptr_shm_data->lock_websock)
#define U_SHM_LOCK_DB_NOT_FOUND   &(UServer_Base::ptr_shm_data->lock_db_not_found)
#define U_SHM_LOCK_BASE           &(UServer_Base::ptr_shm_data->lock_base)
#define U_SHM_LAST_TIME_EMAIL_DOS   UServer_Base::ptr_shm_data->last_time_email_dos

#ifdef USE_LOAD_BALANCE
   static UString* ifname;
   static uint8_t loadavg_threshold;
   static UVector<UIPAllow*>* vallow_cluster;
#endif

   static void setLockUser1()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockUser1()")

      U_INTERNAL_ASSERT_EQUALS(lock_user1, U_NULLPTR)

      U_NEW(ULock, lock_user1, ULock);

      lock_user1->init(U_SRV_LOCK_USER1);
      }

   static void setLockUser2()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockUser2()")

      U_INTERNAL_ASSERT_EQUALS(lock_user2, U_NULLPTR)

      U_NEW(ULock, lock_user2, ULock);

      lock_user2->init(U_SRV_LOCK_USER2);
      }

   // NB: two step shared memory acquisition - first we get the offset, after the pointer...

   static void* getOffsetToDataShare(uint32_t shared_data_size)
      {
      U_TRACE(0, "UServer_Base::getOffsetToDataShare(%u)", shared_data_size)

      long offset = sizeof(shared_data) + shared_data_add;
                                          shared_data_add += shared_data_size;

      U_RETURN_POINTER(offset, void);
      }

   static void* getOffsetToDataShm(uint32_t shm_data_size)
      {
      U_TRACE(0, "UServer_Base::getOffsetToDataShm(%u)", shm_data_size)

      long offset = sizeof(shm_data) + shm_data_add;
                                       shm_data_add += shm_data_size;

      U_RETURN_POINTER(offset, void);
      }

   static void* getPointerToDataShare(void* shared_data_ptr)
      {
      U_TRACE(0, "UServer_Base::getPointerToDataShare(%p)", shared_data_ptr)

      U_INTERNAL_ASSERT_POINTER(ptr_shared_data)

      shared_data_ptr = (void*)((ptrdiff_t)ptr_shared_data + (ptrdiff_t)shared_data_ptr);

      U_RETURN_POINTER(shared_data_ptr, void);
      }

   static void* getPointerToDataShm(void* shm_data_ptr)
      {
      U_TRACE(0, "UServer_Base::getPointerToDataShm(%p)", shm_data_ptr)

      U_INTERNAL_ASSERT_POINTER(ptr_shm_data)

      shm_data_ptr = (void*)((ptrdiff_t)ptr_shm_data + (ptrdiff_t)shm_data_ptr);

      U_RETURN_POINTER(shm_data_ptr, void);
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

   static void removeZombies()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::removeZombies()")

#  if defined(U_LOG_DISABLE) || !defined(U_LINUX) || !defined(ENABLE_THREAD)
            (void) UProcess::removeZombies();
#  else
      uint32_t n = UProcess::removeZombies();

      U_INTERNAL_ASSERT_POINTER(ptr_shared_data)

      if (n) U_SRV_LOG("removed %u zombies - current parallelization (%d)", n, U_SRV_CNT_PARALLELIZATION);
#  endif
      }

   static uint32_t num_client_threshold;

   // PARALLELIZATION (dedicated process for long-running task)

   static void    endNewChild() __noreturn;
   static pid_t startNewChild();

   static bool isParallelizationParent()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isParallelizationParent()")

      U_INTERNAL_DUMP("U_ClientImage_parallelization = %d proc->parent() = %b",
                       U_ClientImage_parallelization,     proc->parent())

      if (U_ClientImage_parallelization == U_PARALLELIZATION_PARENT) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isParallelizationChild()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isParallelizationChild()")

      U_INTERNAL_DUMP("U_ClientImage_parallelization = %d proc->child() = %b",
                       U_ClientImage_parallelization,     proc->child())

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) U_RETURN(true);

      U_RETURN(false);
      }

   static bool startParallelization(uint32_t nclient = 1); // it can creates a copy of itself, return true if parent...

   // manage log server...

   typedef struct file_LOG {
      UFile* LOG;
      int    flags;
#  ifdef DEBUG
      const char* dump(bool reset) const { return ""; }
#  endif
   } file_LOG;

   static ULog* log;
   static ULog* apache_like_log;
   static UVector<file_LOG*>* vlog;

   static void  closeLog();
   static void reopenLog()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::reopenLog()")

      file_LOG* item;

      for (uint32_t i = 0, n = vlog->size(); i < n; ++i)
         {
         item = (*vlog)[i];

         item->LOG->reopen(item->flags);
         }
      }

   static bool isLog()      { return (log != U_NULLPTR); }
   static bool isOtherLog() { return (vlog->empty() == false); }

   static bool addLog(UFile* log, int flags = O_APPEND | O_WRONLY);

   static void logCommandMsgError(const char* cmd, bool balways)
      {
      U_TRACE(0, "UServer_Base::logCommandMsgError(%S,%b)", cmd, balways)

#  ifndef U_LOG_DISABLE
      U_INTERNAL_ASSERT_POINTER(log)

      if (UCommand::setMsgError(cmd, !balways) || balways) log->log(U_CONSTANT_TO_PARAM("%s%.*s"), mod_name[0], u_buffer_len, u_buffer);

      errno        = 0;
      u_buffer_len = 0;
#  endif
      }

   // NETWORK CTX

   static char* client_address;
   static int iAddressType, socket_flags, tcp_linger_set;
   static uint32_t client_address_len, min_size_for_sendfile;

#define U_CLIENT_ADDRESS_TO_PARAM UServer_Base::client_address, UServer_Base::client_address_len
#define U_CLIENT_ADDRESS_TO_TRACE UServer_Base::client_address_len, UServer_Base::client_address

   static bool isLocalHost() { return csocket->cRemoteAddress.isLocalHost(); }

   static void      setClientAddress() { setClientAddress(csocket, client_address, client_address_len); }
   static in_addr_t getClientAddress() { return csocket->getClientAddress(); }

   static UString getIPAddress()                         { return *IP_address; }
   static UString getNetworkDevice( const char* exclude) { return USocketExt::getNetworkDevice(exclude); }
   static UString getNetworkAddress(const char* device)  { return USocketExt::getNetworkAddress(socket->getFd(), device); }

#if defined(USE_LIBSSL) && defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   static UThread* pthread_ocsp;
   static ULock* lock_ocsp_staple;

   static void setLockOCSPStaple()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockOCSPStaple()")

      U_INTERNAL_ASSERT_EQUALS(lock_ocsp_staple, U_NULLPTR)

      U_NEW(ULock, lock_ocsp_staple, ULock);

      lock_ocsp_staple->init(&(ptr_shared_data->lock_ocsp_staple));
      }
#endif

#ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   static ULock* lock_sse;
# ifdef USE_LIBSSL
   static ULock* lock_sse_ssl;
# endif

   static void setLockSSE()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::setLockSSE()")

#  ifdef USE_LIBSSL
      if (bssl)
         {
         U_INTERNAL_ASSERT_EQUALS(lock_sse_ssl, U_NULLPTR)

         U_NEW(ULock, lock_sse_ssl, ULock);

         lock_sse_ssl->init(&(ptr_shared_data->lock_sse_ssl));
         }
      else
#  endif
      {
      U_INTERNAL_ASSERT_EQUALS(lock_sse, U_NULLPTR)

      U_NEW(ULock, lock_sse, ULock);

      lock_sse->init(&(ptr_shared_data->lock_sse));
      }
      }

   static void lockSSE()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::lockSSE()")

#  ifdef USE_LIBSSL
      if (bssl) lock_sse_ssl->lock();
      else
#  endif
      lock_sse->lock();
      }

   static void unlockSSE()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::unlockSSE()")

#  ifdef USE_LIBSSL
      if (bssl) lock_sse_ssl->unlock();
      else
#  endif
      lock_sse->unlock();
      }

   static bool isSSEStarted()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::isSSEStarted()")

      U_INTERNAL_DUMP("U_SRV_SSE_CNT1 = %u", U_SRV_SSE_CNT1)

      if (U_SRV_SSE_CNT1) U_RETURN(true);

      U_RETURN(false);
      }

   static UString printSSE(uint32_t id, const UString& data, UString* pevent = U_NULLPTR)
      {
      U_TRACE(0, "UServer_Base::printSSE(%u,%V,%p)", id, data.rep, pevent)

      U_ASSERT_EQUALS(u_find(U_STRING_TO_PARAM(data),"\n",1), U_NULLPTR)

      UString buffer(U_CAPACITY);

      if (pevent == U_NULLPTR) buffer.snprintf(U_CONSTANT_TO_PARAM("id:%u\ndata:%v\n\n"),           id,              data.rep);
      else                     buffer.snprintf(U_CONSTANT_TO_PARAM("id:%u\nevent:%v\ndata:%v\n\n"), id, pevent->rep, data.rep);

      U_RETURN_STRING(buffer);
      }

   static void eventSSE(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UServer_Base::eventSSE(%.*S,%u)", fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)
      U_INTERNAL_ASSERT_DIFFERS(sse_event_fd, -1)

      uint32_t len;
      char buffer[4096];

      va_list argp;
      va_start(argp, fmt_size);

      len = u__vsnprintf(buffer, U_CONSTANT_SIZE(buffer), format, fmt_size, argp);

      va_end(argp);

   // lockSSE();

      (void) U_SYSCALL(write, "%u,%S,%u", sse_event_fd, buffer, len);
      }

   static void sendToIdSSE(const UString& id, const UString& data)
      {
      U_TRACE(0, "UServer_Base::sendToIdSSE(%V,%V)", id.rep, data.rep)

      U_ASSERT_EQUALS(u_find(U_STRING_TO_PARAM(data),"\n",1), U_NULLPTR)

      eventSSE(U_CONSTANT_TO_PARAM("MSG %v %v\n"), id.rep, data.rep);
      }

   static void sendToAllSSE(const UString& subscribe, const UString& data)
      {
      U_TRACE(0, "UServer_Base::sendToAllSSE(%V,%V)", subscribe.rep, data.rep)

      U_ASSERT_EQUALS(u_find(U_STRING_TO_PARAM(data),"\n",1), U_NULLPTR)

      eventSSE(U_CONSTANT_TO_PARAM("%v=%v\n"), subscribe.rep, data.rep);
      }

   static void sendToAllSSE(const UString& data)
      {
      U_TRACE(0, "UServer_Base::sendToAllSSE(%V)", data.rep)

      U_ASSERT_EQUALS(u_find(U_STRING_TO_PARAM(data),"\n",1), U_NULLPTR)

      eventSSE(U_CONSTANT_TO_PARAM("*=%v\n"), data.rep);
      }

   static void sendToAllExceptSSE(const UString& data)
      {
      U_TRACE(0, "UServer_Base::sendToAllExceptSSE(%V)", data.rep)

      U_ASSERT_EQUALS(u_find(U_STRING_TO_PARAM(data),"\n",1), U_NULLPTR)

      eventSSE(U_CONSTANT_TO_PARAM("%v-%v=%v\n"), (sse_event ? sse_event : UString::str_asterisk)->rep, sse_id->rep, data.rep);
      }
#endif

   // DEBUG

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const;
#endif

   static USocket* socket;
   static USocket* csocket;
   static UString* name_sock;  // name file for the listening socket
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
   static UString* IP_address; // IP address of this server

   static int rkids;
   static UString* host;
   static sigset_t mask;
   static UProcess* proc;
   static UString* auth_ip;
   static UEventTime* ptime;
   static UServer_Base* pthis;
   static uint32_t crash_count;
   static UUDPSocket* udp_sock;
   static UString* cenvironment;
   static UString* senvironment;
   static USmtpClient* emailClient;
   static long last_time_email_crash;
   static UString* crashEmailAddress;
   static bool monitoring_process, set_realtime_priority, public_address, binsert, set_tcp_keep_alive, called_from_handlerTime;

   static uint32_t                 vplugin_size;
   static UVector<UString>*        vplugin_name;
   static UVector<UString>*        vplugin_name_static;
   static UVector<UServerPlugIn*>* vplugin;
   static UVector<UServerPlugIn*>* vplugin_static;

   static void init();
   static void loadConfigParam();
   static void runLoop(const char* user);
   static bool handlerTimeoutConnection(void* cimg);

#ifndef U_LOG_DISABLE
   static void logNewClient(USocket* psocket, UClientImage_Base* lClientImage);
#endif

#ifdef U_WELCOME_SUPPORT
   static UString* msg_welcome;
#endif

#ifdef U_ACL_SUPPORT
   static UVector<UIPAllow*>* vallow_IP;
#endif

#ifdef U_RFC1918_SUPPORT
   static bool enable_rfc1918_filter;
   static UVector<UIPAllow*>* vallow_IP_prv;
#endif

#ifdef DEBUG
   static UTimeStat* pstat;
   static uint64_t stats_bytes;
   static uint32_t max_depth, wakeup_for_nothing, nread, nread_again, stats_connections, stats_simultaneous;

   static UString getStats();

# ifndef U_LOG_DISABLE
   static long last_event;
# endif
#endif

#ifdef USERVER_UDP
   static vPF  runDynamicPage_udp;
   static vPFu runDynamicPageParam_udp;

   static int handlerUDP()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::handlerUDP()")

      U_INTERNAL_DUMP("runDynamicPage_udp = %p", runDynamicPage_udp) 

      if (runDynamicPage_udp) runDynamicPage_udp();
      else
         {
         *UClientImage_Base::wbuffer = *UClientImage_Base::rbuffer; // echo server
         }

      UClientImage_Base::bnoheader = true;

      UClientImage_Base::setRequestProcessed();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }
#endif

#ifdef U_THROTTLING_SUPPORT
   typedef struct uthrottling {
      uint64_t bytes_since_avg;
      uint32_t krate, min_limit, max_limit, num_sending;
   } uthrottling;

   static void getThrottlingRecFromBuffer(const char* data, uint32_t datalen)
      {
      U_TRACE(0, "UServer_Base::getThrottlingRecFromBuffer(%.*S,%u)", datalen, data, datalen)

      throttling_rec = (uthrottling*)u_buffer;

      u_buffer_len = sizeof(uthrottling);

      const char* ptr = data;

      throttling_rec->bytes_since_avg = u_strtoullp(&ptr);
      throttling_rec->krate           = u_strtoulp(&ptr);
      throttling_rec->min_limit       = u_strtoulp(&ptr);
      throttling_rec->max_limit       = u_strtoulp(&ptr);
      throttling_rec->num_sending     = u_strtoulp(&ptr);

      U_INTERNAL_DUMP("throttling_rec = { %llu %u %u %u %u }", throttling_rec->bytes_since_avg, throttling_rec->krate,
                                                               throttling_rec->min_limit, throttling_rec->max_limit, throttling_rec->num_sending)

      U_INTERNAL_ASSERT_EQUALS(ptr, data+datalen+1)
      }

   static void printThrottlingRecToBuffer(const char* data, uint32_t datalen)
      {
      U_TRACE(0, "UServer_Base::printThrottlingRecToBuffer(%.*S,%u)", datalen, data, datalen)

      U_INTERNAL_ASSERT_EQUALS(datalen, sizeof(uthrottling))

      throttling_rec = (uthrottling*)data;

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%llu %u %u %u %u"), throttling_rec->bytes_since_avg,
                                 throttling_rec->krate, throttling_rec->min_limit, throttling_rec->max_limit, throttling_rec->num_sending);
      }

   static bool         throttling_chk;
   static UString*     throttling_mask;
   static uthrottling* throttling_rec;
   static URDBObjectHandler<UDataStorage*>* db_throttling;

   static void clearThrottling();
   static bool checkThrottling();
   static bool checkThrottlingBeforeSend(bool bwrite);

   static void initThrottlingClient();
   static void initThrottlingServer();
#endif

#ifdef U_EVASIVE_SUPPORT // provide evasive action in the event of an HTTP DoS or DDoS attack or brute force attack
   typedef struct uevasive {
      uint32_t timestamp, count;
   } uevasive;

   static void getEvasiveRecFromBuffer(const char* data, uint32_t datalen)
      {
      U_TRACE(0, "UServer_Base::getEvasiveRecFromBuffer(%.*S,%u)", datalen, data, datalen)

      evasive_rec = (uevasive*)u_buffer;

      u_buffer_len = sizeof(uevasive);

      const char* ptr = data;

      evasive_rec->timestamp = u_strtoulp(&ptr);
      evasive_rec->count     = u_strtoulp(&ptr);

      U_INTERNAL_DUMP("evasive_rec->timestamp = %u evasive_rec->count = %u", evasive_rec->timestamp, evasive_rec->count)

      U_INTERNAL_ASSERT_EQUALS(ptr, data+datalen+1)
      }

   static void printEvasiveRecToBuffer(const char* data, uint32_t datalen)
      {
      U_TRACE(0, "UServer_Base::printEvasiveRecToBuffer(%.*S,%u)", datalen, data, datalen)

      U_INTERNAL_ASSERT_EQUALS(datalen, sizeof(uevasive))

      evasive_rec = (uevasive*)data;

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%u %u"), evasive_rec->timestamp, evasive_rec->count);
      }

   static UFile* dos_LOG;
   static bool bwhitelist;
   static uevasive* evasive_rec;
   static UString* systemCommand;
   static UString* dosEmailAddress;
   static UVector<UIPAllow*>* vwhitelist_IP;
   static URDBObjectHandler<UDataStorage*>* db_evasive;
   static uint32_t blocking_period, page_interval, page_count, site_interval, site_count;

   static void initEvasive();
   static bool checkHitUriStats();
   static bool checkHitSiteStats();
   static bool checkHold(in_addr_t client);
   static bool checkHitStats(const char* key, uint32_t key_len, uint32_t interval, uint32_t count);
#endif

#ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   struct ucmsghdr {
      size_t cmsg_len; /* Length data in cmsg_data + length of cmsghdr struct */
      int cmsg_level;  /* Originating protocol.                               */
      int cmsg_type;   /* Protocol specific type.                             */
      int cmsg_data;   /* Ancillary data.                                     */
   };

   static UString* sse_id;
   static UString* sse_event;
   static struct msghdr msg;
   static struct iovec iov[1];
   static struct ucmsghdr cmsg;
   static UThread* pthread_sse;
   static uint32_t sse_fifo_pos;
   static UVector<USSEClient*>* sse_vclient;
   static char sse_fifo_name[256], iovbuf[1];
   static int sse_event_fd, sse_socketpair[2];

   static void setFIFOForSSE(const UString& id)
      {
      U_TRACE(0, "UServer_Base::setFIFOForSSE(%V)", id.rep)

      (void) u__snprintf(sse_fifo_name+sse_fifo_pos, 256-sse_fifo_pos, U_CONSTANT_TO_PARAM("%v"), id.rep);
      }
#endif

            UServer_Base(UFileConfig* cfg = U_NULLPTR);
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

   static void suspendThread();

   static void _preallocate()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::_preallocate()")

      U_INTERNAL_ASSERT_POINTER(pthis)

      pthis->preallocate();
      }

   static bool isParallelizationGoingToStart(uint32_t nclient)
      {
      U_TRACE(0, "UServer_Base::isParallelizationGoingToStart(%u)", nclient)

      U_INTERNAL_ASSERT_POINTER(ptr_shared_data)

      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_parallelization = %d UNotifier::num_connection - UNotifier::min_connection = %d",
                       U_ClientImage_pipeline,     U_ClientImage_parallelization,     UNotifier::num_connection - UNotifier::min_connection)

#  if defined(U_LINUX) && (!defined(U_SERVER_CAPTIVE_PORTAL) || defined(ENABLE_THREAD))
      if (U_ClientImage_parallelization != U_PARALLELIZATION_CHILD &&
          (UNotifier::num_connection - UNotifier::min_connection) >= nclient)
         {
         U_INTERNAL_DUMP("U_ClientImage_close = %b", U_ClientImage_close)

         U_RETURN(true);
         }
#  endif

      U_RETURN(false);
      }

   static RETSIGTYPE handlerForSigHUP( int signo);
   static RETSIGTYPE handlerForSigTERM(int signo);
// static RETSIGTYPE handlerForSigCHLD(int signo);

   static void sendSignalToAllChildren(int signo, sighandler_t handler);

private:
   static void manageWaitAll()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::manageWaitAll()")

      U_INTERNAL_ASSERT_POINTER(proc)

      if (proc->waitAll(2) == U_FAILED_ALARM) manageCommand(U_CONSTANT_TO_PARAM("pkill -KILL -P %P"), 0);
      }

   static void manageSigHUP()
      {
      U_TRACE_NO_PARAM(0, "UServer_Base::manageSigHUP()")

      manageWaitAll();

#  if !defined(U_LOG_DISABLE) && defined(U_LINUX) && defined(ENABLE_THREAD)
      U_SRV_TOT_CONNECTION = 0;
#  endif

      if (pluginsHandlerSigHUP() != U_PLUGIN_HANDLER_FINISHED) U_WARNING("Plugins stage SigHUP failed...");
      }

   static void setClientAddress(USocket* psocket, char*& pclient_address, uint32_t& pclient_address_len)
      {
      U_TRACE(0, "UServer_Base::setClientAddress(%p,%p,%u)", psocket, pclient_address, pclient_address_len)

      U_INTERNAL_ASSERT(psocket->isConnected())

      pclient_address     = UIPAddress::resolveStrAddress(iAddressType, psocket->cRemoteAddress.pcAddress.p, psocket->cRemoteAddress.pcStrAddress);
      pclient_address_len = u__strlen(pclient_address, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("client_address = %.*S", pclient_address_len, pclient_address)
      }

   static void logMemUsage(const char* signame)
      {
      U_TRACE(0, "UServer_Base::logMemUsage(%S)", signame)

      U_INTERNAL_ASSERT(isLog())

#  ifndef U_LOG_DISABLE
      unsigned long vsz, rss;

      u_get_memusage(&vsz, &rss);

      log->log(U_CONSTANT_TO_PARAM("%s (Interrupt): "
                "address space usage: %.2f MBytes - "
                          "rss usage: %.2f MBytes"), signame,
                (double)vsz / (1024.0 * 1024.0),
                (double)rss / (1024.0 * 1024.0));
#  endif
      }

   static bool clientImageHandlerRead() U_NO_EXPORT;
   static void loadStaticLinkedModules(const UString& name) U_NO_EXPORT;
   static void manageCommand(const char* format, uint32_t fmt_size, ...) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UServer_Base)

   friend class UHTTP;
   friend class UHTTP2;
   friend class UDayLight;
   friend class UTimeStat;
   friend class USSLSocket;
   friend class USSIPlugIn;
   friend class UWebSocket;
   friend class USocketExt;
   friend class USSEClient;
   friend class USSEThread;
   friend class Application;
   friend class UTimeThread;
   friend class UHttpPlugIn;
   friend class USCGIPlugIn;
   friend class UFCGIPlugIn;
   friend class UApplication;
   friend class UProxyPlugIn;
   friend class UNoCatPlugIn;
   friend class UNoDogPlugIn;
   friend class UGeoIPPlugIn;
   friend class UClient_Base;
   friend class UStreamPlugIn;
   friend class UClientThread;
   friend class UModNoCatPeer;
   friend class UModNoDogPeer;
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

   UServer(UFileConfig* cfg = 0) : UServer_Base(cfg)
      {
      U_TRACE_CTOR(0, UServer, "%p", cfg)

      U_NEW(Socket, socket, Socket(UClientImage_Base::bIPv6));
      }

   virtual ~UServer()
      {
      U_TRACE_DTOR(0, UServer)
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

      U_INTERNAL_ASSERT_MAJOR(UNotifier::max_connection, 0)
      
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

   UServer(UFileConfig* cfg) : UServer_Base(cfg)
      {
      U_TRACE_CTOR(0, UServer<USSLSocket>, "%p", cfg)

#  ifdef DEBUG
      if (cfg &&
          bssl == false)
         {
         U_ERROR("You need to set bssl var before loading the configuration");
         }
#  endif

      U_NEW(USSLSocket, socket, USSLSocket(UClientImage_Base::bIPv6, U_NULLPTR, true));

      if (cfg) ((USSLSocket*)socket)->ciphersuite_model = cfg->readLong(U_CONSTANT_TO_PARAM("CIPHER_SUITE"));
      }

   virtual ~UServer()
      {
      U_TRACE_DTOR(0, UServer<USSLSocket>)
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

#ifdef USERVER_UDP
template <> class U_EXPORT UServer<UUDPSocket> : public UServer_Base {
public:

   typedef UClientImage<UUDPSocket> client_type;

   UServer(UFileConfig* cfg) : UServer_Base(cfg)
      {
      U_TRACE_CTOR(0, UServer<UUDPSocket>, "%p", cfg)

#  ifdef DEBUG
      if (cfg &&
          budp == false)
         {
         U_ERROR("You need to set budp var before loading the configuration");
         }
#  endif

      U_NEW(UUDPSocket, socket, UUDPSocket(UClientImage_Base::bIPv6));
      }

   virtual ~UServer()
      {
      U_TRACE_DTOR(0, UServer<UUDPSocket>)
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
      U_TRACE_NO_PARAM(0+256, "UServer<UUDPSocket>::preallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      vClientImage = new client_type[UNotifier::max_connection];
      }

#ifdef DEBUG
   virtual void deallocate() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0+256, "UServer<UUDPSocket>::deallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      delete[] (client_type*)vClientImage;
      }

   virtual bool check_memory() U_DECL_OVERRIDE { return u_check_memory_vector<client_type>((client_type*)vClientImage, UNotifier::max_connection); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UServer<UUDPSocket>)
};
#endif

#ifdef USERVER_IPC
template <> class U_EXPORT UServer<UUnixSocket> : public UServer_Base {
public:

   typedef UClientImage<UUnixSocket> client_type;

   UServer(UFileConfig* cfg) : UServer_Base(cfg)
      {
      U_TRACE_CTOR(0, UServer<UUnixSocket>, "%p", cfg)

#  ifdef DEBUG
      if (cfg &&
          bipc == false)
         {
         U_ERROR("You need to set bipc var before loading the configuration");
         }
#  endif

      U_NEW(UUnixSocket, socket, UUnixSocket(UClientImage_Base::bIPv6));
      }

   virtual ~UServer()
      {
      U_TRACE_DTOR(0, UServer<UUnixSocket>)
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
      U_TRACE_NO_PARAM(0+256, "UServer<UUnixSocket>::preallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      vClientImage = new client_type[UNotifier::max_connection];
      }

#ifdef DEBUG
   virtual void deallocate() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0+256, "UServer<UUnixSocket>::deallocate()")

      // NB: array are not pointers (virtual table can shift the address of this)...

      delete[] (client_type*)vClientImage;
      }

   virtual bool check_memory() U_DECL_OVERRIDE { return u_check_memory_vector<client_type>((client_type*)vClientImage, UNotifier::max_connection); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UServer<UUnixSocket>)
};
#endif
#endif
