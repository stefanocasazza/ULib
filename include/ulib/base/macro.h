/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    macro.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_BASE_MACRO_H
#define ULIB_BASE_MACRO_H 1

/* Manage location info */

#define U_SET_LOCATION_INFO (u_num_line      = __LINE__, \
                             u_name_file     = __FILE__, \
                             u_name_function = __PRETTY_FUNCTION__)

/**
 * Manage debug for C library
 *
 * NB: the token preceding the special `##' must be a comma, and there must be white space between that comma and whatever comes immediately before it
 */

#ifdef DEBUG_DEBUG
#  define U_INTERNAL_TRACE(format,args...) u_internal_print(false, format"\n" , ##args);
#  define U_INTERNAL_PRINT(format,args...) u_internal_print(false, format"\n" , ##args);
#else
#  define U_INTERNAL_TRACE(format,args...)
#  define U_INTERNAL_PRINT(format,args...)
#endif

/* Design by contract - if (assertion == false) then stop */

#ifdef DEBUG
#  define U_ASSERT_MACRO(assertion,msg,info) \
      if ((bool)(assertion) == false) { \
         u__printf(STDERR_FILENO, U_CONSTANT_TO_PARAM("%W%N%W: %Q%W%s%W\n" \
         "-------------------------------------\n" \
         " pid: %W%P%W\n" \
         " file: %W%s%W\n" \
         " line: %W%d%W\n" \
         " function: %W%s%W\n" \
         " assertion: \"(%W%s%W)\" %W%s%W\n" \
         "-------------------------------------"), \
         GREEN, YELLOW, -1, CYAN, msg, YELLOW, \
         CYAN, YELLOW, \
         CYAN, __FILE__, YELLOW, \
         CYAN, __LINE__, YELLOW, \
         CYAN, __PRETTY_FUNCTION__, YELLOW, \
         CYAN, #assertion, YELLOW, MAGENTA, info, YELLOW); }
#elif defined(U_TEST)
#  ifdef HAVE_ASSERT_H
#     include <assert.h>
#     define U_ASSERT_MACRO(assertion,msg,info) assert(assertion);
#  else
#    ifdef __cplusplus
      extern "C" {
#    endif
      void __assert(const char* __assertion, const char* __file, int __line, const char* __function);
#    ifdef __cplusplus
      }
#    endif
#    define U_ASSERT_MACRO(assertion,msg,info) { if ((int)(assertion) == 0) { __assert(#assertion, __FILE__, __LINE__,__PRETTY_FUNCTION__);  } }
#  endif
#endif

#if defined(DEBUG) || defined(U_TEST)
#  if !defined(U_LINUX) || defined(U_SERVER_CAPTIVE_PORTAL)
#     define U_NULL_POINTER (const void*)0
#  else
#     define U_NULL_POINTER (const void*)0x0000ffff
#  endif

#  define U_DEBUG(fmt,args...) { u__printf(STDERR_FILENO, U_CONSTANT_TO_PARAM("%W%N%W: %WDEBUG: %9D (pid %P) " fmt "%W"), BRIGHTCYAN, RESET, YELLOW, ##args, RESET); }

#  define U_INTERNAL_ASSERT(expr)            { U_ASSERT_MACRO(expr,"ASSERTION FALSE","") }
#  define U_INTERNAL_ASSERT_MSG(expr,info)   { U_ASSERT_MACRO(expr,"ASSERTION FALSE",info) }

#  define U_INTERNAL_ASSERT_MINOR(a,b)          { U_ASSERT_MACRO((a)<(b),"NOT LESS","") }
#  define U_INTERNAL_ASSERT_MINOR_MSG(a,b,info) { U_ASSERT_MACRO((a)<(b),"NOT LESS",info) }

#  define U_INTERNAL_ASSERT_MAJOR(a,b)          { U_ASSERT_MACRO((a)>(b),"NOT GREATER","") }
#  define U_INTERNAL_ASSERT_MAJOR_MSG(a,b,info) { U_ASSERT_MACRO((a)>(b),"NOT GREATER",info) }

#  define U_INTERNAL_ASSERT_EQUALS(a,b)          { U_ASSERT_MACRO((a)==(b),"NOT EQUALS","") }
#  define U_INTERNAL_ASSERT_EQUALS_MSG(a,b,info) { U_ASSERT_MACRO((a)==(b),"NOT EQUALS",info) }

#  define U_INTERNAL_ASSERT_DIFFERS(a,b)          { U_ASSERT_MACRO((a)!=(b),"NOT DIFFERENT","") }
#  define U_INTERNAL_ASSERT_DIFFERS_MSG(a,b,info) { U_ASSERT_MACRO((a)!=(b),"NOT DIFFERENT",info) }

#  define U_INTERNAL_ASSERT_POINTER(ptr)          { U_ASSERT_MACRO((const void*)ptr>U_NULL_POINTER,"~NULL POINTER","") }
#  define U_INTERNAL_ASSERT_POINTER_MSG(ptr,info) { U_ASSERT_MACRO((const void*)ptr>U_NULL_POINTER,"~NULL POINTER",info) }

#  define U_INTERNAL_ASSERT_POINTER_ADDR_SPACE(ptr)          { U_ASSERT_MACRO((access((const char*)ptr,F_OK)==0)||(errno!=EFAULT),"INVALID POINTER","") }
#  define U_INTERNAL_ASSERT_POINTER_ADDR_SPACE_MSG(ptr,info) { U_ASSERT_MACRO((access((const char*)ptr,F_OK)==0)||(errno!=EFAULT),"INVALID POINTER",info) }

#  define U_INTERNAL_ASSERT_RANGE(a,x,b)          { U_ASSERT_MACRO((x)>=(a)&&(x)<=(b),"VALUE OUT OF RANGE","") }
#  define U_INTERNAL_ASSERT_RANGE_MSG(a,x,b,info) { U_ASSERT_MACRO((x)>=(a)&&(x)<=(b),"VALUE OUT OF RANGE",info) }
#else
#  define U_DEBUG(fmt,args...) {}

#  define U_INTERNAL_ASSERT(expr)
#  define U_INTERNAL_ASSERT_MINOR(a,b)
#  define U_INTERNAL_ASSERT_MAJOR(a,b)
#  define U_INTERNAL_ASSERT_EQUALS(a,b)
#  define U_INTERNAL_ASSERT_DIFFERS(a,b)
#  define U_INTERNAL_ASSERT_POINTER(ptr)
#  define U_INTERNAL_ASSERT_RANGE(a,x,b)

#  define U_INTERNAL_ASSERT_MSG(expr,info)
#  define U_INTERNAL_ASSERT_MINOR_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_MAJOR_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_EQUALS_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_DIFFERS_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_POINTER_MSG(ptr,info)
#  define U_INTERNAL_ASSERT_RANGE_MSG(a,x,b,info)
#endif

/* Manage message info */

#define U_MESSAGE(fmt,args...) \
{ u__printf(STDERR_FILENO, U_CONSTANT_TO_PARAM("%W%N%W: " fmt), BRIGHTCYAN, RESET, ##args); }

#define U_ERROR(fmt,args...) \
{ u_flag_exit = -1; u__printf(STDERR_FILENO, U_CONSTANT_TO_PARAM("%W%N%W: %WERROR: %9D (pid %P) " fmt " - Exiting...%W"), BRIGHTCYAN, RESET, RED, ##args, RESET); }

#define U_ABORT(fmt,args...) \
{ u_flag_exit = -2; u__printf(STDERR_FILENO, U_CONSTANT_TO_PARAM("%W%N%W: %WABORT: %9D (pid %P) " fmt "%W"), BRIGHTCYAN, RESET, RED, ##args, RESET); }

#define U_WARNING(fmt,args...) \
{ u_flag_exit = 2; u__printf(STDERR_FILENO, U_CONSTANT_TO_PARAM("%W%N%W: %WWARNING: %9D (pid %P) " fmt "%W"), BRIGHTCYAN, RESET, YELLOW, ##args, RESET); }

#define   U_ERROR_SYSCALL(msg)   U_ERROR("%R",msg)
#define   U_ABORT_SYSCALL(msg)   U_ABORT("%R",msg)
#define U_WARNING_SYSCALL(msg) U_WARNING("%R",msg)

/* Get string costant size from compiler */

#define U_CONSTANT_SIZE(str)     (unsigned int)(sizeof(str)-1)
#define U_CONSTANT_TO_PARAM(str) str,U_CONSTANT_SIZE(str)
#define U_CONSTANT_TO_TRACE(str)     U_CONSTANT_SIZE(str),str

#define U_STREQ(a,n,b) (n == U_CONSTANT_SIZE(b) && (memcmp((const char* restrict)(a),b,U_CONSTANT_SIZE(b)) == 0))

/* Defs */

#ifndef U_min
#define U_min(x,y) ((x) <= (y) ? (x) : (y))
#endif
#ifndef U_max
#define U_max(x,y) ((x) >= (y) ? (x) : (y))
#endif
#ifndef U_abs
#define U_abs(x,y) ((x) >= (y) ? (x-y) : (y-x))
#endif

#define U_ENTRY(name) {name, #name}

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef PAGESIZE
#define PAGESIZE 4096U
#endif
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif
#ifndef MAX_FILENAME_LEN
#define MAX_FILENAME_LEN 255U
#endif

#define U_2M    (2U*1024U*1024U)
#define U_1G (1024U*1024U*1024U)

#define U_2M_MASK      (U_2M-1)
#define U_1G_MASK      (U_1G-1)
#define U_PAGEMASK (PAGESIZE-1)

#define U_TO_FREE   ((uint32_t)-2)
#define U_NOT_FOUND ((uint32_t)-1)

#define U_ONE_HOUR_IN_SECOND  (60L * 60L)
#define U_ONE_DAY_IN_SECOND   (24L * U_ONE_HOUR_IN_SECOND)
#define U_ONE_WEEK_IN_SECOND  ( 7L * U_ONE_DAY_IN_SECOND)
#define U_ONE_MONTH_IN_SECOND (31L * U_ONE_DAY_IN_SECOND)
#define U_ONE_YEAR_IN_SECOND  (12L * U_ONE_MONTH_IN_SECOND)

#define U_NUM_ELEMENTS(array) (unsigned int)(sizeof(array) / sizeof(array[0]))

enum AffermationType {
   U_MAYBE   = 0x0000,
   U_YES     = 0x0001,
   U_NOT     = 0x0002,
   U_PARTIAL = 0x0004,
   U_CLOSE   = 0x0008
};

#define GZIP_MAGIC "\037\213" /* Magic header for gzip files, 1F 8B */

/* MIME type */

#define U_unknow  -1
#define U_know    'K'

#define U_css     'c' /* text/css */
#define U_flv     'f' /* video/x-flv */
#define U_gif     'g' /* image/gif */
#define U_html    'h' /* text/html */
#define U_ico     'i' /* image/x-icon */
#define U_js      'j' /* application/javascript */
#define U_png     'p' /* image/png */
#define U_txt     't' /* text/plain */

#define U_jpg     'J' /* image/jpg */
#define U_gz      'Z' /* gzip */

/* MIME type for dynamic content */

#define U_usp    '0' /* USP (ULib Servlet Page) */
#define U_csp    '1' /* CSP (C    Servlet Page) */
#define U_cgi    '2' /* cgi-bin */
#define U_ssi    '3' /* SSI */
#define U_php    '4' /* PHP  script */
#define U_ruby   '5' /* Ruby script */
#define U_perl   '6' /* Perl script */
#define U_python '7' /* Python script */

#define U_CTYPE_TEXT              "text/plain"
#define U_CTYPE_HTML              "text/html; charset=UTF-8"
#define U_CTYPE_TEXT_WITH_CHARSET "text/plain; charset=UTF-8"

/**
 * Enumeration of Hash (Digest) types
 * 
 * The hash types known to openssl
 */

typedef enum {
   U_HASH_MD2       = 0,
   U_HASH_MD5       = 1,
   U_HASH_SHA       = 2,
   U_HASH_SHA1      = 3,
   U_HASH_SHA224    = 4,
   U_HASH_SHA256    = 5,
   U_HASH_SHA384    = 6,
   U_HASH_SHA512    = 7,
   U_HASH_MDC2      = 8,
   U_HASH_RIPEMD160 = 9
} UHashType;

#define U_PTR2INT(x) ((unsigned int)(long)x)
#define U_INT2PTR(x) (       (void*)(long)x)

union uucflag {
   unsigned char c[4];
   uint16_t lo; 
   uint16_t hi; 
   uint32_t u;
};

union uucflag64 {
   unsigned char c[8];
   uint32_t s;
   uint64_t u;
};

/* Type of file sizes and offsets (LFS) */

#if SIZEOF_OFF_T == SIZEOF_LONG
#  define u_strtooff(nptr,endptr,base) (off_t) strtol((nptr),(char**)(endptr),(base))
#else
#  define u_strtooff(nptr,endptr,base) (off_t) strtoll((nptr),(char**)(endptr),(base))

#  if SIZEOF_OFF_T != 8
#     error ERROR: unexpected size of SIZEOF_OFF_T
#  endif
#endif

#ifdef ENABLE_LFS
#  define U_SEEK_BEGIN 0LL
#else
#  define U_SEEK_BEGIN 0L
#endif

/* Line terminator */

#define U_LF    "\n"
#define U_LF2   "\n\n"
#define U_CRLF  "\r\n"
#define U_CRLF2 "\r\n\r\n"

#define U_CONCAT2(a,b)   a##b
#define U_CONCAT3(a,b,c) a##b##c

/* These two macros makes it possible to turn the builtin line expander into a string literal */

#define U_STRINGIFY2(x) #x
#define U_STRINGIFY(x) U_STRINGIFY2(x)

/* Provide convenience macros for handling structure fields through their offsets */

#ifndef offsetof
#define offsetof(st,m) ((long)(&((st*)0)->m))
#endif

#define U_STRUCT_MEMBER_SIZE(type,member)   (sizeof(((type*)0)->member))
#define U_STRUCT_MEMBER_OFFSET(type,member)       (&((type*)0)->member)

#define U_STRUCT_IS_LAST_MEMBER(type,member)    (U_STRUCT_MEMBER_END_OFFSET(type,member) == sizeof(type))
#define U_STRUCT_MEMBER_END_OFFSET(type,member) (U_STRUCT_MEMBER_SIZE(type,member) + U_STRUCT_OFFSET(type,member))

/**
 * x86 allows unaligned reads and writes (so for example you can read a 16-bit value
 * from a non-even address), but other architectures do not (ARM will raise SIGILL)
 */

#ifndef HAVE_NO_UNALIGNED_ACCESSES
#  define u_get_unalignedp16(ptr)     (*(uint16_t*)(ptr))
#  define u_get_unalignedp32(ptr)     (*(uint32_t*)(ptr))
#  define u_get_unalignedp64(ptr)     (*(uint64_t*)(ptr))
#  define u_put_unalignedp16(ptr,val) (*(uint16_t*)(ptr) = (val))
#  define u_put_unalignedp32(ptr,val) (*(uint32_t*)(ptr) = (val))
#  define u_put_unalignedp64(ptr,val) (*(uint64_t*)(ptr) = (val))
#else
struct u_una_u16 { uint16_t x __attribute__((packed)); };
struct u_una_u32 { uint32_t x __attribute__((packed)); };
struct u_una_u64 { uint64_t x __attribute__((packed)); };

static inline uint16_t u_get_unalignedp16(const void* p)               { const struct u_una_u16 *ptr = (const struct u_una_u16*)p; return ptr->x; }
static inline uint32_t u_get_unalignedp32(const void* p)               { const struct u_una_u32 *ptr = (const struct u_una_u32*)p; return ptr->x; }
static inline uint64_t u_get_unalignedp64(const void* p)               { const struct u_una_u64 *ptr = (const struct u_una_u64*)p; return ptr->x; }
static inline void     u_put_unalignedp16(      void* p, uint16_t val) {       struct u_una_u16 *ptr = (      struct u_una_u16*)p;        ptr->x = val; } 
static inline void     u_put_unalignedp32(      void* p, uint32_t val) {       struct u_una_u32 *ptr = (      struct u_una_u32*)p;        ptr->x = val; }
static inline void     u_put_unalignedp64(      void* p, uint64_t val) {       struct u_una_u64 *ptr = (      struct u_una_u64*)p;        ptr->x = val; }
#endif

/**
 * u_get_unaligned - get value from possibly mis-aligned location
 *
 * This macro should be used for accessing values larger in size than
 * single bytes at locations that are expected to be improperly aligned
 *
 * Note that unaligned accesses can be very expensive on some architectures
 */

#define u_get_unaligned16(ref) u_get_unalignedp16(&(ref))
#define u_get_unaligned32(ref) u_get_unalignedp32(&(ref))
#define u_get_unaligned64(ref) u_get_unalignedp64(&(ref))

/**
 * u_put_unaligned - put value to a possibly mis-aligned location
 *
 * This macro should be used for placing values larger in size than
 * single bytes at locations that are expected to be improperly aligned
 *
 * Note that unaligned accesses can be very expensive on some architectures
 */

#define u_put_unaligned16(ref,val) u_put_unalignedp16(&(ref),(uint16_t)(val))
#define u_put_unaligned32(ref,val) u_put_unalignedp32(&(ref),(uint32_t)(val))
#define u_put_unaligned64(ref,val) u_put_unalignedp64(&(ref),(uint64_t)(val))

/* Endian order (network byte order is big endian) */

#if __BYTE_ORDER == __LITTLE_ENDIAN /* the host byte order is Least Significant Byte first */

#  define u_htonll(x) (((uint64_t)htonl((uint32_t)x))<<32 | htonl((uint32_t)(x>>32)))
#  define u_ntohll(x) (((uint64_t)ntohl((uint32_t)x))<<32 | ntohl((uint32_t)(x>>32)))

#  define u_test_bit(n,c) (((c) & (1 << n)) != 0)

#  define U_NUM2STR16( ptr,val1) u_put_unalignedp16((ptr),u_get_unalignedp16(u_ctn2s+((val1)*2)))

#  define U_NUM2STR32(ptr,val1,val2) u_put_unalignedp32((ptr),u_get_unalignedp16(u_ctn2s+((val1)*2))|\
                                                              u_get_unalignedp16(u_ctn2s+((val2)*2))<<16)

#  define U_NUM2STR64(ptr,c,val1,val2,val3) u_put_unalignedp64((ptr),u_get_unalignedp16(u_ctn2s+((val1)*2))|\
                                                           (uint64_t)(c)<<16|\
                                                          ((uint64_t)u_get_unalignedp16(u_ctn2s+((val2)*2))<<24)|\
                                                           (uint64_t)(c)<<40|\
                                                          ((uint64_t)u_get_unalignedp16(u_ctn2s+((val3)*2))<<48))

#  define U_MULTICHAR_CONSTANT16(a,b)               (uint16_t)((uint8_t)(a)|\
                                                               (uint8_t)(b)<<8)

#  define U_MULTICHAR_CONSTANT32(a,b,c,d)           (uint32_t)((uint8_t)(a)|\
                                                               (uint8_t)(b)<<8|\
                                                               (uint8_t)(c)<<16|\
                                                               (uint8_t)(d)<<24)

#  define U_MULTICHAR_CONSTANT64(a,b,c,d,e,f,g,h)   (uint64_t)((uint64_t)(a)|\
                                                              ((uint64_t)(b))<<8|\
                                                              ((uint64_t)(c))<<16|\
                                                              ((uint64_t)(d))<<24|\
                                                              ((uint64_t)(e))<<32|\
                                                              ((uint64_t)(f))<<40|\
                                                              ((uint64_t)(g))<<48|\
                                                              ((uint64_t)(h))<<56)
#else /* the host byte order is Most Significant Byte first */

#  define u_htonll(x) (x)
#  define u_ntohll(x) (x)

#  define u_test_bit(n,c) ((((c) >> n) & 1) != 0)

#  define u_invert32(n) (((n) >> 24)               | \
                        (((n) >>  8) & 0x0000ff00) | \
                        (((n) <<  8) & 0x00ff0000) | \
                        ( (n) << 24))

#  define U_NUM2STR16(ptr,val1)  u_put_unalignedp16((ptr),(uint16_t)((uint8_t)(u_ctn2s[((val1)*2)+1])|\
                                                                     (uint8_t)(u_ctn2s[((val1)*2)])<<8))

#  define U_NUM2STR32(ptr,val2,val1) u_put_unalignedp32((ptr),(uint16_t)((uint8_t)(u_ctn2s[((val1)*2)+1])|\
                                                                         (uint8_t)(u_ctn2s[((val1)*2)])<<8)|\
                                                             ((uint16_t)((uint8_t)(u_ctn2s[((val2)*2)+1])|\
                                                                         (uint8_t)(u_ctn2s[((val2)*2)])<<8))<<16)

