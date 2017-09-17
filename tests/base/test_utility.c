/* test_utility.c */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/coder/escape.h>

#include <ctype.h>
#include <stdlib.h>

#ifdef HAVE_FNMATCH
#  include <fnmatch.h>
#endif

#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD FNM_IGNORECASE
#endif

#ifndef FNM_LEADING_DIR
#define FNM_LEADING_DIR FNM_PERIOD
#endif

static void check_match_true(bPFpcupcud pfn_match, const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("check_match_true(%p,%.*s,%u,%.*s,%u,%d)", pfn_match, U_min(n1,128), s, n1, n2, pattern, n2, flags)

   U_INTERNAL_ASSERT(       pfn_match(s, n1, pattern, n2, flags))
   U_INTERNAL_ASSERT_EQUALS(pfn_match(s, n1, pattern, n2, flags | FNM_INVERT), false)
}

static void check_match_false(bPFpcupcud pfn_match, const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("check_match_false(%p,%.*s,%u,%.*s,%u,%d)", pfn_match, U_min(n1,128), s, n1, n2, pattern, n2, flags)

   U_INTERNAL_ASSERT_EQUALS(pfn_match(s, n1, pattern, n2, flags), false)
   U_INTERNAL_ASSERT(       pfn_match(s, n1, pattern, n2, flags | FNM_INVERT))
}

