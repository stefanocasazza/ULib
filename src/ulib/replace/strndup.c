/* strndup.c */

/* Copyright (C) 1996, 1997, 1998, 2000 Free Software Foundation, Inc.

   NOTE: The canonical source of this file is maintained with the GNU C Library.
   Bugs can be reported to bug-glibc@prep.ai.mit.edu.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <ulib/base/base.h>

#include <stdio.h>

/* Find the length of STRING, but scan at most MAXLEN characters. If no '\0' terminator is found in that many characters, return MAXLEN. */

extern U_EXPORT char*  strndup(const char* s, size_t n);
extern U_EXPORT size_t strnlen(const char* string, size_t maxlen);

U_EXPORT size_t strnlen(const char* string, size_t maxlen)
{
   const char* end = (const char*) memchr(string, '\0', maxlen);

   return (end ? (size_t)(end - string) : maxlen);
}

U_EXPORT char* strndup(const char* s, size_t n)
{
   size_t len = strnlen(s, n);
   char*  res = (char*) malloc(len + 1);

   if (res == 0) return 0;

   res[len] = '\0';

   return (char*) memcpy(res, s, len);
}
