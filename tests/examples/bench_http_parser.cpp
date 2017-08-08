// bench_http_parser.cpp

/**
 * Took 3.990086 seconds to run
 * 1253105.750000 req/sec
 *
 * https://api.travis-ci.org/nodejs/http-parser.svg?branch=master
 *
 * Took 5.386795 seconds to run
 * 928195.687500 req/sec
 */

#include <ulib/utility/uhttp.h>

static const char data[] =
    "POST /joyent/http-parser HTTP/1.1\r\n"
    "Host: github.com\r\n"
    "DNT: 1\r\n"
    "Accept-Encoding: gzip, deflate, sdch\r\n"
    "Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_1) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/39.0.2171.65 Safari/537.36\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,"
        "image/webp,*/*;q=0.8\r\n"
    "Referer: https://github.com/joyent/http-parser\r\n"
    "Connection: keep-alive\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Cache-Control: max-age=0\r\n\r\nb\r\nhello world\r\n0\r\n\r\n";

static int bench(int iter_count, int silent)
{
   U_TRACE(5, "bench(%d,%d)", iter_count, silent)

   int i;
   float rps;
   struct timeval start, end;

   u_init_ulib_hostname();

   UClientImage_Base::init();

   UString::str_allocate(STR_ALLOCATE_HTTP);

   if (!silent) (void) gettimeofday(&start, U_NULLPTR);

   for (i = 0; i < iter_count; i++)
      {
      UHTTP::parserExecute(data, sizeof(data)-1);
      }

   if (!silent)
      {
      (void) gettimeofday(&end, U_NULLPTR);

      fprintf(stdout, "Benchmark result:\n");

      rps = (float) (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6f;

      fprintf(stdout, "Took %f seconds to run\n", rps);

      rps = (float) iter_count / rps;

      fprintf(stdout, "%f req/sec\n", rps);

      fflush(stdout);
      }

   return 0;
}

int main(int argc, char** argv, char** env)
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   if (argc == 2 &&
       strcmp(argv[1], "infinite") == 0)
      {
      for (;;) bench(5000000, 1);

      return 0;
      }

   return bench(5000000, 0);
}
