# configure.ac -- Process this file with autoconf to produce configure
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
AC_INIT([ULib],[2.4.2],[stefano.casazza@gmail.com],[ULib],[http://stefanocasazza.github.io/ULib/])
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

AC_CANONICAL_TARGET

# Pick a newer and better tar implementation like tar-pax
AM_INIT_AUTOMAKE([subdir-objects no-define tar-pax])
AM_MAINTAINER_MODE
AM_SILENT_RULES([yes])

AC_USE_SYSTEM_EXTENSIONS

dnl This is a C++ library after all
AC_LANG([C++])

dnl Initial CXX, CFLAGS, CXXFLAGS, CPPFLAGS, LDFLAGS, LIBS
AC_MSG_CHECKING(initial CXX)
AC_MSG_RESULT($CXX)
AC_MSG_CHECKING(initial CFLAGS)
AC_MSG_RESULT($CFLAGS)
AC_MSG_CHECKING(initial CXXFLAGS)
AC_MSG_RESULT($CXXFLAGS)
AC_MSG_CHECKING(initial CPPFLAGS)
AC_MSG_RESULT($CPPFLAGS)
AC_MSG_CHECKING(initial LDFLAGS)
AC_MSG_RESULT($LDFLAGS)
AC_MSG_CHECKING(initial LIBS)
AC_MSG_RESULT($LIBS)

m4_include([m4/m4_ax_python_devel.m4])

# Generate a header file with all settings, a config.h file (Automake will add -DHAVE_CONFIG_H)
AC_CONFIG_HEADERS([include/ulib/internal/config.h])

AC_CANONICAL_HOST

# Check what platform we are running on
AC_MSG_CHECKING([which platform we are running on])

local_os_type="unknown"
is_librt_available="yes"

LDFLAGS="$LDFLAGS -L/usr/local/lib"
CXXFLAGS="$CXXFLAGS -I/usr/local/include"

case "$host" in
	*-sco*) AC_MSG_ERROR([We do not support SCO's products!]) ;;

	*-aix*)	  local_os_type="aix" ;;
	*-osf*)	  local_os_type="osf" ;;
	*-irix*)   local_os_type="irix" ;;
	*-hpux*)	  local_os_type="hpux" ;;
	*-linux*)  local_os_type="linux" ;;
	*-cygwin*) local_os_type="cygwin" ;;

	*-darwin*)
		local_os_type="darwin"
		is_librt_available="no"
	;;

	*-netbsd*)
		 local_os_type="netbsd"

		 LDFLAGS="$LDFLAGS -L/usr/local/v6/lib"
	;;

	*-openbsd*)
		 local_os_type="openbsd"

		 LIBS="L/usr/local/lib $LIBS"
	;;

	*mingw*)
		 local_os_type="mingw"
		 is_librt_available="no"

		 CPPFLAGS="$CPPFLAGS -I/mingw/include -L/mingw/lib -D__USE_MINGW_ANSI_STDIO=1"
		 CXXFLAGS="$CXXFLAGS -I/mingw/include -L/mingw/lib"
		 LDFLAGS="$LDFLAGS -L/mingw/lib"
	;;

	*-sunos*)
		 local_os_type="sunos"

		 CXXFLAGS="$CXXFLAGS -pthreads"
		 LDFLAGS="$LDFLAGS -lsocket -lnsl -lresolv"
	;;

	*-solaris*)
		 local_os_type="solaris"

		 CXXFLAGS="$CXXFLAGS -pthreads"
		 LDFLAGS="$LDFLAGS -lsocket -lnsl"
	;;

	*-freebsd*)
		 local_os_type="freebsd"

		 CXXFLAGS="$CXXFLAGS -pthread"
		 LDFLAGS="$LDFLAGS -L/usr/local/v6/lib"
	;;

	*)	local_os_type=unknown ;;
esac

AS_IF([test "x$local_os_type" = xunknown], [AC_MSG_RESULT([unknown])], [test "x$local_os_type" != xunknown], [AC_MSG_RESULT([$local_os_type])])

# Check for architecture for the target
AC_MSG_CHECKING([which architecture the target is])

TARGET_ARCH=
case $target in
	x86_64*)				TARGET_ARCH=x86_64 ;;
	i*86)					TARGET_ARCH=i386 ;;
	sh*)					TARGET_ARCH=sh ;;
	mips*)				TARGET_ARCH=mips ;;
	or1k*)				TARGET_ARCH=openrisc ;;
	microblaze)			TARGET_ARCH=microblaze ;;
	powerpc*|ppc*)		TARGET_ARCH=powerpc ;;
	*netbsd*)			TARGET_ARCH=NetBSD
							enable_stdcpp="yes" ;;
	arm*)					TARGET_ARCH=arm
							AC_DEFINE([HAVE_NO_UNALIGNED_ACCESSES], 1, [Define to 1 if the target don't allows unaligned reads and write]) ;;
	aarch64*)			TARGET_ARCH=arm64
							AC_DEFINE([HAVE_NO_UNALIGNED_ACCESSES], 1, [Define to 1 if the target don't allows unaligned reads and write]) ;;
	*)						TARGET_ARCH=unknown ;;
esac

#AC_SUBST(TARGET_ARCH)
AS_IF([test "x$TARGET_ARCH" = xunknown], [AC_MSG_RESULT([unknown])], [test "x$TARGET_ARCH" != xunknown], [AC_MSG_RESULT([$TARGET_ARCH])])

# Check for Operating system and libc to use in $target
AC_MSG_CHECKING([which operating system the target is])

OPERATINGSYSTEM=
TARGET_LIBC=
case $target in
	*-*-linux-*)	OPERATINGSYSTEM=linux ;;
	*netbsd*)		OPERATINGSYSTEM=NetBSD ;;
	*-none-*)		OPERATINGSYSTEM=baremetal ;;
	*)					OPERATINGSYSTEM=unknown ;;
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
else
	case $target in
		*netbsd*) TARGET_LIBC=libc ;;
		*)			 TARGET_LIBC=unknown ;;
	esac
fi

#AC_SUBST(TARGET_LIBC)
AS_IF([test "x$TARGET_LIBC" = xunknown], [AC_MSG_RESULT([unknown])], [test "x$TARGET_LIBC" != xunknown], [AC_MSG_RESULT([$TARGET_LIBC])])

# check for cross compiling
if test "$cross_compiling" != "yes"; then
	AM_CONDITIONAL(CROSS_COMPILING, false)
	AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
	if test "X$PKG_CONFIG" != "X"; then
		PKG_CONFIG_PATH=`pkg-config --variable pc_path pkg-config`
	fi
else
	AM_CONDITIONAL(CROSS_COMPILING, true)
	PKG_CONFIG_PATH=${STAGING_DIR}/usr/lib/pkgconfig
	TWOCAN_CONF_MSG(Build for Target <${target}> with Environment <${STAGING_DIR}>)
	if test "x$STAGING_DIR" != "x" ; then
		CROSS_ENVIRONMENT=$STAGING_DIR
	fi
	AC_DEFINE([CROSS_COMPILE],[1],[Define if we are cross compiling])
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

# clang is mostly GCC-compatible, but its version is much lower, so we have to check for it
AC_MSG_CHECKING([if compiling with Intel C++])

AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([], [[
#ifndef __INTEL_COMPILER 
not intel
#endif
]])],
[GCC_IS_INTEL=yes], [GCC_IS_INTEL=no])

AC_MSG_RESULT([$GCC_IS_INTEL])
#AC_SUBST(GCC_IS_INTEL)

AM_CONDITIONAL(INTEL, false)
if test "$GCC_IS_INTEL" = "yes"; then
AM_CONDITIONAL(INTEL, true)
gcc_version=`icc -dumpversion`
fi

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

AM_CONDITIONAL(CLANG, false)
if test "$GCC_IS_CLANG" = "yes"; then
AM_CONDITIONAL(CLANG, true)
gcc_version=`clang -dumpversion`
fi

ULIB_LIBS=
MODULE_LIBTOOL_OPTIONS="-shared -export-dynamic -avoid-version"

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
			0.*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3,5\].x.x])
			;;
			1.*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3,5\].x.x])
			;;
			2.1.*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3,5\].x.x])
			;;
			2.2.*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3,5\].x.x])
			;;
			2.3.*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3,5\].x.x])
			;;
			2.5.*)
				AC_MSG_RESULT([unsupported ($kernel)])
				AC_MSG_ERROR([ULib works on kernels 2.\[4,6\].x,\[3,5\].x.x])
			;;
			2.4.*)
				AC_MSG_RESULT([2.4 family ($kernel)])
