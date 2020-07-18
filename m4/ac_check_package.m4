dnl @synopsis AC_CHECK_PACKAGE
dnl Macros that add compilation options to a `configure' script.
dnl AC_CHECK_PACKAGE

AC_DEFUN([AC_CHECK_PACKAGE],[
	AC_MSG_CHECKING(if LIBZ library is wanted)
	wanted=1;
	if test -z "$with_libz" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$enable_shared" = "no"; then
			with_libz="no";
		else
			with_libz="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libz, [  --with-libz             use system     LIBZ library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libzdir="$dir"
			if test -f "$dir/include/zlib.h"; then
				found_libz="yes";
				break;
			fi
		done
		if test x_$found_libz != x_yes; then
			msg="Cannot find libz library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libz found in $libzdir${T_ME}"
			USE_LIBZ=yes
			AC_DEFINE(USE_LIBZ, 1, [Define if enable libz support])
			libz_version=$(grep ZLIB_VERSION $libzdir/include/zlib.h 2>/dev/null | head -n1 | cut -d'"' -f2 2>/dev/null)
			if test -z "${libz_version}"; then
				libz_version="unknown"
			fi
         ULIB_LIBS="$ULIB_LIBS -lz";
			if test $libzdir != "${CROSS_ENVIRONMENT}/" -a $libzdir != "${CROSS_ENVIRONMENT}/usr" -a $libzdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libzdir/include"
				LDFLAGS="$LDFLAGS -L$libzdir/lib -Wl,-R$libzdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libzdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if you want to enable build of ZIP support)
	if test x_$found_libz = x_yes; then
		if test -z "$enable_zip" -o "x_$enable_zip" != "x_no"; then
			enable_zip="yes"
		fi
	else
		enable_zip="no"
	fi
	AC_ARG_ENABLE(zip, [  --enable-zip              enable build of ZIP support - require libz [[default: use if present libz]]])
	AC_MSG_RESULT([$enable_zip])

	AC_MSG_CHECKING(if zopfli library is wanted)
	wanted=1;
	if test -z "$with_libzopfli" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO" -o "$enable_shared" = "no"; then
			with_libzopfli="no";
		else
			with_libzopfli="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libzopfli, [  --with-libzopfli        use system   zopfli library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libzopflidir="$dir"
			if test -f "$dir/include/zopfli.h"; then
				found_libzopfli="yes";
				break;
			fi
		done
		if test x_$found_libzopfli != x_yes; then
			msg="Cannot find libzopfli library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libzopfli found in $libzopflidir${T_ME}"
			USE_LIBZOPFLI=yes
			AC_DEFINE(USE_LIBZOPFLI, 1, [Define if enable libzopfli support])
			libzopfli_version=$(ls $libzopflidir/lib*/libzopfli.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			if test -z "${libzopfli_version}"; then
				libzopfli_version="unknown"
			fi
         ULIB_LIBS="$ULIB_LIBS -lzopfli";
			if test $libzopflidir != "${CROSS_ENVIRONMENT}/" -a $libzopflidir != "${CROSS_ENVIRONMENT}/usr" -a $libzopflidir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libzopflidir/include"
				LDFLAGS="$LDFLAGS -L$libzopflidir/lib -Wl,-R$libzopflidir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libzopflidir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if brotli library is wanted)
	wanted=1;
	if test -z "$with_libbrotli" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO" -o "$enable_shared" = "no"; then
			with_libbrotli="no";
		else
			with_libbrotli="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libbrotli, [  --with-libbrotli        use system   brotli library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libbrotlidir="$dir"
			if test -f "$dir/include/brotli/encode.h"; then
				found_libbrotli="yes";
				break;
			fi
		done
		if test x_$found_libbrotli != x_yes; then
			msg="Cannot find libbrotli library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libbrotli found in $libbrotlidir${T_ME}"
			USE_LIBBROTLI=yes
			AC_DEFINE(USE_LIBBROTLI, 1, [Define if enable libbrotli support])

			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libbrotli_version=$(pkg-config --modversion libbrotlienc 2>/dev/null)
			fi
			if test -z "${libbrotli_version}"; then
				libbrotli_version=$(ls $libbrotlidir/lib*/libbrotli*.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "${libbrotli_version}"; then
				libbrotli_version="unknown"
			fi
         ULIB_LIBS="$ULIB_LIBS -lbrotlidec -lbrotlienc";
			if test $libbrotlidir != "${CROSS_ENVIRONMENT}/" -a $libbrotlidir != "${CROSS_ENVIRONMENT}/usr" -a $libbrotlidir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libbrotlidir/include"
				LDFLAGS="$LDFLAGS -L$libbrotlidir/lib -Wl,-R$libbrotlidir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libbrotlidir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if quiche library is wanted)
	wanted=1;
	if test -z "$with_libquiche" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO" -o "$enable_shared" = "no"; then
			with_libquiche="no";
		else
			with_libquiche="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libquiche, [  --with-libquiche        use system   quiche library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libquichedir="$dir"
			if test -f "$dir/include/quiche.h"; then
				found_libquiche="yes";
				break;
			fi
		done
		if test x_$found_libquiche != x_yes; then
			msg="Cannot find libquiche library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libquiche found in $libquichedir${T_ME}"
			USE_LIBQUICHE=yes
			AC_DEFINE(USE_LIBQUICHE, 1, [Define if enable libquiche support])

			pkg-config --atleast-version=0.5 quiche 2>/dev/null
			if [[ "$?" -eq 0 ]]; then
				AC_DEFINE(LIBQUICHE_AT_LEAST_0_5, 1, [Define if we have at least libquiche 0.5])
			fi

			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libquiche_version=$(pkg-config --modversion quiche 2>/dev/null)
			fi
			if test -z "${libquiche_version}"; then
				libquiche_version=$(ls $libquichedir/lib*/libquiche.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "${libquiche_version}"; then
				libquiche_version="unknown"
			fi
			ULIB_LIBS="$ULIB_LIBS -lquiche";
			if test $libquichedir != "${CROSS_ENVIRONMENT}/" -a $libquichedir != "${CROSS_ENVIRONMENT}/usr" -a $libquichedir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libquichedir/include"
				LDFLAGS="$LDFLAGS -L$libquichedir/lib -Wl,-R$libquichedir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libquichedir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if mimalloc library is wanted)
	wanted=1;
	if test -z "$with_libmimalloc" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO" -o "$enable_shared" = "no"; then
			with_libmimalloc="no";
		else
			with_libmimalloc="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libmimalloc, [  --with-libmimalloc      use system mimalloc library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libmimallocdir="$dir"
			if test -f "$dir/include/mimalloc.h"; then
				found_libmimalloc="yes";
				break;
			fi
		done
		if test x_$found_libmimalloc != x_yes; then
			msg="Cannot find libmimalloc library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libmimalloc found in $libmimallocdir${T_ME}"
			USE_LIBMIMALLOC=yes
			AC_DEFINE(USE_LIBMIMALLOC, 1, [Define if enable libmimalloc support])

			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libmimalloc_version=$(pkg-config --modversion mimalloc 2>/dev/null)
			fi
			if test -z "${libmimalloc}"; then
				libmimalloc_version=$(ls $libmimallocdir/lib*/libmimalloc.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "${libmimalloc_version}"; then
				libmimalloc_version="unknown"
			fi
			ULIB_LIBS="$ULIB_LIBS -lmimalloc";
			if test $libmimallocdir != "${CROSS_ENVIRONMENT}/" -a $libmimallocdir != "${CROSS_ENVIRONMENT}/usr" -a $libmimallocdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libmimallocdir/include"
				LDFLAGS="$LDFLAGS -L$libmimallocdir/lib -Wl,-R$libmimallocdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libmimallocdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if uring library is wanted)
	wanted=1;
	if test -z "$with_liburing" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO" -o "$enable_shared" = "no"; then
			with_liburing="no";
		else
			with_liburing="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(liburing, [  --with-liburing         use system    uring library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			liburingdir="$dir"
			if test -f "$dir/include/liburing.h"; then
				found_liburing="yes";
				break;
			fi
		done
		if test x_$found_liburing != x_yes; then
			msg="Cannot find liburing library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}liburing found in $liburingdir${T_ME}"
			USE_LIBURING=yes
			AC_DEFINE(USE_LIBURING, 1, [Define if enable liburing support])

			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				liburing_version=$(pkg-config --modversion liburing 2>/dev/null)
			fi
			if test -z "${liburing_version}"; then
				liburing_version=$(ls $liburingdir/lib*/liburing.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "${liburing_version}"; then
				liburing_version="unknown"
			fi
			ULIB_LIBS="$ULIB_LIBS -luring";
			if test $liburingdir != "${CROSS_ENVIRONMENT}/" -a $liburingdir != "${CROSS_ENVIRONMENT}/usr" -a $liburingdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$liburingdir/include"
				LDFLAGS="$LDFLAGS -L$liburingdir/lib -Wl,-R$liburingdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$liburingdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if argon2 library is wanted)
	wanted=1;
	if test -z "$with_libargon2" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO" -o "$enable_shared" = "no"; then
			with_libargon2="no";
		else
			with_libargon2="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libargon2, [  --with-libargon2        use system   argon2 library - [[will check /usr /usr/local]] [[default=use if present]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libargon2dir="$dir"
			if test -f "$dir/include/argon2.h"; then
				found_libargon2="yes";
				break;
			fi
		done
		if test x_$found_libargon2 != x_yes; then
			msg="Cannot find libargon2 library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libargon2 found in $libargon2dir${T_ME}"
			USE_LIBARGON2=yes
			AC_DEFINE(USE_LIBARGON2, 1, [Define if enable libargon2 support])

			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libargon2_version=$(pkg-config --modversion libargon2 2>/dev/null)
			fi
			if test -z "${libargon2_version}"; then
				libargon2_version=$(ls $libargon2dir/lib*/libargon2.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "${libargon2_version}"; then
				libargon2_version="unknown"
			fi
			ULIB_LIBS="$ULIB_LIBS -largon2";
			if test $libargon2dir != "${CROSS_ENVIRONMENT}/" -a $libargon2dir != "${CROSS_ENVIRONMENT}/usr" -a $libargon2dir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libargon2dir/include"
				LDFLAGS="$LDFLAGS -L$libargon2dir/lib -Wl,-R$libargon2dir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libargon2dir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if MAGIC library is wanted)
	wanted=1;
	if test -z "$with_magic" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$enable_shared" = "no"; then
			with_magic="no";
		else
			with_magic="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(magic, [  --with-magic            use system libmagic library - [[will check /usr /usr/local]] [[default=use if present]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			magicdir="$dir";
			if test -f "$dir/include/magic.h"; then
				found_magic="yes";
				break;
			fi
		done
		if test x_$found_magic != x_yes; then
			msg="Cannot find libmagic library"
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libmagic found in $magicdir${T_ME}"
			USE_LIBMAGIC=yes
			AC_DEFINE(USE_LIBMAGIC, 1, [Define if enable libmagic support])
			libmagic_version=$($magicdir/bin/file --version 2>&1 | head -n 1 | cut -d'-' -f2)
			if test -z "${libmagic_version}"; then
				libmagic_version="unknown"
			fi
         ULIB_LIBS="-lmagic $ULIB_LIBS";
			if test $magicdir != "${CROSS_ENVIRONMENT}/" -a $magicdir != "${CROSS_ENVIRONMENT}/usr" -a $magicdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$magicdir/include";
				LDFLAGS="$LDFLAGS -L$magicdir/lib -Wl,-R$magicdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$magicdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if SSL library is wanted)
	wanted=1;
	if test -z "$with_ssl" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$enable_shared" = "no"; then
			with_ssl="no";
		else
			with_ssl="${CROSS_ENVIRONMENT}/usr";
		fi
	fi

   AC_ARG_ENABLE(ssl-staticlib-deps, [  --enable-ssl-staticlib-deps  link with dependencies of OpenSSL's static libraries. Must be specified in addition to --with-ssl [[default=no]]])
   if test -z "$enable_ssl_staticlib_deps"; then
      enable_ssl_staticlib_deps="no"
   fi

	AC_ARG_WITH(ssl, [  --with-ssl              use system      SSL library - [[will check /usr /usr/local]] [[default=use if present]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			ssldir="$dir";
			if test -f "$dir/include/openssl/ssl.h"; then
				found_ssl="yes";
				break;
			fi
			if test -f "$dir/include/cyassl/openssl/ssl.h"; then
				found_ssl="yes";
				found_cyassl="yes";
				break;
			fi
		done
		if test x_$found_ssl != x_yes; then
			msg="Cannot find libssl library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			USE_LIBSSL=yes
			AC_DEFINE(USE_LIBSSL, 1, [Define if enable libssl support])
			if test "$found_cyassl" = "yes"; then
				echo "${T_MD}CYASSL found in $ssldir${T_ME}";
				ssl_version=$(grep VERSION $ssldir/include/cyassl/openssl/opensslv.h 2>/dev/null | cut -d' ' -f3 | tr -d '\r\n');
				ULIB_LIBS="-lcyassl $ULIB_LIBS";
			else
				echo "${T_MD}libssl found in $ssldir${T_ME}";
				if test -f "$ssldir/include/openssl/ts.h"; then
					HAVE_SSL_TS="yes";
					AC_DEFINE(HAVE_SSL_TS, 1, [Define if we have time stamp support in openssl])
				fi
				ssl_version=$($ssldir/bin/openssl version 2>/dev/null)
				if test -z "${ssl_version}"; then
					ssl_version=$(grep OPENSSL_VERSION_TEXT $ssldir/include/openssl/opensslv.h 2>/dev/null | grep -v fips | head -1 | cut -d'"' -f2);
				fi
				if test -z "${ssl_version}" -a -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
					ssl_version=$(pkg-config --modversion openssl 2>/dev/null)
				fi
				if test "$enable_ssl_staticlib_deps" != "yes"; then
					ULIB_LIBS="-lssl -lcrypto $OPENSSL_LINK $ULIB_LIBS";
				else
					ULIB_LIBS="$ssldir/lib/libssl.a $ssldir/lib/libcrypto.a $OPENSSL_LINK $ULIB_LIBS";
				fi
			fi
			if test -z "${ssl_version}"; then
				ssl_version="unknown";
			fi
			if test $ssldir != "${CROSS_ENVIRONMENT}/" -a $ssldir != "${CROSS_ENVIRONMENT}/usr" -a $ssldir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$ssldir/include";
				LDFLAGS="$LDFLAGS -L$ssldir/lib -Wl,-R$ssldir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$ssldir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if PCRE library is wanted)
	wanted=1;
	if test -z "$with_pcre" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$enable_shared" = "no"; then
			with_pcre="no";
		else
			with_pcre="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(pcre, [  --with-pcre             use system     PCRE library - [[will check /usr /usr/local]] [[default=use if present]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			pcredir="$dir"
			if test -f "$dir/include/pcre.h"; then
				found_pcre="yes";
				break;
			fi
		done
		if test x_$found_pcre != x_yes; then
			msg="Cannot find libpcre library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libpcre found in $pcredir${T_ME}"
			USE_LIBPCRE=yes
			AC_DEFINE(USE_LIBPCRE, 1, [Define if enable libpcre support])
			pcre_version=$($pcredir/bin/pcre-config --version)
			if test -z "${pcre_version}"; then
				pcre_version="unknown"
			fi
			ULIB_LIBS="-lpcre $ULIB_LIBS";
			if test $pcredir != "${CROSS_ENVIRONMENT}/" -a $pcredir != "${CROSS_ENVIRONMENT}/usr" -a $pcredir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$pcredir/include"
				LDFLAGS="$LDFLAGS -L$pcredir/lib -Wl,-R$pcredir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$pcredir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if EXPAT library for XML parsing is wanted)
	wanted=1;
	if test -z "$with_expat" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$enable_shared" = "no"; then
			with_expat="no";
		else
			with_expat="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(expat, [  --with-expat            use system    EXPAT library - [[will check /usr /usr/local]] [[default=use if present]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			expatdir="$dir"
			if test -f "$dir/include/expat.h"; then
				found_expat="yes";
				break;
			fi
		done
		if test x_$found_expat != x_yes; then
			msg="Cannot find libexpat library"
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libexpat found in $expatdir${T_ME}"
			USE_LIBEXPAT=yes
			AC_DEFINE(USE_LIBEXPAT, 1, [Define if enable libexpat support])
			expat_version=$(strings $expatdir/lib*/libexpat.* 2>/dev/null | grep "^expat_[[0-9]]*" | head -n1 | cut -d'_' -f2)
			if test -z "${expat_version}"; then
				expat_version=$(ls $expatdir/libexpat.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			fi
			if test -z "${expat_version}"; then
				expat_version="unknown"
			fi
			ULIB_LIBS="-lexpat $ULIB_LIBS";
			if test $expatdir != "${CROSS_ENVIRONMENT}/" -a $expatdir != "${CROSS_ENVIRONMENT}/usr" -a $expatdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$expatdir/include"
				LDFLAGS="$LDFLAGS -L$expatdir/lib -Wl,-R$expatdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$expatdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if libuuid library is wanted)
	wanted=1;
	if test -z "$with_libuuid" ; then
		wanted=0;
		if test -n "$CROSS_ENVIRONMENT" -o "$enable_shared" = "no"; then
			with_libuuid="no";
		else
			with_libuuid="${CROSS_ENVIRONMENT}/usr";
		fi
	fi
	AC_ARG_WITH(libuuid, [  --with-libuuid          use system  libuuid library - [[will check /usr /usr/local]] [[default=use if present]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libuuiddir="$dir";
			if test -f "$dir/include/uuid/uuid.h"; then
				found_libuuid="yes";
				break;
			fi
		done
		if test x_$found_libuuid != x_yes; then
			msg="Cannot find libuuid library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libuuid found in $libuuiddir${T_ME}"
			USE_LIBUUID=yes
			AC_DEFINE(USE_LIBUUID, 1, [Define if enable libuuid support])
			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libuuid_version=$(pkg-config --modversion ext2fs 2>/dev/null)
			fi
			if test -z "${libuuid_version}"; then
				libuuid_version="unknown"
			fi
         ULIB_LIBS="-luuid $ULIB_LIBS";
			if test $libuuiddir != "${CROSS_ENVIRONMENT}/" -a $libuuiddir != "${CROSS_ENVIRONMENT}/usr" -a $libuuiddir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libuuiddir/include";
				LDFLAGS="$LDFLAGS -L$libuuiddir/lib -Wl,-R$libuuiddir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libuuiddir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if cURL library is wanted)
	wanted=1;
	if test -z "$with_curl" ; then
		wanted=0;
		with_curl="no";
	fi

   AC_ARG_ENABLE(curl-staticlib-deps,
		[  --enable-curl-staticlib-deps link with dependencies of    cURL's static libraries. Must be specified in addition to --with-curl [[default=no]]])
   if test -z "$enable_curl_staticlib_deps"; then
      enable_curl_staticlib_deps="no"
   fi

	AC_ARG_WITH(curl, [  --with-curl             use system     cURL library - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			curldir="$dir";
			if test -f "$dir/include/curl/curl.h"; then
				found_curl="yes";
				break;
			fi
		done
		if test x_$found_curl != x_yes; then
			AC_MSG_ERROR(Cannot find libcurl library)
		else
			echo "${T_MD}libcurl found in $curldir${T_ME}"
			USE_LIBCURL=yes
			AC_DEFINE(USE_LIBCURL, 1, [Define if enable libcurl support])
			libcurl_version=$($curldir/bin/curl-config --version 2>/dev/null)
			if test -z "${libcurl_version}" -a -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libcurl_version=$(pkg-config --modversion libcurl 2>/dev/null)
			fi
			if test -z "${libcurl_version}"; then
				libcurl_version="unknown"
			fi
			if test "$enable_curl_staticlib_deps" = "yes"; then
				libcurl_linking="$curldir/lib/libcurl.a -lz"
			else
				libcurl_linking=$($curldir/bin/curl-config --libs 2>/dev/null)
			fi
			if test -z "${libcurl_linking}"; then
				libcurl_linking="-lcurl"
			fi
			ULIB_LIBS="$libcurl_linking $ULIB_LIBS";
			if test $curldir != "${CROSS_ENVIRONMENT}/" -a $curldir != "${CROSS_ENVIRONMENT}/usr" -a $curldir != "${CROSS_ENVIRONMENT}/usr/local"; then
				libcurl_cflags=$($curldir/bin/curl-config --cflags 2>/dev/null)
				CPPFLAGS="$CPPFLAGS $libcurl_cflags";
				LDFLAGS="$LDFLAGS -L$curldir/lib -Wl,-R$curldir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$curldir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if tdb library is wanted)
	wanted=1;
	if test -z "$with_libtdb" ; then
		wanted=0;
		with_libtdb="no";
	fi
	AC_ARG_WITH(libtdb,    [  --with-libtdb           use system      tdb library - [[will check /usr /usr/local]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libtdbdir="$dir"
			if test -f "$dir/include/tdb.h"; then
				found_libtdb="yes";
				break;
			fi
		done
		if test x_$found_libtdb != x_yes; then
			msg="Cannot find libtdb library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libtdb found in $libtdbdir${T_ME}"
			USE_LIBTDB=yes
			AC_DEFINE(USE_LIBTDB, 1, [Define if enable libtdb support])
			libtdb_version=$(ls $libtdbdir/lib*/libtdb.so.*.* 2>/dev/null | head -n 1 | awk -F'.so.' '{n=2; print $n}' 2>/dev/null)
			if test -z "${libtdb_version}"; then
				libtdb_version="unknown"
			fi
         ULIB_LIBS="$ULIB_LIBS -ltdb";
			if test $libtdbdir != "${CROSS_ENVIRONMENT}/" -a $libtdbdir != "${CROSS_ENVIRONMENT}/usr" -a $libtdbdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libtdbdir/include"
				LDFLAGS="$LDFLAGS -L$libtdbdir/lib -Wl,-R$libtdbdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libtdbdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if c-ares library for DNS resolution is wanted)
	wanted=1;
	if test -z "$with_libares" ; then
		wanted=0;
		with_libares="no";
	fi
	AC_ARG_WITH(libares, [  --with-libares          use system   c-ares library - [[will check /usr /usr/local]]], [
	if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			caresdir="$dir"
			if test -f "$dir/include/ares.h"; then
				found_cares="yes";
				break;
			fi
		done
		if test x_$found_cares != x_yes; then
			msg="Cannot find libcares library";
			if test $wanted = 1; then
				AC_MSG_ERROR($msg)
			else
				AC_MSG_RESULT($msg)
			fi
		else
			echo "${T_MD}libcares found in $caresdir${T_ME}"
			USE_C_ARES=yes
			AC_DEFINE(USE_C_ARES, 1, [Define if enable c-ares support])
			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				cares_version=$(pkg-config --modversion libcares 2>/dev/null)
			fi
			if test -z "${cares_version}"; then
				cares_version="unknown"
			fi
         ULIB_LIBS="-lcares $ULIB_LIBS";
			if test $caresdir != "${CROSS_ENVIRONMENT}/" -a $caresdir != "${CROSS_ENVIRONMENT}/usr" -a $caresdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$caresdir/include"
				LDFLAGS="$LDFLAGS -L$caresdir/lib -Wl,-R$caresdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$caresdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if SSH library is wanted)
	wanted=1;
	if test -z "$with_ssh" ; then
		wanted=0;
		with_ssh="no";
	fi
	AC_ARG_WITH(ssh, [  --with-ssh              use system      SSH library - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			sshdir="$dir";
			if test -f "$dir/include/libssh/libssh.h"; then
				found_ssh="yes";
				break;
			fi
		done
		if test x_$found_ssh != x_yes; then
			AC_MSG_ERROR(Cannot find libssh library)
		else
			echo "${T_MD}libssh found in $sshdir${T_ME}"
			USE_LIBSSH=yes
			AC_DEFINE(USE_LIBSSH, 1, [Define if enable libssh support])
dnl		libssh_version=$(grep LIBSFTP_VERSION $sshdir/include/libssh/sftp.h | cut -d' ' -f3)
			libssh_version=$(strings $sshdir/lib*/libssh.so 2>/dev/null | grep 'libssh-[[0-9]]' | head -n1 | cut -d'-' -f4)
			if test -z "${libssh_version}"; then
				libssh_version="unknown"
			fi
			ULIB_LIBS="-lssh $ULIB_LIBS";
			if test $sshdir != "${CROSS_ENVIRONMENT}/" -a $sshdir != "${CROSS_ENVIRONMENT}/usr" -a $sshdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$sshdir/include";
				LDFLAGS="$LDFLAGS -L$sshdir/lib -Wl,-R$sshdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$sshdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if LDAP library is wanted)
	wanted=1;
	if test -z "$with_ldap" ; then
		wanted=0;
		with_ldap="no";
	fi
	AC_ARG_WITH(ldap, [  --with-ldap             use system openLDAP library - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			ldapdir="$dir"
			if test -f "$dir/include/ldap.h"; then
				found_ldap="yes"
				LDAP_INCS="$ldapdir/include"
				LDAP_LDFLAGS="$ldapdir/lib"
				LDAP_LIBS="-lldap -llber"
				break
			fi
			if test -f "$dir/include/mozilla/ldap/ldap.h"; then
				found_ldap="yes"
				LDAP_INCS="$ldapdir/include/mozilla/ldap"
				LDAP_LDFLAGS="$ldapdir/lib/mozilla"
				LDAP_LIBS="-lldap50"
				break
			fi
			if test -f "$dir/include/winldap.h"; then
				found_ldap="yes"
				LDAP_INCS="$ldapdir/include"
				LDAP_LDFLAGS="$ldapdir/lib"
				LDAP_LIBS="-lwldap32"
				CPPFLAGS="$CPPFLAGS -DHAVE_WINLDAP_H -DHAVE_LDAP_SSL_H"
				break
			fi
		done
		if test x_$found_ldap != x_yes; then
			AC_MSG_ERROR(Cannot find libldap include)
		else
			echo "${T_MD}libldap found in $ldapdir${T_ME}"
			USE_LIBLDAP=yes
			AC_DEFINE(USE_LIBLDAP, 1, [Define if enable libldap support])
			if test -f "$LDAP_INCS/ldap_ssl.h"; then
				CPPFLAGS="$CPPFLAGS -DHAVE_LDAP_SSL_H"
			fi
dnl		ldap_version=$(strings $LDAP_LDFLAGS/libldap.so 2>/dev/null | grep "@(#)")
dnl		ldap_version=$(ldapsearch -VV 2>&1 | tail -n1 | cut -d':' -f2 | cut -d')' -f1)
			ldap_version=$(grep LDAP_API_VERSION ${LDAP_INCS}/*ldap*.h | awk '{print $NF}')
			if test -z "${ldap_version}"; then
				ldap_version="unknown"
			fi
			ULIB_LIBS="$LDAP_LIBS $ULIB_LIBS";
			if test $ldapdir != "${CROSS_ENVIRONMENT}/" -a $ldapdir != "${CROSS_ENVIRONMENT}/usr" -a $ldapdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$LDAP_INCS"
				LDFLAGS="$LDFLAGS -L$LDAP_LDFLAGS -Wl,-R$LDAP_LDFLAGS"
				PRG_LDFLAGS="$PRG_LDFLAGS -L$ldapdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if DBI library is wanted)
	wanted=1;
	if test -z "$with_dbi" ; then
		wanted=0;
		with_dbi="no";
	fi
	AC_ARG_WITH(dbi, [  --with-dbi              use system      DBI library - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			dbidir="$dir";
			if test -f "$dir/include/dbi/dbi.h"; then
				found_dbi="yes";
				break;
			fi
		done
		if test x_$found_dbi != x_yes; then
			AC_MSG_ERROR(Cannot find libdbi library)
		else
			echo "${T_MD}libdbi found in $dbidir${T_ME}"
			USE_LIBDBI=yes
			AC_DEFINE(USE_LIBDBI, 1, [Define if enable libdbi support])
			libdbi_version=$(strings $dbidir/lib*/libdbi.* 2>/dev/null | grep "^libdbi v[[0-9]]" | cut -d'v' -f2 | head -n1)
			if test -z "${libdbi_version}"; then
				libdbi_version="unknown"
			fi
			ULIB_LIBS="-ldbi $ULIB_LIBS";
			if test $dbidir != "${CROSS_ENVIRONMENT}/" -a $dbidir != "${CROSS_ENVIRONMENT}/usr" -a $dbidir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$dbidir/include";
				LDFLAGS="$LDFLAGS -L$dbidir/lib -Wl,-R$dbidir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$dbidir/lib";
			else
				if ! test -f $dbidir/lib64/libdbi.so -a -f $dbidir/lib64/dbi/libdbi.so; then
					LDFLAGS="$LDFLAGS -L$dbidir/lib64/dbi -Wl,-R$dbidir/lib64/dbi";
					PRG_LDFLAGS="$PRG_LDFLAGS -L$dbidir/lib64/dbi";
				fi
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if libevent library is wanted (deprecated))
	wanted=1;
	if test -z "$with_libevent" ; then
		wanted=0;
		with_libevent="no";
	fi
	AC_ARG_WITH(libevent, [  --with-libevent         use system libevent library (deprecated) - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libeventdir="$dir";
			if test -f "$dir/include/event.h"; then
				found_libevent="yes";
				break;
			fi
		done
		if test x_$found_libevent != x_yes; then
			AC_MSG_ERROR(Cannot find libevent library)
		else
			echo "${T_MD}libevent found in $libeventdir${T_ME}"
			USE_LIBEVENT=yes
			AC_DEFINE(USE_LIBEVENT, 1, [Define if enable libevent support])
			libevent_version=$(strings $libeventdir/lib*/libevent* 2>/dev/null | grep "^libevent-[[0-9]]" | head -n1 | cut -d'-' -f2 | awk -F'.so' '{n=1; print $n}')
			if test -z "${libevent_version}"; then
				libevent_version="unknown"
			fi
			ULIB_LIBS="-levent $ULIB_LIBS";
			if test $libeventdir != "${CROSS_ENVIRONMENT}/" -a $libeventdir != "${CROSS_ENVIRONMENT}/usr" -a $libeventdir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libeventdir/include";
				LDFLAGS="$LDFLAGS -L$libeventdir/lib -Wl,-R$libeventdir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libeventdir/lib";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if libxml2 library is wanted)
	wanted=1;
	if test -z "$with_libxml2" ; then
		wanted=0;
		with_libxml2="no";
	fi
	AC_ARG_WITH(libxml2, [  --with-libxml2          use system  libxml2 library - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			libxml2dir="$dir";
			if test -f "$dir/include/libxml2/libxml/valid.h"; then
				found_libxml2="yes";
				break;
			fi
		done
		if test x_$found_libxml2 != x_yes; then
			AC_MSG_ERROR(Cannot find libxml2 library)
		else
			echo "${T_MD}libxml2 found in $libxml2dir${T_ME}"
			USE_LIBXML2=yes
			AC_DEFINE(USE_LIBXML2, 1, [Define if enable libxml2 support])
			if test -z "$CROSS_ENVIRONMENT" -a x_$PKG_CONFIG != x_no; then
				libxml2_version=$(pkg-config --modversion libxml-2.0 2>/dev/null)
			fi
			if test -z "${libxml2_version}"; then
				libxml2_version="unknown"
			fi
			ULIB_LIBS="-lxml2 $ULIB_LIBS";
			if test $libxml2dir != "${CROSS_ENVIRONMENT}/" -a $libxml2dir != "${CROSS_ENVIRONMENT}/usr" -a $libxml2dir != "${CROSS_ENVIRONMENT}/usr/local"; then
				CPPFLAGS="$CPPFLAGS -I$libxml2dir/include/libxml2";
				LDFLAGS="$LDFLAGS -L$libxml2dir/lib -Wl,-R$libxml2dir/lib";
				PRG_LDFLAGS="$PRG_LDFLAGS -L$libxml2dir/lib";
			else
				CPPFLAGS="$CPPFLAGS -I/usr/include/libxml2";
			fi
		fi
	fi
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if you want to use page-speed SDK)
	wanted=1;
	if test -z "$with_page_speed" ; then
		wanted=0;
		with_page_speed="no";
	fi
	AC_ARG_WITH(page-speed, [  --with-page-speed       use google page-speed SDK   - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			page_speeddir="$dir";
			if test -d $dir/page-speed-*; then
				found_page_speed="yes";
				break;
			fi
		done
		if test x_$found_page_speed != x_yes; then
			AC_MSG_ERROR("Cannot find page-speed SDK");
		else
			echo "${T_MD}page-speed SDK found in $page_speeddir${T_ME}"
			USE_PAGE_SPEED=yes
			AC_DEFINE(USE_PAGE_SPEED, 1, [Define if enable libpagespeed support])
			page_speed_version=$(ls -1 $page_speeddir 2>/dev/null | grep page-speed | cut -d'-' -f3)
			PAGESPEED_ROOT_DIR=$page_speeddir/page-speed-$page_speed_version
		fi
	fi
   AC_SUBST(PAGESPEED_ROOT_DIR)
	], [AC_MSG_RESULT(no)])

	AC_MSG_CHECKING(if you want to use V8 JavaScript Engine)
	wanted=1;
	if test -z "$with_v8-javascript" ; then
		wanted=0;
		with_v8-javascript="no";
	fi
	AC_ARG_WITH(v8-javascript, [  --with-v8-javascript    use V8 JavaScript Engine    - [[will check /usr /usr/local]]],
	[if test "$withval" = "no"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
		for dir in $withval ${CROSS_ENVIRONMENT}/ ${CROSS_ENVIRONMENT}/usr ${CROSS_ENVIRONMENT}/usr/local; do
			v8dir="$dir";
			if test -f "$dir/include/v8.h"; then
				found_v8="yes";
				break;
			fi
		done
		if test x_$found_v8 != x_yes; then
			AC_MSG_ERROR("Cannot find V8 JavaScript Engine");
		else
			echo "${T_MD}V8 JavaScript Engine found in $v8dir${T_ME}"
			USE_LIBV8=yes
			AC_DEFINE(USE_LIBV8, 1, [Define if enable libv8 support])
			v8_version=$(strings $v8dir/lib*/libv8.so 2>/dev/null | grep -i 'libv8' | head -n1 | cut -b10-)
		fi
	fi
	], [AC_MSG_RESULT(no)])
])

AC_DEFUN([AC_CHECK_SQLITE3], [
ac_sqlite3="no"
ac_sqlite3_incdir="no"
ac_sqlite3_libdir="no"

# exported variables
SQLITE3_LIBS=""
SQLITE3_LDFLAGS=""
SQLITE3_INCLUDE=""

AC_MSG_CHECKING(for SQLite3 support)

AC_ARG_WITH(sqlite3,
   [  --with-sqlite3          Include SQLite3 support],
   [  ac_sqlite3="$withval" ])
AC_ARG_WITH(sqlite3-incdir,
   [  --with-sqlite3-incdir   Specifies where the SQLite3 include files are],
   [  ac_sqlite3_incdir="$withval" ])
AC_ARG_WITH(sqlite3-libdir,
   [  --with-sqlite3-libdir   Specifies where the SQLite3 libraries are],
   [  ac_sqlite3_libdir="$withval" ])

if test "$ac_sqlite3" != "yes"; then
   AC_MSG_RESULT(no)
	AM_CONDITIONAL(HAVE_SQLITE3, false)
else
   AC_MSG_RESULT([yes])
   if test "$ac_sqlite3_incdir" = "no"; then
      AC_CHECK_HEADER([sqlite3.h])
      SQLITE3_INCLUDE=""
		libsqlite_version=$(grep SQLITE_VERSION /usr/include/sqlite3.h 2>/dev/null | grep '"' | cut -d'"' -f2)
   else
      SQLITE3_INCLUDE=-I$ac_sqlite3_incdir
		libsqlite_version=$(grep SQLITE_VERSION $ac_sqlite3_incdir/sqlite3.h 2>/dev/null | grep '"' | cut -d'"' -f2)
   fi
   if test "$ac_sqlite3_libdir" = "no"; then
      AC_SEARCH_LIBS_VAR([sqlite3_exec], sqlite3, , , , SQLITE3_LIBS)
      SQLITE3_LDFLAGS=""
   else
      SQLITE3_LIBS=-lsqlite
      SQLITE3_LDFLAGS=-L$ac_sqlite3_libdir
   fi

	USE_SQLITE=yes
	AC_DEFINE(USE_SQLITE, 1, [Define if enable SQLite database support])
	if test -z "${libsqlite_version}"; then
		libsqlite_version="unknown"
	fi

	AC_MSG_CHECKING(for SQLite3 includes)
	AC_MSG_RESULT($SQLITE3_INCLUDE)
	AC_MSG_CHECKING(for SQLite3 libraries)
	AC_MSG_RESULT($SQLITE3_LIBS)

   AC_SUBST(SQLITE3_LIBS)
   AC_SUBST(SQLITE3_INCLUDE)
   AC_SUBST(SQLITE3_LDFLAGS)

   AM_CONDITIONAL(HAVE_SQLITE3, true)
fi
])

AC_DEFUN([AC_CHECK_MYSQL], [
ac_mysql="no"
ac_mysql_incdir="no"
ac_mysql_libdir="no"

# exported variables
MYSQL_LIBS=""
MYSQL_LDFLAGS=""
MYSQL_INCLUDE=""

AC_MSG_CHECKING(for MySQL support)

AC_ARG_WITH(mysql,
	[  --with-mysql            Include MySQL support],
	[  ac_mysql="$withval" ])
AC_ARG_WITH(mysql-incdir,
	[  --with-mysql-incdir     Specifies where the MySQL include files are],
	[  ac_mysql_incdir="$withval" ])
AC_ARG_WITH(mysql-libdir,
	[  --with-mysql-libdir     Specifies where the MySQL libraries are],
	[  ac_mysql_libdir="$withval" ])

ac_mysql_threadsafe="NO"
AC_ARG_ENABLE(mysql-threadsafe,
	[  --enable-mysql-threadsafe use threadsafe version of libmysqlclient],
	[  if test "$enable_mysql_threadsafe" = "yes"; then
	       ac_mysql_threadsafe="YES"
	   fi])

if test "$ac_mysql" != "yes"; then
	AC_MSG_RESULT(no)
	AM_CONDITIONAL(HAVE_MYSQL, false)
else
   AC_MSG_RESULT(yes)
	if test "$ac_mysql_incdir" = "no" || test "$ac_mysql_libdir" = "no"; then
	   	AC_CHECK_PROG([MYSQL_CONFIG], [mysql_config], [yes], [no])
		if test "$MYSQL_CONFIG" = "no"; then
		   AC_MSG_ERROR([cannot auto-configure MySQL without mysql_config])
		fi
	fi

	if test "$ac_mysql_incdir" = "no"; then
		MYSQL_INCLUDE=`mysql_config --include`
	else
		MYSQL_INCLUDE=$ac_mysql_incdir
	fi
	if test "$ac_mysql_libdir" = "no"; then
	   	if test "$ac_mysql_threadsafe" = "YES"; then
			MYSQL_LIBS="`mysql_config --libs_r`"
		else
			MYSQL_LIBS="`mysql_config --libs`"
		fi
	else
	  	if test "$ac_mysql_threadsafe" = "YES"; then
			MYSQL_LIBS="-L$ac_mysql_libdir -lmysqlclient_r"
		else
			MYSQL_LIBS="-L$ac_mysql_libdir -lmysqlclient"
		fi
	fi

	USE_MYSQL=yes
	AC_DEFINE(USE_MYSQL, 1, [Define if enable MySQL database support])
	libmysql_version=`mysql_config --version`

	if test -z "${libmysql_version}"; then
		libmysql_version="unknown"
	fi

	AC_MSG_CHECKING(for MySQL includes)
	AC_MSG_RESULT($MYSQL_INCLUDE)
	AC_MSG_CHECKING(for MySQL libraries)
	AC_MSG_RESULT($MYSQL_LIBS)

	AC_SUBST(MYSQL_LIBS)
	AC_SUBST(MYSQL_INCLUDE)
	AC_SUBST(MYSQL_LDFLAGS)

	AM_CONDITIONAL(HAVE_MYSQL, true)
fi
])
