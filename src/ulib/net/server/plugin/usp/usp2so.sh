#!/bin/sh

#export WINELOADER=wine

#set -x

OBJ="usp_compile.sh"

for i in *.usp
do
	OBJ="$OBJ $(basename $i .usp).la"
done

UMEMPOOL="0,0,0,48,-20,-20,-20,-20,0" make -j6 $OBJ

rm -f .libs/lib*.so

chmod 777 usp_compile.sh

# echo "  CXX   " upload.lo;/bin/sh ../../../../../../libtool --silent --tag=CXX   --mode=compile g++ -DHAVE_CONFIG_H -I. -I../../../../../../include   -DDEBUG
# -I/usr/include/libxml2 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -pipe -D_GNU_SOURCE  -fstrict-aliasing -fno-stack-protector -fomit-frame-pointer -finline -findirect-inlining
# -ftree-switch-conversion -Wstrict-aliasing=2 -Wall -Wextra -Wsign-compare -Wpointer-arith -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wpacked -Wswitch-enum
# -Wmissing-format-attribute -Winit-self -Wformat -Wformat-extra-args -Wenum-compare -Wno-unused-result -Wshadow -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn -Ofast -flto
# -Wunsafe-loop-optimizations -Wno-unused-parameter  -g -O2  -fno-check-new -fno-exceptions -fno-rtti -Wno-deprecated -Wdelete-non-virtual-dtor -Wc++11-compat -fvisibility=hidden
# -fvisibility-inlines-hidden -MT upload.lo -MD -MP -MF .deps/upload.Tpo -c -o upload.lo upload.cpp
# mv -f .deps/upload.Tpo .deps/upload.Plo

# /bin/sh ../../../../../../libtool --silent --tag=CXX   --mode=link g++  -g -O2  -fno-check-new -fno-exceptions -fno-rtti -Wno-deprecated -Wdelete-non-virtual-dtor -Wc++11-compat
# -fvisibility=hidden -fvisibility-inlines-hidden  -Wl,-O1 -Wl,--as-needed -Wl,-z,now,-O1,--hash-style=gnu,--sort-common -Wl,--as-needed -o upload.la -rpath
# /usr/local/libexec/ulib/usp -module -export-dynamic -avoid-version -no-undefined upload.lo ../../../../../../src/ulib/libulib_g.la -ltcc -lxml2 -ldbi -lmysqlclient -lldap -llber
# -lcurl -lssh -lexpat -lpcre -lssl -lcrypto -lmagic -luuid -lz  -lpthread -ldl -lcap
# rm upload.lo