#				AC_DEFINE([KERNEL_2_4], 1, [Define to 1 if you're using Linux 2.4.x])
			;;
			2.6.*)
				AC_MSG_RESULT([2.6 family ($kernel)])
#				AC_DEFINE([KERNEL_2_6], 1, [Define to 1 if you're using Linux 2.6.x])
			;;
			3.*)
				AC_MSG_RESULT([3 family ($kernel)])
#				AC_DEFINE([KERNEL_3], 1, [Define to 1 if you're using Linux 3.x])
			;;
			4.*)
				AC_MSG_RESULT([4 family ($kernel)])
#				AC_DEFINE([KERNEL_4], 1, [Define to 1 if you're using Linux 4.x])
			;;
			5.*)
				AC_MSG_RESULT([5 family ($kernel)])
#				AC_DEFINE([KERNEL_5], 1, [Define to 1 if you're using Linux 5.x])
			;;
			6.*)
				AC_MSG_RESULT([6 family ($kernel)])
#				AC_DEFINE([KERNEL_6], 1, [Define to 1 if you're using Linux 6.x])
			;;
			7.*)
				AC_MSG_RESULT([7 family ($kernel)])
#				AC_DEFINE([KERNEL_7], 1, [Define to 1 if you're using Linux 7.x])
			;;
			8.*)
				AC_MSG_RESULT([8 family ($kernel)])
#				AC_DEFINE([KERNEL_8], 1, [Define to 1 if you're using Linux 8.x])
			;;
			9.*)
				AC_MSG_RESULT([9 family ($kernel)])
#				AC_DEFINE([KERNEL_9], 1, [Define to 1 if you're using Linux 9.x])
			;;
			*)
				AC_MSG_RESULT([1x family ($kernel)])
#				AC_DEFINE([KERNEL_1x], 1, [Define to 1 if you're using Linux 1x.x])
			;;
		esac

		enable_stdcpp="yes"

		AM_CONDITIONAL(LINUX, true)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;

	mingw*)
		AC_MSG_RESULT([ok ($target_os)])

		enable_stdcpp="no"

		CPPFLAGS="$CPPFLAGS -D__USE_CRTIMP -Wno-sign-compare"

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
		MODULE_LIBTOOL_OPTIONS="-export-dynamic -avoid-version -no-undefined"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, true)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;

	cygwin*)
		AC_MSG_RESULT([ok ($target_os)])

		CPPFLAGS="$CPPFLAGS -U__STRICT_ANSI__" # if not, the functions fdopen, fileno and strptime will not available...

		MODULE_LIBTOOL_OPTIONS="-shared -no-undefined"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, true)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;

	*netbsd*)
		AC_MSG_RESULT([ok ($target_os)])

		CPPFLAGS="$CPPFLAGS -fpermissive -Wno-sign-compare"

		if test "$target_os" = "netbsd"; then
			CPPFLAGS="$CPPFLAGS -D__UNIKERNEL__"
		##	LDFLAGS="${LDFLAGS} -Wl,--no-as-needed -lrump -Wl,--no-as-needed -lrumpuser"
			AC_CHECK_HEADERS([rump/rump.h],, AC_MSG_ERROR([rump kernel required]))
		else
			ULIB_LIBS="$ULIB_LIBS -lexecinfo"
			# Disable -ffunction-sections -fdata-sections and accompanying --gc-sections on netbsd because --gc-sections doesn't work with the netbsd-4 and netbsd-5 linker
			LDFLAGS="${LDFLAGS} -Wl,--no-gc-sections"
		fi

		MODULE_LIBTOOL_OPTIONS="-shared -avoid-version -no-undefined"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, true)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

	*solaris*)
		AC_MSG_RESULT([ok ($target_os)])

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, true)
	;;

	darwin*)
		AC_MSG_RESULT([maybe ($target_os)])

		CPPFLAGS="$CPPFLAGS -D__OSX__ -D__APPLE__ -Wno-deprecated-declarations"
		MODULE_LIBTOOL_OPTIONS="-dynamic -bundle -undefined suppress -flat_namespace -read_only_relocs suppress"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, true)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

	osf*)
		AC_MSG_RESULT([maybe ($target_os)])

		MODULE_LIBTOOL_OPTIONS="-shared -no-undefined"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

   *minix*)
		AC_MSG_RESULT([maybe ($target_os)])

		enable_stdcpp="no"

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
   ;;

	*)
		AC_MSG_RESULT([no ($target_os)])

		AM_CONDITIONAL(LINUX, false)
		AM_CONDITIONAL(MINGW, false)
		AM_CONDITIONAL(CYGWIN, false)
		AM_CONDITIONAL(NETBSD, false)
		AM_CONDITIONAL(DARWIN, false)
		AM_CONDITIONAL(SOLARIS, false)
	;;
esac

AM_CONDITIONAL(MS_LIB_AVAILABLE, test x$ms_librarian = xyes)

# Check for the compiler support
gcc_major_4_1="no"
if test "$ac_cv_c_compiler_gnu" = "yes" -a "x$GCC_IS_CLANG" = xno -a "x$GCC_IS_INTEL" = xno; then
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
LT_INIT
AC_SUBST(MODULE_LIBTOOL_OPTIONS)

if test "$enable_static" = "yes" -a "$enable_shared" = "no"; then
	AM_CONDITIONAL(STATIC_ONLY, true)
	AC_DEFINE(U_STATIC_ONLY, [1], [we want only static linking])
	if test -z "$enable_stdcpp"; then
		enable_stdcpp="no"
	fi
else
	AM_CONDITIONAL(STATIC_ONLY, false)
fi

AC_MSG_CHECKING([for full c++ support and linking model])
AC_ARG_ENABLE(stdcpp,  [  --enable-stdcpp           compile with stdc++ runtime overhead [[default=yes]]])

if test -z "$enable_stdcpp"; then
	enable_stdcpp="yes"
fi

CXX_save=$CXX

if test "$enable_stdcpp" = "yes"; then
   AC_MSG_RESULT(yes)
	AM_CONDITIONAL(STDCPP, true)
	AC_DEFINE(U_STDCPP_ENABLE, [1], [full c++ support and linking model])

	# Be sure to add any -std option to CXXFLAGS before we invoke any AC_COMPILE_IFELSE()
	# or similar macros. Any such macros that are invoked before we update CXXFLAGS will
	# not be run with the same options that we use during the real build

	AX_CXX_COMPILE_STDCXX_11([ext], [optional])
	AX_CXX_COMPILE_STDCXX_14([ext], [optional])
	AX_CXX_COMPILE_STDCXX_17([ext], [optional])
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

AC_MSG_CHECKING(for target that allows unaligned reads and writes)
AC_ARG_ENABLE(unaligned, [  --enable-unaligned        compile for target that allows unaligned reads and writes [[default=yes]]])

if test -z "$enable_unaligned"; then
	enable_unaligned="yes"
fi

if test "$enable_unaligned" = "yes"; then
   AC_MSG_RESULT(yes)
