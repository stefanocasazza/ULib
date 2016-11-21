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

/* Random number generator */ 

U_EXPORT double   u_get_uniform(void);
U_EXPORT uint32_t u_get_num_random(uint32_t range);

static inline void u_set_seed_random(uint32_t u, uint32_t v)
{
   U_INTERNAL_TRACE("u_set_seed_random(%u,%u)", u, v)

   if (u != 0) u_m_w = u;
   if (v != 0) u_m_z = v;

   U_INTERNAL_ASSERT_MAJOR(u_m_w, 0)
   U_INTERNAL_ASSERT_MAJOR(u_m_z, 0)
}

U_EXPORT double u_calcRate(uint64_t bytes, uint32_t msecs, int* restrict units); /* Calculate the transfert rate */

U_EXPORT char* u_getPathRelativ(const char* restrict path, uint32_t* restrict path_len);

U_EXPORT char*    u_memoryDump( char* restrict bp, unsigned char* restrict cp, uint32_t n);
U_EXPORT uint32_t u_memory_dump(char* restrict bp, unsigned char* restrict cp, uint32_t n);

U_EXPORT int  u_getScreenWidth(void) __pure; /* Determine the width of the terminal we're running on */
U_EXPORT bool u_isNumber(const char* restrict s, uint32_t n) __pure;
U_EXPORT uint32_t u_printSize(char* restrict buffer, uint64_t bytes); /* print size using u_calcRate() */
U_EXPORT bool u_rmatch(const char* restrict haystack, uint32_t haystack_len, const char* restrict needle, uint32_t needle_len) __pure;

U_EXPORT const char* u_get_mimetype(const char* restrict suffix, int* pmime_index);

static inline bool u_isSuffixSwap(const char* restrict suffix) // NB: vi tmp...
{
   U_INTERNAL_TRACE("u_isSuffixSwap(%s)", suffix)

   U_INTERNAL_ASSERT_EQUALS(suffix[0], '.')
   U_INTERNAL_ASSERT_EQUALS(strchr(suffix, '/'), 0)

   if (u_get_unalignedp32(suffix) == U_MULTICHAR_CONSTANT32('.','s','w','p')) return true;

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

   p = memmem(s, n, U_CONSTANT_TO_PARAM(U_CRLF2));

   return (p ? (const char*)p - s + U_CONSTANT_SIZE(U_CRLF2) : U_NOT_FOUND);
}

U_EXPORT uint32_t u_findEndHeader( const char* restrict s, uint32_t n) __pure; /* find sequence of U_CRLF2 or U_LF2 */

U_EXPORT bool   u_endsWith(const char* restrict a, uint32_t n1, const char* restrict b, uint32_t n2) __pure; /* check if string a terminate with string b */
U_EXPORT bool u_startsWith(const char* restrict a, uint32_t n1, const char* restrict b, uint32_t n2) __pure; /* check if string a start     with string b */

/* find char not quoted */

U_EXPORT const char* u_find_char(const char* restrict s, const char* restrict end, char c) __pure;

/* skip string delimiter or white space and line comment */

U_EXPORT const char* u_skip(const char* restrict s, const char* restrict end, const char* restrict delim, char line_comment) __pure;

/* delimit token */

U_EXPORT const char* u_delimit_token(const char* restrict s,   const char** restrict p, const char* restrict end, const char*  restrict delim, char skip_line_comment);

/* Search a string for any of a set of characters. Locates the first occurrence in the string s of any of the characters in the string accept */

U_EXPORT const char* u__strpbrk(const char* restrict s, uint32_t slen, const char* restrict accept) __pure;

/* Search a string for a terminator of a group of delimitator {} [] () <%%>...*/

U_EXPORT const char* u_strpend(const char* restrict s, uint32_t slen,
                               const char* restrict group_delimitor, uint32_t group_delimitor_len, char skip_line_comment) __pure;

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

U_EXPORT bool u_match_with_OR(bPFpcupcud pfn_match, const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags) __pure;

