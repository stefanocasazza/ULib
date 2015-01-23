                               ULib - C++ library

   ULib is a highly optimized class framework for writing C++
   applications. I wrote this framework as my tool for writing
   applications in various contexts. It is a result of many years of work
   as C++ programmer. I think, in my opinion, that its strongest points
   are simplicity, efficiency and sophisticate debugging. This framework
   offers a class foundation that disables language features that consume
   memory or introduce runtime overhead, such as rtti and exception
   handling, and assumes one will mostly be linking applications with
   other pure C based libraries rather than using the overhead of the
   standard C++ library and other similar class frameworks. It include as
   application example a powerful search engine with relative web
   interface and a multi purpose server (plugin oriented) which results,
   out of John Fremlin accurate investigations, to be one of the
   fastest web application frameworks for serving small dynamic webpages
   (and also make easier the usage of shell scripts for CGI application)

Quickstart

   Take a look at:
$ ./configure --help

     * ......
     * --enable-zip enable build of ZIP support - require libz [default: use if present libz]
     * --enable-thread enable build of thread support - require libpthread [default: use if present libpthread]
     * --with-libz use system LIBZ library - [will check /usr /usr/local] [default=use if present]
     * --with-libuuid use system libuuid library - [will check /usr /usr/local] [default=use if present]
     * --with-magic use system libmagic library - [will check /usr /usr/local] [default=use if present]
     * --with-ssl use system SSL library - [will check /usr /usr/local] [default=use if present]
     * --with-pcre use system PCRE library - [will check /usr /usr/local] [default=use if present]
     * --with-expat use system EXPAT library - [will check /usr /usr/local] [default=use if present]
     * --with-ssh use system SSH library - [will check /usr /usr/local]
     * --with-curl use system cURL library - [will check /usr /usr/local]
     * --with-ldap use system openLDAP library - [will check /usr /usr/local]
     * --with-mysql use system MySQL library - [will check /usr /usr/local]
     * --with-dbi use system DBI library - [will check /usr /usr/local]
     * --with-libevent use system libevent library - [will check /usr /usr/local]
     * --with-libxml2 use system libxml2 library - [will check /usr /usr/local]
     * --with-page-speed use google page-speed SDK - [will check /usr /usr/local]
     * --with-v8-javascript use V8 JavaScript Engine - [will check /usr /usr/local]

   if you desire wrapping of some system library installed.

userver (_tcp | _ssl | _ipc) application server (plugin oriented)

   The current version offers the following features :
     * HTTP/1.0 and 1.1 protocols supported.
     * Persistent connections for HTTP/1.1 and Keep-Alive support for HTTP/1.0.
     * Browser cache management (headers: If-Modified-Since/Last-modified).
     * Chunk-encoding transfers support.
     * HTTP multi-range request support.
     * Memory caching of document root for (small) static pages with smart gzip compression and CSS/JS reduction.
     * Support for automatic update of caching document root with inotify (on Linux).
     * Support for pipelining.
     * Support for virtual hosts (also with SSL).
     * Support for basic/digest authentication.
     * Support for directory listings via basic/digest authentication.
     * Support for uri protection.
     * Support for aliases/redirection.
     * Support for switch the site to a maintenance page only.
     * Support for overriden of error messages by local document (ErrorDocument/40x|500.html).
     * Support for RewriteRule (lighttpd-like) that check for file
       existence as they do on Apache, some CMS (SilverStripe) require it.
     * Support for (apache-like) log NCSA extended/combined format
     * Support for JSONRequest.
     * Accept HTTP uploads up to 4 GB without increasing memory usage.
     * Support for upload progress via USP (ULib Servlet Page).
     * General CGI support (run any CGI script) with automatic output compression (using gzip method).
     * CGI support for shell script processes (with automatic management of form and cookie).
     * CGI support for the X-Sendfile feature and also supports X-Accel-Redirect headers transparently.
     * Support for minify HTML CGI output with wrapping google page speed SDK.
     * Support for running JavaScript code with wrapping google V8 JavaScript Engine.
     * HTTP pseudo-streaming for FLV video managed transparently.
     * C Servlet Support with libtcc (if available) as a backend for
       dynamic code generation (experimental).
     * Preforking mode to improve concurrency.
     * Support for Windows (without preforking).
     * Customizable builds (you can remove unneeded functionality).
     * Requests cut in phases for modular architecture (apache-like).
     * Configuration file with dedicated section.
     * Built-in modules :
          + mod_echo : echo features.
          + mod_rpc : generic Remote Procedure Call.
          + mod_http : core features, static file handler and dynamic page (ULib Servlet Page).
          + mod_ssi : Server Side Includes support with enhanced #set, direct include and #exec servlet (C/ULib Servlet Page).
          + mod_nocat : captive portal implementation.
          + mod_proxy : multi-features reverse proxy with websocket support.
          + mod_tsa : server side Time Stamp support.
          + mod_soap : generic SOAP server services support.
          + mod_fcgi : third-party applications support thru FastCGI interface.
          + mod_scgi : module that implements the client side of the SCGI protocol (experimental).
          + mod_shib : web single sign-on support (experimental).
          + mod_geoip : geolocation support (experimental).
          + mod_stream : simple streaming support (experimental).
          + mod_socket : Web Socket application framework (experimental).
     * Security protection :
          + HTTP Session Hijacking mitigation.
          + Algorithmic Complexity Attacks prevention by randomizing hash seed.
          + DNS rebinding prevention by RFC1918 filtering and Host header validation.
          + selective uri support (DOS regex) for HTTP Strict Transport Security.
     * Immune to Slow Read DoS attack
     * High SSL server quality score