else
   AC_MSG_RESULT(no)
	AC_DEFINE([HAVE_NO_UNALIGNED_ACCESSES], 1, [Define to 1 if the target don't allows unaligned reads and write])
fi

AC_MSG_CHECKING(for apex memcpy/memmove)
AC_ARG_ENABLE(apexmem,   [  --enable-apexmem          use apex_memmove written by Trevor Herselman in 2014 [[default=no]]])

if test -z "$enable_apexmem"; then
	enable_apexmem="no"
fi

AM_CONDITIONAL(APEXMEM, false)

if test "$enable_apexmem" = "yes"; then
   AC_MSG_RESULT(yes)
	AM_CONDITIONAL(APEXMEM, true)
	AC_DEFINE(U_APEX_ENABLE, [1], [use apex_memmove written by Trevor Herselman in 2014])
else
   AC_MSG_RESULT(no)
fi

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
AC_CHECK_HEADERS([arpa/inet.h asm/mman.h cpuid.h fcntl.h float.h inttypes.h libintl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h \
						string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h sys/resource.h syslog.h termios.h unistd.h wchar.h \
						assert.h byteswap.h endian.h execinfo.h ifaddrs.h mcheck.h ndir.h dirent.h ndir.h string.h netpacket/packet.h sched.h \
						semaphore.h sysexits.h streambuf.h poll.h net/if_arp.h sys/dir.h sys/endian.h sys/ndir.h sys/inotify.h sys/sendfile.h sys/sched.h \
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
if test -z "$enable_thread" -a "$enable_shared" = "yes"; then
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
                AC_CHECK_LIB($tlib,pthread_suspend,[
                    AC_DEFINE(HAVE_PTHREAD_SUSPEND, [1], [pthread suspend])
                ])
                AC_CHECK_LIB($tlib,pthread_cancel,[
                    AC_DEFINE(HAVE_PTHREAD_CANCEL, [1], [pthread cancel])
                ])
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
AM_CONDITIONAL(MEMMEM, false)
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
AM_CONDITIONAL(SCHED_GETCPU, false)
AM_CONDITIONAL(SEM_TIMEDWAIT, false)

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
   AC_CHECK_LIB(numa,numa_max_node)
	if test "$ac_cv_lib_numa_numa_max_node" = "yes"; then
		plib="$plib -lnuma"
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

for func in accept4 atexit clock_gettime daemon dup3 epoll_create1 epoll_wait epoll_ctl_batch fallocate fallocate64 fnmatch getaddrinfo getnameinfo getopt_long getpriority gettid inet_ntop kqueue kqueue1 lrintl memmem memrchr mkdtemp mremap pipe2 posix_spawn pread sched_getcpu sched_getaffinity sem_init sem_getvalue sem_timedwait sendfile sendfile64 strndup strptime strsignal strtof strtold strtoull gmtime_r timegm nanosleep strerror shm_open; do
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
    clock_gettime)
		  AC_DEFINE(HAVE_CLOCK_GETTIME, [1], [has clock_gettime])
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
    gettid)
        AC_DEFINE(HAVE_GETTID, [1], [has gettid])
        ;;
    inet_ntop)
        AC_DEFINE(HAVE_INET_NTOP, [1], [has inet_ntop])
        ;;
    kqueue)
        AC_DEFINE(HAVE_KQUEUE, [1], [has kqueue])
        ;;
    kqueue1)
        AC_DEFINE(HAVE_KQUEUE1, [1], [has kqueue1])
        ;;
    lrintl)
        AC_DEFINE(HAVE_LRINTL, [1], [has lrintl])
        ;;
    memmem)
		  AM_CONDITIONAL(MEMMEM, true)
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
    sched_getcpu)
		  AM_CONDITIONAL(SCHED_GETCPU, true)
        AC_DEFINE(HAVE_SCHED_GETCPU, [1], [has sched_getcpu])
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
    sem_timedwait)
		  AM_CONDITIONAL(SEM_TIMEDWAIT, true)
        AC_DEFINE(HAVE_SEM_TIMEDWAIT, [1], [has sem_timedwait])
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
if test "$ac_cv_func_sendfile" = "yes"; then
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
ulib_libbrotli_msg="no (--with-libbrotli)"
   ulib_libtdb_msg="no (--with-libtdb)"
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
   ulib_python_msg="no (--with-python-embedded)"
     ulib_ipv6_msg="no (--with-ipv6)"
      ulib_LFS_msg="no (--enable-LFS)"
    ulib_final_msg="no (--enable-final)"
      ulib_zip_msg="no (--enable-zip)"
     ulib_memp_msg="no (--enable-memory-pool)"
    ulib_cares_msg="no (--enable-libares)"
	   ulib_plugins="no (--enable-plugins)"
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

if test -z "$enable_examples" -a "$enable_shared" = "yes"; then
	enable_examples="yes"
fi

AM_CONDITIONAL(USERVER_IPC, false)
AM_CONDITIONAL(USERVER_UDP, false)

if test "$enable_examples" = "yes"; then
   AC_MSG_RESULT(yes)
	ulib_examples="enabled"
	AM_CONDITIONAL(ENABLE_EXAMPLES, true)

	AC_MSG_CHECKING(if you want to build userver_ipc)
	AC_ARG_WITH(userver-ipc,
	[  --with-userver-ipc      Enable building of userver_ipc, default is disabled],
	[use_userver_ipc=$withval],
	[use_userver_ipc=no])
	AC_MSG_RESULT($use_userver_ipc)

	if test "$use_userver_ipc" = "yes" ; then
		AM_CONDITIONAL(USERVER_IPC, true)
		AC_DEFINE(USERVER_IPC, 1, [Define if use userver_ipc])
	fi

	AC_MSG_CHECKING(if you want to build userver_udp)
	AC_ARG_WITH(userver-udp,
	[  --with-userver-udp      Enable building of userver_udp, default is disabled],
	[use_userver_udp=$withval],
	[use_userver_udp=no])
	AC_MSG_RESULT($use_userver_udp)

	if test "$use_userver_udp" = "yes" ; then
		AM_CONDITIONAL(USERVER_UDP, true)
		AC_DEFINE(USERVER_UDP, 1, [Define if use userver_udp])
	fi
else
   AC_MSG_RESULT(no)
	AM_CONDITIONAL(ENABLE_EXAMPLES, false)
fi

AM_CONDITIONAL(ENABLE_SHARED, false)
AM_CONDITIONAL(ENABLE_PLUGINS, false)

if test "$enable_shared" = "yes"; then # NB: if not don't build .so object...
	AM_CONDITIONAL(ENABLE_SHARED, true)
	AC_MSG_CHECKING(if you want to enable build of plugins)
	AC_ARG_ENABLE(plugins,[  --enable-plugins          enable building of plugins for userver [[default=yes]]])

	if test -z "$enable_plugins"; then
      if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
         enable_plugins="no"
      else
         enable_plugins="yes"
      fi
   fi

	if test "$enable_plugins" != "yes"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		ulib_plugins="enabled"
		AM_CONDITIONAL(ENABLE_PLUGINS, true)
	fi
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

# SECTION 4: checks for libraries

libz_version="unknow"
libzopfli_version="unknown"
libbrotli_version="unknown"
libtdb_version="unknown"
pcre_version="unknown"
ldap_version="unknown"
expat_version="unknown"
libssh_version="unknown"
libcurl_version="unknown"
ssl_version="unknown"
libmagic_version="unknown"
libmysql_version="unknown"
libsqlite_version="unknown"
pgsql_version="unknown"
libuuid_version="unknown"
libevent_version="unknown"
libxml2_version="unknown"
libdbi_version="unknown"
page_speed_version="unknown"
v8_version="unknown"
php_version="unknown"
ruby_version="unknown"
ac_python_version="unknown"

TWOCAN_CONF_MSG(Checking for libraries to link against)

AC_CHECK_PACKAGE

AC_DEFUN([AC_SEARCH_LIBS_VAR],
[AC_PREREQ([2.13])
AC_CACHE_CHECK([for library containing $1], [ac_cv_search_$1],
[ac_func_search_save_LIBS="$LIBS"
ac_cv_search_$1="no"
AC_TRY_LINK_FUNC([$1], [ac_cv_search_$1="none required"])
test "$ac_cv_search_$1" = "no" && for i in $2; do
LIBS="-l$i $5 $ac_func_search_save_LIBS"
AC_TRY_LINK_FUNC([$1],
[ac_cv_search_$1="-l$i"
break])
done
LIBS="$ac_func_search_save_LIBS"])
if test "$ac_cv_search_$1" != "no"; then
  test "$ac_cv_search_$1" = "none required" || $6="$$6 $ac_cv_search_$1"
  $3
else :
  $4
fi])

AC_CHECK_SQLITE3
AC_CHECK_MYSQL
AX_LIB_POSTGRESQL

AM_CONDITIONAL(HAVE_PGSQL, false)
AM_CONDITIONAL(POSTGRESQL_USE_OPENSSL, false)

if test "$found_postgresql" = "yes"; then
   AM_CONDITIONAL(HAVE_PGSQL, true)
	AC_DEFINE(USE_PGSQL, 1, [Define if enable PostgreSQL database support])
	AC_MSG_CHECKING([whether PostgreSQL use openssl])
	if test -n "`pg_config --configure | grep openssl`"; then
		AC_MSG_RESULT([yes])
		AM_CONDITIONAL(POSTGRESQL_USE_OPENSSL, true)
	else
		AC_MSG_RESULT([no])
   fi
fi

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
	ulib_libzopfli_msg="yes ( $libzopfli_version )"
fi

if test "$USE_LIBBROTLI" = "yes"; then
	ulib_libbrotli_msg="yes ( $libbrotli_version )"
fi

if test "$USE_LIBTDB" = "yes"; then
	ulib_libtdb_msg="yes ( $libtdb_version )"
	AC_CHECK_LIB(tdb,tdb_traverse_read)
	if test "$ac_cv_lib_tdb_tdb_traverse_read" = "yes"; then
		AC_DEFINE(U_TDB_TRAVERSE_READ, 1, [Define if we have libtdb with this function])
	fi
fi

