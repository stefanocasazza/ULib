// userver.cpp

#include <ulib/file_config.h>

#ifdef U_SSL_SOCKET
#  ifdef USERVER_IPC
#  undef USERVER_IPC
#  endif
#  ifdef USERVER_UDP
#  undef USERVER_UDP
#  endif
#  include <ulib/ssl/net/sslsocket.h>
#  define Server UServer<USSLSocket>
#elif defined(U_UDP_SOCKET)
#  ifdef USERVER_IPC
#  undef USERVER_IPC
#  endif
#  include <ulib/net/udpsocket.h>
#  define Server UServer<UUDPSocket>
#elif defined(U_UNIX_SOCKET)
#  ifdef USERVER_IPC
#  undef USERVER_IPC
#  endif
#  ifdef USERVER_UDP
#  undef USERVER_UDP
#  endif
#  include <ulib/net/unixsocket.h>
#  define Server UServer<UUnixSocket>
#elif defined(U_TCP_SOCKET)
#  ifdef USERVER_IPC
#  undef USERVER_IPC
#  endif
#  ifdef USERVER_UDP
#  undef USERVER_UDP
#  endif
#  include <ulib/net/tcpsocket.h>
#  define Server UServer<UTCPSocket>
#else
#  error "you must define the socket type (U_SSL_SOCKET | U_TCP_SOCKET | U_UNIX_SOCKET | U_UDP_SOCKET)"
#endif

#include <ulib/net/server/server.h>

#undef  PACKAGE
#define PACKAGE "userver"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose 'application server by ULib' \n" \
"option c config 1 'path of configuration file' ''\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      server = U_NULLPTR;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      U_DELETE(server)
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT(U_SYSCONFDIR "/userver.cfg");

      cfg.UFile::setPath(cfg_str);

      // ---------------------------------------------------------------------------------------------------------------------------------------
      // userver - configuration parameters
      // ---------------------------------------------------------------------------------------------------------------------------------------
      // ENABLE_IPV6    flag to indicate the use of ipv6
      // SERVER         host name or ip address for the listening socket
      // PORT           port number             for the listening socket
      // SOCKET_NAME    file name               for the listening socket
      // IP_ADDRESS     ip address of host for the interface connected to the Internet (autodetected if not specified)
      //
      // ALLOWED_IP            list of comma separated client address for IP-based access control (IPADDR[/MASK])
      // ALLOWED_IP_PRIVATE    list of comma separated client private address for IP-based access control (IPADDR[/MASK]) for public server
      // ENABLE_RFC1918_FILTER reject request from private IP to public server address
      //
      // MIN_SIZE_FOR_SENDFILE for major size it is better to use sendfile() to serve static content
      //
      // LISTEN_BACKLOG             max number of ready to be delivered connections to accept()
      // SET_REALTIME_PRIORITY      flag indicating that the preforked processes will be scheduled under the real-time policies SCHED_FIFO
      //
      // CLIENT_THRESHOLD           min number of clients to active polling
      // CLIENT_FOR_PARALLELIZATION minum number of clients to active parallelization 
      //
      // LOAD_BALANCE_CLUSTER           list of comma separated IP address (IPADDR[/MASK]) to define the load balance cluster
      // LOAD_BALANCE_DEVICE_NETWORK    network interface name of cluster of physical server
      // LOAD_BALANCE_LOADAVG_THRESHOLD system load threshold to proxies the request on other userver on the network cluster ([0-9].[0-9])
      //
      // PID_FILE       write main process pid on file indicated
      // WELCOME_MSG    message of welcome to send initially to client connected
      // RUN_AS_USER    downgrade security to that user account
      // DOCUMENT_ROOT  The directory out of which we will serve your documents
      //
      // LOG_FILE         locations for file log
      // LOG_FILE_SZ    memory size for file log
      // LOG_MSG_SIZE   limit length of printed log entry to LOG_MSG_SIZE chars (default 128)
      //
      // PLUGIN         list of plugins to load, a flexible way to add specific functionality to the server
      // PLUGIN_DIR     directory where there are plugins to load
      //
      // REQ_TIMEOUT    timeout for request from client connected
      // TCP_KEEP_ALIVE Specifies to active the TCP keepalive implementation in the linux kernel
      // TCP_LINGER_SET Specifies how the TCP initiated the close
      // MAX_KEEP_ALIVE Specifies the maximum number of requests that can be served through a Keep-Alive (Persistent) session. (Value <= 0 will disable Keep-Alive)
      //
      // DH_FILE        DH param
      // CERT_FILE      certificate of server
      // KEY_FILE       private key of server
      // PASSWORD       password for private key of server
      // CA_FILE        locations of trusted CA certificates used in the verification
      // CA_PATH        locations of trusted CA certificates used in the verification
      // VERIFY_MODE    mode of verification (SSL_VERIFY_NONE=0, SSL_VERIFY_PEER=1, SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2, SSL_VERIFY_CLIENT_ONCE=4)
      // CIPHER_SUITE   cipher suite model (Intermediate=0, Modern=1, Old=2)
      //
      // PREFORK_CHILD  number of child server processes created at startup ( 0 - serialize, no forking
      //                                                                      1 - classic, forking after accept client)
      //                                                                     >1 - pool of process serialize plus monitoring process)
      //
      // CRASH_COUNT         this is the threshold for the number of crash of child server processes
      // CRASH_EMAIL_NOTIFY  the email address to send a message whenever the number of crash > CRASH_COUNT
      // ---------------------------------------------------------------------------------------------------------------------------------------
      // This directive are for evasive action in the event of an HTTP DoS or DDoS attack or brute force attack
      // ---------------------------------------------------------------------------------------------------------------------------------------
      // DOS_PAGE_COUNT      this is the threshold for the number of requests for the same page (or URI) per page interval
      // DOS_PAGE_INTERVAL   the interval for the page count threshold; defaults to 1 second intervals
      // DOS_SITE_COUNT      this is the threshold for the total number of requests for any object by the same client per site interval
      // DOS_SITE_INTERVAL   the interval for the site count threshold; defaults to 1 second intervals
      // DOS_BLOCKING_PERIOD the blocking period is the amount of time (in seconds) that a client will be blocked for if they are added to the blocking list (defaults to 10)
      // DOS_WHITE_LIST      list of comma separated IP addresses of trusted clients can be whitelisted to insure they are never denied (IPADDR[/MASK])
      // DOS_EMAIL_NOTIFY    the email address to send a message whenever an IP address becomes blacklisted
      // DOS_SYSTEM_COMMAND  the system command specified will be executed whenever an IP address becomes blacklisted. Use %.*s to denote the IP address of the blacklisted IP 
      // DOS_LOGFILE         the file to write DOS event
      // ---------------------------------------------------------------------------------------------------------------------------------------

