dnl @synopsis AC_COMPILATION_OPTIONS
dnl Macros that add compilation options to a `configure' script.
dnl AC_COMPILATION_OPTIONS

AC_DEFUN([AC_COMPILATION_OPTIONS],[
	AC_MSG_CHECKING(if you want to enable use of memory pool)
	AC_ARG_ENABLE(memory-pool,
				[  --enable-memory-pool      enable memory pool features [[default=yes]]])
	if test -z "$enable_memory_pool" -a "x$OPERATINGSYSTEM" = xlinux; then
		enable_memory_pool="yes"
	fi
	if test "$enable_memory_pool" = "yes"; then
		AC_DEFINE(ENABLE_MEMPOOL, 1, [enable memory pool features])
	fi
	AC_MSG_RESULT([${enable_memory_pool}])

	AC_MSG_CHECKING(if you want to enable Large File Support)
	AC_ARG_ENABLE(LFS,
				[  --enable-LFS              enable Large File Support [[default=yes]]])
	if test -z "$enable_LFS"; then
		enable_LFS="yes"
	fi
	if test "$enable_LFS" = "yes"; then
		AC_DEFINE(ENABLE_LFS, 1, [enable Large File Support features])
	fi
	AC_MSG_RESULT([$enable_LFS])

	AC_MSG_CHECKING(if you want to enable use of coverage)
	AC_ARG_ENABLE(coverage,
				[  --enable-coverage         enable coverage [[default=no]]])
	if test -z "$enable_coverage"; then
		enable_coverage="no"
	elif test "$enable_coverage" = "yes"; then
		CPPFLAGS="$CPPFLAGS --coverage"
		LDFLAGS="$LDFLAGS -O0 -fprofile-arcs"
		CFLAGS="$CFLAGS -O0 -fprofile-arcs -ftest-coverage"
		CXXFLAGS="$CXXFLAGS -O0 -fprofile-arcs -ftest-coverage"
	fi
	AC_MSG_RESULT([$enable_coverage])

	dnl Check if compile with GCC optimizations flags enabled

	AC_MSG_CHECKING(for compile with GCC optimizations flags enabled)
	AC_ARG_ENABLE(gcc-optimized,
				[  --enable-gcc-optimized    compile with GCC optimizations flags enabled (-finline,-fstrict-aliasing,...) [[default=yes]]])
	if test -z "$enable_gcc_optimized" -a "x$OPERATINGSYSTEM" = xlinux; then
		enable_gcc_optimized="yes"
	fi
	AC_MSG_RESULT([${enable_gcc_optimized}])

	AC_MSG_CHECKING(if you want to enable mode final for build of ulib library)
	AC_ARG_ENABLE(final,
				[  --enable-final            build size optimized apps (needs more amounts of memory) [[default=yes]]])
	if test -z "$enable_final" ; then
		enable_final="yes"
	fi
	AC_MSG_RESULT([$enable_final])

	dnl Check if the linker supports --enable-new-dtags and --as-needed

	AC_MSG_CHECKING(for use of the new linker flags)
	AC_ARG_ENABLE(new-ldflags,
				[  --enable-new-ldflags      enable the new linker flags (enable-new-dtags,as-needed,...) [[default=yes]]])
	if test -z "$enable_new_ldflags" -a "x$OPERATINGSYSTEM" = xlinux; then
		enable_new_ldflags="yes"
	fi
	AC_MSG_RESULT([${enable_new_ldflags}])

	dnl check for GNUC visibility support

	AC_CACHE_CHECK(whether ${CXX} supports -fvisibility-inlines-hidden,
	ac_cv_cxx_visibility_inlines_hidden_flag,
	[
	echo '#include <sstream>' >conftest.c
	echo 'void f(){}' >>conftest.c
	if test -z "`${CXX} -fvisibility-inlines-hidden -c conftest.c 2>&1`"; then
		ac_cv_cxx_visibility_inlines_hidden_flag=yes
	else
		ac_cv_cxx_visibility_inlines_hidden_flag=no
	fi
	rm -f conftest*
	])

	dnl Check if compiler understands the C99 feature of restricted pointers,
	dnl (TWO UNRELATED TYPES CAN'T POINT TO THE SAME MEMORY, ONLY CHAR* HAS THIS PRIVILEGE)
	dnl specified with the __restrict__, or __restrict  type qualifier

	AC_C_RESTRICT

	AC_MSG_CHECKING(if you want to enable client response partial write support)
	AC_ARG_ENABLE(CRPWS,
				[  --enable-CRPWS            enable Client Response Partial Write Support [[default=no]]])
	if test -z "$enable_CRPWS"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_CRPWS="no"
		elif test "$enable_debug" = "yes"; then
			enable_CRPWS="yes"
		else
			enable_CRPWS="no"
		fi
	fi
	if test "$enable_CRPWS" = "yes"; then
		AC_DEFINE(U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT, 1, [enable client response partial write support])
	fi
	AC_MSG_RESULT([$enable_CRPWS])

	AC_MSG_CHECKING(if you want to enable server captive portal mode)
	AC_ARG_ENABLE(captive-portal,
				[  --enable-captive-portal   enable server captive portal mode [[default=no]]])
	if test -z "$enable_captive_portal"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_captive_portal="yes"
		else
			enable_captive_portal="no"
		fi
	fi
	if test "$enable_captive_portal" = "yes"; then
		AC_DEFINE(U_SERVER_CAPTIVE_PORTAL, 1, [enable server captive portal mode])
	fi
	AC_MSG_RESULT([$enable_captive_portal])

	AC_MSG_CHECKING(if you want to enable server thread approach support)
	AC_ARG_ENABLE(thread-approach,
				[  --enable-thread-approach  enable server thread approach support [[default=no]]])
	if test -z "$enable_thread_approach" ; then
		enable_thread_approach="no"
	fi
	if test "$enable_thread_approach" = "yes"; then
		AC_DEFINE(U_SERVER_THREAD_APPROACH_SUPPORT, 1, [enable server thread approach support])
	fi
	AC_MSG_RESULT([$enable_thread_approach])

	AC_MSG_CHECKING(if you want to enable HTTP inotify support)
	AC_ARG_ENABLE(HIS,
				[  --enable-HIS              enable HTTP Inotify Support [[default=no]]])
	if test -z "$enable_HIS" ; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_HIS="no"
		elif test "$enable_debug" = "yes"; then
			enable_HIS="yes"
		else
			enable_HIS="no"
		fi
	fi
	if test "$enable_HIS" = "yes"; then
		AC_DEFINE(U_HTTP_INOTIFY_SUPPORT, 1, [enable HTTP inotify support])
	fi
	AC_MSG_RESULT([$enable_HIS])

	AC_MSG_CHECKING(if you want to enable client and server log support)
	AC_ARG_ENABLE(log,
				[  --enable-log              enable client and server log support [[default=yes]]])
	if test -z "$enable_log"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_log="no"
		else
			enable_log="yes"
		fi
	fi
	if test "$enable_log" != "yes"; then
		AC_DEFINE(U_LOG_DISABLE, 1, [disable client and server log support])
	fi
	AC_MSG_RESULT([$enable_log])

	AC_MSG_CHECKING(if you want to enable GDB stack dump support)
	AC_ARG_ENABLE(GSDS,
				[  --enable-GSDS             enable GDB Stack Dump Support [[default=no]]])
	if test -z "$enable_GSDS"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_GSDS="no"
		elif test "$enable_debug" = "yes"; then
			enable_GSDS="yes"
		else
			enable_GSDS="no"
		fi
	fi
	if test "$enable_GSDS" = "yes"; then
		AC_DEFINE(U_GDB_STACK_DUMP_ENABLE, 1, [enable GDB stack dump support])
	fi
	AC_MSG_RESULT([$enable_GSDS])

	AC_MSG_CHECKING(if you want to enable cache request support)
	AC_ARG_ENABLE(HCRS,
				[  --enable-HCRS             enable Cache Request Support [[default=no]]])
	if test -z "$enable_HCRS"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_HCRS="yes"
		else
			enable_HCRS="no"
		fi
	fi
	if test "$enable_HCRS" != "yes"; then
		AC_DEFINE(U_CACHE_REQUEST_DISABLE, 1, [disable cache request support])
	fi
	AC_MSG_RESULT([$enable_HCRS])

	AC_MSG_CHECKING(if you want to enable homogeneous pipeline request support)
	AC_ARG_ENABLE(HPRS,
				[  --enable-HPRS             enable Homogeneous Pipeline Request Support [[default=no]]])
	if test -z "$enable_HPRS"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_HPRS="yes"
		else
			enable_HPRS="no"
		fi
	fi
	if test "$enable_HPRS" != "yes"; then
		AC_DEFINE(U_PIPELINE_HOMOGENEOUS_DISABLE, 1, [disable homogeneous pipeline request support])
	fi
	AC_MSG_RESULT([$enable_HPRS])

	AC_MSG_CHECKING(if you want to enable Server-Sent Events support)
	AC_ARG_ENABLE(SSE,
				[  --enable-SSE              enable Server-Sent Events Support [[default=no]]])
	if test -z "$enable_SSE"; then
		enable_SSE="no"
	fi
	if test "$enable_SSE" = "yes"; then
		AC_DEFINE(U_SSE_ENABLE, 1, [enable Server-Sent Events support])
	fi
	AC_MSG_RESULT([$enable_SSE])

	AC_MSG_CHECKING(if you want to enable HTTP/2 support)
	AC_ARG_ENABLE(http2,
				[  --enable-http2            enable HTTP/2 support [[default=no]]])
	if test -z "$enable_http2"; then
		enable_http2="no"
	fi
	if test "$enable_http2" != "yes"; then
		AC_DEFINE(U_HTTP2_DISABLE, 1, [disable HTTP/2 support])
	fi
	AC_MSG_RESULT([$enable_http2])

	AC_MSG_CHECKING(if you want to enable server check time between request for parallelization)
	AC_ARG_ENABLE(check-time,
				[  --enable-check-time       enable server check time between request for parallelization [[default=no]]])
	if test -z "$enable_check_time"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_check_time="no"
		elif test "$enable_debug" = "yes" -a "$enable_http2" != "yes"; then
			enable_check_time="yes"
		else
			enable_check_time="no"
		fi
	fi
	if test "$enable_check_time" = "yes"; then
		AC_DEFINE(U_SERVER_CHECK_TIME_BETWEEN_REQUEST, 1, [enable server check time between request for parallelization])
	fi
	AC_MSG_RESULT([$enable_check_time])

	AC_MSG_CHECKING(if you want to enable server classic model support)
	AC_ARG_ENABLE(classic,
				[  --enable-classic          enable server classic model support [[default=no]]])
	if test -z "$enable_classic"; then
		enable_classic="no"
	fi
	if test "$enable_classic" = "yes"; then
		AC_DEFINE(U_CLASSIC_SUPPORT, 1, [enable server classic model support])
	fi
	AC_MSG_RESULT([$enable_classic])

	AC_MSG_CHECKING(if you want to enable server bandwidth throttling support)
	AC_ARG_ENABLE(throttling,
				[  --enable-throttling       enable server bandwidth throttling support [[default=no]]])
	if test -z "$enable_throttling"; then
		enable_throttling="no"
	fi
	if test "$enable_throttling" = "yes"; then
		AC_DEFINE(U_THROTTLING_SUPPORT, 1, [enable server bandwidth throttling support])
	fi
	AC_MSG_RESULT([$enable_throttling])

	AC_MSG_CHECKING(if you want to enable to provide evasive action in the event of an HTTP DoS or DDoS attack or brute force attack)
	AC_ARG_ENABLE(evasive,
				[  --enable-evasive          enable server evasive action support [[default=no]]])
	if test -z "$enable_evasive"; then
		enable_evasive="no"
	fi
	if test "$enable_evasive" = "yes"; then
		AC_DEFINE(U_EVASIVE_SUPPORT, 1, [enable server evasive action support])
	fi
	AC_MSG_RESULT([$enable_evasive])

	AC_MSG_CHECKING(if you want to enable alias URI support)
	AC_ARG_ENABLE(alias,
				[  --enable-alias            enable alias URI support [[default=yes]]])
	if test -z "$enable_alias"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_alias="no"
		else
			enable_alias="yes"
		fi
	fi
	if test "$enable_alias" = "yes"; then
		AC_DEFINE(U_ALIAS, 1, [enable alias URI support])
	fi
	AC_MSG_RESULT([$enable_alias])

	AC_MSG_CHECKING(if you want to enable welcome message support)
	AC_ARG_ENABLE(welcome,
				[  --enable-welcome          enable welcome message support [[default=no]]])
	if test -z "$enable_welcome"; then
		enable_welcome="no"
	fi
	if test "$enable_welcome" = "yes"; then
		AC_DEFINE(U_WELCOME_SUPPORT, 1, [enable welcome message support])
	fi
	AC_MSG_RESULT([$enable_welcome])

	AC_MSG_CHECKING(if you want to enable ACL filtering support)
	AC_ARG_ENABLE(ACL,
				[  --enable-ACL              enable ACL filtering support [[default=no]]])
	if test -z "$enable_ACL"; then
		enable_ACL="no"
	fi
	if test "$enable_ACL" = "yes"; then
		AC_DEFINE(U_ACL_SUPPORT, 1, [enable ACL filtering support])
	fi
	AC_MSG_RESULT([$enable_ACL])

	AC_MSG_CHECKING(if you want to enable RFC1918 filtering support)
	AC_ARG_ENABLE(RFC1918,
				[  --enable-RFC1918          enable RFC1918 filtering support [[default=no]]])
	if test -z "$enable_RFC1918"; then
		enable_RFC1918="no"
	fi
	if test "$enable_RFC1918" = "yes"; then
		AC_DEFINE(U_RFC1918_SUPPORT, 1, [enable RFC1918 filtering support])
	fi
	AC_MSG_RESULT([$enable_RFC1918])

	AC_MSG_CHECKING(if you want to enable HTTP Strict Transport Security support)
	AC_ARG_ENABLE(HSTS,
				[  --enable-HSTS             enable HTTP Strict Transport Security support [[default=no]]])
	if test -z "$enable_HSTS"; then
		if test "$USP_FLAGS" = "-DAS_cpoll_cppsp_DO"; then
			enable_HSTS="no"
		elif test "$enable_debug" = "yes"; then
			enable_HSTS="yes"
		else
			enable_HSTS="no"
		fi
	fi
	if test "$enable_HSTS" = "yes"; then
		AC_DEFINE(U_HTTP_STRICT_TRANSPORT_SECURITY, 1, [enable HTTP Strict Transport Security support])
	fi
	AC_MSG_RESULT([$enable_HSTS])

	AC_MSG_CHECKING(if you want to enable HTML pagination support)
	AC_ARG_ENABLE(HPS,
				[  --enable-HPS              enable HTML Pagination Support [[default=no]]])
	if test -z "$enable_HPS"; then
		enable_HPS="no"
	fi
	if test "$enable_HPS" = "yes"; then
		AC_DEFINE(U_HTML_PAGINATION_SUPPORT, 1, [enable HTML pagination support])
	fi
	AC_MSG_RESULT([$enable_HPS])

	AC_MSG_CHECKING(if you want to enable load balance support between physical server via udp brodcast)
	AC_ARG_ENABLE(load-balance,
				[  --enable-load-balance     enable load balance support between physical server via udp brodcast [[default=no]]])
	if test -z "$enable_load_balance"; then
		enable_load_balance="no"
	fi
	if test "$enable_load_balance" = "yes"; then
		AC_DEFINE(USE_LOAD_BALANCE, 1, [enable load balance support between physical server via udp brodcast])
	fi
	AC_MSG_RESULT([$enable_load_balance])
])
