// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    portable.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/internal/portable.h>

#ifdef U_STD_STRING

unsigned split(UVector<UString>& vec, const UString& buffer, const char* delim)
{
   U_TRACE(5, "split(%p,%V,%S)", &vec, buffer.rep, delim)

   unsigned n      = vec.size();
   const char* s   = buffer.data();
   const char* end = s + buffer.size();

   string x;
   const char* p;
   const char* b = s;

loop:

   if (s >= end)
      {
      goto done;
      }

   if (isspace(*s) ||
       strchr(delim, *s))
      { 
      ++s; 

      goto loop;
      }

   if (*s == '"')
      {
      p = s++;

      while (s < end &&
             *s != '"')
         {
         if (*s == '\\')
            {
            ++s;
            }

         ++s;
         }

      if (s < end) ++s;
      }
   else
      {
      p = s++;

      while (s < end && 
             isspace(*s) == false &&
             strchr(delim, *s) == 0)
         {
         ++s;
         }
      }

   x = buffer.substr(p - b, s - p);

   vec.push_back(x);

   ++s;

   goto loop;

done:
   unsigned r = vec.size() - n;

   U_RETURN(r);
}

#endif
