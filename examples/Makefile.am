## Makefile.am for examples

AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = README \
				 WiAuth/Makefile.* \
				 XAdES/*.h XAdES/*.cpp XAdES/Makefile.* \
				 IR/*.h IR/*.cpp IR/Makefile.* \
				 userver/*.cpp userver/Makefile.* \
				 simple_client_server/*.cpp simple_client_server/Makefile.* \
				 csp/*.cpp csp/Makefile.* csp/usage.txt \
				 lcsp/*.cpp lcsp/Makefile.* \
				 rsign/*.cpp rsign/Makefile.* \
				 doc_classifier/*.h doc_classifier/*.cpp doc_classifier/Makefile.* \
				 doc_parse/*.cpp doc_parse/Makefile.* \
				 download_accelerator/*.cpp download_accelerator/Makefile.* \
				 form_completion/*.cpp form_completion/Makefile.* \
				 http_header/include http_header/*.cpp http_header/Makefile.* \
				 lrp_session/*.cpp lrp_session/Makefile.* \
				 parser/*.h parser/*.cpp parser/*.ll parser/*.yy parser/Makefile.* \
				 test_manager/*.cpp test_manager/Makefile.* \
				 PEC_log/*.cpp PEC_log/Makefile.* \
				 xml2txt/*.cpp xml2txt/Makefile.* \
				 workflow/*.h workflow/*.cpp workflow/Makefile.*

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = userver test_manager simple_client_server workflow

if STDCPP
SUBDIRS += IR
if !STATIC_ONLY
if !STATIC_SERVLET_WI_AUTH
if  STATIC_HANDLER_SSI
SUBDIRS += WiAuth
endif
endif
endif
endif

## MINGW don't have unix socket...
if !MINGW
SUBDIRS += lcsp
endif

if SSL
SUBDIRS += http_header uclient
if EXPAT
SUBDIRS += csp rsign
endif
if LIBZ
if STDCPP
SUBDIRS += PEC_log
if ZIP
SUBDIRS += doc_parse doc_classifier
if CURL
if LIBXML2
SUBDIRS += XAdES
endif
endif
endif
endif
endif
endif

if EXPAT
SUBDIRS += xml2txt
endif

if LDAP
SUBDIRS += form_completion
#if SSL
#if SSH
#SUBDIRS += lrp_session
#endif
#endif
endif

#if USE_PARSER
#SUBDIRS += parser
#endif

#if LINUX
#SUBDIRS += download_accelerator
#endif

dist-hook:
	find $(distdir) -type f -name \*.h   -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cpp -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.usp -exec /utility/expand_tab '{}' ';'

clean-local:
	-rm -rf core
