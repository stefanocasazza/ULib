## Makefile.am for page speed plugin for uhttp

MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = -Wno-portability

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I..

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_pagespeed.la

# Path to the root of the page speed build tree.
pagespeed_root_dir = @PAGESPEED_ROOT_DIR@

# The page speed libraries we depend on.
PAGESPEED_DEPS = \
        pagespeed/html/libpagespeed_html.a \
        third_party/jsmin/libjsmin.a \
        pagespeed/cssmin/libpagespeed_cssmin.a \
        third_party/instaweb/src/net/instaweb/libinstaweb_htmlparse_core.a \
        third_party/instaweb/src/net/instaweb/libinstaweb_rewriter_html.a \
        third_party/instaweb/src/net/instaweb/libinstaweb_util_core.a \
        pagespeed/core/libpagespeed_init.a \
        build/temp_gyp/libgoogleurl.a \
        pagespeed/image_compression/libpagespeed_png_optimizer.a \
        pagespeed/image_compression/libpagespeed_jpeg_optimizer.a \
        pagespeed/image_compression/libpagespeed_jpeg_reader.a \
        third_party/libpng/libpng.a \
        third_party/libjpeg_trans/libjpeg_trans.a \
        third_party/libjpeg/libjpeg.a \
        third_party/optipng/libpngxrgif.a \
        third_party/optipng/libopngreduc.a \
        base/libbase.a

#       net/libnet_base.a \
#       pagespeed/image_compression/libpagespeed_image_attributes_factory.a \
#       pagespeed/libpagespeed_library.a \
#       pagespeed/core/libpagespeed_core.a \
#       pagespeed/formatters/libpagespeed_formatters.a \
#       pagespeed/har/libpagespeed_har.a \
#       pagespeed/proto/libpagespeed_output_pb.a \
#       third_party/protobuf2/libprotobuf_lite.a \
#       third_party/modp_b64/libmodp_b64.a \
#       third_party/cJSON/libcJSON.a \
#       third_party/zlib/libchrome_zlib.a

# The following are used to specify the location of the generated libraries within the page speed SDK make build
TPREFIX=@PAGESPEED_ROOT_DIR@/out/Release/obj.target
PAGESPEED_LIBS=$(addprefix $(TPREFIX)/,$(PAGESPEED_DEPS))

# ----------------------------------------------
# NB: compile with CXXFLAGS='-fPIC' make
# ----------------------------------------------
$(PAGESPEED_LIBS):
	CFLAGS='-fPIC -DPIC -Wno-error -Wno-error=unused-but-set-variable' CXXFLAGS='-fPIC -DPIC -Wno-error -Wno-error=unused-but-set-variable' $(MAKE) -C @PAGESPEED_ROOT_DIR@ 

DEFAULT_INCLUDES += -I@PAGESPEED_ROOT_DIR@/third_party/chromium/src \
    					  -I@PAGESPEED_ROOT_DIR@ \
						  -I@PAGESPEED_ROOT_DIR@/third_party/instaweb/src \
						  -I@PAGESPEED_ROOT_DIR@/third_party/protobuf2 \
						  -I@PAGESPEED_ROOT_DIR@/third_party/protobuf2/src/src

DEFS = -DGOOGLE_PROTOBUF_NO_RTTI -DPAGESPEED_PNG_OPTIMIZER_GIF_READER -D__STDC_FORMAT_MACROS -DNDEBUG @DEFS@

server_plugin_pagespeed_la_SOURCES = mod_pagespeed.cpp
server_plugin_pagespeed_la_LDFLAGS = -Wl,--start-group $(PAGESPEED_LIBS) -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_pagespeed_la_LIBADD  = $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
