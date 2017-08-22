// test_http_parser.cpp

#include "test_http_parser.h"

#include <ulib/url.h>
#include <ulib/utility/uhttp.h>

static struct message _messages[5];

static size_t parse(const char* buf, size_t len, int req = HTTP_REQUEST)
{
   U_TRACE(5, "::parse(%.*S,%u,%d)", len, buf, len, req)

   len = UHTTP::parserExecute(buf, len, req == HTTP_RESPONSE);

   U_RETURN(len);
}

static inline int check_str_eq(const struct message* m, const char* prop, const char* expected, const char* found)
{
   U_TRACE(5, "::check_str_eq(%p,%S,%S,%S)", m, prop, expected, found)

   if ((expected == U_NULLPTR) != (found == U_NULLPTR))
      {
      printf("\n*** Error: %s in '%s' ***\n\n", prop, m->name);
      printf("expected %s\n", (expected == U_NULLPTR) ? "NULL" : expected);
      printf("   found %s\n", (found == U_NULLPTR) ? "NULL" : found);

      U_RETURN(0);
      }

   if (expected != U_NULLPTR && m->body_size && 0 != memcmp(expected, found, m->body_size))
      {
      printf("\n*** Error: %s in '%s' ***\n\n", prop, m->name);
      printf("expected '%s'\n", expected);
      printf("   found '%s'\n", found);

      U_RETURN(0);
      }

   U_RETURN(1);
}

static inline int check_num_eq(const struct message* m, const char* prop, int expected, int found)
{
   U_TRACE(5, "::check_num_eq(%p,%p,%u,%u)", m, prop, expected, found)

   if (expected != found)
      {
      printf("\n*** Error: %s in '%s' ***\n\n", prop, m->name);
      printf("expected %d\n", expected);
      printf("   found %d\n", found);

      U_RETURN(0);
      }

   U_RETURN(1);
}