static inline bool u_dosmatch_with_OR(const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("u_dosmatch_with_OR(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), s, n1, n2, pattern, n2, flags)

   return u_match_with_OR(u_dosmatch, s, n1, pattern, n2, flags);
}

static inline bool u_dosmatch_ext_with_OR(const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("u_dosmatch_ext_with_OR(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), s, n1, n2, pattern, n2, flags)

   return u_match_with_OR(u_dosmatch_ext, s, n1, pattern, n2, flags);
}

/* Change the current working directory to the `user` user's home dir, and downgrade security to that user account */

U_EXPORT bool u_runAsUser(const char* restrict user, bool change_dir);

/* Verifies that the passed string is actually an e-mail address */

U_EXPORT bool u_validate_email_address(const char* restrict address, uint32_t address_len) __pure;

/* Perform 'natural order' comparisons of strings. */

U_EXPORT int u_strnatcmp(char const* restrict a, char const* restrict b) __pure;

/* Get address space and rss (resident set size) usage */

U_EXPORT void u_get_memusage(unsigned long* vsz, unsigned long* rss);

/* Get the uptime of the system (seconds) */

U_EXPORT uint32_t u_get_uptime(void);

/* Get the number of the processors including offline CPUs */

U_EXPORT int u_get_num_cpu(void);

/* Pin the process to a particular core */

U_EXPORT void u_bind2cpu(cpu_set_t* cpuset, int n);

/* Set the process to maximum priority that can be used with the scheduling algorithm */

U_EXPORT void u_switch_to_realtime_priority(void);

/**
 * Canonicalize PATH, and build a new path. The new path differs from PATH in that:
 * --------------------------------------------------------------------------------
 * Multiple    '/'   are collapsed to a single '/'
 * Leading     './'  are removed
 * Trailing    '/.'  are removed
 * Trailing    '/'   are removed
 * Non-leading '../' and trailing '..' are handled by removing portions of the path
 * --------------------------------------------------------------------------------
 */

U_EXPORT bool u_canonicalize_pathname(char* restrict path);

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
static inline bool u__isdigit(unsigned char c)   { return ((u_cttab(c) & 0x00001000) != 0); } /* __D Digit */
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
static inline bool u__isename( unsigned char c)  { return ((u_cttab(c) & 0x00000240) != 0); } /* (__G | __R)                           */
static inline bool u__isalnum( unsigned char c)  { return ((u_cttab(c) & 0x0000B000) != 0); } /* (__L | __U | __D)                     */
static inline bool u__islitem( unsigned char c)  { return ((u_cttab(c) & 0x00000109) != 0); } /* (__S | __V | __B)                     */
static inline bool u__ispecial(unsigned char c)  { return ((u_cttab(c) & 0x00000034) != 0); } /* (__H | __O | __N)                     */
static inline bool u__isname(  unsigned char c)  { return ((u_cttab(c) & 0x0000B080) != 0); } /* (__Q | __L | __U | __D)               */
static inline bool u__isipv6(  unsigned char c)  { return ((u_cttab(c) & 0x00081060) != 0); } /* (__N | __G | __X | __D)               */
static inline bool u__isgraph( unsigned char c)  { return ((u_cttab(c) & 0x0000F000) != 0); } /* (__L | __U | __D | __I)               */
static inline bool u__isprint( unsigned char c)  { return ((u_cttab(c) & 0x0000F001) != 0); } /* (__S | __L | __U | __D | __I)         */
static inline bool u__ishname( unsigned char c)  { return ((u_cttab(c) & 0x0000B0B0) != 0); } /* (__O | __N | __Q | __L | __U | __D)   */
static inline bool u__isbase64(unsigned char c)  { return ((u_cttab(c) & 0x0010B000) != 0); } /* (__A | __L | __U | __D)               */
static inline bool u__isb64url(unsigned char c)  { return ((u_cttab(c) & 0x0000B090) != 0); } /* (      __L | __U | __D | __O | __Q)   */

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
U_EXPORT bool u_isHostName(  const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isFileName(  const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isWhiteSpace(const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isPrintable( const char* restrict s, uint32_t n, bool bline) __pure;

U_EXPORT bool u_isUrlEncodeNeeded(const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isUrlEncoded(     const char* restrict s, uint32_t n, bool bquery) __pure;

U_EXPORT const char* u_isUrlScheme(const char* restrict url, uint32_t len) __pure;

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

U_EXPORT bool u_isText(  const unsigned char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isUTF8(  const unsigned char* restrict s, uint32_t n) __pure;
U_EXPORT int  u_isUTF16( const unsigned char* restrict s, uint32_t n) __pure;

static inline bool u_isBinary(const unsigned char* restrict s, uint32_t n) { return ((u_isText(s,n) || u_isUTF8(s,n) || u_isUTF16(s,n)) == false); }

U_EXPORT unsigned long u_strtoul( const char* restrict s, const char* restrict e) __pure;
U_EXPORT uint64_t      u_strtoull(const char* restrict s, const char* restrict e) __pure;
U_EXPORT long          u_strtol(  const char* restrict s, const char* restrict e) __pure;
U_EXPORT int64_t       u_strtoll( const char* restrict s, const char* restrict e) __pure;

static inline unsigned u__octc2int(unsigned char c) { return ((c - '0') & 07); }

/**
 * Quick and dirty int->hex. The only standard way is to call snprintf (?),
 * which is undesirably slow for such a frequently-called function...
 */

extern U_EXPORT const unsigned char u__ct_hex2int[112];

static inline unsigned int u__hexc2int(unsigned char c) { return u__ct_hex2int[c]; }

static inline void u_int2hex(char* restrict p, uint32_t n) { int s; for (s = 28; s >= 0; s -= 4, ++p) *p = "0123456789ABCDEF"[((n >> s) & 0x0F)]; }

U_EXPORT unsigned long u_hex2int(const char* restrict s, const char* restrict e) __pure;

/* ip address type identification */

U_EXPORT bool u_isIPv4Addr(const char* restrict s, uint32_t n) __pure;
U_EXPORT bool u_isIPv6Addr(const char* restrict s, uint32_t n) __pure;

static inline bool u_isIPAddr(bool IPv6, const char* restrict p, uint32_t n) { return (IPv6 ? u_isIPv6Addr(p, n)
                                                                                            : u_isIPv4Addr(p, n)); }

#ifdef __cplusplus
}
#endif

#endif
