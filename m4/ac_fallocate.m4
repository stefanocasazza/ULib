dnl check compiler flags
AC_DEFUN([AC_TRY_FALLOCATE],
[dnl
	AC_MSG_CHECKING([whether fallocate is supported])

        AC_RUN_IFELSE([
                AC_LANG_PROGRAM([[
								#include <unistd.h>
								#include <fcntl.h>
                ]], [[
                int fd = creat("/tmp/fallocate", 0666), result = fallocate(fd, 0, 0, 1024);
					 (void) unlink("/tmp/fallocate");
					 return result;
                ]])
        ], [
                AH_TEMPLATE([FALLOCATE_IS_SUPPORTED], [Define to 1 if the fallocate() function is supported])
                AC_DEFINE([FALLOCATE_IS_SUPPORTED])
                AC_MSG_RESULT([yes])
        ], [
                AC_MSG_RESULT([no])
        ])
])
