# ULib - C++ library

Travis CI: [![Build Status](https://travis-ci.org/stefanocasazza/ULib.svg?branch=master)](https://travis-ci.org/stefanocasazza/ULib)
Coverity Scan: [![Coverity Scan Build Status](https://scan.coverity.com/projects/3322/badge.svg)](https://scan.coverity.com/projects/3322)

ULib is a highly optimized class framework for writing C++ applications. I wrote this framework as my tool for writing applications in various contexts. It is a result of many years of work as C++ programmer. I think, in my opinion, that its strongest points are simplicity, efficiency and sophisticate debugging.

ULib is meant as a very light-weight C++ library to facilitate using C++ design patterns even for very deeply embedded applications, such as for systems using uclibc along with posix threading support. For this reason, ULib disables language features that consume memory or introduce runtime overhead, such as rtti and exception handling, and assumes one will mostly be linking applications with other pure C based libraries rather than using the overhead of the standard C++ library and other similar class frameworks.

It include as application example a powerful search engine with relative [web interface](https://github.com/stefanocasazza/ULib/blob/master/examples/IR/ir_web.usp) and a multi purpose server (plugin oriented) which results, out of [John Fremlin accurate investigations](http://john.freml.in/ulib-fast-io-framework), to be one of the fastest web application frameworks for serving small dynamic webpages (and also make easier the usage of shell scripts for CGI application)

# userver (`_tcp` | `_ssl` | `_ipc`) application server (`plugin oriented`)

The current version offers the following features :

   * HTTP/1.0 and 1.1 protocols supported and initial implementations of HTTP/2.
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
   * Support for RewriteRule (lighttpd-like) that check for file existence as they do on Apache, some CMS (SilverStripe) require it.
   * Support for (apache-like) log [NCSA extended/combined format](http://httpd.apache.org/docs/2.0/mod/mod_log_config.html)
   * Support for [JSONRequest](http://json.org/JSONRequest.html).
   * Accept HTTP uploads up to 4 GB without increasing memory usage.
   * General [CGI](http://it.wikipedia.org/wiki/Common_Gateway_Interface) support (run any CGI script) with automatic output compression (using gzip method).
   * CGI support for shell script processes (with automatic management of form and cookie).
   * CGI support for the X-Sendfile feature and also supports X-Accel-Redirect headers transparently.
   * Support for minify HTML CGI output by wrapping [google page speed SDK](http://code.google.com/speed/page-speed/download.html#pagespeed-sdk).
   * Support for running JavaScript code by wrapping [google V8 JavaScript Engine](http://code.google.com/apis/v8/intro.html).
   * [HTTP pseudo-streaming](http://en.wikipedia.org/wiki/Progressive_download) for FLV video managed transparently.
   * [C Servlet Support](http://bellard.org/tcc/) with libtcc (if available) as a backend for dynamic code generation (experimental).
   * Support for running Ruby on Rails applications natively (experimental).
   * Support for running natively PHP applications whith a php (embedded) library (experimental).
   * Preforking mode to improve concurrency with dedicated process for long-time request.
   * Support for Windows (without preforking).
   * Customizable builds (you can remove unneeded functionality).
   * Requests cut in phases for modular architecture (apache-like).
   * Configuration file with dedicated section.
   * Built-in modules :
       * `mod_echo` : echo features.
       * `mod_rpc` : generic Remote Procedure Call.
       * `mod_http` : core features, static file handler and dynamic page (ULib Servlet Page).
       * `mod_ssi` : [Server Side Includes]( http://en.wikipedia.org/wiki/Server_Side_Include) support with enhanced #set, direct include and #exec servlet (C/ULib Servlet Page).
       * `mod_nocat` : [captive portal](http://dev.wifidog.org/wiki/NoCat) implementation.
       * `mod_proxy` : multi-features reverse proxy with websocket support.
       * `mod_tsa` : server side [Time Stamp](http://sourceforge.net/projects/timestamping/) support.
       * `mod_soap` : generic [SOAP](http://en.wikipedia.org/wiki/SOAP) server services support.
       * `mod_fcgi` : third-party applications support thru [FastCGI](http://www.fastcgi.com/drupal) interface.
       * `mod_scgi` : module that implements the client side of the [SCGI](http://en.wikipedia.org/wiki/Simple_Common_Gateway_Interface) protocol (experimental).
       * `mod_shib` : [web single sign-on support](http://shibboleth.internet2.edu) (experimental).
       * `mod_geoip` : [geolocation support](http://en.wikipedia.org/wiki/Geolocation) (experimental).
       * `mod_stream` : simple streaming support (experimental).
       * `mod_socket` : [Web Socket](http://dev.w3.org/html5/websockets) application framework (experimental).
   * Security protection :
       * [HTTP Session Hijacking](http://en.wikipedia.org/wiki/Session_hijacking) mitigation.
       * [Algorithmic Complexity Attacks](http://lwn.net/Articles/474365/) prevention by randomizing hash seed.
       * [DNS rebinding](http://en.wikipedia.org/wiki/DNS_rebinding) prevention by RFC1918 filtering and Host header validation.
       * selective uri support (DOS regex) for [HTTP Strict Transport Security](https://developer.mozilla.org/en/Security/HTTP_Strict_Transport_Security).
   * Immune to [Slow Read DoS attack](http://code.google.com/p/slowhttptest/)
   * [High SSL server quality score](https://www.ssllabs.com/ssltest/analyze.html?d=wifi-aaa.comune.fi.it)

It is the main software component of [city of Florence wireless network](http://wifi-aaa.comune.fi.it/login?mac=00%3A00%3A00%3A00%3A00%3A00&ip=172.22.11.124&redirect=http%3A//pasta.dianxinos.com/api/data&gateway=159.213.248.230%3A5280&timeout=0&token=1810300063&ap=05@159.213.248.230)

userver application server is in the [10th round of TechEmpower's web framework benchmarks](http://www.techempower.com/benchmarks). This independent work tests a large number of frameworks and platforms against a set of tests common to web applications, such as JSON serialization, database queries and templating.

ULib is normally built and installed as a set of shared object libraries and header files. These libraries and headers are installed using directories selected through a "configure" script that has been prepared with automake and autoconf. As such, they should build and install similarly to and in a manner compatible and consistent with most other GNU software. ULib is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. ULib is Free Software under the LGPL

Comments and suggestions are welcome.

	stefano casazza <stefano.casazza@gmail.com>
