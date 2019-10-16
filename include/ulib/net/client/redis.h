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

#include <ulib/notifier.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/unixsocket.h>
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

typedef void (*vPFcs)  (const UString&);
typedef void (*vPFcscs)(const UString&,const UString&);

class UREDISClusterMaster;

class U_EXPORT UREDISClient_Base : public UClient_Base, UEventFd {
public:

   ~UREDISClient_Base();

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

   void setUInt8(uint8_t& value, uint32_t i = 0, uint8_t ldefault = 0)
      {
      U_TRACE(0, "UREDISClient_Base::setUInt8(%p,%u,%u)",  &value, i, ldefault)

      value = (getResult(i) ? x.first_char()-'0' : ldefault);
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UREDISClient_Base::clear()")

      vitem.clear();
          x.clear();
      }

   // Connect to REDIS server

   bool connect(const char* host = U_NULLPTR, unsigned int _port = 6379);

   // by Victor Stewart

   UString single(const UString& pipeline)
      {
      U_TRACE(0, "UREDISClient_Base::single(%V)", pipeline.rep)

      (void) processRequest(U_RC_MULTIBULK, U_STRING_TO_PARAM(pipeline));

      return vitem[0];
      }

   bool silencedSingle(UString& pipeline)
      {
      U_TRACE(0, "UREDISClient_Base::silencedSingle(%V)", pipeline.rep)

      return sendRequest(U_CONSTANT_TO_PARAM("CLIENT REPLY SKIP \r\n"), pipeline);
      }

   const UVector<UString>& multi(const UString& pipeline)
      {
      U_TRACE(0, "UREDISClient_Base::multi(%V)", pipeline.rep)

      (void) processRequest(U_RC_MULTIBULK, U_STRING_TO_PARAM(pipeline));

      return vitem;
      }

   bool silencedMulti(UString& pipeline)
      {
      U_TRACE(0, "UREDISClient_Base::silencedMulti(%V)", pipeline.rep)

      bool result = sendRequest(U_CONSTANT_TO_PARAM("CLIENT REPLY OFF \r\n"), pipeline + "CLIENT REPLY ON \r\n");

      // CLIENT REPLY ON responds with "+OK\r\n" and no way to silence it
      UClient_Base::readResponse();

      return result;
      }

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

   // HMGET myhash field1 field2 nofield