Benchmarking

$ ./configure && make
$ cd tests/examples
$ ./benchmarking.sh (or hello_world.sh)

   Use apachebench (ab)
$ ab -n 100000 -c10 http://10.30.1.131/servlet/benchmarking?name=stefano (or)
$ ab -n 100000 -c10 http://10.30.1.131/servlet/hello_world

Comparative Benchmarking

   I consider in this benchmark the server [G-WAN 3.2.24 (64 bit)]
   (http://www.gwan.ch/) and [NGINX 1.1.16] (http://nginx.net/).

gwan run with the follow options:

-b: enable the TCP_DEFER_ACCEPT option
-d: daemon mode (with an 'angel' process)

nginx is configured in this way:

nginx version: nginx/1.1.16
TLS SNI support enabled
configure arguments: --prefix=/usr --sbin-path=/usr/sbin/nginx --conf-path=/etc/nginx/nginx.conf
--error-log-path=/var/log/nginx/error_log --pid-path=/var/run/nginx.pid --lock-path=/var/lock/nginx.lock
--user=nginx --group=nginx --with-cc-opt=-I/usr/include --with-ld-opt=-L/usr/lib --http-log-path=/var/log/nginx/access_log
--http-client-body-temp-path=/var/tmp/nginx/client --http-proxy-temp-path=/var/tmp/nginx/proxy --http-fastcgi-temp-path=/var/tmp/nginx/fastcgi
--http-scgi-temp-path=/var/tmp/nginx/scgi --http-uwsgi-temp-path=/var/tmp/nginx/uwsgi --with-ipv6 --with-pcre --with-http_realip_module
--with-http_ssl_module --without-mail_imap_module --without-mail_pop3_module --without-mail_smtp_module

nginx run with the follow configuration:

user nginx nginx;
worker_processes 2;

events {
     use epoll;
     worker_connections 1024;
     multi_accept on;
}

http {
    include /etc/nginx/mime.types;
    default_type application/octet-stream;

    log_format main
        '$remote_addr - $remote_user [$time_local] '
        '"$request" $status $bytes_sent '
        '"$http_referer" "$http_user_agent" '
        '"$gzip_ratio"';

    client_header_timeout 10m;
    client_body_timeout 10m;
    send_timeout 10m;

    connection_pool_size 256;
    client_header_buffer_size 1k;
    large_client_header_buffers 4 2k;
    request_pool_size 4k;

    gzip off;
    gzip_min_length 1100;
    gzip_buffers 4 8k;
    gzip_types text/plain;

    output_buffers 1 32k;
    postpone_output 1460;

    error_log   off;
    access_log  off;

    open_file_cache max=1000 inactive=20s;
    open_file_cache_valid 30s;
    open_file_cache_min_uses 2;

    server_tokens off;

    tcp_nopush on;
    tcp_nodelay on;

    keepalive_timeout 75 20;

    ignore_invalid_headers on;

    index index.html;

    server {
        listen 8080;
        server_name 10.30.1.131;

        access_log  off;
        error_log   off;

        root /usr/src/ULib-1.1.0/tests/examples/benchmark/docroot;
    }
}

   All tests are performed on an Intel Pentium 4 2.8 Ghz, Hard drive 5400
   rpm, Memory: 2GB DDR2 800MHz running Gentoo 2.0.3 AMD64 (kernel 3.2.7).
   Yes, this CPU is 11-year old (single-core) P4, but some test on more
   recent processor (dual-core AMD) give similar results.

   The client ab.c relies on ApacheBench (ab) and it is a slightly
   modified version of G-WAN client.

   I have considered two scenario for benchmarking:
     * The client as well as the web server tested are hosted on the same computer.
     * The client is running on different computer than the web server (networking is involved).

   I had to increase the local port range on client (because of the TIME_WAIT status of the TCP ports).
     * HTTP Keep-Alives: yes/no
     * Concurrency: from 0 to 1000, step 10
     * Requests: up to 1000000 - within a fixed total amount of time (1 sec)

   For serving static content I use 3 file of different size:
     * 100.html (100 bytes - only 'XXX...' without CR/LF)
     * 1000.html (1000 bytes - only 'XXX...' without CR/LF)
     * WebSocketMain.swf (80K bytes)

   For serving dynamic content I use a simple request: Hello {name}

userver_tcp is the winner of this benchmark for almost all level of concurrency.

More info

   ULib is normally built and installed as a set of shared object
   libraries and header files. These libraries and headers are installed
   using directories selected through a "configure" script that has been
   prepared with automake and autoconf. As such, they should build and
   install similarly to and in a manner compatible and consistent with
   most other GNU software. ULib is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version. This program is
   distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details. You should have received a copy of the GNU General
   Public License along with this program; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   Comments and suggestions are welcome.
stefano casazza <stefano.casazza@gmail.com>
