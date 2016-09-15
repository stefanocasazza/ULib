// test_redis.cpp

#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/redis.h>

int main(int argc, char *argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UREDISClient<UTCPSocket> rc;

   if (rc.connect())
      {
      char buffer[4096];
      bool ok = rc.auth(U_CONSTANT_TO_PARAM("home"));

      U_INTERNAL_ASSERT(ok == false)

      ok = rc.time();

      U_INTERNAL_ASSERT(ok)
      
      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("TIME = %O\n"), U_OBJECT_TO_TRACE(rc.vitem)));

      ok = rc.echo(U_CONSTANT_TO_PARAM("puppamelo"));

      U_INTERNAL_ASSERT(ok)
      
      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("ECHO = %V\n"), rc.vitem[0].rep));

      ok = rc.ping();

      U_INTERNAL_ASSERT(ok)

      ok = rc.set(U_CONSTANT_TO_PARAM("MYKEY"),
                  U_CONSTANT_TO_PARAM("my-value-tester"));

      U_INTERNAL_ASSERT(ok)

      ok = rc.set(U_CONSTANT_TO_PARAM("MYKEY1"),
                  U_CONSTANT_TO_PARAM("my-value-tester"));

      U_INTERNAL_ASSERT(ok)

      ok = rc.set(U_CONSTANT_TO_PARAM("MYKEY2"),
                  U_CONSTANT_TO_PARAM("my-value-tester"));

      U_INTERNAL_ASSERT(ok)

      ok = rc[U_STRING_FROM_CONSTANT("MYKEY")];

      U_INTERNAL_ASSERT(ok)

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("MYKEY  = %V\n"), rc.vitem[0].rep));

      ok = rc[U_STRING_FROM_CONSTANT("MYKEY1")];

      U_INTERNAL_ASSERT(ok)

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("MYKEY1 = %V\n"), rc.vitem[0].rep));

      ok = rc[U_STRING_FROM_CONSTANT("MYKEY2")];

      U_INTERNAL_ASSERT(ok)

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("MYKEY2 = %V\n"), rc.vitem[0].rep));

      ok = rc.sadd(U_CONSTANT_TO_PARAM("MY_SET"), U_CONSTANT_TO_PARAM("123 14"));

      U_INTERNAL_ASSERT(ok)

      ok = rc.smembers(U_CONSTANT_TO_PARAM("MY_SET"));

      U_INTERNAL_ASSERT(ok)

      ok = rc.deleteSetMembers(U_CONSTANT_TO_PARAM("MY_SET"));

      U_INTERNAL_ASSERT(ok)

      ok = rc.deleteKeys(U_CONSTANT_TO_PARAM("MY*"));

      U_INTERNAL_ASSERT(ok)

      ok = rc.lrange(U_CONSTANT_TO_PARAM("fortunes 0 -1"));

      U_INTERNAL_ASSERT(ok)

   // ok = rc.processRequest(U_RC_INLINE, U_CONSTANT_TO_PARAM("MULTI\r\nLPUSH metavars foo foobar hoge\r\nLRANGE metavars 0 -1\r\nEXEC"));
      ok = rc.quit();

      U_INTERNAL_ASSERT(ok)
      }
}