   bool hmget(const UString& str)
      {
      U_TRACE(0, "UREDISClient_Base::hmget(%V)", str.rep)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("HMGET"), U_STRING_TO_PARAM(str));
      }

   bool hmget(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UREDISClient_Base::hmget(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("HMGET"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   // HMSET myhash field1 "Hello" field2 "World"

   bool hmset(const UString& str)
      {
      U_TRACE(0, "UREDISClient_Base::hmset(%V)", str.rep)

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("HMSET"), U_STRING_TO_PARAM(str));
      }

   bool hmset(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UREDISClient_Base::hmset(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INLINE, U_CONSTANT_TO_PARAM("HMSET"), format, fmt_size, argp);

      va_end(argp);

      U_RETURN(ok);
      }

   // HDEL key field [field ...]

   bool hdel(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UREDISClient_Base::hdel(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = (processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("HDEL"), format, fmt_size, argp) ? getUInt8() : false);

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

   // GEO (@see https://redis.io/commands#geo)
   
   bool geoadd(const char* param, uint32_t len) // GEOADD key longitude latitude member [longitude latitude member ...]
      {
      U_TRACE(0, "UREDISClient_Base::geoadd(%.*S,%u)", len, param, len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("GEOADD"), param, len)) return getUInt8();

      U_RETURN(false);
      }
   
   // GEORADIUS key longitude latitude radius m|km|ft|mi [WITHCOORD] [WITHDIST] [WITHHASH] [COUNT count] [ASC|DESC] [STORE key] [STOREDIST key]

   bool georadius(const char* param, uint32_t len)
      {
      U_TRACE(0, "UREDISClient_Base::georadius(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("GEORADIUS"), param, len);
      }
   
   // SET (@see http://redis.io/commands#set)

   bool sadd(const char* key, uint32_t keylen, const char* param, uint32_t len) // Add one or more members to a set
      {
      U_TRACE(0, "UREDISClient_Base::sadd(%.*S,%u,%.*S,%u)", keylen, key, keylen, len, param, len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("SADD"), key, keylen, param, len)) return getUInt8();

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

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("SREM"), key, keylen, param, len)) return getUInt8();

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

   bool del(const UString& keys) // Delete one or more key
      {
      U_TRACE(0, "UREDISClient_Base::del(%V)", keys.rep)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("DEL"), U_STRING_TO_PARAM(keys))) return getUInt8();

      U_RETURN(false);
      }

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

   bool deleteKeys(const char* pattern, uint32_t len); // Delete all keys matching pattern

   bool scan(vPFcs function, const char* pattern = "*", uint32_t len = 1); // Returns all keys matching pattern (scan 0 MATCH *11*)

   bool dump(const char* key, uint32_t keylen) // Return a serialized version of the value stored at the specified key
      {
      U_TRACE(0, "UREDISClient_Base::dump(%.*S,%u)", keylen, key, keylen)

      return processRequest(U_RC_BULK, U_CONSTANT_TO_PARAM("DUMP"), key, keylen);
      }

   bool exists(const char* key, uint32_t keylen) // EXISTS key1 
      {
      U_TRACE(0, "UREDISClient_Base::exists(%.*S,%u)", keylen, key, keylen)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXISTS"), key, keylen)) return getUInt8();

      U_RETURN(false);
      }

   bool hexists(const char* key, uint32_t keylen, const char* field, uint32_t fieldlen) // HEXISTS myhash field1
      {
      U_TRACE(0, "UREDISClient_Base::hexists(%.*S,%u,%.*S,%u)", keylen, key, keylen, fieldlen, field, fieldlen)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("HEXISTS"), key, keylen, field, fieldlen)) return getUInt8();

      U_RETURN(false);
      }

   uint64_t hincrby(const char* key, uint32_t keylen, const char* field, uint32_t fieldlen) // HINCRBY myhash field1
      {
      U_TRACE(0, "UREDISClient_Base::hincrby(%.*S,%u,%.*S,%u)", keylen, key, keylen, fieldlen, field, fieldlen)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("HINCRBY"), key, keylen, field, fieldlen)) return getUInt64();

      U_RETURN(0ULL);
      }

   uint64_t hincrby(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UREDISClient_Base::hincrby(%.*S,%u)", fmt_size, format, fmt_size)

      bool ok;

      va_list argp;
      va_start(argp, fmt_size);

      ok = processMethod(U_RC_INT, U_CONSTANT_TO_PARAM("HINCRBY"), format, fmt_size, argp);

      va_end(argp);

      if (ok) return getUInt64();

      U_RETURN(0ULL);
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

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PERSIST"), key, keylen)) return getUInt8();

      U_RETURN(false);
      }

   bool move(const char* key, uint32_t keylen, uint32_t destination_db) // Move a key to another database
      {
      U_TRACE(0, "UREDISClient_Base::move(%.*S,%u,%u)", keylen, key, keylen, destination_db)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("MOVE"), key, keylen, u_buffer, u_num2str32(destination_db, u_buffer) - u_buffer)) return getUInt8();

      U_RETURN(false);
      }

   bool expire(const char* key, uint32_t keylen, uint32_t sec) // Set a key's time to live in seconds
      {
      U_TRACE(0, "UREDISClient_Base::expire(%.*S,%u,%u)", keylen, key, keylen, sec)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXPIRE"), key, keylen, u_buffer, u_num2str32(sec, u_buffer) - u_buffer)) return getUInt8();

      U_RETURN(false);
      }

   bool pexpire(const char* key, uint32_t keylen, uint32_t millisec) // Set a key's time to live in milliseconds
      {
      U_TRACE(0, "UREDISClient_Base::pexpire(%.*S,%u,%u)", keylen, key, keylen, millisec)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PEXPIRE"), key, keylen, u_buffer, u_num2str32(millisec, u_buffer) - u_buffer)) return getUInt8();

      U_RETURN(false);
      }

   bool expireat(const char* key, uint32_t keylen, time_t timestamp) // Set the expiration for a key as a UNIX timestamp (seconds since January 1, 1970)
      {
      U_TRACE(0, "UREDISClient_Base::expireat(%.*S,%u,%T)", keylen, key, keylen, timestamp)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

#  if SIZEOF_TIME_T == 8
      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXPIREAT"), key, keylen, u_buffer, u_num2str64(timestamp, u_buffer) - u_buffer)) return getUInt8();
#  else
      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("EXPIREAT"), key, keylen, u_buffer, u_num2str32(timestamp, u_buffer) - u_buffer)) return getUInt8();
