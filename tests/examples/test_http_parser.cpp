// test_http_parser.cpp

#include "test_http_parser.h"

#include <ulib/url.h>
#include <ulib/utility/uhttp.h>

static int num_messages;
static struct message _messages[5];

/**
 strnlen() is a POSIX.2008 addition. Can't rely on it being available so define it ourselves

static size_t strnlen(const char* s, size_t maxlen)
{
   const char* p = (const char*) memchr(s, '\0', maxlen);

   if (p == U_NULLPTR) return maxlen;

   return p - s;
}

static size_t strlncat(char* dst, size_t len, const char* src, size_t n)
{
   size_t rlen;
   size_t ncpy;
   size_t slen = strnlen(src, n);
   size_t dlen = strnlen(dst, len);

   if (dlen < len)
      {
      rlen = len - dlen;
      ncpy = slen < rlen ? slen : (rlen - 1);

      (void) memcpy(dst + dlen, src, ncpy);

      dst[dlen + ncpy] = '\0';
      }

// assert(len > slen + dlen);

   return slen + dlen;
}
*/

static size_t strlncpy(char* dst, size_t len, const char* src, size_t n)
{
   size_t ncpy;
   size_t slen = strnlen(src, n);

   if (len > 0)
      {
      ncpy = slen < len ? slen : (len - 1);

      (void) memcpy(dst, src, ncpy);

      dst[ncpy] = '\0';
      }

/* assert(len > slen); */

   return slen;
}

/*
static size_t strlcat(char* dst, const char* src, size_t len) { return strlncat(dst, len, src, (size_t) -1); }
static size_t strlcpy(char* dst, const char* src, size_t len) { return strlncpy(dst, len, src, (size_t) -1); }
*/

static size_t parse(const char* buf, size_t len, int req = HTTP_REQUEST)
{
   U_TRACE(5, "::parse(%.*S,%u,%d)", len, buf, len, req)

   return UHTTP::parserExecute(buf, len, req == HTTP_RESPONSE);
}

static inline int check_str_eq(const struct message* m, const char* prop, const char* expected, const char* found)
{
   U_TRACE(5, "::check_str_eq(%p,%p,%S,%S)", m, prop, expected, found)

   if ((expected == U_NULLPTR) != (found == U_NULLPTR))
      {
      printf("\n*** Error: %s in '%s' ***\n\n", prop, m->name);
      printf("expected %s\n", (expected == U_NULLPTR) ? "NULL" : expected);
      printf("   found %s\n", (found == U_NULLPTR) ? "NULL" : found);

      return 0;
      }

   if (expected != U_NULLPTR && 0 != strcmp(expected, found))
      {
      printf("\n*** Error: %s in '%s' ***\n\n", prop, m->name);
      printf("expected '%s'\n", expected);
      printf("   found '%s'\n", found);

      return 0;
      }

   return 1;
}

static inline int check_num_eq(const struct message* m, const char* prop, int expected, int found)
{
   U_TRACE(5, "::check_num_eq(%p,%p,%u,%u)", m, prop, expected, found)

   if (expected != found)
      {
      printf("\n*** Error: %s in '%s' ***\n\n", prop, m->name);
      printf("expected %d\n", expected);
      printf("   found %d\n", found);

      return 0;
      }

   return 1;
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

   int i;
   struct message* m = _messages+index;

   MESSAGE_CHECK_NUM_EQ(expected, m, http_major);
   MESSAGE_CHECK_NUM_EQ(expected, m, http_minor);

   if (expected->type == HTTP_REQUEST)
      {
      MESSAGE_CHECK_NUM_EQ(expected, m, method);
      }
   else
      {
      MESSAGE_CHECK_NUM_EQ(expected, m, status_code);
      MESSAGE_CHECK_STR_EQ(expected, m, response_status);
      }

   if (!connect)
      {
      MESSAGE_CHECK_NUM_EQ(expected, m, should_keep_alive);
      MESSAGE_CHECK_NUM_EQ(expected, m, message_complete_on_eof);
      }

   /*
   assert(m->message_begin_cb_called);
   assert(m->headers_complete_cb_called);
   assert(m->message_complete_cb_called);
   */

   MESSAGE_CHECK_STR_EQ(expected, m, request_url);

   /*
   * Check URL components; we can't do this w/ CONNECT since it doesn't send us a well-formed URL.
   */

   if (*m->request_url && m->method != HTTP_CONNECT)
      {
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
      check_num_eq(m, "body_size", 0, m->body_size);
      }
   else if (expected->body_size)
      {
      MESSAGE_CHECK_NUM_EQ(expected, m, body_size);
      }
   else
      {
      MESSAGE_CHECK_STR_EQ(expected, m, body);
      }

   if (connect)
      {
      check_num_eq(m, "num_chunks_complete", 0, m->num_chunks_complete);
      }
   else
      {
   /* assert(m->num_chunks == m->num_chunks_complete); */

      MESSAGE_CHECK_NUM_EQ(expected, m, num_chunks_complete);

      for (i = 0; i < m->num_chunks && i < MAX_CHUNKS; i++)
         {
         MESSAGE_CHECK_NUM_EQ(expected, m, chunk_lengths[i]);
         }
      }

   MESSAGE_CHECK_NUM_EQ(expected, m, num_headers);

   int r;
   for (i = 0; i < m->num_headers; i++)
      {
      r = check_str_eq(expected, "header field", expected->headers[i][0], m->headers[i][0]);

      if (!r) return 0;

      r = check_str_eq(expected, "header value", expected->headers[i][1], m->headers[i][1]);

      if (!r) return 0;
      }

   MESSAGE_CHECK_STR_EQ(expected, m, upgrade);

   return 1;
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
      struct message *m = va_arg(ap, struct message *);

      if (m->upgrade)
         {
         va_end(ap);

         return i + 1;
         }
      }

   va_end(ap);

   return nmsgs;
}

