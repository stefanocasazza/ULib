# configure.in -- Process this file with autoconf to produce configure
#
# An autoconf script to automatically configure ULib.
#
# Process this file with autoconf to produce a configure script.
#
# The autoconf documentation states that this is the recommended sequence for this file:
#
#  AC_INIT
#
#  checks for programs
#  checks for libraries
#  checks for header files
#  checks for typedefs
#  checks for structures
#  checks for variables
#  checks for compiler characteristics
#  checks for library functions
#  checks for system services
#
#  AC_CONFIG_FILES([FILE...])

# SECTION 1: AC_INIT
#
# This macro must be the very first invoked

AC_PREREQ([2.69])
AC_INIT([ULib],[1.4.2],[stefano.casazza@gmail.com],[ULib],[http://stefanocasazza.github.io/ULib/])
AC_CONFIG_SRCDIR([src/ulib/string.cpp])
AC_CONFIG_MACRO_DIR([m4])

# Define a msg macro
define(TWOCAN_CONF_MSG,[
AC_MSG_RESULT()
AC_MSG_RESULT(${T_MD}$1:${T_ME})
])

# Scary shtool stuff... borowed from pth-1.0
if test ".`echo dummy $@ | grep help`" = .; then
    ac_shtool="$srcdir/shtool"
    PLATFORM=`$ac_shtool platform`
    T_MD="`$ac_shtool echo -n -e %B 2>/dev/null`"
    T_ME="`$ac_shtool echo -n -e %b 2>/dev/null`"
    echo "Platform: ${T_MD}${PLATFORM}${T_ME}"
    echo "Configuring ${T_MD}ULib${T_ME} (C++ Library)"
fi

TWOCAN_CONF_MSG(Initial phase)

# Check what platform we are running on
AC_CANONICAL_TARGET
# Pick a newer and better tar implementation like tar-pax
AM_INIT_AUTOMAKE([subdir-objects no-define tar-pax])
AM_MAINTAINER_MODE
AM_SILENT_RULES([yes])

AC_USE_SYSTEM_EXTENSIONS

dnl This is a C++ library after all
AC_LANG([C++])

dnl Initial CFLAGS
AC_MSG_CHECKING(initial CFLAGS)
AC_MSG_RESULT($CFLAGS)

# Generate a header file with all settings, a config.h file (Automake will add -DHAVE_CONFIG_H)
AC_CONFIG_HEADERS([include/ulib/internal/config.h])

# Check for architecture for the target
AC_MSG_CHECKING([which architecture the target is])

LINUX_ARCH=
case $target in
	x86_64)			LINUX_ARCH=x86_64 ;;
	i*86)				LINUX_ARCH=i386 ;;
	arm*)				LINUX_ARCH=arm ;;
	aarch64*)		LINUX_ARCH=arm64 ;;
	sh*)				LINUX_ARCH=sh ;;
	mips*)			LINUX_ARCH=mips ;;
	or1k*)			LINUX_ARCH=openrisc ;;
	microblaze)		LINUX_ARCH=microblaze ;;
	powerpc*|ppc*) LINUX_ARCH=powerpc ;;
	*)					LINUX_ARCH=unknown ;;
esac

#AC_SUBST(LINUX_ARCH)
AS_IF([test "x$LINUX_ARCH" = xunknown], [AC_MSG_RESULT([unknown])], [test "x$LINUX_ARCH" != xunknown], [AC_MSG_RESULT([$LINUX_ARCH])])

# Check for Operating system and libc to use in $target
AC_MSG_CHECKING([which operating system the target is])

OPERATINGSYSTEM=
TARGET_LIBC=
case $target in
	*-*-linux-*) OPERATINGSYSTEM=linux ;;
	*-none-*)	 OPERATINGSYSTEM=baremetal ;;
	*)				 OPERATINGSYSTEM=unknown ;;
esac

#AC_SUBST(OPERATINGSYSTEM)
AS_IF([test "x$OPERATINGSYSTEM" = xunknown], [AC_MSG_RESULT([unknown])], [test "x$OPERATINGSYSTEM" != xunknown], [AC_MSG_RESULT([$OPERATINGSYSTEM])])

AC_MSG_CHECKING([which libc is detected for the target])
if test "x$OPERATINGSYSTEM" = xlinux ; then
	case $target in
		*-*-linux-gnu*)	 TARGET_LIBC=glibc ;;
		*-*-linux-musl*)	 TARGET_LIBC=musl ;;
		*-*-linux-uclibc*) TARGET_LIBC=uclibc ;;
		*)						 TARGET_LIBC=unknown ;;
	esac
fi

#AC_SUBST(TARGET_LIBC)
AS_IF([test "x$TARGET_LIBC" = xunknown], [AC_MSG_RESULT([unknown])], [test "x$TARGET_LIBC" != xunknown], [AC_MSG_RESULT([$TARGET_LIBC])])

# check for cross compiling
if test "$cross_compiling" = "yes"; then
	AM_CONDITIONAL(CROSS_COMPILING, true)
	PKG_CONFIG_PATH=${STAGING_DIR}/usr/lib/pkgconfig
	TWOCAN_CONF_MSG(Build for Target <${target}> with Environment <${STAGING_DIR}>)
	if test "x$STAGING_DIR" != "x" ; then
		CROSS_ENVIRONMENT=$STAGING_DIR
	fi
else
	AM_CONDITIONAL(CROSS_COMPILING, false)
	PKG_CONFIG_PATH=`pkg-config --variable pc_path pkg-config`
fi

# checks for programs
TWOCAN_CONF_MSG(Checking for programs)

AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AC_C_RESTRICT
AC_C_VOLATILE
AC_C_INLINE

# clang is mostly GCC-compatible, but its version is much lower, so we have to check for it.
AC_MSG_CHECKING([if compiling with clang])

AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([], [[
#ifndef __clang__
not clang
#endif
]])],
[GCC_IS_CLANG=yes], [GCC_IS_CLANG=no])

AC_MSG_RESULT([$GCC_IS_CLANG])
#AC_SUBST(GCC_IS_CLANG)

os_version=`uname -a`
AC_DEFINE_UNQUOTED(_OS_VERSION, "$os_version", [Operating System version])
AC_MSG_CHECKING([for a supported OS])
case "$target_os" in
	*linux*)
		AC_MSG_RESULT([ok ($target_os)])

		# Ok, this is linux. Check the kernel version
		AC_MSG_CHECKING([for the kernel version])

		kernel=`uname -r`

		case "$kernel" in
			4.*)
				AC_MSG_RESULT([4 family ($kernel)])
#				AC_DEFINE([KERNEL_4], 1, [Define to 1 if you're using Linux 4.x])
			;;

			3.*)
				AC_MSG_RESULT([3 family ($kernel)])
#				AC_DEFINE([KERNEL_3], 1, [Define to 1 if you're using Linux 3.x])
			;;

			2.6.*)
				AC_MSG_RESULT([2.6 family ($kernel)])
#				AC_DEFINE([KERNEL_2_6], 1, [Define to 1 if you're using Linux 2.6.x])
			;;

			2.4.*)
				AC_MSG_RESULT([2.4 family ($kernel)])
#				AC_DEFINE([KERNEL_2_4], 1, [Define to 1 if you're using Linux 2.4.x])
			;;

			*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3-4\].x.x])
			;;
		esac

		AM_CONDITIONAL(LINUX, true)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;

	mingw*)
		AC_MSG_RESULT([ok ($target_os)])

		enable_stdcpp="no"
		CPPFLAGS="$CPPFLAGS -D__USE_CRTIMP"

		if test "$enable_static" = "yes"; then
			AC_MSG_WARN([Disabling static library build, must build as DLL on Windows])
			enable_static=no
		fi
		if test "$enable_shared" = "no"; then
			AC_MSG_WARN([Enabling shared library build, must build as DLL on Windows])
		fi
		enable_shared=yes

		AC_CHECK_TOOL(WINDRES, windres, no)
		if test "$WINDRES" = no; then
			AC_MSG_ERROR([*** Could not find an implementation of windres in your PATH])
		fi
		AC_CHECK_PROG(ms_librarian, lib.exe, yes, no)

		# We currently use .def files on Windows
		# Note that -module isn't included here since automake needs to see it to know
		# that something like server_plugin_http.la is a valid libtool archive
		MODULE_LIBTOOL_OPTIONS=" $MODULE_LIBTOOL_OPTIONS -no-undefined"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, true)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;

	cygwin*)
		AC_MSG_RESULT([ok ($target_os)])

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, true)
		AM_CONDITIONAL(SOLARIS, false)

		MODULE_FLAGS="-module -shared -no-undefined"
	;;

	*solaris*)
		AC_MSG_RESULT([ok ($target_os)])

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, true)
	;;

	osf*)
		MODULE_FLAGS="-module -shared -no-undefined"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

	darwin*)
		CPPFLAGS="$CPPFLAGS -D__OSX__ -D__APPLE__"
		MODULE_FLAGS="-dynamic -bundle -undefined suppress -flat_namespace -read_only_relocs suppress"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

   *minix*)
		enable_stdcpp="no"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

	*)
		AC_MSG_RESULT([no ($target_os)])

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;
esac

# Check for the compiler support
gcc_major_4_1="no"
if test "$ac_cv_c_compiler_gnu" = "yes" -a "x$GCC_IS_CLANG" = xno; then
	# We don't want gcc 2.7
	AC_MSG_CHECKING([for a supported version of gcc])
	gcc_version=`$CC -dumpversion`
	case "$gcc_version" in
		1.*|2.7.*|2.8.*)
			AC_MSG_RESULT([no ($gcc_version)])
			AC_MSG_ERROR([please use a more recent (>= gcc-2.95) version of gcc])
		;;
		2.95)
			enable_stdcpp="no"
			AC_MSG_RESULT([ok ($gcc_version)])
		;;
      3*|4.0*)
			AC_MSG_RESULT([ok ($gcc_version)])
		;;
		4.1*|4.2*|4.3*|4.4*|4.5*|4.6*|4.7*)
			AC_MSG_RESULT([ok ($gcc_version)])

			gcc_major_4_1="yes"
		;;
		*)
			AC_MSG_RESULT([ok ($gcc_version)])

			gcc_major_4_1="yes"

			AC_MSG_CHECKING([whether there are the gcc built-in functions to detect run-time CPU type and ISA])
			AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>]],[[__builtin_cpu_init();if (__builtin_cpu_supports("sse4.2")) return 0; return 1;]])],
			  [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_BUILTIN_CPU_INIT],[1],[there are the built-in functions to detect run-time CPU type and ISA])],
			  [AC_MSG_RESULT([no])])

			if test "$cross_compiling" != "yes"; then
				AC_MSG_CHECKING([if we have support for crc32 intrinsics])
				echo 'void f(){}' >conftest.cpp
				if test -z "`$CC -mcrc32 -c conftest.cpp 2>&1`"; then
					CPPFLAGS_save=$CPPFLAGS
					CPPFLAGS="$CPPFLAGS -mcrc32"
					AC_CACHE_VAL(ac_cv_crc32_intrinsics, [
					  AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>]],[[__builtin_cpu_init();if (__builtin_cpu_supports("sse4.2")) return 0; return 1;]])],
						[ac_cv_crc32_intrinsics="yes"], [ac_cv_crc32_intrinsics="no"]) ])
					if test "x$ac_cv_crc32_intrinsics" = xyes; then
						AC_MSG_RESULT([yes])
						AC_DEFINE([USE_HARDWARE_CRC32],[1],[Define if we have support for crc32 intrinsics])
					else
						AC_MSG_RESULT([no])
						CPPFLAGS=$CPPFLAGS_save
					fi
				fi
			fi
		;;
	esac
fi

# GNUC 4.1.0 and later versions provide built-in functions
# for atomic memory access. see online doc for details at
# http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html

if test "$gcc_major_4_1" = "yes"; then
	AC_MSG_CHECKING(if you want to enable build of gcc atomic support)
	AC_ARG_ENABLE(atomics, [  --enable-atomics          enable gcc atomic support [[default=yes]]])
	if test -z "$enable_atomics"; then
		enable_atomics="yes"
	fi
	AC_MSG_RESULT([$enable_atomics])
fi

if test "$enable_atomics" = "yes" ; then
	AC_DEFINE(HAVE_GCC_ATOMICS, [1], [cannot test in autoconf safely])
fi

# SECTION 2: Configure script command line options

# Shared library versioning
#GENERIC_LIBRARY_VERSION=$PACKAGE_VERSION
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed
#AC_SUBST(GENERIC_LIBRARY_VERSION)