#define MESSAGE_CHECK_STR_EQ(expected, found, prop) \
  if (!check_str_eq(expected, #prop, expected->prop, found->prop)) return 0

#define MESSAGE_CHECK_NUM_EQ(expected, found, prop) \
  if (!check_num_eq(expected, #prop, expected->prop, found->prop)) return 0

#define MESSAGE_CHECK_URL_EQ(u, expected, found, prop, fn)                   \
do {                                                                         \
  check_str_eq(expected, #prop, expected->prop, u.getFieldValue(fn).data()); \
} while(0)

static int message_eq(int index, int connect, const struct message* expected)
{
   U_TRACE(5, "::message_eq(%d,%d,%p)", index, connect, expected)

   struct message* m = _messages+index;

   if (!check_num_eq(expected, "http_minor", expected->http_minor, (U_http_version ? U_http_version-'0' : 0))) U_RETURN(0);

   if (expected->type == HTTP_REQUEST)
      {
      if (!check_num_eq(expected, "method", expected->method, U_http_method_type)) U_RETURN(0);
      }
   else
      {
      if (!check_num_eq(expected, "status_code", expected->status_code, U_http_info.nResponseCode)) U_RETURN(0);
      }

   /**
    * Check URL components; we can't do this w/ CONNECT since it doesn't send us a well-formed URL
    */

   if (*m->request_url &&
        m->method != HTTP_CONNECT)
      {
      MESSAGE_CHECK_STR_EQ(expected, m, request_url);

      Url u(m->request_url, strlen(m->request_url));

      if (expected->host)
         {
         MESSAGE_CHECK_URL_EQ(u, expected, m, host, Url::U_HOST);
         }

      if (expected->userinfo)
         {
         MESSAGE_CHECK_URL_EQ(u, expected, m, userinfo, Url::U_USERINFO);
         }

      m->port = u.getPortNumber();

      MESSAGE_CHECK_URL_EQ(u, expected, m, query_string, Url::U_QUERY);
      MESSAGE_CHECK_URL_EQ(u, expected, m, fragment, Url::U_FRAGMENT);
      MESSAGE_CHECK_URL_EQ(u, expected, m, request_path, Url::U_PATH);
      MESSAGE_CHECK_NUM_EQ(expected, m, port);
      }

   if (connect)
      {
      if (!check_num_eq(m, "body_size", 0, m->body_size)) U_RETURN(0);
      }
   else if (expected->body_size)
      {
      if (!check_num_eq(expected, "body_size", expected->body_size, UClientImage_Base::body->size())) U_RETURN(0);
      }
   else
      {
      if (!check_str_eq(expected, "body", expected->body, UClientImage_Base::body->data())) U_RETURN(0);
      }

   U_RETURN(1);
}

/**
 * Given a sequence of varargs messages, return the number of them that the
 * parser should successfully parse, taking into account that upgraded
 * messages prevent all subsequent messages from being parsed.
 */

static size_t count_parsed_messages(const size_t nmsgs, ...)
{
   U_TRACE(5, "::count_parsed_messages(%u)", nmsgs)

   size_t i;
   va_list ap;

   va_start(ap, nmsgs);

   for (i = 0; i < nmsgs; i++)
      {
      struct message* m = va_arg(ap, struct message*);

      if (m->upgrade)
         {
         va_end(ap);

         return i + 1;
         }
      }

   va_end(ap);

   return nmsgs;
}

static void print_error(const char* raw, size_t error_location)
{
   U_TRACE(5, "::print_error(%S,%u)", raw, error_location)

   int this_line = 0, char_len = 0;
   size_t len = strlen(raw), error_location_line = 0;

   for (size_t i = 0; i < len; i++)
      {
      if (i == error_location) this_line = 1;

      switch (raw[i])
         {
         case '\r':
            char_len = 2;
            fprintf(stderr, "\\r");
         break;

         case '\n':
            fprintf(stderr, "\\n\n");

            if (this_line) goto print;

            error_location_line = 0;
         continue;

         default:
            char_len = 1;
            fputc(raw[i], stderr);
         break;
         }

      if (!this_line) error_location_line += char_len;
      }

   fprintf(stderr, "[eof]\n");

print:
   for (size_t j = 0; j < error_location_line; j++)
      {
      fputc(' ', stderr);
      }

   fprintf(stderr, "^\n\nerror location: %u\n", (unsigned int)error_location);
}

static void test_message(const struct message* message)
{
   U_TRACE(5+256, "::test_message(%p)", message)

   (void) parse(message->raw, strlen(message->raw), message->type);

   if (!message_eq(0, 0, message)) U_INTERNAL_ASSERT(false)
}

static void test_simple(const char* buf, int err_expected)
{
   U_TRACE(5+256, "::test_simple(%S,%d)", buf, err_expected)

   if (parse(buf, strlen(buf)) &&
       err_expected != 0)
      {
      fprintf(stderr, "\n*** test_simple expected %d, but saw %d ***\n\n%s\n", err_expected, 0, buf);

      U_INTERNAL_ASSERT(false)
      }
}

static void test_invalid_header_field_content_error()
{
   U_TRACE_NO_PARAM(5+256, "::test_invalid_header_field_content_error()")

   if (parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nFoo: F\01ailure"),  HTTP_REQUEST)  ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nFoo: F\01ailure"), HTTP_RESPONSE) ||
       parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nFoo: B\02ar"),      HTTP_REQUEST)  ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nFoo: B\02ar"),     HTTP_RESPONSE))
      {
      fprintf(stderr, "\n*** Error expected but none in invalid header content test ***\n");

      U_INTERNAL_ASSERT(false)
      }
}

static void test_invalid_header_field_token_error()
{
   U_TRACE_NO_PARAM(5+256, "::test_invalid_header_field_token_error()")

   if (parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nFo@: Failure"),      HTTP_REQUEST)  ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nFo@: Failure"),     HTTP_RESPONSE) ||
       parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nFoo\01\test: Bar"),  HTTP_REQUEST)  ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nFoo\01\test: Bar"), HTTP_RESPONSE))
      {
      fprintf(stderr, "\n*** Error expected but none in invalid header token test ***\n");

      U_INTERNAL_ASSERT(false)
      }
}