#  endif

      U_RETURN(false);
      }

   bool pexpireat(const char* key, uint32_t keylen, uint64_t timestamp) // Set the expiration for a key as a UNIX timestamp (milliseconds since January 1, 1970)
      {
      U_TRACE(0, "UREDISClient_Base::pexpireat(%.*S,%u,%llu)", keylen, key, keylen, timestamp)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("PEXPIREAT"), key, keylen, u_buffer, u_num2str64(timestamp, u_buffer) - u_buffer)) return getUInt8();

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

   bool pipeline(const char* param, uint32_t len)
      {
      U_TRACE(0, "UREDISClient_Base::pipeline(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, param, len);
      }

   // RPUSH key value [value ...]

   bool rpush(const char* param, uint32_t len)
      {
      U_TRACE(0, "UREDISClient_Base::rpush(%.*S,%u)", len, param, len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("RPUSH"), param, len)) return getUInt8();

      U_RETURN(false);
      }

   // LIST (@see http://redis.io/list)

   bool lpush(const char* key, uint32_t keylen, const char* param, uint32_t len)
      {
      U_TRACE(0, "UREDISClient_Base::lpush((%.*S,%u,%.*S,%u)", keylen, key, keylen, len, param, len)

      if (processRequest(U_RC_INT, U_CONSTANT_TO_PARAM("LPUSH"), key, keylen, param, len)) return getUInt8();

      U_RETURN(false);
      }

   bool lrange(const char* param, uint32_t len) // Get a range of elements from a list
      {
      U_TRACE(0, "UREDISClient_Base::lrange(%.*S,%u)", len, param, len)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("LRANGE"), param, len);
      }

   bool ltrim(const char* key, uint32_t keylen, int32_t _min, int32_t _max)
      {
      U_TRACE(0, "UREDISClient_Base::ltrim(%.*S,%u,%d,%d)", keylen, key, keylen, _min, _max)

      char buf[128];
      uint32_t buf_len = u__snprintf(buf, U_CONSTANT_SIZE(buf), U_CONSTANT_TO_PARAM("%d %d"), _min, _max);

      return processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("LTRIM"), key, keylen, buf, buf_len);
      }

   // MULTI-EXEC (@see https://redis.io/commands/exec)

   bool processMultiRequest(const char* format, uint32_t fmt_size, ...);

   // REDI-SEARCH (@see https://oss.redislabs.com/redisearch/)

   bool suggest(const char* key, uint32_t keyLength, const char* prefix, uint32_t prefixLength, bool fuzzy, bool withPayloads)
      {
      U_TRACE(0, "UREDISClient_Base::suggest(%.*S,%u,%.*S,%u,%b,%b)", keyLength, key, keyLength, prefixLength, prefix, prefixLength, fuzzy, withPayloads)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return processRequest(U_RC_MULTIBULK, U_CONSTANT_TO_PARAM("FT.SUGGET"), u_buffer,
                              u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%.*s %.*s %.*s %.*s"),
                                          keyLength, key, prefixLength, prefix,
                                          (fuzzy        ? U_CONSTANT_SIZE("FUZZY")        : 0), "FUZZY",
                                          (withPayloads ? U_CONSTANT_SIZE("WITHPAYLOADS") : 0), "WITHPAYLOADS"));
      }

   // define method VIRTUAL of class UEventFd

   virtual int handlerRead() U_DECL_FINAL;

   virtual void handlerDelete() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UREDISClient_Base::handlerDelete()")

      U_INTERNAL_DUMP("UEventFd::fd = %d", UEventFd::fd)

      UEventFd::fd = -1;
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   int err;

   static uint32_t start;
   static ptrdiff_t diff;
   static UVector<UString>* pvec;
   static UREDISClient_Base* pthis;

   static UHashMap<void*>* pchannelCallbackMap;

   UREDISClient_Base() : UClient_Base(U_NULLPTR)
      {
      U_TRACE_CTOR(0, UREDISClient_Base, "")

      err = 0;
      }

   void init();
   void processResponse();
   bool processRequest(char recvtype);

   bool sendRequest(const UString& pipeline)
      {
      U_TRACE(0, "UREDISClient_Base::sendRequest(%V)", pipeline.rep)

      UClient_Base::iov[0].iov_base = (caddr_t)pipeline.data();
      UClient_Base::iov[0].iov_len  =          pipeline.size();
      UClient_Base::iov[1].iov_base = (caddr_t)U_CRLF;
      UClient_Base::iov[1].iov_len  =
               UClient_Base::iovcnt = 2;

      return UClient_Base::sendRequest(false);
      }

   bool sendRequest(const char* p1, uint32_t len1, const UString& pipeline)
      {
      U_TRACE(0, "UREDISClient_Base::sendRequest(%.*S,%u,%V)", len1, p1, len1, pipeline.rep)

      UClient_Base::iov[0].iov_base = (caddr_t)p1;
      UClient_Base::iov[0].iov_len  =          len1;
      UClient_Base::iov[1].iov_base = (caddr_t)pipeline.data();
      UClient_Base::iov[1].iov_len  =          pipeline.size();
      UClient_Base::iov[2].iov_base = (caddr_t)U_CRLF;
      UClient_Base::iov[2].iov_len  =
               UClient_Base::iovcnt = 3;

      return UClient_Base::sendRequest(false);
      }

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

   static void manageResponseBufferResize(uint32_t n);

