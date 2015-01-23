dnl check compiler flags
AC_DEFUN([AC_CC_TRY_FLAG],
[dnl
	AC_MSG_CHECKING([whether $CC supports $1])

	_save_cppflags="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS -Werror $1"
	AC_COMPILE_IFELSE([AC_LANG_SOURCE([ ])], [ulib_cc_flag=yes], [ulib_cc_flag=no])
	CPPFLAGS="$_save_cppflags"

	if test "x$ulib_cc_flag" = "xyes"; then
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
	AC_MSG_RESULT([$ulib_cc_flag])
])