# ==========================================================================================================
# Updating library version information
# ==========================================================================================================
# If you want to use libtool's versioning system, then you must specify the version information to libtool
# using the -version-info flag during link mode (Note: Link mode)
#
# This flag accepts an argument of the form CURRENT[:REVISION[:AGE]].
# So, passing -version-info 3:12:1 sets CURRENT to 3, REVISION to 12, and AGE to 1
#
# If either REVISION or AGE are omitted, they default to 0. Also note that AGE must be less than or equal to the CURRENT interface number.
#
# Here are a set of rules to help you update your library version information:
#
#  1. Start with version information of 0:0:0 for each libtool library.
#
#  2. Update the version information only immediately before a public
#     release of your software.  More frequent updates are unnecessary,
#     and only guarantee that the current interface number gets larger
#     faster.
#
#  3. If the library source code has changed at all since the last
#     update, then increment REVISION (C:R:A becomes C:r+1:A).
#
#  4. If any interfaces have been added, removed, or changed since the
#     last update, increment CURRENT, and set REVISION to 0.
#
#  5. If any interfaces have been added since the last public release,
#     then increment AGE.
#
#  6. If any interfaces have been removed since the last public release,
#     then set AGE to 0.
#
#   *NEVER* try to set the interface numbers so that they correspond to the release number of your package. This is an abuse that only
#   fosters misunderstanding of the purpose of library versions. Instead, use the -release flag (Note: Release numbers), but be warned
#   that every release of your package will not be binary compatible with any other release.
# ==========================================================================================================

# By default, this macro turns on shared libraries if they are available, and also enables static libraries
# if they don't conflict with the shared libraries. You can modify these defaults by calling either the
# AC_DISABLE_SHARED or AC_DISABLE_STATIC macros:

dnl Initialize Libtool
m4_ifdef([LT_INIT], [LT_INIT([dlopen shared static win32-dll])], [AC_LIBTOOL_DLOPEN AC_LIBTOOL_WIN32_DLL])

dnl ********************************************************
dnl * Options to pass to libtool
dnl ********************************************************

MODULE_FLAGS="-module -shared -avoid-version"
MODULE_LIBTOOL_OPTIONS="-export-dynamic -avoid-version"

AM_CONDITIONAL(MS_LIB_AVAILABLE, test x$ms_librarian = xyes)

LT_INIT

AC_SUBST(MODULE_LIBTOOL_OPTIONS)

if test "$enable_static" = "yes" && test "$enable_shared" = "no"; then
	enable_stdcpp="no"
	AM_CONDITIONAL(STATIC_ONLY, true)
	AC_DEFINE(U_STATIC_ONLY,, [STATIC_ONLY])
else
	AM_CONDITIONAL(STATIC_ONLY, false)
fi

AC_MSG_CHECKING([for full c++ support and linking model])
AC_ARG_ENABLE(stdcpp,  [  --enable-stdcpp           compile with stdc++ runtime overhead [[default=yes]]])

if test -z "$enable_stdcpp"; then
	enable_stdcpp="yes"
fi

ULIB_LIBS=
if test "$enable_stdcpp" = "yes"; then
   AC_MSG_RESULT(yes)
	AM_CONDITIONAL(STDCPP, true)
	AC_DEFINE(U_STDCPP_ENABLE, [1], [full c++ support and linking model])
else
   AC_MSG_RESULT(no)
	AM_CONDITIONAL(STDCPP, false)
	if test x"$GCC" = "xyes" ; then
		lcflags="-nodefaultlibs -nostdinc++"
	else
      lcflags="-library=no%Cstd -features=no%except -features=no%rtti"
	fi
   for flag in $lcflags; do
		AC_MSG_CHECKING([whether $CXX supports $flag])
		echo 'void f(){}' >conftest.cpp
		if test -z "`$CXX $flag -c conftest.cpp 2>&1`"; then
			ULIB_LIBS="$flag $ULIB_LIBS"
			AC_MSG_RESULT(yes)
		else
			AC_MSG_RESULT(no)
		fi
		rm -f conftest*
	done
fi

ULIB="ulib"
ulib_dbg_msg="no (--enable-debug)"

AC_MSG_CHECKING(for debugging)
AC_ARG_ENABLE(debug, [  --enable-debug            compile for debugging [[default=no]]])

if test -z "$enable_debug"; then
	enable_debug="no"
fi

AM_CONDITIONAL(DEBUG, false)

if test "$enable_debug" = "yes"; then
   AC_MSG_RESULT(yes)
	AM_CONDITIONAL(DEBUG, true)
	AC_DEFINE(DEBUG, [1], [debugging model])
	ULIB="ulib_g"
   ulib_dbg_msg="enabled"
   CPPFLAGS="$CPPFLAGS -DDEBUG"
else
   AC_MSG_RESULT(no)
fi

AC_SUBST(ULIB)

ULIB_LIBC="-lc"

AC_ARG_WITH(libc,
    AS_HELP_STRING([--with-libc=path],[specify path to alternate libc [[default=c]]]),
	 [ if test x$with_libc != x ; then
        clib=`echo $with_libc | sed s/[-]l//`
        ULIB_LIBC=-l${clib}
		fi ])

clib=`echo $ULIB_LIBC | sed s/[-]l//`

AH_BOTTOM([
#include <ulib/internal/platform.h>
])

TWOCAN_CONF_MSG(Checking for headers)

AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_MAJOR
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT

AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h libintl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h \
						string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h sys/resource.h syslog.h termios.h unistd.h wchar.h \
						assert.h byteswap.h endian.h execinfo.h ifaddrs.h mcheck.h ndir.h dirent.h ndir.h string.h netpacket/packet.h sched.h \
						semaphore.h sysexits.h streambuf.h poll.h net/if_arp.h sys/dir.h sys/ndir.h sys/inotify.h sys/sendfile.h sys/sched.h \
						sys/syscall.h linux/netfilter_ipv4/ipt_ACCOUNT.h libnetfilter_conntrack/libnetfilter_conntrack.h \
						/usr/src/linux/include/generated/uapi/linux/version.h])

AC_MSG_CHECKING([whether sys/inotify.h actually works])
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sys/inotify.h>]],[[return (-1 == inotify_init());]])],
  [AC_MSG_RESULT([yup]); AC_DEFINE([SYS_INOTIFY_H_EXISTS_AND_WORKS],[1],[sys/inotify.h exists and works correctly])],
  [AC_MSG_RESULT([nope, using own inotify headers])])

AM_CONDITIONAL(ASSERT, test "$ac_cv_header_assert_h" = "yes")

if test "$ac_cv_header_sys_inotify_h" = "yes"; then
AC_CHECK_FUNCS([inotify_init1])
fi

socket_lib=""
OPENSSL_LINK=""
threading="none"

AC_CHECK_LIB(msvcrt, fopen, [
    threading="msw"
    clib="msvcrt"
    ULIB_LIBC="-lmingwex -lmingw32 -lcrypt32 -luser32 -lwinmm -lws2_32 -lwsock32 -lmsvcrt -lshell32 -ladvapi32 -lkernel32 -lgcc"
    ULIB_LIBS="--enable-stdcall-fixup $ULIB_LIBS"
    OPENSSL_LINK="-lgdi32"
    AC_DEFINE(HAVE_INET_NTOP, [1], [have inet_ntop])
    AC_DEFINE(HAVE_GETADDRINFO, [1], [have getaddrinfo]) ],[
    AC_CHECK_LIB($clib, socketpair, [
        AC_DEFINE(HAVE_SOCKETPAIR, [1], [have socketpair])
    ],[
        AC_CHECK_LIB(socket, socketpair, [AC_DEFINE(HAVE_SOCKETPAIR, [1], [have socketpair])])
    ])
    AC_CHECK_LIB($clib, getaddrinfo, [
        AC_DEFINE(HAVE_GETADDRINFO, [1], [have getaddrinfo])
    ],[
        AC_CHECK_LIB(socket, getaddrinfo, [AC_DEFINE(HAVE_GETADDRINFO, [1], [have getaddrinfo])])
    ])
    AC_CHECK_LIB($clib, gethostbyname2, [
        AC_DEFINE(HAVE_GETHOSTBYNAME2, [1], [have gethostbyname2])
    ],[
        AC_CHECK_LIB(socket, gethostbyname2, [AC_DEFINE(HAVE_GETHOSTBYNAME2, [1], [have gethostbyname2])])
    ])

    AC_CHECK_LIB($clib, inet_ntop, [
        AC_DEFINE(HAVE_INET_NTOP, [1], [have inet ntop])
    ],[
        AC_CHECK_LIB(socket, inet_ntop, [
            AC_DEFINE(HAVE_INET_NTOP, [1], [have inet ntop])
        ],[
            AC_CHECK_LIB(nsl, inet_ntop, [
                AC_DEFINE(HAVE_INET_NTOP, [1], [have inet_ntop])
                socket_lib="-lnsl"
            ])
        ])
    ])

    AC_CHECK_LIB(socket, socket,[
        AC_CHECK_LIB(socket, socketpair,[
            AC_DEFINE(HAVE_SOCKETPAIR, [1], [have socketpair in libsocket])
        ])
        socket_lib="-lsocket"
    ])
])

AC_CHECK_LIB($clib, lstat, [
    AC_DEFINE(HAVE_LSTAT, [1], [have lstat])
])

AC_CHECK_LIB($clib, strcoll, [
    AC_DEFINE(HAVE_STRCOLL, [1], [string collation])
])

AC_CHECK_LIB($clib, stricmp, [
    AC_DEFINE(HAVE_STRICMP, [1], [string icmp])
])

AC_CHECK_LIB($clib, stristr, [
    AC_DEFINE(HAVE_STRISTR, [1], [string istr])
])

AC_CHECK_LIB($clib, sysconf, [
    AC_DEFINE(HAVE_SYSCONF, [1], [system config])
])

##AC_CHECK_LIB($clib, localtime_r, [
##    AC_DEFINE(HAVE_LOCALTIME_R, [1], [system localtime_r])
##])
##AC_CHECK_LIB($clib, gmtime_r, [
##    AC_DEFINE(HAVE_GMTIME_R, [1], [system gmtime_r])
##])

AC_CHECK_LIB($clib, posix_memalign, [
    AC_DEFINE(HAVE_POSIX_MEMALIGN, [1], [posix memory alignment])
])

tlib=""
plib=""
PTHREAD_FLAGS=""

AC_MSG_CHECKING(if you want to enable build of thread support)
AC_ARG_ENABLE(thread, [  --enable-thread           enable build of thread support [[default=yes]]])
if test -z "$enable_thread"; then
	enable_thread="yes"
fi
AC_MSG_RESULT([$enable_thread])

AC_MSG_CHECKING(if you want to always use GNU pth for threading)
AC_ARG_ENABLE(pth,	 [  --enable-pth              always use GNU pth for threading [[default=no]]])
if test -z "$enable_pth"; then
	enable_pth="no"
fi
AC_MSG_RESULT([$enable_pth])

if test "$threading" = "none" ; then
    if test "$enable_pth" != "yes" ; then
        AC_CHECK_HEADER(pthread.h, [
            PTHREAD_FLAGS="-mt -pthread -pthreads"
            AC_CHECK_LIB(pthread,pthread_create,[ tlib="pthread" ])
            if test -z "$tlib" ; then
                AC_CHECK_LIB($clib, pthread_create, [tlib="$clib"])
            fi
            if test -z "$tlib" ; then
                AC_CHECK_LIB(c_r, pthread_create, [
                    tlib="c_r"
                    if test "$clib" = "c" ; then
                        AC_CHECK_LIB(c_r, malloc, [clib="c_r"])
                    fi
                ])
            fi
            if test ! -z "$tlib" ; then
                AC_CHECK_LIB($tlib,pthread_condattr_setclock,[
                    AC_DEFINE(HAVE_PTHREAD_CONDATTR_SETCLOCK, [1], [pthread clocking])
                ])
                AC_CHECK_LIB($tlib,pthread_setconcurrency,[
                    AC_DEFINE(HAVE_PTHREAD_SETCONCURRENCY, [1], [pthread concurrency])
                ])

                AC_CHECK_LIB($tlib,pthread_yield,[
                    AC_DEFINE(HAVE_PTHREAD_YIELD, [1], [pthread yield],[
                        AC_CHECK_LIB($tlib,pthread_yield_np,[
                            AC_DEFINE(HAVE_PTHREAD_YIELD_NP, [1], [pthread yield np])
                        ])
                    ])
                ])

                AC_CHECK_LIB($tlib,pthread_delay,[
                    AC_DEFINE(HAVE_PTHREAD_DELAY, [1], [pthread delay])
                ])
                AC_CHECK_LIB($tlib,pthread_delay_np,[
                    AC_DEFINE(HAVE_PTHREAD_DELAY_NP, [1], [pthread delay np])
                ])
                AC_CHECK_LIB($tlib,pthread_setschedprio,[
                    AC_DEFINE(HAVE_PTHREAD_SETSCHEDPRIO, [1], [pthread scheduling])
                ])
                # Missing from Android's pthread implementation but the default
                # values for newly created threads corresponds to the one we set
                AC_CHECK_LIB($tlib,pthread_attr_setinheritsched,[
                    AC_DEFINE(HAVE_PTHREAD_ATTR_SETINHRITSCHED, [1], [pthread inherit scheduling attribute])
                ])
            fi
        ],[
            AC_CHECK_HEADER(windows.h,, [
                AC_CHECK_HEADER(pth.h,[
                    plib="-lpth"
						  CPPFLAGS="$CPPFLAGS -D__PTH__"
                ],[
                    AC_MSG_ERROR("pthread or GNU pth support required")
                ])
            ])
        ])
    else
        AC_CHECK_HEADER(pth.h,[
            plib="-lpth"
				CPPFLAGS="$CPPFLAGS -D__PTH__"
        ],[
            AC_MSG_ERROR("GNU pth required")
        ])
    fi