#  if defined(U_SSL_SOCKET)
      UServer_Base::bssl = true;
#  elif defined(U_UDP_SOCKET)
      UServer_Base::budp = true;
#  elif defined(U_UNIX_SOCKET)
      UServer_Base::bipc = true;
#  elif defined(U_TCP_SOCKET) && defined(USERVER_RNG)
      UServer_Base::brng = true;
#  elif defined(USE_FSTACK)
      UString x = cfg.at(U_CONSTANT_TO_PARAM("FSTACK_ARG"));

      if (x.empty())
         {
         if (UFile::access("config.ini", R_OK) == false)
            {
            U_ERROR("file ./config.ini is missing, exiting...");
            }

         x = UString((void*)U_CONSTANT_TO_PARAM("-c config.ini -p 0 -t auto"));
         }

      const char* argp[10];
      uint32_t n = u_split(U_STRING_TO_PARAM(x), (char**)argp+1, U_NULLPTR);

      argp[0] = PACKAGE;

      if (U_FF_SYSCALL(init, "%u,%p", n+1, (char* const*)argp)) // NB: load and parse ./config.ini
       {
       U_ERROR("Sorry, ff_init() failed, exiting...");
       }
#  endif

      server = new Server(&cfg);

#if defined(USE_FSTACK) && !defined(U_SSL_SOCKET) && !defined(U_UDP_SOCKET) && !defined(U_UNIX_SOCKET)
      U_FF_SYSCALL_VOID(run, "%p,%p", UServer_Base::ff_run, U_NULLPTR);
#  else
      UServer_Base::run();
#  endif
      }

private:
   Server* server;
   // NB: we put this here to avoid unnecessary destructor at runtime...
   UString cfg_str;
   UFileConfig cfg;

#ifndef U_COVERITY_FALSE_POSITIVE
   U_DISALLOW_COPY_AND_ASSIGN(Application)
#endif
};

U_MAIN
