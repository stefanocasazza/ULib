dnl @synopsis HAVE_OLD_IOSTREAM
dnl Determine kind of C++ iostream support
dnl

AC_DEFUN([AC_CXX_OLD_IOSTREAM],
[AC_CACHE_CHECK(determine if the kind of C++ iostream support is old, ac_cv_cxx_old_iostream,
 [AC_TRY_COMPILE([
#include <iostream.h>
#ifdef CCXX_NAMESPACES
using namespace std;
#endif
#undef  GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#if GCC_VERSION >= 3004
pippo(); /* force error... */
#endif
class mystr : public iostream, public streambuf { mystr(); };
mystr::mystr() : iostream(),streambuf() { init((streambuf*)this); }
  ],[], ac_cv_cxx_old_iostream=yes, ac_cv_cxx_old_iostream=no) ])
if test $ac_cv_cxx_old_iostream = yes ; then
	AC_DEFINE(HAVE_OLD_IOSTREAM,1,[define if the compiler implements old style iostreams])
fi
])