if test "$enable_zip" = "yes" -a "$USE_LIBZ" = "yes"; then
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
		LIBS=

		AC_CHECK_TYPE([siginfo_t],			  [AC_DEFINE_UNQUOTED(HAVE_SIGINFO_T, 1, [Define if exist type siginfo_t])],				[],[#include <signal.h>])
		AC_CHECK_MEMBER([siginfo_t.si_addr],AC_DEFINE_UNQUOTED(HAVE_MEMBER_SI_ADDR,1,[Define if you have si_addr in siginfo_t]),[],[#include <signal.h>])

		if test "$ac_cv_cxx_visibility_inlines_hidden_flag" = "yes"; then
			AC_DEFINE(HAVE_GNUC_VISIBILTY, 1, [enable gcc visibility features])
			CXXFLAGS="$CXXFLAGS -fvisibility=hidden -fvisibility-inlines-hidden"
		fi
		AC_MSG_CHECKING([whether new ldflags are wanted])
		if test "$with_gnu_ld" = "yes" -a "$enable_new_ldflags" = "yes"; then
		   AC_MSG_RESULT([yes])
			LDFLAGS="$LDFLAGS -Wl,-O1 -Wl,--as-needed -Wl,-z,now,-O1,--hash-style=gnu,--sort-common -Wl,--as-needed"
		else
		   AC_MSG_RESULT([no])
		fi
	;;
esac

# ---------------------------------------------------------------------------------------------------
# Check for MONGODB
# ---------------------------------------------------------------------------------------------------
AM_CONDITIONAL(MONGODB, false)

AC_MSG_CHECKING(if you want to use a mongoDB client)
AC_ARG_WITH(mongodb,
	[  --with-mongodb                 use mongodb client],
	[use_mongodb=$withval],
	[use_mongodb=no])
AC_MSG_RESULT($use_mongodb)

if test "$use_mongodb" = "yes"; then
	TWOCAN_CONF_MSG(Checking for mongodb package specific)

	# List of MONGODB include paths
	AC_ARG_WITH(mongodb-includes,
		 [  --with-mongodb-includes=PATHS  mongodb include locations],
		 [MONGODBINCLUDES="$withval"],
		 [MONGODBINCLUDES=""])

	if test -z "$MONGODBINCLUDES"; then
		MONGODBINCLUDES="-I/usr/include/libmongoc-1.0/ -I/usr/include/libbson-1.0 -I/usr/local/include/libmongoc-1.0/ -I/usr/local/include/libbson-1.0"
	fi

	# Check for mongoc.h and bson.h
	AC_LANG([C])
	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $MONGODBINCLUDES"

	AC_MSG_CHECKING([whether mongodb mongoc.h header actually works])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <mongoc.h>]], [[return 0;]])],
						 [AC_MSG_RESULT([exists and works correctly])],
						 [AC_MSG_ERROR([Cannot find mongodb mongoc.h header])])

	AC_MSG_CHECKING([whether mongodb bson.h header actually works])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <bson.h>]], [[return 0;]])],
						 [AC_MSG_RESULT([exists and works correctly])],
						 [AC_MSG_ERROR([Cannot find mongodb bson.h header])])

	CPPFLAGS=$save_CPPFLAGS
	AC_LANG([C++])

	# List of mongodb ldflags
	AC_ARG_WITH(mongodb-ldflags,
		 [  --with-mongodb-ldflags=LDFLAGS mongodb ldflags list],
		 [MONGODBLDFLAGS="$withval"],
		 [MONGODBLDFLAGS=""])

	if test -n "$MONGODBLDFLAGS"; then
		LDFLAGS="$LDFLAGS $MONGODBLDFLAGS"
	fi

	# List of mongodb lib paths
	AC_ARG_WITH(mongodb-libs,
		 [  --with-mongodb-libs=LIBS       mongodb lib locations],
		 [MONGODBLIBS="$withval"],
		 [MONGODBLIBS=""])

	if test -z "$MONGODBLIBS"; then
		MONGODBLIBS="-lmongoc-1.0 -lbson-1.0"
	fi

	AC_LANG([C])
   LIBS="$MONGODBLIBS"

   AC_CHECK_LIB(mongoc-1.0, mongoc_init, , [AC_MSG_ERROR([Not found libmongoc library])])
   AC_CHECK_LIB(bson-1.0, bson_new, , [AC_MSG_ERROR([Not found libbson library])])

   LIBS=
	AC_LANG([C++])

	ULIB_LIBS="$ULIB_LIBS $MONGODBLIBS"
	CXXFLAGS="$CXXFLAGS $MONGODBINCLUDES"

	AM_CONDITIONAL(MONGODB, true)
	AC_DEFINE(USE_MONGODB, 1, [Define if enable mongodb support])
fi
# ---------------------------------------------------------------------------------------------------

TWOCAN_CONF_MSG(Checking for language support)

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
		 [  --with-php-includes=PATHS  php-config includes list],
		 [PHPCONFIGINCLUDES="$withval"],
		 [PHPCONFIGINCLUDES=""])

	if test -z "$PHPCONFIGINCLUDES"; then
		PHPCONFIGINCLUDES="`$PHPCONFIG --includes`"
	fi

	AC_SUBST(PHPCONFIGINCLUDES)

	# Check for php_embed.h
	AC_LANG([C])
	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $PHPCONFIGINCLUDES"

	AC_MSG_CHECKING([whether php_embed.h actually works])
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <sapi/embed/php_embed.h>]], [[return 0;]])],
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
		PHPCONFIGLDFLAGS="`$PHPCONFIG --ldflags`"
	fi

	AC_SUBST(PHPCONFIGLDFLAGS)

	# List of php-config lib paths
	AC_ARG_WITH(php-libs,
		 [  --with-php-libs=LIBS       php-config libs list],
		 [PHPCONFIGLIBS="$withval"],
		 [PHPCONFIGLIBS=""])

	if test -z "$PHPCONFIGLIBS"; then
		PHPCONFIGLIBS="`$PHPCONFIG --libs`"

		if test -f "${php_prefix}/lib/libphp5.so"; then
			PHPCONFIGLIBS="-lphp5 $PHPCONFIGLIBS"
		elif test -f "${php_prefix}/lib/libphp7.so"; then
			PHPCONFIGLIBS="-lphp7 $PHPCONFIGLIBS"
		else
			AC_MSG_ERROR([libphp[5/7] not found. Required for further processing. Use --with-php-libs=list to specify the libs dependencies.])
		fi
	fi

	AC_SUBST(PHPCONFIGLIBS)

	TWOCAN_CONF_MSG(Checking for language support)
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

		ruby_version=`$cv_path_ruby -r rbconfig -e "print $ruby_rbconfig::CONFIG[['ruby_version']].gsub(/\./, '')[[0,2]]"`

		if test "X$ruby_version" = "X"; then
			ruby_version=`$cv_path_ruby -e "print ((VERSION rescue RUBY_VERSION)).gsub(/\./, '')[[0,2]]"`
		fi

		RUBYCPPFLAGS="-DRUBY_VERSION=$ruby_version -DDYNAMIC_RUBY_DLL=\\\"$libruby\\\" -DDYNAMIC_RUBY_VER=$ruby_version"

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

	TWOCAN_CONF_MSG(Checking for language support)
fi
# ---------------------------------------------------------------------------------------------------

# ---------------------------------------------------------------------------------------------------
# Check for PYTHON embedded
# ---------------------------------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to use python-embedded)
AC_ARG_WITH(python-embedded,
	[  --with-python-embedded     use python-embedded],
	[use_python=$withval],
	[use_python=no])
AC_MSG_RESULT($use_python)

if test "$use_python" = "yes"; then
  TWOCAN_CONF_MSG(Checking for python-embedded package specific)

  AX_PYTHON_DEVEL()

  if test $pythonexists = "yes"; then
	 USE_PYTHON=yes
	 AC_DEFINE(USE_PYTHON, 1, [Define if enable python support])

	 PYTHONCPPFLAGS="${PYTHON_CPPFLAGS}"
	 PYTHONCONFIGLIBS="${PYTHON_EXTRA_LIBS}"
	 PYTHONCONFIGLDFLAGS="${PYTHON_LDFLAGS} ${PYTHON_EXTRA_LDFLAGS}"

	 AC_SUBST(PYTHONCPPFLAGS)
	 AC_SUBST(PYTHONCONFIGLIBS)
	 AC_SUBST(PYTHONCONFIGLDFLAGS)
  fi
fi

TWOCAN_CONF_MSG(Checking for f-stack support)
# ---------------------------------------------------------------------------------------------------
# Check for F-STACK (http://www.f-stack.org/?from=github)
# ---------------------------------------------------------------------------------------------------
AC_MSG_CHECKING(if you want to use f-stack)
AC_ARG_WITH(f-stack,
	[  --with-f-stack             use f-stack (experimental)],
	[use_fstack=$withval],
	[use_fstack=no])
