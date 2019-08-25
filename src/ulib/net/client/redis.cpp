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

      if (len     != 1   ||
          ptr1[0] != '0' ||
          prefix != U_RC_INT)
         {
         pvec->push_back(UClient_Base::response.substr(ptr1, len));
         }

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

   UVector<UString> vec1(len);
   UVector<UString>* pvec1 = pvec;
                             pvec = &vec1;

   start += (ptr2-ptr1);

   for (uint32_t i = 0; i < len; ++i)
      {
      if (getResponseItem() == false)
         {
         if (UClient_Base::isConnected() == false)
            {
            (void) UClient_Base::connect();

            U_RETURN(false);
            }

         pvec1->move(vec1);
         }
      else
         {
         typedef UVector<UString> uvectorstring;

         char buffer_output[64U * 1024U];
         uint32_t buffer_output_len = UObject2String<uvectorstring>(vec1, buffer_output, sizeof(buffer_output));

         pvec1->push_back(UStringRep::create(buffer_output_len, buffer_output_len, (const char*)buffer_output));

         vec1.clear();
         }
      }

   pvec = pvec1;

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

      if (  prefix != recvtype &&
          recvtype != U_RC_ANY)
         {
         err = (prefix == U_RC_ERROR ? U_RC_ERROR
                                     : U_RC_ERR_PROTOCOL);

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

   U_INTERNAL_ASSERT_POINTER(pchannelCallbackMap)

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

      vPFcscs callback = (vPFcscs) pchannelCallbackMap->at(channel); 

      if (callback) callback(channel, vitem[2]);
      }

   U_RETURN(U_NOTIFIER_OK);
}

#if defined(U_STDCPP_ENABLE)

// by Victor Stewart

#  if defined(HAVE_CXX17)
void UREDISClusterClient::calculateNodeMap()
{
   U_TRACE_NO_PARAM(0, "UREDISClusterClient::calculateNodeMap()")

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

   bool findHashSlots = true;
   uint16_t workingLowHashSlot;
   uint16_t workingHighHashSlot;

   (void) UREDISClient_Base::processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("CLUSTER SLOTS"));

   const UVector<UString>& rawNodes = UREDISClient_Base::vitem;

   for (uint32_t a = 0, b = rawNodes.size(); a < b; ++a)
      {
      if (findHashSlots)
         {
         if (rawNodes[a].isNumber() &&
             rawNodes[a+1].isNumber())
            {
             workingLowHashSlot = rawNodes[a++].strtoul();
            workingHighHashSlot = rawNodes[a].strtoul();

            findHashSlots = false;
            }
         }
      else
         {
         // the immediate next after hash slot is the master

         RedisNode workingNode;

         workingNode.lowHashSlot  =  workingLowHashSlot;
         workingNode.highHashSlot = workingHighHashSlot;

         (void) workingNode.ipAddress.replace(rawNodes[a]);

         workingNode.port = rawNodes[++a].strtoul();

         workingNode.client.connect(workingNode.ipAddress.data(), workingNode.port);

         redisNodes.push_back(std::move(workingNode));

         findHashSlots = true;
         }
      }
}