fi

if test ! -z "$tlib"; then
	plib="$plib -l$tlib"
fi

AM_CONDITIONAL(PREAD, false)
AM_CONDITIONAL(DAEMON, false)
AM_CONDITIONAL(TIMEGM, false)
AM_CONDITIONAL(MREMAP, false)
AM_CONDITIONAL(PTHREAD, false)
AM_CONDITIONAL(MEMRCHR, false)
AM_CONDITIONAL(STRNDUP, false)
AM_CONDITIONAL(MKDTEMP, false)
AM_CONDITIONAL(FNMATCH, false)
AM_CONDITIONAL(STRPTIME, false)
AM_CONDITIONAL(GMTIME_R, false)
AM_CONDITIONAL(SENDFILE, false)
AM_CONDITIONAL(NANOSLEEP, false)
AM_CONDITIONAL(SEMAPHORE, false)
AM_CONDITIONAL(FALLOCATE, false)
AM_CONDITIONAL(FALLOCATE64, false)
AM_CONDITIONAL(GETOPT_LONG, false)

if test "$enable_thread" != "yes"; then
   ulib_thread_msg="no (--enable-thread)"
else
	ulib_thread_msg="enabled"
	AM_CONDITIONAL(PTHREAD, true)
	AC_DEFINE(ENABLE_THREAD, [1], [Define if enable thread support])
	if test ! -z "$PTHREAD_FLAGS"; then
		 for flag in $PTHREAD_FLAGS ; do
			  AC_MSG_CHECKING([whether $CXX supports $flag])
			  echo 'void f(){}' >conftest.cpp
			  if test -z "`$CXX $flag -c conftest.cpp 2>&1`"; then
					AC_MSG_RESULT(yes)
					CPPFLAGS="$CPPFLAGS $flag"
					break
			  else
					AC_MSG_RESULT(no)
			  fi
		 done
	fi
fi

TWOCAN_CONF_MSG(Checking for functions)

AC_FUNC_CHOWN
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_FUNC_CLOSEDIR_VOID

rlib=""
posix4_lib=""

for func in accept4 atexit daemon dup3 epoll_create1 epoll_wait epoll_ctl_batch fallocate fallocate64 fnmatch getaddrinfo getnameinfo getopt_long getpriority inet_ntop memmem memrchr mkdtemp mremap pipe2 posix_spawn pread sched_getaffinity sem_init sem_getvalue sendfile sendfile64 strndup strptime strsignal strtof strtold strtoull gmtime_r timegm nanosleep strerror shm_open; do
    found="no"
    AC_CHECK_FUNC($func,[
        found=$func
    ],[
        AC_CHECK_LIB(posix4, $func, [
            found=$func
            posix4_lib="-lposix4"
        ],[
            AC_CHECK_LIB(rt, $func, [
                found=$func
                rlib="-lrt"
            ],[
                if test ! -z "$tlib" ; then
                    AC_CHECK_LIB($tlib, $func, [
                        found=$func
                    ])
                fi
            ])
        ])
    ])
    case $found in
    accept4)
        AC_DEFINE(HAVE_ACCEPT4, [1], [has accept4])
        ;;
    atexit)
        AC_DEFINE(HAVE_ATEXIT, [1], [has atexit])
        ;;
    daemon)
		  AM_CONDITIONAL(DAEMON, true)
        AC_DEFINE(HAVE_DAEMON, [1], [has daemon])
        ;;
    dup3)
        AC_DEFINE(HAVE_DUP3, [1], [has dup3])
        ;;
    epoll_create1)
        AC_DEFINE(HAVE_EPOLL_CREATE1, [1], [has epoll_create1])
        ;;
    epoll_wait)
        AC_DEFINE(HAVE_EPOLL_WAIT, [1], [has epoll_wait])
        ;;
    epoll_ctl_batch)
        AC_DEFINE(HAVE_EPOLL_CTL_BATCH, [1], [has epoll_ctl_batch])
        ;;
    fallocate)
		  AM_CONDITIONAL(FALLOCATE, true)
        AC_DEFINE(HAVE_FALLOCATE, [1], [has fallocate])
        ;;
    fallocate64)
		  AM_CONDITIONAL(FALLOCATE64, true)
        AC_DEFINE(HAVE_FALLOCATE64, [1], [has fallocate64])
        ;;
    fnmatch)
		  AM_CONDITIONAL(FNMATCH, true)
        AC_DEFINE(HAVE_FNMATCH, [1], [has fnmatch])
        ;;
    getaddrinfo)
        AC_DEFINE(HAVE_GETADDRINFO, [1], [has getaddrinfo])
        ;;
    getnameinfo)
        AC_DEFINE(HAVE_GETNAMEINFO, [1], [has getnameinfo])
        ;;
    getopt_long)
		  AM_CONDITIONAL(GETOPT_LONG, true)
        AC_DEFINE(HAVE_GETOPT_LONG, [1], [has getopt_long])
        ;;
    getpriority)
        AC_DEFINE(HAVE_GETPRIORITY, [1], [has getpriority])
        ;;
    inet_ntop)
        AC_DEFINE(HAVE_INET_NTOP, [1], [has inet_ntop])
        ;;
    memmem)
        AC_DEFINE(HAVE_MEMMEM, [1], [has memmem])
        ;;
    memrchr)
		  AM_CONDITIONAL(MEMRCHR, true)
        AC_DEFINE(HAVE_MEMRCHR, [1], [has memrchr])
        ;;
    mkdtemp)
		  AM_CONDITIONAL(MKDTEMP, true)
        AC_DEFINE(HAVE_MKDTEMP, [1], [has mkdtemp])
        ;;
    mremap)
		  AM_CONDITIONAL(MREMAP, true)
        AC_DEFINE(HAVE_MREMAP, [1], [has mremap])
        ;;
    pipe2)
        AC_DEFINE(HAVE_PIPE2, [1], [has pipe2])
        ;;
    posix_spawn)
        AC_DEFINE(HAVE_POSIX_SPAWN, [1], [has posix_spawn])
        ;;
    pread)
		  AM_CONDITIONAL(PREAD, true)
        AC_DEFINE(HAVE_PREAD, [1], [has pread])
        ;;
    sched_getaffinity)
        AC_DEFINE(HAVE_SCHED_GETAFFINITY, [1], [has sched_getaffinity])
        ;;
    sem_init)
		  AM_CONDITIONAL(SEMAPHORE, true)
        AC_DEFINE(HAVE_SEM_INIT, [1], [has sem_init])
        ;;
    sem_getvalue)
        AC_DEFINE(HAVE_SEM_GETVALUE, [1], [has sem_getvalue])
        ;;
    sendfile)
		  AM_CONDITIONAL(SENDFILE, true)
        AC_DEFINE(HAVE_SENDFILE, [1], [has sendfile])
        ;;
    sendfile64)
		  AM_CONDITIONAL(SENDFILE, true)
        AC_DEFINE(HAVE_SENDFILE64, [1], [has sendfile64])
        ;;
    strndup)
		  AM_CONDITIONAL(STRNDUP, true)
        AC_DEFINE(HAVE_STRNDUP, [1], [has strndup])
        ;;
    strptime)
		  AM_CONDITIONAL(STRPTIME, true)
        AC_DEFINE(HAVE_STRPTIME, [1], [has strptime])
        ;;
    strsignal)
        AC_DEFINE(HAVE_STRSIGNAL, [1], [has strsignal])
        ;;
    strtof)
        AC_DEFINE(HAVE_STRTOF, [1], [has strtof])
        ;;
    strtold)
        AC_DEFINE(HAVE_STRTOLD, [1], [has strtold])
        ;;
    strtoull)
        AC_DEFINE(HAVE_STRTOULL, [1], [has strtoull])
        ;;
    timegm)
		  AM_CONDITIONAL(TIMEGM, true)
        AC_DEFINE(HAVE_TIMEGM, [1], [has timegm])
        ;;
    gmtime_r)
		  AM_CONDITIONAL(GMTIME_R, true)
        AC_DEFINE(HAVE_GMTIME_R, [1], [has gmtime_r])
        ;;
    nanosleep)
		  AM_CONDITIONAL(NANOSLEEP, true)
        AC_DEFINE(HAVE_NANOSLEEP, [1], [has nanosleep])
        ;;
    strerror)
        AC_DEFINE(HAVE_STRERROR, [1], [has strerror])
        ;;
    shm_open)
        AC_DEFINE(HAVE_SHM_OPEN, [1], [has shm_open])
        ;;
    esac
done

dnl Mac OSX (10.5 and later)
if test "$ac_cv_func_sendfile" != "yes"; then
  AC_TRY_LINK(
      [ #include <sys/types.h>
        #include <sys/socket.h>
        #include <sys/uio.h>
      ],
      [ int i;
        off_t o, n;
        struct sf_hdtr h;
        (void)sendfile(i,i,o,&n,&h,i);
      ],
		AC_DEFINE([HAVE_MACOSX_SENDFILE], [1], [Define if you have Mac OSX sendfile() semantics]) )
fi

# C++ stuff must done after library and header (some C++ define require some header)

TWOCAN_CONF_MSG(C++ stuff)

dnl initialize all the info variables to 'no'
      ulib_ssl_msg="no (--with-ssl)"
      ulib_ssh_msg="no (--with-ssh)"
     ulib_pcre_msg="no (--with-pcre)"
     ulib_ldap_msg="no (--with-ldap)"
     ulib_libz_msg="no (--with-libz)"
ulib_libzopfli_msg="no (--with-libzopfli)"
     ulib_curl_msg="no (--with-curl)"
    ulib_expat_msg="no (--with-expat)"
    ulib_magic_msg="no (--with-magic)"
   ulib_parser_msg="no (--with-parser)"
    ulib_mysql_msg="no (--with-mysql)"
   ulib_sqlite_msg="no (--with-sqlite)"
    ulib_pgsql_msg="no (--with-pgsql)"
  ulib_libuuid_msg="no (--with-libuuid)"
 ulib_libevent_msg="no (--with-libevent)"
  ulib_libxml2_msg="no (--with-libxml2)"
      ulib_dbi_msg="no (--with-libdbi)"
ulib_pagespeed_msg="no (--with-page-speed)"
       ulib_v8_msg="no (--with-v8-javascript)"
      ulib_php_msg="no (--with-php-embedded)"
     ulib_ruby_msg="no (--with-ruby-embedded)"
     ulib_ruby_msg="no (--with-ruby-embedded)"
     ulib_ipv6_msg="no (--with-ipv6)"
      ulib_LFS_msg="no (--enable-LFS)"
    ulib_final_msg="no (--enable-final)"
      ulib_zip_msg="no (--enable-zip)"
     ulib_memp_msg="no (--enable-memory-pool)"
    ulib_cares_msg="no (--enable-libares)"
	  ulib_examples="no (--enable-examples)"
#  ulib_manual_msg="no (--enable-manual)"

# Add --{enable,disable,with,without}-feature options

