/* test_http_parser.h */

#ifndef test_http_parser_h
#define test_http_parser_h 1

#include <ulib/base/base.h>
#include <ulib/internal/chttp.h>

#define MAX_HEADERS 13
#define MAX_ELEMENT_SIZE 2048
#define MAX_CHUNKS 16

#define HTTP_LINK   -1
#define HTTP_UNLINK -2

#define HPE_OK 0
#define HPE_INVALID_METHOD 1
#define HPE_INVALID_VERSION 2
#define HPE_INVALID_HEADER_TOKEN 3

/* REQUESTS */
#define CURL_GET 0
#define FIREFOX_GET 1
#define DUMBFUCK 2
#define FRAGMENT_IN_URI 3
#define GET_NO_HEADERS_NO_BODY 4
#define GET_ONE_HEADER_NO_BODY 5
#define GET_FUNKY_CONTENT_LENGTH 6
#define POST_IDENTITY_BODY_WORLD 7
#define POST_CHUNKED_ALL_YOUR_BASE 8
#define TWO_CHUNKS_MULT_ZERO_END 9
#define CHUNKED_W_TRAILING_HEADERS 10
#define CHUNKED_W_BULLSHIT_AFTER_LENGTH 11
#define WITH_QUOTES 12
#define APACHEBENCH_GET 13
#define QUERY_URL_WITH_QUESTION_MARK_GET 14
#define PREFIX_NEWLINE_GET 15
#define UPGRADE_REQUEST 16
#define CONNECT_REQUEST 17
#define REPORT_REQ 18
#define NO_HTTP_VERSION 19
#define MSEARCH_REQ 20
#define LINE_FOLDING_IN_HEADER 21
#define QUERY_TERMINATED_HOST 22
#define QUERY_TERMINATED_HOSTPORT 23
#define SPACE_TERMINATED_HOSTPORT 24
#define PATCH_REQ 25
#define CONNECT_CAPS_REQUEST 26
#define UTF8_PATH_REQ 27
#define HOSTNAME_UNDERSCORE 28
#define EAT_TRAILING_CRLF_NO_CONNECTION_CLOSE 29
#define EAT_TRAILING_CRLF_WITH_CONNECTION_CLOSE 30
#define PURGE_REQ 31
#define SEARCH_REQ 32
#define PROXY_WITH_BASIC_AUTH 33
#define LINE_FOLDING_IN_HEADER_WITH_LF 34
#define CONNECTION_MULTI 35
#define CONNECTION_MULTI_LWS 36
#define CONNECTION_MULTI_LWS_CRLF 37
#define UPGRADE_POST_REQUEST 38
#define CONNECT_WITH_BODY_REQUEST 39
#define LINK_REQUEST 40
#define UNLINK_REQUEST 41
/* RESPONSES */
#define GOOGLE_301 0
#define NO_CONTENT_LENGTH_RESPONSE 1
#define NO_HEADERS_NO_BODY_404 2
#define NO_REASON_PHRASE 3
#define TRAILING_SPACE_ON_CHUNKED_BODY 4
#define NO_CARRIAGE_RET 5
#define PROXY_CONNECTION 6
#define UNDERSTORE_HEADER_KEY 7
#define BONJOUR_MADAME_FR 8
#define RES_FIELD_UNDERSCORE 9
#define NON_ASCII_IN_STATUS_LINE 10
#define HTTP_VERSION_0_9 11
#define NO_CONTENT_LENGTH_NO_TRANSFER_ENCODING_RESPONSE 12
#define NO_BODY_HTTP10_KA_200 13
#define NO_BODY_HTTP10_KA_204 14
#define NO_BODY_HTTP11_KA_200 15
#define NO_BODY_HTTP11_KA_204 16
#define NO_BODY_HTTP11_NOKA_204 17
#define NO_BODY_HTTP11_KA_CHUNKED_200 18
#define SPACE_IN_FIELD_RES 19
#define AMAZON_COM 20
#define EMPTY_REASON_PHRASE_AFTER_SPACE 20
#define CONTENT_LENGTH_X 21

#ifdef __cplusplus
extern "C" {
#endif

enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };

struct message {
  const char* name; /* for debugging purposes */
  const char* raw;
  int type;
  int method;
  int status_code;
  char response_status[MAX_ELEMENT_SIZE];
  char request_path[MAX_ELEMENT_SIZE];
  char request_url[MAX_ELEMENT_SIZE];
  char fragment[MAX_ELEMENT_SIZE];
  char query_string[MAX_ELEMENT_SIZE];
  char body[MAX_ELEMENT_SIZE];
  size_t body_size;
  const char* host;
  const char* userinfo;
  uint16_t port;
  int num_headers;
  enum { NONE=0, FIELD, VALUE } last_header_element;
  char headers [MAX_HEADERS][2][MAX_ELEMENT_SIZE];
  int should_keep_alive;

  int num_chunks;
  int num_chunks_complete;
  int chunk_lengths[MAX_CHUNKS];

  const char* upgrade; /* upgraded body */

  unsigned short http_major;
  unsigned short http_minor;

  int message_begin_cb_called;
  int headers_complete_cb_called;
  int message_complete_cb_called;
  int message_complete_on_eof;
  int body_is_final;
};

enum UrlFieldType { UF_SCHEMA, UF_HOST, UF_PORT, UF_PATH, UF_QUERY, UF_FRAGMENT, UF_USERINFO, UF_MAX };

struct http_parser_url {
	uint16_t field_set; /* Bitmask of (1 << UF_*) values */
	uint16_t port;      /* Converted UF_PORT string */

	struct {
		uint16_t off;     /* Offset into buffer in which field starts */
		uint16_t len;     /* Length of run in buffer */
	} field_data[7];
};

struct url_test {
	const char* name;
	const char* url;
	int is_connect;
	struct http_parser_url u;
	int rv;
};

extern U_EXPORT const struct message requests[];  /* * R E Q U E S T S   * */
extern U_EXPORT const struct message responses[]; /* * R E S P O N S E S * */
extern U_EXPORT		 struct message large_chunked;

extern U_EXPORT const struct url_test url_tests[];

extern U_EXPORT char large_chunked_message[sizeof("HTTP/1.0 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n")+(5+1024+2)*31337+5];

U_EXPORT unsigned int getNumUrlTests(void);

#ifdef __cplusplus
}
#endif
#endif