/**
 * Given a sequence of bytes and the number of these that we were able to
 * parse, verify that upgrade bodies are correct.

static void upgrade_message_fix(char* body, const size_t nread, const size_t nmsgs, ...)
{
   va_list ap;
   size_t i;
   size_t off = 0;

   va_start(ap, nmsgs);

   for (i = 0; i < nmsgs; i++)
      {
      struct message* m = va_arg(ap, struct message *);

      off += strlen(m->raw);

      if (m->upgrade)
         {
         off -= strlen(m->upgrade);

         // Check the portion of the response after its specified upgrade

         if (!check_str_eq(m, "upgrade", body + off, body + nread)) abort();

         // Fix up the response so that message_eq() will verify the beginning of the upgrade

         *(body + nread + strlen(m->upgrade)) = '\0';

         _messages[num_messages -1 ].upgrade = body + nread;

         va_end(ap);

         return;
         }
      }

   va_end(ap);

   printf("\n\n*** Error: expected a message with upgrade ***\n");

   abort();
}
*/

static void print_error(const char* raw, size_t error_location)
{
   U_TRACE(5, "::print_error(%S,%u)", raw, error_location)

   int this_line = 0, char_len = 0;
   size_t i, j, len = strlen(raw), error_location_line = 0;

   for (i = 0; i < len; i++)
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
   for (j = 0; j < error_location_line; j++)
      {
      fputc(' ', stderr);
      }

   fprintf(stderr, "^\n\nerror location: %u\n", (unsigned int)error_location);
}

static void test_message(const struct message* message)
{
   U_TRACE(5, "::test_message(%p)", message)

   size_t raw_len = strlen(message->raw);
   size_t msg1len;

   for (msg1len = 0; msg1len < raw_len; msg1len++)
      {
      size_t read;
      const char* msg1 = message->raw;
      const char* msg2 = msg1 + msg1len;
      size_t msg2len = raw_len - msg1len;

      if (msg1len)
         {
         read = parse(msg1, msg1len, message->type);

         if (message->upgrade && num_messages > 0)
            {
            _messages[num_messages - 1].upgrade = msg1 + read;

            goto test;
            }

         if (read != msg1len)
            {
            print_error(msg1, read);

            abort();
            }
         }

      read = parse(msg2, msg2len);

      if (message->upgrade)
         {
         _messages[num_messages - 1].upgrade = msg2 + read;

         goto test;
         }

      if (read != msg2len)
         {
         print_error(msg2, read);

         abort();
         }

test: if (num_messages != 1)
         {
         printf("\n*** num_messages != 1 after testing '%s' ***\n\n", message->name);

         abort();
         }

      if (!message_eq(0, 0, message)) abort();
      }
}

static void test_simple(const char* buf, int err_expected)
{
   U_TRACE(5, "::test_simple(%S,%d)", buf, err_expected)

   int err = 0;

   parse(buf, strlen(buf));

   if (err_expected != err)
      {
      fprintf(stderr, "\n*** test_simple expected %d, but saw %d ***\n\n%s\n", err_expected, err, buf);

      abort();
      }
}

