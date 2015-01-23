/*
   Asking the system what it has (header)
   AUP2, Sec. 1.05.3

   Copyright 2003 by Marc J. Rochkind. All rights reserved.
   May be copied only for purposes and under conditions described
   on the Web page www.basepath.com/aup/copyright.htm.

   The Example Files are provided "as is," without any warranty;
   without even the implied warranty of merchantability or fitness
   for a particular purpose. The author and his publisher are not
   responsible for any damages, direct or incidental, resulting
   from the use or non-use of these Example Files.

   The Example Files may contain defects, and some contain deliberate
   coding mistakes that were included for educational reasons.
   You are responsible for determining if and how the Example Files
   are to be used.
*/

#ifndef _SUVREQ_H_
#define _SUVREQ_H_ 1
/*
   Header to request specific standard support. Before including it, one
   of the following symbols must be defined (1003.1-1988 isn't supported):

      SUV_POSIX1990  for 1003.1-1990
      SUV_POSIX1993  for 1003.1b-1993 - real-time
      SUV_POSIX1996  for 1003.1-1996
      SUV_SUS1       for Single UNIX Specification, v. 1 (UNIX 95)
      SUV_SUS2       for Single UNIX Specification, v. 2 (UNIX 98)
      SUV_SUS3       for Single UNIX Specification, v. 3
*/

#ifdef _POSIX_SOURCE
#  error "_POSIX_SOURCE defined"
#endif

#if defined(SUV_POSIX1990)
#  define _POSIX_SOURCE
#  define _POSIX_C_SOURCE 1
#elif defined(SUV_POSIX1993)
#  define _POSIX_SOURCE
#  define _POSIX_C_SOURCE 199309L
#elif defined(SUV_POSIX1996)
#  define _POSIX_SOURCE
#  define _POSIX_C_SOURCE 199506L
#elif defined(SUV_SUS1)
#  define _POSIX_SOURCE
#  define _POSIX_C_SOURCE 2
#  define _XOPEN_SOURCE
#  define _XOPEN_SOURCE_EXTENDED 1
#elif defined(SUV_SUS2)
#  define _POSIX_SOURCE
#  define _POSIX_C_SOURCE 199506L
#  define _XOPEN_SOURCE 500
#  define _XOPEN_SOURCE_EXTENDED 1
#elif defined(SUV_SUS3)
#  define _POSIX_SOURCE
#  define _POSIX_C_SOURCE 200112L
#  define _XOPEN_SOURCE 600
#  define _XOPEN_SOURCE_EXTENDED 1
#endif

#endif