private:
   bool getResponseItem() U_NO_EXPORT;

   friend class UREDISClusterMaster;

// U_DISALLOW_COPY_AND_ASSIGN(UREDISClient_Base)
};

template <class Socket> class U_EXPORT UREDISClient : public UREDISClient_Base {
public:

   UREDISClient() : UREDISClient_Base()
      {
      U_TRACE_CTOR(0, UREDISClient, "")

      U_NEW(Socket, UClient_Base::socket, Socket(UClient_Base::bIPv6));
      }

   ~UREDISClient()
      {
      U_TRACE_DTOR(0, UREDISClient)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UREDISClient_Base::dump(_reset); }
#endif

private:
   
// U_DISALLOW_COPY_AND_ASSIGN(UREDISClient)
};

template <> class U_EXPORT UREDISClient<UUnixSocket> : public UREDISClient_Base {
public:

   UREDISClient() : UREDISClient_Base()
      {
      U_TRACE_CTOR(0, UREDISClient<UUnixSocket>, "")

      U_NEW(UUnixSocket, UClient_Base::socket, UUnixSocket(false));
      }

   ~UREDISClient()
      {
      U_TRACE_DTOR(0, UREDISClient<UUnixSocket>)
      }

   // Connect to REDIS server via pathname (unix socket)

   bool connect(const char* pathname = "/tmp/redis.sock", unsigned int _port = 6379)
      {
      U_TRACE(0, "UREDISClient<UUnixSocket>::connect(%S,%u)", pathname, _port)

      UString path(pathname);

      if (UClient_Base::socket->connectServer(path, port))
         {
         UREDISClient_Base::init();

         U_RETURN(true);
         }

      UClient_Base::response.snprintf(U_CONSTANT_TO_PARAM("Sorry, couldn't connect to unix socket %v%R"), path.rep, 0); // NB: the last argument (0) is necessary...

      U_CLIENT_LOG("%v", UClient_Base::response.rep)

      U_RETURN(false);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UREDISClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UREDISClient<UUnixSocket>)
};

// by Victor Stewart

#if defined(U_STDCPP_ENABLE) && defined(U_LINUX)
#  if defined(HAVE_CXX17)

typedef UREDISClient<UTCPSocket> UREDISClusterClient;

struct RedisClusterNode {

   U_MEMORY_TEST
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR
      
   UString ipAddress;
   UREDISClusterClient *client;
   uint16_t port, lowHashSlot, highHashSlot;

   RedisClusterNode(const UString& _ipAddress, uint16_t _port, uint16_t _lowHashSlot, uint16_t _highHashSlot) : ipAddress(_ipAddress), port(_port), lowHashSlot(_lowHashSlot), highHashSlot(_highHashSlot)
   {
      U_NEW(UREDISClusterClient, client, UREDISClusterClient);
      client->connect(ipAddress.c_str(), port);
   }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return ""; }
#endif
};

enum class ClusterError : uint8_t {
   none,
   moved,
   ask,
   tryagain
};

class AnonymousClusterPipeline;

