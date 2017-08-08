/* ctest_http_parser.c */

#include "test_http_parser.h"

/* * R E Q U E S T S * */
const struct message requests[] =
{ {.name= "curl get"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /test HTTP/1.1\r\n"
         "User-Agent: curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1\r\n"
         "Host: 0.0.0.0=5000\r\n"
         "Accept: */*\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/test"
  ,.request_url= "/test"
  ,.num_headers= 3
  ,.headers=
    { { "User-Agent", "curl/7.18.0 (i486-pc-linux-gnu) libcurl/7.18.0 OpenSSL/0.9.8g zlib/1.2.3.3 libidn/1.1" }
    , { "Host", "0.0.0.0=5000" }
    , { "Accept", "*/*" }
    }
  ,.body= ""
  }

, {.name= "firefox get"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /favicon.ico HTTP/1.1\r\n"
         "Host: 0.0.0.0=5000\r\n"
         "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
         "Accept-Language: en-us,en;q=0.5\r\n"
         "Accept-Encoding: gzip,deflate\r\n"
         "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
         "Keep-Alive: 300\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/favicon.ico"
  ,.request_url= "/favicon.ico"
  ,.num_headers= 8
  ,.headers=
    { { "Host", "0.0.0.0=5000" }
    , { "User-Agent", "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0" }
    , { "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" }
    , { "Accept-Language", "en-us,en;q=0.5" }
    , { "Accept-Encoding", "gzip,deflate" }
    , { "Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.7" }
    , { "Keep-Alive", "300" }
    , { "Connection", "keep-alive" }
    }
  ,.body= ""
  }

, {.name= "dumbfuck"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /dumbfuck HTTP/1.1\r\n"
         "aaaaaaaaaaaaa:++++++++++\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/dumbfuck"
  ,.request_url= "/dumbfuck"
  ,.num_headers= 1
  ,.headers=
    { { "aaaaaaaaaaaaa",  "++++++++++" }
    }
  ,.body= ""
  }

, {.name= "fragment in url"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /forums/1/topics/2375?page=1#posts-17408 HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= "page=1"
  ,.fragment= "posts-17408"
  ,.request_path= "/forums/1/topics/2375"
  /* XXX request url does include fragment? */
  ,.request_url= "/forums/1/topics/2375?page=1#posts-17408"
  ,.num_headers= 0
  ,.body= ""
  }

, {.name= "get no headers no body"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /get_no_headers_no_body/world HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false /* would need Connection: close */
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/get_no_headers_no_body/world"
  ,.request_url= "/get_no_headers_no_body/world"
  ,.num_headers= 0
  ,.body= ""
  }

, {.name= "get one header no body"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /get_one_header_no_body HTTP/1.1\r\n"
         "Accept: */*\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false /* would need Connection: close */
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/get_one_header_no_body"
  ,.request_url= "/get_one_header_no_body"
  ,.num_headers= 1
  ,.headers=
    { { "Accept" , "*/*" }
    }
  ,.body= ""
  }

, {.name= "get funky content length body hello"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /get_funky_content_length_body_hello HTTP/1.0\r\n"
         "conTENT-Length: 5\r\n"
         "\r\n"
         "HELLO"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/get_funky_content_length_body_hello"
  ,.request_url= "/get_funky_content_length_body_hello"
  ,.num_headers= 1
  ,.headers=
    { { "conTENT-Length" , "5" }
    }
  ,.body= "HELLO"
  }

, {.name= "post identity body world"
  ,.type= HTTP_REQUEST
  ,.raw= "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
         "Accept: */*\r\n"
         "Transfer-Encoding: identity\r\n"
         "Content-Length: 5\r\n"
         "\r\n"
         "World"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= "q=search"
  ,.fragment= "hey"
  ,.request_path= "/post_identity_body_world"
  ,.request_url= "/post_identity_body_world?q=search#hey"
  ,.num_headers= 3
  ,.headers=
    { { "Accept", "*/*" }
    , { "Transfer-Encoding", "identity" }
    , { "Content-Length", "5" }
    }
  ,.body= "World"
  }

, {.name= "post - chunked body: all your base are belong to us"
  ,.type= HTTP_REQUEST
  ,.raw= "POST /post_chunked_all_your_base HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "1e\r\nall your base are belong to us\r\n"
         "0\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/post_chunked_all_your_base"
  ,.request_url= "/post_chunked_all_your_base"
  ,.num_headers= 1
  ,.headers=
    { { "Transfer-Encoding" , "chunked" }
    }
  ,.body= "all your base are belong to us"
  ,.num_chunks_complete= 2
  ,.chunk_lengths= { 0x1e }
  }

, {.name= "two chunks ; triple zero ending"
  ,.type= HTTP_REQUEST
  ,.raw= "POST /two_chunks_mult_zero_end HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhello\r\n"
         "6\r\n world\r\n"
         "000\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/two_chunks_mult_zero_end"
  ,.request_url= "/two_chunks_mult_zero_end"
  ,.num_headers= 1
  ,.headers=
    { { "Transfer-Encoding", "chunked" }
    }
  ,.body= "hello world"
  ,.num_chunks_complete= 3
  ,.chunk_lengths= { 5, 6 }
  }

, {.name= "chunked with trailing headers. blech."
  ,.type= HTTP_REQUEST
  ,.raw= "POST /chunked_w_trailing_headers HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhello\r\n"
         "6\r\n world\r\n"
         "0\r\n"
         "Vary: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/chunked_w_trailing_headers"
  ,.request_url= "/chunked_w_trailing_headers"
  ,.num_headers= 3
  ,.headers=
    { { "Transfer-Encoding",  "chunked" }
    , { "Vary", "*" }
    , { "Content-Type", "text/plain" }
    }
  ,.body= "hello world"
  ,.num_chunks_complete= 3
  ,.chunk_lengths= { 5, 6 }
  }

, {.name= "with bullshit after the length"
  ,.type= HTTP_REQUEST
  ,.raw= "POST /chunked_w_bullshit_after_length HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5; ihatew3;whatthefuck=aretheseparametersfor\r\nhello\r\n"
         "6; blahblah; blah\r\n world\r\n"
         "0\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/chunked_w_bullshit_after_length"
  ,.request_url= "/chunked_w_bullshit_after_length"
  ,.num_headers= 1
  ,.headers=
    { { "Transfer-Encoding", "chunked" }
    }
  ,.body= "hello world"
  ,.num_chunks_complete= 3
  ,.chunk_lengths= { 5, 6 }
  }

, {.name= "with quotes"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /with_\"stupid\"_quotes?foo=\"bar\" HTTP/1.1\r\n\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= "foo=\"bar\""
  ,.fragment= ""
  ,.request_path= "/with_\"stupid\"_quotes"
  ,.request_url= "/with_\"stupid\"_quotes?foo=\"bar\""
  ,.num_headers= 0
  ,.headers= { }
  ,.body= ""
  }

/* The server receiving this request SHOULD NOT wait for EOF
 * to know that content-length == 0.
 * How to represent this in a unit test? message_complete_on_eof
 * Compare with NO_CONTENT_LENGTH_RESPONSE.
 */
, {.name = "apachebench get"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /test HTTP/1.0\r\n"
         "Host: 0.0.0.0:5000\r\n"
         "User-Agent: ApacheBench/2.3\r\n"
         "Accept: */*\r\n\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/test"
  ,.request_url= "/test"
  ,.num_headers= 3
  ,.headers= { { "Host", "0.0.0.0:5000" }
             , { "User-Agent", "ApacheBench/2.3" }
             , { "Accept", "*/*" }
             }
  ,.body= ""
  }

/* Some clients include '?' characters in query strings.
 */
, {.name = "query url with question mark"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /test.cgi?foo=bar?baz HTTP/1.1\r\n\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= "foo=bar?baz"
  ,.fragment= ""
  ,.request_path= "/test.cgi"
  ,.request_url= "/test.cgi?foo=bar?baz"
  ,.num_headers= 0
  ,.headers= {}
  ,.body= ""
  }

/* Some clients, especially after a POST in a keep-alive connection,
 * will send an extra CRLF before the next request
 */
, {.name = "newline prefix get"
  ,.type= HTTP_REQUEST
  ,.raw= "\r\nGET /test HTTP/1.1\r\n\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/test"
  ,.request_url= "/test"
  ,.num_headers= 0
  ,.headers= { }
  ,.body= ""
  }

, {.name = "upgrade request"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /demo HTTP/1.1\r\n"
         "Host: example.com\r\n"
         "Connection: Upgrade\r\n"
         "Sec-WebSocket-Key2: 12998 5 Y3 1  .P00\r\n"
         "Sec-WebSocket-Protocol: sample\r\n"
         "Upgrade: WebSocket\r\n"
         "Sec-WebSocket-Key1: 4 @1  46546xW%0l 1 5\r\n"
         "Origin: http://example.com\r\n"
         "\r\n"
         "Hot diggity dogg"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/demo"
  ,.request_url= "/demo"
  ,.num_headers= 7
  ,.upgrade="Hot diggity dogg"
  ,.headers= { { "Host", "example.com" }
             , { "Connection", "Upgrade" }
             , { "Sec-WebSocket-Key2", "12998 5 Y3 1  .P00" }
             , { "Sec-WebSocket-Protocol", "sample" }
             , { "Upgrade", "WebSocket" }
             , { "Sec-WebSocket-Key1", "4 @1  46546xW%0l 1 5" }
             , { "Origin", "http://example.com" }
             }
  ,.body= ""
  }

, {.name = "connect request"
  ,.type= HTTP_REQUEST
  ,.raw= "CONNECT 0-home0.netscape.com:443 HTTP/1.0\r\n"
         "User-agent: Mozilla/1.1N\r\n"
         "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
         "\r\n"
         "some data\r\n"
         "and yet even more data"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.method= HTTP_CONNECT
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= ""
  ,.request_url= "0-home0.netscape.com:443"
  ,.num_headers= 2
  ,.upgrade="some data\r\nand yet even more data"
  ,.headers= { { "User-agent", "Mozilla/1.1N" }
             , { "Proxy-authorization", "basic aGVsbG86d29ybGQ=" }
             }
  ,.body= ""
  }

, {.name= "report request"
  ,.type= HTTP_REQUEST
  ,.raw= "REPORT /test HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_REPORT
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/test"
  ,.request_url= "/test"
  ,.num_headers= 0
  ,.headers= {}
  ,.body= ""
  }

, {.name= "request with no http version"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 0
  ,.http_minor= 9
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/"
  ,.request_url= "/"
  ,.num_headers= 0
  ,.headers= {}
  ,.body= ""
  }

, {.name= "m-search request"
  ,.type= HTTP_REQUEST
  ,.raw= "M-SEARCH * HTTP/1.1\r\n"
         "HOST: 239.255.255.250:1900\r\n"
         "MAN: \"ssdp:discover\"\r\n"
         "ST: \"ssdp:all\"\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_MSEARCH
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "*"
  ,.request_url= "*"
  ,.num_headers= 3
  ,.headers= { { "HOST", "239.255.255.250:1900" }
             , { "MAN", "\"ssdp:discover\"" }
             , { "ST", "\"ssdp:all\"" }
             }
  ,.body= ""
  }

, {.name= "line folding in header value"
  ,.type= HTTP_REQUEST
  ,.raw= "GET / HTTP/1.1\r\n"
         "Line1:   abc\r\n"
         "\tdef\r\n"
         " ghi\r\n"
         "\t\tjkl\r\n"
         "  mno \r\n"
         "\t \tqrs\r\n"
         "Line2: \t line2\t\r\n"
         "Line3:\r\n"
         " line3\r\n"
         "Line4: \r\n"
         " \r\n"
         "Connection:\r\n"
         " close\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/"
  ,.request_url= "/"
  ,.num_headers= 5
  ,.headers= { { "Line1", "abc\tdef ghi\t\tjkl  mno \t \tqrs" }
             , { "Line2", "line2\t" }
             , { "Line3", "line3" }
             , { "Line4", "" }
             , { "Connection", "close" },
             }
  ,.body= ""
  }


, {.name= "host terminated by a query string"
  ,.type= HTTP_REQUEST
  ,.raw= "GET http://hypnotoad.org?hail=all HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= "hail=all"
  ,.fragment= ""
  ,.request_path= ""
  ,.request_url= "http://hypnotoad.org?hail=all"
  ,.host= "hypnotoad.org"
  ,.num_headers= 0
  ,.headers= { }
  ,.body= ""
  }

, {.name= "host:port terminated by a query string"
  ,.type= HTTP_REQUEST
  ,.raw= "GET http://hypnotoad.org:1234?hail=all HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= "hail=all"
  ,.fragment= ""
  ,.request_path= ""
  ,.request_url= "http://hypnotoad.org:1234?hail=all"
  ,.host= "hypnotoad.org"
  ,.port= 1234
  ,.num_headers= 0
  ,.headers= { }
  ,.body= ""
  }

, {.name= "host:port terminated by a space"
  ,.type= HTTP_REQUEST
  ,.raw= "GET http://hypnotoad.org:1234 HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= ""
  ,.request_url= "http://hypnotoad.org:1234"
  ,.host= "hypnotoad.org"
  ,.port= 1234
  ,.num_headers= 0
  ,.headers= { }
  ,.body= ""
  }

, {.name = "PATCH request"
  ,.type= HTTP_REQUEST
  ,.raw= "PATCH /file.txt HTTP/1.1\r\n"
         "Host: www.example.com\r\n"
         "Content-Type: application/example\r\n"
         "If-Match: \"e0023aa4e\"\r\n"
         "Content-Length: 10\r\n"
         "\r\n"
         "cccccccccc"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_PATCH
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/file.txt"
  ,.request_url= "/file.txt"
  ,.num_headers= 4
  ,.headers= { { "Host", "www.example.com" }
             , { "Content-Type", "application/example" }
             , { "If-Match", "\"e0023aa4e\"" }
             , { "Content-Length", "10" }
             }
  ,.body= "cccccccccc"
  }

, {.name = "connect caps request"
  ,.type= HTTP_REQUEST
  ,.raw= "CONNECT HOME0.NETSCAPE.COM:443 HTTP/1.0\r\n"
         "User-agent: Mozilla/1.1N\r\n"
         "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.method= HTTP_CONNECT
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= ""
  ,.request_url= "HOME0.NETSCAPE.COM:443"
  ,.num_headers= 2
  ,.upgrade=""
  ,.headers= { { "User-agent", "Mozilla/1.1N" }
             , { "Proxy-authorization", "basic aGVsbG86d29ybGQ=" }
             }
  ,.body= ""
  }

, {.name= "utf-8 path request"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /δ¶/δt/pope?q=1#narf HTTP/1.1\r\n"
         "Host: github.com\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= "q=1"
  ,.fragment= "narf"
  ,.request_path= "/δ¶/δt/pope"
  ,.request_url= "/δ¶/δt/pope?q=1#narf"
  ,.num_headers= 1
  ,.headers= { {"Host", "github.com" }
             }
  ,.body= ""
  }

, {.name = "hostname underscore"
  ,.type= HTTP_REQUEST
  ,.raw= "CONNECT home_0.netscape.com:443 HTTP/1.0\r\n"
         "User-agent: Mozilla/1.1N\r\n"
         "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.method= HTTP_CONNECT
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= ""
  ,.request_url= "home_0.netscape.com:443"
  ,.num_headers= 2
  ,.upgrade=""
  ,.headers= { { "User-agent", "Mozilla/1.1N" }
             , { "Proxy-authorization", "basic aGVsbG86d29ybGQ=" }
             }
  ,.body= ""
  }

/* see https://github.com/ry/http-parser/issues/47 */
, {.name = "eat CRLF between requests, no \"Connection: close\" header"
  ,.raw= "POST / HTTP/1.1\r\n"
         "Host: www.example.com\r\n"
         "Content-Type: application/x-www-form-urlencoded\r\n"
         "Content-Length: 4\r\n"
         "\r\n"
         "q=42\r\n" /* note the trailing CRLF */
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/"
  ,.request_url= "/"
  ,.num_headers= 3
  ,.upgrade= 0
  ,.headers= { { "Host", "www.example.com" }
             , { "Content-Type", "application/x-www-form-urlencoded" }
             , { "Content-Length", "4" }
             }
  ,.body= "q=42"
  }

/* see https://github.com/ry/http-parser/issues/47 */
, {.name = "eat CRLF between requests even if \"Connection: close\" is set"
  ,.raw= "POST / HTTP/1.1\r\n"
         "Host: www.example.com\r\n"
         "Content-Type: application/x-www-form-urlencoded\r\n"
         "Content-Length: 4\r\n"
         "Connection: close\r\n"
         "\r\n"
         "q=42\r\n" /* note the trailing CRLF */
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false /* input buffer isn't empty when on_message_complete is called */
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/"
  ,.request_url= "/"
  ,.num_headers= 4
  ,.upgrade= 0
  ,.headers= { { "Host", "www.example.com" }
             , { "Content-Type", "application/x-www-form-urlencoded" }
             , { "Content-Length", "4" }
             , { "Connection", "close" }
             }
  ,.body= "q=42"
  }

, {.name = "PURGE request"
  ,.type= HTTP_REQUEST
  ,.raw= "PURGE /file.txt HTTP/1.1\r\n"
         "Host: www.example.com\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_PURGE
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/file.txt"
  ,.request_url= "/file.txt"
  ,.num_headers= 1
  ,.headers= { { "Host", "www.example.com" } }
  ,.body= ""
  }

, {.name = "SEARCH request"
  ,.type= HTTP_REQUEST
  ,.raw= "SEARCH / HTTP/1.1\r\n"
         "Host: www.example.com\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_SEARCH
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/"
  ,.request_url= "/"
  ,.num_headers= 1
  ,.headers= { { "Host", "www.example.com" } }
  ,.body= ""
  }

, {.name= "host:port and basic_auth"
  ,.type= HTTP_REQUEST
  ,.raw= "GET http://a%12:b!&*$@hypnotoad.org:1234/toto HTTP/1.1\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.fragment= ""
  ,.request_path= "/toto"
  ,.request_url= "http://a%12:b!&*$@hypnotoad.org:1234/toto"
  ,.host= "hypnotoad.org"
  ,.userinfo= "a%12:b!&*$"
  ,.port= 1234
  ,.num_headers= 0
  ,.headers= { }
  ,.body= ""
  }

, {.name= "line folding in header value"
  ,.type= HTTP_REQUEST
  ,.raw= "GET / HTTP/1.1\n"
         "Line1:   abc\n"
         "\tdef\n"
         " ghi\n"
         "\t\tjkl\n"
         "  mno \n"
         "\t \tqrs\n"
         "Line2: \t line2\t\n"
         "Line3:\n"
         " line3\n"
         "Line4: \n"
         " \n"
         "Connection:\n"
         " close\n"
         "\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/"
  ,.request_url= "/"
  ,.num_headers= 5
  ,.headers= { { "Line1", "abc\tdef ghi\t\tjkl  mno \t \tqrs" }
             , { "Line2", "line2\t" }
             , { "Line3", "line3" }
             , { "Line4", "" }
             , { "Connection", "close" },
             }
  ,.body= ""
  }

, {.name = "multiple connection header values with folding"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /demo HTTP/1.1\r\n"
         "Host: example.com\r\n"
         "Connection: Something,\r\n"
         " Upgrade, ,Keep-Alive\r\n"
         "Sec-WebSocket-Key2: 12998 5 Y3 1  .P00\r\n"
         "Sec-WebSocket-Protocol: sample\r\n"
         "Upgrade: WebSocket\r\n"
         "Sec-WebSocket-Key1: 4 @1  46546xW%0l 1 5\r\n"
         "Origin: http://example.com\r\n"
         "\r\n"
         "Hot diggity dogg"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/demo"
  ,.request_url= "/demo"
  ,.num_headers= 7
  ,.upgrade="Hot diggity dogg"
  ,.headers= { { "Host", "example.com" }
             , { "Connection", "Something, Upgrade, ,Keep-Alive" }
             , { "Sec-WebSocket-Key2", "12998 5 Y3 1  .P00" }
             , { "Sec-WebSocket-Protocol", "sample" }
             , { "Upgrade", "WebSocket" }
             , { "Sec-WebSocket-Key1", "4 @1  46546xW%0l 1 5" }
             , { "Origin", "http://example.com" }
             }
  ,.body= ""
  }

, {.name = "multiple connection header values with folding and lws"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /demo HTTP/1.1\r\n"
         "Connection: keep-alive, upgrade\r\n"
         "Upgrade: WebSocket\r\n"
         "\r\n"
         "Hot diggity dogg"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/demo"
  ,.request_url= "/demo"
  ,.num_headers= 2
  ,.upgrade="Hot diggity dogg"
  ,.headers= { { "Connection", "keep-alive, upgrade" }
             , { "Upgrade", "WebSocket" }
             }
  ,.body= ""
  }

, {.name = "multiple connection header values with folding and lws"
  ,.type= HTTP_REQUEST
  ,.raw= "GET /demo HTTP/1.1\r\n"
         "Connection: keep-alive, \r\n upgrade\r\n"
         "Upgrade: WebSocket\r\n"
         "\r\n"
         "Hot diggity dogg"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_GET
  ,.query_string= ""
  ,.fragment= ""
  ,.request_path= "/demo"
  ,.request_url= "/demo"
  ,.num_headers= 2
  ,.upgrade="Hot diggity dogg"
  ,.headers= { { "Connection", "keep-alive,  upgrade" }
             , { "Upgrade", "WebSocket" }
             }
  ,.body= ""
  }

, {.name = "upgrade post request"
  ,.type= HTTP_REQUEST
  ,.raw= "POST /demo HTTP/1.1\r\n"
         "Host: example.com\r\n"
         "Connection: Upgrade\r\n"
         "Upgrade: HTTP/2.0\r\n"
         "Content-Length: 15\r\n"
         "\r\n"
         "sweet post body"
         "Hot diggity dogg"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_POST
  ,.request_path= "/demo"
  ,.request_url= "/demo"
  ,.num_headers= 4
  ,.upgrade="Hot diggity dogg"
  ,.headers= { { "Host", "example.com" }
             , { "Connection", "Upgrade" }
             , { "Upgrade", "HTTP/2.0" }
             , { "Content-Length", "15" }
             }
  ,.body= "sweet post body"
  }

, {.name = "connect with body request"
  ,.type= HTTP_REQUEST
  ,.raw= "CONNECT foo.bar.com:443 HTTP/1.0\r\n"
         "User-agent: Mozilla/1.1N\r\n"
         "Proxy-authorization: basic aGVsbG86d29ybGQ=\r\n"
         "Content-Length: 10\r\n"
         "\r\n"
         "blarfcicle"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.method= HTTP_CONNECT
  ,.request_url= "foo.bar.com:443"
  ,.num_headers= 3
  ,.upgrade="blarfcicle"
  ,.headers= { { "User-agent", "Mozilla/1.1N" }
             , { "Proxy-authorization", "basic aGVsbG86d29ybGQ=" }
             , { "Content-Length", "10" }
             }
  ,.body= ""
  }

/* Examples from the Internet draft for LINK/UNLINK methods:
 * https://tools.ietf.org/id/draft-snell-link-method-01.html#rfc.section.5
 */

, {.name = "link request"
  ,.type= HTTP_REQUEST
  ,.raw= "LINK /images/my_dog.jpg HTTP/1.1\r\n"
         "Host: example.com\r\n"
         "Link: <http://example.com/profiles/joe>; rel=\"tag\"\r\n"
         "Link: <http://example.com/profiles/sally>; rel=\"tag\"\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_LINK
  ,.request_path= "/images/my_dog.jpg"
  ,.request_url= "/images/my_dog.jpg"
  ,.query_string= ""
  ,.fragment= ""
  ,.num_headers= 3
  ,.headers= { { "Host", "example.com" }
             , { "Link", "<http://example.com/profiles/joe>; rel=\"tag\"" }
        , { "Link", "<http://example.com/profiles/sally>; rel=\"tag\"" }
             }
  ,.body= ""
  }

, {.name = "link request"
  ,.type= HTTP_REQUEST
  ,.raw= "UNLINK /images/my_dog.jpg HTTP/1.1\r\n"
         "Host: example.com\r\n"
         "Link: <http://example.com/profiles/sally>; rel=\"tag\"\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.method= HTTP_UNLINK
  ,.request_path= "/images/my_dog.jpg"
  ,.request_url= "/images/my_dog.jpg"
  ,.query_string= ""
  ,.fragment= ""
  ,.num_headers= 2
  ,.headers= { { "Host", "example.com" }
        , { "Link", "<http://example.com/profiles/sally>; rel=\"tag\"" }
             }
  ,.body= ""
  }

, {.name= NULL } /* sentinel */
};