#  define U_NUM2STR64(ptr,c,val3,val2,val1) u_put_unalignedp64((ptr),((uint64_t)((uint8_t)(u_ctn2s[((val1)*2)+1])|\
                                                                                 (uint8_t)(u_ctn2s[((val1)*2)])<<8))|\
                                                                     (((uint64_t)(c))<<16)|\
                                                                     (((uint64_t)((uint8_t)(u_ctn2s[((val2)*2)+1])|\
                                                                                 (uint8_t)(u_ctn2s[((val2)*2)])<<8))<<24)|\
                                                                     (((uint64_t)(c))<<40)|\
                                                                     (((uint64_t)((uint8_t)(u_ctn2s[((val3)*2)+1])|\
                                                                                 (uint8_t)(u_ctn2s[((val3)*2)])<<8))<<48))

#  define U_MULTICHAR_CONSTANT16(b,a)               (uint16_t)((uint8_t)(a)|\
                                                               (uint8_t)(b)<<8)

#  define U_MULTICHAR_CONSTANT32(d,c,b,a)           (uint32_t)((uint8_t)(a)|\
                                                               (uint8_t)(b)<<8|\
                                                               (uint8_t)(c)<<16|\
                                                               (uint8_t)(d)<<24)

#  define U_MULTICHAR_CONSTANT64(h,g,f,e,d,c,b,a)   (uint64_t)((uint64_t)(a)|\
                                                              ((uint64_t)(b))<<8|\
                                                              ((uint64_t)(c))<<16|\
                                                              ((uint64_t)(d))<<24|\
                                                              ((uint64_t)(e))<<32|\
                                                              ((uint64_t)(f))<<40|\
                                                              ((uint64_t)(g))<<48|\
                                                              ((uint64_t)(h))<<56)
