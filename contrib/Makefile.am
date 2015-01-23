## Makefile.am for examples

AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = HCSP/Makefile.in HCSP/Makefile.am HCSP/HCSP.c \
				 HCSP/CSP.dbg HCSP/CSP.fcfg HCSP/CSP.func HCSP/CSP.h HCSP/HCSP.err HCSP/WinCrypt.h HCSP/doc/* \
				 signer/Makefile.in signer/Makefile.am signer/*.h signer/*.cpp \
				 RSIGN/Makefile.in RSIGN/Makefile.am RSIGN/RSIGN.c RSIGN/RSIGN.err

MAINTAINERCLEANFILES = Makefile.in

## SOURCE_LINK = http_parser
## TARGET_LINK = http_parser_request http_parser_response

## $(TARGET_LINK):
## 	NFILE=1; \
## 	for tfile in $(TARGET_LINK); \
##  	do \
##    	rm -f $$tfile; \
##    	@LN_S@ `echo $(SOURCE_LINK) | cut -d ' ' -f $$NFILE` $$tfile; \
##    	NFILE=`expr $$NFILE + 1`; \
##    	export NFILE; \
##    done

SUBDIRS =

if MINGW
SUBDIRS += signer
if SSL
SUBDIRS += HCSP RSIGN
endif
endif

dist-hook:
	find $(distdir) -type f -name \*.h   -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.c   -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cpp -exec /utility/expand_tab '{}' ';'

clean-local:
	-rm -rf core