AC_MSG_RESULT($use_fstack)

if test "$use_fstack" = "yes"; then
  TWOCAN_CONF_MSG(Checking for f-stack package specific)

  # List of f-stack include paths
  AC_ARG_WITH(f-stack-includes,
		 [  --with-f-stack-includes=PATHS  f-stack includes list],
		 [FSTACKINCLUDES="$withval"],
		 [FSTACKINCLUDES=""])

  if test -z "$FSTACKINCLUDES"; then
	FSTACKINCLUDES="-I/opt/f-stack/lib"
  fi

  AC_SUBST(FSTACKINCLUDES)

  # Check for ff_api.h
  AC_LANG([C])
  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $FSTACKINCLUDES"

  AC_MSG_CHECKING([whether ff_api.h actually works])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <ff_api.h>]], [[return 0;]])],
						 [AC_MSG_RESULT([ff_api.h exists and works correctly])],
						 [AC_MSG_ERROR([Cannot find ff_api.h])])

  CPPFLAGS=$save_CPPFLAGS
  AC_LANG([C++])

  CXXFLAGS="$CXXFLAGS $FSTACKINCLUDES"

  # List of f-stack ldflags
  AC_ARG_WITH(f-stack-ldflags,
		 [  --with-f-stack-ldflags=LDFLAGS f-stack ldflags list],
		 [FSTACKLDFLAGS="$withval"],
		 [FSTACKLDFLAGS=""])

  if test -z "$FSTACKLDFLAGS"; then
	FSTACKLDFLAGS="-L/opt/f-stack/lib -L/opt/f-stack/dpdk/x86_64-native-linuxapp-gcc/lib"
  fi

##LDFLAGS="$LDFLAGS $FSTACKLDFLAGS"

  AC_SUBST(FSTACKLDFLAGS)

  # List of f-stack lib paths
  AC_ARG_WITH(f-stack-libs,
		 [  --with-f-stack-libs=LIBS       f-stack libs list],
		 [FSTACKLIBS="$withval"],
		 [FSTACKLIBS=""])

  if test -z "$FSTACKLIBS"; then
	FSTACKLIBS="-lfstack -lrte_eal -lrte_kni -lrte_port -lrte_cryptodev -lrte_ethdev -lrte_gso -lrte_ip_frag -lrte_mempool -lrte_pdump -lrte_pipeline -lrte_pmd_af_packet -lrte_reorder -lrte_sched -lrte_ring -lrte_timer -lrte_mbuf"
  fi

##ULIB_LIBS="$ULIB_LIBS $FSTACKLIBS"

  AC_SUBST(FSTACKLIBS)

  USE_FSTACK=yes
  AC_DEFINE(USE_FSTACK, 1, [Define if enable f-stack support])
fi

TWOCAN_CONF_MSG(Checking for typedefs)

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
AM_CONDITIONAL(LIBTDB,		test "$USE_LIBTDB"		= "yes")
AM_CONDITIONAL(LIBEVENT,	test "$USE_LIBEVENT"		= "yes")
AM_CONDITIONAL(LIBXML2,		test "$USE_LIBXML2"		= "yes")
AM_CONDITIONAL(PAGE_SPEED, test "$USE_PAGE_SPEED"	= "yes")
AM_CONDITIONAL(V8,			test "$USE_LIBV8"			= "yes")
AM_CONDITIONAL(PHP,			test "$USE_PHP"			= "yes")
AM_CONDITIONAL(RUBY,			test "$USE_RUBY"			= "yes")
AM_CONDITIONAL(CARES,		test "$USE_C_ARES"		= "yes")
AM_CONDITIONAL(PYTHON,		test "$USE_PYTHON"		= "yes")

