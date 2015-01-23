dnl @synopsis AC_COMPILATION_ENVIRONMENT
dnl Macros that collect building environment to `configure' script.
dnl AC_COMPILATION_ENVIRONMENT

AC_DEFUN([AC_COMPILATION_ENVIRONMENT],[
dnl Collect building environment
dnl gcc_version=`${CC} -v 2>&1 | head -n1`
AC_DEFINE_UNQUOTED(GCC_VERSION, "${gcc_version}", [GNU C/C++ Compiler version])

ld_version=`${LD} -V 2>/dev/null | head -n1 2>/dev/null`

if test -z "$ld_version"; then
	ld_version=`${LD} -v -V 2>/dev/null | head -n1 2>/dev/null`
fi

if test -z "$ld_version"; then
	ld_version="unknown"
fi

AC_DEFINE_UNQUOTED(LD_VERSION, "${ld_version}", [LD Linker version])
dnl ldd /bin/ls | grep libc.so | awk '{print $3}'
dnl $(ldd /bin/ls | grep libc.so | awk '{print $3}')
if test "$cross_compiling" != "yes" && test -e /lib/libc.so.6; then
	libc_version=`getconf GNU_LIBC_VERSION 2>/dev/null`
	if test -z "$libc_version"; then
		if test "`ls -l /lib/libc.so.* | wc -l | sed -e 's/ //g'`" != "0"; then
			 libc_version=`ls -l /lib/libc.so.* | sed -e 's/.* //g'`
dnl		 Is glibc ?
			 tmp_libc_version="`PATH=${PATH}:/lib; $libc_version | grep 'GNU C Library'`"
			 if test -n "$tmp_libc_version"; then
				  libc_version="$tmp_libc_version"
			 else
				  libc_version=`echo $libc_version | sed -e 's/\/lib\///g'`
			 fi
		else
			 libc_version="unknown"
		fi
	fi
fi

if test -z "$libc_version"; then
	libc_version="unknown"
fi

AC_DEFINE_UNQUOTED(LIBC_VERSION, "${libc_version}", [Standard C lib version])

if test "$cross_compiling" != "yes"; then
	if test -e /usr/lib/libstdc++.so; then
		LS=`ls -l /usr/lib/libstdc++.so.*`
		if test "`echo $LS | wc -l | sed -e 's/ //g'`" != "0"; then
			stdgpp_version=`echo $LS | sed -e 's/.* //g' | head -n 1`
		else
			stdgpp_dir=`gcc -v 2>&1 | grep "Reading specs from " | sed -e 's/Reading specs from //g' | sed -e 's/\/specs//g'`
		fi
	else
		stdgpp_dir=`gcc -v 2>&1 | grep 'with-gxx-include-dir=' | awk -F'with-gxx-include-dir=' '{ n=2; print $n }' | sed -e 's/\/include.*//g'`
	fi

	if test -n "$stdgpp_dir"; then
		stdgpp_version=`ls -l ${stdgpp_dir}/libstdc++.* 2>/dev/null | tail -n 1 | awk '{ nf=split ($NF, f, "/"); print f[[nf]] }'`
	else
		stdgpp_version="unknown"
	fi
fi

if test -z "$stdgpp_version"; then
	stdgpp_version="unknown"
fi

AC_DEFINE_UNQUOTED(STDGPP_VERSION, "${stdgpp_version}", [Stardard GNU C++ Library version])
])