AC_ARG_WITH(ipv6,
   AS_HELP_STRING([--with-ipv6],[Enable ipv6, default is disabled]),
	ulib_ipv6_msg="enabled",
   [ AC_CHECK_HEADERS(netinet6/in6.h linux/in6.h)
	  AC_CHECK_LIB(c, inet_pton,[ AC_DEFINE(HAVE_INET_PTON, [1], [ipv6 support])])
	  AC_CHECK_LIB(c, gethostbyname2,[ AC_DEFINE(HAVE_GETHOSTBYNAME2, [1], [ipv6 host lookup])]) ])

if test "$ulib_ipv6_msg" = "enabled"; then
	AC_DEFINE(ENABLE_IPV6,  1, [Define if  enable ipv6 support])
else
	AC_DEFINE(DISABLE_IPV6, 1, [Define if disable ipv6 support])
fi

AC_MSG_CHECKING(if you want to enable build of examples)
AC_ARG_ENABLE(examples,[  --enable-examples         enable building of examples [[default=yes]]])

if test -z "$enable_examples"; then
	enable_examples="yes"
	ulib_examples="enabled"
fi

if test "$enable_examples" = "yes"; then
   AC_MSG_RESULT(yes)
	AM_CONDITIONAL(ENABLE_EXAMPLES, true)
else
   AC_MSG_RESULT(no)
	AM_CONDITIONAL(ENABLE_EXAMPLES, false)
fi

AC_COMPILATION_OPTIONS

# SECTION 3: check for programs

AM_CONDITIONAL(USE_PARSER, false)

AC_MSG_CHECKING(if you want to enable use of bison/yacc)
AC_ARG_WITH(parser,
	[  --with-parser           use parser generator bison/yacc],
	[use_parser=$withval],
	[use_parser=no])
AC_MSG_RESULT($use_parser)
if test "$use_parser" = "yes" ; then
	AC_PROG_YACC
	AC_PROG_LEX
	# Just additional warning checks, since AC_PROG just sets 'yacc' even without one present
   AC_CHECK_PROGS(XYACC,$YACC bison yacc,none)
   if test "$XYACC" = "none"
   then
    	AC_MSG_ERROR([no suitable bison/yacc found. Please install the 'bison' package.])
   fi
   AC_CHECK_PROGS(XLEX,$LEX flex lex,none)
   if test "$XLEX" = "none"
   then
    	AC_MSG_ERROR([no suitable lex found. Please install the 'flex' package.])
   fi
	ulib_parser_msg="enabled"
	flex_version=$(flex --version | head -n 1)
	bison_version=$(bison --version | head -n 1)
	AM_CONDITIONAL(USE_PARSER, true)
	AC_DEFINE(USE_PARSER, 1, [Define if use parser generator bison/yacc])
else
	flex_version="None"
	bison_version="None"
fi

AC_MSG_CHECKING(if you want to use distcc)
AC_ARG_WITH(distcc,
	[  --with-distcc           using distcc we must avoid to use some gcc flags (-mtune=native,-flto,...)],
	[use_distcc=$withval],
	[use_distcc=no])
AC_MSG_RESULT($use_distcc)

# ---------------------------------------------------------------------------------------------------
# Enable Linux Netfilter Transparent Proxy
# ---------------------------------------------------------------------------------------------------
# AC_ARG_ENABLE(linux-netfilter,
# AS_HELP_STRING([--enable-linux-netfilter],[enable Transparent Proxy support for Linux (Netfilter)]),
# [ if test "$enableval" = "yes" ; then
#    AC_MSG_NOTICE([Linux (Netfilter) Transparent Proxy enabled])
#    AC_DEFINE(LINUX_NETFILTER,1,[Enable support for Transparent Proxy on Linux (Netfilter) systems])
#    LINUX_NETFILTER="yes"
#   else
#    AC_DEFINE(LINUX_NETFILTER,0,[Enable support for Transparent Proxy on Linux (Netfilter) systems])
#   fi
# ])
# if test "$LINUX_NETFILTER"; then
#  AC_MSG_CHECKING(if packages libnfnetlink and libnetfilter_conntrack are installed)
# 	PKG_CHECK_MODULES([LIBNFNETLINK], [libnfnetlink >= 1.0.0])
# 	PKG_CHECK_MODULES([LIBNETFILTER_CONNTRACK], [libnetfilter_conntrack >= 0.9.1])
# 	AC_SUBST(pkg_cv_LIBNETFILTER_CONNTRACK_LIBS)
# fi
# ---------------------------------------------------------------------------------------------------

if test "$enable_stdcpp" = "yes"; then
	AX_CXX_COMPILE_STDCXX_11([noext], [optional])

	AC_LANG_PUSH(C++)

	# Check that std::chrono::steady_clock is available. In particular,
	# gcc 4.6 does not have one, but has monotonic_clock which is the old
	# name existed in the pre-standard draft. If steady_clock is not
	# available, don't define HAVE_STEADY_CLOCK and replace steady_clock
	# with monotonic_clock.
	AC_MSG_CHECKING([whether std::chrono::steady_clock is available])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
	[[
	#include <chrono>
	]],
	[[
	auto tp = std::chrono::steady_clock::now();
	]])],
		 [AC_DEFINE([HAVE_STEADY_CLOCK], [1],
						[Define to 1 if you have the `std::chrono::steady_clock`.])
		  AC_MSG_RESULT([yes])],
		 [AC_MSG_RESULT([no])])

	# Check that std::future is available.
	AC_MSG_CHECKING([whether std::future is available])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
	[[
	#include <vector>
	#include <future>
	]],
	[[
	std::vector<std::future<int>> v;
	]])],
		 [AC_DEFINE([HAVE_STD_FUTURE], [1],
						[Define to 1 if you have the `std::future`.])
		  have_std_future=yes
		  AC_MSG_RESULT([yes])],
		 [have_std_future=no
		  AC_MSG_RESULT([no])])

	AC_LANG_POP()
fi

# SECTION 4: checks for libraries

libz_version="no"
libzopfli_version="no"
pcre_version="no"
ldap_version="no"
expat_version="no"
libssh_version="no"
libcurl_version="no"
ssl_version="no"
libmagic_version="no"
libmysql_version="no"
libsqlite_version="no"
pgsql_version="no"
libuuid_version="no"
libevent_version="no"
libxml2_version="no"
libdbi_version="no"
page_speed_version="no"
v8_version="no"
php_version="no"
rubyversion="no"

TWOCAN_CONF_MSG(Checking for libraries to link against)

AC_CHECK_PACKAGE

AC_CHECK_SQLITE3
AC_CHECK_MYSQL
AC_CHECK_PGSQL

AC_SUBST(PRG_LDFLAGS)

TWOCAN_CONF_MSG(Checking for libraries specific)

AM_CONDITIONAL(TCC, false)
AM_CONDITIONAL(ZIP, false)
 
if test "$USE_LIBZ" = "yes"; then
	ulib_libz_msg="yes ( $libz_version )"
	AC_CHECK_LIB(z,zlib_deflate_workspacesize)
	if test "$ac_cv_lib_z_zlib_deflate_workspacesize" = "yes"; then
		AC_DEFINE(U_ZLIB_DEFLATE_WORKSPACESIZE, 1, [Define if we have zlib with this function])
	fi
fi

if test "$USE_LIBZOPFLI" = "yes"; then
	ulib_libzopfli_msg="yes ( 1.0.1 )"
fi

if test "$enable_zip" = "yes" && test "$USE_LIBZ" = "yes"; then
	ulib_zip_msg="enabled"
	AM_CONDITIONAL(ZIP, true)
	AC_DEFINE(ENABLE_ZIP, 1, [Define if enable ZIP support])
else
	AC_DEFINE(DISABLE_ZIP, 1, [Define if disable ZIP support])
fi

if test "$USE_C_ARES" = "yes"; then
	ulib_cares_msg="yes ( $cares_version )"
fi

case "$target_os" in
	mingw*)
		if test "$with_gnu_ld" = "yes"; then
			LDFLAGS="${LDFLAGS} -Wl,--enable-auto-import -Wl,--stack=10485760"
		fi
	;;
	cygwin*)
		AC_DEFINE(HAVE_SIGINFO_T, 1, [Define if exist type siginfo_t])
		AC_DEFINE(HAVE_NANOSLEEP, 1, [Define if compiler recognizes nanosleep function])
	;;
	*)
      LIBS="-ldl"
      AC_CHECK_LIB(tcc,tcc_compile_string)
		if test "$ac_cv_lib_tcc_tcc_compile_string" = "yes"; then
			AM_CONDITIONAL(TCC, true)
			ULIB_LIBS="$ULIB_LIBS -ltcc"
		fi
      AC_CHECK_LIB(GeoIP,GeoIP_id_by_ipnum)
      LIBS=""

		AC_CHECK_TYPE([siginfo_t],			  [AC_DEFINE_UNQUOTED(HAVE_SIGINFO_T, 1, [Define if exist type siginfo_t])],				[],[#include <signal.h>])
		AC_CHECK_MEMBER([siginfo_t.si_addr],AC_DEFINE_UNQUOTED(HAVE_MEMBER_SI_ADDR,1,[Define if you have si_addr in siginfo_t]),[],[#include <signal.h>])

		if test "$ac_cv_cxx_visibility_inlines_hidden_flag" = "yes"; then
			AC_DEFINE(HAVE_GNUC_VISIBILTY, 1, [enable gcc visibility features])
			CXXFLAGS="$CXXFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
		fi
		if test "$with_gnu_ld" = "yes" && test "$enable_new_ldflags" = "yes"; then
			LDFLAGS="$LDFLAGS -Wl,-O1 -Wl,--as-needed -Wl,-z,now,-O1,--hash-style=gnu,--sort-common -Wl,--as-needed"
		fi
	;;
esac

# ---------------------------------------------------------------------------------------------------
# Check for PHP embedded
# ---------------------------------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to use php-embedded)
AC_ARG_WITH(php-embedded,
	[  --with-php-embedded        use php-embedded],
	[use_php=$withval],
	[use_php=no])
AC_MSG_RESULT($use_php)

if test "$use_php" = "yes" ; then
	TWOCAN_CONF_MSG(Checking for php-embedded package specific)

	# Path to php-config program
	AC_ARG_WITH(php-config,
		[  --with-php-config=PATH     Path to php-config tool. EX: /<localpath>/php-config],
		[PHPCONFIG="$withval"],
		[AC_PATH_PROGS(PHPCONFIG, php-config, , $PATH)])

	if test -z "$PHPCONFIG"; then
		AC_MSG_ERROR([Program php-config not found. Required for further processing. Use --with-php-config=path to specify absolute path to the php-config tool.])
	fi

	AC_SUBST(PHPCONFIG)

	USE_PHP=yes
	php_prefix=`$PHPCONFIG --prefix`
	php_version=$(php-config --version)
	AC_DEFINE(USE_PHP, 1, [Define if enable php support])

	# List of php-config include paths
	AC_ARG_WITH(php-includes,
		 [  --with-php-includes=PATHS  php-config include locations],
		 [PHPCONFIGINCLUDES="$withval"],
		 [PHPCONFIGINCLUDES=""])

	if test -z "$PHPCONFIGINCLUDES"; then
		PHPCONFIGINCLUDES="-I${php_prefix}/include/php/sapi/embed `$PHPCONFIG --includes`"
	fi

	AC_SUBST(PHPCONFIGINCLUDES)

	# Check for php_embed.h
	AC_LANG([C])
	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $PHPCONFIGINCLUDES"

	AC_MSG_CHECKING([whether php_embed.h actually works])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <php_embed.h>]], [[return 0;]])],
						 [AC_MSG_RESULT([php_embed.h exists and works correctly])],
						 [AC_MSG_ERROR([Cannot find php_embed.h. Check if php was compiled with the --enable-embed option.])])

	CPPFLAGS=$save_CPPFLAGS
	AC_LANG([C++])

	# List of php-config ldflags
	AC_ARG_WITH(php-ldflags,
		 [  --with-php-ldflags=LDFLAGS php-config ldflags list],
		 [PHPCONFIGLDFLAGS="$withval"],
		 [PHPCONFIGLDFLAGS=""])

	if test -z "$PHPCONFIGLDFLAGS"; then
		PHPCONFIGLDFLAGS="-L${php_prefix}/lib -L${php_prefix}/lib32 -L${php_prefix}/lib64 `$PHPCONFIG --ldflags`"
	fi

	AC_SUBST(PHPCONFIGLDFLAGS)

	# List of php-config include paths
	AC_ARG_WITH(php-libs,
		 [  --with-php-libs=LIBS       php-config lib locations],
		 [PHPCONFIGLIBS="$withval"],
		 [PHPCONFIGLIBS=""])

	if test -z "$PHPCONFIGLIBS"; then
		PHPCONFIGLIBS="-L`$PHPCONFIG --ldflags`/lib -lphp5 `$PHPCONFIG --libs`"
	fi

	AC_SUBST(PHPCONFIGLIBS)
