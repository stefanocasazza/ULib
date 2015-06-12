## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies ## dist-shar dist-zip
MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = ulib

TO_INC = ulib/README \
			ulib/*.h \
			ulib/base/*.h \
			ulib/base/zip/*.h \
			ulib/base/ssl/*.h \
			ulib/base/miniz/*.h \
			ulib/base/coder/*.h \
			ulib/base/win32/*.h \
			ulib/base/replace/*.h \
			ulib/container/*.h \
			ulib/curl/*.h \
			ulib/db/*.h \
			ulib/debug/*.h \
			ulib/dynamic/*.h \
			ulib/event/*.h \
			ulib/flex/* \
			ulib/internal/*.h \
			ulib/ldap/*.h \
			ulib/magic/*.h \
			ulib/mime/*.h \
			ulib/dbi/*.h \
			ulib/orm/*.h \
			ulib/orm/driver/*.h \
			ulib/libevent/*.h \
			ulib/net/*.h \
			ulib/examples/*.h \
			ulib/net/rpc/*.h \
			ulib/net/client/*.h \
			ulib/net/server/*.h \
			ulib/net/server/plugin/*.h \
			ulib/pcre/*.h \
			ulib/json/*.h \
			ulib/query/*.h \
			ulib/replace/*.h \
			ulib/ssh/net/*.h \
			ulib/ssl/*.h \
			ulib/ssl/mime/*.h \
			ulib/ssl/net/*.h \
			ulib/ui/*.h \
			ulib/utility/*.h \
			ulib/xml/soap/*.h \
			ulib/xml/expat/*.h \
			ulib/xml/libxml2/*.h \
			ulib/zip/*.h

#if !GETOPT_LONG
#TO_INC += ulib/replace/getopt.h
#endif

#if DEBUG
#TO_INC += ulib/debug/*.h
#endif

#if MEMORY_POOL
#TO_INC += ulib/internal/memory_pool.h
#endif

#if USE_PARSER
#TO_INC += ulib/flex/*
#endif

#if PCRE
#TO_INC += ulib/pcre/*.h
#endif

#if SSL
#TO_INC += ulib/base/ssl/*.h ulib/ssl/*.h ulib/ssl/mime/*.h ulib/ssl/net/*.h
#endif

#if LDAP
#TO_INC += ulib/ldap/*.h
#endif

#if CURL
#TO_INC += ulib/curl/*.h
#endif

#if SIGSLOT
#TO_INC += ulib/sigslot/*.h ulib/sigslot/my/*.h
#endif

#if ZIP
#TO_INC += ulib/base/zip/*.h ulib/zip/*.h
#endif

#if MAGIC
#TO_INC += ulib/magic/*.h
#endif

#if MYSQL
#TO_INC += ulib/mysql/*.h
#endif

#if MINGW
#TO_INC += ulib/base/win32/*
#endif

nobase_include_HEADERS = $(TO_INC)
