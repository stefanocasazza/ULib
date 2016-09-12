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

#include <ulib/net/client/redis.h>

// Connect to REDIS server

bool UREDISClient_Base::connect(const char* phost, unsigned int _port)
{
   U_TRACE(0, "UREDISClient_Base::connect(%S,%u)", phost, _port)

   UString host;

   if (phost) (void) host.assign(phost);
   else
      {
      const char* env_redis_host = (const char*) U_SYSCALL(getenv, "%S", "REDIS_HOST");

      if (env_redis_host == 0)
         {
         (void) UClient_Base::response.replace(U_CONSTANT_TO_PARAM("connection disabled"));

         U_RETURN(false);
         }

      (void) host.assign(env_redis_host, u__strlen(env_redis_host, __PRETTY_FUNCTION__));

      const char* env_redis_port = (const char*) U_SYSCALL(getenv, "%S", "REDIS_PORT");

      if (env_redis_port) _port = atoi(env_redis_port);
      }

   if (UClient_Base::setHostPort(host, _port) &&
       UClient_Base::connect())
      {
      UClient_Base::iovcnt = 4;

      UClient_Base::iov[1].iov_base =
      UClient_Base::iov[4].iov_base = (caddr_t)" ";
      UClient_Base::iov[1].iov_len  =
      UClient_Base::iov[4].iov_len  = 1;

      UClient_Base::iov[3].iov_base =
      UClient_Base::iov[5].iov_base = (caddr_t)U_CRLF;
      UClient_Base::iov[3].iov_len  =
      UClient_Base::iov[5].iov_len  = 2;

      U_DUMP("getRedisVersion() = %V", getRedisVersion().rep)

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

U_NO_EXPORT bool UREDISClient_Base::processRequest(char recvtype)
{
   U_TRACE(0, "UREDISClient_Base::processRequest(%C)", recvtype)

   if (UClient_Base::sendRequest() &&
       (vitem.clear(), UClient_Base::response.setBuffer(U_CAPACITY), UClient_Base::readResponse(U_SINGLE_READ)))
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

U_NO_EXPORT char* UREDISClient_Base::getResponseItem(const UString& response, char* ptr, UVector<UString>& vec, uint32_t depth)
{
   U_TRACE(0, "UREDISClient_Base::getResponseItem(%p,%p,%p,%u)", response.rep, ptr, &vec, depth)

   U_INTERNAL_DUMP("ptr = %.20S", ptr)

   char prefix = *ptr++;

   U_INTERNAL_DUMP("prefix = %C", prefix)

   if (prefix != U_RC_BULK &&
       prefix != U_RC_MULTIBULK)
      {
      char* start = ptr;

      while (*ptr != '\r') ++ptr;

      vec.push_back(response.substr(start, ptr-start));

      return ptr;
      }

   int len = (int) strtol(ptr, &ptr, 10);

   U_INTERNAL_DUMP("len = %d errno = %d", len, errno)

   U_INTERNAL_ASSERT_EQUALS(memcmp(ptr, "\r\n", 2), 0)

   if (prefix == U_RC_BULK) // "$15\r\nmy-value-tester\r\n"
      {
      if (len == -1) vec.push_back(UString::getStringNull());
      else
         {
         vec.push_back(response.substr(ptr +2,len));
                                       ptr+=2+len;
         }

      return ptr;
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

   bool bnested;
   UVector<UString> vec1(len);

   for (int i = 0; i < len; ++i)
      {
      bnested = (ptr[2] == U_RC_MULTIBULK && ++depth);

      U_INTERNAL_DUMP("prefix = %C bnested = %b", ptr[2], bnested)

      ptr = getResponseItem(response, ptr+2, vec1, depth);

      if (bnested == false) vec.move(vec1);
      else
         {
         typedef UVector<UString> uvectorstring;

         UStringRep* rep = UObject2StringRep<uvectorstring>(vec1, true);

         vec.push_back(rep);

         vec1.clear();
         }
      }

   U_DUMP_CONTAINER(vec)

   return ptr;
}

U_NO_EXPORT void UREDISClient_Base::processResponse()
{
   U_TRACE_NO_PARAM(0, "UREDISClient_Base::processResponse()")

   U_INTERNAL_DUMP("err = %d", err)

   U_INTERNAL_ASSERT_EQUALS(err, U_RC_OK)

         char* ptr = UClient_Base::response.data();
   const char* end = UClient_Base::response.pend();

   do {
      ptr = getResponseItem(UClient_Base::response, ptr, vitem, 0);

      U_DUMP_CONTAINER(vitem)

      U_INTERNAL_ASSERT_EQUALS(memcmp(ptr, "\r\n", 2), 0)

      ptr += 2;
      }
   while (ptr < end);
}

bool UREDISClient_Base::processRequest(char recvtype, const char* p1, uint32_t len1)
{
   U_TRACE(0, "UREDISClient_Base::processRequest(%C,%.*S,%u)", recvtype, len1, p1, len1)

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iovcnt, 4)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[1].iov_len, 1)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[3].iov_len, 2)

   UClient_Base::iovcnt = 2;

   UClient_Base::iov[0].iov_base = (caddr_t)p1;
   UClient_Base::iov[0].iov_len  = len1;
   UClient_Base::iov[1].iov_base = (caddr_t)U_CRLF;
   UClient_Base::iov[1].iov_len  = 2;

   bool result = processRequest(recvtype);

   UClient_Base::iovcnt = 4;

   UClient_Base::iov[1].iov_base = (caddr_t)" ";
   UClient_Base::iov[1].iov_len  = 1;

   if (result)
      {
      processResponse();

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UREDISClient_Base::processRequest(char recvtype, const char* p1, uint32_t len1, const char* p2, uint32_t len2)
{
   U_TRACE(0, "UREDISClient_Base::processRequest(%C,%.*S,%u,%.*S,%u)", recvtype, len1, p1, len1, len2, p2, len2)

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iovcnt, 4)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[1].iov_len, 1)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[3].iov_len, 2)

   UClient_Base::iov[0].iov_base = (caddr_t)p1;
   UClient_Base::iov[0].iov_len  = len1;
   UClient_Base::iov[2].iov_base = (caddr_t)p2;
   UClient_Base::iov[2].iov_len  = len2;

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

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iovcnt, 4)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[1].iov_len, 1)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[3].iov_len, 2)

   UClient_Base::iovcnt = 6;

   UClient_Base::iov[0].iov_base = (caddr_t)p1;
   UClient_Base::iov[0].iov_len  = len1;
   UClient_Base::iov[2].iov_base = (caddr_t)p2;
   UClient_Base::iov[2].iov_len  = len2;
   UClient_Base::iov[3].iov_base = (caddr_t)" ";
   UClient_Base::iov[3].iov_len  = 1;
   UClient_Base::iov[4].iov_base = (caddr_t)p3;
   UClient_Base::iov[4].iov_len  = len3;

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[5].iov_len, 2)

   bool result = processRequest(recvtype);

   UClient_Base::iovcnt = 4;

   UClient_Base::iov[3].iov_base = (caddr_t)U_CRLF;
   UClient_Base::iov[3].iov_len  = 2;

   if (result)
      {
      processResponse();

      U_RETURN(true);
      }

   U_RETURN(false);
}

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

   return 0;
}
#endif
