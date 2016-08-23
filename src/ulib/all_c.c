/* all.c */

#define U_ALL_C

#include "base/hash.c"
#include "base/utility.c"
#include "base/base.c"
#include "base/base_error.c"
#include "base/miniz/miniz.c"
#include "base/coder/cbase64.c"
#include "base/coder/cescape.c"
#include "base/coder/chexdump.c"
#include "base/coder/curl_coder.c"
#include "base/coder/cxml_coder.c"
#include "base/coder/cquoted_printable.c"

#ifdef DEBUG
#  include "base/base_trace.c"
#endif

#ifdef USE_LIBZ
#  include "base/coder/cgzio.c"
#endif

#ifdef ENABLE_ZIP
#  include "base/zip/dostime.c"
#  include "base/zip/inflate.c"
#  include "base/zip/pushback.c"
#  include "base/zip/ziptool.c"
#endif

#ifdef USE_LIBSSL
#  include "base/ssl/dgst.c"
#  include "base/ssl/cdes3.c"
#endif

#ifdef _MSWINDOWS_
#  include "base/win32/mingw32.c"
#endif

#ifndef HAVE_ASSERT_H
#  include "replace/assert.c"
#endif

#ifndef HAVE_SEM_INIT
#  include "replace/sem.c"
#elif !defined(HAVE_SEM_TIMEDWAIT)
#  include "replace/sem_timedwait.c"
#endif

#ifndef HAVE_DAEMON
#  include "replace/daemon.c"
#endif

#ifndef HAVE_GETOPT_LONG
#  include "replace/getopt_long.c"
#endif

#ifndef HAVE_STRNDUP
#  include "replace/strndup.c"
#endif

#ifndef HAVE_STRPTIME
#  include "replace/strptime.c"
#endif

#ifndef HAVE_NANOSLEEP
#  include "replace/nanosleep.c"
#endif

#ifndef HAVE_MREMAP
#  include "replace/mremap.c"
#endif

#if !defined(HAVE_SENDFILE) && !defined(HAVE_MACOSX_SENDFILE)
#  include "replace/sendfile.c"
#endif

#ifndef HAVE_MKDTEMP
#  include "replace/mkdtemp.c"
#endif

#ifndef HAVE_MEMMEM
#  include "replace/memmem.c"
#endif

#ifndef HAVE_MEMRCHR
#  include "replace/memrchr.c"
#endif

#ifndef HAVE_GMTIME_R
#  include "replace/gmtime.c"
#endif

#ifndef HAVE_TIMEGM
#  include "replace/timegm.c"
#endif

#ifndef HAVE_FALLOCATE
#  include "replace/fallocate.c"
#endif

#ifndef HAVE_FALLOCATE64
#  include "replace/fallocate64.c"
#endif

#ifndef HAVE_PREAD
#  include "replace/pread.c"
#endif

#ifndef HAVE_SCHED_GETCPU
#  include "replace/sched_getcpu.c"
#endif

#ifdef U_APEX_ENABLE
#  include "base/apex/apex_memmove.c"
#endif