static void test_invalid_header_content(int req, const char* str)
{
   U_TRACE(5, "::test_invalid_header_content(%d,%S)", req, str)

   const char* buf = req ? "GET / HTTP/1.1\r\n" : "HTTP/1.1 200 OK\r\n";

   parse(buf, strlen(buf));

   buf = str;
   size_t buflen = strlen(buf);

   if (parse(buf, buflen)) fprintf(stderr, "\n*** Error expected but none in invalid header content test ***\n");
}

static void test_invalid_header_field_content_error(int req)
{
   U_TRACE(5, "::test_invalid_header_field_content_error(%d)", req)

   test_invalid_header_content(req, "Foo: F\01ailure");
   test_invalid_header_content(req, "Foo: B\02ar");
}

static void test_invalid_header_field(int req, const char* str)
{
   U_TRACE(5, "::test_invalid_header_field(%d,%S)", req, str)

   const char* buf = req ? "GET / HTTP/1.1\r\n" : "HTTP/1.1 200 OK\r\n";

   parse(buf, strlen(buf));

   buf = str;
   size_t buflen = strlen(buf);

   if (parse(buf, buflen)) fprintf(stderr, "\n*** Error expected but none in invalid header token test ***\n");
}

static void test_invalid_header_field_token_error(int req)
{
   U_TRACE(5, "::test_invalid_header_field_token_error(%d)", req)

   test_invalid_header_field(req, "Fo@: Failure");
   test_invalid_header_field(req, "Foo\01\test: Bar");
}

static void test_double_content_length_error(int req)
{
   U_TRACE(5, "::test_double_content_length_error(%d)", req)

   const char* buf = req ? "GET / HTTP/1.1\r\n" : "HTTP/1.1 200 OK\r\n";

   parse(buf, strlen(buf));

   buf = "Content-Length: 0\r\nContent-Length: 1\r\n\r\n";
   size_t buflen = strlen(buf);

   if (parse(buf, buflen)) fprintf(stderr, "\n*** Error expected but none in double content-length test ***\n");
}

static void test_chunked_content_length_error(int req)
{
   U_TRACE(5, "::test_chunked_content_length_error(%d)", req)

   const char* buf = req ? "GET / HTTP/1.1\r\n" : "HTTP/1.1 200 OK\r\n";

   parse(buf, strlen(buf));

   buf = "Transfer-Encoding: chunked\r\nContent-Length: 1\r\n\r\n";
   size_t buflen = strlen(buf);

   if (parse(buf, buflen)) fprintf(stderr, "\n*** Error expected but none in chunked content-length test ***\n");
}

static void test_header_cr_no_lf_error(int req)
{
   U_TRACE(5, "::test_header_cr_no_lf_error(%d)", req)

   const char* buf = req ? "GET / HTTP/1.1\r\n" : "HTTP/1.1 200 OK\r\n";

   parse(buf, strlen(buf));

   buf = "Foo: 1\rBar: 1\r\n\r\n";
   size_t buflen = strlen(buf);

   if (parse(buf, buflen)) fprintf(stderr, "\n*** Error expected but none in header whitespace test ***\n");
}