class U_EXPORT UREDISClusterMaster {
private:

   friend class AnonymousClusterPipeline;

   UREDISClusterClient *subscriptionClient;
   UREDISClusterClient *managementClient;
   UHashMap<RedisClusterNode *> *clusterNodes;

   static uint16_t hashslotForKey(const UString& hashableKey) { return u_crc16(U_STRING_TO_PARAM(hashableKey)) % 16384; }
   
   UREDISClusterClient* clientForHashslot(uint16_t hashslot)
   {
      U_TRACE(0, "UREDISClusterMaster::clientForHashslot(%u)", hashslot)

      for (UHashMapNode *node : *clusterNodes)
      {
         RedisClusterNode* workingNode = (RedisClusterNode *)(node->elem);

         if ((workingNode->lowHashSlot <= hashslot) && (workingNode->highHashSlot >= hashslot)) return workingNode->client;
      }

      return managementClient; // never reached
   }
   
   UREDISClusterClient* clientForIP(const UString& ip)
   {
      for (UHashMapNode *node : *clusterNodes)
      {
         RedisClusterNode* workingNode = (RedisClusterNode *)(node->elem);

         if (ip == workingNode->ipAddress) return workingNode->client;
      }

      return managementClient; // never reached
   }

   UREDISClusterClient* clientForHashableKey(const UString& hashableKey) { return clientForHashslot(hashslotForKey(hashableKey)); }

   // this might delete cluster nodes so be careful of client pointers after
   void calculateNodeMap();

   void sendToCluster(UREDISClusterClient*& workingClient, const UString& hashableKey, const UString& pipeline);
   
public:
   
   U_MEMORY_TEST
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR
      
   bool connect(const char* host = U_NULLPTR, unsigned int _port = 6379);

   UString clusterSingle(const UString& hashableKey, const UString& pipeline)
   { 
      UREDISClusterClient* workingClient;
      sendToCluster(workingClient, hashableKey, pipeline);
      return workingClient->UREDISClient_Base::vitem[0];
   }

   // both of these multis require all keys to exist within a single hash slot (on the same node isn't good enough)
   const UVector<UString>& clusterMulti( const UString& hashableKey, const UString& pipeline)
   { 
      UREDISClusterClient* workingClient;
      sendToCluster(workingClient, hashableKey, pipeline);
      return workingClient->UREDISClient_Base::vitem;
   }

   // if reorderable == false, commands are grouped and pushed SEQUENTIALLY BY HASHSLOT. even if other commands point to hashslots on the same cluster node, we are unable to garuntee ordering since Redis only checks for -MOVED etc errors command by command as it executes them, and does not fail upon reaching a -MOVED etc error. this requires waiting for each response, to ensure no errors occured, before moving onto the next batch of commands.

   // if reorderable == true, we are able to group and push commands BY NODE regardless of sequence. we assume the fast-path 99.999% occurance that -MOVED and other errors did not occur, and push commands to redis as rapidly as possible without waiting on responses. we same a copy of all commands, and then at the end, check for -MOVED or other errors, and correct those if need be, while we process all resposnes.
   const UVector<UString>& clusterAnonMulti(const AnonymousClusterPipeline& pipeline, bool reorderable);

   bool clusterUnsubscribe(const UString& channel); 
   bool clusterSubscribe(  const UString& channel, vPFcscs callback);

   UREDISClusterMaster()
   {
      clusterNodes = U_NULLPTR;
      U_NEW(UREDISClusterClient, managementClient, UREDISClusterClient);
      U_NEW(UREDISClusterClient, subscriptionClient, UREDISClusterClient);
   }
   
   ~UREDISClusterMaster()
   {
      U_DELETE(subscriptionClient);
      if (clusterNodes) U_DELETE(clusterNodes);
   }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return subscriptionClient->UREDISClient_Base::dump(_reset); }
#endif
};

class AnonymousClusterPipeline {
private:

   friend class UREDISClusterMaster;

   struct Span {

      int16_t hashslot;
      size_t beginning, end, index;

      Span(uint16_t _hashslot, size_t _beginning, size_t _end, size_t _index) : hashslot(_hashslot), beginning(_beginning), end(_end), index(_index) {}
   };

   UString pipeline;
   std::vector<Span> spans;

public:
   
   size_t size()
   {
      return pipeline.size();
   }
   
