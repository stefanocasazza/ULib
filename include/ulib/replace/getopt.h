/* getopt.h */

#ifndef _GETOPT_H
#define _GETOPT_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* For communication from `getopt' to the caller. When `getopt' finds an option that takes an argument, the argument value is returned here.
 * Also, when `ordering' is RETURN_IN_ORDER, each non-option ARGV-element is returned here.
 */

U_EXPORT char* optarg;

/* Index in ARGV of the next element to be scanned.  This is used for communication to and from the caller and for communication
 * between successive calls to `getopt'. On entry to `getopt', zero means this is the first call; initialize. When `getopt' returns
 * -1, this is the index of the first of the non-option elements that the caller should itself scan. Otherwise, `optind' communicates
 * from one call to the next how much of ARGV has been scanned so far.
 */

U_EXPORT int optind;
U_EXPORT int opterr; /* Callers store zero here to inhibit the error message `getopt' prints for unrecognized options */
U_EXPORT int optopt; /* Set to an option character which was unrecognized */

/* Describe the long-named options requested by the application. The LONG_OPTIONS argument to getopt_long or getopt_long_only is a vector
 * of `struct option' terminated by an element containing a name which is zero. The field `has_arg' is:
 *
 * no_argument       (or 0) if the option does not take an argument,
 * required_argument (or 1) if the option requires an argument,
 * optional_argument (or 2) if the option takes an optional argument.
 *
 * If the field `flag' is not NULL, it points to a variable that is set to the value given in the field `val' when the option is found, but
 * left unchanged if the option is not found. To have a long-named option do something other than set an `int' to a compiled-in constant,
 * such as set a value from `optarg', set the option's `flag' field to zero and its `val' field to a nonzero value (the equivalent single-letter
 * option character, if there is one). For long options that have a zero `flag' field, `getopt' returns the contents of the `val' field
 */

struct option {
   const char *name;
   int has_arg; /* has_arg can't be an enum because some compilers complain about type mismatches in all the code that assumes it is an int */
   int* flag;
   int val;
};

/* Names for the values of the `has_arg' field of `struct option'
# define no_argument       0
# define required_argument 1
# define optional_argument 2
*/

/* Get definitions and prototypes for functions to process the arguments in ARGV (ARGC of them, minus the program name) for options given in OPTS.
 * Return the option character from OPTS just read.  Return -1 when there are no more options.  For unrecognized options, or options missing
 * arguments, `optopt' is set to the option letter, and '?' is returned. The OPTS string is a list of characters which are recognized option
 * letters, optionally followed by colons, specifying that that letter takes an argument, to be placed in `optarg'. If a letter in OPTS is
 * followed by two colons, its argument is optional.  This behavior is specific to the GNU `getopt'. The argument `--' causes premature termination
 * of argument scanning, explicitly telling `getopt' that there are no more options. If OPTS begins with `--', then non-option arguments are treated
 * as arguments to the option '\0'. This behavior is specific to the GNU `getopt'
 */

U_EXPORT int u_getopt_long(int argc, char* const* argv, const char* shortopts, const struct option* longopts, int* longind);

/* I don't use this...
 * int u_getopt(int argc, char* const* argv, const char* optstring);
 * int u_getopt_long_only(int argc, char* const* argv, const char* shortopts, const struct option* longopts, int* longind);
 */

#ifdef __cplusplus
}
#endif

#endif
