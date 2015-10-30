## Makefile.am for embed the PHP interpreter in the plugin for uhttp

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I.. @PHPCONFIGINCLUDES@

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_php.la
server_plugin_php_la_SOURCES = mod_php.cpp
server_plugin_php_la_LDFLAGS = $(PHPCONFIGLDFLAGS) -Wl,--start-group $(PHPCONFIGLIBS) -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_php_la_LIBADD  = $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out

## ---------------------------------------------------------------------------------------------------------------------------------------------------
## You can safely run PHP scripts using CGI support. The downside of this approach is the latency caused by the spawn of a new PHP interpreter at each
## request. To get far superior performance you will want to embed the PHP interpreter in the core and use the PHP plugin.
##
## A bunch of distros (such as Fedora, Red Hat and CentOS) include a php-embedded package. Install it, along with php-devel and you should be able to
## build the php plugin. For distros that do not supply a libphp package (all Debian-based distros, for instance), you have to rebuild PHP with the
## --enable-embed flag to ./configure
##
## That's a good starting point (gentoo environment)
## ---------------------------------------------------------------------------------------------------------------------------------------------------
## ./configure \
## --prefix=/usr --build=x86_64-pc-linux-gnu --host=x86_64-pc-linux-gnu --mandir=/usr/share/man --infodir=/usr/share/info \
## --datadir=/usr/share --sysconfdir=/etc --localstatedir=/var/lib --prefix=/usr/lib64/php5.4 --mandir=/usr/lib64/php5.4/man \
## --infodir=/usr/lib64/php5.4/info --libdir=/usr/lib64/php5.4/lib --with-libdir=lib64 --without-pear --disable-maintainer-zts \
## --disable-bcmath --with-bz2=/usr --disable-calendar --enable-ctype --with-curl=/usr --without-curlwrappers --enable-dom \
## --without-enchant --enable-exif --enable-fileinfo --enable-filter --enable-ftp --with-gettext=/usr --without-gmp --enable-hash \
## --without-mhash --with-iconv --disable-intl --enable-ipv6 --enable-json --with-kerberos=/usr --enable-libxml --with-libxml-dir=/usr \
## --enable-mbstring --with-mcrypt=/usr --without-mssql --with-onig=/usr --with-openssl=/usr --with-openssl-dir=/usr --disable-pcntl \
## --enable-phar --enable-pdo --without-pgsql --enable-posix --with-pspell=/usr --without-recode --enable-simplexml --enable-shmop \
## --with-snmp=/usr --disable-soap --disable-sockets --with-sqlite3=/usr --without-sybase-ct --disable-sysvmsg --disable-sysvsem \
## --disable-sysvshm --without-tidy --enable-tokenizer --disable-wddx --enable-xml --enable-xmlreader --enable-xmlwriter --with-xmlrpc \
## --without-xsl --enable-zip --with-zlib=/usr --disable-debug --enable-dba --without-cdb --with-db4=/usr --enable-flatfile --with-gdbm=/usr \
## --disable-inifile --without-qdbm --with-freetype-dir=/usr --with-t1lib=/usr --disable-gd-jis-conv --with-jpeg-dir=/usr --with-png-dir=/usr \
## --with-xpm-dir=/usr --with-gd --with-imap=/usr --with-imap-ssl=/usr --with-ldap=/usr --without-ldap-sasl --with-mysql=/usr \
## --with-mysql-sock=/var/run/mysqld/mysqld.sock --with-mysqli=/usr/bin/mysql_config --without-pdo-dblib --with-pdo-mysql=/usr \
## --without-pdo-pgsql --with-pdo-sqlite=/usr --without-pdo-odbc --with-readline=/usr --without-libedit --with-mm=/usr --with-pic \
## --with-pcre-regex=/usr --with-pcre-dir=/usr --with-config-file-path=/etc/php/embed-php5.4 \
## --with-config-file-scan-dir=/etc/php/embed-php5.4/ext-active --enable-embed --disable-cli --disable-cgi --disable-fpm --without-apxs2
## ---------------------------------------------------------------------------------------------------------------------------------------------------
