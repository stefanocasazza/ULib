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

class U_EXPORT UREDISClient_Base : public UClient_Base {
public:

   ~UREDISClient_Base() { U_TRACE_DTOR(0, UREDISClient_Base); }

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

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   int err;

   static uint32_t start;
   static ptrdiff_t diff;
   static UREDISClient_Base* pthis;

   UREDISClient_Base() : UClient_Base(U_NULLPTR)
      {
      U_TRACE_CTOR(0, UREDISClient_Base, "")

      err = 0;
      }

   void init();
   void processResponse();
   bool processRequest(char recvtype);

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX20) && defined(U_LINUX) && !defined(__clang__)
   bool sendRequest(UStringType&& pipeline)
#else
   bool sendRequest(const UString& pipeline)
#endif
      {
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

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX20) && defined(U_LINUX) && !defined(__clang__)

class U_EXPORT UREDISClusterClient : public UREDISClient<UTCPSocket>, public UEventFd {
public:

   enum class ClientType : uint8_t {

      subscription,
      management,
      node
   };

   const ClientType type;
   UREDISClusterMaster *master;

   virtual int handlerRead() U_DECL_FINAL;

   virtual void handlerDelete() U_DECL_FINAL
   {
      U_TRACE_NO_PARAM(0, "UREDISClusterClient::handlerDelete()")

      U_INTERNAL_DUMP("UREDISClusterClient::handlerDelete() -> client = %p", this);
      U_INTERNAL_DUMP("UEventFd::fd = %d", UEventFd::fd)

      UEventFd::fd = -1;
   }

   UREDISClusterClient(UREDISClusterMaster *_master, const ClientType _type) : UREDISClient<UTCPSocket>(), type(_type), master(_master) {}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UREDISClient_Base::dump(_reset); }
#endif
};

struct RedisClusterNode {

   U_MEMORY_TEST
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR
      
   UString ipAddress;
   UREDISClusterClient *client;
   uint16_t port, lowHashSlot, highHashSlot;

   RedisClusterNode(UREDISClusterMaster *master, const UString& _ipAddress, uint16_t _port, uint16_t _lowHashSlot, uint16_t _highHashSlot) : ipAddress(_ipAddress), port(_port), lowHashSlot(_lowHashSlot), highHashSlot(_highHashSlot)
   {
      U_NEW(UREDISClusterClient, client, UREDISClusterClient(master, UREDISClusterClient::ClientType::node));
      client->setHostPort(ipAddress, _port);
      client->connect(ipAddress.c_str(), port);
   }

#if defined(DEBUG)
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
   friend class UREDISClusterClient;

   UREDISClusterClient *subscriptionClient;
   UREDISClusterClient *managementClient;
   UHashMap<RedisClusterNode *> *clusterNodes;
   UHashMap<void*>* pchannelCallbackMap;

   static uint16_t hashslotForKey(UStringType&& hashableKey) {return u_crc16(U_STRING_TO_PARAM(hashableKey)) % 16384;}
   
   UREDISClusterClient* clientForHashslot(uint16_t hashslot)
   {
      for (UHashMapNode *node : *clusterNodes)
      {
         RedisClusterNode* workingNode = (RedisClusterNode *)(node->elem);

         if ((workingNode->lowHashSlot <= hashslot) && (workingNode->highHashSlot >= hashslot)) 
         {
            return workingNode->client;
         }
      }

      return U_NULLPTR; // never reached
   }
   
   UREDISClusterClient* clientForIP(const UString& ip)
   {
      for (UHashMapNode *node : *clusterNodes)
      {
         RedisClusterNode* workingNode = (RedisClusterNode *)(node->elem);

         if (ip == workingNode->ipAddress) return workingNode->client;
      }  

      return U_NULLPTR; // never reached
   }

   template <UStringType A>
   UREDISClusterClient* clientForHashableKey(A&& hashableKey) { return clientForHashslot(hashslotForKey(std::forward<A>(hashableKey)));}

   // this might delete cluster nodes so be careful of client pointers after
   void calculateNodeMap();

   static ClusterError checkResponseForClusterErrors(const UString& response, size_t offset);

