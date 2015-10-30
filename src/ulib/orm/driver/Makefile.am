## Makefile.am for orm driver

EXTRA_DIST = *.cpp Makefile.*

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES =

if HAVE_SQLITE3
if !STATIC_ORM_DRIVER_SQLITE
module_LTLIBRARIES  += orm_driver_sqlite.la
orm_driver_sqlite_la_SOURCES  = orm_driver_sqlite.cpp
orm_driver_sqlite_la_CXXFLAGS = -I@SQLITE3_INCLUDE@
orm_driver_sqlite_la_LIBADD   = $(ulib_la)
orm_driver_sqlite_la_LDFLAGS  = @SQLITE3_LDFLAGS@ -Wl,--start-group @SQLITE3_LIBS@ -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
endif
endif

if HAVE_MYSQL
if !STATIC_ORM_DRIVER_MYSQL
module_LTLIBRARIES  += orm_driver_mysql.la
orm_driver_mysql_la_SOURCES  = orm_driver_mysql.cpp
orm_driver_mysql_la_CXXFLAGS = -I@MYSQL_INCLUDE@
orm_driver_mysql_la_LIBADD   = $(ulib_la)
orm_driver_mysql_la_LDFLAGS  = @MYSQL_LDFLAGS@ -Wl,--start-group @MYSQL_LIBS@ -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
endif
endif

if HAVE_PGSQL
if !STATIC_ORM_DRIVER_PGSQL
module_LTLIBRARIES  += orm_driver_pgsql.la
orm_driver_pgsql_la_SOURCES  = orm_driver_pgsql.cpp
orm_driver_pgsql_la_CXXFLAGS = -I@PGSQL_INCLUDE@
orm_driver_pgsql_la_LIBADD   = $(ulib_la)
orm_driver_pgsql_la_LDFLAGS  = @PGSQL_LDFLAGS@ -Wl,--start-group @PGSQL_LIBS@ -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
