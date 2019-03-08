/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    utility.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_BASE_UTILITY_H
#define ULIB_BASE_UTILITY_H 1

#include <ulib/base/base.h>

#ifdef  _MSWINDOWS_
typedef uint32_t in_addr_t;
#endif

#if HAVE_DIRENT_H
#  include <dirent.h>
#  ifdef _DIRENT_HAVE_D_NAMLEN
#     define NAMLEN(dirent) (dirent)->d_namlen
#  else
#     define NAMLEN(dirent) u__strlen((dirent)->d_name, __PRETTY_FUNCTION__)
#  endif
#else
#  define dirent direct
#  define NAMLEN(dirent) (dirent)->d_namlen
#  if HAVE_SYS_NDIR_H
#     include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#     include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#     include <ndir.h>
#  endif
#endif

#ifdef HAVE_FNMATCH
#  include <fnmatch.h>
#endif

#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD (1 <<  4) /* compare without regard to case */
#endif
#define FNM_INVERT   (1 << 28) /* invert the result */

#ifndef FNM_LEADING_DIR
#define FNM_LEADING_DIR FNM_PERIOD
#endif
#ifndef FNM_IGNORECASE
#define FNM_IGNORECASE FNM_CASEFOLD
#endif

#ifdef HAVE_SCHED_H
#  include <sched.h>
#elif defined(HAVE_SYS_SCHED_H)
#  include <sys/sched.h>
#endif

#if defined(HAVE_CONFIG_H) && !defined(HAVE_CPU_SET_T)
typedef uint64_t cpu_set_t;
#endif

#ifdef CPU_SETSIZE
#  define CPUSET_BITS(set) ((set)->__bits)
#else
#  define CPU_SETSIZE                   (sizeof(cpu_set_t) * 8)
#  define CPU_ISSET(index, cpu_set_ptr) (*(cpu_set_ptr)  &  (1ULL << (index)))
#  define CPU_SET(index, cpu_set_ptr)   (*(cpu_set_ptr) |=  (1ULL << (index)))
#  define CPU_ZERO(cpu_set_ptr)         (*(cpu_set_ptr)  = 0)
#  define CPU_CLR(index, cpu_set_ptr)   (*(cpu_set_ptr) &= ~(1ULL << (index)))
#  define CPUSET_BITS(set) (set)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295U
#endif

/**
 * TOKEN ID
 */
#define U_TK_ERROR       -1
#define U_TK_AND          1
#define U_TK_OR           2
#define U_TK_EQ           3
#define U_TK_NE           4
#define U_TK_GT           5
#define U_TK_GE           6
#define U_TK_LT           7
#define U_TK_LE           8
#define U_TK_STARTS_WITH  9
#define U_TK_ENDS_WITH   10
#define U_TK_IS_PRESENT  11
#define U_TK_CONTAINS    12
#define U_TK_PLUS        13
#define U_TK_MINUS       14
#define U_TK_MULT        15
#define U_TK_DIV         16
#define U_TK_MOD         17
#define U_TK_NOT         18
#define U_TK_FN_CALL     19
#define U_TK_LPAREN      20
#define U_TK_RPAREN      21
#define U_TK_VALUE       22
#define U_TK_COMMA       23
#define U_TK_NAME        24
#define U_TK_PID         25

