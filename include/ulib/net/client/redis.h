// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    redis.h - Simple Redis client 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_REDIS_H
#define ULIB_REDIS_H 1

#include <ulib/net/client/client.h>

/**
 * @see http://redis.io/topics/protocol
 *
 * For Simple Strings the first byte of the reply is "+"
 * For Errors the first byte of the reply is "-"
 * For Integers the first byte of the reply is ":"
 * For Bulk Strings the first byte of the reply is "$"
 * For Arrays the first byte of the reply is "*"
 */

#define U_RC_ANY       '?'
#define U_RC_NONE      ' '
#define U_RC_INT       ':'
#define U_RC_BULK      '$'
#define U_RC_ERROR     '-'
#define U_RC_INLINE    '+'
#define U_RC_MULTIBULK '*'

#define U_RC_LOG_NONE   0
#define U_RC_LOG_ERROR  1
#define U_RC_LOG_WARN   2
#define U_RC_LOG_LOG    3
#define U_RC_LOG_DEBUG  4

#define U_RC_OK                          0
#define U_RC_ERR                        -1
#define U_RC_ERR_CONECTION_CLOSE        -2
#define U_RC_ERR_SEND                 -101
#define U_RC_ERR_TIMEOUT              -102
#define U_RC_ERR_RECV                 -103
#define U_RC_ERR_PROTOCOL             -104
#define U_RC_ERR_BUFFER_OVERFLOW      -105
#define U_RC_ERR_DATA_FORMAT          -106
#define U_RC_ERR_DATA_BUFFER_OVERFLOW -107

/**
 * @class UREDISClient
 *
 * @brief UREDISClient is a wrapper to REDIS API
 */

typedef void (*vPFcs)(const UString&);

class U_EXPORT UREDISClient_Base : public UClient_Base {
public:

   ~UREDISClient_Base()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UREDISClient_Base)
      }

   // RESPONSE

   UString x;
   UVector<UString> vitem;

   bool getResult(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getResult(%u)", i)

      if (i < vitem.size() &&
          (x = vitem[i]))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   UString getString(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getString(%u)", i)

      if (getResult(i)) return x.copy();

      return UString::getStringNull();
      }

   bool getBool(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getBool(%u)", i)

      if (getResult(i)) return x.strtob();

      U_RETURN(false);
      }

   uint8_t getUInt8(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getUInt8(%u)", i)

      if (getResult(i)) return x.first_char()-'0';

      U_RETURN(0);
      }

   void setUInt8(uint8_t& value, uint32_t i = 0, uint8_t _default = 0)
      {
      U_TRACE(0, "UREDISClient_Base::setUInt8(%p,%u,%u)",  &value, i, _default)

      value = (getResult(i) ? x.first_char()-'0' : _default);
      }

   long getLong(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getLong(%u)", i)

      if (getResult(i)) return x.strtol();

      U_RETURN(0L);
      }

   unsigned long getULong(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getULong(%u)", i)

      if (getResult(i)) return x.strtoul();

      U_RETURN(0UL);
      }

   uint64_t getUInt64(uint32_t i = 0)
      {
      U_TRACE(0, "UREDISClient_Base::getUInt64(%u)", i)

      if (getResult(i)) return x.strtoull();

      U_RETURN(0ULL);
      }

   bool setMultiBulk(uint32_t i = 1)
      {
      U_TRACE(0, "UREDISClient_Base::setMultiBulk(%u)", i)

      if (getResult(i) &&
          x.equal(U_CONSTANT_TO_PARAM("( )")) == false)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool setMultiBulk(UVector<UString>& vec, uint32_t i = 1)
      {
      U_TRACE(0, "UREDISClient_Base::setMultiBulk(%p,%u)", &vec, i)

      if (setMultiBulk(i))
         {
         U_ASSERT(vec.empty())

         UString2Object(U_STRING_TO_PARAM(x), vec);

         U_ASSERT_DIFFERS(vec.empty(), true)

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // Connect to REDIS server

   bool connect(const char* host = U_NULLPTR, unsigned int _port = 6379);

   // STRING (@see http://redis.io/commands#string)

   bool get(const char* key, uint32_t keylen) // Get the value of a key
      {
      U_TRACE(0, "UREDISClient_Base::get(%.*S,%u)", keylen, key, keylen)

      return processRequest(U_RC_BULK, U_CONSTANT_TO_PARAM("GET"), key, keylen);
      }

   bool mget(const char* param, uint32_t len) // Returns the values of all specified keys
      {
      U_TRACE(0, "UREDISClient_Base::mget(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("MGET"), param, len);
      }

   bool hmget(const char* format, uint32_t fmt_size, ...) // HMGET myhash field1 field2 nofield
      {
      U_TRACE(0, "UREDISClient_Base::hmget(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("HMGET"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool set(const char* key, uint32_t keylen, const char* value, uint32_t valuelen) // Set the string value of a key
      {
      U_TRACE(0, "UREDISClient_Base::set(%.*S,%u,%.*S,%u)", keylen, key, keylen, valuelen, value, valuelen)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("SET"), key, keylen, value, valuelen);
      }

   bool mset(const char* param, uint32_t len) // Sets the given keys to their respective values
      {
      U_TRACE(0, "UREDISClient_Base::mset(%.*S,%u)", len, param, len)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("MSET"), param, len);
      }

   bool hmset(const char* format, uint32_t fmt_size, ...) // HMSET myhash field1 "Hello" field2 "World"
      {
      U_TRACE(0, "UREDISClient_Base::hmset(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INLINE, U_CONSTANT_TO_PARAM("HMSET"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool operator[](const UString& key) { return get(U_STRING_TO_PARAM(key)); }

   int operator+=(const char* key) // Increment the integer value of a key by one
      {
      U_TRACE(0, "UREDISClient_Base::operator+=(%S)", key)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("INCR"), key, u__strlen(key, __PRETTY_FUNCTION__))) return getLong();

      U_RETURN(-1);
      }

   int operator-=(const char* key) // Decrement the integer value of a key by one
      {
      U_TRACE(0, "UREDISClient_Base::operator-=(%S)", key)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("DECR"), key, u__strlen(key, __PRETTY_FUNCTION__))) return getLong();

      U_RETURN(-1);
      }

   // CONNECTION (@see http://redis.io/commands#connection)

   bool selectDB(uint32_t index = 0) // Change the selected database for the current connection
      {
      U_TRACE(0, "UREDISClient_Base::selectDB(%u)", index)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("SELECT"), u_buffer, u_num2str32(index, u_buffer) - u_buffer);
      }

   bool auth(const char* _password, uint32_t _password_len) // Authenticate to the server
      {
      U_TRACE(0, "UREDISClient_Base::auth(%.*S,%u)", _password_len, _password, _password_len)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("AUTH"), _password, _password_len);
      }

   bool echo(const char* message, uint32_t len) // Echo the given string
      {
      U_TRACE(0, "UREDISClient_Base::echo(%.*S,%u)", len, message, len)

      return processRequest(U_RC_BULK, U_CONSTANT_TO_PARAM("ECHO"), message, len);
      }

   bool ping() // Ping the server
      {
      U_TRACE_NO_PARAM(0, "UREDISClient_Base::ping()")

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("PING"));
      }

   bool quit() // Close the connection
      {
      U_TRACE_NO_PARAM(0, "UREDISClient_Base::quit()")

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("QUIT"));
      }

   // SERVER (@see http://redis.io/commands#server)

   bool time() // Return the current server time 
      {
      U_TRACE_NO_PARAM(0, "UREDISClient_Base::time()")

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("TIME"));
      }

   bool info(const char* section = "default") // Get information and statistics about the server
      {
      U_TRACE(0, "UREDISClient_Base::info(%S)", section)

      return processRequest(U_RC_BULK, U_CONSTANT_TO_PARAM("INFO"), section, u__strlen(section, __PRETTY_FUNCTION__));
      }

   UString getInfoData(const char* section, const char* key, uint32_t len); // Get information and statistics about the server

   UString getRedisVersion() { return getInfoData("default", U_CONSTANT_TO_PARAM("redis_version:")); }

   // SET (@see http://redis.io/commands#set)

   bool sadd(const char* key, uint32_t keylen, const char* param, uint32_t len) // Add one or more members to a set
      {
      U_TRACE(0, "UREDISClient_Base::sadd(%.*S,%u,%.*S,%u)", keylen, key, keylen, len, param, len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("SADD"), key, keylen, param, len)) return getBool();

      U_RETURN(false);
      }

   bool zadd(const char* format, uint32_t fmt_size, ...) // ZADD myzset 2 "two" 3 "three"
      {
      U_TRACE(0, "UREDISClient_Base::zadd(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("ZADD"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool zrem(const char* format, uint32_t fmt_size, ...) // ZREM myzset "two"
      {
      U_TRACE(0, "UREDISClient_Base::zrem(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("ZREM"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   // Returns the elements in the sorted set at key with a score between min and max (including elements with score equal to min or max)

   bool zrangebyscore(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UREDISClient_Base::zrangebyscore(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_ANY, U_CONSTANT_TO_PARAM("ZRANGEBYSCORE"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool zrangebyscore(const char* key, uint32_t keylen, uint32_t _min, uint32_t _max)
      {
      U_TRACE(0, "UREDISClient_Base::zrangebyscore(%.*S,%u,%u,%u)", keylen, key, keylen, _min, _max)

      char buf[128];
      uint32_t buf_len = u__snprintf(buf, U_CONSTANT_SIZE(buf), U_CONSTANT_TO_PARAM("%u %u"), _min, _max);

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("ZRANGEBYSCORE"), key, keylen, buf, buf_len);
      }

   bool srem(const char* key, uint32_t keylen, const char* param, uint32_t len) // Remove one or more members from a set
      {
      U_TRACE(0, "UREDISClient_Base::srem(%.*S,%u,%.*S,%u)", keylen, key, keylen, len, param, len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("SREM"), key, keylen, param, len)) return getBool();

      U_RETURN(false);
      }

   bool srem(const char* key, uint32_t keylen, const UString& param) { return srem(key, keylen, U_STRING_TO_PARAM(param)); }

   bool smembers(const char* key, uint32_t keylen) // Get all the members in a set
      {
      U_TRACE(0, "UREDISClient_Base::smembers(%.*S,%u)", keylen, key, keylen)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SMEMBERS"), key, keylen);
      }

   bool deleteSetMembers(const char* key, uint32_t keylen) // Delete all the members in a set
      {
      U_TRACE(0, "UREDISClient_Base::deleteKeys(%.*S,%u)", keylen, key, keylen)

      if (smembers(key, keylen)) return srem(key, keylen, vitem.join());

      U_RETURN(false);
      }

   // KEYS (@see http://redis.io/commands#keys)

   bool randomkey() // Return a random key from the keyspace
      {
      U_TRACE_NO_PARAM(0, "UREDISClient_Base::randomkey()")

      return processRequest(U_RC_BULK, U_CONSTANT_TO_PARAM("RANDOMKEY"));
      }

   bool scan(const char* pattern, uint32_t len, vPFcs function); // Returns all keys matching pattern (scan 0 MATCH *11*)

   bool del(const UString& keys) // Delete one or more key
      {
      U_TRACE(0, "UREDISClient_Base::del(%V)", keys.rep)

      return processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("DEL"), U_STRING_TO_PARAM(keys));
      }

   bool deleteKeys(const char* pattern, uint32_t len); // Delete all keys matching pattern

   bool del(const char* format, uint32_t fmt_size, ...) // Delete one or more key
      {
      U_TRACE(0, "UREDISClient_Base::del(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("DEL"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool dump(const char* key, uint32_t keylen) // Return a serialized version of the value stored at the specified key
      {
      U_TRACE(0, "UREDISClient_Base::dump(%.*S,%u)", keylen, key, keylen)

      return processRequest(U_RC_BULK, U_CONSTANT_TO_PARAM("DUMP"), key, keylen);
      }

   bool exists(const char* format, uint32_t fmt_size, ...) // EXISTS key1 
      {
      U_TRACE(0, "UREDISClient_Base::exists(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("EXISTS"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool hexists(const char* format, uint32_t fmt_size, ...) // HEXISTS myhash field1
      {
      U_TRACE(0, "UREDISClient_Base::hexists(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("HEXISTS"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   bool type(const char* key, uint32_t keylen) // Determine the type stored at key
      {
      U_TRACE(0, "UREDISClient_Base::type(%.*S,%u)", keylen, key, keylen)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("TYPE"), key, keylen);
      }

   int ttl(const char* key, uint32_t keylen) // Get the time to live for a key in seconds
      {
      U_TRACE(0, "UREDISClient_Base::ttl(%.*S,%u)", keylen, key, keylen)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("TTL"), key, keylen)) return getLong();

      U_RETURN(-1);
      }

   int pttl(const char* key, uint32_t keylen) // Get the time to live for a key in milliseconds
      {
      U_TRACE(0, "UREDISClient_Base::pttl(%.*S,%u)", keylen, key, keylen)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PTTL"), key, keylen)) return getLong();

      U_RETURN(-1);
      }

   bool persist(const char* key, uint32_t keylen) // Remove the expiration from a key
      {
      U_TRACE(0, "UREDISClient_Base::persist(%.*S,%u)", keylen, key, keylen)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PERSIST"), key, keylen)) return getBool();

      U_RETURN(false);
      }

   bool move(const char* key, uint32_t keylen, uint32_t destination_db) // Move a key to another database
      {
      U_TRACE(0, "UREDISClient_Base::move(%.*S,%u,%u)", keylen, key, keylen, destination_db)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("MOVE"), key, keylen, u_buffer, u_num2str32(destination_db, u_buffer) - u_buffer)) return getBool();

      U_RETURN(false);
      }

   bool expire(const char* key, uint32_t keylen, uint32_t sec) // Set a key's time to live in seconds
      {
      U_TRACE(0, "UREDISClient_Base::expire(%.*S,%u,%u)", keylen, key, keylen, sec)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXPIRE"), key, keylen, u_buffer, u_num2str32(sec, u_buffer) - u_buffer)) return getBool();

      U_RETURN(false);
      }

   bool pexpire(const char* key, uint32_t keylen, uint32_t millisec) // Set a key's time to live in milliseconds
      {
      U_TRACE(0, "UREDISClient_Base::pexpire(%.*S,%u,%u)", keylen, key, keylen, millisec)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PEXPIRE"), key, keylen, u_buffer, u_num2str32(millisec, u_buffer) - u_buffer)) return getBool();

      U_RETURN(false);
      }

   bool expireat(const char* key, uint32_t keylen, time_t timestamp) // Set the expiration for a key as a UNIX timestamp (seconds since January 1, 1970)
      {
      U_TRACE(0, "UREDISClient_Base::expireat(%.*S,%u,%T)", keylen, key, keylen, timestamp)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

#  if SIZEOF_TIME_T == 8
      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXPIREAT"), key, keylen, u_buffer, u_num2str64(timestamp, u_buffer) - u_buffer)) return getBool();
#  else
      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXPIREAT"), key, keylen, u_buffer, u_num2str32(timestamp, u_buffer) - u_buffer)) return getBool();
#  endif

      U_RETURN(false);
      }

   bool pexpireat(const char* key, uint32_t keylen, uint64_t timestamp) // Set the expiration for a key as a UNIX timestamp (milliseconds since January 1, 1970)
      {
      U_TRACE(0, "UREDISClient_Base::pexpireat(%.*S,%u,%llu)", keylen, key, keylen, timestamp)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PEXPIREAT"), key, keylen, u_buffer, u_num2str64(timestamp, u_buffer) - u_buffer)) return getBool();

      U_RETURN(false);
      }

   // Atomically transfer a key from a Redis instance to another one

   bool migrate(const char* key, uint32_t keylen, const char* host, int _port = 6379, uint32_t timeout_ms = 10000, uint32_t destination_db = 0, bool COPY = false, bool REPLACE = false)
      {
      U_TRACE(0, "UREDISClient_Base::migrate(%.*S,%u,%S,%d,%u,%u,%b,%b)", keylen, key, keylen, host, _port, timeout_ms, destination_db, COPY, REPLACE)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("MIGRATE"), u_buffer,
                              u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%s %d %.*s %u %u %s %s"), // host port key destination-db timeout [COPY] [REPLACE]
                                          host, port, keylen, key, destination_db, timeout_ms, COPY ? "COPY" : "", REPLACE ? "REPLACE" : ""));
      }

   // PUB/SUB (@see http://redis.io/pubsub)

   bool publish(const char* channel, uint32_t channel_len, const char* msg, uint32_t msg_len) // Posts a message to the given channel
      {
      U_TRACE(0, "UREDISClient_Base::publish(%.*S,%u,%.*S,%u)", channel_len, channel, channel_len, msg_len, msg, msg_len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PUBLISH"), channel, channel_len, msg, msg_len)) return getBool();

      U_RETURN(false);
      }

   bool subscribe(const char* param, uint32_t len) // Listen for messages published to the given channels
      {
      U_TRACE(0, "UREDISClient_Base::subscribe(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("SUBSCRIBE"), param, len);
      }

   bool unsubscribe(const char* param, uint32_t len) // Stop listening for messages posted to the given channels
      {
      U_TRACE(0, "UREDISClient_Base::unsubscribe(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("UNSUBSCRIBE"), param, len);
      }

   // LIST (@see http://redis.io/list)

   bool lrange(const char* param, uint32_t len) // Get a range of elements from a list
      {
      U_TRACE(0, "UREDISClient_Base::lrange(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("LRANGE"), param, len);
      }

   // MULTI-EXEC (@see https://redis.io/commands/exec)

   bool processMultiRequest(const char* format, uint32_t fmt_size, ...);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   int err;

   UREDISClient_Base() : UClient_Base(U_NULLPTR)
      {
      U_TRACE_REGISTER_OBJECT(0, UREDISClient_Base, "", 0)

      err = 0;
      }

   void processResponse();
   bool processRequest(char recvtype);

   bool processRequest(char recvtype, const char* p1, uint32_t len1)
      {
      U_TRACE(0, "UREDISClient_Base::processRequest(%C,%.*S,%u)", recvtype, len1, p1, len1)

      UClient_Base::iov[0].iov_base = (caddr_t)p1;
      UClient_Base::iov[0].iov_len  = len1;
      UClient_Base::iov[1].iov_base = (caddr_t)U_CRLF;
      UClient_Base::iov[1].iov_len  =
               UClient_Base::iovcnt = 2;

      if (processRequest(recvtype))
         {
         processResponse();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool processMethod(char recvtype, const char* method, uint32_t method_len, const char* format, uint32_t fmt_size, va_list argp)
      {
      U_TRACE(0, "UREDISClient_Base::processMethod(%C,%.*S,%u,%.*S,%u)", recvtype, method_len, method, method_len, fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)
      U_INTERNAL_ASSERT_POINTER(method)
      U_INTERNAL_ASSERT_MAJOR(fmt_size, 0)
      U_INTERNAL_ASSERT_MAJOR(method_len, 0)
      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      UClient_Base::iovcnt = 4;

      UClient_Base::iov[0].iov_base = (caddr_t)method;
      UClient_Base::iov[0].iov_len  = method_len;
      UClient_Base::iov[1].iov_base = (caddr_t)" ";
      UClient_Base::iov[1].iov_len  = 1;

      UClient_Base::iov[3].iov_base = (caddr_t)U_CRLF;
      UClient_Base::iov[3].iov_len  = 2;

      UClient_Base::iov[2].iov_len = u__vsnprintf((char*)(UClient_Base::iov[2].iov_base = (caddr_t)u_buffer), U_BUFFER_SIZE, format, fmt_size, argp);

      if (processRequest(recvtype))
         {
         processResponse();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool processRequest(char recvtype, const char* p1, uint32_t len1, const char* p2, uint32_t len2);
   bool processRequest(char recvtype, const char* p1, uint32_t len1, const char* p2, uint32_t len2, const char* p3, uint32_t len3);

private:
   char* getResponseItem(char* ptr, UVector<UString>& vec, uint32_t depth) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UREDISClient_Base)
};

template <class Socket> class U_EXPORT UREDISClient : public UREDISClient_Base {
public:

   UREDISClient() : UREDISClient_Base()
      {
      U_TRACE_REGISTER_OBJECT(0, UREDISClient, "", 0)

      U_NEW(Socket, UClient_Base::socket, Socket(UClient_Base::bIPv6));
      }

   ~UREDISClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UREDISClient)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UREDISClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UREDISClient)
};

#endif