/* * R E S P O N S E S * */
const struct message responses[] =
{ {.name= "google 301"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 301 Moved Permanently\r\n"
         "Location: http://www.google.com/\r\n"
         "Content-Type: text/html; charset=UTF-8\r\n"
         "Date: Sun, 26 Apr 2009 11:11:49 GMT\r\n"
         "Expires: Tue, 26 May 2009 11:11:49 GMT\r\n"
         "X-$PrototypeBI-Version: 1.6.0.3\r\n" /* $ char in header field */
         "Cache-Control: public, max-age=2592000\r\n"
         "Server: gws\r\n"
         "Content-Length:  219  \r\n"
         "\r\n"
         "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n"
         "<TITLE>301 Moved</TITLE></HEAD><BODY>\n"
         "<H1>301 Moved</H1>\n"
         "The document has moved\n"
         "<A HREF=\"http://www.google.com/\">here</A>.\r\n"
         "</BODY></HTML>\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 301
  ,.response_status= "Moved Permanently"
  ,.num_headers= 8
  ,.headers=
    { { "Location", "http://www.google.com/" }
    , { "Content-Type", "text/html; charset=UTF-8" }
    , { "Date", "Sun, 26 Apr 2009 11:11:49 GMT" }
    , { "Expires", "Tue, 26 May 2009 11:11:49 GMT" }
    , { "X-$PrototypeBI-Version", "1.6.0.3" }
    , { "Cache-Control", "public, max-age=2592000" }
    , { "Server", "gws" }
    , { "Content-Length", "219  " }
    }
  ,.body= "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n"
          "<TITLE>301 Moved</TITLE></HEAD><BODY>\n"
          "<H1>301 Moved</H1>\n"
          "The document has moved\n"
          "<A HREF=\"http://www.google.com/\">here</A>.\r\n"
          "</BODY></HTML>\r\n"
  }

/* The client should wait for the server's EOF. That is, when content-length
 * is not specified, and "Connection: close", the end of body is specified
 * by the EOF.
 * Compare with APACHEBENCH_GET
 */
, {.name= "no content-length response"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n"
         "Server: Apache\r\n"
         "X-Powered-By: Servlet/2.5 JSP/2.1\r\n"
         "Content-Type: text/xml; charset=utf-8\r\n"
         "Connection: close\r\n"
         "\r\n"
         "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
         "  <SOAP-ENV:Body>\n"
         "    <SOAP-ENV:Fault>\n"
         "       <faultcode>SOAP-ENV:Client</faultcode>\n"
         "       <faultstring>Client Error</faultstring>\n"
         "    </SOAP-ENV:Fault>\n"
         "  </SOAP-ENV:Body>\n"
         "</SOAP-ENV:Envelope>"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 5
  ,.headers=
    { { "Date", "Tue, 04 Aug 2009 07:59:32 GMT" }
    , { "Server", "Apache" }
    , { "X-Powered-By", "Servlet/2.5 JSP/2.1" }
    , { "Content-Type", "text/xml; charset=utf-8" }
    , { "Connection", "close" }
    }
  ,.body= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
          "  <SOAP-ENV:Body>\n"
          "    <SOAP-ENV:Fault>\n"
          "       <faultcode>SOAP-ENV:Client</faultcode>\n"
          "       <faultstring>Client Error</faultstring>\n"
          "    </SOAP-ENV:Fault>\n"
          "  </SOAP-ENV:Body>\n"
          "</SOAP-ENV:Envelope>"
  }

, {.name= "404 no headers no body"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 404 Not Found\r\n\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 404
  ,.response_status= "Not Found"
  ,.num_headers= 0
  ,.headers= {}
  ,.body_size= 0
  ,.body= ""
  }

, {.name= "301 no response phrase"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 301\r\n\r\n"
  ,.should_keep_alive = false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 301
  ,.response_status= ""
  ,.num_headers= 0
  ,.headers= {}
  ,.body= ""
  }

, {.name="200 trailing space on chunked body"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/plain\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "25  \r\n"
         "This is the data in the first chunk\r\n"
         "\r\n"
         "1C\r\n"
         "and this is the second one\r\n"
         "\r\n"
         "0  \r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 2
  ,.headers=
    { {"Content-Type", "text/plain" }
    , {"Transfer-Encoding", "chunked" }
    }
  ,.body_size = 37+28
  ,.body =
         "This is the data in the first chunk\r\n"
         "and this is the second one\r\n"
  ,.num_chunks_complete= 3
  ,.chunk_lengths= { 0x25, 0x1c }
  }

, {.name="no carriage ret"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\n"
         "Content-Type: text/html; charset=utf-8\n"
         "Connection: close\n"
         "\n"
         "these headers are from http://news.ycombinator.com/"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 2
  ,.headers=
    { {"Content-Type", "text/html; charset=utf-8" }
    , {"Connection", "close" }
    }
  ,.body= "these headers are from http://news.ycombinator.com/"
  }

, {.name="proxy connection"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/html; charset=UTF-8\r\n"
         "Content-Length: 11\r\n"
         "Proxy-Connection: close\r\n"
         "Date: Thu, 31 Dec 2009 20:55:48 +0000\r\n"
         "\r\n"
         "hello world"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 4
  ,.headers=
    { {"Content-Type", "text/html; charset=UTF-8" }
    , {"Content-Length", "11" }
    , {"Proxy-Connection", "close" }
    , {"Date", "Thu, 31 Dec 2009 20:55:48 +0000"}
    }
  ,.body= "hello world"
  }

  /* shown by curl -o /dev/null -v "http://ad.doubleclick.net/pfadx/DARTSHELLCONFIGXML;dcmt=text/xml;" */
, {.name="underscore header key"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Server: DCLK-AdSvr\r\n"
         "Content-Type: text/xml\r\n"
         "Content-Length: 0\r\n"
         "DCLK_imp: v7;x;114750856;0-0;0;17820020;0/0;21603567/21621457/1;;~okv=;dcmt=text/xml;;~cs=o\r\n\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 4
  ,.headers=
    { {"Server", "DCLK-AdSvr" }
    , {"Content-Type", "text/xml" }
    , {"Content-Length", "0" }
    , {"DCLK_imp", "v7;x;114750856;0-0;0;17820020;0/0;21603567/21621457/1;;~okv=;dcmt=text/xml;;~cs=o" }
    }
  ,.body= ""
  }

/* The client should not merge two headers fields when the first one doesn't
 * have a value.
 */
, {.name= "bonjourmadame.fr"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.0 301 Moved Permanently\r\n"
         "Date: Thu, 03 Jun 2010 09:56:32 GMT\r\n"
         "Server: Apache/2.2.3 (Red Hat)\r\n"
         "Cache-Control: public\r\n"
         "Pragma: \r\n"
         "Location: http://www.bonjourmadame.fr/\r\n"
         "Vary: Accept-Encoding\r\n"
         "Content-Length: 0\r\n"
         "Content-Type: text/html; charset=UTF-8\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.status_code= 301
  ,.response_status= "Moved Permanently"
  ,.num_headers= 9
  ,.headers=
    { { "Date", "Thu, 03 Jun 2010 09:56:32 GMT" }
    , { "Server", "Apache/2.2.3 (Red Hat)" }
    , { "Cache-Control", "public" }
    , { "Pragma", "" }
    , { "Location", "http://www.bonjourmadame.fr/" }
    , { "Vary",  "Accept-Encoding" }
    , { "Content-Length", "0" }
    , { "Content-Type", "text/html; charset=UTF-8" }
    , { "Connection", "keep-alive" }
    }
  ,.body= ""
  }

/* Should handle spaces in header fields */
, {.name= "field underscore"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Date: Tue, 28 Sep 2010 01:14:13 GMT\r\n"
         "Server: Apache\r\n"
         "Cache-Control: no-cache, must-revalidate\r\n"
         "Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n"
         ".et-Cookie: PlaxoCS=1274804622353690521; path=/; domain=.plaxo.com\r\n"
         "Vary: Accept-Encoding\r\n"
         "_eep-Alive: timeout=45\r\n" /* semantic value ignored */
         "_onnection: Keep-Alive\r\n" /* semantic value ignored */
         "Transfer-Encoding: chunked\r\n"
         "Content-Type: text/html\r\n"
         "Connection: close\r\n"
         "\r\n"
         "0\r\n\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 11
  ,.headers=
    { { "Date", "Tue, 28 Sep 2010 01:14:13 GMT" }
    , { "Server", "Apache" }
    , { "Cache-Control", "no-cache, must-revalidate" }
    , { "Expires", "Mon, 26 Jul 1997 05:00:00 GMT" }
    , { ".et-Cookie", "PlaxoCS=1274804622353690521; path=/; domain=.plaxo.com" }
    , { "Vary", "Accept-Encoding" }
    , { "_eep-Alive", "timeout=45" }
    , { "_onnection", "Keep-Alive" }
    , { "Transfer-Encoding", "chunked" }
    , { "Content-Type", "text/html" }
    , { "Connection", "close" }
    }
  ,.body= ""
  ,.num_chunks_complete= 1
  ,.chunk_lengths= {}
  }

/* Should handle non-ASCII in status line */
, {.name= "non-ASCII in status line"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 500 Oriëntatieprobleem\r\n"
         "Date: Fri, 5 Nov 2010 23:07:12 GMT+2\r\n"
         "Content-Length: 0\r\n"
         "Connection: close\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 500
  ,.response_status= "Oriëntatieprobleem"
  ,.num_headers= 3
  ,.headers=
    { { "Date", "Fri, 5 Nov 2010 23:07:12 GMT+2" }
    , { "Content-Length", "0" }
    , { "Connection", "close" }
    }
  ,.body= ""
  }

/* Should handle HTTP/0.9 */
, {.name= "http version 0.9"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/0.9 200 OK\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 0
  ,.http_minor= 9
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 0
  ,.headers=
    {}
  ,.body= ""
  }

/* The client should wait for the server's EOF. That is, when neither
 * content-length nor transfer-encoding is specified, the end of body
 * is specified by the EOF.
 */
, {.name= "neither content-length nor transfer-encoding response"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n"
         "hello world"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 1
  ,.headers=
    { { "Content-Type", "text/plain" }
    }
  ,.body= "hello world"
  }

, {.name= "HTTP/1.0 with keep-alive and EOF-terminated 200 status"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.0 200 OK\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 0
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 1
  ,.headers=
    { { "Connection", "keep-alive" }
    }
  ,.body_size= 0
  ,.body= ""
  }

, {.name= "HTTP/1.0 with keep-alive and a 204 status"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.0 204 No content\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 0
  ,.status_code= 204
  ,.response_status= "No content"
  ,.num_headers= 1
  ,.headers=
    { { "Connection", "keep-alive" }
    }
  ,.body_size= 0
  ,.body= ""
  }

, {.name= "HTTP/1.1 with an EOF-terminated 200 status"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 0
  ,.headers={}
  ,.body_size= 0
  ,.body= ""
  }

, {.name= "HTTP/1.1 with a 204 status"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 204 No content\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 204
  ,.response_status= "No content"
  ,.num_headers= 0
  ,.headers={}
  ,.body_size= 0
  ,.body= ""
  }

, {.name= "HTTP/1.1 with a 204 status and keep-alive disabled"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 204 No content\r\n"
         "Connection: close\r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 204
  ,.response_status= "No content"
  ,.num_headers= 1
  ,.headers=
    { { "Connection", "close" }
    }
  ,.body_size= 0
  ,.body= ""
  }

, {.name= "HTTP/1.1 with chunked endocing and a 200 response"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "0\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 1
  ,.headers=
    { { "Transfer-Encoding", "chunked" }
    }
  ,.body_size= 0
  ,.body= ""
  ,.num_chunks_complete= 1
  }

/* Should handle spaces in header fields */
, {.name= "field space"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Server: Microsoft-IIS/6.0\r\n"
         "X-Powered-By: ASP.NET\r\n"
         "en-US Content-Type: text/xml\r\n" /* this is the problem */
         "Content-Type: text/xml\r\n"
         "Content-Length: 16\r\n"
         "Date: Fri, 23 Jul 2010 18:45:38 GMT\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
         "<xml>hello</xml>" /* fake body */
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 7
  ,.headers=
    { { "Server",  "Microsoft-IIS/6.0" }
    , { "X-Powered-By", "ASP.NET" }
    , { "en-US Content-Type", "text/xml" }
    , { "Content-Type", "text/xml" }
    , { "Content-Length", "16" }
    , { "Date", "Fri, 23 Jul 2010 18:45:38 GMT" }
    , { "Connection", "keep-alive" }
    }
  ,.body= "<xml>hello</xml>"
  }

, {.name= "amazon.com"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 301 MovedPermanently\r\n"
         "Date: Wed, 15 May 2013 17:06:33 GMT\r\n"
         "Server: Server\r\n"
         "x-amz-id-1: 0GPHKXSJQ826RK7GZEB2\r\n"
         "p3p: policyref=\"http://www.amazon.com/w3c/p3p.xml\",CP=\"CAO DSP LAW CUR ADM IVAo IVDo CONo OTPo OUR DELi PUBi OTRi BUS PHY ONL UNI PUR FIN COM NAV INT DEM CNT STA HEA PRE LOC GOV OTC \"\r\n"
         "x-amz-id-2: STN69VZxIFSz9YJLbz1GDbxpbjG6Qjmmq5E3DxRhOUw+Et0p4hr7c/Q8qNcx4oAD\r\n"
         "Location: http://www.amazon.com/Dan-Brown/e/B000AP9DSU/ref=s9_pop_gw_al1?_encoding=UTF8&refinementId=618073011&pf_rd_m=ATVPDKIKX0DER&pf_rd_s=center-2&pf_rd_r=0SHYY5BZXN3KR20BNFAY&pf_rd_t=101&pf_rd_p=1263340922&pf_rd_i=507846\r\n"
         "Vary: Accept-Encoding,User-Agent\r\n"
         "Content-Type: text/html; charset=ISO-8859-1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "1\r\n"
         "\n\r\n"
         "0\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 301
  ,.response_status= "MovedPermanently"
  ,.num_headers= 9
  ,.headers= { { "Date", "Wed, 15 May 2013 17:06:33 GMT" }
             , { "Server", "Server" }
             , { "x-amz-id-1", "0GPHKXSJQ826RK7GZEB2" }
             , { "p3p", "policyref=\"http://www.amazon.com/w3c/p3p.xml\",CP=\"CAO DSP LAW CUR ADM IVAo IVDo CONo OTPo OUR DELi PUBi OTRi BUS PHY ONL UNI PUR FIN COM NAV INT DEM CNT STA HEA PRE LOC GOV OTC \"" }
             , { "x-amz-id-2", "STN69VZxIFSz9YJLbz1GDbxpbjG6Qjmmq5E3DxRhOUw+Et0p4hr7c/Q8qNcx4oAD" }
             , { "Location", "http://www.amazon.com/Dan-Brown/e/B000AP9DSU/ref=s9_pop_gw_al1?_encoding=UTF8&refinementId=618073011&pf_rd_m=ATVPDKIKX0DER&pf_rd_s=center-2&pf_rd_r=0SHYY5BZXN3KR20BNFAY&pf_rd_t=101&pf_rd_p=1263340922&pf_rd_i=507846" }
             , { "Vary", "Accept-Encoding,User-Agent" }
             , { "Content-Type", "text/html; charset=ISO-8859-1" }
             , { "Transfer-Encoding", "chunked" }
             }
  ,.body= "\n"
  ,.num_chunks_complete= 2
  ,.chunk_lengths= { 1 }
  }

, {.name= "empty reason phrase after space"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 \r\n"
         "\r\n"
  ,.should_keep_alive= false
  ,.message_complete_on_eof= true
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= ""
  ,.num_headers= 0
  ,.headers= {}
  ,.body= ""
  }

, {.name= "Content-Length-X"
  ,.type= HTTP_RESPONSE
  ,.raw= "HTTP/1.1 200 OK\r\n"
         "Content-Length-X: 0\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "2\r\n"
         "OK\r\n"
         "0\r\n"
         "\r\n"
  ,.should_keep_alive= true
  ,.message_complete_on_eof= false
  ,.http_major= 1
  ,.http_minor= 1
  ,.status_code= 200
  ,.response_status= "OK"
  ,.num_headers= 2
  ,.headers= { { "Content-Length-X", "0" }
             , { "Transfer-Encoding", "chunked" }
             }
  ,.body= "OK"
  ,.num_chunks_complete= 2
  ,.chunk_lengths= { 2 }
  }

, {.name= NULL } /* sentinel */
};