#ifdef __cplusplus
extern "C" {
#endif

/* Security functions */

U_EXPORT void u_init_security(void);

U_EXPORT void u_dont_need_root(void);
U_EXPORT void u_dont_need_group(void);

U_EXPORT void u_never_need_root(void);
U_EXPORT void u_never_need_group(void);

U_EXPORT void u_need_root(bool necessary);
U_EXPORT void u_need_group(bool necessary);

/* Services */

U_EXPORT uint32_t u_gettid(void);

static inline void u_setPid(void)
{
   U_INTERNAL_TRACE("u_setPid()")

   u_pid_str_len = u_num2str32(u_pid = u_gettid(), u_pid_str) - u_pid_str;
}

static inline uint8_t u_loadavg(const char* buffer)
{
   U_INTERNAL_TRACE("u_loadavg(%s)", buffer)

   U_INTERNAL_ASSERT_EQUALS(buffer[1], '.')

   return (((buffer[0]-'0') * 10) + (buffer[2]-'0') + (buffer[3] > '5')); // 0.19 => 2, 4.56 => 46, ...
}

/**
 * Return the smallest power of two value greater than n
 *
 *  Input range: [2..2147483648]
 * Output range: [2..2147483648]
 */

static inline uint32_t u_nextPowerOfTwo(uint32_t n)
{
   U_INTERNAL_TRACE("u_nextPowerOfTwo(%u)", n)

   U_INTERNAL_ASSERT_MAJOR(n, 1)
   U_INTERNAL_ASSERT(n <= ((UINT32_MAX/2)+1))

#if (defined(__GNUC__) || defined(__clang__)) && !defined(HAVE_OLD_IOSTREAM)
   return 1U << (sizeof(uint32_t) * 8 - __builtin_clz(n-1));
#else
   /* @https://web.archive.org/web/20170704200003/https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */

   --n;

   n |= n >>  1;
   n |= n >>  2;
   n |= n >>  4;
   n |= n >>  8;
   n |= n >> 16;

   U_INTERNAL_PRINT("n+1 = %u", n+1)

   return n+1;
#endif
}

/**
 * Random number generator
 *
 * We use George Marsaglia's MWC algorithm to produce an unsigned integer
 *
 * see http://www.bobwheeler.com/statistics/Password/MarsagliaPost.txt
 */

static inline void u_set_seed_random(uint32_t u, uint32_t v)
{
   U_INTERNAL_TRACE("u_set_seed_random(%u,%u)", u, v)

   U_INTERNAL_ASSERT_MAJOR(u, 0)
   U_INTERNAL_ASSERT_MAJOR(v, 0)

   u_m_w = u;
   u_m_z = v;
}

static inline uint32_t u_get_num_random(void)
{
   U_INTERNAL_TRACE("u_get_num_random()")

   U_INTERNAL_ASSERT_MAJOR(u_m_w, 0)
   U_INTERNAL_ASSERT_MAJOR(u_m_z, 0)

   u_m_z = 36969 * (u_m_z & 65535) + (u_m_z >> 16);
   u_m_w = 18000 * (u_m_w & 65535) + (u_m_w >> 16);

   return (u_m_z << 16) + u_m_w; /* 0 <= u < 2^32 */
}

static inline uint32_t u_get_num_random_range0(uint32_t range)
{
   uint32_t result;

   U_INTERNAL_TRACE("u_get_num_random_range0(%u)", range)

   U_INTERNAL_ASSERT_MAJOR(range, 1)

   result = u_get_num_random() % range;

   U_INTERNAL_ASSERT_MINOR(result,range)

   return result;
}

static inline uint32_t u_get_num_random_range1(uint32_t range)
{
   uint32_t result;

   U_INTERNAL_TRACE("u_get_num_random_range1(%u)", range)

   U_INTERNAL_ASSERT_MAJOR(range, 2)

   result = (u_get_num_random() % (range-1))+1;

   U_INTERNAL_ASSERT_RANGE(1,result,range-1)

   return result;
}

/* Produce a uniform random sample from the open interval (0, 1). The method will not return either end point */

static inline double u_get_uniform(void)
{
   U_INTERNAL_TRACE("u_get_uniform()")

   /* The magic number below is 1/(2^32 + 2). The result is strictly between 0 and 1 */

   return (u_get_num_random() + 1.0) * 2.328306435454494e-10;
}

U_EXPORT const char* u_get_mimetype(const char* restrict suffix, int* pmime_index);

U_EXPORT char*    u_memoryDump( char* restrict bp, unsigned char* restrict cp, uint32_t n);
U_EXPORT uint32_t u_memory_dump(char* restrict bp, unsigned char* restrict cp, uint32_t n);

U_EXPORT uint8_t  u_get_loadavg(void); /* Get the load average of the system (over last 1 minute) */
U_EXPORT uint16_t u_crc16(const char* a, uint32_t len); /* CRC16 implementation according to CCITT standards */
U_EXPORT uint32_t u_printSize(char* restrict buffer, uint64_t bytes); /* print size using u_calcRate() */

U_EXPORT int    u_getScreenWidth(void) __pure; /* Determine the width of the terminal we're running on */
U_EXPORT bool   u_isNumber(const char* restrict s, uint32_t n) __pure;
U_EXPORT double u_calcRate(uint64_t bytes, uint32_t msecs, int* restrict units); /* Calculate the transfert rate */
U_EXPORT char*  u_getPathRelativ(const char* restrict path, uint32_t* restrict path_len);

U_EXPORT int  u_get_num_cpu(void); /* Get the number of the processors including offline CPUs */
U_EXPORT void u_switch_to_realtime_priority(void); /* Set the process to maximum priority that can be used with the scheduling algorithm */
U_EXPORT void u_bind2cpu(cpu_set_t* cpuset, int n); /* Pin the process to a particular core */
U_EXPORT void u_get_memusage(unsigned long* vsz, unsigned long* rss); /* Get address space and rss (resident set size) usage */
U_EXPORT bool u_runAsUser(const char* restrict user, bool change_dir); /* Change the current working directory to the user's home dir, and downgrade security to that user account */
U_EXPORT int  u_strnatcmp(char const* restrict a, char const* restrict b) __pure; /* Perform 'natural order' comparisons of strings */
U_EXPORT bool u_validate_email_address(const char* restrict address, uint32_t address_len) __pure; /* Verifies that the passed string is actually an e-mail address */

static inline bool u_isSuffixSwap(const char* restrict suffix) // NB: vi tmp...
{
   U_INTERNAL_TRACE("u_isSuffixSwap(%p)", suffix)

   if (suffix)
      {
      U_INTERNAL_ASSERT_EQUALS(suffix[0], '.')
      U_INTERNAL_ASSERT_EQUALS(strchr(suffix, '/'), U_NULLPTR)

      if (u_get_unalignedp32(suffix) == U_MULTICHAR_CONSTANT32('.','s','w','p')) return true;
      }

   return false;
}

static inline void* u_find(const char* restrict a, uint32_t n1, const char* restrict b, uint32_t n2) /* check if string b is contained within string a */
{
   U_INTERNAL_TRACE("u_find(%.*s,%u,%.*s,%u)", U_min(n1,128), a, n1, U_min(n2,128), b, n2)

   U_INTERNAL_ASSERT_POINTER(a)
   U_INTERNAL_ASSERT_POINTER(b)
   U_INTERNAL_ASSERT_MAJOR(n2, 0)

   return memmem(a, n1, b, n2);
}

static inline uint32_t u_findEndHeader1(const char* restrict s, uint32_t n) /* find sequence of U_CRLF2 */
{
   void* p;

   U_INTERNAL_TRACE("u_findEndHeader1(%.*s,%u)", U_min(n,128), s, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n, 0)

   if (u_get_unalignedp32(s+n-4) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n')) return (n-4);

   p = memmem(s, n, U_CONSTANT_TO_PARAM(U_CRLF2));

   return (p ? (const char*)p - s + U_CONSTANT_SIZE(U_CRLF2) : U_NOT_FOUND);
}

U_EXPORT uint32_t u_findEndHeader(const char* restrict s, uint32_t n) __pure; /* find sequence of U_CRLF2 or U_LF2 */

/* check if string a start with string b */

__pure static inline bool u_startsWith(const char* restrict a, uint32_t n1, const char* restrict b, uint32_t n2)
{
   U_INTERNAL_TRACE("u_startsWith(%.*s,%u,%.*s,%u)", U_min(n1,128), a, n1, U_min(n2,128), b, n2)

   U_INTERNAL_ASSERT(n1 >= n2)
   U_INTERNAL_ASSERT_POINTER(a)
   U_INTERNAL_ASSERT_POINTER(b)
   U_INTERNAL_ASSERT_MAJOR(n1, 0)

   return (memcmp(a, b, n2) == 0);
}

/* check if string a terminate with string b */

__pure static inline bool u_endsWith(const char* restrict haystack, uint32_t haystack_len, const char* restrict needle, uint32_t needle_len)
{
   /* see if substring characters match at end */

   const char* restrict nn = needle   + needle_len   - 1;
   const char* restrict hh = haystack + haystack_len - 1;

   U_INTERNAL_TRACE("u_endsWith(%.*s,%u,%.*s,%u)", U_min(haystack_len,128), haystack, haystack_len, U_min(needle_len,128), needle, needle_len)

   U_INTERNAL_ASSERT_POINTER(needle)
   U_INTERNAL_ASSERT_POINTER(haystack)
   U_INTERNAL_ASSERT_MAJOR(haystack_len, 0)
   U_INTERNAL_ASSERT(haystack_len >= needle_len)

   while (*nn-- == *hh--)
      {
      if (nn >= needle) continue;

      return true; /* we got all the way to the start of the substring so we must've won */
      }

   return false;
}

/* check if the string is quoted... */

static inline bool u_is_quoted(const char* restrict s, uint32_t n)
{
   U_INTERNAL_TRACE("u_is_quoted(%.*s,%u)", U_min(n,128), s, n)

   if (s[0]   == '"' &&
       s[n-1] == '"')
      {
      return true;
      }

   return false;
}

/* find first char not quoted */

__pure static inline const char* u_find_char(const char* restrict s, const char* restrict end, char c)
{
   U_INTERNAL_TRACE("u_find_char(%.*s,%p,%d)", U_min(end-s,128), s, end, c)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(end)
   U_INTERNAL_ASSERT_EQUALS(s[-1],c)

loop:
   s = (const char* restrict) memchr(s, c, end - s);

   if (s == U_NULLPTR) return end;

   if (*(s-1) == '\\')
      {
      uint32_t i;

      for (i = 2; (*(s-i) == '\\'); ++i) {}

      if ((i & 1) == 0)
         {
         ++s;

         goto loop;
         }
      }

   return s;
}

/* skip string delimiter or white space and line comment */

U_EXPORT const char* u_skip(const char* restrict s, const char* restrict end, const char* restrict delim, char line_comment) __pure;

/* delimit token */

U_EXPORT const char* u_delimit_token(const char* restrict s,   const char** restrict p, const char* restrict end, const char*  restrict delim, char skip_line_comment);

/* Search a string for any of a set of characters. Locates the first occurrence in the string s of any of the characters in the string accept */

U_EXPORT const char* u__strpbrk(const char* restrict s, uint32_t slen, const char* restrict accept) __pure;

/* Search a string for a terminator of a group of delimitator {} [] () <%%>...*/

U_EXPORT const char* u_strpend(const char* restrict s, uint32_t slen, const char* restrict group_delimitor, uint32_t group_delimitor_len, char skip_line_comment) __pure;

/**
 * WILDCARD PATTERN - The rules are as follows (POSIX.2, 3.13)
 * --------------------------------------------------------------------------------
 * Wildcard Matching
 *
 * A string is a wildcard pattern if it contains one of the characters '?', '*' or '['. Globbing is the operation that
 * expands a wildcard pattern into the list of pathnames matching the pattern. Matching is defined by:
 *
 * A '?' (not between brackets) matches any single character.
 * A '*' (not between brackets) matches any string, including the empty string. 
 * --------------------------------------------------------------------------------
 * Character classes
 *
 * An expression '[...]' where the first character after the leading '[' is not an '!' matches a single character, namely
 * any of the characters enclosed by the brackets. The string enclosed by the brackets cannot be empty; therefore ']' can
 * be allowed between the brackets, provided that it is the first character. (Thus, '[][!]' matches the three characters
 * '[', ']' and '!'.)
 * --------------------------------------------------------------------------------
 * Ranges
 *
 * There is one special convention: two characters separated by '-' denote a range. (Thus, '[A-Fa-f0-9]' is equivalent to
 * '[ABCDEFabcdef0123456789]'.) One may include '-' in its literal meaning by making it the first or last character
 * between the brackets. (Thus, '[]-]' matches just the two characters ']' and '-', and '[--0]' matches the three characters
 * '-', '.', '0', since '/' cannot be matched.)
 * --------------------------------------------------------------------------------
 * Complementation
 *
 * An expression '[!...]' matches a single character, namely any character that is not matched by the expression obtained
 * by removing the first '!' from it. (Thus, '[!]a-]' matches any single character except ']', 'a' and '-'.)
 *
 * One can remove the special meaning of '?', '*' and '[' by preceding them by a backslash, or, in case this is part of a
 * shell command line, enclosing them in quotes. Between brackets these characters stand for themselves. Thus, '[[?*\]'
 * matches the four characters '[', '?', '*' and '\'. 
 * --------------------------------------------------------------------------------
 * Pathnames
 *
 * Globbing is applied on each of the components of a pathname separately. A '/' in a pathname cannot be matched by a '?'
 * or '*'  wildcard, or by a range like '[.-0]'. A range cannot contain an explicit '/' character; this would lead to a
 * syntax error. 
 *
 * If a filename starts with a '.', this character must be matched explicitly. (Thus, 'rm *' will not remove .profile,
 * and 'tar c *' will not archive all your files; 'tar c .' is better.)
 *
 * Note that wildcard patterns are not regular expressions, although they are a bit similar. First of all, they match
 * filenames, rather than text, and secondly, the conventions are not the same: for example, in a regular expression '*'
 * means zero or more copies of the preceding thing.
 *
 * Now that regular expressions have bracket expressions where the negation is indicated by a '^', POSIX has declared the
 * effect of a wildcard pattern '[^...]' to be undefined
 */

typedef bool (*bPFpcupcud)(const char*, uint32_t, const char*, uint32_t, int);

U_EXPORT bool u_fnmatch(     const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags);
U_EXPORT bool u_dosmatch(    const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags) __pure;
U_EXPORT bool u_dosmatch_ext(const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags) __pure;

/* multiple patterns separated by '|' */

extern U_EXPORT const char* restrict u_pOR;

U_EXPORT uint32_t u_match_with_OR(bPFpcupcud pfn_match, const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags);

static inline uint32_t u_dosmatch_with_OR(const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("u_dosmatch_with_OR(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), s, n1, n2, pattern, n2, flags)

   return u_match_with_OR(u_dosmatch, s, n1, pattern, n2, flags);
}

static inline uint32_t u_dosmatch_ext_with_OR(const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("u_dosmatch_ext_with_OR(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), s, n1, n2, pattern, n2, flags)

   return u_match_with_OR(u_dosmatch_ext, s, n1, pattern, n2, flags);
}

/**
 * Canonicalize path by building a new path. The new path differs from original in that:
 *
 * Multiple    '/'                     are collapsed to a single '/'
 * Trailing    '/'                     are removed
 * Leading     './'  and trailing '/.' are removed
 * Non-leading '../' and trailing '..' are handled by removing portions of the path
 */

U_EXPORT uint32_t u_canonicalize_pathname(char* restrict path, uint32_t sz);

/**
 * find a FILE MODE along PATH
 * --------------------------------------------------------------
 * pathfind looks for a a file with name FILENAME and MODE access
 * along colon delimited PATH, and returns the full pathname as a
 * string, or NULL if not found
 * --------------------------------------------------------------
 */

U_EXPORT bool u_pathfind(char* restrict result, const char* restrict path, uint32_t path_len, const char* restrict filename, int mode); /* R_OK | X_OK */

/* Prepare command for call to exec() */

U_EXPORT uint32_t u_split(       char* restrict s, uint32_t n, char** restrict argv, const char* restrict delim);
U_EXPORT int      u_splitCommand(char* restrict s, uint32_t n, char** restrict argv, char* restrict pathbuf, uint32_t pathbuf_size);

/* To avoid libc locale overhead */

U_EXPORT int u__strncasecmp(const char* restrict s1, const char* restrict s2, size_t n) __pure;

/* character type identification - Assumed an ISO-1 character set */

extern U_EXPORT const unsigned int  u__ct_tab[256];
extern U_EXPORT const unsigned char u__ct_tol[256];
extern U_EXPORT const unsigned char u__ct_tou[256];

/* NB: we use u__tolower(), u__toupper(), u__isspace(), ... cause of conflit with /usr/include/unicode/uchar.h */

static inline unsigned int  u_cttab(   unsigned char c) { return u__ct_tab[c]; }
static inline unsigned char u__tolower(unsigned char c) { return u__ct_tol[c]; }
static inline unsigned char u__toupper(unsigned char c) { return u__ct_tou[c]; }

                                                                      /* 0x00000001              __S character space    ' ' (32 0x20) */
                                                                      /* 0x00000002              __E character used in printf format  */
                                                                      /* 0x00000004              __H character          '+' (43 0x2B) */
                                                                      /* 0x00000008              __V character          ',' (44 0x2C) */
                                                                      /* 0x00000010              __O character minus    '-' (45 0x2D) */
                                                                      /* 0x00000020              __N character point    '.' (46 0x2E) */
                                                                      /* 0x00000040              __G character          ':' (58 0x3A) */
                                                                      /* 0x00000080              __Q character underbar '_' (95 0x5F) */
                                                                      /* 0x00000100)             __B character tab       \t (09 0x09) */
static inline bool u__islterm(unsigned char c)   { return ((u_cttab(c) & 0x00000200) != 0); } /* __R carriage return | new line (\r | \n) */
static inline bool u__isspace(unsigned char c)   { return ((u_cttab(c) & 0x00000400) != 0); } /* __W WhiteSpace */
static inline bool u__iscntrl(unsigned char c)   { return ((u_cttab(c) & 0x00000800) != 0); } /* __C Control character */
static inline bool u__islower(unsigned char c)   { return ((u_cttab(c) & 0x00002000) != 0); } /* __L Lowercase */
static inline bool u__ispunct(unsigned char c)   { return ((u_cttab(c) & 0x00004000) != 0); } /* __I Punctuation */
static inline bool u__isupper(unsigned char c)   { return ((u_cttab(c) & 0x00008000) != 0); } /* __U Uppercase */
static inline bool u__isoctal(unsigned char c)   { return ((u_cttab(c) & 0x00010000) != 0); } /* __Z Octal */
static inline bool u__istext( unsigned char c)   { return ((u_cttab(c) & 0x00020000) == 0); } /* __F character never appears in plain ASCII text */
                                                                      /* 0x00040000              __T character       appears in plain ASCII text */
                                                                      /* 0x00080000              __X Hexadecimal */
                                                                      /* 0x00100000              __A BASE64 encoded: '+' | '/' (47 0x2F) | '=' (61 0x3D) */
                                                                      /* 0x08000000              __UF filename invalid char: ":<>*?\| */
                                                                      /* 0x10000000              __XM char >= (32 0x20) */
                                                                      /* 0x20000000              __XE char '}' | ']' */
                                                                      /* 0x40000000              __XD char [1-9] */
static inline bool u__ismethod( unsigned char c) { return ((u_cttab(c) & 0x00200000) != 0); } /* __M HTTP (COPY,DELETE,GET,HEAD|HTTP,OPTIONS,POST/PUT/PATCH) */
static inline bool u__isheader( unsigned char c) { return ((u_cttab(c) & 0x00400000) != 0); } /* __Y HTTP header (Host,Range,...) */
static inline bool u__isquote(  unsigned char c) { return ((u_cttab(c) & 0x00800000) != 0); } /* __K string quote: '"' (34 0x22) | ''' (39 0x27) */
static inline bool u__ishtmlc(  unsigned char c) { return ((u_cttab(c) & 0x01000000) != 0); } /* __J HTML: '&' (38 0x26) | '<' (60 0x3C) | '>' (62 0x3E) */
static inline bool u__is2urlenc(unsigned char c) { return ((u_cttab(c) & 0x02000000) != 0); } /* __UE URL: char TO encoded ... */ 
static inline bool u__isurlqry( unsigned char c) { return ((u_cttab(c) & 0x04000000) != 0); } /* __UQ URL: char FROM query '&' (38 0x26) | '=' (61 0x3D) | '#' (35 0x23) */

static inline bool u__isprintf(unsigned char c)  { return ((u_cttab(c) & 0x00000002) != 0); } /*  __E                                  */
static inline bool u__issign(  unsigned char c)  { return ((u_cttab(c) & 0x00000014) != 0); } /* (__H | __O)                           */
static inline bool u__isipv4(  unsigned char c)  { return ((u_cttab(c) & 0x00001020) != 0); } /* (__N | __D)                           */
static inline bool u__isblank( unsigned char c)  { return ((u_cttab(c) & 0x00000101) != 0); } /* (__S | __B)                           */
static inline bool u__isalpha( unsigned char c)  { return ((u_cttab(c) & 0x0000A000) != 0); } /* (__L | __U)                           */
static inline bool u__isxdigit(unsigned char c)  { return ((u_cttab(c) & 0x00081000) != 0); } /* (__X | __D)                           */
static inline bool u__isalnum( unsigned char c)  { return ((u_cttab(c) & 0x0000B000) != 0); } /* (__L | __U | __D)                     */
static inline bool u__islitem( unsigned char c)  { return ((u_cttab(c) & 0x00000109) != 0); } /* (__S | __V | __B)                     */
static inline bool u__ispecial(unsigned char c)  { return ((u_cttab(c) & 0x00000034) != 0); } /* (__H | __O | __N)                     */
static inline bool u__isipv6(  unsigned char c)  { return ((u_cttab(c) & 0x00081060) != 0); } /* (__N | __G | __X | __D)               */
static inline bool u__isgraph( unsigned char c)  { return ((u_cttab(c) & 0x0000F000) != 0); } /* (__L | __U | __D | __I)               */
static inline bool u__isprint( unsigned char c)  { return ((u_cttab(c) & 0x0000F001) != 0); } /* (__S | __L | __U | __D | __I)         */
static inline bool u__isname(  unsigned char c)  { return ((u_cttab(c) & 0x0000B080) != 0); } /* (__Q | __L | __U | __D)               */
static inline bool u__isename( unsigned char c)  { return ((u_cttab(c) & 0x000030B0) != 0); } /* (__O | __N | __Q | __L | __D)         */
static inline bool u__ishname( unsigned char c)  { return ((u_cttab(c) & 0x0000B0B0) != 0); } /* (__O | __N | __Q | __L | __D | __U)   */
static inline bool u__isbase64(unsigned char c)  { return ((u_cttab(c) & 0x0010B000) != 0); } /* (__A | __L | __U | __D)               */
static inline bool u__isb64url(unsigned char c)  { return ((u_cttab(c) & 0x0000B090) != 0); } /* (      __L | __U | __D | __O | __Q)   */

static inline bool u__isdigit(unsigned char c)   { return ((u_cttab(c) & 0x00001000) != 0); } /* __D  Digit */
static inline bool u__isdigitw0(unsigned char c) { return ((u_cttab(c) & 0x40000000) != 0); } /* __DX Digit without '0' */

static inline bool u__isfnameinvalid(unsigned char c) { return ((u_cttab(c) & 0x08000000) != 0); } /* __UF                   */
static inline bool u__isvalidchar(   unsigned char c) { return ((u_cttab(c) & 0x10000000) != 0); } /* __XM                   */
static inline bool u__isxmlvalidchar(unsigned char c) { return ((u_cttab(c) & 0x10000300) != 0); } /* __XM | __B | __R       */
static inline bool u__isendtoken(    unsigned char c) { return ((u_cttab(c) & 0x20000009) != 0); } /* __XE | __V | __S       */
static inline bool u__isnumberchar(  unsigned char c) { return ((u_cttab(c) & 0x00001034) != 0); } /*  __H | __O | __N | __D */

/* buffer type identification - Assumed an ISO-1 character set */

U_EXPORT bool u_isURL(       const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isHTML(      const char* restrict s            ) __pure;
U_EXPORT bool u_isName(      const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isDigit(     const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isBase64(    const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isBase64Url( const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isMacAddr(   const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isXMacAddr(  const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isHostName(  const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isFileName(  const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isWhiteSpace(const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isPrintable( const char* restrict s, uint32_t n, bool bline) __pure;

U_EXPORT bool u_isUrlEncodeNeeded(const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isUrlEncoded(     const char* restrict s, uint32_t n, bool bquery) __pure;

U_EXPORT const char* u_isUrlScheme(const char* restrict url, uint32_t len) __pure;

static inline void u_getXMAC(const char* restrict src, char* restrict dst)
{
   U_INTERNAL_TRACE("u_getXMAC(%p,%p)", src, dst)

   U_INTERNAL_ASSERT(u_isMacAddr(src,12+5))

   /**
    * %2u:%2u:%2u:%2u:%2u:%2u
    *
    * (void) memcpy(dst,    src,      2);
    * (void) memcpy(dst+ 2, src+ 2+1, 2);
    * (void) memcpy(dst+ 4, src+ 4+2, 2);
    * (void) memcpy(dst+ 6, src+ 6+3, 2);
    * (void) memcpy(dst+ 8, src+ 8+4, 2);
    * (void) memcpy(dst+10, src+10+5, 2);
    */

   u_put_unalignedp16(dst,    *(uint16_t*)src);
   u_put_unalignedp16(dst+ 2, *(uint16_t*)(src+ 2+1));
   u_put_unalignedp16(dst+ 4, *(uint16_t*)(src+ 4+2));
   u_put_unalignedp16(dst+ 6, *(uint16_t*)(src+ 6+3));
   u_put_unalignedp16(dst+ 8, *(uint16_t*)(src+ 8+4));
   u_put_unalignedp16(dst+10, *(uint16_t*)(src+10+5));

   U_INTERNAL_ASSERT(u_isXMacAddr(dst,12))
}

static inline int u_equal(const void* restrict s1, const void* restrict s2, uint32_t n, bool ignore_case) /* Equal with ignore case */
{
   U_INTERNAL_TRACE("u_equal(%p,%p,%u)", s1, s2, n)

   U_INTERNAL_ASSERT_MAJOR(n,0)
   U_INTERNAL_ASSERT_POINTER(s1)
   U_INTERNAL_ASSERT_POINTER(s2)

   return (ignore_case ? u__strncasecmp((const char*)s1, (const char*)s2, n)
                       :         memcmp(             s1,              s2, n));
}

enum TextType {
   U_TYPE_TEXT_ASCII, /* X3.4, ISO-8859, non-ISO ext. ASCII */
   U_TYPE_TEXT_UTF8,
   U_TYPE_TEXT_UTF16LE,
   U_TYPE_TEXT_UTF16BE,
   U_TYPE_BINARY_DATA
};

extern U_EXPORT const unsigned char u_validate_utf8[];

U_EXPORT bool u_isText( const unsigned char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isUTF8( const unsigned char* restrict s, uint32_t n) __pure;
U_EXPORT int  u_isUTF16(const unsigned char* restrict s, uint32_t n) __pure;

static inline bool u_isBinary(const unsigned char* restrict s, uint32_t n) { return ((u_isText(s,n) || u_isUTF8(s,n) || u_isUTF16(s,n)) == false); }

U_EXPORT unsigned long u__strtoul( const char* restrict s, uint32_t len) __pure;
U_EXPORT          long u__strtol(  const char* restrict s, uint32_t len) __pure;
U_EXPORT       int64_t u__strtoll( const char* restrict s, uint32_t len) __pure;
U_EXPORT      uint64_t u__strtoull(const char* restrict s, uint32_t len) __pure;

static inline unsigned long u_strtoul(const char* restrict s, const char* restrict e)
{
#ifndef DEBUG
   return u__strtoul(s, e-s);
#else
   unsigned long value = u__strtoul(s, e-s), tmp = strtoul(s, U_NULLPTR, 10);

   if (value != tmp)
      {
      U_WARNING("u_strtoul(%.*S); value = %lu differs from ::strtoul() = %lu", e-s, s, value, tmp);
      }

   return value;
#endif
}

static inline long u_strtol(const char* restrict s, const char* restrict e)
{
#ifndef DEBUG
   return u__strtol(s, e-s);
#else
   long value = u__strtol(s, e-s), tmp = strtol(s, U_NULLPTR, 10);

   if (value != tmp)
      {
      U_WARNING("u_strtol(%.*S); value = %ld differs from ::strtol() = %ld", e-s, s, value, tmp);
      }

   return value;
#endif
}

static inline int64_t u_strtoll( const char* restrict s, const char* restrict e)
{
#ifndef DEBUG
   return u__strtoll(s, e-s);
#else
   int64_t value = u__strtoll(s, e-s), tmp = strtoll(s, U_NULLPTR, 10);

   if (value != tmp)
      {
      U_WARNING("u_strtoll(%.*S); value = %lld differs from ::strtoll() = %lld", e-s, s, value, tmp);
      }

   return value;
#endif
}

static inline uint64_t u_strtoull(const char* restrict s, const char* restrict e)
{
#ifndef DEBUG
   return u__strtoull(s, e-s);
#else
   uint64_t value = u__strtoull(s, e-s), tmp = strtoull(s, U_NULLPTR, 10);

   if (value != tmp)
      {
      U_WARNING("u_strtoull(%.*S); value = %llu differs from ::strtoull() = %llu", e-s, s, value, tmp);
      }

   return value;
#endif
}

extern U_EXPORT unsigned long u_strtoulp( const char** restrict s);
extern U_EXPORT uint64_t      u_strtoullp(const char** restrict s);

extern U_EXPORT unsigned long u__atoi(const char* restrict s) __pure;

static inline unsigned long u_atoi(const char* restrict s)
{
#ifndef DEBUG
   return u__atoi(s);
#else
   unsigned long value = u__atoi(s), tmp = atoi(s);

   if (value != tmp)
      {
      U_WARNING("u_atoi(%12S); value = %lu differs from ::atoi() = %lu", s, value, tmp);
      }

   return value;
#endif
}

U_EXPORT int8_t u_log2(uint64_t value) __pure;

static inline unsigned u__octc2int(unsigned char c) { return ((c - '0') & 07); }

/**
 * Quick and dirty int->hex. The only standard way is to call snprintf (?), which is undesirably slow for such a frequently-called function...
 */

extern U_EXPORT const unsigned char u__ct_hex2int[112];

static inline unsigned int u__hexc2int(unsigned char c) { return u__ct_hex2int[c]; }

static inline void u_int2hex(char* restrict p, uint32_t n) { int s; for (s = 28; s >= 0; s -= 4, ++p) *p = "0123456789ABCDEF"[((n >> s) & 0x0F)]; }

U_EXPORT unsigned long u_hex2int(const char* restrict s, uint32_t len) __pure;

/* ip address type identification */

U_EXPORT bool u_isIPv4Addr(const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isIPv6Addr(const char* restrict s, uint32_t n) __pure;

static inline bool u_isIPAddr(bool IPv6, const char* restrict p, uint32_t n) { return (IPv6 ? u_isIPv6Addr(p, n)
                                                                                            : u_isIPv4Addr(p, n)); }

U_EXPORT uint32_t u_set_uptime(char* buffer); /* Get the uptime of the system (seconds) */

static inline uint32_t u_get_uptime(void) /* Get the uptime of the system (seconds) */
{
   /**
    * /proc/uptime (ex: 1753.44 6478.08)
    *
    * This file contains two numbers: how long the system has been running (seconds), and the amount of time spent in idle process (seconds)
    */

#if defined(U_LINUX) && !defined(U_COVERITY_FALSE_POSITIVE)
   char buffer[12];

   U_INTERNAL_TRACE("u_get_uptime()")

   if (u_set_uptime(buffer)) return u_atoi(buffer);
#endif

   return 0;
}

/**
 * sign
 * |  exponent
 * |  |
 * [0][11111111111][yyyyxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx]
 *                  |   |
 *                  tag |
 *                      payload
 *
 * 48 bits payload [enough](http://en.wikipedia.org/wiki/X86-64#Virtual_address_space_details)
 * for store any pointer on x64. Double use zero tag, so infinity and nan are accessible
 */

#define U_VALUE_TAG_MASK     0xF
#define U_VALUE_TAG_SHIFT    47
#define U_VALUE_NAN_MASK     0x7FF8000000000000ULL
#define U_VALUE_PAYLOAD_MASK 0x00007FFFFFFFFFFFULL

typedef enum ValueType {
   U_REAL_VALUE =  0, // double value
    U_INT_VALUE =  1, //   signed integer value
   U_UINT_VALUE =  2, // unsigned integer value
   U_TRUE_VALUE =  3, // bool value
  U_FALSE_VALUE =  4, // bool value
 U_STRING_VALUE =  5, // string value
    U_UTF_VALUE =  6, // string value (need to be emitted)
  U_ARRAY_VALUE =  7, // array value (ordered list)
 U_OBJECT_VALUE =  8, // object value (collection of name/value pairs)
   U_NULL_VALUE =  9, // null value
U_COMPACT_VALUE = 10, // compact value
U_BOOLEAN_VALUE = 11, // bool value
   U_CHAR_VALUE = 12, //   signed char value
  U_UCHAR_VALUE = 13, // unsigned char value
  U_SHORT_VALUE = 14, //   signed short integer value
 U_USHORT_VALUE = 15, // unsigned short integer value
   U_LONG_VALUE = 16, //   signed long value
  U_ULONG_VALUE = 17, // unsigned      long value
  U_LLONG_VALUE = 18, //   signed long long value
 U_ULLONG_VALUE = 19, // unsigned long long value
  U_FLOAT_VALUE = 20, // float value
  U_LREAL_VALUE = 21  // long double value
} ValueType;

static inline uint8_t  u_getTag(    uint64_t val) { return (val >> U_VALUE_TAG_SHIFT) & U_VALUE_TAG_MASK; }
static inline uint64_t u_getPayload(uint64_t val) { return (val & U_VALUE_PAYLOAD_MASK); }

static inline uint64_t u_getValue(uint8_t tag, void* payload)
{
   U_INTERNAL_TRACE("u_getValue(%u,%p)", tag, payload)

   U_INTERNAL_ASSERT(payload <= (void*)U_VALUE_PAYLOAD_MASK)

   return                   U_VALUE_NAN_MASK   |
          ((uint64_t)tag << U_VALUE_TAG_SHIFT) |
          ((uint64_t)(long)payload & U_VALUE_PAYLOAD_MASK);
}

static inline void u_setTag(uint8_t tag, uint64_t* pval) { uint64_t payload = u_getPayload(*pval); *pval = u_getValue(tag, (void*)(long)payload); }

#if defined(USE_PGSQL) && defined(LIBPGPORT_NOT_FOUND)
static inline void pg_qsort(void* a, size_t n, size_t es, int (*cmp)(const void*, const void*)) { qsort(a, n, es, cmp); }
#endif

#ifdef __cplusplus
}
#endif

#endif