fi
# ---------------------------------------------------------------------------------------------------

# ---------------------------------------------------------------------------------------------------
# Check for RUBY embedded
# ---------------------------------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to use ruby-embedded)
AC_ARG_WITH(ruby-embedded,
	[  --with-ruby-embedded       use ruby-embedded],
	[use_ruby=$withval],
	[use_ruby=no])
AC_MSG_RESULT($use_ruby)

if test "$use_ruby" = "yes"; then
	TWOCAN_CONF_MSG(Checking for ruby-embedded package specific)

	AC_MSG_CHECKING(--with-ruby-command argument)

	AC_ARG_WITH(ruby-command, [  --with-ruby-command=RUBY   name of the Ruby command (default: ruby)],
		RUBY_CMD="$withval"; cv_path_ruby="$withval"; AC_MSG_RESULT($RUBY_CMD),
		RUBY_CMD="ruby"; AC_MSG_RESULT(defaulting to $RUBY_CMD))

	AC_PATH_PROG(cv_path_ruby, $RUBY_CMD)

	if test "X$cv_path_ruby" = "X"; then
		AC_MSG_ERROR(not found; disabling Ruby)
	fi

	AC_MSG_CHECKING(Ruby version)

	if `$cv_path_ruby -e 'exit 1 if (VERSION rescue RUBY_VERSION) >= "1.8.0"'`; then
		AC_MSG_ERROR(Your Ruby is too old! At least Ruby-1.8.0 is required)
	else
		AC_MSG_RESULT(OK)
	fi

	USE_RUBY=yes
	AC_DEFINE(USE_RUBY, 1, [Define if enable ruby support])

	AC_MSG_CHECKING(Ruby rbconfig)

	ruby_rbconfig="RbConfig"

	if ! $cv_path_ruby -r rbconfig -e 'RbConfig' >/dev/null 2>/dev/null; then
		ruby_rbconfig="Config"
	fi

	AC_MSG_RESULT($ruby_rbconfig)
	AC_MSG_CHECKING(Ruby header files)

	dnl Locate "ruby.h", "intern.h"
	rubyhdrdir=`$cv_path_ruby -r mkmf -e "print $ruby_rbconfig::CONFIG[['rubyhdrdir']] || $ruby_rbconfig::CONFIG[['archdir']] || \\$hdrdir" 2>/dev/null`

	if test "X$rubyhdrdir" != "X"; then
		AC_MSG_RESULT($rubyhdrdir)

		rubyarch=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig::CONFIG[['arch']]"`

		if ! test -d "$rubyhdrdir/$rubyarch"; then
			RUBYCONFIGINCLUDES="-I$rubyhdrdir"
		else
			RUBYCONFIGINCLUDES="-I$rubyhdrdir -I$rubyhdrdir/$rubyarch"
		fi

		AC_SUBST(RUBYCONFIGINCLUDES)

		libruby=`$cv_path_ruby -r rbconfig -e "puts $ruby_rbconfig::CONFIG[['LIBRUBY_SO']]"`

		rubyversion=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig::CONFIG[['ruby_version']].gsub(/\./, '')[[0,2]]"`

		if test "X$rubyversion" = "X"; then
			rubyversion=`$cv_path_ruby -e "print ((VERSION rescue RUBY_VERSION)).gsub(/\./, '')[[0,2]]"`
		fi

		RUBYCPPFLAGS="-DRUBY_VERSION=$rubyversion -DDYNAMIC_RUBY_DLL=\\\"$libruby\\\" -DDYNAMIC_RUBY_VER=$rubyversion"

		AC_SUBST(RUBYCPPFLAGS)

		RUBYCONFIGLDFLAGS=""

		rubyldflags=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig::CONFIG[['LDFLAGS']]"`

		if test "X$rubyldflags" != "X"; then

			dnl Ruby on Mac OS X 10.5 adds "-arch" flags but these should only
			dnl be included if requested by passing --with-mac-arch to
			dnl configure, so strip these flags first (if present)

			rubyldflags=`echo "$rubyldflags" | sed -e 's/-arch\ ppc//' -e 's/-arch\ i386//' -e 's/-arch\ x86_64//'`

			if test "X$rubyldflags" != "X"; then
				if test "X`echo \"$LDFLAGS\" | grep -F -e \"$rubyldflags\"`" = "X"; then
					RUBYCONFIGLDFLAGS="$rubyldflags"
				fi
			fi
		fi

		AC_SUBST(RUBYCONFIGLDFLAGS)

		dnl Locate libruby-static.a + extra libraries (libcrypt)
		dnl rubylibs=`$ruby_path -r mkmf -e 'c=Config::CONFIG; libs=c[["libdir"]]+"/"+c[["LIBRUBY"]]+" "+c[["LIBS"]]; print libs'`

		RUBYCONFIGLIBS=""

		rubylibs=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig::CONFIG[['LIBS']]"`

		if test "X$rubylibs" != "X"; then
			RUBYCONFIGLIBS="$rubylibs"
		fi

		librubya=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig.expand($ruby_rbconfig::CONFIG[['LIBRUBY_A']])"`

		rubylibdir=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig.expand($ruby_rbconfig::CONFIG[['libdir']])"`

		librubyarg=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig.expand($ruby_rbconfig::CONFIG[['LIBRUBYARG']])"`

		if test -f "$rubylibdir/$librubya"; then
			librubyarg="$librubyarg"
			RUBYCONFIGLIBS="$RUBYCONFIGLIBS -L$rubylibdir"
		elif test "$librubyarg" = "libruby.a"; then
			dnl Required on Mac OS 10.3 where libruby.a doesn't exist
			librubyarg="-lruby"
			RUBYCONFIGLIBS="$RUBYCONFIGLIBS -L$rubylibdir"
		fi

		if test "X$librubyarg" != "X"; then
			RUBYCONFIGLIBS="$librubyarg $RUBYCONFIGLIBS"
		fi

		AC_SUBST(RUBYCONFIGLIBS)
	fi
fi
# ---------------------------------------------------------------------------------------------------

AM_CONDITIONAL(SSL,			test "$USE_LIBSSL"		= "yes")
AM_CONDITIONAL(SSL_TS,		test "$HAVE_SSL_TS"		= "yes")
AM_CONDITIONAL(SSH,			test "$USE_LIBSSH"		= "yes")
AM_CONDITIONAL(PCRE,			test "$USE_LIBPCRE"		= "yes")
AM_CONDITIONAL(LDAP,			test "$USE_LIBLDAP"		= "yes")
AM_CONDITIONAL(CURL,			test "$USE_LIBCURL"		= "yes")
AM_CONDITIONAL(LIBZ,			test "$USE_LIBZ"			= "yes")
AM_CONDITIONAL(EXPAT,		test "$USE_LIBEXPAT"		= "yes")
AM_CONDITIONAL(MAGIC,		test "$USE_LIBMAGIC"		= "yes")
AM_CONDITIONAL(DBI,			test "$USE_LIBDBI"		= "yes")
AM_CONDITIONAL(LIBEVENT,	test "$USE_LIBEVENT"		= "yes")
AM_CONDITIONAL(LIBXML2,		test "$USE_LIBXML2"		= "yes")
AM_CONDITIONAL(PAGE_SPEED, test "$USE_PAGE_SPEED"	= "yes")
AM_CONDITIONAL(V8,			test "$USE_LIBV8"			= "yes")
AM_CONDITIONAL(PHP,			test "$USE_PHP"			= "yes")
AM_CONDITIONAL(RUBY,			test "$USE_RUBY"			= "yes")
AM_CONDITIONAL(CARES,		test "$USE_C_ARES"		= "yes")

AC_DEFINE_UNQUOTED(_FLEX_VERSION,		 "$flex_version",			[Flex version])
AC_DEFINE_UNQUOTED(_BISON_VERSION,		 "$bison_version",		[Bison version])
AC_DEFINE_UNQUOTED(_PCRE_VERSION,		 "$pcre_version",			[PCRE - Perl compatible regular expressions version])
AC_DEFINE_UNQUOTED(_LDAP_VERSION,		 "$ldap_version",			[Ldap version])
AC_DEFINE_UNQUOTED(_EXPAT_VERSION,		 "$expat_version",		[Expat version])
AC_DEFINE_UNQUOTED(_LIBZ_VERSION,		 "$libz_version",			[libz - general purpose compression library version])
AC_DEFINE_UNQUOTED(_LIBZOPFLI_VERSION,	 "$libzopfli_version",	[libzopfli - google compression library version])
AC_DEFINE_UNQUOTED(_LIBSSH_VERSION,		 "$libssh_version",		[libSSH version])
AC_DEFINE_UNQUOTED(_SSL_VERSION,			 "$ssl_version",			[SSL version])
AC_DEFINE_UNQUOTED(_CURL_VERSION,		 "$libcurl_version",		[libcURL - multiprotocol file transfer library version])
AC_DEFINE_UNQUOTED(_MAGIC_VERSION,		 "$libmagic_version",	[libmagic - magic number recognition library version])
AC_DEFINE_UNQUOTED(_SQLITE_VERSION,		 "$libsqlite_version",	[libsqlite - SQLITE Database System library version])
AC_DEFINE_UNQUOTED(_MYSQL_VERSION,		 "$libmysql_version",	[libmysql - MySQL Database System library version])
AC_DEFINE_UNQUOTED(_PGSQL_VERSION,		 "$pgsql_version",		[version of PostgreSQL database support])
AC_DEFINE_UNQUOTED(_DBI_VERSION,			 "$libdbi_version",		[libdbi - Database Independent Abstraction Layer version])
AC_DEFINE_UNQUOTED(_LIBUUID_VERSION,	 "$libuuid_version",		[libuuid - version library to create a new UUID value])
AC_DEFINE_UNQUOTED(_LIBEVENT_VERSION,	 "$libevent_version",	[libevent - event notification library version])
AC_DEFINE_UNQUOTED(_LIBXML2_VERSION,	 "$libxml2_version",		[libxml2 - version 2 of the library to manipulate XML files])
AC_DEFINE_UNQUOTED(_PAGE_SPEED_VERSION, "$page_speed_version", [PageSpeed - version of Google page speed SDK])
AC_DEFINE_UNQUOTED(_V8_VERSION,			 "$v8_version",			[V8 - version of Google V8 JavaScript Engine])
AC_DEFINE_UNQUOTED(_PHP_VERSION,			 "$php_version",			[version of PHP language runtime engine embed SAPIs])
AC_DEFINE_UNQUOTED(_RUBY_VERSION,		 "$rubyversion",			[version of RUBY language runtime engine embed])
AC_DEFINE_UNQUOTED(_C_ARES_VERSION,		 "$cares_version",		[version of c-ares library for asynchronous DNS requests])

# SECTION 5: Checks for header files

# SECTION 6: Checks for typedefs

AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