AC_DEFINE_UNQUOTED(_FLEX_VERSION,		 "$flex_version",			[Flex version])
AC_DEFINE_UNQUOTED(_BISON_VERSION,		 "$bison_version",		[Bison version])
AC_DEFINE_UNQUOTED(_PCRE_VERSION,		 "$pcre_version",			[PCRE - Perl compatible regular expressions version])
AC_DEFINE_UNQUOTED(_LDAP_VERSION,		 "$ldap_version",			[Ldap version])
AC_DEFINE_UNQUOTED(_EXPAT_VERSION,		 "$expat_version",		[Expat version])
AC_DEFINE_UNQUOTED(_LIBZ_VERSION,		 "$libz_version",			[libz - general purpose compression library version])
AC_DEFINE_UNQUOTED(_LIBZOPFLI_VERSION,	 "$libzopfli_version",	[libzopfli - google compression library version])
AC_DEFINE_UNQUOTED(_LIBBROTLI_VERSION,	 "$libbrotli_version",	[libbrotli - google compression library version])
AC_DEFINE_UNQUOTED(_LIBTDB_VERSION,		 "$libtdb_version",		[libtdb - samba Trivial DB library version])
AC_DEFINE_UNQUOTED(_LIBSSH_VERSION,		 "$libssh_version",		[libSSH version])
AC_DEFINE_UNQUOTED(_SSL_VERSION,			 "$ssl_version",			[SSL version])
AC_DEFINE_UNQUOTED(_CURL_VERSION,		 "$libcurl_version",		[libcURL - multiprotocol file transfer library version])
AC_DEFINE_UNQUOTED(_MAGIC_VERSION,		 "$libmagic_version",	[libmagic - magic number recognition library version])
AC_DEFINE_UNQUOTED(_SQLITE_VERSION,		 "$libsqlite_version",	[libsqlite - SQLITE Database System library version])
AC_DEFINE_UNQUOTED(_MYSQL_VERSION,		 "$libmysql_version",	[libmysql - MySQL Database System library version])
AC_DEFINE_UNQUOTED(_PGSQL_VERSION,		 "$POSTGRESQL_VERSION",	[version of PostgreSQL database support])
AC_DEFINE_UNQUOTED(_DBI_VERSION,			 "$libdbi_version",		[libdbi - Database Independent Abstraction Layer version])
AC_DEFINE_UNQUOTED(_LIBUUID_VERSION,	 "$libuuid_version",		[libuuid - version library to create a new UUID value])
AC_DEFINE_UNQUOTED(_LIBEVENT_VERSION,	 "$libevent_version",	[libevent - event notification library version])
AC_DEFINE_UNQUOTED(_LIBXML2_VERSION,	 "$libxml2_version",		[libxml2 - version 2 of the library to manipulate XML files])
AC_DEFINE_UNQUOTED(_PAGE_SPEED_VERSION, "$page_speed_version", [PageSpeed - version of Google page speed SDK])
AC_DEFINE_UNQUOTED(_V8_VERSION,			 "$v8_version",			[V8 - version of Google V8 JavaScript Engine])
AC_DEFINE_UNQUOTED(_PHP_VERSION,			 "$php_version",			[version of PHP language runtime engine embed SAPIs])
AC_DEFINE_UNQUOTED(_RUBY_VERSION,		 "$ruby_version",			[version of RUBY language runtime engine embed])
AC_DEFINE_UNQUOTED(_PYTHON_VERSION,		 "$ac_python_version",	[version of PYTHON language runtime engine embed])
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
AC_CHECK_MEMBER([struct dirent.d_type],AC_DEFINE(DIRENT_HAVE_D_TYPE,1,[Define if you have d_type in struct dirent]),[],[ #include <dirent.h> ])
AC_CHECK_MEMBER([struct dirent.d_reclen],AC_DEFINE(DIRENT_HAVE_D_RECLEN,1,[Define if you have d_reclen in struct dirent]),[],[ #include <dirent.h> ])
AC_CHECK_MEMBER([struct dirent.d_namlen],AC_DEFINE(DIRENT_HAVE_D_NAMLEN,1,[Define if you have d_namlen in struct dirent]),[],[ #include <dirent.h> ])
AC_CHECK_MEMBER([struct tm.tm_gmtoff],AC_DEFINE(TM_HAVE_TM_GMTOFF,1,[Define if you have tm_gmtoff in struct tm]),[],[ #include <time.h> ])

TWOCAN_CONF_MSG(Checking for types, and defing if needed)

AC_C_CONST
AC_TYPE_SIZE_T
AC_CHECK_TYPE(ptrdiff_t,long)
AC_CHECK_TYPE([cpu_set_t], [AC_DEFINE_UNQUOTED(HAVE_CPU_SET_T,1,[Define if exist type cpu_set_t])],[],[#include <sched.h>])

AC_COMPILE_CHECK_SIZEOF(unsigned char)
AC_COMPILE_CHECK_SIZEOF(short)
AC_COMPILE_CHECK_SIZEOF(int)
AC_COMPILE_CHECK_SIZEOF(long)
AC_COMPILE_CHECK_SIZEOF(long long)
AC_COMPILE_CHECK_SIZEOF(time_t)
AC_COMPILE_CHECK_SIZEOF(size_t)
AC_COMPILE_CHECK_SIZEOF(void *)

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
	if test "$ac_cv_func_fallocate" = "yes" -a "$ac_cv_func_fallocate64" = "yes"; then
		AC_TRY_FALLOCATE
	fi
fi

# SECTION 14: Checks for aggregated features

modules="rpc shib stream nocat nodog socket scgi fcgi geoip proxy soap ssi tsa http echo"

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

if test "$found_postgresql" = "yes"; then
   ulib_pgsql_msg="yes ( $POSTGRESQL_VERSION )"
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
   ulib_ruby_msg="yes ( $ruby_version )"
fi

if test "$USE_PYTHON" = "yes"; then
   ulib_python_msg="yes ( $ac_python_version )"
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
static_handler_nodog=""
static_handler_http=""
static_handler_echo=""

AC_ARG_ENABLE(static-server-plugin,
				  AS_HELP_STRING([--enable-static-server-plugin=MODULES],[ rpc shib stream socket scgi fcgi geoip proxy soap ssi tsa nocat nodog http echo ]),
				  [use_static_module="$enableval"])

for mod in $use_static_module; do
	AC_MSG_CHECKING([server plugin "$mod"])

	if echo $use_static_module | grep -w $mod >/dev/null || echo $use_static_module | grep all >/dev/null; then

		add_calls1="$add_calls1 \n loadStaticLinkedModules(U_STRING_FROM_CONSTANT(\"$mod\"));"

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
		elif test "$mod" = "nodog"; then
			static_handler_nodog="yes"
			AC_DEFINE(U_STATIC_HANDLER_NODOG,, [STATIC_HANDLER_NODOG])
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
AM_CONDITIONAL(STATIC_HANDLER_NODOG,  test "$static_handler_nodog"  = "yes")
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

if test "$HAVE_POSTGRESQL" != "yes"; then
	drivers=`echo $drivers | sed s/pgsql//`
fi

add_calls2=""
use_static_driver=""
static_handler_mysql=""
static_handler_sqlite=""
static_handler_pgsql=""

AC_ARG_ENABLE(static-orm-driver,
				  AS_HELP_STRING([--enable-static-orm-driver=DRIVERS],[ sqlite mysql pgsql ]),
				  [use_static_driver="$enableval"])

for drv in $use_static_driver; do
	AC_MSG_CHECKING([ORM driver "$drv"])

	if echo $use_static_driver | grep -w $drv >/dev/null || echo $use_static_driver | grep all >/dev/null; then

		add_calls2="$add_calls2 \n loadStaticLinkedModules(U_STRING_FROM_CONSTANT(\"$drv\"));"

		if test "$drv" = "sqlite"; then
			static_handler_sqlite="yes"
         ULIB_LIBS="$ULIB_LIBS $SQLITE3_LIBS"
			LDFLAGS="$SQLITE3_LDFLAGS $LDFLAGS"
			CXXFLAGS="$SQLITE3_INCLUDE $CXXFLAGS"
			AC_DEFINE(U_STATIC_ORM_DRIVER_SQLITE,, [STATIC_ORM_DRIVER_SQLITE])
		elif test "$drv" = "pgsql"; then
			static_handler_pgsql="yes"
         CPPFLAGS="$POSTGRESQL_CPPFLAGS $CPPFLAGS"
			AC_DEFINE(U_STATIC_ORM_DRIVER_PGSQL,, [STATIC_ORM_DRIVER_PGSQL])
		elif test "$drv" = "mysql"; then
			static_handler_mysql="yes"
         ULIB_LIBS="$ULIB_LIBS $MYSQL_LIBS"
			LDFLAGS="$MYSQL_LDFLAGS $LDFLAGS"
			CXXFLAGS="$MYSQL_INCLUDE $CXXFLAGS"
			AC_DEFINE(U_STATIC_ORM_DRIVER_MYSQL,, [STATIC_ORM_DRIVER_MYSQL])
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

if test "$enable_static" = "yes" -a "$enable_shared" = "no"; then

	use_static_servlet=""

	TWOCAN_CONF_MSG(Checking for static HTTP servlet)

	AC_ARG_ENABLE(static-servlet, AS_HELP_STRING([--enable-static-servlet=NAMES],[ wi_auth ... ]),
					  [use_static_servlet="$enableval"])

	if test -z "$use_static_servlet" ; then
		echo >> src/ulib/net/server/plugin/usp/loader.autoconf.ext
		echo >> src/ulib/net/server/plugin/usp/loader.autoconf.cpp
	else
		add_calls3=""
		add_calls4=""
		add_calls5=""

		static_servlet_make=""
		dir=net/server/plugin/usp

		for srv in $use_static_servlet; do

			AC_MSG_CHECKING([servlet "$srv"])

			add_calls3="$add_calls3 \n extern \"C\" { void runDynamicPage_${srv}(); } \n extern \"C\" { void runDynamicPageParam_${srv}(uint32_t); }"
			add_calls4="$add_calls4 \n loadStaticLinkedServlet(\"${srv}\",U_CONSTANT_SIZE(\"${srv}\"),(vPF)::runDynamicPage_${srv},(vPFu)::runDynamicPageParam_${srv});"

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

CXXFLAGS="$CXXFLAGS -fno-exceptions -fno-rtti"

# If gcc compiler add values to *FLAGS...

if test "$ac_cv_c_compiler_gnu" = "yes" -a "x$OPERATINGSYSTEM" = xlinux; then

	TWOCAN_CONF_MSG(Checking for $CC compiler flags)

	  if test "x$GCC_IS_INTEL" = xyes; then
		MAYBE_FLAGS="-fno-check-new -fno-stack-protector -fno-stack-protector-all -Ofast -march=native -mtune=native"
	elif test "x$GCC_IS_CLANG" = xyes; then
		MAYBE_FLAGS="-fno-check-new -fno-stack-protector -fno-stack-protector-all -Ofast -march=native -mtune=native \
						 -ffast-math -ftree-vectorize -fno-crossjumping -fpartial-inlining"
	else
		AC_MSG_CHECKING([whether to use libgcc])
		AC_ARG_ENABLE(libgcc,
		AC_HELP_STRING([--enable-libgcc],[use libgcc when linking]),
			[ case "$enableval" in
			  yes)
				 LIBS="-lgcc $LIBS"
				 AC_MSG_RESULT(yes)
				 ;;
			  *) AC_MSG_RESULT(no)
				 ;;
			  esac ],
				 AC_MSG_RESULT(no)
			)

		MAYBE_FLAGS="-Wstrict-aliasing=2 -Wall -Wextra -Wsign-compare -Wpointer-arith -Wwrite-strings \
						 -Wmissing-declarations -Wpacked -Wswitch-enum -Wmissing-format-attribute -Winit-self -Wformat \
						 -Wformat-extra-args -Wenum-compare -Wlogical-not-parentheses -Wsizeof-array-argument -Wbool-compare -Wno-cast-function-type"

		# -------------------------------------------------------------------------------------
		# to turn various gcc/glibc-specific preprocessor checks
		# -------------------------------------------------------------------------------------
		# -Wno-format -Wno-format-extra-args
		# -Wsuggest-final-methods -Wsuggest-final-types
		# -------------------------------------------------------------------------------------
		# -Wcast-qual
		# -------------------------------------------------------------------------------------

		 CC_MAYBE_FLAGS="-Werror-implicit-function-declaration -Wstrict-prototypes \
							  -Wmissing-prototypes -Wnested-externs -Wdeclaration-after-statement -Wsizeof-pointer-memaccess"



		CXX_MAYBE_FLAGS="-fno-check-new -fno-enforce-eh-specs -Wno-deprecated -Wdelete-non-virtual-dtor -Wodr"

			 MAYBE_FLAGS_MODERN=""
		CXX_MAYBE_FLAGS_MODERN=""

		if test "$found_postgresql" = "yes"; then
			if test "$static_handler_pgsql" != "yes"; then
				CC_MAYBE_FLAGS="$CC_MAYBE_FLAGS -Wc++-compat -Wold-style-definition"
			else
				CC_MAYBE_FLAGS="$CC_MAYBE_FLAGS -Wno-discarded-qualifiers -Wno-switch-enum -Wno-missing-field-initializers -Wno-sign-compare -Wno-strict-aliasing \
									 -Wno-packed -Wno-shadow -Wno-suggest-attribute=pure -Wno-strict-prototypes -Wno-stringop-truncation -Wno-format-truncation \
									 -Wno-implicit-fallthrough -Wno-null-dereference"
			fi
		fi

		case "$gcc_version" in
			2*|3*|4.0*|4.1*|4.2*)
			;;
			4.3*) # openWRT backfire: ... c++/4.3.3/tr1_impl/boost_shared_ptr.h:630: error: cannot use typeid with -fno-rtti
			;;
			4.4*) # centOS 64: ... c++/4.4.7/bits/shared_ptr.h:1005: error: cannot use typeid with -fno-rtti
			;;
			4.5*) # openwrt: SIGSEGV
			;;
			4.6*|4.7*|4.8*|4.9*|5*)
				 MAYBE_FLAGS_MODERN="-Wno-unused-result -Wshadow -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn" # -Wsuggest-attribute=const
			;;
			6.1.0|*)
				 MAYBE_FLAGS_MODERN="-Wno-unused-result -Wshadow -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn \
											-Wlogical-op -Wduplicated-cond -Wtautological-compare -Wswitch-bool -Wshift-negative-value \
											-Wshift-overflow -Wshift-overflow=2 -Wnull-dereference -Wnonnull"
			CXX_MAYBE_FLAGS_MODERN="-Wterminate -Wlto-type-mismatch -Wsubobject-linkage -Wplacement-new -Woverride-init-side-effects \
											-Wvirtual-inheritance" # -Wnamespaces -Wtemplates -Wmultiple-inheritance
			;;
		esac

			 MAYBE_FLAGS="    $MAYBE_FLAGS     $MAYBE_FLAGS_MODERN"
		CXX_MAYBE_FLAGS="$CXX_MAYBE_FLAGS $CXX_MAYBE_FLAGS_MODERN"

		if test "$enable_gcc_optimized" = "yes" -a "$enable_coverage" != "yes"; then
			## ----------------------------------------------------------------------------------------------------------------------------------
			## -fgcse Perform a global common subexpression elimination pass. This pass also performs global constant and copy propagation. When
			##			 compiling a program using computed gotos, a GCC extension, you may get better runtime performance if you disable the global
			##			 common subexpression elimination pass by adding -fno-gcse to the command line
			## ----------------------------------------------------------------------------------------------------------------------------------
			## -ffunction-sections -fdata-sections
			## ----------------------------------------------------------------------------------------------------------------------------------
			MAYBE_FLAGS="$MAYBE_FLAGS -fno-stack-protector -fno-stack-protector-all -ffast-math -ftree-vectorize -fno-crossjumping -fno-gcse -fpartial-inlining -Ofast"

			if test "$use_distcc" != "yes" -a "$cross_compiling" != "yes"; then
				MAYBE_FLAGS="$MAYBE_FLAGS -flto -ffat-lto-objects -fuse-linker-plugin -march=native -mtune=native" # NB: distcc don't work with this...
			fi
		fi

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
			MAYBE_FLAGS="$MAYBE_FLAGS -Wno-unused-parameter -Wno-unused-variable" # -Winline
		else
			## _FORTIFY_SOURCE is enabled by default from g++ 4.7.2
			MAYBE_FLAGS="$MAYBE_FLAGS -Wunsafe-loop-optimizations -Wno-unused-parameter -rdynamic -fdiagnostics-color=auto -Wmisleading-indentation"
			CXX_MAYBE_FLAGS="$CXX_MAYBE_FLAGS -Wc++11-compat -Wno-implicit-fallthrough" ## -Wzero-as-null-pointer-constant
		fi
	fi

	AC_CACHE_CHECK([for $CC supported flags], ulib_cv_flags, [
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

		AC_MSG_CHECKING([which $CC flags were supported])
	])

	SANITIZE_FLAGS=""
	SANITIZE_THREAD=""
	SANITIZE_ADDRESS=""

	if test "x$GCC_IS_CLANG" = xno -a "x$GCC_IS_INTEL" = xno; then
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
				AC_PATH_PROG(GCC_AR, gcc-ar, no)
				if test "$GCC_AR" != "no"; then
					AR=gcc-ar
					NM=gcc-nm
					RANLIB=gcc-ranlib
				fi
			fi
			;;
		esac
	fi

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
	if test "$with_gnu_ld" = "yes" -a "$enable_new_ldflags" = "yes"; then
		ULIB_LIBS="$ULIB_LIBS -Wl,--no-as-needed $plib";
	else
		ULIB_LIBS="$ULIB_LIBS $plib";
	fi
fi
if test "$enable_coverage" = "yes"; then
	ULIB_LIBS="$ULIB_LIBS -lgcov"
fi
if test -n "$rlib"; then
   ULIB_LIBS="$ULIB_LIBS $rlib";
fi
if test "$USE_LIBSSL" = "yes"; then
	case "$target_os" in
		*netbsd*) ULIB_LIBS="$ULIB_LIBS -ldes" ;;
	esac
fi

ULIB_LIBS="$ULIB_LIBS $ULIB_LIBC"

if test "$enable_stdcpp" != "yes"; then
	AC_CHECK_LIB(Systemstubs, printf, [ULIB_LIBS="$ULIB_LIBS -lSystemstubs -lgcc"])
else if test -n "$ac_cv_env_LIBS_value"; then
	ULIB_LIBS="$ULIB_LIBS $ac_cv_env_LIBS_value"
 fi
fi

##########################
# to avoid: hidden symbol `__cpu_indicator_init' in /usr/lib/gcc/x86_64-linux-gnu/4.8/libgcc.a(cpuinfo.o) is referenced by DSO
##########################
if test "x$ac_cv_crc32_intrinsics" = xyes; then
	# TRAVIS CI
	if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
		ULIB_LIBS="$ULIB_LIBS -lgcc_s -lgcc"
 	else
		to_match='gcc (Ubuntu 5.*'
		case `gcc --version | head -n 1` in
			$to_match) ULIB_LIBS="$ULIB_LIBS -lgcc_s -lgcc" ;; # On Ubuntu 16.04 bug: [#65612](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=65612)
		esac
	fi
