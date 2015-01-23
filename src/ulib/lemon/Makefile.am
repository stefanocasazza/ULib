## Makefile.am for LEMON parser generator

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I.

EXTRA_DIST = expression.* skeleton.c

lemon_SOURCES = lemon.c

noinst_PROGRAMS = lemon

## expression.cpp: expression.y
## 	./lemon expression.y
## 	mv expression.c expression.cpp

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
