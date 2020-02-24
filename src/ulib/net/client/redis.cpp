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

int UREDISClusterMaster::handlerRead()
{
   // BytesRead(100) = "*3\r\n$7\r\nmessage\r\n$19\r\n{ABC}.trafficSignal\r\n$1\r\n1\r\n*3\r\n$7\r\nmessage\r\n$19\r\n{DEF}.trafficSignal\r\n$1\r\n1\r\n"

   U_TRACE_NO_PARAM(0, "UREDISClusterMaster::handlerRead()")

   if (subscriptionString.size()) subscriptionString.setEmpty();
   USocketExt::read(subscriptionSocket, subscriptionString, U_SINGLE_READ, 1000);

   const char *ptr1, *ptr2, *pend = subscriptionString.pend();

   ptr1 = subscriptionString.data();

   do
   {
      // [0]   message
      // [1]   channel
      // [2]   payload

      if (*(ptr1 += 5) == '7')
      {  
         ptr2 = (ptr1 += 13);
         while (*ptr2 != '\r') ++ptr2;

         // ptr2 on \r after channel name length
         size_t lengthOfChannelName = u_strtoul(ptr1, ptr2);
         const UString& channel = subscriptionString.substr((ptr2 += U_CONSTANT_SIZE(U_CRLF)), lengthOfChannelName);

         // both on first digit of message length
         ptr1 = (ptr2 += lengthOfChannelName + U_CONSTANT_SIZE(U_CRLF) + 1);
         while (*ptr2 != '\r') ++ptr2;
         size_t lengthOfMessage = u_strtoul(ptr1, ptr2);
         const UString& message = subscriptionString.substr((ptr2 += U_CONSTANT_SIZE(U_CRLF)), lengthOfMessage);

         vPFcscs callback = (vPFcscs)(pchannelCallbackMap->at(channel));
         if (callback) callback(channel, message);
      } 

      while (ptr1 < pend && *ptr1 != '*') ++ptr1;

   } while (ptr1 < pend);

   U_RETURN(U_NOTIFIER_OK);
}

void UREDISClusterMaster::clusterUnsubscribe(const UString& channel) // unregister the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterMaster::clusterUnsubscribe(%V)", channel.rep)

   if (subscriptionString.size()) subscriptionString.setEmpty();
   UCompileTimeRESPEncoder::encode<"UNSUBSCRIBE {}"_ctv>(subscriptionString, channel);
   USocketExt::write(subscriptionSocket, U_STRING_TO_PARAM(subscriptionString), 1000);
   (void)pchannelCallbackMap->erase(channel);
}

void UREDISClusterMaster::clusterSubscribe(const UString& channel, vPFcscs callback) // register the callback for messages published to the given channels
{
   U_TRACE(0, "UREDISClusterMaster::clusterSubscribe(%V,%p)", channel.rep, callback)

   if (subscriptionString.size()) subscriptionString.setEmpty();
   UCompileTimeRESPEncoder::encode<"SUBSCRIBE {}"_ctv>(subscriptionString, channel);
   USocketExt::write(subscriptionSocket, U_STRING_TO_PARAM(subscriptionString), 1000);

   UString channelCopy(U_STRING_TO_PARAM(channel));
   pchannelCallbackMap->insert(channelCopy, (const void*)callback);
}

void UREDISClusterMaster::cloneClusterTopology()
{
   U_TRACE_NO_PARAM(0, "UREDISClusterMaster::cloneClusterTopology()")

   // the first node in each array is the master
   // currently we ignore slaves

   static constexpr auto clusterSlotsRequest = "*2\r\n$7\r\nCLUSTER\r\n$5\r\nSLOTS\r\n"_ctv;

   USocketExt::write(managementSocket, U_STRING_TO_PARAM(clusterSlotsRequest), 1000);

   if (workingString.size()) workingString.setEmpty();
   USocketExt::read(managementSocket, workingString, U_SINGLE_READ, 1000);

   UHashMap<RedisClusterNode *> *newNodes;
   U_NEW(UHashMap<RedisClusterNode *>, newNodes, UHashMap<RedisClusterNode *>);
   
   size_t index = workingString.find('\r', 1), workingEnd;
 
   size_t numberOfMasterNodes = u__strtoul(workingString.data() + 1, index - 1);

   // skip past number of masters
   index += 2;

   auto extractNumber = [&] (size_t startOfNumber) -> size_t
   {
      workingEnd = workingString.find('\r', startOfNumber);
      index = workingEnd + 2; // lands on next prefix
      return workingString.substr(startOfNumber, workingEnd - startOfNumber).strtoul();
   };

repeat:

   while (index < workingString.size())
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
      uint16_t lowHashSlot = extractNumber(index);

      // :5460
      ++index; // skip over prefix
      uint16_t highHashSlot = extractNumber(index);

      // skip to length of address
      index = workingString.find('$', index) + 1;
      size_t lengthOfAddress = extractNumber(index);

      UString address = workingString.substr(index, lengthOfAddress);
      uint16_t port = extractNumber(index + lengthOfAddress + 3);

      UString compositeAddress(50U);
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
         U_NEW(RedisClusterNode, workingNode, RedisClusterNode(address, port, lowHashSlot, highHashSlot));
      }

      for (; lowHashSlot <= highHashSlot; lowHashSlot++)
      {
         hashslotToSocket[lowHashSlot] = workingNode->socket;
      }

      newNodes->insert(compositeAddress, workingNode);

      // scan til we hit the beginning of the next node cluster
      do { index = workingString.find('*', index + 1); } while ((--slaveCount) > -1 && index < workingString.size());
   }

   if (newNodes->size() < numberOfMasterNodes) 
   {
      index = workingString.size();
      USocketExt::read(managementSocket, workingString, U_SINGLE_READ, 1000);
      goto repeat;
   }

   // if any nodes were taken offline, the clients would've disconnected by default
   if (clusterNodes) U_DELETE(clusterNodes);

   clusterNodes = newNodes;
}

bool UREDISClusterMaster::connect(const UString& host, uint16_t port)
{
   U_TRACE(0, "UREDISClusterMaster::connect(%v,%hhu)", host.rep, port)

   // const UString& singleTest = single()

   if (managementSocket->connectServer(host, port, 1000))
   {
      cloneClusterTopology();
   
      RedisClusterNode *randomNode = clusterNodes->randomElement();

      if (randomNode)
      {
         subscriptionSocket->connectServer(randomNode->ipAddress, randomNode->port, 1000);

         U_NEW(UHashMap<void*>, pchannelCallbackMap, UHashMap<void*>());

         this->UEventFd::fd = subscriptionSocket->getFd();
         this->UEventFd::op_mask |= EPOLLET;
         UServer_Base::addHandlerEvent(this);
        
         U_RETURN(true);
      }
   }

   U_RETURN(false);
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
