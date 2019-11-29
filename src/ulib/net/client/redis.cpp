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
UREDISClient_Base* UREDISClient_Base::pthis;

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

void UREDISClient_Base::processResponse()
{
   U_TRACE_NO_PARAM(0, "UREDISClient_Base::processResponse()")

   U_DUMP_CONTAINER(vitem);

   // for now alwasys process from beginning
   size_t index = 0;
   char prefix;
   uint32_t len;
   const char* ptr1;
   const char* ptr2;

   while (index < UClient_Base::response.size())
   {
      prefix = UClient_Base::response.c_char(index++);
      len = UClient_Base::response.size() - index + 1;

      ptr1 =
      ptr2 = UClient_Base::response.c_pointer(index);

      while (*ptr2 != '\r') ++ptr2;

      //*3\r\n*4\r\n:5461\r\n:10922\r\n*3\r\n$9\r\n127.0.0.1\r\n:7001\r\n$40\r\ncd153c9c98419c156db43b91f20464d0feaed1b7\r\n*3\r\n$9\r\n127.0.0.1\r\n:7003\r\n$40\r\n5

      switch (prefix)
      {
         // :0\r\n
         case U_RC_INT:
         // -Error message\r\n
         case U_RC_ERROR:
         // +OK\r\n
         case U_RC_INLINE:
         {
            len = ptr2-ptr1;
            vitem.push_back(UClient_Base::response.substr(ptr1, len));
            index += len + U_CONSTANT_SIZE(U_CRLF);
            break;
         }
         case U_RC_BULK:
         {
            // $-1\r\n (Null Bulk String)
            if (ptr1[0] == '-')
            {
               vitem.push_back(UString::getStringNull());
               index += 2 + U_CONSTANT_SIZE(U_CRLF);
            }
            else
            {
               len = u_strtoul(ptr1, ptr2);
               ptr2 += U_CONSTANT_SIZE(U_CRLF);
               vitem.push_back(UClient_Base::response.substr(ptr2, len));
               index += (ptr2 - ptr1) + len + U_CONSTANT_SIZE(U_CRLF);
            }
            break;
         }
         // *2\r\n$10\r\n1439822796\r\n$6\r\n311090\r\n
         case U_RC_MULTIBULK:
         {
            index += (ptr2 - ptr1) + U_CONSTANT_SIZE(U_CRLF);
            break;
         }
         // never
         default: break;
      }
   }

   UClient_Base::response.setEmpty();

   U_DUMP_CONTAINER(vitem);
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

// by Victor Stewart

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX20) && defined(U_LINUX) && !defined(__clang__)

// this is called for subscribed channels
int UREDISClusterClient::handlerRead()
{
   // BytesRead(100) = "*3\r\n$7\r\nmessage\r\n$19\r\n{ABC}.trafficSignal\r\n$1\r\n1\r\n*3\r\n$7\r\nmessage\r\n$19\r\n{DEF}.trafficSignal\r\n$1\r\n1\r\n"

   U_TRACE_NO_PARAM(0, "UREDISClusterClient::handlerRead()")

   if ((clear(), response.setEmpty(), readResponse(U_SINGLE_READ)))
   {
      processResponse();

      // [0]   message
      // [1]   channel
      // [2]   payload  

      for (size_t index = 0, n = vitem.size(); index < n; index += 3)
      {
         if (vitem[index] == "message"_ctv)
         {
            vPFcscs callback = (vPFcscs)(master->pchannelCallbackMap->at(vitem[index + 1]));
            if (callback) callback(vitem[index + 1], vitem[index + 2]);
         }
      }
   }

   U_RETURN(U_NOTIFIER_OK);
}