fi
##########################

if test "$ac_cv_c_compiler_gnu" = "yes" -a "x$GCC_IS_CLANG" = xno -a "x$GCC_IS_INTEL" = xno; then
	if test "$enable_static" = "yes" -a "$enable_shared" = "no" -a "$enable_stdcpp" = "yes"; then

##		if test -f "$gcc_dir/libstdc++.a"; then
##			gcc_lib=`gcc -print-libgcc-file-name`
##			gcc_dir=`dirname $gcc_lib`
##			ULIB_LIBS="$ULIB_LIBS $gcc_dir/libstdc++.a $gcc_lib"
##		else
##			ULIB_LIBS="$ULIB_LIBS -lstdc++ -lgcc"
##		fi

		ULIB_LIBS="$ULIB_LIBS -lstdc++ -lgcc"
	else
		AC_MSG_CHECKING([whether to use libgcc])
		AC_ARG_ENABLE(libgcc,
		AC_HELP_STRING([--enable-libgcc],[use libgcc when linking]),
			[ case "$enableval" in
			  yes)
				 ULIB_LIBS="$ULIB_LIBS -lgcc"
				 AC_MSG_RESULT(yes)
				 ;;
			  *) AC_MSG_RESULT(no)
				 ;;
			  esac ],
				 AC_MSG_RESULT(no)
			)
	fi
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

STD_GNU11=
if test "x$ax_cv_cxx_compile_cxx11" = xno -a "x$ax_cv_cxx_compile_cxx11__std_gnupp11" = xyes; then
	CXXFLAGS="-std=gnu++11 $CXXFLAGS"

	AC_MSG_CHECKING(for workaround to enable on Sabayon)
	to_match='gcc (Gentoo Hardened 4.9.3*'
	case `gcc --version | head -n 1` in
		$to_match)
			STD_GNU11="-std=gnu++11"

			AC_MSG_RESULT(enabled)
		;;
		*)	AC_MSG_RESULT(no) ;;
	esac
fi
AC_SUBST(STD_GNU11)
if test "x$ax_cv_cxx_compile_cxx14" = xno -a "x$ax_cv_cxx_compile_cxx14__std_gnupp14" = xyes; then
	CXXFLAGS="-std=gnu++14 $CXXFLAGS"
fi
if test "x$ax_cv_cxx_compile_cxx17" = xno -a "x$ax_cv_cxx_compile_cxx17__std_gnupp17" = xyes; then
	CXXFLAGS="-std=gnu++17 $CXXFLAGS"
fi

CXX=$CXX_save
AC_SUBST(CXX)
AC_SUBST(LDFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)

