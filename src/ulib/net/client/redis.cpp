// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    redis.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/tokenizer.h>
#include <ulib/net/client/redis.h>

uint32_t           UREDISClient_Base::start;
ptrdiff_t          UREDISClient_Base::diff;
UHashMap<void*>*   UREDISClient_Base::pchannelCallbackMap;
UVector<UString>*  UREDISClient_Base::pvec;
UREDISClient_Base* UREDISClient_Base::pthis;

UREDISClient_Base::~UREDISClient_Base()
{
   U_TRACE_DTOR(0, UREDISClient_Base)

   if (pchannelCallbackMap)
      {
      U_DELETE(pchannelCallbackMap)
               pchannelCallbackMap = U_NULLPTR;
      }
}

// Connect to REDIS server

void UREDISClient_Base::init()
{
   U_TRACE_NO_PARAM(0, "UREDISClient_Base::init()")

   U_DUMP("getRedisVersion() = %V", getRedisVersion().rep)

   clear();

   pthis = this;

   UClient_Base::response.clear();

   UClient_Base::reserve(UString::_getReserveNeed());

   UClient_Base::csocket = socket;
}

bool UREDISClient_Base::connect(const char* phost, unsigned int _port)
{
   U_TRACE(0, "UREDISClient_Base::connect(%S,%u)", phost, _port)

   UString host;

   if (phost) (void) host.assign(phost);
   else
      {
      const char* env_redis_host = (const char*) U_SYSCALL(getenv, "%S", "REDIS_HOST");

      if (env_redis_host == U_NULLPTR)
         {
         (void) UClient_Base::response.replace(U_CONSTANT_TO_PARAM("connection disabled"));

         U_RETURN(false);
         }

      (void) host.assign(env_redis_host, u__strlen(env_redis_host, __PRETTY_FUNCTION__));

      const char* env_redis_port = (const char*) U_SYSCALL(getenv, "%S", "REDIS_PORT");

      if (env_redis_port) _port = u_atoi(env_redis_port);
      }

   if (UClient_Base::setHostPort(host, _port) &&
       UClient_Base::connect())
      {
      init();

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString UREDISClient_Base::getInfoData(const char* section, const char* key, uint32_t len)
{
   U_TRACE(0, "UREDISClient_Base::getInfoData(%S,%.*S,%u)", section, len, key, len)

   if (info(section))
      {
      UString item = vitem[0];
      uint32_t pos = item.find(key, 0, len);

      if (pos != U_NOT_FOUND)
         {
         uint32_t n;
         const char* ptr = item.c_pointer(pos += len);

         for (n = 0; *ptr != '\r'; ++n, ++ptr) {}

         return item.substr(pos, n);
         }
      }

   return UString::getStringNull();
}

void UREDISClient_Base::manageResponseBufferResize(uint32_t n)
{
   U_TRACE(0, "UREDISClient_Base::manageResponseBufferResize(%u)", n)

   U_INTERNAL_DUMP("pthis->UClient_Base::response.size() = %u pthis->UClient_Base::response.capacity() = %u start = %u",
                    pthis->UClient_Base::response.size(),     pthis->UClient_Base::response.capacity(),     start)

   U_INTERNAL_ASSERT_MAJOR(n, 0)
   U_INTERNAL_ASSERT_MAJOR(start, 0)

   if (n < (64U * 1024U)) n = (64U * 1024U);

   if (pthis->UClient_Base::response.space() < n)
      {
      U_ASSERT(pthis->x.empty())

      UStringRep* rep = pthis->UClient_Base::response.rep;

      U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->_length = %u rep->_capacity = %u",
                       rep,     rep->parent,     rep->references,     rep->_length,     rep->_capacity)

      UStringRep* nrep = UStringRep::create(rep->_length, rep->_length+n, rep->data());

      if ((n = pthis->vitem.size()))
         {
         diff = nrep->data() - rep->data();

         U_INTERNAL_DUMP("diff = %ld", diff)

         UStringRep* r;
#     if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
         uint32_t ref = 0;
#     endif

         for (uint32_t i = 0; i < n; ++i)
            {
            r = pthis->vitem.UVector<UStringRep*>::at(i);

            if (r->isSubStringOf(rep))
               {
#           if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
               r->parent = nrep;

               ++ref;
#           endif

               r->shift(diff);
               }
            }

#     if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
         U_INTERNAL_DUMP("ref = %u rep->child = %u", ref, rep->child)

         nrep->child = rep->child;
                       rep->child = 0;
#     endif
         }

      pthis->UClient_Base::response._set(nrep);

      U_INTERNAL_ASSERT(pthis->UClient_Base::response.invariant())
      }
}

U_NO_EXPORT bool UREDISClient_Base::getResponseItem()
{
   U_TRACE_NO_PARAM(0, "UREDISClient_Base::getResponseItem()")

   char prefix;
   uint32_t len;
   const char* ptr1;
   const char* ptr2;
   
   // this start == size guards against reading upon every loop
   if (start == UClient_Base::response.size() &&
       UClient_Base::readResponse() == false)
   {
      U_RETURN(false);
   }

   U_INTERNAL_DUMP("start = %u (%.20S)", start, UClient_Base::response.c_pointer(start))

   U_INTERNAL_ASSERT(memcmp(UClient_Base::response.c_pointer(start), U_CRLF, U_CONSTANT_SIZE(U_CRLF)))

   prefix = UClient_Base::response.c_char(start++);

   U_INTERNAL_DUMP("prefix = %C", prefix)

   ptr1 =
   ptr2 = UClient_Base::response.c_pointer(start);

   if (prefix != U_RC_BULK &&    // "$15\r\nmy-value-tester\r\n"
       prefix != U_RC_MULTIBULK) // "*2\r\n$10\r\n1439822796\r\n$6\r\n311090\r\n"
   {
      U_INTERNAL_ASSERT(prefix == U_RC_ANY   || // '?'
                        prefix == U_RC_INT   || // ':'
                        prefix == U_RC_ERROR || // '-'
                        prefix == U_RC_INLINE)  // '+'

      while (*ptr2 != '\r') ++ptr2;

      len = ptr2-ptr1;
      
      // U_RC_INLINE example -> +OK\r\n
      // U_RC_INT    example -> :0\r\n
      // U_RC_ERROR  example -> -Error message\r\n

      pvec->push_back(UClient_Base::response.substr(ptr1, len));

      start += len + U_CONSTANT_SIZE(U_CRLF);

      U_RETURN(false);
   }

   if (ptr2[0] == '-')
   {
      U_INTERNAL_ASSERT_EQUALS(ptr2[1], '1')
      U_INTERNAL_ASSERT_EQUALS(prefix, U_RC_BULK) // "$-1\r\n" (Null Bulk String)

      pvec->push_back(UString::getStringNull());

      start += (ptr2-ptr1) + 2 + U_CONSTANT_SIZE(U_CRLF);

      U_RETURN(false);
   }

   len = u_strtoulp(&ptr2);

   ++ptr2;

   U_INTERNAL_DUMP("len = %u ptr2 = %#.2S", len, ptr2-2)

   U_INTERNAL_ASSERT_EQUALS(memcmp(ptr2-2, U_CRLF, U_CONSTANT_SIZE(U_CRLF)), 0)

   if (prefix == U_RC_BULK) // "$15\r\nmy-value-tester\r\n"
   {
      len += U_CONSTANT_SIZE(U_CRLF);

      while (len > (uint32_t)UClient_Base::response.remain(ptr2))
      {
         uint32_t d = UClient_Base::response.distance(ptr2);

         manageResponseBufferResize(len);

         if (UClient_Base::readResponse() == false)
         {
            U_RETURN(false);
         }

         ptr1 = UClient_Base::response.c_pointer(start);
         ptr2 = UClient_Base::response.c_pointer(d);
      }

      pvec->push_back(UClient_Base::response.substr(ptr2, len-U_CONSTANT_SIZE(U_CRLF)));

      start += (ptr2-ptr1) + len;

      U_RETURN(false);
   }

   /**
    * Ex: "*2\r\n$10\r\n1439822796\r\n$6\r\n311090\r\n"
    *
    * Only certain commands (especially those returning list of values) return multi-bulk replies, you can try by using LRANGE for example
    * but you can check the command reference for more details. Usually multi-bulk replies are only 1-level deep but some Redis commands can
    * return nested multi-bulk replies, notably EXEC (depending on the commands executed while inside the transaction context) and both
    * EVAL / EVALSHA (depending on the value returned by the Lua script). Here is an example using EXEC:
    *
    * redis 127.0.0.1:6379> MULTI
    * OK
    * redis 127.0.0.1:6379> LPUSH metavars foo foobar hoge
    * QUEUED
    * redis 127.0.0.1:6379> LRANGE metavars 0 -1
    * QUEUED
    * redis 127.0.0.1:6379> EXEC
    * 1) (integer) 4
    * 2) 1) "hoge"
    *    2) "foobar"
    *    3) "foo"
    *    4) "metavars"
    *
    * The second element of the multi-bulk reply to EXEC is a multi-bulk itself
    */

   U_INTERNAL_ASSERT_EQUALS(prefix, U_RC_MULTIBULK)

   start += (ptr2-ptr1);

   for (uint32_t i = 0; i < len; ++i)
   {
      getResponseItem();
   }
   
   U_RETURN(true);
}

void UREDISClient_Base::processResponse()
{
   U_TRACE_NO_PARAM(0, "UREDISClient_Base::processResponse()")

   U_INTERNAL_DUMP("err = %d", err)

   U_INTERNAL_ASSERT_EQUALS(err, U_RC_OK)

   start = 0;
   pvec  = &vitem;

   do {
      getResponseItem();

      U_DUMP_CONTAINER(vitem)
      }
   while (start < UClient_Base::response.size());
}

bool UREDISClient_Base::processRequest(char recvtype)
{
   U_TRACE(0, "UREDISClient_Base::processRequest(%C)", recvtype)

   if (UClient_Base::sendRequest(false) &&
       (clear(), UClient_Base::response.setEmpty(), UClient_Base::readResponse(U_SINGLE_READ)))
      {
      char prefix = UClient_Base::response[0];

      if (UNLIKELY(prefix == U_RC_ERROR))
         {
         err =  U_RC_ERROR;
         U_RETURN(false);
         }

      err = U_RC_OK;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UREDISClient_Base::processRequest(char recvtype, const char* p1, uint32_t len1, const char* p2, uint32_t len2)
{
   U_TRACE(0, "UREDISClient_Base::processRequest(%C,%.*S,%u,%.*S,%u)", recvtype, len1, p1, len1, len2, p2, len2)

   UClient_Base::iovcnt = 4;

   UClient_Base::iov[0].iov_base = (caddr_t)p1;
   UClient_Base::iov[0].iov_len  = len1;
   UClient_Base::iov[1].iov_base = (caddr_t)" ";
   UClient_Base::iov[1].iov_len  = 1;
   UClient_Base::iov[2].iov_base = (caddr_t)p2;
   UClient_Base::iov[2].iov_len  = len2;
   UClient_Base::iov[3].iov_base = (caddr_t)U_CRLF;
   UClient_Base::iov[3].iov_len  = 2;

   if (processRequest(recvtype))
      {
      processResponse();

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UREDISClient_Base::processRequest(char recvtype, const char* p1, uint32_t len1, const char* p2, uint32_t len2, const char* p3, uint32_t len3)
{
   U_TRACE(0, "UREDISClient_Base::processRequest(%C,%.*S,%u,%.*S,%u,%.*S,%u)", recvtype, len1, p1, len1, len2, p2, len2, len3, p3, len3)

   UClient_Base::iovcnt = 6;

   UClient_Base::iov[0].iov_base = (caddr_t)p1;
   UClient_Base::iov[0].iov_len  = len1;
   UClient_Base::iov[1].iov_base = (caddr_t)" ";
   UClient_Base::iov[1].iov_len  = 1;
   UClient_Base::iov[2].iov_base = (caddr_t)p2;
   UClient_Base::iov[2].iov_len  = len2;
   UClient_Base::iov[3].iov_base = (caddr_t)" ";
   UClient_Base::iov[3].iov_len  = 1;
   UClient_Base::iov[4].iov_base = (caddr_t)p3;
   UClient_Base::iov[4].iov_len  = len3;
   UClient_Base::iov[5].iov_base = (caddr_t)U_CRLF;
   UClient_Base::iov[5].iov_len  = 2;

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[5].iov_len, 2)

   if (processRequest(recvtype))
      {
      processResponse();

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UREDISClient_Base::processMultiRequest(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UREDISClient_Base::processMultiRequest(%.*S,%u)", fmt_size, format, fmt_size)

   UClient_Base::iovcnt = 1;

   UClient_Base::iov[0].iov_base = (caddr_t)"MULTI\r\n";
   UClient_Base::iov[0].iov_len  = 7;

   if (processRequest(U_RC_INLINE)) // +OK\r\n
      {
      U_INTERNAL_ASSERT_POINTER(format)
      U_INTERNAL_ASSERT_MAJOR(fmt_size, 0)
      U_INTERNAL_ASSERT(u_endsWith(format, fmt_size, U_CONSTANT_TO_PARAM(U_CRLF)))

      x = UString::getUBuffer();

      va_list argp;
      va_start(argp, fmt_size);

      x.vsnprintf(format, fmt_size, argp);

      va_end(argp);

      UString cmd;
      UTokenizer tok(x, "\r\n");

      while (tok.next(cmd, (bool*)U_NULLPTR))
         {
         UClient_Base::iov[0].iov_base = (caddr_t)u_buffer; // cmd.data();
         UClient_Base::iov[0].iov_len  = cmd.size()+U_CONSTANT_SIZE(U_CRLF);

         if (processRequest(U_RC_INLINE) == false) U_RETURN(false); // +QUEUED\r\n
         }

      UClient_Base::iov[0].iov_base = (caddr_t)"EXEC\r\n";
      UClient_Base::iov[0].iov_len  = 6;

      if (processRequest(U_RC_MULTIBULK)) // *2\r\n+OK\r\n-ERR...\r\n
         {
         processResponse();

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

bool UREDISClient_Base::scan(vPFcs function, const char* pattern, uint32_t len) // Returns all keys matching pattern (scan 0 MATCH *11*)
{
   U_TRACE(0, "UREDISClient_Base::scan(%p,%.*S,%u)", function, len, pattern, len)

   /**
    * You start by giving a cursor value of 0; each call returns a new cursor value which you pass into the next SCAN call. A value of 0 indicates iteration is finished.
    * Supposedly no server or client state is needed (except for the cursor value)
    */

   char buf[4096];
   uint32_t buf_len = u__snprintf(buf, U_CONSTANT_SIZE(buf), U_CONSTANT_TO_PARAM("MATCH %.*s COUNT 500"), len, pattern);

   if (processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SCAN"), U_CONSTANT_TO_PARAM("0"), buf, buf_len))
      {
      char ncursor[22];
      UVector<UString> vec;
      uint32_t i, n, cursor = getULong();

loop: if (setMultiBulk(vec))
         {
         for (i = 0, n = vec.size(); i < n; ++i) function(vec[i]);
         }

      if (cursor == 0) U_RETURN(true);

      if (processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SCAN"), ncursor, u_num2str32(cursor,ncursor)-ncursor, buf, buf_len))
         {
         cursor = getULong();

         vec.clear();

         goto loop;
         }
      }

   U_RETURN(false);
}

bool UREDISClient_Base::deleteKeys(const char* pattern, uint32_t len) // Delete all keys matching pattern
{
   U_TRACE(0, "UREDISClient_Base::deleteKeys(%.*S,%u)", len, pattern, len)

   char buf[4096], ncursor[22];
   uint32_t cursor = 0, buf_len = u__snprintf(buf, U_CONSTANT_SIZE(buf), U_CONSTANT_TO_PARAM("MATCH %.*s COUNT 1000"), len, pattern);

   while (processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SCAN"), ncursor, u_num2str32(cursor,ncursor)-ncursor, buf, buf_len))
      {
      cursor = getULong();

      if (setMultiBulk())
         {
         x.unQuote();

         if (del(x) == false) U_RETURN(false);
         }

      if (cursor == 0) U_RETURN(true);
      }

   U_RETURN(true);
}

// define method VIRTUAL of class UEventFd

int UREDISClient_Base::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UREDISClient_Base::handlerRead()")

   if ((clear(), UClient_Base::response.setEmpty(), UClient_Base::readResponse(U_SINGLE_READ)))
      {
      char prefix = UClient_Base::response[0];

      if (prefix != U_RC_MULTIBULK)
         {
         err = (prefix == U_RC_ERROR ? U_RC_ERROR
                                     : U_RC_ERR_PROTOCOL);

         U_RETURN(false);
         }

      err = U_RC_OK;

      processResponse();

      UString channel = vitem[1];

      U_INTERNAL_ASSERT_POINTER(pchannelCallbackMap)

      vPFcscs callback = (vPFcscs) pchannelCallbackMap->at(channel); 

      if (callback) callback(channel, vitem[2]);
      }

   U_RETURN(U_NOTIFIER_OK);
}

// by Victor Stewart

#if defined(U_STDCPP_ENABLE) && defined(U_LINUX)
#  if defined(HAVE_CXX17)

static ClusterError checkResponseForClusterErrors(const UString& response, size_t offset)
{
   U_TRACE_NO_PARAM(0, "checkResponseForClusterErrors()")

   // all of these errors are very rare, and only occur in the midst of cluster topology changes

   // -MOVED 3999 127.0.0.1:6381 => the hashslot has been moved to another master node
   if (UNLIKELY(response.find("-MOVED", offset, 6) != U_NOT_FOUND))    return ClusterError::moved;

   // -ASK 3999 127.0.0.1:6381 => this means that one of the hash slots is being migrated to another server
   if (UNLIKELY(response.find("-ASK", offset, 4) != U_NOT_FOUND))      return ClusterError::ask;

   // during a resharding the multi-key operations targeting keys that all exist and are all still in the same node (either the source or destination node) are still available.
   // Operations on keys that don't exist or are - during the resharding - split between the source and destination nodes, will generate a -TRYAGAIN error. The client can try
   // the operation after some time, or report back the error. As soon as migration of the specified hash slot has terminated, all multi-key operations are available again for
   // that hash slot
   if (UNLIKELY(response.find("-TRYAGAIN", offset, 9) != U_NOT_FOUND)) return ClusterError::tryagain;

   return ClusterError::none;
}

void UREDISClusterMaster::calculateNodeMap()
{
   U_TRACE_NO_PARAM(0, "UREDISClusterMaster::calculateNodeMap()")

   /*
   127.0.0.1:30001> cluster slots
   1) 1) (integer) 0
      2) (integer) 5460
      3) 1) "127.0.0.1"
         2) (integer) 30001
         3) "09dbe9720cda62f7865eabc5fd8857c5d2678366"
      4) 1) "127.0.0.1"
         2) (integer) 30004
         3) "821d8ca00d7ccf931ed3ffc7e3db0599d2271abf"
   2) 1) (integer) 5461
      2) (integer) 10922
      3) 1) "127.0.0.1"
         2) (integer) 30002
         3) "c9d93d9f2c0c524ff34cc11838c2003d8c29e013"
      4) 1) "127.0.0.1"
         2) (integer) 30005
         3) "faadb3eb99009de4ab72ad6b6ed87634c7ee410f"
   3) 1) (integer) 10923
      2) (integer) 16383
      3) 1) "127.0.0.1"
         2) (integer) 30003
         3) "044ec91f325b7595e76dbcb18cc688b6a5b434a1"
      4) 1) "127.0.0.1"
         2) (integer) 30006
         3) "58e6e48d41228013e5d9c1c37c5060693925e97e"
   */

   // the first node in each array is the master

   bool findHashSlots = true;
   uint16_t workingLowHashSlot;
   uint16_t workingHighHashSlot;

   (void) managementClient->processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("CLUSTER SLOTS"));
   
   UHashMap<RedisClusterNode *> *newNodes;
   U_NEW(UHashMap<RedisClusterNode *>, newNodes, UHashMap<RedisClusterNode *>);

   const UVector<UString>& rawNodes = managementClient->vitem;

   for (uint32_t a = 0, b = rawNodes.size(); a < b; a+=2)
   {
      const UString& first = rawNodes[a];
      const UString& second = rawNodes[a+1];

      if (findHashSlots)
      {
         if (first.isNumber() && second.isNumber())
         {
            workingLowHashSlot = first.strtoul();
            workingHighHashSlot = second.strtoul();

            findHashSlots = false;
         }
      }
      else
      {
         UString compositeAddress(50U);
         compositeAddress.snprintf(U_CONSTANT_TO_PARAM("%v.%v"), first.rep, second.rep);

         RedisClusterNode *workingNode = clusterNodes ? clusterNodes->erase(compositeAddress) : U_NULLPTR;

         // in the case of MOVE some nodes will be new, but others we'll already be connected to
         if (workingNode)
         {  
            workingNode->lowHashSlot = workingLowHashSlot;
            workingNode->highHashSlot = workingHighHashSlot;
         }
         else
         {  
            U_NEW(RedisClusterNode, workingNode, RedisClusterNode(first, second.strtoul(), workingLowHashSlot, workingHighHashSlot));
         }
         
         newNodes->insert(compositeAddress, workingNode);

         findHashSlots = true;
      }
   }

   // if any nodes were taken offline, the clients would've disconnected by default
   if (clusterNodes) U_DELETE(clusterNodes);

   clusterNodes = newNodes;
   managementClient->UClient_Base::response.setEmpty();
}

bool UREDISClusterMaster::connect(const char* host, unsigned int _port)
{
   U_TRACE(0, "UREDISClusterMaster::connect(%S,%u)", host, _port)

   if (managementClient->connect(host, _port))
   {
      calculateNodeMap();
      
      RedisClusterNode *randomNode = clusterNodes->randomElement();

      if (randomNode)
         {
         subscriptionClient->connect(randomNode->ipAddress.c_str(), randomNode->port);
        
         U_INTERNAL_ASSERT_EQUALS(UREDISClient_Base::pchannelCallbackMap, U_NULLPTR)

         U_NEW(UHashMap<void*>, UREDISClient_Base::pchannelCallbackMap, UHashMap<void*>());

         subscriptionClient->UEventFd::fd = subscriptionClient->getFd();
         subscriptionClient->UEventFd::op_mask |=  EPOLLET;
         
         UServer_Base::addHandlerEvent(subscriptionClient);
        
         U_RETURN(true);
         }
   }

   U_RETURN(false);
}

bool UREDISClusterMaster::clusterUnsubscribe(const UString& channel) // unregister the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterMaster::clusterUnsubscribe(%V)", channel.rep)

   if (subscriptionClient->processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("UNSUBSCRIBE"), U_STRING_TO_PARAM(channel)))
   {
      (void)subscriptionClient->UREDISClient_Base::pchannelCallbackMap->erase(channel);

      U_RETURN(true);
   }

   U_RETURN(false);
}

bool UREDISClusterMaster::clusterSubscribe(const UString& channel, vPFcscs callback) // register the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterMaster::clusterSubscribe(%V,%p)", channel.rep, callback)

   if (subscriptionClient->processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SUBSCRIBE"), U_STRING_TO_PARAM(channel)))
   {
      subscriptionClient->UREDISClient_Base::pchannelCallbackMap->insert(channel, (const void*)callback);

      U_RETURN(true);
   }

   U_RETURN(false);
}

void UREDISClusterMaster::sendToCluster(UREDISClusterClient*& workingClient, const UString& hashableKey, const UString& pipeline)
   {
      ClusterError error;
      workingClient = clientForHashableKey(hashableKey);

   retry:
      
      workingClient->clear();

      workingClient->UREDISClient_Base::sendRequest(pipeline);

      workingClient->UClient_Base::response.setEmpty();
      workingClient->UClient_Base::readResponse(U_SINGLE_READ);

      error = checkResponseForClusterErrors(workingClient->UClient_Base::response, 0);

      while (error != ClusterError::none)
      {
         switch (error)
         {
            case ClusterError::moved:
            {
               calculateNodeMap();
               workingClient = clientForHashableKey(hashableKey);
               break;
            }
            case ClusterError::ask:
            {
               uint32_t _start = workingClient->UClient_Base::response.find(' ', U_CONSTANT_SIZE("-ASK 3999")) + 1,
                           end = workingClient->UClient_Base::response.find(':', _start);

               workingClient = clientForIP(workingClient->UClient_Base::response.substr(_start, end - _start));
               break;
            }
            case ClusterError::tryagain:
            {
               UTimeVal(0L, 1000L).nanosleep(); // 0 sec, 1000 microsec = 1ms
               break;
            }
            case ClusterError::none: break;
         }

         goto retry;
      }

      workingClient->UREDISClient_Base::processResponse();
   }

static void getNextMark(const UString& string, size_t& marker)
{
   int8_t depth = 0;
   char prefix;
   const char *pointer1, *pointer2;

loop:
   
   prefix = string.c_char(marker++);
   pointer1 = pointer2 = string.c_pointer(marker);

   switch (prefix)
   {
      // :0\r\n
      case U_RC_INT:
      // +OK\r\n
      case U_RC_INLINE:
      // -MOVED 3999 127.0.0.1:6381\r\n
      case U_RC_ERROR:
      {
         while (*pointer2 != '\r') ++pointer2;
         marker += (pointer2-pointer1) + U_CONSTANT_SIZE(U_CRLF);
         break;
      }
      // $-1\r\n                       (Null Bulk String)
      // $15\r\nmy-value-tester\r\n
      case U_RC_BULK:
      {
         if (*pointer2 == '-')
         {
            marker += 2 + U_CONSTANT_SIZE(U_CRLF);
         }
         else
         {
            while (*pointer2 != '\r') ++pointer2;
            marker += (pointer2-pointer1) + U_CONSTANT_SIZE(U_CRLF) + u_strtol(pointer1, pointer2) + U_CONSTANT_SIZE(U_CRLF);
         }
         
         break;
      }
      // *2\r\n$10\r\n1439822796\r\n$6\r\n311090\r\n
      case U_RC_MULTIBULK:
      {
         while (*pointer2 != '\r') ++pointer2;
         depth += u_strtol(pointer1, pointer2);
         break;
      }
   }

   depth--;
   
   if (depth > -1)  goto loop;
}

const UVector<UString>& UREDISClusterMaster::clusterAnonMulti(const AnonymousClusterPipeline& pipeline, const bool reorderable)
{
   U_TRACE(0, "UREDISClusterMaster::clusterAnonMulti(%b)", reorderable)

   U_DUMP("pipeline = %v", pipeline.pipeline.rep);

   static UString workingString(500U);
   workingString.reserve(pipeline.pipeline.size() * 1.5);

   // if reorderable == false, commands are grouped and pushed SEQUENTIALLY BY HASHSLOT. even if other commands point to hashslots on the same cluster node, we are unable to garuntee ordering since Redis only checks for -MOVED etc errors command by command as it executes them, and does not fail upon reaching a -MOVED etc error. this requires waiting for each response, to ensure no errors occured, before moving onto the next batch of commands.
   if (reorderable == false)
   {
      auto it = pipeline.spans.begin();
      uint16_t workingHashslot = it->hashslot;
      UREDISClusterClient* workingClient = clientForHashslot(workingHashslot);

      goto startNoReorder;

      while (it != pipeline.spans.end())
      {
         if (workingHashslot == it->hashslot)
         {
   startNoReorder:
            workingString.append(pipeline.pipeline.data() + it->beginning, it->end - it->beginning);

            it++;

            // if end, fall through
            if (UNLIKELY(it == pipeline.spans.end())) goto retryPush;
         }
         else
         {
         retryPush:

            // push
            workingClient->sendRequest(workingString);
            workingClient->UClient_Base::response.setEmpty();
            workingClient->UClient_Base::readResponse();

         // bweare: if the wrong hashable key is passed, this will loop forever.
            switch (checkResponseForClusterErrors(workingClient->UClient_Base::response, 0))
            {
               case ClusterError::none:
               {
                  U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::none", reorderable);

                  workingString.setEmpty();

                  UString& workingClientBuffer = workingClient->UClient_Base::response;
                  UString& masterBuffer = managementClient->UClient_Base::response;
                  masterBuffer.reserve(masterBuffer.size() + workingClientBuffer.size());

                  masterBuffer.append(workingClientBuffer);
                  workingClientBuffer.setEmpty();

                  if (LIKELY(it != pipeline.spans.end()))
                  {
                     workingHashslot = it->hashslot;
                     workingClient = clientForHashslot(workingHashslot);
                     goto startNoReorder;
                  }
                  else goto finish;

                  break;
               }
               case ClusterError::moved:
               {
                  U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::moved", reorderable);

                  workingClient->UClient_Base::response.setEmpty();

                  calculateNodeMap();

                  workingClient = clientForHashslot(workingHashslot);
                  break;
               }
               case ClusterError::ask:
               {
                  U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::ask", reorderable);

                  // -ASK 3999 127.0.0.1:6381
                  uint32_t _start = workingClient->UClient_Base::response.find(' ', U_CONSTANT_SIZE("-ASK 3999")) + 1,
                              end = workingClient->UClient_Base::response.find(':', _start);

                  const UString& ip = workingClient->UClient_Base::response.substr(_start, end - _start);

                  workingClient->UClient_Base::response.setEmpty();

                  workingClient = clientForIP(ip);
                  break;
               }
               case ClusterError::tryagain:
               {
                  U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::tryagain", reorderable);

                  UTimeVal(0L, 1000L).nanosleep(); // 0 sec, 1000 microsec = 1ms
                  break;
               }
            }

            goto retryPush;
         }
      }
   }
   // if reorderable == true, we are able to group and push commands BY NODE regardless of sequence. we assume the fast-path 99.999% occurance that -MOVED and other errors did not occur, and push commands to redis as rapidly as possible without waiting on responses. we same a copy of all commands, and then at the end, check for -MOVED or other errors, and correct those if need be, while we process all resposnes.
   else
   {
      // so we go through rapid fire... pushing all commands grouped by node... then at the end we check for failures, and if so we just repush them one at a time ?

      struct ExecutionMapping {

         UREDISClusterClient *client;
         size_t ordering;

         ExecutionMapping(UREDISClusterClient *_client, size_t _ordering) : client(_client), ordering(_ordering) {}
      };

      // copy spans
      std::unordered_map<UREDISClusterClient*, bool> touchedClients;
      std::vector<AnonymousClusterPipeline::Span> spans = pipeline.spans;
      std::unordered_map<size_t, ExecutionMapping> spanToExeuction;
      auto it = spans.begin();
      UREDISClusterClient* workingClient;
      size_t executionCount;

   rinseAndRepeat:

      workingString.setEmpty();
      it = spans.begin();
      workingClient = clientForHashslot(it->hashslot);
      executionCount = 0;
      goto startReorder;

      while (it != spans.end())
      {  
         if (clientForHashslot(it->hashslot) == workingClient)
         {
      startReorder:
            workingString.append(pipeline.pipeline.data() + it->beginning, it->end - it->beginning);
            
            spanToExeuction.insert_or_assign(it->index, ExecutionMapping(workingClient, executionCount++));

            it = spans.erase(it);
         }
         else it++;
      }

      touchedClients.insert_or_assign(workingClient, true);
      workingClient->sendRequest(workingString);
      // we'll get here once having exhausted the commands for a given node
      if (spans.size() > 0) goto rinseAndRepeat;

      struct Mark {

         size_t beginning, ending;
      };

      size_t totalBufferSize = 0;
      std::unordered_map<UREDISClusterClient*, std::vector<Mark>> marksByClient;

      for (const auto& [touchedClient, meaninglessFlag] : touchedClients)
      {     
         touchedClient->UClient_Base::readResponse();
         UString& clientBuffer = touchedClient->UClient_Base::response;

         size_t clientBufferSize = clientBuffer.size();
         totalBufferSize += clientBufferSize;

         size_t marker = 0;
         std::vector<Mark>& marks = marksByClient[touchedClient];

         while (marker < clientBufferSize)
         {
            size_t beginning = marker;
            getNextMark(clientBuffer, marker);

            U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), mark, [%lu, %lu)", reorderable, beginning, marker);

            marks.push_back({beginning, marker});
         }
      }

      UString& masterBuffer = managementClient->UClient_Base::response;
      masterBuffer.reserve(totalBufferSize);

      for (size_t index = 0; index < pipeline.spans.size(); index++)
      {
         const ExecutionMapping& mapping = spanToExeuction.at(index);
         UString* clientBuffer = &mapping.client->UClient_Base::response;

         Mark& mark = marksByClient[mapping.client][mapping.ordering];
         size_t beginning = mark.beginning;
         size_t ending = mark.ending;

      checkAgain:

         workingClient = 0;

         switch (checkResponseForClusterErrors(*clientBuffer, beginning))
         {
            case ClusterError::none:
            {
               U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::none", reorderable);

               masterBuffer.append(clientBuffer->data() + beginning, ending - beginning);

               continue;
            }
            case ClusterError::moved:
            {
               U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::moved", reorderable);

               calculateNodeMap();
               break;
            }
            case ClusterError::ask:
            {
               U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::ask", reorderable);

               uint32_t _start = clientBuffer->find(' ', beginning + U_CONSTANT_SIZE("-ASK 3999")) + 1,
                           _end = clientBuffer->find(':', _start);

               workingClient = clientForIP(clientBuffer->substr(_start, _end - _start));
               break;
            }
            case ClusterError::tryagain:
            {
               U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), ClusterError::tryagain", reorderable);
               UTimeVal(0L, 1000L).nanosleep(); // 0 sec, 1000 microsec = 1ms
               break;
            }
         }

         const AnonymousClusterPipeline::Span& span = pipeline.spans[index];

         if (!workingClient) workingClient = clientForHashslot(span.hashslot);

         clientBuffer = &workingClient->UClient_Base::response;

         beginning = clientBuffer->size();

         workingString.setEmpty();
         workingString.append(pipeline.pipeline.data() + span.beginning, span.end - span.beginning);

         workingClient->UREDISClient_Base::sendRequest(workingString);
         workingClient->UClient_Base::readResponse();

         ending = clientBuffer->size();

         goto checkAgain;
      }
   }
finish:
   
   U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), finished. masterBuffer = %v", reorderable, managementClient->UClient_Base::response.rep);

   managementClient->UREDISClient_Base::processResponse();
   
   return managementClient->vitem;
}
#  endif
#endif

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UREDISClient_Base::dump(bool _reset) const
{
   UClient_Base::dump(false);

   *UObjectIO::os << '\n'
                  << "err                                 " << err           << '\n'
                  << "vitem          (UVector             " << (void*)&vitem << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
# endif