   void setEmpty()
   {
      pipeline.setEmpty();
      spans.clear();
   }
   
   void append(const UString& hashableKey, const UString& command)
   {
      size_t beginning = pipeline.size();

      pipeline.reserve(pipeline.size() + command.size());

      pipeline.append(command);

      spans.emplace_back(UREDISClusterMaster::hashslotForKey(hashableKey), beginning, pipeline.size(), spans.size());
   }

   void append(size_t increaseCapacityBy, const UString& hashableKey, const char* format, uint32_t fmt_size, ...)
   {
      size_t beginning = pipeline.size();

      if (increaseCapacityBy > 0) pipeline.reserve(pipeline.size() + increaseCapacityBy);

      va_list args;
      va_start(args, fmt_size);
      pipeline.vsnprintf_add(format, fmt_size, args);
      va_end(args);

      spans.emplace_back(UREDISClusterMaster::hashslotForKey(hashableKey), beginning, pipeline.size(), spans.size());
   }

   AnonymousClusterPipeline() : pipeline(300U) {}
};
#  endif
#  if defined(HAVE_CXX20)

class UCompileTimeRESPEncoder {
private:
      
   // "HMSET {%v}.cache firstname %v lastname %v picture %fbb \r\n"
   template <class X>
   static constexpr size_t countSegments(X rawFormat)
   {
      size_t index = 0;

      while (rawFormat[index] == ' ') ++index;

      size_t segmentCount = 1;
      bool inSegment = true;

      while (index < rawFormat.length)
      {     
         char ch = rawFormat[index];

         if (ch == '\r') return segmentCount;

         if (inSegment && ch == ' ')
         {
            inSegment = false;
         }
         else if (!inSegment && ch != ' ')
         {
            inSegment = true;
            ++segmentCount;
         }

         ++index;
      }

      return segmentCount;
   }

   enum class ParameterType {

      none,
      _ustring, // v
      cstring,  // s
      fstring,  // fbs, flatbuffer string
      fbinary,  // fbb, flatbuffer binary
      _int32,   // ld
      _int64    // lld
   };

   struct SegmentOutline {

      const size_t start;
      const size_t length;
      const size_t lengthCorrection;
      const ParameterType parameter;
   };

   template <auto rawFormat, size_t argumentCount, size_t segmentCount = countSegments(rawFormat)>
   struct RESPFormatter {
   private:

      // HMSET {%v}.cache firstname %v lastname %v birthdayEpoch %lld \r\n
      // HMSET {%v}.cache firstname %v lastname %v birthdayEpoch %lld\r\n
      // HMSET {%v}.cache firstname %v lastname %v birthdayEpoch %lld
      
      template<ssize_t number, bool terminate = false, typename ...DigitStrings>
      static constexpr auto integerToString(DigitStrings... digitStrings)
      {
         if constexpr (terminate) return ((""_ctv + digitStrings) + ...);
         else
         {
            if constexpr (number < 0) return integerToString<number * -1>("-"_ctv, std::forward<DigitStrings>(digitStrings) ...);
            else
            {
               constexpr size_t digitValue = number % 10;
               constexpr bool newTerminate = number < 10;
               constexpr size_t newNumber = number / 10;

                    if constexpr (digitValue == 0) return integerToString<newNumber, newTerminate>("0"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 1) return integerToString<newNumber, newTerminate>("1"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 2) return integerToString<newNumber, newTerminate>("2"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 3) return integerToString<newNumber, newTerminate>("3"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 4) return integerToString<newNumber, newTerminate>("4"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 5) return integerToString<newNumber, newTerminate>("5"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 6) return integerToString<newNumber, newTerminate>("6"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 7) return integerToString<newNumber, newTerminate>("7"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 8) return integerToString<newNumber, newTerminate>("8"_ctv, std::forward<DigitStrings>(digitStrings) ...);
               else if constexpr (digitValue == 9) return integerToString<newNumber, newTerminate>("9"_ctv, std::forward<DigitStrings>(digitStrings) ...);
            }
         }
      }

      template <size_t ... n>
      static constexpr auto generateSegmentMarkersHelper(std::integer_sequence<size_t, n...>)
      {
         size_t start = 0;
         return std::array<SegmentOutline, segmentCount>{getNextSegmentOutline(start, n) ...};
      }

