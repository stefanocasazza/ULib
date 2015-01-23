####################################################################################
# ULib.m4 - Example autoconf macro showing how to find ULib library and header files
####################################################################################

dnl @synopsis AC_ULIB
dnl 
dnl This macro tries to find the ULib library and header files.
dnl
dnl We define the following configure script flags:
dnl
dnl	--with-ulib: Give prefix for both library and headers, and try to guess subdirectory names for each.
dnl
dnl	--with-ulib-lib:		Similar to --with-ulib, but for library only.
dnl	--with-ulib-include: Similar to --with-ulib, but for headers only.

AC_DEFUN([AC_ULIB],[
AC_CACHE_CHECK([for ULib library stuff], ac_cv_ulib,
[ # Set up configure script macros
	AC_ARG_WITH(ulib,
	[  --with-ulib=<path>     path containing ULib header and library subdirs],
	[ULIB_lib_check="$with_ulib/lib $with_ulib/lib/ulib"
	 ULIB_inc_check="$with_ulib/include $with_ulib/include/ulib"],
	[ULIB_lib_check="/usr/local/ulib/lib /usr/local/lib/ulib /opt/ulib/lib /usr/lib/ulib /usr/local/lib /usr/lib"
	 ULIB_inc_check="/usr/local/ulib/include /usr/local/include/ulib /opt/ulib/include /usr/local/include/ulib /usr/local/include /usr/include/ulib /usr/include"])
	AC_ARG_WITH(ulib-lib,
		[  --with-ulib-lib=<path> directory path of ULib library],
		[ULIB_lib_check="$with_ulib_lib $with_ulib_lib/lib $with_ulib_lib/lib/ulib"])
	AC_ARG_WITH(ulib-include,
		[  --with-ulib-include=<path> directory path of ULib headers],
		[ULIB_inc_check="$with_ulib_include $with_ulib_include/include $with_ulib_include/include/ulib"])

	#
	# Look for ULib library
	#
	ULIB_libdir=
	for dir in $ULIB_lib_check
	do
		if test -d "$dir" && \
			( test -f "$dir/libulib.so" ||
			  test -f "$dir/libulib.a" )
		then
			ULIB_libdir=$dir
			break
		fi
	done

	if test -z "$ULIB_libdir"
	then
		AC_MSG_ERROR([Didn't find the ULib library dir in '$ULIB_lib_check'])
	fi

	case "$ULIB_libdir" in
		/* ) ;;
		 * )  AC_MSG_ERROR([The ULib library directory ($ULIB_libdir) must be an absolute path.]) ;;
	esac

	AC_MSG_RESULT([lib in $ULIB_libdir])

	case "$ULIB_libdir" in
	  /usr/lib) ;;
	  *) LDFLAGS="$LDFLAGS -L${ULIB_libdir}" ;;
	esac

	#
	# Look for ULib headers
	#
	AC_MSG_CHECKING([for ULib header directory])
	ULIB_incdir=
	for dir in $ULIB_inc_check
	do
		if test -d "$dir" && test -f "$dir/all.h"
		then
			ULIB_incdir=$dir
			break
		fi
	done

	if test -z "$ULIB_incdir"
	then
		AC_MSG_ERROR([Didn't find the ULib header dir in '$ULIB_inc_check'])
	fi

	case "$ULIB_incdir" in
		/* ) ;;
		 * )  AC_MSG_ERROR([The ULib header directory ($ULIB_incdir) must be an absolute path.]) ;;
	esac

	AC_MSG_RESULT([$ULIB_incdir])

	CPPFLAGS="$CPPFLAGS -I${ULIB_incdir}"

	AC_MSG_CHECKING([that we can build ULib programs])
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([#include <all.h>],
		[UString s; s.c_str();])],
		ac_cv_ulib=yes,
		AC_MSG_ERROR(no))
])
])
dnl End ULIB

