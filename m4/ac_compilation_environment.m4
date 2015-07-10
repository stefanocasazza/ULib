dnl @synopsis AC_COMPILATION_ENVIRONMENT
dnl Macros that collect building environment to configure script
dnl AC_COMPILATION_ENVIRONMENT

AC_DEFUN([AC_COMPILATION_ENVIRONMENT],[
	AC_DEFINE_UNQUOTED(GCC_VERSION, "${gcc_version}", [GNU C/C++ Compiler version])
	ld_version=`${LD} -V 2>/dev/null | head -n1 2>/dev/null`
	if test -z "$ld_version"; then
		ld_version=`${LD} -v -V 2>/dev/null | head -n1 2>/dev/null`
		if test -z "$ld_version"; then
			ld_version="unknown"
		fi
	fi
	AC_DEFINE_UNQUOTED(LD_VERSION, "${ld_version}", [LD Linker version])
	for dir in ${CROSS_ENVIRONMENT}/usr/lib ${CROSS_ENVIRONMENT}/usr/local/lib; do
		if test -f "$dir/libstdc++.so"; then
			stdgpp_dir="$dir"
		fi
		if test -f "$dir/libc.so"; then
			libc_version=`getconf GNU_LIBC_VERSION 2>/dev/null`
			if test -z "$libc_version"; then
				libc_version=$(ls ${dir}/libc.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "$libc_version"; then
				if test "`ls -l /lib/libc.so.* | wc -l | sed -e 's/ //g'`" != "0"; then
					libc_version=`ls -l /lib/libc.so.* | sed -e 's/.* //g'`
					tmp_libc_version="`PATH=${PATH}:/lib; $libc_version | grep 'GNU C Library'`"
					if test -n "$tmp_libc_version"; then
						libc_version="$tmp_libc_version"
					else
						libc_version=`echo $libc_version | sed -e 's/\/lib\///g'`
					fi
				fi
			fi
			break;
		fi
	done
	if test -z "$libc_version"; then
		libc_version="unknown"
	fi
	AC_DEFINE_UNQUOTED(LIBC_VERSION, "${libc_version}", [Standard C lib version])
	if test -z "$stdgpp_dir"; then
		stdgpp_dir=`gcc -v 2>&1 | grep 'with-gxx-include-dir=' | awk -F'with-gxx-include-dir=' '{n=2; print $n}' | sed -e 's/\/include.*//g'`
	fi
	if test -n "$stdgpp_dir"; then
		stdgpp_version=$(ls ${stdgpp_dir}/libstdc++.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
	fi
	if test -z "$stdgpp_version"; then
		stdgpp_version="unknown"
	fi
	AC_DEFINE_UNQUOTED(STDGPP_VERSION, "${stdgpp_version}", [Stardard GNU C++ Library version])
])