static void test_header_overflow_error(int req)
{
   U_TRACE(5+256, "::test_header_overflow_error(%d)", req)

   const char* buf = req ? "GET / HTTP/1.1\r\n" : "HTTP/1.0 200 OK\r\n";

   parse(buf, strlen(buf), req);

   buf = "header-key: header-value\r\n";
   size_t buflen = strlen(buf);

   if (parse(buf, buflen)) fprintf(stderr, "\n*** Error expected but none in header overflow test ***\n");
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
   U_TRACE_NO_PARAM(5, "::test_header_content_length_overflow_error()")

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
   U_TRACE_NO_PARAM(5, "::test_chunk_content_length_overflow_error()")

#define X(size)                                                               \
    "HTTP/1.1 200 OK\r\n"                                                     \
    "Transfer-Encoding: chunked\r\n"                                          \
    "\r\n"                                                                    \
    #size "\r\n"                                                              \
    "..."
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

static void test_multiple3(const struct message* r1, const struct message* r2, const struct message* r3)
{
   U_TRACE(5, "::test_multiple3(%p,%p,%p)", r1, r2, r3)

   int message_count = count_parsed_messages(3, r1, r2, r3);

   char total[ strlen(r1->raw)
            + strlen(r2->raw)
            + strlen(r3->raw)
            + 1
            ];

   total[0] = '\0';

   strcat(total, r1->raw);
   strcat(total, r2->raw);
   strcat(total, r3->raw);

   parse(total, strlen(total));

   if (!message_eq(0, 0, r1)) abort();
   if (message_count > 1 && !message_eq(1, 0, r2)) abort();
   if (message_count > 2 && !message_eq(2, 0, r3)) abort();
}

/*
 * SCAN through every possible breaking to make sure the
 * parser can handle getting the content in any chunks that
 * might come from the socket
 */

static void test_scan(const struct message* r1, const struct message* r2, const struct message* r3)
{
   U_TRACE(5, "::test_scan(%p,%p,%p)", r1, r2, r3)

   char total[80*1024] = "\0";
   char buf1[80*1024] = "\0";
   char buf2[80*1024] = "\0";
   char buf3[80*1024] = "\0";

   strcat(total, r1->raw);
   strcat(total, r2->raw);
   strcat(total, r3->raw);

   int total_len = strlen(total);

   int total_ops = 2 * (total_len - 1) * (total_len - 2) / 2;
   int ops = 0 ;

   size_t buf1_len, buf2_len, buf3_len;
   int i, j, message_count = count_parsed_messages(3, r1, r2, r3);

   for (int type_both = 0; type_both < 2; type_both++)
      {
      for (j = 2; j < total_len; j++)
         {
         for (i = 1; i < j; i++)
            {
            if (ops % 1000 == 0)
               {
               printf("\b\b\b\b%3.0f%%", 100 * (float)ops /(float)total_ops);

               fflush(stdout);
               }

            ops += 1;

            buf1_len = i;
            strlncpy(buf1, sizeof(buf1), total, buf1_len);
            buf1[buf1_len] = 0;

            buf2_len = j - i;
            strlncpy(buf2, sizeof(buf1), total+i, buf2_len);
            buf2[buf2_len] = 0;

            buf3_len = total_len - j;
            strlncpy(buf3, sizeof(buf1), total+j, buf3_len);
            buf3[buf3_len] = 0;

            parse(buf1, buf1_len);

            parse(buf2, buf2_len);

            parse(buf3, buf3_len);

            if (message_count != num_messages)
               {
               fprintf(stderr, "\n\nParser didn't see %d messages only %d\n", message_count, num_messages);

               goto error;
               }

            if (!message_eq(0, 0, r1))
               {
               fprintf(stderr, "\n\nError matching messages[0] in test_scan.\n");

               goto error;
               }

            if (message_count > 1 && !message_eq(1, 0, r2))
               {
               fprintf(stderr, "\n\nError matching messages[1] in test_scan.\n");

               goto error;
               }

            if (message_count > 2 && !message_eq(2, 0, r3))
               {
               fprintf(stderr, "\n\nError matching messages[2] in test_scan.\n");

               goto error;
               }
            }
         }
      }

   puts("\b\b\b\b100%");

   return;

error:
   fprintf(stderr, "i=%d  j=%d\n", i, j);
   fprintf(stderr, "buf1 (%u) %s\n\n", (unsigned int)buf1_len, buf1);
   fprintf(stderr, "buf2 (%u) %s\n\n", (unsigned int)buf2_len , buf2);
   fprintf(stderr, "buf3 (%u) %s\n", (unsigned int)buf3_len, buf3);

   abort();
}

// user required to free the result string terminated by \0

static void create_large_chunked_message(int body_size_in_kb, const char* headers)
{
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

/* Verify that body and next message won't be parsed in responses to CONNECT */

static void test_message_connect(const struct message* msg)
{
   U_TRACE(5, "::test_message_connect(%p)", msg)

   char* buf = (char*) msg->raw;
   size_t buflen = strlen(msg->raw);

   parse(buf, buflen);

   if (num_messages != 1)
      {
      printf("\n*** num_messages != 1 after testing '%s' ***\n\n", msg->name);

      abort();
      }

   if (!message_eq(0, 1, msg)) abort();
}

static size_t parse_count_body(const char* buf, size_t len)
{
  size_t nparsed = parse(buf, len);

  return nparsed;
}

static void test_message_count_body(const struct message* message)
{
   U_TRACE(5, "::test_message_count_body(%p)", message)

   size_t read;
   size_t l = strlen(message->raw);
   size_t i, toread;
   size_t chunk = 4024;

   for (i = 0; i < l; i+= chunk)
      {
      toread = U_min(l-i, chunk);

      read = parse_count_body(message->raw + i, toread);

      if (read != toread)
         {
         print_error(message->raw, read);

         abort();
         }
      }

   if (num_messages != 1)
      {
      printf("\n*** num_messages != 1 after testing '%s' ***\n\n", message->name);

      abort();
      }

   if (!message_eq(0, 0, message)) abort();
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

   printf("\tfield_set: 0x%x, port: %u\n", u->field_set, u->port);

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

            abort();
            }
         }
      else
         {
         /* test->rv != 0 */

         if (rv == 0)
            {
            printf("\n*** http_parser_parse_url(\"%s\") \"%s\" test failed, unexpected rv %d ***\n\n", test->url, test->name, rv);

            abort();
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
   test_header_overflow_error(HTTP_REQUEST);
   test_header_overflow_error(HTTP_RESPONSE);

   test_no_overflow_long_body(HTTP_REQUEST, 1000);
   test_no_overflow_long_body(HTTP_RESPONSE, 1000);

   test_header_content_length_overflow_error();
   test_chunk_content_length_overflow_error();

   //// HEADER FIELD CONDITIONS
   test_double_content_length_error(HTTP_REQUEST);
   test_chunked_content_length_error(HTTP_REQUEST);
   test_header_cr_no_lf_error(HTTP_REQUEST);
   test_invalid_header_field_token_error(HTTP_REQUEST);
   test_invalid_header_field_content_error(HTTP_REQUEST);
   test_double_content_length_error(HTTP_RESPONSE);
   test_chunked_content_length_error(HTTP_RESPONSE);
   test_header_cr_no_lf_error(HTTP_RESPONSE);
   test_invalid_header_field_token_error(HTTP_RESPONSE);
   test_invalid_header_field_content_error(HTTP_RESPONSE);

   //// RESPONSES

   for (i = 0; i < response_count; i++) test_message(&responses[i]);
   for (i = 0; i < response_count; i++) test_message_connect(&responses[i]);

   for (i = 0; i < response_count; i++)
      {
      if (!responses[i].should_keep_alive) continue;

      for (j = 0; j < response_count; j++)
         {
         if (!responses[j].should_keep_alive) continue;

         for (k = 0; k < response_count; k++) test_multiple3(&responses[i], &responses[j], &responses[k]);
         }
      }

   test_message_count_body(&responses[NO_HEADERS_NO_BODY_404]);
   test_message_count_body(&responses[TRAILING_SPACE_ON_CHUNKED_BODY]);

   // test very large chunked response
   {
   create_large_chunked_message(31337,
      "HTTP/1.0 200 OK\r\n"
      "Transfer-Encoding: chunked\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n");

   for (i = 0; i < MAX_CHUNKS; i++) large_chunked.chunk_lengths[i] = 1024;

   test_message_count_body(&large_chunked);
   }

   printf("response scan 1/2      ");
   test_scan( &responses[TRAILING_SPACE_ON_CHUNKED_BODY]
   , &responses[NO_BODY_HTTP10_KA_204]
   , &responses[NO_REASON_PHRASE]
   );

   printf("response scan 2/2      ");
   test_scan( &responses[BONJOUR_MADAME_FR]
   , &responses[UNDERSTORE_HEADER_KEY]
   , &responses[NO_CARRIAGE_RET]
   );

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
   test_simple("GET / HTTP/1.1\r\n" "name\r\n" " : value\r\n" "\r\n", HPE_INVALID_HEADER_TOKEN);

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

   test_simple(corrupted_connection, HPE_INVALID_HEADER_TOKEN);

   const char* corrupted_header_name =
   "GET / HTTP/1.1\r\n"
   "Host: www.example.com\r\n"
   "X-Some-Header\r\033\065\325eep-Alive\r\n"
   "Accept-Encoding: gzip\r\n"
   "\r\n";

   test_simple(corrupted_header_name, HPE_INVALID_HEADER_TOKEN);

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

         for (k = 0; k < request_count; k++) test_multiple3(&requests[i], &requests[j], &requests[k]);
         }
      }

   printf("request scan 1/4      ");
   test_scan( &requests[GET_NO_HEADERS_NO_BODY]
   , &requests[GET_ONE_HEADER_NO_BODY]
   , &requests[GET_NO_HEADERS_NO_BODY]
   );

   printf("request scan 2/4      ");
   test_scan( &requests[POST_CHUNKED_ALL_YOUR_BASE]
   , &requests[POST_IDENTITY_BODY_WORLD]
   , &requests[GET_FUNKY_CONTENT_LENGTH]
   );

   printf("request scan 3/4      ");
   test_scan( &requests[TWO_CHUNKS_MULT_ZERO_END]
   , &requests[CHUNKED_W_TRAILING_HEADERS]
   , &requests[CHUNKED_W_BULLSHIT_AFTER_LENGTH]
   );

   printf("request scan 4/4      ");
   test_scan( &requests[QUERY_URL_WITH_QUESTION_MARK_GET]
   , &requests[PREFIX_NEWLINE_GET ]
   , &requests[CONNECT_REQUEST]
   );

   puts("requests okay");
}