   template<bool psuedoSilence>
   UREDISClusterClient* sendToCluster(uint16_t hashslot, UStringType&& pipeline, UREDISClusterClient* workingClient)
   {
      U_TRACE_NO_PARAM(0, "UREDISClusterMaster::sendToCluster");

      U_DUMP("pipeline = %.*s", pipeline.size(), pipeline.data());

      ClusterError error;

    retry:

      workingClient->response.setEmpty();
      workingClient->vitem.clear();

      workingClient->sendRequest(pipeline);
      workingClient->readResponse(U_SINGLE_READ);

      error = checkResponseForClusterErrors(workingClient->response, 0);

      while (error != ClusterError::none)
      {
         switch (error)
         {
            case ClusterError::moved:
            {
               calculateNodeMap();
               workingClient = clientForHashslot(hashslot);
               break;
            }
            case ClusterError::ask:
            {
               uint32_t _start = workingClient->response.find(' ', U_CONSTANT_SIZE("-ASK 3999")) + 1,
                           end = workingClient->response.find(':', _start);

               workingClient = clientForIP(workingClient->response.substr(_start, end - _start));
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

      if constexpr (!psuedoSilence) workingClient->processResponse();

      return workingClient;
   }

   template<bool psuedoSilence, UStringType A, UStringType B>
   inline UREDISClusterClient* routeToCluster(A&& hashableKey, B&& pipeline)
   {
      uint16_t hashslot = UREDISClusterMaster::hashslotForKey(std::forward<A>(hashableKey));
      return sendToCluster<psuedoSilence>(hashslot, std::forward<B>(pipeline), clientForHashslot(hashslot));
   }

	public:

   U_MEMORY_TEST
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR
      
   bool connect(const char* host = U_NULLPTR, unsigned int _port = 6379);

   template<UStringType A, UStringType B>
   const UString clusterSingle(A&& hashableKey, B&& pipeline) {return routeToCluster<false>(std::forward<A>(hashableKey), std::forward<B>(pipeline))->vitem[0];}

   // both of these multis require all keys to exist within a single hash slot (on the same node isn't good enough)
   template<UStringType A, UStringType B>
   const UVector<UString>& clusterMulti(A&& hashableKey, B&& pipeline) {return routeToCluster<false>(std::forward<A>(hashableKey), std::forward<B>(pipeline))->vitem;}

// these are "psuedo-silenced", aka we wait on responses to ensure no cluster errors, but don't waste resources processing the responses
   template<UStringType A, UStringType B>
   void clusterSilencedSingle(A&& hashableKey, B&& pipeline) {routeToCluster<true>(std::forward<A>(hashableKey), std::forward<B>(pipeline));}
   template<UStringType A, UStringType B>
   void clusterSilencedMulti(A&& hashableKey, B&& pipeline)  {routeToCluster<true>(std::forward<A>(hashableKey), std::forward<B>(pipeline));}

   // if reorderable == false, commands are grouped and pushed SEQUENTIALLY BY HASHSLOT. even if other commands point to hashslots on the same cluster node, we are unable to garuntee ordering since Redis only checks for -MOVED etc errors command by command as it executes them, and does not fail upon reaching a -MOVED etc error. this requires waiting for each response, to ensure no errors occured, before moving onto the next batch of commands.

   // if reorderable == true, we are able to group and push commands BY NODE regardless of sequence. we assume the fast-path 99.999% occurance that -MOVED and other errors did not occur, and push commands to redis as rapidly as possible without waiting on responses. we same a copy of all commands, and then at the end, check for -MOVED or other errors, and correct those if need be, while we process all resposnes.
   const UVector<UString>& clusterAnonMulti(const AnonymousClusterPipeline& pipeline, bool reorderable);

   void clusterUnsubscribe(const UString& channel); 
   void clusterSubscribe(  const UString& channel, vPFcscs callback);

   UREDISClusterMaster()
   {
      clusterNodes = U_NULLPTR;
      U_NEW(UREDISClusterClient, managementClient, UREDISClusterClient(this, UREDISClusterClient::ClientType::management));
      U_NEW(UREDISClusterClient, subscriptionClient, UREDISClusterClient(this, UREDISClusterClient::ClientType::subscription));
   }
   
   ~UREDISClusterMaster()
   {
      U_DELETE(subscriptionClient);
		U_DELETE(managementClient);
      if (clusterNodes) U_DELETE(clusterNodes);
      if (pchannelCallbackMap) U_DELETE(pchannelCallbackMap);
   }

#if defined(DEBUG)
   const char* dump(bool _reset) const { return subscriptionClient->UREDISClient_Base::dump(_reset); }
#endif
};

class UCompileTimeRESPEncoder : public UCompileTimeStringFormatter {
private:

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"

   template<bool isPartial, size_t workingIndex = 0, size_t workingSegmentCount = 0, typename StringClass, typename... Xs, typename T, typename... Ts>
   static constexpr auto generateSegments(StringClass format, size_t& outputCount, std::tuple<Xs...>&& workingCommand, T&& t, Ts&&... ts)
   {
      constexpr size_t segmentStart = StringClass::instance.find(workingIndex, " "_ctv, StringClass::notChars);

      if constexpr (segmentStart == StringClass::length || StringClass::string[segmentStart] == '\r')
      {  
         if constexpr (isPartial)
         {
            outputCount = workingSegmentCount;
            return workingCommand;
         }
         else
         {  
            constexpr auto segmentCountString = "*"_ctv + integerToString<workingSegmentCount>() + "\r\n"_ctv;
            constexpr size_t nextCommand = StringClass::instance.find(segmentStart, " \r\n"_ctv, StringClass::notChars);

            outputCount += 1;

            if constexpr (nextCommand < StringClass::length)
            {
               return std::apply([&] (auto... params) {

                  return generateSegments<isPartial, nextCommand>(format, outputCount, std::tuple(), std::forward<T>(t), std::forward<Ts>(ts)..., segmentCountString, params...);

               }, workingCommand);
            }
            else return std::tuple_cat(std::forward_as_tuple(ts...), std::tie(segmentCountString), workingCommand);
         }                
      }
      else
      {
         constexpr size_t segmentEnd = StringClass::instance.find(segmentStart, " \r"_ctv);
         constexpr size_t formatStart = StringClass::instance.find(segmentStart, "{"_ctv, StringClass::skipDoubles, segmentEnd);

         if constexpr (formatStart < segmentEnd)
         {
            constexpr size_t formatTermination = formatStart + 1;
            constexpr size_t lengthSurplus = (segmentEnd + formatStart) - (segmentStart + formatTermination) - 1;
            
            // some ocassional bug where the length surplus ctor seems to pull references and the first number value in overflowing, so bypass it for now
            return generateSegments<isPartial, segmentEnd, workingSegmentCount + 1>(format, outputCount, std::tuple_cat(workingCommand, std::make_tuple("$"_ctv, getLength(t) + lengthSurplus, "\r\n"_ctv, StringClass::instance.template substr<segmentStart, formatStart>(), std::forward<T>(t), StringClass::instance.template substr<(std::min(formatTermination + 1, segmentEnd)), segmentEnd>() + "\r\n"_ctv)), std::forward<Ts>(ts)...);
         }
         else
         {
            constexpr auto segmentString = "$"_ctv + integerToString<segmentEnd - segmentStart>() + "\r\n"_ctv + StringClass::instance.template substr<segmentStart, segmentEnd>() + "\r\n"_ctv;

            return generateSegments<isPartial, segmentEnd, workingSegmentCount + 1>(format, outputCount, std::tuple_cat(workingCommand, std::tie(segmentString)), std::forward<T>(t), std::forward<Ts>(ts)...);
         }
      }
   }
#pragma GCC diagnostic pop

   template<bool isPartial, bool overwrite, auto format, typename... Ts>
   static size_t encode_impl(size_t writePosition, UString& workingString, Ts&&... ts)
   {  
      U_TRACE_NO_PARAM(0, "UCompileTimeRESPEncoder::encode_impl()");

      // if partial will output segment count
      // if full, will output command count
      size_t count = 0;

      std::apply([&] (auto... params) {

         UCompileTimeStringFormatter::snprintf_impl<overwrite>(writePosition, workingString, params...);

      }, generateSegments<isPartial>(format, count, std::tuple(), std::forward<Ts>(ts)..., ""_ctv));

      return count;
   }

public:

   // CLIENT REPLY ON
   static constexpr auto CLIENTREPLYON    = "*3\r\n$6\r\nCLIENT\r\n$5\r\nREPLY\r\n$2\r\nON\r\n"_ctv;
   // CLIENT REPLY OFF
   static constexpr auto CLIENTREPLYOFF   = "*3\r\n$6\r\nCLIENT\r\n$5\r\nREPLY\r\n$3\r\nOFF\r\n"_ctv;
   // CLIENT REPLY SKIP
   static constexpr auto CLIENTREPLYSKIP  = "*3\r\n$6\r\nCLIENT\r\n$5\r\nREPLY\r\n$4\r\nSKIP\r\n"_ctv;

// fulls
   template<auto format, typename ... Ts>
   static size_t encode(UString& workingString, Ts&&... ts)
   {
      return encode_impl<false, true, format>(0, workingString, std::forward<Ts>(ts)...);
   }

   template<auto format, typename ... Ts>
   static size_t encode_add(UString& workingString, Ts&&... ts)
   {
      return encode_impl<false, false, format>(workingString.size(), workingString, std::forward<Ts>(ts)...);
   }
   
   template<auto format, typename ... Ts>
   static size_t encode_pos(size_t writePosition, UString& workingString, Ts&&... ts)
   {
      return encode_impl<false, false, format>(writePosition, workingString, std::forward<Ts>(ts)...);
   }

// partials
   template<auto format, typename... Ts>
   static void encode_partial_pos(size_t writePosition, size_t& segmentCountAccumulator, UString& workingString, Ts&&... ts)
   {
      segmentCountAccumulator += encode_impl<true, false, format>(writePosition, workingString, std::forward<Ts>(ts)...);
   }

   template<auto format, typename... Ts>
   static void encode_partial_add(size_t& segmentCountAccumulator, UString& workingString, Ts&&... ts)
   {
      segmentCountAccumulator += encode_impl<true, false, format>(workingString.size(), workingString, std::forward<Ts>(ts)...);
   }

   static void encode_partial_count(UString& workingString, size_t segmentCount)
   {
      UCompileTimeStringFormatter::snprintf_impl<false>(0, workingString, "*"_ctv, segmentCount, "\r\n"_ctv);
   }
};

class AnonymousClusterPipeline {
private:

   friend class UREDISClusterMaster;

   struct Span {

      uint8_t commandCount;
      int16_t hashslot;
      size_t beginning, end, index;

      Span(uint8_t _commandCount, uint16_t _hashslot, size_t _beginning, size_t _end, size_t _index) : commandCount(_commandCount), hashslot(_hashslot), beginning(_beginning), end(_end), index(_index) {}
   };

public:

   UString pipeline;
   std::vector<Span> spans;
   
   size_t size()
   {
      return pipeline.size();
   }
   
   void setEmpty()
   {
      pipeline.setEmpty();
      spans.clear();
   }
         
   template <UStringType A>
   void append(A&& hashableKey, const UString& command, uint8_t commandCount = 1)
   {
      U_TRACE_NO_PARAM(0, "AnonymousClusterPipeline::append(ustring)")

      size_t beginning = pipeline.size();

      pipeline.reserve(pipeline.size() + command.size());

      pipeline.append(command);

      spans.emplace_back(commandCount, UREDISClusterMaster::hashslotForKey(std::forward<A>(hashableKey)), beginning, pipeline.size(), spans.size());

      U_DUMP("appended, %.*s", pipeline.size() - beginning, pipeline.data());
   }

   template <auto format, UStringType A, typename... Ts>
   void append(A&& hashableKey, Ts&&... ts)
   {
      U_TRACE_NO_PARAM(0, "AnonymousClusterPipeline::append(variadic)")

      size_t beginning = pipeline.size();

      size_t commandCount = UCompileTimeRESPEncoder::encode_add<format>(pipeline, std::forward<Ts>(ts)...);

      spans.emplace_back(commandCount, UREDISClusterMaster::hashslotForKey(std::forward<A>(hashableKey)), beginning, pipeline.size(), spans.size());

      U_DUMP("appended, %.*s", pipeline.size() - beginning, pipeline.data());
   }

   AnonymousClusterPipeline() : pipeline(300U) {}
};

#endif
#endif