char large_chunked_message[sizeof("HTTP/1.0 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n")+(5+1024+2)*31337+5];

struct message large_chunked =
   {.name= "large chunked"
   ,.type= HTTP_RESPONSE
   ,.raw= large_chunked_message
   ,.should_keep_alive= false
   ,.message_complete_on_eof= false
   ,.http_major= 1
   ,.http_minor= 0
   ,.status_code= 200
   ,.response_status= "OK"
   ,.num_headers= 2
   ,.headers=
   { { "Transfer-Encoding", "chunked" }
   , { "Content-Type", "text/plain" }
   }
   ,.body_size= 31337*1024
   ,.num_chunks_complete= 31338
};

const struct url_test url_tests[] =
{ {.name="proxy request"
  ,.url="http://hostname/"
  ,.is_connect=0
  ,.u=
    {.field_set=(1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  7,  8 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 15,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="proxy request with port"
  ,.url="http://hostname:444/"
  ,.is_connect=0
  ,.u=
    {.field_set=(1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PORT) | (1 << UF_PATH)
    ,.port=444
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  7,  8 } /* UF_HOST */
      ,{ 16,  3 } /* UF_PORT */
      ,{ 19,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="CONNECT request"
  ,.url="hostname:443"
  ,.is_connect=1
  ,.u=
    {.field_set=(1 << UF_HOST) | (1 << UF_PORT)
    ,.port=443
    ,.field_data=
      {{  0,  0 } /* UF_SCHEMA */
      ,{  0,  8 } /* UF_HOST */
      ,{  9,  3 } /* UF_PORT */
      ,{  0,  0 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="CONNECT request but not connect"
  ,.url="hostname:443"
  ,.is_connect=0
  ,.rv=1
  }

, {.name="proxy ipv6 request"
  ,.url="http://[1:2::3:4]/"
  ,.is_connect=0
  ,.u=
    {.field_set=(1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  8,  8 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 17,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="proxy ipv6 request with port"
  ,.url="http://[1:2::3:4]:67/"
  ,.is_connect=0
  ,.u=
    {.field_set=(1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PORT) | (1 << UF_PATH)
    ,.port=67
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  8,  8 } /* UF_HOST */
      ,{ 18,  2 } /* UF_PORT */
      ,{ 20,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="CONNECT ipv6 address"
  ,.url="[1:2::3:4]:443"
  ,.is_connect=1
  ,.u=
    {.field_set=(1 << UF_HOST) | (1 << UF_PORT)
    ,.port=443
    ,.field_data=
      {{  0,  0 } /* UF_SCHEMA */
      ,{  1,  8 } /* UF_HOST */
      ,{ 11,  3 } /* UF_PORT */
      ,{  0,  0 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="ipv4 in ipv6 address"
  ,.url="http://[2001:0000:0000:0000:0000:0000:1.9.1.1]/"
  ,.is_connect=0
  ,.u=
    {.field_set=(1 << UF_SCHEMA) | (1 << UF_HOST) | (1 << UF_PATH)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  8, 37 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 46,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="extra ? in query string"
  ,.url="http://a.tbcdn.cn/p/fp/2010c/??fp-header-min.css,fp-base-min.css,"
  "fp-channel-min.css,fp-product-min.css,fp-mall-min.css,fp-category-min.css,"
  "fp-sub-min.css,fp-gdp4p-min.css,fp-css3-min.css,fp-misc-min.css?t=20101022.css"
  ,.is_connect=0
  ,.u=
    {.field_set=(1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH) | (1<<UF_QUERY)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  7, 10 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 17, 12 } /* UF_PATH */
      ,{ 30,187 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="space URL encoded"
  ,.url="/toto.html?toto=a%20b"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_PATH) | (1<<UF_QUERY)
    ,.port=0
    ,.field_data=
      {{  0,  0 } /* UF_SCHEMA */
      ,{  0,  0 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{  0, 10 } /* UF_PATH */
      ,{ 11, 10 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }


, {.name="URL fragment"
  ,.url="/toto.html#titi"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_PATH) | (1<<UF_FRAGMENT)
    ,.port=0
    ,.field_data=
      {{  0,  0 } /* UF_SCHEMA */
      ,{  0,  0 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{  0, 10 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{ 11,  4 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="complex URL fragment"
  ,.url="http://www.webmasterworld.com/r.cgi?f=21&d=8405&url="
    "http://www.example.com/index.html?foo=bar&hello=world#midpage"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH) | (1<<UF_QUERY) |\
      (1<<UF_FRAGMENT)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  7, 22 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 29,  6 } /* UF_PATH */
      ,{ 36, 69 } /* UF_QUERY */
      ,{106,  7 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="complex URL from node js url parser doc"
  ,.url="http://host.com:8080/p/a/t/h?query=string#hash"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PORT) | (1<<UF_PATH) |\
      (1<<UF_QUERY) | (1<<UF_FRAGMENT)
    ,.port=8080
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  7,  8 } /* UF_HOST */
      ,{ 16,  4 } /* UF_PORT */
      ,{ 20,  8 } /* UF_PATH */
      ,{ 29, 12 } /* UF_QUERY */
      ,{ 42,  4 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="complex URL with basic auth from node js url parser doc"
  ,.url="http://a:b@host.com:8080/p/a/t/h?query=string#hash"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PORT) | (1<<UF_PATH) |\
      (1<<UF_QUERY) | (1<<UF_FRAGMENT) | (1<<UF_USERINFO)
    ,.port=8080
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{ 11,  8 } /* UF_HOST */
      ,{ 20,  4 } /* UF_PORT */
      ,{ 24,  8 } /* UF_PATH */
      ,{ 33, 12 } /* UF_QUERY */
      ,{ 46,  4 } /* UF_FRAGMENT */
      ,{  7,  3 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="double @"
  ,.url="http://a:b@@hostname:443/"
  ,.is_connect=0
  ,.rv=1
  }

, {.name="proxy empty host"
  ,.url="http://:443/"
  ,.is_connect=0
  ,.rv=1
  }

, {.name="proxy empty port"
  ,.url="http://hostname:/"
  ,.is_connect=0
  ,.rv=1
  }

, {.name="CONNECT with basic auth"
  ,.url="a:b@hostname:443"
  ,.is_connect=1
  ,.rv=1
  }

, {.name="CONNECT empty host"
  ,.url=":443"
  ,.is_connect=1
  ,.rv=1
  }

, {.name="CONNECT empty port"
  ,.url="hostname:"
  ,.is_connect=1
  ,.rv=1
  }

, {.name="CONNECT with extra bits"
  ,.url="hostname:443/"
  ,.is_connect=1
  ,.rv=1
  }

, {.name="space in URL"
  ,.url="/foo bar/"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy basic auth with space url encoded"
  ,.url="http://a%20:b@host.com/"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH) | (1<<UF_USERINFO)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{ 14,  8 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 22,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  7,  6 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="carriage return in URL"
  ,.url="/foo\rbar/"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy double : in URL"
  ,.url="http://hostname::443/"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy basic auth with double :"
  ,.url="http://a::b@host.com/"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH) | (1<<UF_USERINFO)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{ 12,  8 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 20,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  7,  4 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="line feed in URL"
  ,.url="/foo\nbar/"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy empty basic auth"
  ,.url="http://@hostname/fo"
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  8,  8 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 16,  3 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }
, {.name="proxy line feed in hostname"
  ,.url="http://host\name/fo"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy % in hostname"
  ,.url="http://host%name/fo"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy ; in hostname"
  ,.url="http://host;ame/fo"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy basic auth with unreservedchars"
  ,.url="http://a!;-_!=+$@host.com/"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH) | (1<<UF_USERINFO)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{ 17,  8 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 25,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  7,  9 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="proxy only empty basic auth"
  ,.url="http://@/fo"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy only basic auth"
  ,.url="http://toto@/fo"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy emtpy hostname"
  ,.url="http:///fo"
  ,.rv=1 /* s_dead */
  }

, {.name="proxy = in URL"
  ,.url="http://host=ame/fo"
  ,.rv=1 /* s_dead */
  }

, {.name="ipv6 address with Zone ID"
  ,.url="http://[fe80::a%25eth0]/"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  8, 14 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 23,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="ipv6 address with Zone ID, but '%' is not percent-encoded"
  ,.url="http://[fe80::a%eth0]/"
  ,.is_connect=0
  ,.u=
    {.field_set= (1<<UF_SCHEMA) | (1<<UF_HOST) | (1<<UF_PATH)
    ,.port=0
    ,.field_data=
      {{  0,  4 } /* UF_SCHEMA */
      ,{  8, 12 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{ 21,  1 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="ipv6 address ending with '%'"
  ,.url="http://[fe80::a%]/"
  ,.rv=1 /* s_dead */
  }

, {.name="ipv6 address with Zone ID including bad character"
  ,.url="http://[fe80::a%$HOME]/"
  ,.rv=1 /* s_dead */
  }

, {.name="just ipv6 Zone ID"
  ,.url="http://[%eth0]/"
  ,.rv=1 /* s_dead */
  }

, {.name="tab in URL"
  ,.url="/foo\tbar/"
  ,.u=
    {.field_set=(1 << UF_PATH)
    ,.field_data=
      {{  0,  0 } /* UF_SCHEMA */
      ,{  0,  0 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{  0,  9 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }

, {.name="form feed in URL"
  ,.url="/foo\fbar/"
  ,.u=
    {.field_set=(1 << UF_PATH)
    ,.field_data=
      {{  0,  0 } /* UF_SCHEMA */
      ,{  0,  0 } /* UF_HOST */
      ,{  0,  0 } /* UF_PORT */
      ,{  0,  9 } /* UF_PATH */
      ,{  0,  0 } /* UF_QUERY */
      ,{  0,  0 } /* UF_FRAGMENT */
      ,{  0,  0 } /* UF_USERINFO */
      }
    }
  ,.rv=0
  }
};

unsigned int getNumUrlTests(void) { return U_NUM_ELEMENTS(url_tests); }
