## Makefile.am for examples/parser

MAINTAINERCLEANFILES = Makefile.in

AM_YFLAGS = -d -v -Sbison.skl
AM_LFLAGS = -olex.yy.c -S$(shell pwd)/../../include/ulib/flex/flex.skl

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

INC = Parser.h ParserInterface.h
SRC = Lexer.ll Parser.yy ParserInterface.cpp main.cpp

parser_LDADD   = $(ulib_la)
parser_SOURCES = $(INC) $(SRC)
parser_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS = parser

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