static void test_double_content_length_error()
{
   U_TRACE_NO_PARAM(5+256, "::test_double_content_length_error()")

   if (parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nContent-Length: 1\r\nContent-Length: 0\r\n\r\n"),  HTTP_REQUEST) ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nContent-Length: 1\r\nContent-Length: 0\r\n\r\n"), HTTP_RESPONSE))
      {
      fprintf(stderr, "\n*** Error expected but none in double content-length test ***\n");

      U_INTERNAL_ASSERT(false)
      }
}

static void test_chunked_content_length_error()
{
   U_TRACE_NO_PARAM(5+256, "::test_chunked_content_length_error()")

   if (parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nTransfer-Encoding: chunked\r\nContent-Length: 1\r\n\r\n"),  HTTP_REQUEST) ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Length: 1\r\n\r\n"), HTTP_RESPONSE))
      {
      fprintf(stderr, "\n*** Error expected but none in chunked content-length test ***\n");

      U_INTERNAL_ASSERT(false)
      }
}

static void test_header_cr_no_lf_error()
{
   U_TRACE_NO_PARAM(5, "::test_header_cr_no_lf_error()")

   /*
   if (parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nFoo: 1\rBar: 1\r\n\r\n"),  HTTP_REQUEST) ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nFoo: 1\rBar: 1\r\n\r\n"), HTTP_RESPONSE))
      {
      fprintf(stderr, "\n*** Error expected but none in header whitespace test ***\n");

      U_INTERNAL_ASSERT(false)
      }
   */
}