      static constexpr auto generateSegmentMarkers()
      {
         return generateSegmentMarkersHelper(std::make_integer_sequence<size_t, segmentCount>{});
      }

      static constexpr SegmentOutline getNextSegmentOutline(size_t& start, size_t segmentIndex) // just to use parameter pack expansion
      {
         while (rawFormat[start] == ' ') ++start;

         size_t segmentStart = start;
         size_t lengthCorrection = 0;
         ParameterType parameter = ParameterType::none;

         while (start < rawFormat.length)
         {     
            char ch = rawFormat[start];

            if (ch == ' ' || ch == '\r') break;

            if (ch == '%')
            {
               ch = rawFormat[++start];

               /*
               enum class ParameterType {

                  none,
                  _ustring, // v
                  cstring,  // s
                  fstring,  // fbs, flatbuffer string
                  fbinary,  // fbb, flatbuffer binary
                  _int32,   // ld
                  _int64    // lld
               };*/

               switch (ch)
               {
                  case 'v':
                  {
                     lengthCorrection = 2;
                     parameter = ParameterType::_ustring;
                     break;
                  }
                  case 's':
                  {
                     lengthCorrection = 2;
                     parameter = ParameterType::cstring;
                     break;
                  }
                  case 'f':
                  {
                     lengthCorrection = 4;
                     ch = rawFormat[++(++start)];

                     // %fbs
                     if (ch == 's') parameter = ParameterType::fstring;
                     // %fbb
                     else           parameter = ParameterType::fbinary;

                     break;
                  }
                  case 'l':
                  {
                     ch = rawFormat[++start];

                     // %ld
                     if (ch == 'd')
                     {
                        lengthCorrection = 3;
                        parameter = ParameterType::_int32;
                     }
                     // %lld
                     else
                     {
                        lengthCorrection = 4;
                        parameter = ParameterType::_int64;
                        ++start;
                     }
                     break;
                  }
               }
            }
            
            ++start;
         }

         size_t segmentLength = start - segmentStart;
         return {segmentStart, segmentLength, lengthCorrection, parameter};
      }