ClusterError UREDISClusterMaster::checkResponseForClusterErrors(const UString& response, size_t offset)
{
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

   // the first node in each array is the master
   // currently we ignore slaves

   managementClient->sendRequest("*2\r\n$7\r\nCLUSTER\r\n$5\r\nSLOTS\r\n"_ctv);
   managementClient->response.setEmpty();
   managementClient->readResponse(U_SINGLE_READ);

   UString& response = managementClient->response;

   uint16_t lowHashSlot;
   uint16_t highHashSlot;
   UString compositeAddress(50U);

   UHashMap<RedisClusterNode *> *newNodes;
   U_NEW(UHashMap<RedisClusterNode *>, newNodes, UHashMap<RedisClusterNode *>);

   size_t index = response.find('\r', 0) + 2, workingEnd;

   auto extractNumber = [&] (size_t startOfNumber) -> size_t
   {
      workingEnd = response.find('\r', startOfNumber);
      index = workingEnd + 2; // lands on next prefix
      return response.substr(startOfNumber, workingEnd - startOfNumber).strtoul();
   };

   while (index < response.size())
   {  
      // *4
      //    :0
      //    :5460
      //    *3
      //       $9
      //       127.0.0.1
      //       :7000
      //       $40
      //       c4869fd1346c4731d2980f46a3dd934627d5dbb4
      //    *3
      //       $9
      //       127.0.0.1
      //       :7005
      //       $40
      //       576ce7f8a927b00bb191344f6a167e6615569587

      // *4
      ++index; // skip over prefix
      int16_t slaveCount = extractNumber(index) - 3;

      // :0
      ++index; // skip over prefix
      lowHashSlot = extractNumber(index);

      // :5460
      ++index; // skip over prefix
      highHashSlot = extractNumber(index);

      // skip to length of address
      index = response.find('$', index) + 1;
      size_t lengthOfAddress = extractNumber(index);

      UString address = response.substr(index, lengthOfAddress);
      uint16_t port = extractNumber(index + lengthOfAddress + 3);

      UCompileTimeStringFormatter::snprintf<"{}.{}"_ctv>(compositeAddress, address, port);

      RedisClusterNode *workingNode = clusterNodes ? clusterNodes->erase(compositeAddress) : U_NULLPTR;

      // in the case of MOVE some nodes will be new, but others we'll already be connected to
      if (workingNode)
      {  
         workingNode->lowHashSlot = lowHashSlot;
         workingNode->highHashSlot = highHashSlot;
      }
      else
      {  
         U_NEW(RedisClusterNode, workingNode, RedisClusterNode(this, address, port, lowHashSlot, highHashSlot));
		}

      newNodes->insert(compositeAddress, workingNode);

      // scan til we hit the beginning of the next node cluster
      do { index = response.find('*', index + 1); } while ((--slaveCount) > -1);
   }

   // if any nodes were taken offline, the clients would've disconnected by default
   if (clusterNodes) U_DELETE(clusterNodes);

   clusterNodes = newNodes;

   response.setEmpty();
}

bool UREDISClusterMaster::connect(const char* host, unsigned int _port)
{
   U_TRACE(0, "UREDISClusterMaster::connect(%S,%u)", host, _port)

   managementClient->UEventFd::op_mask |= EPOLLET;

   if (managementClient->connect(host, _port))
   {
      calculateNodeMap();
   
      RedisClusterNode *randomNode = clusterNodes->randomElement();

      if (randomNode)
      {
         subscriptionClient->connect(randomNode->ipAddress.c_str(), randomNode->port);

         U_NEW(UHashMap<void*>, pchannelCallbackMap, UHashMap<void*>());

         subscriptionClient->UEventFd::fd = subscriptionClient->getFd();
         subscriptionClient->UEventFd::op_mask |= EPOLLET;
            
         UServer_Base::addHandlerEvent(subscriptionClient);
        
         U_RETURN(true);
      }
   }

   U_RETURN(false);
}

void UREDISClusterMaster::clusterUnsubscribe(const UString& channel) // unregister the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterMaster::clusterUnsubscribe(%V)", channel.rep)

   UCompileTimeRESPEncoder::encode<"UNSUBSCRIBE {}"_ctv>(subscriptionClient->response, channel);
   subscriptionClient->sendRequest(subscriptionClient->response);
   (void)pchannelCallbackMap->erase(channel);
}

void UREDISClusterMaster::clusterSubscribe(const UString& channel, vPFcscs callback) // register the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterMaster::clusterSubscribe(%V,%p)", channel.rep, callback)

   UCompileTimeRESPEncoder::encode<"SUBSCRIBE {}"_ctv>(subscriptionClient->response, channel);
   subscriptionClient->sendRequest(subscriptionClient->response);

   UString channelCopy(U_STRING_TO_PARAM(channel));
   pchannelCallbackMap->insert(channelCopy, (const void*)callback);
}