bool UREDISClusterClient::connect(const char* host, unsigned int _port)
{
   U_TRACE(0, "UREDISClusterClient::connect(%S,%u)", host, _port)

   if (UREDISClient<UTCPSocket>::connect(host, _port))
      {
      calculateNodeMap();

      // select random master node to be responsible for SUB/PUB traffic

      const RedisNode& node = redisNodes[u_get_num_random_range0(redisNodes.size())];

      subscriptionClient.connect(node.ipAddress.data(), node.port);

      subscriptionClient.UEventFd::op_mask |=  EPOLLET;
      subscriptionClient.UEventFd::op_mask &= ~EPOLLRDHUP;

      UNotifier::insert(&subscriptionClient, EPOLLEXCLUSIVE | EPOLLROUNDROBIN); // NB: we ask to listen for events to a Redis publish channel... 

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UREDISClusterClient::clusterUnsubscribe(const UString& channel) // unregister the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterClient::clusterUnsubscribe(%V)", channel.rep)

   if (subscriptionClient.processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("UNSUBSCRIBE"), U_STRING_TO_PARAM(channel)))
      {
      if (pchannelCallbackMap == U_NULLPTR)
         {
         U_NEW(UHashMap<void*>, pchannelCallbackMap, UHashMap<void*>);
         }

      (void) pchannelCallbackMap->erase(channel);

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UREDISClusterClient::clusterSubscribe(const UString& channel, vPFcscs callback) // register the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterClient::clusterSubscribe(%V,%p)", channel.rep, callback)

   if (subscriptionClient.processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SUBSCRIBE"), U_STRING_TO_PARAM(channel)))
      {
      if (pchannelCallbackMap == U_NULLPTR)
         {
         U_NEW(UHashMap<void*>, pchannelCallbackMap, UHashMap<void*>);
         }

      pchannelCallbackMap->insert(channel, (const void*)callback);

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UREDISClusterClient::processResponse()
{
   U_TRACE_NO_PARAM(0, "UREDISClusterClient::processResponse()")

   if (UClient_Base::response.find("MOVED", 0, 5) != U_NOT_FOUND)
      {
      // MOVED 3999 127.0.0.1:6381 => the hashslot has been moved to another master node

      error = ClusterError::moved;

      calculateNodeMap();
      }
   else if (UClient_Base::response.find("ASK", 0, 3) != U_NOT_FOUND)
      {
      // ASK 3999 127.0.0.1:6381 => this means that one of the hash slots is being migrated to another server

      error = ClusterError::ask;

      uint32_t _start = UClient_Base::response.find(' ', 8) + 1,
                  end = UClient_Base::response.find(':', _start);

      (void) temporaryASKip.replace(UClient_Base::response.substr(_start, end - _start));
      }

   else if (UClient_Base::response.find("TRYAGAIN", 0, 8) != U_NOT_FOUND)
      {
      /**
       * during a resharding the multi-key operations targeting keys that all exist and are all still in the same node (either the source or destination node) are still available.
       * Operations on keys that don't exist or are - during the resharding - split between the source and destination nodes, will generate a -TRYAGAIN error. The client can try
       * the operation after some time, or report back the error. As soon as migration of the specified hash slot has terminated, all multi-key operations are available again for
       * that hash slot
       */

      error = ClusterError::tryagain;

      UTimeVal(0L, 1000L).nanosleep(); // 0 sec, 1000 microsec = 1ms
      }
   else
      {
      error = ClusterError::none;

      UREDISClient<UTCPSocket>::processResponse();
      }
}

template const UVector<UString>& UREDISClusterClient::pripeline<true>(UString& pipeline, bool reorderable);
template const UVector<UString>& UREDISClusterClient::processPipeline<false>(UString& pipeline, bool reorderable);

template <bool silence>
const UVector<UString>& UREDISClusterClient::processPipeline(UString& pipeline, const bool reorderable)
{
   U_TRACE(0, "UREDISClusterClient::processPipeline(%V,%b)", pipeline.rep, reorderable)

   UString workingString(U_CAPACITY);
   UVector<UString> commands(pipeline, "\r\n");
   uint16_t hashslot, workingHashslot, count = 0;

   auto pushPipeline = [&] (void) -> void {

      if constexpr (silence)
         {
         if (count > 1)
            {
            (void) workingString.insert(0, U_CONSTANT_TO_PARAM("CLIENT REPLY OFF \r\n"));
            (void) workingString.append(U_CONSTANT_TO_PARAM("CLIENT REPLY ON \r\n"));
            }
         else
            {
            (void) workingString.insert(0, U_CONSTANT_TO_PARAM("CLIENT REPLY SKIP \r\n"));
            }
         }

      UREDISClient<UTCPSocket>& client = clientForHashslot(hashslot);

      if constexpr (silence) (void) client.sendRequest(workingString);
      else
         {
replay:  (void) client.processRequest(U_RC_MULTIBULK, U_STRING_TO_PARAM(workingString));

         switch (error)
            {
            case ClusterError::moved:
            case ClusterError::tryagain:
               {
               goto replay;
               }
            break;

            case ClusterError::ask:
               {
               UREDISClient<UTCPSocket>& temporaryClient = clientForASKip();

               (void) temporaryClient.processRequest(U_RC_MULTIBULK, U_STRING_TO_PARAM(workingString));
               }
            break;

            case ClusterError::none: break;
            }

         if constexpr (silence == false) vitem.move(client.vitem);

         count = 0;
         workingString.clear();
         }
   };

   /*
   */
   if (reorderable) {
      
      for (UVectorStringIter it = commands.begin(); it != commands.end(); ) {

         if (it == commands.begin()) hashslot = hashslotFromCommand(*it);

         while (it != commands.end()) {

            const UString& command = *it;
          //  if (command.find("CLIENT", 0, 6)) goto isADirective;

            if (hashslotFromCommand(command) == hashslot)
               {
               
               ++count;

          //  goto isADirective;
            
               (void) workingString.append(command + "\r\n");
               it = commands.erase(it);
               }
            else ++it;
         }
         
         pushPipeline();

         if (commands.size() != 0) it = commands.begin();
      }
   }
   else {

      for (uint32_t index = 0, n = commands.size(); index < n; index++)
         {

         if (index == 0) hashslot = hashslotFromCommand(commands[0]);

         UString command = commands[index];
         command.trim();

       //  if (command.find("CLIENT", 0, 6)) goto isADirective;

         workingHashslot = hashslotFromCommand(command);

         if (workingHashslot == hashslot) {
               
            ++count;

       //  goto isADirective;

            (void) workingString.append(command + "\r\n");
            if ((index +1) < n) continue;
         }

         pushPipeline();
         hashslot = workingHashslot;
         count = 0;
      }
   }

   return vitem;
}
#  endif

// DEBUG

#  if defined(DEBUG)
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
#  endif
#endif