static void test_header_overflow_error()
{
   U_TRACE_NO_PARAM(5+256, "::test_header_overflow_error()")

   if (parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nheader-key: header-value\r\n"),  HTTP_REQUEST) ||
       parse(U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\nheader-key: header-value\r\n"), HTTP_RESPONSE))
      {
      fprintf(stderr, "\n*** Error expected but none in header overflow test ***\n");
      }
}

static void test_header_nread_value()
{
   U_TRACE_NO_PARAM(5+256, "::test_header_nread_value()")

   bool result = parse(U_CONSTANT_TO_PARAM("GET / HTTP/1.1\r\nheader: value\nhdr: value\r\n"));

   U_INTERNAL_ASSERT_EQUALS(result, false)
   U_INTERNAL_ASSERT(U_ClientImage_data_missing)
}

static void test_content_length_overflow(const char* buf, size_t buflen, int req, int expect_ok)
{
   U_TRACE(5, "::test_content_length_overflow(%.*S,%u,%d,%d)", buflen, buf, buflen, req, expect_ok)

   bool result = (expect_ok == (parse(buf, buflen, req) != 0));

   U_INTERNAL_ASSERT(result)
}

static void test_header_content_length_overflow_error(void)
{
   U_TRACE_NO_PARAM(5+256, "::test_header_content_length_overflow_error()")

#define X(size)                                                               \
  "HTTP/1.1 200 OK\r\n"                                                       \
  "Content-Length: " #size "\r\n"                                             \
  "\r\n"
  const char a[] = X(1844674407370955160);  /* 2^64 / 10 - 1 */
  const char b[] = X(18446744073709551615); /* 2^64-1 */
  const char c[] = X(18446744073709551616); /* 2^64   */
#undef X
  test_content_length_overflow(a, sizeof(a)-1, HTTP_RESPONSE, 1); /* expect ok      */
  test_content_length_overflow(b, sizeof(b)-1, HTTP_RESPONSE, 0); /* expect failure */
  test_content_length_overflow(c, sizeof(c)-1, HTTP_RESPONSE, 0); /* expect failure */
}

static void test_chunk_content_length_overflow_error(void)
{
   U_TRACE_NO_PARAM(5+256, "::test_chunk_content_length_overflow_error()")

#define X(size)                                                               \
    "HTTP/1.1 200 OK\r\n"                                                     \
    "Transfer-Encoding: chunked\r\n"                                          \
    "\r\n"                                                                    \
    #size "\r\n"                                                              \
    "...\r\n"                                                                 \
    "\r\n"
  const char a[] = X(FFFFFFFFFFFFFFE);   /* 2^64 / 16 - 1 */
  const char b[] = X(FFFFFFFFFFFFFFFF);  /* 2^64-1 */
  const char c[] = X(10000000000000000); /* 2^64   */
#undef X
  test_content_length_overflow(a, sizeof(a)-1, HTTP_RESPONSE, 1); /* expect ok      */
  test_content_length_overflow(b, sizeof(b)-1, HTTP_RESPONSE, 0); /* expect failure */
  test_content_length_overflow(c, sizeof(c)-1, HTTP_RESPONSE, 0); /* expect failure */
}

static void test_no_overflow_long_body(int req, size_t length)
{
   U_TRACE(5+256, "::test_no_overflow_long_body(%d,%u)", req, length)

   size_t i;
   char buf1[3000];
   size_t buf1len = sprintf(buf1, "%s\r\nConnection: Keep-Alive\r\nContent-Length: %lu\r\n\r\n", req == HTTP_REQUEST ? "POST / HTTP/1.0" : "HTTP/1.0 200 OK", (unsigned long)length);

   for (i = 0; i < length; i++) buf1[buf1len+i] = 'a';

   buf1[buf1len+i] = '\0';

   bool result = parse(buf1, buf1len+i, req);

   U_INTERNAL_ASSERT(result)
}

static void test_multiple3(const struct message* r1, const struct message* r2, const struct message* r3, int req = HTTP_REQUEST)
{
   U_TRACE(5+256, "::test_multiple3(%p,%p,%p,%d)", r1, r2, r3, req)

   int message_count = count_parsed_messages(3, r1, r2, r3);

   parse(r1->raw, strlen(r1->raw), req);

   if (!message_eq(0, 0, r1)) U_INTERNAL_ASSERT(false)

   if (message_count > 1)
      {
      parse(r2->raw, strlen(r2->raw), req);

      if (!message_eq(1, 0, r2)) U_INTERNAL_ASSERT(false)
      }

   if (message_count > 2)
      {
      parse(r3->raw, strlen(r3->raw), req);

      if (!message_eq(2, 0, r3)) U_INTERNAL_ASSERT(false)
      }
}

// user required to free the result string terminated by \0

static void create_large_chunked_message(int body_size_in_kb, const char* headers)
{
   U_TRACE(5+256, "::create_large_chunked_message(%d,%S)", body_size_in_kb, headers)

   size_t wrote = 0;
   size_t headers_len = strlen(headers);

   memcpy(large_chunked_message, headers, headers_len);

   wrote += headers_len;

   for (int i = 0; i < body_size_in_kb; i++)
      {
      // write 1kb chunk into the body.
      memcpy(large_chunked_message + wrote, "400\r\n", 5);
      wrote += 5;
      memset(large_chunked_message + wrote, 'C', 1024);
      wrote += 1024;
      strcpy(large_chunked_message + wrote, "\r\n");
      wrote += 2;
      }

   memcpy(large_chunked_message + wrote, "0\r\n\r\n", 6);
}

static void test_message_count_body(const struct message* message)
{
   U_TRACE(5+256, "::test_message_count_body(%p)", message)

   size_t toread = strlen(message->raw), 
            read =  parse(message->raw, strlen(message->raw), message->type);

   if (read != toread)
      {
      print_error(message->raw, read);

      U_INTERNAL_ASSERT(false)
      }

   if (message_eq(0, 0, message) == 0) U_INTERNAL_ASSERT(false)
}

static int http_parser_parse_url(const struct url_test* test, struct http_parser_url* u)
{
   U_TRACE(5, "::http_parser_parse_url(%p,%p)", test, u)

   bool result = parse(test->url, strlen(test->url));

   (void) memcpy(u, &test->u, sizeof(struct http_parser_url));

   return (test->rv ? result == false : result);
}

static void dump_url(const char* url, const struct http_parser_url* u)
{
   U_TRACE(5, "::dump_url(%S,%p)", url, u)

   unsigned int i;

   printf("\tfield_set: %#x, port: %u\n", u->field_set, u->port);

   for (i = 0; i < UF_MAX; i++)
      {
      if ((u->field_set & (1 << i)) == 0)
         {
         printf("\tfield_data[%u]: unset\n", i);

         continue;
         }

      printf("\tfield_data[%u]: off: %u len: %u part: \"%.*s\n\"", i, u->field_data[i].off, u->field_data[i].len, u->field_data[i].len, url + u->field_data[i].off);
      }
}

static void test_parse_url(void)
{
   U_TRACE_NO_PARAM(5+256, "::test_parse_url()")

   unsigned int i, rv;
   struct http_parser_url u;
   const struct url_test* test;

   for (i = 0; i < getNumUrlTests(); i++)
      {
      test = url_tests+i;

      memset(&u, 0, sizeof(u));

      rv = http_parser_parse_url(test, &u);

      if (test->rv == 0)
         {
         if (rv != 0)
            {
            printf("\n*** http_parser_parse_url(\"%s\") \"%s\" test failed, " "unexpected rv %d ***\n\n", test->url, test->name, rv);

            abort();
            }

         if (memcmp(&u, &test->u, sizeof(u)) != 0)
            {
            printf("\n*** http_parser_parse_url(\"%s\") \"%s\" failed ***\n", test->url, test->name);

            printf("target http_parser_url:\n");

            dump_url(test->url, &test->u);

            printf("result http_parser_url:\n");

            dump_url(test->url, &u);

            U_INTERNAL_ASSERT(false)
            }
         }
      else
         {
         /* test->rv != 0 */

         if (rv == 0)
            {
            printf("\n*** http_parser_parse_url(\"%s\") \"%s\" test failed, unexpected rv %d ***\n\n", test->url, test->name, rv);

            U_INTERNAL_ASSERT(false)
            }
         }
      }
}

int main(int argc, char** argv, char** env)
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   int i, j, k, request_count, response_count;

   u_init_ulib_hostname();

   UClientImage_Base::init();

   UString::str_allocate(STR_ALLOCATE_HTTP);

   for ( request_count = 0; requests[  request_count].name;  request_count++);
   for (response_count = 0; responses[response_count].name; response_count++);

   //// API
   test_parse_url();

   //// NREAD
   test_header_nread_value();

   //// OVERFLOW CONDITIONS
   test_header_overflow_error();

   test_no_overflow_long_body(HTTP_REQUEST, 1000);
   test_no_overflow_long_body(HTTP_RESPONSE, 1000);

   test_header_content_length_overflow_error();
   test_chunk_content_length_overflow_error();

   //// HEADER FIELD CONDITIONS
   test_double_content_length_error();
   test_chunked_content_length_error();
   test_header_cr_no_lf_error();

   test_invalid_header_field_token_error();
   test_invalid_header_field_content_error();

   //// RESPONSES

   test_message_count_body(&responses[NO_HEADERS_NO_BODY_404]);
   test_message_count_body(&responses[TRAILING_SPACE_ON_CHUNKED_BODY]);

   // test very large chunked response

   create_large_chunked_message(31337,
      "HTTP/1.0 200 OK\r\n"
      "Transfer-Encoding: chunked\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n");

   for (i = 0; i < MAX_CHUNKS; i++) large_chunked.chunk_lengths[i] = 1024;

   test_message_count_body(&large_chunked);

   for (i = 0; i < response_count; i++) test_message(&responses[i]);

   for (i = 0; i < response_count; i++)
      {
      if (!responses[i].should_keep_alive) continue;

      for (j = 0; j < response_count; j++)
         {
         if (!responses[j].should_keep_alive) continue;

         for (k = 0; k < response_count; k++) test_multiple3(&responses[i], &responses[j], &responses[k], HTTP_RESPONSE);
         }
      }

   test_multiple3(&responses[TRAILING_SPACE_ON_CHUNKED_BODY], &responses[NO_BODY_HTTP10_KA_204], &responses[NO_REASON_PHRASE], HTTP_RESPONSE);
   test_multiple3(&responses[BONJOUR_MADAME_FR],              &responses[UNDERSTORE_HEADER_KEY], &responses[NO_CARRIAGE_RET],  HTTP_RESPONSE);

   puts("responses okay");

   /// REQUESTS

   test_simple("GET / HTP/1.1\r\n\r\n", HPE_INVALID_VERSION);

   // Extended characters - see nodejs/test/parallel/test-http-headers-obstext.js
   test_simple("GET / HTTP/1.1\r\n" "Test: Düsseldorf\r\n", HPE_OK);

   // Well-formed but incomplete
   test_simple("GET / HTTP/1.1\r\n"
   "Content-Type: text/plain\r\n"
   "Content-Length: 6\r\n"
   "\r\n"
   "fooba",
   HPE_OK);

   static const char* all_methods[] = {
   "DELETE",
   "GET",
   "HEAD",
   "POST",
   "PUT",
   //"CONNECT", //CONNECT can't be tested like other methods, it's a tunnel
   "OPTIONS",
   "TRACE",
   "COPY",
   "LOCK",
   "MKCOL",
   "MOVE",
   "PROPFIND",
   "PROPPATCH",
   "SEARCH",
   "UNLOCK",
   "BIND",
   "REBIND",
   "UNBIND",
   "ACL",
   "REPORT",
   "MKACTIVITY",
   "CHECKOUT",
   "MERGE",
   "M-SEARCH",
   "NOTIFY",
   "SUBSCRIBE",
   "UNSUBSCRIBE",
   "PATCH",
   "PURGE",
   "MKCALENDAR",
   "LINK",
   "UNLINK",
   U_NULLPTR };

   const char** this_method;

   for (this_method = all_methods; *this_method; this_method++)
      {
      char buf[200];

      sprintf(buf, "%s / HTTP/1.1\r\n\r\n", *this_method);

      test_simple(buf, HPE_OK);
      }

   static const char* bad_methods[] = {
   "ASDF",
   "C******",
   "COLA",
   "GEM",
   "GETA",
   "M****",
   "MKCOLA",
   "PROPPATCHA",
   "PUN",
   "PX",
   "SA",
   "hello world",
   U_NULLPTR };

   for (this_method = bad_methods; *this_method; this_method++)
      {
      char buf[200];

      sprintf(buf, "%s / HTTP/1.1\r\n\r\n", *this_method);

      test_simple(buf, HPE_INVALID_METHOD);
      }

   // illegal header field name line folding
   test_simple("GET / HTTP/1.1\r\n" "name\r\n" " : value\r\n" "\r\n", 0);

   const char* dumbfuck2 =
   "GET / HTTP/1.1\r\n"
   "X-SSL-Bullshit:   -----BEGIN CERTIFICATE-----\r\n"
   "\tMIIFbTCCBFWgAwIBAgICH4cwDQYJKoZIhvcNAQEFBQAwcDELMAkGA1UEBhMCVUsx\r\n"
   "\tETAPBgNVBAoTCGVTY2llbmNlMRIwEAYDVQQLEwlBdXRob3JpdHkxCzAJBgNVBAMT\r\n"
   "\tAkNBMS0wKwYJKoZIhvcNAQkBFh5jYS1vcGVyYXRvckBncmlkLXN1cHBvcnQuYWMu\r\n"
   "\tdWswHhcNMDYwNzI3MTQxMzI4WhcNMDcwNzI3MTQxMzI4WjBbMQswCQYDVQQGEwJV\r\n"
   "\tSzERMA8GA1UEChMIZVNjaWVuY2UxEzARBgNVBAsTCk1hbmNoZXN0ZXIxCzAJBgNV\r\n"
   "\tBAcTmrsogriqMWLAk1DMRcwFQYDVQQDEw5taWNoYWVsIHBhcmQYJKoZIhvcNAQEB\r\n"
   "\tBQADggEPADCCAQoCggEBANPEQBgl1IaKdSS1TbhF3hEXSl72G9J+WC/1R64fAcEF\r\n"
   "\tW51rEyFYiIeZGx/BVzwXbeBoNUK41OK65sxGuflMo5gLflbwJtHBRIEKAfVVp3YR\r\n"
   "\tgW7cMA/s/XKgL1GEC7rQw8lIZT8RApukCGqOVHSi/F1SiFlPDxuDfmdiNzL31+sL\r\n"
   "\t0iwHDdNkGjy5pyBSB8Y79dsSJtCW/iaLB0/n8Sj7HgvvZJ7x0fr+RQjYOUUfrePP\r\n"
   "\tu2MSpFyf+9BbC/aXgaZuiCvSR+8Snv3xApQY+fULK/xY8h8Ua51iXoQ5jrgu2SqR\r\n"
   "\twgA7BUi3G8LFzMBl8FRCDYGUDy7M6QaHXx1ZWIPWNKsCAwEAAaOCAiQwggIgMAwG\r\n"
   "\tA1UdEwEB/wQCMAAwEQYJYIZIAYb4QgHTTPAQDAgWgMA4GA1UdDwEB/wQEAwID6DAs\r\n"
   "\tBglghkgBhvhCAQ0EHxYdVUsgZS1TY2llbmNlIFVzZXIgQ2VydGlmaWNhdGUwHQYD\r\n"
   "\tVR0OBBYEFDTt/sf9PeMaZDHkUIldrDYMNTBZMIGaBgNVHSMEgZIwgY+AFAI4qxGj\r\n"
   "\tloCLDdMVKwiljjDastqooXSkcjBwMQswCQYDVQQGEwJVSzERMA8GA1UEChMIZVNj\r\n"
   "\taWVuY2UxEjAQBgNVBAsTCUF1dGhvcml0eTELMAkGA1UEAxMCQ0ExLTArBgkqhkiG\r\n"
   "\t9w0BCQEWHmNhLW9wZXJhdG9yQGdyaWQtc3VwcG9ydC5hYy51a4IBADApBgNVHRIE\r\n"
   "\tIjAggR5jYS1vcGVyYXRvckBncmlkLXN1cHBvcnQuYWMudWswGQYDVR0gBBIwEDAO\r\n"
   "\tBgwrBgEEAdkvAQEBAQYwPQYJYIZIAYb4QgEEBDAWLmh0dHA6Ly9jYS5ncmlkLXN1\r\n"
   "\tcHBvcnQuYWMudmT4sopwqlBWsvcHViL2NybC9jYWNybC5jcmwwPQYJYIZIAYb4QgEDBDAWLmh0\r\n"
   "\tdHA6Ly9jYS5ncmlkLXN1cHBvcnQuYWMudWsvcHViL2NybC9jYWNybC5jcmwwPwYD\r\n"
   "\tVR0fBDgwNjA0oDKgMIYuaHR0cDovL2NhLmdyaWQt5hYy51ay9wdWIv\r\n"
   "\tY3JsL2NhY3JsLmNybDANBgkqhkiG9w0BAQUFAAOCAQEAS/U4iiooBENGW/Hwmmd3\r\n"
   "\tXCy6Zrt08YjKCzGNjorT98g8uGsqYjSxv/hmi0qlnlHs+k/3Iobc3LjS5AMYr5L8\r\n"
   "\tUO7OSkgFFlLHQyC9JzPfmLCAugvzEbyv4Olnsr8hbxF1MbKZoQxUZtMVu29wjfXk\r\n"
   "\thTeApBv7eaKCWpSp7MCbvgzm74izKhu3vlDk9w6qVrxePfGgpKPqfHiOoGhFnbTK\r\n"
   "\twTC6o2xq5y0qZ03JonF7OJspEd3I5zKY3E+ov7/ZhW6DqT8UFvsAdjvQbXyhV8Eu\r\n"
   "\tYhixw1aKEPzNjNowuIseVogKOLXxWI5vAi5HgXdS0/ES5gDGsABo4fqovUKlgop3\r\n"
   "\tRA==\r\n"
   "\t-----END CERTIFICATE-----\r\n"
   "\r\n";

   test_simple(dumbfuck2, HPE_OK);

   const char* corrupted_connection =
   "GET / HTTP/1.1\r\n"
   "Host: www.example.com\r\n"
   "Connection\r\033\065\325eep-Alive\r\n"
   "Accept-Encoding: gzip\r\n"
   "\r\n";

   test_simple(corrupted_connection, 0);

   const char* corrupted_header_name =
   "GET / HTTP/1.1\r\n"
   "Host: www.example.com\r\n"
   "X-Some-Header\r\033\065\325eep-Alive\r\n"
   "Accept-Encoding: gzip\r\n"
   "\r\n";

   test_simple(corrupted_header_name, 0);

#if 0
   // NOTE(Wed Nov 18 11:57:27 CET 2009) this seems okay. we just read body until EOF.
   // no content-length error if there is a body without content length

   const char* bad_get_no_headers_no_body = "GET /bad_get_no_headers_no_body/world HTTP/1.1\r\n"
                      "Accept: */*\r\n"
                      "\r\n"
                      "HELLO";

   test_simple(bad_get_no_headers_no_body, 0);
#endif
   /* TODO sending junk and large headers gets rejected */

   /* check to make sure our predefined requests are okay */
   for (i = 0; requests[i].name; i++) test_message(&requests[i]);

   for (i = 0; i < request_count; i++)
      {
      if (!requests[i].should_keep_alive) continue;

      for (j = 0; j < request_count; j++)
         {
         if (!requests[j].should_keep_alive) continue;

         for (k = 0; k < request_count; k++) test_multiple3(&requests[i], &requests[j], &requests[k], HTTP_REQUEST);
         }
      }

   test_multiple3(&requests[GET_NO_HEADERS_NO_BODY],           &requests[GET_ONE_HEADER_NO_BODY],     &requests[GET_NO_HEADERS_NO_BODY],           HTTP_REQUEST);
   test_multiple3(&requests[POST_CHUNKED_ALL_YOUR_BASE],       &requests[POST_IDENTITY_BODY_WORLD],   &requests[GET_FUNKY_CONTENT_LENGTH],         HTTP_REQUEST);
   test_multiple3(&requests[TWO_CHUNKS_MULT_ZERO_END],         &requests[CHUNKED_W_TRAILING_HEADERS], &requests[CHUNKED_W_BULLSHIT_AFTER_LENGTH],  HTTP_REQUEST);
   test_multiple3(&requests[QUERY_URL_WITH_QUESTION_MARK_GET], &requests[PREFIX_NEWLINE_GET],         &requests[CONNECT_REQUEST],                  HTTP_REQUEST);

   puts("requests okay");
}