static void getNextCommandResponse(const UString& string, size_t& marker)
{
   int8_t depth = 0;
   char prefix;
   const char *pointer1, *pointer2;

   do
   {
      prefix = string.c_char(marker++);
      pointer1 = pointer2 = string.c_pointer(marker);

      while (*pointer2 != '\r') ++pointer2;
      marker += (pointer2-pointer1) + U_CONSTANT_SIZE(U_CRLF);

      switch (prefix)
      {
         // :0\r\n
         case U_RC_INT:
         // +OK\r\n
         case U_RC_INLINE:
         // -MOVED 3999 127.0.0.1:6381\r\n
         case U_RC_ERROR: break;
         // $-1\r\n                      
         // $15\r\nmy-value-tester\r\n
         case U_RC_BULK:
         {
            ssize_t length = u_strtol(pointer1, pointer2);
            if (length > -1) marker += length + U_CONSTANT_SIZE(U_CRLF);
            break;
         }
         // *2\r\n$10\r\n1439822796\r\n$6\r\n311090\r\n
         case U_RC_MULTIBULK:
         {
            depth += u_strtol(pointer1, pointer2);
            break;
         }
      }

      depth--;

   } while (depth > -1);
}

const UVector<UString>& UREDISClusterMaster::clusterAnonMulti(const AnonymousClusterPipeline& pipeline, const bool reorderable)
{
   U_TRACE(0, "UREDISClusterMaster::clusterAnonMulti(%b)", reorderable)

   U_DUMP("pipeline = %v", pipeline.pipeline.rep);

   managementClient->vitem.clear();

   static UString workingString(500U);
   workingString.setEmpty();
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
            if (UNLIKELY(it == pipeline.spans.end())) goto push;
         }
         else
         {

         push:

            // returns when no error. might change client
            workingClient = sendToCluster<true>(workingHashslot, workingString, workingClient);

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
         uint8_t numberOfCommands;

         ExecutionMapping(UREDISClusterClient *_client, size_t _ordering, uint8_t _numberOfCommands) : client(_client), ordering(_ordering), numberOfCommands(_numberOfCommands) {}
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
            
            spanToExeuction.insert_or_assign(it->index, ExecutionMapping(workingClient, executionCount, it->commandCount));
            executionCount += it->commandCount;

            it = spans.erase(it);
         }
         else it++;
      }

      touchedClients.insert_or_assign(workingClient, true);
   
      workingClient->sendRequest(workingString);
      // we'll get here once having exhausted the commands for a given node
      if (spans.size() > 0) goto rinseAndRepeat;

      struct CommandResponse {

         size_t beginning, ending;
      };

      size_t totalBufferSize = 0;
      std::unordered_map<UREDISClusterClient*, std::vector<CommandResponse>> commandResponsesByClient;

      for (const auto& [touchedClient, meaninglessFlag] : touchedClients)
      {
         UString& clientBuffer = touchedClient->UClient_Base::response;

         touchedClient->UClient_Base::readResponse();

         size_t clientBufferSize = clientBuffer.size();
         totalBufferSize += clientBufferSize;

         size_t marker = 0;

         std::vector<CommandResponse> commandResponses;

         while (marker < clientBufferSize)
         {
            size_t beginning = marker;
            getNextCommandResponse(clientBuffer, marker);

            commandResponses.push_back({beginning, marker});
         }

         (void)commandResponsesByClient.insert_or_assign(touchedClient, commandResponses);
      }

      UString& masterBuffer = managementClient->UClient_Base::response;

      masterBuffer.reserve(totalBufferSize);

      for (size_t index = 0; index < pipeline.spans.size(); index++)
      {
         const ExecutionMapping& mapping = spanToExeuction.at(index);
         UString* clientBuffer = &mapping.client->UClient_Base::response;

         std::vector<CommandResponse>& commandResponses = commandResponsesByClient[mapping.client];
         
         size_t beginning = commandResponses[mapping.ordering].beginning;
         size_t ending = commandResponses[(mapping.ordering + mapping.numberOfCommands - 1)].ending;
         
      checkAgain:

         U_DUMP("response = %.*s", ending - beginning, clientBuffer->data() + beginning);

         workingClient = 0;
         switch (checkResponseForClusterErrors(*clientBuffer, beginning))
         {
            case ClusterError::none:
            {
               masterBuffer.append(clientBuffer->data() + beginning, ending - beginning);

               continue;
            }
            case ClusterError::moved:
            {
               calculateNodeMap();
               break;
            }
            case ClusterError::ask:
            {
               uint32_t _start = clientBuffer->find(' ', beginning + U_CONSTANT_SIZE("-ASK 3999")) + 1,
                           _end = clientBuffer->find(':', _start);

               workingClient = clientForIP(clientBuffer->substr(_start, _end - _start));
               break;
            }
            case ClusterError::tryagain:
            {
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
   
   U_DUMP("UREDISClusterMaster::clusterAnonMulti(%b), masterBuffer = %v", reorderable, managementClient->UClient_Base::response.rep);

   managementClient->UREDISClient_Base::processResponse();
   
   return managementClient->vitem;
}
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