#endif

/* Check for dot entry in directory */

#define U_ISDOTS(d_name) (  d_name[0] == '.'  && \
                          ( d_name[1] == '\0' || \
                           (d_name[1] == '.'  && \
                            d_name[2] == '\0')))

/* Memory alignment for pointer */

#define U_MEMORY_ALIGNMENT(ptr, alignment)  ptr += alignment - ((long)ptr & (alignment - 1))

/* Manage number suffix */

#define U_NUMBER_SUFFIX(num,suffix) \
   switch (suffix) { \
      case 'G': num <<= 10; \
      case 'M': num <<= 10; \
      case 'K': \
      case 'k': num <<= 10; }
#endif

/* Optimization if it is enough a resolution of one second */

#if defined(U_LINUX) && defined(ENABLE_THREAD)
#  if defined(U_LOG_DISABLE) && !defined(USE_LIBZ)
#     define U_gettimeofday
#  else
#     define U_gettimeofday { if (u_pthread_time == 0) u_now->tv_sec = time(0); }
#  endif
#else
#  define U_gettimeofday u_now->tv_sec = time(0);
#endif

/* To print size of class */

#define U_PRINT_SIZEOF(class) printf("%u sizeof(%s)\n", sizeof(class), #class)

/* Avoid "unused parameter" warnings */

#if defined(DEBUG) || defined(DEBUG_DEBUG)
#  define U_VAR_UNUSED(x)
#else
#  define U_VAR_UNUSED(x) (void)x;
#endif

/* #undef PLATFORM_VAR */

#ifndef PLATFORM_VAR
#define PLATFORM_VAR ""
#endif

/* Some useful macros for conditionally compiling memcheck features... */

#ifndef HAVE_ARCH64
#  define U_CHECK_MEMORY_SENTINEL      0x0a1b2c3d
#  define U_CHECK_MEMORY_SENTINEL_STR "0x0a1b2c3d" 
#else
#  define U_CHECK_MEMORY_SENTINEL      0x0a1b2c3d4e5f6789
#  define U_CHECK_MEMORY_SENTINEL_STR "0x0a1b2c3d4e5f6789"
#endif
