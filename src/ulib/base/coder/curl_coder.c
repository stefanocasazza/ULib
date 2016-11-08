/* ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    curl_coder.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/coder/url.h>

/**
 * Converts a Unicode string into the MIME x-www-form-urlencoded format.
 * ---------------------------------------------------------------------------------------------------------------
 *  To convert a String, each Unicode character is examined in turn:
 *  - The ASCII characters (alpha: 'a' through 'z', 'A' through 'Z', '0' through '9'), and xalpha remain the same.
 *  - The space character ' '(U+20) is converted into a plus sign '+'.
 *  - All other characters are converted into their UTF-8 equivalent and the subsequent bytes are encoded
 *    as the 3-byte string "%xy", where xy is the two-digit hexadecimal representation of the byte.
 * ---------------------------------------------------------------------------------------------------------------
 * BNF for specific URL schemes
 *
 * This is a BNF-like description of the Uniform Resource Locator syntax. A vertical  line "|" indicates alternatives, and [brackets]
 * indicate optional parts.  Spaces are represented by the word "space", and the vertical line character by "vline". Single letters stand
 * for single letters. All words of more than one letter below are entities described somewhere in this description. The current IETF URI
 * working group preference is for the prefixedurl production. (Nov 1993. July 93: url). The "national" and "punctuation" characters do not
 * appear in any productions and therefore may not appear in URLs. The "afsaddress" is left in as historical note, but is not a url production
 *
 * ialpha      alpha [ xalphas ]
 * scheme      ialpha
 * prefixedurl url : url
 * url         httpaddress   | ftpaddress    | newsaddress | nntpaddress   | cidaddress |
 *             telnetaddress | gopheraddress | waisaddress | mailtoaddress | midaddress | prosperoaddress
 *
 * void
 * digit   0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
 * hex     a | b | c | d | e | f | A | B | C | D | E | F | digit
 * alpha   a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z |
 *         A | B | C | D | E | F | G | H | I | J | K | L | M | N | O | P | Q | R | S | T | U | V | W | X | Y | Z
 * safe    $ | - | _ | @ | . | & | + | -
 *
 * extra        ! | * | " | ' | ( | ) | ,
 * escape       % hex hex
 * xalpha       alpha | digit | safe | extra | escape
 * xpalpha      xalpha | +
 * xalphas      xalpha [ xalphas ]
 * xpalphas     xpalpha [ xpalphas ]
 * path         void | xpalphas [ / path ]
 * search       xalphas [ + search ]
 * port         digits
 * hostport     host [ : port ]
 * httpaddress  http://hostport[/path][?search]
 *
 * ftpaddress    ftp://login/path[ftptype]
 * telnetaddress telnet://login
 * mailtoaddress mailto:xalphas@hostname
 * gopheraddress gopher://hostport[/gtype[gcommand]]
 * afsaddress    afs://cellname/path
 * newsaddress   news:groupart
 * nntpaddress   nntp:group/digits
 * midaddress    mid:addr-spec
 * cidaddress    cid:content-identifier
 * waisaddress   waisindex | waisdoc
 * waisindex     wais://hostport/database[?search]
 * waisdoc       wais://hostport/database/wtype/wpath
 * prosperolink  prospero://hostport/hsoname[%00version[attributes]]
 *
 * wpath           digits = path ; [ wpath ]
 * groupart        * | group | article
 * group           ialpha [ . group ]
 * article         xalphas @ host
 * database        xalphas
 * wtype           xalphas
 * prosperoaddress prosperolink
 *
 * hsoname      path
 * version      digits
 * attributes   attribute [ attributes ]
 * attribute    alphanums
 *
 * login        [user[:password]@]hostport
 * host         hostname | hostnumber
 * ftptype      A formcode | E formcode | I | L digits
 * formcode     N | T | C
 * cellname     hostname
 * hostname     ialpha [ . hostname ]
 * hostnumber   digits . digits . digits . digits
 * gcommand     path
 * user         alphanum2 [ user ]
 * password     alphanum2 [ password ]
 * fragmentid   xalphas
 * gtype        xalpha
 * alphanum2    alpha | digit | - | _ | . | +
 * reserved     = | ; | / | # | ? | : | space
 * national     { | } | vline | [ | ] | \ | ^ | ~
 * punctuation  < | >
 * digits       digit [ digits ]
 * alphanum     alpha | digit
 * alphanums    alphanum [ alphanums ]
 * ----------------------------------------------------------------------------------------------------------
 * RFC2231 (which in HTTP is mostly used for giving UTF8-encoded filenames in the Content-Disposition header)
 * ----------------------------------------------------------------------------------------------------------
 * extra_enc_chars -> " *'%()<>@,;:\\\"/[]?="
 * ----------------------------------------------------------------------------------------------------------
 * RFC3986
 * ----------------------------------------------------------------------------------------------------------
 * #define U_URI_UNRESERVED  0 // ALPHA (%41-%5A and %61-%7A) DIGIT (%30-%39) '-' '.' '_' '~'
 * #define U_URI_PCT_ENCODED 1
 * #define U_URI_GEN_DELIMS  2 // ':' '/' '?'  '#' '[' ']' '@'
 * #define U_URI_SUB_DELIMS  4 // '!' '$' '&' '\'' '(' ')' '*' '+' ',' ';' '='
 * -----------------------------------------------------------------------------------------------------------
 */ 

uint32_t u_url_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
   uint32_t i;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_url_encode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   for (i = 0; i < len; ++i)
      {
      unsigned char ch = input[i];

      if (u__is2urlenc(ch) == false) *r++ = ch;
      else
         {
         if (ch == ' ') *r++ = '+';
         else
            {
            *r++ = '%';

            u_put_unalignedp16(r, U_MULTICHAR_CONSTANT16("0123456789ABCDEF"[(ch >> 4) & 0x0F],
                                                         "0123456789ABCDEF"[ ch      & 0x0F]));

            r += 2;
            }
         }
      }

   *r = 0;

   return (r - result);
}

uint32_t u_url_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
   uint32_t i;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_url_decode(%.*s,%u,%p,%lu)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   for (i = 0; i < len; ++i)
      {
      unsigned char ch = ((unsigned char* restrict)input)[i];

           if (ch == '+') *r++ = ' ';
      else if (ch != '%') *r++ = ch;
      else
         {
         *r++ = ((u__hexc2int(((unsigned char* restrict)input)[i+1]) & 0x0F) << 4) |
                 (u__hexc2int(((unsigned char* restrict)input)[i+2]) & 0x0F);

         i += 2;
         }
      }

   *r = 0;

   return (r - result);
}
