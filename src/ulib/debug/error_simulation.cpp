// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    error_simulation.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/debug/error_simulation.h>

bool           USimulationError::flag_init;
char*          USimulationError::file_mem;
uint32_t       USimulationError::file_size;
union uuvararg USimulationError::var_arg;

/**
 * #if defined(HAVE_STRTOF) && !defined(strtof)
 * extern "C" { float strtof(const char* nptr, char** endptr); }
 * #endif
 * #if defined(HAVE_STRTOLD) && !defined(strtold)
 * extern "C" { long double strtold(const char* nptr, char** endptr); }
 * #endif
 */

void USimulationError::init()
{
   U_INTERNAL_TRACE("USimulationError::init()")

   int fd    = 0;
   char* env = getenv("USIMERR");

   char file[MAX_FILENAME_LEN];

   if ( env &&
       *env)
      {
      flag_init = true;

      // format: <file_error_simulation>
      //               error.sim

      (void) sscanf(env, "%254s", file);

      fd = open(file, O_RDONLY | O_BINARY, 0666);

      if (fd != -1)
         {
         struct stat st;

         if (fstat(fd, &st) == 0)
            {
            file_size = st.st_size;

            if (file_size)
               {
               file_mem = (char*) mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0);

               if (file_mem == MAP_FAILED) file_size = 0;
               }
            }

         (void) close(fd);
         }
      }

   if (fd <= 0)
      {
      U_MESSAGE("SIMERR%W<%Woff%W>%W", YELLOW, RED, YELLOW, RESET);
      }
   else
      {
      U_MESSAGE("SIMERR%W<%Won%W>: File<%W%s%W>%W", YELLOW, GREEN, YELLOW, CYAN, file, YELLOW, RESET);
      }
}

void* USimulationError::checkForMatch(const char* call_name)
{
   U_INTERNAL_TRACE("USimulationError::checkForMatch(%s)", call_name);

   if (flag_init &&
       file_size)
      {
      const char* ptr = call_name;

      while (*ptr && *ptr != '(') ++ptr;

      if (*ptr == '(')
         {
         int len        = ptr - call_name;
         char* limit    = file_mem + file_size - len;
         char* file_ptr = file_mem;

         // format: <classname>::method <random range> <return type> <return value> <errno value>
         //                    ::lstat       10             i           -1                13 

         bool match = false;

         for (; file_ptr <= limit; ++file_ptr)
            {
            while (u__isspace(*file_ptr)) ++file_ptr;

            if (*file_ptr != '#')
               {
               while (u__isspace(*file_ptr)) ++file_ptr;

               U_INTERNAL_PRINT("file_ptr = %.*s", len, file_ptr);

               if (u__isspace(file_ptr[len]) &&
                   memcmp(file_ptr, call_name, len) == 0)
                  {
                  match = true;

                  file_ptr += len;

                  // manage random testing

                  uint32_t range = (uint32_t) strtol(file_ptr, &file_ptr, 10);

                  if (range > 0) match = (u_get_num_random(range) == (range / 2));

                  break;
                  }
               }

            while (*file_ptr != '\n') ++file_ptr;
            }

         if (match)
            {
            while (u__isspace(*file_ptr)) ++file_ptr;

            char type = *file_ptr++;

            switch (type)
               {
               case 'p': // pointer
                  {
                  var_arg.p = (void*) strtol(file_ptr, &file_ptr, 16);
                  }
               break;

               case 'l': // long
                  {
                  var_arg.l = strtol(file_ptr, &file_ptr, 10);
                  }
               break;

               case 'L': // long long
                  {
#              ifdef HAVE_STRTOULL
                  var_arg.ll = (long long) strtoull(file_ptr, &file_ptr, 10);
#              endif
                  }
               break;

               case 'f': // float
                  {
#              ifdef HAVE_STRTOF
                  var_arg.f = strtof(file_ptr, &file_ptr);
#              endif
                  }
               break;

               case 'd': // double
                  {
                  var_arg.d = strtod(file_ptr, &file_ptr);
                  }
               break;

               case 'D': // long double
                  {
#              ifdef HAVE_STRTOLD
                  var_arg.ld = strtold(file_ptr, &file_ptr);
#              endif
                  }
               break;

               case 'i': // word-size (int)
               default:
                  {
                  var_arg.i = (int) strtol(file_ptr, &file_ptr, 10);
                  }
               break;
               }

            errno = atoi(file_ptr);

            U_INTERNAL_PRINT("errno = %d var_arg = %d", errno, var_arg.i);

            return &var_arg;
            }
         }
      }

   return 0;
}