AC_DEFINE_UNQUOTED(CXX_VAR, 		"$CXX", [Compiler])
AC_DEFINE_UNQUOTED(LIBS_VAR, 		"$ULIB_LIBS", [Libraries])
AC_DEFINE_UNQUOTED(CFLAGS_VAR,	"$CFLAGS", [C Flags])
AC_DEFINE_UNQUOTED(LDFLAGS_VAR, 	"$LDFLAGS", [Linker Flags])
AC_DEFINE_UNQUOTED(CXXFLAGS_VAR, "$CXXFLAGS", [C++ Flags])
AC_DEFINE_UNQUOTED(CPPFLAGS_VAR, "$CPPFLAGS", [Preprocessor Flags])

AC_DEFINE_UNQUOTED(LD_VAR, 		"$LD", [Linker])
AC_DEFINE_UNQUOTED(PLATFORM_VAR, "$PLATFORM", [Platform])

# rpm spec
RPM_REQUIRE=rpmpkgreq.lst
AC_SUBST_FILE([RPM_REQUIRE])
RPM_CONFIGURE="--enable-static --enable-shared"
AC_SUBST([RPM_CONFIGURE])

# Collect building environment
AC_COMPILATION_ENVIRONMENT

# if it's set to NONE, it means it hasn't been given on the command line
if test "$prefix" = "NONE"; then
	u_prefixdir="$ac_default_prefix"
else
   u_prefixdir="$prefix"
fi
if test "$libdir" = "NONE"; then
	libdir="$u_prefixdir/lib"
fi
if test "$libexecdir" = "NONE"; then
   u_libexecdir="$u_prefixdir/libexec/ulib"
else
	exec_prefix="$u_prefixdir"
   eval u_libexecdir=\""$libexecdir\""
fi
if test "$sysconfdir" = "NONE"; then
   u_sysconfdir="$u_prefixdir/etc/ulib"
else
	prefix="$u_prefixdir"
   eval u_sysconfdir=\""$sysconfdir\""
fi

# Prefix dir
AC_DEFINE_UNQUOTED(U_PREFIXDIR,	"$u_prefixdir", [install directory])
# Plugin dir - program executables [PREFIX/libexec]
AC_DEFINE_UNQUOTED(U_LIBEXECDIR, "$u_libexecdir", [install directory for plugins])
# Config dir - read-only single-machine data [PREFIX/etc]
AC_DEFINE_UNQUOTED(U_SYSCONFDIR, "$u_sysconfdir", [install directory for configuration data])

ULIB_PREFIXDIR="$u_prefixdir"
AC_SUBST(ULIB_PREFIXDIR)
ULIB_MODULEDIR="$u_libexecdir"
AC_SUBST(ULIB_MODULEDIR)
ULIB_SYSCONFDIR="$u_sysconfdir"
AC_SUBST(ULIB_SYSCONFDIR)

AC_MSG_NOTICE([Configured to build src/ulib/lib${ULIB}:
------------------------------------------------------------------------
           ULib version: ${ULIB_VERSION}
             Build ULib: Shared=${enable_shared}, Static=${enable_static}

             Host setup: ${host}
         Install prefix: ${u_prefixdir}
         Install plugin: ${u_libexecdir}
     Configuration data: ${u_sysconfdir}

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
      LIBBROTLI support: ${ulib_libbrotli_msg}
         LIBTDB support: ${ulib_libtdb_msg}
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
       libevent support: ${ulib_libevent_msg} (deprecated)
        libxml2 support: ${ulib_libxml2_msg}
         c-ares support: ${ulib_cares_msg}
     Page-Speed support: ${ulib_pagespeed_msg}
  V8 JavaScript support: ${ulib_v8_msg}
   PHP language support: ${ulib_php_msg}
  RUBY language support: ${ulib_ruby_msg}
PYTHON language support: ${ulib_python_msg}

                Plugins: ${ulib_plugins}
               Examples: ${ulib_examples}
       LEX/YACC support: ${ulib_parser_msg}
       Lexical analyzer: ${LEX} ( ${flex_version} )
       Parser generator: ${YACC} ( ${bison_version} )

See include/ulib/internal/config.h for further configuration information
------------------------------------------------------------------------])
#      Built-in manual: ${ulib_manual_msg}

# SECTION 16: `AC_CONFIG_FILES([FILE...])

AC_CONFIG_FILES([
	Makefile rpm.sh ULib.spec userver.service m4/Makefile doc/Makefile include/Makefile contrib/Makefile
	include/ulib/Makefile contrib/HCSP/Makefile contrib/RSIGN/Makefile contrib/signer/Makefile fuzz/Makefile
	src/ulib/net/server/plugin/v8/Makefile src/ulib/net/server/plugin/php/Makefile src/ulib/net/server/plugin/usp/Makefile
	src/ulib/net/server/plugin/ruby/Makefile src/ulib/net/server/plugin/mod_shib/Makefile src/ulib/net/server/plugin/mod_geoip/Makefile
	src/ulib/net/server/plugin/usp/usp_compile.sh src/ulib/net/server/plugin/page_speed/Makefile src/ulib/net/server/plugin/python/Makefile
	src/ulib/Makefile src/ulib/ULib.rc src/ulib/ULib.pc src/ulib/orm/driver/Makefile src/ulib/net/server/plugin/Makefile src/ulib/orm/driver/libpq/Makefile
	examples/serialize/Makefile examples/WiAuth/Makefile examples/WiAuth/v2/Makefile examples/xml2txt/Makefile examples/uclient/Makefile examples/userver/Makefile
	examples/Makefile examples/IR/Makefile examples/csp/Makefile examples/lcsp/Makefile examples/rsign/Makefile examples/hello/Makefile examples/loginCookie/Makefile
	examples/PEC_log/Makefile examples/workflow/Makefile examples/doc_parse/Makefile examples/lrp_session/Makefile examples/http_header/Makefile
	examples/test_manager/Makefile examples/doc_classifier/Makefile examples/form_completion/Makefile examples/simple_client_server/Makefile
	examples/download_accelerator/Makefile examples/XAdES/Makefile examples/XAdES/XAdES.spec examples/xml2txt/xml2txt.spec examples/IR/searchengine-bin.spec
	examples/lcsp/lcsp.spec examples/lcsp/lcsp_rpc.spec examples/csp/cspclient.spec examples/csp/cspclient_rpc.spec examples/rsign/rsignclient.spec
	examples/rsign/rsignclient_rpc.spec examples/userver/web_server.spec examples/userver/wagsmserver.spec examples/workflow/workflow.spec 
	examples/PEC_log/PEC_log.spec examples/userver/tsaserver.spec examples/userver/cspserver.spec examples/userver/rsignserver.spec examples/doc_parse/doc_parse.spec
	tests/Makefile tests/base/Makefile tests/debug/Makefile tests/ulib/Makefile tests/ulib/http2/Makefile tests/examples/Makefile tests/contrib/Makefile])

#	examples/parser/Makefile \
#	src/ulib/base/Makefile src/ulib/debug/Makefile \
#	ulib-config ,[chmod a+x ulib-config] )

AC_OUTPUT

if test -n "$AWK"; then
  $AWK -f configure_log.awk config.log							  > include/ulib/internal/config1.h
  $AWK -f configure_h.awk   include/ulib/internal/config.h > include/ulib/internal/config2.h
  cd include/ulib/internal
  cat   config2.h config1.h >> config.h
  rm -f config2.h config1.h
  cd ../../..
fi

echo

# Verify the integrity of the current configuration
if test "$cross_compiling" != "yes"; then
	CPPFLAGS="-DHAVE_CONFIG_H -I${srcdir}/include $CPPFLAGS"

	AC_CACHE_CHECK([if generated configuration is usable], ac_cv_configuration_is_usable,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include "src/ulib/internal/common.cpp"
]], [[int a = 0; a += 1;]])],[ac_cv_configuration_is_usable="yes"],[ac_cv_configuration_is_usable="no"])
])

	if test "$ac_cv_configuration_is_usable" = "no" ; then
		# It appears that ulib/internal/common.cpp not compiled. If it didn't compile then there would be no chance that the rest would compile.
		TWOCAN_CONF_MSG(The generated configuration appears to be unusable. Please verify that your system path and environment variables are correct)
		echo "***********************************************************************************************************************************"
		NLINE=`grep -n 'checking if generated configuration is usable' config.log | cut -d':' -f1`
		CFGLOG=`tail -n +$NLINE config.log`
		echo "${CFGLOG}"
		echo "***********************************************************************************************************************************"
		exit 1
	fi
fi

echo
echo "You are now ready to build ULib"
echo "Enter the following command: ${T_MD}make && make install${T_ME}"
echo "After on some environment you must run also the following command: ${T_MD}sudo ldconfig${T_ME}"