AC_DEFINE_UNQUOTED([RETSIGTYPE],[void],[Define as the return type of signal handlers (`int' or `void').])


# SECTION 7: Checks for structures

TWOCAN_CONF_MSG(Checking for structures)

AC_STRUCT_TM
AC_HEADER_STAT
AC_STRUCT_ST_BLOCKS
AC_CHECK_TYPE([struct iphdr],[AC_DEFINE(HAVE_IPHDR,1,[struct iphdr needed for IPv6 support])],[],[#include <netinet/ip.h>])
AC_CHECK_MEMBER([struct dirent.d_reclen],AC_DEFINE(DIRENT_HAVE_D_RECLEN,1,[Define if you have d_reclen in struct dirent]),[],[ #include <dirent.h> ])
AC_CHECK_MEMBER([struct dirent.d_namlen],AC_DEFINE(DIRENT_HAVE_D_NAMLEN,1,[Define if you have d_namlen in struct dirent]),[],[ #include <dirent.h> ])
AC_CHECK_MEMBER([struct tm.tm_gmtoff],AC_DEFINE(TM_HAVE_TM_GMTOFF,1,[Define if you have tm_gmtoff in struct tm]),[],[ #include <time.h> ])

TWOCAN_CONF_MSG(Checking for types, and defing if needed)

# For miniLZO
AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_TYPE(ptrdiff_t,long)
AC_CHECK_SIZEOF(unsigned char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(time_t)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(__int64)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(ptrdiff_t)
AC_CHECK_TYPE([cpu_set_t], [AC_DEFINE_UNQUOTED(HAVE_CPU_SET_T,1,[Define if exist type cpu_set_t])],[],[#include <sched.h>])

if test "$ac_cv_sizeof_void_p" = "8"; then
	AC_DEFINE(HAVE_ARCH64, 1, [Define if we are on 64 bit architecture])
fi

# SECTION 8: Checks for variables

# SECTION 9: Checks for compiler characteristics

# SECTION 10: Checks for library functions

# SECTION 11: Checks for function characteristics

# SECTION 12: Checks for type characteristics

# SECTION 13: Checks for system services

if test "$cross_compiling" != "yes"; then
	AM_SOCKET_SO_RCVTIMEO
	if test "$ac_cv_func_fallocate" = "yes" && test "$ac_cv_func_fallocate64" = "yes"; then
		AC_TRY_FALLOCATE
	fi
fi

# SECTION 14: Checks for aggregated features

# UServer plugin

AM_CONDITIONAL(PLUGIN, test "$enable_shared"	= "yes") # NB: if not don't build .so object...

modules="rpc shib stream nocat socket scgi fcgi geoip proxy soap ssi tsa http echo"

if test "$USE_LIBPCRE" = "yes"; then
	ulib_pcre_msg="yes ( $pcre_version )"
else
	modules=`echo $modules | sed s/proxy//`
fi

if test "$USE_LIBCURL" = "yes"; then
	ulib_curl_msg="yes ( $libcurl_version )"
fi

if test "$USE_LIBLDAP" = "yes"; then
	ulib_ldap_msg="yes ( $ldap_version )"
fi

if test "$USE_LIBEXPAT" = "yes"; then
	ulib_expat_msg="yes ( $expat_version )"
else
	modules=`echo $modules | sed s/soap//`
fi

if test "$USE_LIBMAGIC" = "yes"; then
	ulib_magic_msg="yes ( $libmagic_version )"
fi

if test "$USE_LIBSSL" = "yes"; then
   ulib_ssl_msg="yes ( $ssl_version )"
fi

if test "$USE_LIBSSH" = "yes"; then
   ulib_ssh_msg="yes ( $libssh_version )"
fi

if test "$USE_SQLITE" = "yes"; then
   ulib_sqlite_msg="yes ( $libsqlite_version )"
fi

if test "$USE_MYSQL" = "yes"; then
   ulib_mysql_msg="yes ( $libmysql_version )"
fi

if test "$USE_PGSQL" = "yes"; then
   ulib_pgsql_msg="yes ( $pgsql_version )"
fi

if test "$USE_LIBDBI" = "yes"; then
   ulib_dbi_msg="yes ( $libdbi_version )"
fi

if test "$USE_LIBUUID" = "yes"; then
   ulib_libuuid_msg="yes ( $libuuid_version )"
fi

if test "$USE_LIBEVENT" = "yes"; then
   ulib_libevent_msg="yes ( $libevent_version )"
fi

if test "$USE_LIBXML2" = "yes"; then
   ulib_libxml2_msg="yes ( $libxml2_version )"
fi

if test "$USE_PAGE_SPEED" = "yes"; then
   ulib_pagespeed_msg="yes ( $page_speed_version )"
fi

if test "$USE_LIBV8" = "yes"; then
   ulib_v8_msg="yes ( $v8_version )"
fi

if test "$USE_PHP" = "yes"; then
   ulib_php_msg="yes ( $php_version )"
fi

if test "$USE_RUBY" = "yes"; then
   ulib_ruby_msg="yes ( $rubyversion )"
fi

AM_CONDITIONAL(GEOIP, true)
AM_CONDITIONAL(MOD_SHIB, true)

if test "$ac_cv_lib_GeoIP_GeoIP_id_by_ipnum" != "yes"; then
	AM_CONDITIONAL(GEOIP, false)
	modules=`echo $modules | sed s/geoip//`
fi

# NB: This is the only version of gcc compiler that work with...
if test "$gcc_version" != "4.2.2" || test "$enable_debug" != "yes"; then
	AM_CONDITIONAL(MOD_SHIB, false)
	modules=`echo $modules | sed s/shib//`
fi

# UServer plugin

TWOCAN_CONF_MSG(Checking for server plugin)

add_calls1=""
use_static_module=""
static_handler_rcp=""
static_handler_shib=""
static_handler_stream=""
static_handler_socket=""
static_handler_scgi=""
static_handler_fcgi=""
static_handler_geoip=""
static_handler_proxy=""
static_handler_soap=""
static_handler_ssi=""
static_handler_tsa=""
static_handler_nocat=""
static_handler_http=""
static_handler_echo=""

AC_ARG_ENABLE(static-server-plugin,
				  AS_HELP_STRING([--enable-static-server-plugin=MODULES],[ rpc shib stream socket scgi fcgi geoip proxy soap ssi tsa nocat http echo ]),
				  [use_static_module="$enableval"])

for mod in $modules; do
	AC_MSG_CHECKING([server plugin "$mod"])

	if echo $use_static_module | grep -w $mod >/dev/null || echo $use_static_module | grep all >/dev/null; then

		add_calls1="$add_calls1 \n loadStaticLinkedModules(\"$mod\");"

		if test "$mod" = "rpc"; then
			static_handler_rpc="yes"
			AC_DEFINE(U_STATIC_HANDLER_RPC,, [STATIC_HANDLER_RPC])
		elif test "$mod" = "shib"; then
			static_handler_shib="yes"
			AC_DEFINE(U_STATIC_HANDLER_SHIB,, [STATIC_HANDLER_SHIB])
         ULIB_LIBS="$ULIB_LIBS -llog4cpp -lshib -lshib-target -lsaml -lxml-security-c -lxerces-c"
		elif test "$mod" = "stream"; then
			static_handler_stream="yes"
			AC_DEFINE(U_STATIC_HANDLER_STREAM,, [STATIC_HANDLER_STREAM])
		elif test "$mod" = "socket"; then
			static_handler_socket="yes"
			AC_DEFINE(U_STATIC_HANDLER_SOCKET,, [STATIC_HANDLER_SOCKET])
		elif test "$mod" = "scgi"; then
			static_handler_scgi="yes"
			AC_DEFINE(U_STATIC_HANDLER_SCGI,, [STATIC_HANDLER_SCGI])
		elif test "$mod" = "fcgi"; then
			static_handler_fcgi="yes"
			AC_DEFINE(U_STATIC_HANDLER_FCGI,, [STATIC_HANDLER_FCGI])
		elif test "$mod" = "geoip"; then
			static_handler_geoip="yes"
         ULIB_LIBS="$ULIB_LIBS -lGeoIP"
			AC_DEFINE(U_STATIC_HANDLER_GEOIP,, [STATIC_HANDLER_GEOIP])
		elif test "$mod" = "proxy"; then
			static_handler_proxy="yes"
			AC_DEFINE(U_STATIC_HANDLER_PROXY,, [STATIC_HANDLER_PROXY])
		elif test "$mod" = "soap"; then
			static_handler_soap="yes"
			AC_DEFINE(U_STATIC_HANDLER_SOAP,, [STATIC_HANDLER_SOAP])
		elif test "$mod" = "ssi"; then
			static_handler_ssi="yes"
			AC_DEFINE(U_STATIC_HANDLER_SSI,, [STATIC_HANDLER_SSI])
		elif test "$mod" = "tsa"; then
			static_handler_tsa="yes"
			AC_DEFINE(U_STATIC_HANDLER_TSA,, [STATIC_HANDLER_TSA])
		elif test "$mod" = "nocat"; then
			static_handler_nocat="yes"
			AC_DEFINE(U_STATIC_HANDLER_NOCAT,, [STATIC_HANDLER_NOCAT])
		elif test "$mod" = "http"; then
			static_handler_http="yes"
			AC_DEFINE(U_STATIC_HANDLER_HTTP,, [STATIC_HANDLER_HTTP])
		elif test "$mod" = "echo"; then
			static_handler_echo="yes"
			AC_DEFINE(U_STATIC_HANDLER_ECHO,, [STATIC_HANDLER_ECHO])
		fi

	   AC_MSG_RESULT([static])
	else
	   AC_MSG_RESULT([dynamic])
	fi
done

AC_MSG_CHECKING([loader.autoconf files])
printf "$add_calls1\n" > src/ulib/net/server/plugin/loader.autoconf.cpp
AC_MSG_RESULT([done])

AM_CONDITIONAL(STATIC_HANDLER_RPC,    test "$static_handler_rpc"	  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_SHIB,	  test "$static_handler_shib"	  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_STREAM, test "$static_handler_stream" = "yes")
AM_CONDITIONAL(STATIC_HANDLER_SOCKET, test "$static_handler_socket" = "yes")
AM_CONDITIONAL(STATIC_HANDLER_SCGI,	  test "$static_handler_scgi"	  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_FCGI,	  test "$static_handler_fcgi"	  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_GEOIP,  test "$static_handler_geoip"  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_PROXY,  test "$static_handler_proxy"  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_SOAP,   test "$static_handler_soap"   = "yes")
AM_CONDITIONAL(STATIC_HANDLER_SSI,    test "$static_handler_ssi"    = "yes")
AM_CONDITIONAL(STATIC_HANDLER_TSA,    test "$static_handler_tsa"    = "yes")
AM_CONDITIONAL(STATIC_HANDLER_NOCAT,  test "$static_handler_nocat"  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_HTTP,   test "$static_handler_http"	  = "yes")
AM_CONDITIONAL(STATIC_HANDLER_ECHO,	  test "$static_handler_echo"   = "yes")

# ORM driver

TWOCAN_CONF_MSG(Checking for ORM driver)

drivers="sqlite mysql pgsql"

if test "$USE_SQLITE" != "yes"; then
	drivers=`echo $drivers | sed s/sqlite//`
fi

if test "$USE_MYSQL" != "yes"; then
	drivers=`echo $drivers | sed s/mysql//`
fi

if test "$USE_PGSQL" != "yes"; then
	drivers=`echo $drivers | sed s/pgsql//`
fi

add_calls2=""
use_static_driver=""
static_handler_mysql=""
static_handler_sqlite=""
static_handler_pgsql=""

AC_ARG_ENABLE(static-orm-driver, AS_HELP_STRING([--enable-static-orm-driver=DRIVERS],[ sqlite mysql pgsql ]),
				  [use_static_driver="$enableval"])

for drv in $drivers; do
	AC_MSG_CHECKING([ORM driver "$drv"])

	if echo $use_static_driver | grep -w $drv >/dev/null || echo $use_static_driver | grep all >/dev/null; then

		add_calls2="$add_calls2 \n loadStaticLinkedModules(\"$drv\");"

		if test "$drv" = "sqlite"; then
			static_handler_sqlite="yes"
         ULIB_LIBS="$ULIB_LIBS $SQLITE3_LIBS"
			LDFLAGS="$SQLITE3_LDFLAGS $LDFLAGS"
			CPPFLAGS="$SQLITE3_INCLUDE $CPPFLAGS"
			AC_DEFINE(U_STATIC_ORM_DRIVER_SQLITE,, [STATIC_ORM_DRIVER_SQLITE])
		elif test "$drv" = "mysql"; then
			static_handler_mysql="yes"
         ULIB_LIBS="$ULIB_LIBS $MYSQL_LIBS"
			LDFLAGS="$MYSQL_LDFLAGS $LDFLAGS"
			CPPFLAGS="$MYSQL_INCLUDE $CPPFLAGS"
			AC_DEFINE(U_STATIC_ORM_DRIVER_MYSQL,, [STATIC_ORM_DRIVER_MYSQL])
		elif test "$drv" = "pgsql"; then
			static_handler_pgsql="yes"
         ULIB_LIBS="$ULIB_LIBS $PGSQL_LIBS"
			LDFLAGS="$PGSQL_LDFLAGS $LDFLAGS"
			CPPFLAGS="$PGSQL_INCLUDE $CPPFLAGS"
			AC_DEFINE(U_STATIC_ORM_DRIVER_PGSQL,, [STATIC_ORM_DRIVER_PGSQL])
		fi

	   AC_MSG_RESULT([static])
	else
	   AC_MSG_RESULT([dynamic])
	fi
done

AC_MSG_CHECKING([loader.autoconf files])
printf "$add_calls2\n" > src/ulib/orm/driver/loader.autoconf.cpp
AC_MSG_RESULT([done])

AM_CONDITIONAL(STATIC_ORM_DRIVER_SQLITE, test "$static_handler_sqlite" = "yes")
AM_CONDITIONAL(STATIC_ORM_DRIVER_MYSQL,  test "$static_handler_mysql"  = "yes")
AM_CONDITIONAL(STATIC_ORM_DRIVER_PGSQL,  test "$static_handler_pgsql"  = "yes")

# SERVLET

static_servlet=""
static_servlet_wi_auth=""

if test "$enable_static" = "yes" && test "$enable_shared" = "no"; then

	use_static_servlet=""

	TWOCAN_CONF_MSG(Checking for static HTTP servlet)

	AC_ARG_ENABLE(static-servlet, AS_HELP_STRING([--enable-static-servlet=NAMES],[ wi_auth ... ]),
					  [use_static_servlet="$enableval"])

	if test -n "$use_static_servlet" ; then

		add_calls3=""
		add_calls4=""
		add_calls5=""

		static_servlet_make=""
		dir=net/server/plugin/usp

		for srv in $use_static_servlet; do

			AC_MSG_CHECKING([servlet "$srv"])

			add_calls3="$add_calls3 \n extern \"C\" { int runDynamicPage_${srv}(UClientImage_Base*); }"
			add_calls4="$add_calls4 \n loadStaticLinkedServlet(\"${srv}\",U_CONSTANT_SIZE(\"${srv}\"),(iPFpv)::runDynamicPage_${srv});"

			if test "$srv" = "wi_auth"; then
				static_servlet_wi_auth="yes"
				AC_DEFINE(U_STATIC_SERVLET_WI_AUTH,, [STATIC_SERVLET_WI_AUTH])
			else
				static_servlet="yes"
				AC_DEFINE(U_STATIC_SERVLET,, [STATIC_SERVLET])
				add_calls5="$add_calls5 \n #include \"net/server/plugin/usp/${srv}.cpp\""
				static_servlet_make="${static_servlet_make}\n\n${dir}/${srv}.cpp: ${dir}/${srv}.usp ${dir}/usp_translator"
			fi

			AC_MSG_RESULT([static])
		done
 
		if test -n "$static_servlet_make" ; then

			STATIC_SERVLET_MAKE=`printf "$static_servlet_make\n\n"`

			AC_SUBST(STATIC_SERVLET_MAKE)

			static_servlet_make=""
		fi

		printf "$add_calls3\n" > src/ulib/net/server/plugin/usp/loader.autoconf.ext
		printf "$add_calls4\n" > src/ulib/net/server/plugin/usp/loader.autoconf.cpp
		printf "$add_calls5\n" > src/ulib/net/server/plugin/usp/loader.autoconf.inc
	fi

	AC_MSG_CHECKING([loader.autoconf files])
	AC_MSG_RESULT([done])
fi

AM_CONDITIONAL(STATIC_SERVLET,			test "$static_servlet"			 = "yes")
AM_CONDITIONAL(STATIC_SERVLET_WI_AUTH, test "$static_servlet_wi_auth" = "yes")

# SECTION 15: Final checks

AM_CONDITIONAL(HUPS, false)
AM_CONDITIONAL(FINAL, false)
AM_CONDITIONAL(HTTP2, false)
AM_CONDITIONAL(SANITIZE, false)
AM_CONDITIONAL(MEMORY_POOL, false)

if test "$enable_memory_pool" = "yes"; then
   ulib_memp_msg="enabled"
	AM_CONDITIONAL(MEMORY_POOL, true)
fi

if test "$enable_LFS" = "yes"; then
	ulib_LFS_msg="enabled"
	CPPFLAGS="$CPPFLAGS -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
	AC_DEFINE(SIZEOF_OFF_T, 8, [Define if enable LFS support])
else
	AC_CHECK_SIZEOF(off_t)
fi

if test "$enable_final" = "yes"; then
	ulib_final_msg="enabled"
	AM_CONDITIONAL(FINAL, true)
fi

if test "$enable_HUPS" = "yes"; then
	AM_CONDITIONAL(HUPS, true)
fi

if test "$enable_http2" = "yes"; then
	AM_CONDITIONAL(HTTP2, true)
fi

# If gcc compiler add values to *FLAGS...

if test "$ac_cv_c_compiler_gnu" = "yes" -a "x$GCC_IS_CLANG" = xno; then

	TWOCAN_CONF_MSG(Checking for gcc compiler flags)

	AC_CC_TRY_FLAG

	MAYBE_FLAGS="-Wstrict-aliasing=2 -Wall -Wextra -Wsign-compare -Wpointer-arith -Wwrite-strings -Wlogical-op \
					 -Wmissing-declarations -Wpacked -Wswitch-enum -Wmissing-format-attribute -Winit-self -Wformat \
					 -Wformat-extra-args -Wenum-compare"
	# -------------------------------------------------------------------------------------
	# to turn various gcc/glibc-specific preprocessor checks
	# -------------------------------------------------------------------------------------
	# -Wno-format -Wno-format-extra-args
	# -------------------------------------------------------------------------------------

	# -Wcast-qual
	CC_MAYBE_FLAGS="-Werror-implicit-function-declaration -Wstrict-prototypes -Wc++-compat \
						 -Wmissing-prototypes -Wnested-externs -Wdeclaration-after-statement -Wold-style-definition -Wsizeof-pointer-memaccess"

	CXX_MAYBE_FLAGS="-fno-check-new -fno-exceptions -fno-rtti -fno-enforce-eh-specs -Wno-deprecated -Wdelete-non-virtual-dtor"

	case "$gcc_version" in
		2*|3*|4.0*|4.1*|4.2*)
		;;
		4.3*) # openWRT backfire: ... c++/4.3.3/tr1_impl/boost_shared_ptr.h:630: error: cannot use typeid with -fno-rtti
		;;
		4.4*) # centOS 64: ... c++/4.4.7/bits/shared_ptr.h:1005: error: cannot use typeid with -fno-rtti
		;;
		4.5*) # openwrt: SIGSEGV 
		;;
		*)
			    MAYBE_FLAGS="$MAYBE_FLAGS -Wno-unused-result -Wshadow -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn" # -Wsuggest-attribute=const
 		##	CXX_MAYBE_FLAGS="$CXX_MAYBE_FLAGS -std=c++0x -std=c++11" # -std=c++14

			if test "$enable_gcc_optimized" = "yes"; then
				## ----------------------------------------------------------------------------------------------------------------------------------
				## -fgcse Perform a global common subexpression elimination pass. This pass also performs global constant and copy propagation. When
				##			 compiling a program using computed gotos, a GCC extension, you may get better runtime performance if you disable the global
				##			 common subexpression elimination pass by adding -fno-gcse to the command line
				## ----------------------------------------------------------------------------------------------------------------------------------
				MAYBE_FLAGS="$MAYBE_FLAGS -fno-stack-protector -fno-stack-protector-all -fno-math-errno -fno-crossjumping -fno-gcse -Ofast" ## -ffunction-sections -fdata-sections

				if test "$use_distcc" != "yes" ; then
					MAYBE_FLAGS="$MAYBE_FLAGS -flto -ffat-lto-objects -mtune=native" # NB: distcc don't work with this...
				fi
			fi
		;;
	esac

	## -------------------------------------------------------------------------------------
	##	-fno-common
	## -Wno-missing-field-initializers -Wno-attributes -Wno-long-long
	## -------------------------------------------------------------------------------------
	##	-fwhole-program
	## -------------------------------------------------------------------------------------
	## Assume that the current compilation unit represents the whole program being compiled.
	## All public functions and variables with the exception of main and those merged by attribute
	## externally_visible become static functions
	## -------------------------------------------------------------------------------------

	if test "$enable_debug" != "yes"; then
		MAYBE_FLAGS="$MAYBE_FLAGS -Winline -Wno-unused-parameter -Wno-unused-variable"
	else
		## _FORTIFY_SOURCE is enabled by default from g++ 4.7.2
		MAYBE_FLAGS="$MAYBE_FLAGS -Wunsafe-loop-optimizations -Wno-unused-parameter -rdynamic -fdiagnostics-color=auto"
		# -Wzero-as-null-pointer-constant"
		CXX_MAYBE_FLAGS="$CXX_MAYBE_FLAGS -Wc++11-compat"
	fi

	AC_CACHE_CHECK([for gcc supported flags], ulib_cv_flags, [
		echo
		FLAGS=""
		CC_FLAGS=""
		CXX_FLAGS=""

		# -------------------------------------------------------------------------------------------------------------------------
		# Some warning options are not supported by all versions of gcc, so test all desired options against the current compiler.
		# Note that there are some order dependencies here. Specifically, an option that disables a warning will have no net effect
		# if a later option then enables that warnings, (perhaps implicitly). So we put some grouped options (-Wall and -Wextra) up
		# front and the -Wno options last
		# -------------------------------------------------------------------------------------------------------------------------

		# ------------------------------------------------
		# Set the test language as C
		# ------------------------------------------------
		AC_LANG([C])

		for W in $CC_MAYBE_FLAGS; do
			AC_CC_TRY_FLAG([$W], [CC_FLAGS="$CC_FLAGS $W"])
		done
		ulib_cv_CC_flags=$CC_FLAGS
		# ------------------------------------------------

		# ------------------------------------------------
		# reset the test language as C++
		# ------------------------------------------------
		AC_LANG([C++])

		for W in $MAYBE_FLAGS; do
			AC_CC_TRY_FLAG([$W], [FLAGS="$FLAGS $W"])
		done

		ulib_cv_flags="$FLAGS $ulib_cv_flags"

		for W in $CXX_MAYBE_FLAGS; do
			AC_CC_TRY_FLAG([$W], [CXX_FLAGS="$CXX_FLAGS $W"])
		done
		ulib_cv_CXX_flags=$CXX_FLAGS
		# ------------------------------------------------

		AC_MSG_CHECKING([which gcc flags were supported])
	])

	SANITIZE_FLAGS=""
	SANITIZE_THREAD=""
	SANITIZE_ADDRESS=""

	case "$gcc_version" in
		2.9*|3*|4.0*|4.1*|4.2*|4.3*|4.4*|4.5*|4.6*|4.7*|4.8*)
		;;
		*)
		## -------------------------------------------------------------------------------------------------------------------------
		## FATAL: ThreadSanitizer can not mmap the shadow memory (something is mapped at 0x400000 < 0x7cf000000000)
		## echo 2 >/proc/sys/kernel/randomize_va_space
		## This may be fixed in future kernels, see https://bugzilla.kernel.org/show_bug.cgi?id=66721 
		## -------------------------------------------------------------------------------------------------------------------------
		##	if test "$enable_debug" = "yes"; then
		##		AC_CC_TRY_FLAG(["-fsanitize=thread"], [SANITIZE_THREAD="-fsanitize=thread"])
		##		if test "$SANITIZE_THREAD" != ""; then
      ##			ULIB_LIBS="$ULIB_LIBS -ltsan"
		##			SANITIZE_FLAGS="-fPIE -pie"
		##		fi
		## -------------------------------------------------------------------------------------------------------------------------
	   ##		AC_CC_TRY_FLAG(["-fsanitize=address"], [SANITIZE_ADDRESS="-fsanitize=address"])
		##		if test "$SANITIZE_ADDRESS" != ""; then
      ##			ULIB_LIBS="$ULIB_LIBS -lasan"
		##			SANITIZE_FLAGS="-fPIE -pie"
		##		fi
		##	fi
		## -------------------------------------------------------------------------------------------------------------------------

		AC_MSG_CHECKING([sanitize support request])
		if echo $CPPFLAGS | grep fsanitize >/dev/null; then
			AC_MSG_RESULT(yes)
			AM_CONDITIONAL(SANITIZE, true)
		else
			AC_MSG_RESULT(no)
		fi

		# When using a linker plugin, compiling with the -flto option now generates slim object files (.o) which only contain intermediate
		# language representation for LTO. Use -ffat-lto-objects to create files which contain additionally the object code. To generate
		# static libraries suitable for LTO processing, use gcc-ar and gcc-ranlib; to list symbols from a slim object file use gcc-nm.
		# (This requires that ar, ranlib and nm have been compiled with plugin support)

		if test "$cross_compiling" != "yes"; then
			AR=gcc-ar
			NM=gcc-nm
			RANLIB=gcc-ranlib
		fi
		;;
	esac

	CFLAGS="$CFLAGS $ulib_cv_CC_flags"
	CXXFLAGS="$CXXFLAGS $ulib_cv_CXX_flags"
	# NB: "-g -O2" it is already present in configure script...
	CPPFLAGS="$CPPFLAGS -pipe $SANITIZE_FLAGS $ulib_cv_flags $SANITIZE_ADDRESS $SANITIZE_THREAD"
fi

# Cpp

AC_PATH_PROG(cpp, cpp, no)
if test "$cpp" != "no"; then
	AC_DEFINE(HAVE_CPP, 1, [enable cpp preprocessor features])
fi
AC_PATH_PROG(mcpp, mcpp, no)
if test "$mcpp" != "no"; then
	AC_DEFINE(HAVE_MCPP, 1, [enable mcpp preprocessor features])
fi

# atomic

AC_CHECK_HEADER([atomic],[],[AC_MSG_RESULT(C++ atomic not found)],[AC_INCLUDES_DEFAULT])
if test "$ac_cv_header_atomic" = "yes"; then
	AC_DEFINE(HAVE_ATOMIC_H, [1], [C++ atomic header])
fi

# Manual

AC_PATH_PROG(DOXYGEN, doxygen, no)
AM_CONDITIONAL(DOXY, test "$DOXYGEN" != "no")
#AC_SUBST(DOXYGEN)

# Sostituzioni

case "$target_os" in
	*bsd*)
		if test "$enable_stdcpp" != "yes"; then
			  if test "$GCC" = "yes" -a "$ULIB_LIBC" = "-lc" ; then
				ULIB_LIBC="-Wl,-lc"
			elif test "$GCC" = "yes" -a "$ULIB_LIBC" = "-lc_r" ; then
				ULIB_LIBC="-Wl,-lc_r"
			fi
		fi
    ;;
	osf*)
		CPPFLAGS="$CPPFLAGS -D_POSIX_C_SOURCE=1 -D_OSF_SOURCE=1 -D__USE_STD_IOSTREAM"
	;;
	cygwin*|mingw*)
		ULIB_LIBS="-no-undefined $ULIB_LIBS"
	;;
esac

AC_CHECK_LIB($clib, dlopen,,[
    AC_CHECK_LIB(dl, dlopen, [rlib="$rlib -ldl"],[
        AC_CHECK_LIB(compat, dlopen, [rlib="$rlib -lcompat"])
        AC_CHECK_LIB(dld, shl_load,[
            AC_DEFINE(HAVE_SHL_LOAD, [1], [alternate dyn loader])
            rlib="$rlib -ldld"
        ])
    ])
])

if test -n "$socket_lib"; then
   ULIB_LIBS="$ULIB_LIBS $socket_lib";
fi
if test -n "$posix4_lib"; then
   ULIB_LIBS="$ULIB_LIBS $posix4_lib";
fi
if test -n "$plib"; then
   ULIB_LIBS="$ULIB_LIBS $plib";
fi
if test -n "$rlib"; then
   ULIB_LIBS="$ULIB_LIBS $rlib";
fi
 
ULIB_LIBS="$ULIB_LIBS $ULIB_LIBC"

if test "$enable_stdcpp" != "yes"; then
	AC_CHECK_LIB(Systemstubs, printf, [ ULIB_LIBS="$ULIB_LIBS -lSystemstubs -lgcc" ])
fi

# Check for availablity of "old style" C++ strstream header
AC_CHECK_HEADERS([strstream.h])

# Determine kind of C++ iostream support
AC_CXX_OLD_IOSTREAM

AC_DEFINE_UNQUOTED(ULIB_VERSION, "$PACKAGE_VERSION", [ULib version])

TWOCAN_CONF_MSG(Output Substitution)

ULIB_VERSION="$PACKAGE_VERSION"

AC_SUBST(ULIBS)
AC_SUBST(ULIB_LIBS)
AC_SUBST(ULIB_VERSION)

AC_SUBST(USP_LIBS)
AC_SUBST(USP_FLAGS)
AC_SUBST(USP_LDFLAGS)

AC_DEFINE_UNQUOTED(CXX_VAR, 		"$CXX", [Compiler])
AC_DEFINE_UNQUOTED(LIBS_VAR, 		"$ULIB_LIBS", [Libraries])
AC_DEFINE_UNQUOTED(CFLAGS_VAR,	"$CFLAGS", [C Flags])
AC_DEFINE_UNQUOTED(LDFLAGS_VAR, 	"$LDFLAGS", [Linker Flags])
AC_DEFINE_UNQUOTED(CXXFLAGS_VAR, "$CXXFLAGS",[C++ Flags])
AC_DEFINE_UNQUOTED(CPPFLAGS_VAR, "$CPPFLAGS",[Preprocessor Flags])

AC_DEFINE_UNQUOTED(LD_VAR, 		"$LD", [Linker])
AC_DEFINE_UNQUOTED(PLATFORM_VAR, "$PLATFORM", [Platform])

# rpm spec
RPM_REQUIRE=rpmpkgreq.lst
AC_SUBST_FILE([RPM_REQUIRE])
RPM_CONFIGURE="--enable-static --enable-shared"
AC_SUBST([RPM_CONFIGURE])

# Collect building environment
AC_COMPILATION_ENVIRONMENT

if test "$prefix" = "NONE"; then
	prefixdir="$ac_default_prefix"
else
   prefixdir="$prefix"
fi

pkglibdir="$prefixdir/lib"
sysconfdir="$prefixdir/etc/ulib"
pkglibexecdir="$prefixdir/libexec/ulib"

# Prefix dir
AC_DEFINE_UNQUOTED(U_PREFIXDIR,	"$prefixdir", [install directory])
# Plugin dir
AC_DEFINE_UNQUOTED(U_LIBEXECDIR, "$pkglibexecdir", [install directory for plugins])
# Config dir
AC_DEFINE_UNQUOTED(U_SYSCONFDIR, "$sysconfdir", [install directory for configuration data])

AC_MSG_NOTICE([Configured to build src/ulib/lib${ULIB}:
------------------------------------------------------------------------
          ULib version: ${ULIB_VERSION}
            Build ULib: Shared=${enable_shared}, Static=${enable_static}

            Host setup: ${host}
        Install prefix: ${prefixdir}
        Install plugin: ${pkglibexecdir}
    Configuration data: ${sysconfdir}

      Operating System: $os_version
          C++ Compiler: ${CXX} ( $gcc_version )
                Linker: ${LD} ( $ld_version )
  Standard C   library: ${libc_version}
  Standard C++ library: ${stdgpp_version}
             Libraries: ${ULIB_LIBS}

             C   Flags: ${CFLAGS}
             C++ Flags: ${CXXFLAGS}
          Linker Flags: ${LDFLAGS}
    Preprocessor Flags: ${CPPFLAGS}

         debug support: ${ulib_dbg_msg}
      final build mode: ${ulib_final_msg}
   memory pool support: ${ulib_memp_msg}
           LFS support: ${ulib_LFS_msg}
          ipv6 support: ${ulib_ipv6_msg}
           zip support: ${ulib_zip_msg}
        thread support: ${ulib_thread_msg}

          LIBZ support: ${ulib_libz_msg}
     LIBZOPFLI support: ${ulib_libzopfli_msg}
          PCRE support: ${ulib_pcre_msg}
           SSL support: ${ulib_ssl_msg}
           SSH support: ${ulib_ssh_msg}
          LDAP support: ${ulib_ldap_msg}
          cURL support: ${ulib_curl_msg}
           XML support: ${ulib_expat_msg}
         MAGIC support: ${ulib_magic_msg}
        SQLite support: ${ulib_sqlite_msg}
         MySQL support: ${ulib_mysql_msg}
         PgSQL support: ${ulib_pgsql_msg}
           DBI support: ${ulib_dbi_msg}
       libuuid support: ${ulib_libuuid_msg}
      libevent support: ${ulib_libevent_msg}
       libxml2 support: ${ulib_libxml2_msg}
        c-ares support: ${ulib_cares_msg}
    Page-Speed support: ${ulib_pagespeed_msg}
 V8 JavaScript support: ${ulib_v8_msg}
  PHP language support: ${ulib_php_msg}
 RUBY language support: ${ulib_ruby_msg}

      Examples:         ${ulib_examples}
      LEX/YACC support: ${ulib_parser_msg}
      Lexical analyzer: ${LEX} ( ${flex_version} )
      Parser generator: ${YACC} ( ${bison_version} )

See include/ulib/internal/config.h for further configuration information
------------------------------------------------------------------------])
#      Built-in manual: ${ulib_manual_msg}

# SECTION 16: `AC_CONFIG_FILES([FILE...])

AC_OUTPUT([Makefile \
			 rpm.sh \
			 ULib.spec \
			 m4/Makefile \
			 doc/Makefile \
			 include/Makefile \
			 contrib/Makefile \
			 src/ulib/Makefile \
			 src/ulib/ULib.rc \
			 src/ulib/ULib.pc \
			 src/ulib/orm/driver/Makefile \
			 src/ulib/net/server/plugin/Makefile \
			 src/ulib/net/server/plugin/v8/Makefile \
			 src/ulib/net/server/plugin/php/Makefile \
			 src/ulib/net/server/plugin/ruby/Makefile \
			 src/ulib/net/server/plugin/usp/Makefile \
			 src/ulib/net/server/plugin/mod_shib/Makefile \
			 src/ulib/net/server/plugin/mod_geoip/Makefile \
			 src/ulib/net/server/plugin/usp/usp_compile.sh \
			 src/ulib/net/server/plugin/page_speed/Makefile \
			 include/ulib/Makefile \
			 contrib/HCSP/Makefile \
			 contrib/RSIGN/Makefile \
			 contrib/signer/Makefile \
			 examples/Makefile \
			 examples/IR/Makefile \
			 examples/csp/Makefile \
			 examples/lcsp/Makefile \
			 examples/rsign/Makefile \
			 examples/WiAuth/Makefile \
			 examples/xml2txt/Makefile \
			 examples/uclient/Makefile \
			 examples/userver/Makefile \
			 examples/PEC_log/Makefile \
			 examples/workflow/Makefile \
			 examples/doc_parse/Makefile \
			 examples/lrp_session/Makefile \
			 examples/http_header/Makefile \
			 examples/test_manager/Makefile \
			 examples/doc_classifier/Makefile \
			 examples/form_completion/Makefile \
			 examples/simple_client_server/Makefile \
			 examples/download_accelerator/Makefile \
			 examples/XAdES/Makefile examples/XAdES/XAdES.spec \
			 examples/xml2txt/xml2txt.spec examples/IR/searchengine-bin.spec examples/lcsp/lcsp.spec examples/lcsp/lcsp_rpc.spec \
			 examples/csp/cspclient.spec examples/csp/cspclient_rpc.spec examples/rsign/rsignclient.spec examples/rsign/rsignclient_rpc.spec \
			 examples/userver/web_server.spec examples/userver/wagsmserver.spec examples/workflow/workflow.spec  examples/PEC_log/PEC_log.spec \
			 examples/userver/tsaserver.spec examples/userver/cspserver.spec examples/userver/rsignserver.spec examples/doc_parse/doc_parse.spec \
			 tests/Makefile tests/base/Makefile tests/debug/Makefile tests/ulib/Makefile tests/examples/Makefile tests/contrib/Makefile])

#			 examples/parser/Makefile \
#			 src/ulib/base/Makefile src/ulib/debug/Makefile \
#			 ulib-config ,[chmod a+x ulib-config] )

awk -f configure_log.awk config.log							  > include/ulib/internal/config1.h
awk -f configure_h.awk   include/ulib/internal/config.h > include/ulib/internal/config2.h
cd include/ulib/internal
cat   config2.h config1.h >> config.h
rm -f config2.h config1.h
cd ../../..

# Verify the integrity of the current configuration

echo
echo "***"

CPPFLAGS="-DHAVE_CONFIG_H -I${srcdir}/include $CPPFLAGS"

AC_CACHE_CHECK([if generated configuration is usable], ac_cv_configuration_is_usable,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include "src/ulib/internal/common.cpp"
]], [[int a = 0; a += 1;]])],[ac_cv_configuration_is_usable="yes"],[ac_cv_configuration_is_usable="no"])
])

if test "$ac_cv_configuration_is_usable" = "no" ; then
	# It appears that ulib/internal/common.cpp not compiled. If it didn't compile then there would be no chance that the rest would compile.
	AC_MSG_ERROR([The generated configuration appears to be unusable.
	Please verify that your system path and environment variables are correct.])
	exit 1
fi

echo
echo "You are now ready to build ULib"
echo "Enter the following command: ${T_MD}make && make install${T_ME}"
echo "***"