      template <auto segmentOutlines, size_t segmentIndex = 0, class StringClass, typename ...StringClasses>
      static constexpr auto parse(StringClass raw, std::array<size_t, argumentCount>& argumentToSegment, size_t argumentIndex = 0, StringClasses... strings)
      {
         if constexpr (segmentIndex >= segmentCount) return (strings + ... );
         else
         {
            constexpr SegmentOutline segmentOutline = segmentOutlines[segmentIndex];

            if constexpr (segmentOutline.parameter == ParameterType::none)
            {
               constexpr auto segmentString = "$"_ctv + integerToString<segmentOutline.length>() + "\r\n"_ctv + StringClass::instance.template substr<segmentOutline.start, segmentOutline.start + segmentOutline.length>() + "\r\n"_ctv;

               return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, argumentIndex, std::forward<StringClasses>(strings)..., segmentString);
            }
            else if constexpr (segmentOutline.parameter == ParameterType::_int32 && segmentOutline.length == 3)
            {
               argumentToSegment[argumentIndex] = segmentIndex;
               return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., ":%ld\r\n"_ctv);
            }
            else if constexpr (segmentOutline.parameter == ParameterType::_int64 && segmentOutline.length == 4)
            {
               argumentToSegment[argumentIndex] = segmentIndex;
               return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., ":%lld\r\n"_ctv);
            }
            else if constexpr (segmentOutline.parameter == ParameterType::fstring || segmentOutline.parameter == ParameterType::fbinary)
            {
               argumentToSegment[argumentIndex] = segmentIndex;

               if constexpr (segmentOutline.lengthCorrection > 4)
               {
                  // replace the fbs or fbb with %.*s
                  size_t start = segmentOutline.start;

                  while (start < rawFormat.length)
                  {     
                     if (rawFormat[start++] == '%') break;
                  }

                  // 2 sub-segments
                  if (start == segmentOutline.start)
                  {
                     constexpr auto segmentString = "$%d\r\n%.*s"_ctv + StringClass::instance.template substr<start + 4, segmentOutline.length - 4>() + "\r\n"_ctv;

                     return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., segmentString);
                  }
                  else if (start == (segmentOutline.start + segmentOutline.length - 4))
                  {
                     constexpr auto segmentString = "$%d\r\n"_ctv + StringClass::instance.template substr<segmentOutline.start, segmentOutline.length - 4>() + "%.*s\r\n"_ctv;

                     return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., segmentString);
                  }
                  // 3 sub-segments
                  else
                  {
                     constexpr auto segmentString = "$%d\r\n"_ctv + StringClass::instance.template substr<segmentOutline.start, start - segmentOutline.start>() + "%.*s"_ctv + StringClass::instance.template substr<start + 4, segmentOutline.length - 4 - (start - segmentOutline.start)>() + "\r\n"_ctv;
                     return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., segmentString);
                  }
               }
               else // segment is only %fbs or %fbb
               {
                  return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., "$%d\r\n%.*s\r\n"_ctv);
               }
            }
            else
            {
               argumentToSegment[argumentIndex] = segmentIndex;

               constexpr auto segmentString = "$%d\r\n"_ctv + StringClass::instance.template substr<segmentOutline.start, segmentOutline.start + segmentOutline.length>() + "\r\n"_ctv;
               return parse<segmentOutlines, segmentIndex + 1>(raw, argumentToSegment, ++argumentIndex, std::forward<StringClasses>(strings)..., segmentString);
            }
         }
      }

   public:

      static constexpr auto parseToRESP()
      {
         constexpr std::array<SegmentOutline, segmentCount> segmentOutlines = generateSegmentMarkers();

         std::array<size_t, argumentCount> argumentToSegment = {};
         
         return std::make_tuple("*"_ctv + integerToString<segmentCount>() + "\r\n"_ctv + parse<segmentOutlines>(rawFormat, argumentToSegment), segmentOutlines, argumentToSegment);
      }
   };

   template <typename T, typename U>
   struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type {};

   template< class T, class U >
   static inline constexpr bool decay_equiv_v = decay_equiv<T, U>::value;

   template <class X>
   static void fill(X respformat, UString& workingString, size_t argumentCount, size_t workingCount)
   {
      const auto& [format, segmentOutlines, argumentToSegment] = respformat;

      workingString.snprintf(format.string, format.length);
   }

   template <class X, typename T, typename ... Ts>
   static void fill(X respformat, UString& workingString, size_t argumentCount, size_t workingCount, T t, Ts... ts)
   {
      const auto& [format, segmentOutlines, argumentToSegment] = respformat;
      const SegmentOutline& outline = segmentOutlines[argumentToSegment[workingCount]];
      const size_t lengthSurplus = outline.length - outline.lengthCorrection;

      if (workingCount++ < argumentCount)
      {
         if constexpr (decay_equiv_v<T, UString>)
         {  
            if constexpr (std::is_pointer_v<T>) // will only accept single pointer depth
            {
               fill(respformat, workingString, argumentCount, workingCount, std::forward<Ts>(ts)..., t->size() + lengthSurplus, t->rep);
            }
            else fill(respformat, workingString, argumentCount, workingCount, std::forward<Ts>(ts)..., t.size() + lengthSurplus, t.rep);
         }
         else if constexpr (decay_equiv_v<T, char>) // only pointers
         {  
            fill(respformat, workingString, argumentCount, workingCount, std::forward<Ts>(ts)..., strlen(t) + lengthSurplus, t);
         }
         else if constexpr (std::is_integral_v<T>) 
         {
            fill(respformat, workingString, argumentCount, workingCount, std::forward<T>(t), std::forward<Ts>(ts)...);
         }
         //#ifdef FLATBUFFERS_H_
         else //if constexpr (decay_equiv_v<T, flatbuffers::String>)
         {
            fill(respformat, workingString, argumentCount, workingCount, std::forward<Ts>(ts)..., t->size() + lengthSurplus, (outline.parameter == ParameterType::fstring ? t->c_str() : (const char *)t->Data()));
         }
         //#endif
      }
      else workingString.snprintf(format.string, format.length, std::forward<T>(t), std::forward<Ts>(ts)...);
   }

public:

   template<auto rawFormat, typename ... Args>
   static void encode(UString& workingString, Args... args)
   {
      constexpr size_t argumentCount = sizeof...(Args);
      constexpr auto respformat = RESPFormatter<rawFormat, argumentCount>::parseToRESP();

      fill(respformat, workingString, argumentCount, 0, std::forward<Args>(args)...);
   }
};
#  endif
#endif
#endif