static void check_match1(bPFpcupcud pfn_match)
{
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("01*2345"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("01*2?46"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("010000000 2?45"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("01*2?4?"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("01*00*00*2?46"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("01*2?46?????????????????????"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("01000000002345"), U_CONSTANT_TO_PARAM("*****01*2?46?????????????????????"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("aaAA4b334"),      U_CONSTANT_TO_PARAM("aaAA?b*4"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("aaAA4b334"),      U_CONSTANT_TO_PARAM("aaaa?b*4"), FNM_CASEFOLD);
}

static void check_match2(bPFpcupcud pfn_match)
{
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("pluto.au"),    U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("pippo.gif"),   U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("workflow"),    U_CONSTANT_TO_PARAM("??/??/????|??:??:??|workflow"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("index.shtml"), U_CONSTANT_TO_PARAM("*.css|*.js|*.*html|*.png|*.gif|*.jpg"), 0);
}

static void check_match3(bPFpcupcud pfn_match)
{
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("pluto.au"),    U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("pippo.gif"),   U_CONSTANT_TO_PARAM("*.jpg|*.gif|*.mp3"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("workflow"),    U_CONSTANT_TO_PARAM("??/??/????|??:??:??|workflow"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("index.shtml"), U_CONSTANT_TO_PARAM("*.css|*.js|*.*html|*.png|*.gif|*.jpg"), 0);
}

static void check_match4(bPFpcupcud pfn_match)
{
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("abbb/xy"), U_CONSTANT_TO_PARAM("a*b/*"), FNM_PATHNAME | FNM_PERIOD);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("!#%+,-./01234567889"), U_CONSTANT_TO_PARAM("!#%+,-./01234567889"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM(":;=@ABCDEFGHIJKLMNO"), U_CONSTANT_TO_PARAM(":;=@ABCDEFGHIJKLMNO"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("PQRSTUVWXYZ]abcdefg"), U_CONSTANT_TO_PARAM("PQRSTUVWXYZ]abcdefg"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("hijklmnopqrstuvwxyz"), U_CONSTANT_TO_PARAM("hijklmnopqrstuvwxyz"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("^_{}~"),               U_CONSTANT_TO_PARAM("^_{}~"),               0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("\"$&'()"), U_CONSTANT_TO_PARAM("\\\"\\$\\&\\'\\(\\)"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("*?[\\`|"), U_CONSTANT_TO_PARAM("\\*\\?\\[\\\\\\`\\|"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("<>"),      U_CONSTANT_TO_PARAM("\\<\\>"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("?*["), U_CONSTANT_TO_PARAM("[?*[][?*[][?*[]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("?/b"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("a?b"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("a/?"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("aa/b"), U_CONSTANT_TO_PARAM("?/b"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("aa/b"), U_CONSTANT_TO_PARAM("a?b"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a/bb"), U_CONSTANT_TO_PARAM("a/?"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("abc"), U_CONSTANT_TO_PARAM("[abc]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("[abc]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[abc]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("["), U_CONSTANT_TO_PARAM("[[abc]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("xyz"), U_CONSTANT_TO_PARAM("[!abc]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("abc]"), U_CONSTANT_TO_PARAM("[][abc]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[!]]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("aa]"), U_CONSTANT_TO_PARAM("[!]a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[a-c]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[a-c]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[a-c]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[b-c]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[b-c]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("B"), U_CONSTANT_TO_PARAM("[a-c]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[A-C]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("as"), U_CONSTANT_TO_PARAM("[a-ca-z]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[a-c0-9]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[a-c0-9]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("B"), U_CONSTANT_TO_PARAM("[a-c0-9]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[-b]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[!-b]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[a-c-0-9]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[a-c-0-9]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a:"), U_CONSTANT_TO_PARAM("a[0-9-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a:"), U_CONSTANT_TO_PARAM("a[09-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("*"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("as"), U_CONSTANT_TO_PARAM("[a-c][a-z]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("as"), U_CONSTANT_TO_PARAM("??"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("as*df"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("as*"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("*df"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("asd/sdf"), U_CONSTANT_TO_PARAM("as*dg"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("as*df"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("as*df?"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("as*??"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("a*???"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("*????"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("????*"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("asdf"), U_CONSTANT_TO_PARAM("??*?"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/*"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("*/"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/?"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("?/"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("?"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("."), U_CONSTANT_TO_PARAM("?"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("/"), FNM_PATHNAME);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("//"), U_CONSTANT_TO_PARAM("//"), FNM_PATHNAME);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/.a"), U_CONSTANT_TO_PARAM("/*"), FNM_PATHNAME);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/.a"), U_CONSTANT_TO_PARAM("/?a"), FNM_PATHNAME);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/.a/.b"), U_CONSTANT_TO_PARAM("/*/?b"), FNM_PATHNAME);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/$"), U_CONSTANT_TO_PARAM("\\/\\$"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/["), U_CONSTANT_TO_PARAM("\\/\\["), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/\\$"), U_CONSTANT_TO_PARAM("\\/\\$"), FNM_NOESCAPE);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM(".*"), FNM_PERIOD);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/.asd"), U_CONSTANT_TO_PARAM("*"), FNM_PERIOD);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/as/.df"), U_CONSTANT_TO_PARAM("*/?*f"), FNM_PERIOD);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM("[!a-z]*"), FNM_PERIOD);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/."), FNM_PATHNAME|FNM_PERIOD);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/.a./.b."), U_CONSTANT_TO_PARAM("/.*/.*"), FNM_PATHNAME|FNM_PERIOD);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("foobar"), U_CONSTANT_TO_PARAM("foo*[abc]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo*[abc][xyz]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo?*[abc][xyz]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo?*[abc][x/yz]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("foobaz"), U_CONSTANT_TO_PARAM("foo?*[abc]/[xyz]"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("a/"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a/"), U_CONSTANT_TO_PARAM("a"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("//a"), U_CONSTANT_TO_PARAM("/a"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/a"), U_CONSTANT_TO_PARAM("//a"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("cz"), U_CONSTANT_TO_PARAM("[ab-]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("az"), U_CONSTANT_TO_PARAM("[-a]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("bz"), U_CONSTANT_TO_PARAM("[-ab]z"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("cz"), U_CONSTANT_TO_PARAM("[-ab]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-z"), U_CONSTANT_TO_PARAM("[-ab]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("_"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("!"), U_CONSTANT_TO_PARAM("[\\!-]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[\\!-]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[\\!-]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("Z"), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("["), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[Z-\\\\]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x"), FNM_PATHNAME|FNM_LEADING_DIR);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("*"), FNM_PATHNAME|FNM_LEADING_DIR);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("*x"), FNM_PATHNAME|FNM_LEADING_DIR);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x*"), FNM_PATHNAME|FNM_LEADING_DIR);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("a"), FNM_PATHNAME|FNM_LEADING_DIR);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x/y"),  FNM_PATHNAME|FNM_LEADING_DIR);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("x?y"), FNM_PATHNAME|FNM_LEADING_DIR);
}

/* Tests for fnmatch function */

static void check_match5(bPFpcupcud pfn_match)
{
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.a.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.-.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.-.][.].]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.].][.-.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[[.-.][=u=]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[![.a.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.b.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.b.][.c.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.b.][=b=]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=a=]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[=a=]b]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[=a=][=b=]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=a=][=b=]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=a=][.b.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("="), U_CONSTANT_TO_PARAM("[[=a=]b]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[[=a=]b]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=b=][=c=]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[=b=][.].]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("b"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[[.a.]-c]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[a-[.c.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("d"), U_CONSTANT_TO_PARAM("[[.a.]-[.c.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.c.]-a]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[c-[.a.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[[.c.]-[.a.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.c.]-a]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[c-[.a.]]"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[[.c.]-[.a.]]"), 0);
// check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("??"), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/["), U_CONSTANT_TO_PARAM("\\/["), 0);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("*"), FNM_PATHNAME|FNM_PERIOD);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/*"), FNM_PATHNAME|FNM_PERIOD);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/?"), FNM_PATHNAME|FNM_PERIOD);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/."), U_CONSTANT_TO_PARAM("/[!a-z]"), FNM_PATHNAME|FNM_PERIOD);
// check_match_false(pfn_match, U_CONSTANT_TO_PARAM("abbb/.x"), U_CONSTANT_TO_PARAM("a*b/*"), FNM_PATHNAME | FNM_PERIOD);

   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a]"), U_CONSTANT_TO_PARAM("[]a]]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("[]abc"), U_CONSTANT_TO_PARAM("[][]abc"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[!abc]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[][abc]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("x"), U_CONSTANT_TO_PARAM("[!abc]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[][abc]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[!a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("]]"), U_CONSTANT_TO_PARAM("[!a]]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[c-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("c"), U_CONSTANT_TO_PARAM("[c-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[!-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("[!a-c]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("."), U_CONSTANT_TO_PARAM("[!a-c]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("/.a"), U_CONSTANT_TO_PARAM("/[!a-z]a"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/$"), U_CONSTANT_TO_PARAM("\\/\\$"), FNM_NOESCAPE);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("\\/\\$"), U_CONSTANT_TO_PARAM("\\/\\$"), FNM_NOESCAPE);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("..asd"), U_CONSTANT_TO_PARAM(".[!a-z]*"), FNM_PERIOD);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM("*"), FNM_PERIOD);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM(".asd"), U_CONSTANT_TO_PARAM("?asd"), FNM_PERIOD);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("az"), U_CONSTANT_TO_PARAM("[a-]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("bz"), U_CONSTANT_TO_PARAM("[ab-]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("-z"), U_CONSTANT_TO_PARAM("[ab-]z"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("_"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[\\\\-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("_"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("-"), U_CONSTANT_TO_PARAM("[\\]-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a"), U_CONSTANT_TO_PARAM("[!\\\\-a]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("Z"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("["), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("\\"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0);
   check_match_true( pfn_match, U_CONSTANT_TO_PARAM("]"), U_CONSTANT_TO_PARAM("[Z-\\]]"), 0);

#ifndef __MINGW32__
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("?"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/"), U_CONSTANT_TO_PARAM("*"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("a/b"), U_CONSTANT_TO_PARAM("a?b"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/.a/.b"), U_CONSTANT_TO_PARAM("/*b"), FNM_PATHNAME);
   check_match_false(pfn_match, U_CONSTANT_TO_PARAM("/a./.b."), U_CONSTANT_TO_PARAM("/*/*"), FNM_PATHNAME|FNM_PERIOD);
#endif
}

static int compare_str(const void* str1, const void* str2) { return u_strnatcmp(*(const char**)str1, *(const char**)str2); }

#define U_TESTO_SEMPLICE "testosemplicetxt" /* no _.:?! */

int main(int argc, char* argv[])
{
   static char buf[4096];

   bool ok;
   int i, n;
   char buf8[9];
   const char* ptr;
   char* sargv[128];
   uint32_t x, path_len;
   const char* path_rel;
   char path[U_PATH_MAX + 1];

   const char* vec[] = {
      "libpng-1.0.10.tar.gz",
      "libpng-1.0.11.tar.gz",
      "libpng-1.0.12.tar.gz",
      "libpng-1.0.3.tar.gz",
      "libpng-1.0.5.tar.gz",
      "libpng-1.0.6-patch-a.txt.gz",
      "libpng-1.0.6-patch-b.txt.gz",
      "libpng-1.0.6-patch-c.txt.gz",
      "libpng-1.0.6.tar.gz",
      "libpng-1.0.7.tar.gz",
      "libpng-1.0.8.tar.gz",
      "libpng-1.0.9.tar.gz",
      "libpng-1.2.0.tar.gz" };

   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   u_pOR = U_NULLPTR;

   path_len = u_dosmatch_with_OR(U_CONSTANT_TO_PARAM("/tavarnelle.shtml"),
                                 U_CONSTANT_TO_PARAM("/admin*|/tavarnelle*"), 0);

   U_INTERNAL_PRINT("u_pOR(%u) = %.*s", path_len, path_len, u_pOR)

   U_INTERNAL_ASSERT_EQUALS(path_len, U_CONSTANT_SIZE("/tavarnelle*"))

   U_INTERNAL_ASSERT_EQUALS(memcmp(u_pOR, U_CONSTANT_TO_PARAM("/tavarnelle*")), 0)

   x = u_nextPowerOfTwo(2);

   U_INTERNAL_PRINT("x = %u", x)

   U_INTERNAL_ASSERT_EQUALS( x, 2 )

   x = u_nextPowerOfTwo(3);

   U_INTERNAL_PRINT("x = %u", x)

   U_INTERNAL_ASSERT_EQUALS( x, 4 )

   x = u_nextPowerOfTwo(4);

   U_INTERNAL_PRINT("x = %u", x)

   U_INTERNAL_ASSERT_EQUALS( x, 4 )

   x = u_nextPowerOfTwo(5);

   U_INTERNAL_PRINT("x = %u", x)

   U_INTERNAL_ASSERT_EQUALS( x, 8 )

   x = u_nextPowerOfTwo(123);

   U_INTERNAL_PRINT("x = %u", x)

   U_INTERNAL_ASSERT_EQUALS( x, 128 )

   x = u_nextPowerOfTwo(222);

   U_INTERNAL_PRINT("x = %u", x)

   U_INTERNAL_ASSERT_EQUALS( x, 256 )

   (void) strcpy(path, "../../pippo");

   ok = u_canonicalize_pathname(path, strlen(path)) == U_CONSTANT_SIZE( "../../pippo");

   U_INTERNAL_ASSERT( ok )
   U_INTERNAL_ASSERT_EQUALS( memcmp(path, U_CONSTANT_TO_PARAM("../../pippo")), 0 )

   (void) strcpy(path, "../../pippo/./../pluto");

   ok = u_canonicalize_pathname(path, strlen(path)) == U_CONSTANT_SIZE( "../../pluto");

   U_INTERNAL_ASSERT( ok )
   U_INTERNAL_ASSERT_EQUALS( memcmp(path, U_CONSTANT_TO_PARAM("../../pluto")), 0 )

   (void) strcpy(path, "././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././../../../../../../../../");

   ok = u_canonicalize_pathname(path, strlen(path)) == U_CONSTANT_SIZE("../../../../../../../..");

   U_INTERNAL_ASSERT( ok )
   U_INTERNAL_ASSERT_EQUALS( memcmp(path, U_CONSTANT_TO_PARAM("../../../../../../../..")), 0 )

   (void) strcpy(path, "/usr/src/ULib-1.4.2/tests/examples/benchmark/docroot././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././././../../../../../../../../");

   ok = u_canonicalize_pathname(path, strlen(path)) == U_CONSTANT_SIZE("/");

   U_INTERNAL_ASSERT( ok )
   U_INTERNAL_ASSERT_EQUALS( memcmp(path, U_CONSTANT_TO_PARAM("/")), 0 )

   U_INTERNAL_ASSERT( u__isdigit('1') )
   U_INTERNAL_ASSERT( u__ispunct('.') )
   U_INTERNAL_ASSERT( u__isprint('1') )
   U_INTERNAL_ASSERT( u__isprint('.') )
   U_INTERNAL_ASSERT( u_isText((const unsigned char*)U_CONSTANT_TO_PARAM("binary:  "))  == false )
   U_INTERNAL_ASSERT( u_isBinary((const unsigned char*)U_CONSTANT_TO_PARAM("binary: ‚g df d")) )
   U_INTERNAL_ASSERT( u_isWhiteSpace(U_CONSTANT_TO_PARAM("           \n\t\r")) )
   U_INTERNAL_ASSERT( u_isFileName(U_CONSTANT_TO_PARAM("/Z/x,validate")) )
   U_INTERNAL_ASSERT( u_isBase64(U_CONSTANT_TO_PARAM("gXWUj7VekBdkycg3Z9kXuglV9plUl2cs4XkNLSDhe5VHRgE03e63VypMChCWDGI=")) )

   for (i = 0; i < 0xff; ++i)
      {
      ok = ( (bool)isalnum(i)  == u__isalnum((unsigned char)i) );

      if (ok == false) U_WARNING("isalnum(%x) = %b u__isalnum(%x) = %b", i, isalnum(i), i, u__isalnum((unsigned char)i));

      ok = ( (bool)isalpha(i)  == u__isalpha((unsigned char)i) );

      if (ok == false) U_WARNING("isalpha(%x) = %b u__isalpha(%x) = %b", i, isalpha(i), i, u__isalpha((unsigned char)i));

      ok = ( (bool)iscntrl(i)  == u__iscntrl((unsigned char)i) );

      if (ok == false) U_WARNING("iscntrl(%x) = %b u__iscntrl(%x) = %b", i, iscntrl(i), i, u__iscntrl((unsigned char)i));

      ok = ( (bool)isdigit(i)  == u__isdigit((unsigned char)i) );

      if (ok == false) U_WARNING("isdigit(%x) = %b u__isdigit(%x) = %b", i, isdigit(i), i, u__isdigit((unsigned char)i));

      ok = ( (bool)isdigit(i)  == u__isdigitw0((unsigned char)i) );

      if (ok == false) U_WARNING("isdigit(%x) = %b u__isdigitw0(%x) = %b", i, isdigit(i), i, u__isdigitw0((unsigned char)i));

      ok = ( (bool)isgraph(i)  == u__isgraph((unsigned char)i) );

      if (ok == false) U_WARNING("isgraph(%x) = %b u__isgraph(%x) = %b", i, isgraph(i), i, u__isgraph((unsigned char)i));

      ok = ( (bool)islower(i)  == u__islower((unsigned char)i) );

      if (ok == false) U_WARNING("islower(%x) = %b u__islower(%x) = %b", i, islower(i), i, u__islower((unsigned char)i));

      ok = ( (bool)isprint(i)  == u__isprint((unsigned char)i) );

      if (ok == false) U_WARNING("isprint(%x) = %b u__isprint(%x) = %b", i, isprint(i), i, u__isprint((unsigned char)i));

      ok = ( (bool)ispunct(i)  == u__ispunct((unsigned char)i) );

      if (ok == false) U_WARNING("ispunct(%x) = %b u__ispunct(%x) = %b", i, ispunct(i), i, u__ispunct((unsigned char)i));

      ok = ( (bool)isspace(i)  == u__isspace((unsigned char)i) );

      if (ok == false) U_WARNING("isspace(%x) = %b u__isspace(%x) = %b", i, isspace(i), i, u__isspace((unsigned char)i));

      ok = ( (bool)isupper(i)  == u__isupper((unsigned char)i) );

      if (ok == false) U_WARNING("isupper(%x) = %b u__isupper(%x) = %b", i, isupper(i), i, u__isupper((unsigned char)i));

      ok = ( (bool)isblank(i)  == u__isblank((unsigned char)i) );

      if (ok == false) U_WARNING("isblank(%x) = %b u__isblank(%x) = %b", i, isblank(i), i, u__isblank((unsigned char)i));

      ok = ( (bool)isxdigit(i) == u__isxdigit((unsigned char)i) );

      if (ok == false) U_WARNING("isxdigit(%x) = %b u__isxdigit(%x) = %b", i, isxdigit(i), i, u__isxdigit((unsigned char)i));
      }

   /* for (i = 0; i < U_CONSTANT_SIZE(U_TESTO_SEMPLICE); ++i) U_INTERNAL_PRINT("u__istext(%C) = %d", U_TESTO_SEMPLICE[i], u__istext(U_TESTO_SEMPLICE[i])) */

   U_INTERNAL_ASSERT( u_isText((const unsigned char* restrict)U_CONSTANT_TO_PARAM(U_TESTO_SEMPLICE)) )

   strcpy(buf, "HOME=/home/stefano\n\"RSIGN_CMD=./rsa_priv_enc.sh -c rsa_priv_enc.cfg -k INKEY < DIGEST\"\nLOG_FILE=/tmp/log");

   n = strlen(buf);

   U_VAR_UNUSED(n)

   U_INTERNAL_ASSERT( u_split(buf, n, sargv, 0) == 3 )
   U_INTERNAL_ASSERT( sargv[3] == 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[0], U_CONSTANT_TO_PARAM("HOME=/home/stefano")), 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[1], U_CONSTANT_TO_PARAM("RSIGN_CMD=./rsa_priv_enc.sh -c rsa_priv_enc.cfg -k INKEY < DIGEST")), 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[2], U_CONSTANT_TO_PARAM("LOG_FILE=/tmp/log")), 0 )

   (void) strcpy(buf, "first second third \
               \"four five six\" \
               'uno due tre'");

   U_INTERNAL_ASSERT( u_split(buf, strlen(buf), sargv, 0) == 5 )
   U_INTERNAL_ASSERT( sargv[5] == 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[0], U_CONSTANT_TO_PARAM("first")), 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[1], U_CONSTANT_TO_PARAM("second")), 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[2], U_CONSTANT_TO_PARAM("third")), 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[3], U_CONSTANT_TO_PARAM("four five six")), 0 )
   U_INTERNAL_ASSERT_EQUALS( memcmp(sargv[4], U_CONSTANT_TO_PARAM("uno due tre")), 0 )

   if (u_pathfind(path, 0, 0, "gzip", R_OK | X_OK))
      {
      U_INTERNAL_PRINT("path = %s", path)

      U_INTERNAL_ASSERT(strstr(path, "gzip") != 0)
      }

   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("a"))          == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("A"))          == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("1"))          )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("234"))        )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("A34"))        == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("1A3"))        == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("1A3d3###"))   == false )
   U_INTERNAL_ASSERT( u_isNumber(U_CONSTANT_TO_PARAM("0123456789")) )

   ptr = "+1234";
   U_INTERNAL_ASSERT_EQUALS( u_strtol( ptr,    ptr + U_CONSTANT_SIZE("+1234")), 1234 )
   U_INTERNAL_ASSERT_EQUALS( u_strtoll(ptr,    ptr + U_CONSTANT_SIZE("+1234")), 1234 )
   U_INTERNAL_ASSERT_EQUALS( u_strtoul( ptr+1, ptr + U_CONSTANT_SIZE("+1234")), 1234 )
   U_INTERNAL_ASSERT_EQUALS( u_strtoull(ptr+1, ptr + U_CONSTANT_SIZE("+1234")), 1234 )

   ptr = "-1234";
   U_INTERNAL_ASSERT_EQUALS( u_strtol( ptr,    ptr + U_CONSTANT_SIZE("-1234")), -1234 )
   U_INTERNAL_ASSERT_EQUALS( u_strtoll(ptr,    ptr + U_CONSTANT_SIZE("-1234")), -1234 )
   U_INTERNAL_ASSERT_EQUALS( u_strtoul( ptr+1, ptr + U_CONSTANT_SIZE("-1234")),  1234 )
   U_INTERNAL_ASSERT_EQUALS( u_strtoull(ptr+1, ptr + U_CONSTANT_SIZE("-1234")),  1234 )

   U_INTERNAL_ASSERT( u_isIPv4Addr("127.0.0.1/8", 9)                       == true )
   U_INTERNAL_ASSERT( u_isIPv4Addr(U_CONSTANT_TO_PARAM("8.0.0.1"))         == true )
   U_INTERNAL_ASSERT( u_isIPv4Addr(U_CONSTANT_TO_PARAM("127.0.0.1"))       == true )
   U_INTERNAL_ASSERT( u_isIPv4Addr(U_CONSTANT_TO_PARAM("151.11.47."))      == false )
   U_INTERNAL_ASSERT( u_isIPv4Addr(U_CONSTANT_TO_PARAM(".11.47.255"))      == false )
   U_INTERNAL_ASSERT( u_isIPv4Addr(U_CONSTANT_TO_PARAM("255.255.255.255")) )

   U_INTERNAL_ASSERT( u_isIPv6Addr(U_CONSTANT_TO_PARAM("2001:db8:85a3::8a2e:370:7334"))            )
   U_INTERNAL_ASSERT( u_isIPv6Addr(U_CONSTANT_TO_PARAM("2001:0db8:85a3:0000:0000:8a2e:0370:7334")) )

   U_INTERNAL_ASSERT( u_isMacAddr(U_CONSTANT_TO_PARAM("00:16:ec:f0:e4:3b")) )
   U_INTERNAL_ASSERT( u_isMacAddr(U_CONSTANT_TO_PARAM("00:16:ec:fk:e4:3b")) == false )

   buf8[8] = '\0';

   u_int2hex(buf8, 123456789);
   U_INTERNAL_ASSERT_EQUALS( memcmp(buf8, U_CONSTANT_TO_PARAM("075BCD15")), 0 )
   U_INTERNAL_ASSERT( u_hex2int(buf8, 8) == 123456789 )

   u_int2hex(buf8,  23456789);
   U_INTERNAL_ASSERT_EQUALS( memcmp(buf8, U_CONSTANT_TO_PARAM("0165EC15")), 0 )
   U_INTERNAL_ASSERT( u_hex2int(buf8, 8) == 23456789 )

   check_match1(u_fnmatch);
   check_match1(u_dosmatch);
   check_match1(u_dosmatch_ext);
   check_match1((bPFpcupcud)u_dosmatch_with_OR);
   check_match1((bPFpcupcud)u_dosmatch_ext_with_OR);

   check_match2((bPFpcupcud)u_dosmatch_with_OR);
   check_match2((bPFpcupcud)u_dosmatch_ext_with_OR);

   check_match3((bPFpcupcud)u_dosmatch_with_OR);
   check_match3((bPFpcupcud)u_dosmatch_ext_with_OR);

   check_match4(u_fnmatch);
   check_match4(u_dosmatch_ext);

   check_match5(u_fnmatch);

   qsort(vec, 13, sizeof(const char*), compare_str);

   // U_INTERNAL_ASSERT( strcmp(vec[0],  "libpng-1.0.3.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[1],  "libpng-1.0.5.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[2],  "libpng-1.0.6-patch-a.txt.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[3],  "libpng-1.0.6-patch-b.txt.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[4],  "libpng-1.0.6-patch-c.txt.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[5],  "libpng-1.0.6.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[6],  "libpng-1.0.7.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[7],  "libpng-1.0.8.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[8],  "libpng-1.0.9.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[9],  "libpng-1.0.10.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[10], "libpng-1.0.11.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[11], "libpng-1.0.12.tar.gz") == 0 )
   // U_INTERNAL_ASSERT( strcmp(vec[12], "libpng-1.2.0.tar.gz") == 0 )

   for (i = 0; i < 13; ++i) puts(vec[i]);

   fflush(stdout);

   /* PATH RELATIV */

   u_cwd_len = 4;
   (void) strcpy(u_cwd, "/usr");

   path_len = U_CONSTANT_SIZE( "/usr///local/src/pippo.txt");
   path_rel = u_getPathRelativ("/usr///local/src/pippo.txt", &path_len);

   U_VAR_UNUSED(path_rel)

   U_INTERNAL_ASSERT_EQUALS( memcmp(path_rel, U_CONSTANT_TO_PARAM("local/src/pippo.txt")), 0 )

   path_rel = u_getPathRelativ(".///usr/local/src/pippo.txt", &path_len);

   U_VAR_UNUSED(path_rel)

   U_INTERNAL_ASSERT_EQUALS( memcmp(path_rel, U_CONSTANT_TO_PARAM("usr/local/src/pippo.txt")), 0 )

   u_cwd_len = 1;
   (void) strcpy(u_cwd, "/");

   path_len = U_CONSTANT_SIZE( "/usr/local/src/pippo.txt");
   path_rel = u_getPathRelativ("/usr/local/src/pippo.txt", &path_len);

   U_VAR_UNUSED(path_rel)

   U_INTERNAL_ASSERT_EQUALS( memcmp(path_rel, U_CONSTANT_TO_PARAM("usr/local/src/pippo.txt")), 0 )

   U_VAR_UNUSED(sargv)

   return 0;
}
