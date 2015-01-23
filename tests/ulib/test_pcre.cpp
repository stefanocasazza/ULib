// test_pcre.cpp

#include <ulib/pcre/pcre.h>

#ifdef __MINGW32__
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

static void ex()
{
   U_TRACE(5, "ex()")

   /* this will generate only one substring, "This" */

   UPCRE ex(U_STRING_FROM_CONSTANT("([a-z]+)"), "i");

   U_ASSERT(ex.search(U_STRING_FROM_CONSTANT("This is a test.")) == true) /* check if the expression matched */

   U_INTERNAL_DUMP("num substrings = %d", ex.matches())

   U_ASSERT(ex.matches() == 1)
}

static void regex(const UString& expression, const UString& stuff)
{
   U_TRACE(5, "regex(%.*S,%.*S)", U_STRING_TO_TRACE(expression), U_STRING_TO_TRACE(stuff))

   UPCRE reg(expression, 0); /* UPCRE object */

   bool result = reg.search(stuff);

   U_INTERNAL_ASSERT(result == true) /* check if the expression matched */

   U_INTERNAL_DUMP("num substrings = %d", reg.matches())

   int start, end, pos;

   for (pos = 0; pos < reg.matches(); ++pos) /* iterate over the matched sub strings */
      {
      end   = reg.getMatchEnd(pos);
      start = reg.getMatchStart(pos);

      /* print out the start/end offset of the current substring within the searched string(stuff) */

      U_DUMP("substrings[%3d] = %S", pos, reg[pos].c_str()) // also possible: reg.getMatch(pos);
      U_DUMP("(%3d, %3d)      = %S", start, end, stuff.substr(start, end - start + 1).c_str());
      }
}

static void regex()
{
   U_TRACE(5, "regex()")

   UString expression = U_STRING_FROM_CONSTANT("([a-z]*) ([0-9]+)"), /* define a string with a regular expression */
           stuff      = U_STRING_FROM_CONSTANT("hallo 11 robert");    /* this is the string in which we want to search */

   UPCRE reg(expression, "i"); /* UPCRE object, search case-insensitive ("i") */

   U_ASSERT(reg.search(stuff) == true) /* check if the expression matched */

   U_INTERNAL_DUMP("num substrings = %d", reg.matches())

   U_ASSERT(reg.matches() > 0) /* check if the expression generated any substrings */

   int start, end, pos;

   for (pos = 0; pos < reg.matches(); ++pos) /* iterate over the matched sub strings */
      {
      end   = reg.getMatchEnd(pos);
      start = reg.getMatchStart(pos);

      /* print out the start/end offset of the current substring within the searched string(stuff) */

      U_DUMP("substrings[%3d] = %S", pos, reg[pos].c_str()) // also possible: reg.getMatch(pos);
      U_DUMP("(%3d, %3d)      = %S", start, end, stuff.substr(start, end - start + 1).c_str());
      }

   U_ASSERT(reg[0] == U_STRING_FROM_CONSTANT("hallo"))
   U_ASSERT(reg[1] == U_STRING_FROM_CONSTANT("11"))
}

static void split() // Sample of split() usage
{
   U_TRACE(5, "split()")

   UString sp_orig = U_STRING_FROM_CONSTANT("was21willst2387461du3alter!"),
        delimiter  = U_STRING_FROM_CONSTANT("[0-9]+"); // define a regex for digits (character class)

   UPCRE S(delimiter, "g"); // new UPCRE object, match globally ("g" flag)

   UVector<UString> splitted;
   unsigned n = S.split(splitted, sp_orig); // split "was21willst2387461du3alter!" by digits

   for (unsigned i = 0; i < n; ++i) // iterate over the resulting list
      {
      U_DUMP("splitted[%3d] = %S", i, splitted[i].c_str())
      }

   U_ASSERT(splitted[0] == U_STRING_FROM_CONSTANT("was"))
   U_ASSERT(splitted[1] == U_STRING_FROM_CONSTANT("willst"))
   U_ASSERT(splitted[2] == U_STRING_FROM_CONSTANT("du"))
   U_ASSERT(splitted[3] == U_STRING_FROM_CONSTANT("alter!"))
}

static void replace() // Sample of replace() usage
{
   U_TRACE(5, "replace()")

   UString orig = U_STRING_FROM_CONSTANT("Hans ist 22 Jahre alt. Er ist 8 Jahre älter als Fred.");

   UPCRE p(U_STRING_FROM_CONSTANT("([0-9]+)"), 0); // define a regex for digits (character class)

   UString n = p.replace(orig, U_STRING_FROM_CONSTANT("zweiundzwanzig($1)")); // replace the 1st occurence of [0-9]+ with "zweiundzwanzig"

   U_INTERNAL_DUMP("n = %.*S", U_STRING_TO_TRACE(n))

   U_ASSERT(n == U_STRING_FROM_CONSTANT("Hans ist zweiundzwanzig(22) Jahre alt. Er ist 8 Jahre älter als Fred."))
}

static void normalize() // another sample to check if normalizing using replace() works
{
   U_TRACE(5, "normalize()")

   UString orig = U_STRING_FROM_CONSTANT("Heute   ist ein  schoener  Tag        gell?");

   UPCRE reg(U_STRING_FROM_CONSTANT("[\\s]+"), "gs"); // create regex for normalizing whitespace

   UString n = reg.replace(orig, U_STRING_FROM_CONSTANT(" ")); // do the normalizing process

   U_INTERNAL_DUMP("n = %.*S", U_STRING_TO_TRACE(n))

   U_ASSERT(n == U_STRING_FROM_CONSTANT("Heute ist ein schoener Tag gell?"))
}

static void multisearch()
{
   U_TRACE(5, "multisearch()")

   size_t i = 0, pos = 0;
   UPCRE reg(U_STRING_FROM_CONSTANT("([^\\n]+\\n)"), 0);
   UString str = U_STRING_FROM_CONSTANT("\nline1\nline2\nline3\n"), vec[3];

   while (pos <= str.length() && reg.search(str, pos))
      {
      pos    = reg.getMatchEnd(0);
      vec[i] = reg[0];

      U_DUMP("pos: %2d match (%d): %.*S", pos, i, U_STRING_TO_TRACE(vec[i]))

      vec[i++].duplicate();
      }

   U_DUMP("vec[0] = %.*S", U_STRING_TO_TRACE(vec[0]))
   U_DUMP("vec[1] = %.*S", U_STRING_TO_TRACE(vec[1]))
   U_DUMP("vec[2] = %.*S", U_STRING_TO_TRACE(vec[2]))

   U_ASSERT(vec[0] == U_STRING_FROM_CONSTANT("line1\n"))
   U_ASSERT(vec[1] == U_STRING_FROM_CONSTANT("line2\n"))
   U_ASSERT(vec[2] == U_STRING_FROM_CONSTANT("line3\n"))
}

static void replace_multi() // Sample of replace() usage with multiple substrings
{
   U_TRACE(5, "replace_multi()")

   UString orig = U_STRING_FROM_CONSTANT(" 08:23 ");

   UPCRE reg(U_STRING_FROM_CONSTANT(" ([0-9]+)(:)([0-9]+) "), "sig"); // create regex which, if it matches, creates 3 substrings

   // remove $2 (":") * re-use $1 ("08") and $3 ("23") in the replace string
   UString n = reg.replace(orig, U_STRING_FROM_CONSTANT("$1 Stunden und $3 Minuten"));

   U_INTERNAL_DUMP("n = %.*S", U_STRING_TO_TRACE(n))

   U_ASSERT(n == U_STRING_FROM_CONSTANT("08 Stunden und 23 Minuten"))
}

static UPCRE* reg;
static string flags;
static map<string, vector<string> > _hash;

static void set(std::string& str)
{
   U_TRACE(5, "set(%p)", &str)

   if (reg) delete reg;

   UString expression(str.c_str());

   reg = new UPCRE(expression, flags.c_str());
}

static bool search(UPCRE& _pcre, std::string& str)
{
   U_TRACE(5, "search(%p,%p)", &_pcre, &str)

   UString stuff(str.c_str());

   return _pcre.search(stuff, 0, 0);
}

static bool read_data(const char* file)
{
   U_TRACE(5, "read_data(%S)", file)

   ifstream data(file);

   if (!data) return false;

   char zeichen;
   string line, regex;
   bool is_regex = false;
   vector<string> content;
   UPCRE find_end(U_STRING_FROM_CONSTANT("\\/[gimsxADEGIMNSUX8L\\+]{0,2}$"), 0);

   while (data)
      {
      data.get(zeichen);

      if (zeichen == '\n')
         {
         if (line[0] == '/')
            {
            regex    = line;
            is_regex = (search(find_end, line) == false);
            }
         else if (is_regex)
            {
            regex   += "\n" + line;
            is_regex = (search(find_end, line) == false);
            }
         else if (line[0] == ' ')
            {
            line.replace(0, 4, ""); // remove leading whitespaces

            content.push_back(line);
            }
         else if (line.empty())
            {
            _hash[regex] = content;

            content.clear();
            }

         line = "";
         }
      else
         {
         line += zeichen;
         }
      }

   data.close();

   return true;
}

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   /* define a string with a regular expression */
   UString expression = U_STRING_FROM_CONSTANT("^/lms/doceboCore(/|/index.php)?$"),
   /* this is the string in which we want to search */
           stuff      = U_STRING_FROM_CONSTANT("/lms/doceboCore/index.php");

   regex(expression, stuff);

   stuff = U_STRING_FROM_CONSTANT("/lms/doceboCore/");
   regex(expression, stuff);

   stuff = U_STRING_FROM_CONSTANT("/lms/doceboCore");
   regex(expression, stuff);

   expression = U_STRING_FROM_CONSTANT("^/secure/?$");

   stuff      = U_STRING_FROM_CONSTANT("/secure");
   regex(expression, stuff);

   stuff      = U_STRING_FROM_CONSTANT("/secure/");
   regex(expression, stuff);

   expression = U_STRING_FROM_CONSTANT("^/WAYF/?\\?shire=http*");

   stuff      = U_STRING_FROM_CONSTANT("/WAYF?shire=http%3A%2F%2Flocalhost%2FShibboleth.sso%2FSAML%2FPOST&time=1196764890&target=cookie&providerId=http%3A%2F%2Flocalhost%2Fsp");

   regex(expression, stuff);

   stuff      = U_STRING_FROM_CONSTANT("/WAYF/?shire=http%3A%2F%2Flocalhost%2FShibboleth.sso%2FSAML%2FPOST&time=1196764890&target=cookie&providerId=http%3A%2F%2Flocalhost%2Fsp");
   regex(expression, stuff);

   expression = U_STRING_FROM_CONSTANT("^/SWITCHaai/images/.*\\.gif$");

   stuff      = U_STRING_FROM_CONSTANT("/SWITCHaai/images/toplevel.gif");
   regex(expression, stuff);

   /*
   U_ASSERT( UPCRE::isValidURL(U_STRING_FROM_CONSTANT("http://immike.net/blog/2007/06/21/extreme-regex-foo-what-you-need-to-know-to-become-a-regular-expression-pro/")) == true )
   */

   U_ASSERT( UPCRE::validateUsername(U_STRING_FROM_CONSTANT("%! mike_84&")) == false )

#define XML_MSG \
"<REQUEST sid=\"sid1\" version=\"1\">" \
"<EXPORT-POLICYLABEL name=\"openvpn-server\"/>" \
"</REQUEST>" \
"<REQUEST sid=\"sid2\" version=\"1\">" \
"  <EXPORT-POLICYLABEL name=\"openvpn-default\"/>" \
"</REQUEST>"

   UVector<UString> vec;

   U_ASSERT( UPCRE::getTag(vec, U_STRING_FROM_CONSTANT(XML_MSG), "name", "openvpn-default", "EXPORT-POLICYLABEL") == 1 )

#define HTTP_MSG \
   "HTTP/1.1 301 Moved Permanently\r\n" \
   "Date: Fri, 10 Nov 2006 16:59:55 GMT\r\n" \
   "Server: Apache/2.0.54 (Debian GNU/Linux) PHP/4.3.10-16 mod_ssl/2.0.54 OpenSSL/0.9.7e\r\n" \
   "Location: http://laptop.unirel.intranet/webmail/\r\n" \
   "Content-Length: 388\r\n" \
   "Keep-Alive: timeout=15, max=100\r\n" \
   "Connection: Keep-Alive\r\n" \
   "Content-Type: text/html; charset=iso-8859-1\r\n" \
   "\r\n" \
   "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">" \
   "<html><head>" \
   "<title>301 Moved Permanently</title>" \
   "</head><body>" \
   "<h1>Moved Permanently</h1>" \
   "<p>The document has moved <a href=\"http://laptop.unirel.intranet/webmail/\">here</a>.</p>" \
   "<hr>" \
   "<address>Apache/2.0.54 (Debian GNU/Linux) PHP/4.3.10-16 mod_ssl/2.0.54 OpenSSL/0.9.7e Server at laptop.unirel.intranet Port 80</address>" \
   "</body></html>"

   expression = U_STRING_FROM_CONSTANT("(Location: http:)"),
   stuff      = U_STRING_FROM_CONSTANT(HTTP_MSG);

   regex(expression, stuff);

   ex();
   regex();
   split();
   replace();
   normalize();
   multisearch();
   replace_multi();

   if (read_data(argv[1]))
      {
      typedef vector<string>::iterator vec_iter;
      typedef map<string, vector<string> >::iterator map_iter;

      string _expression;

      for (map_iter mp = _hash.begin(); mp != _hash.end(); ++mp)
         {
         string regex           = mp->first;
         vector<string> content = mp->second;
         unsigned pos           = regex.find_last_of("/", string::npos);

         if (pos == regex.size())
            {
            flags      = "";
            _expression = regex.substr(1, regex.size() - 1);
            }
         else
            {
            flags      = regex.substr(pos + 1, regex.size());
            _expression = regex.substr(1, pos - 1);
            }

         U_INTERNAL_DUMP("_expression = %S flags = %S", _expression.c_str(), flags.c_str())

         cout << "/" << _expression << "/" << flags << endl;

         set(_expression);

         for (vec_iter vp = content.begin(); vp != content.end(); ++vp)
            {
            string data = *vp;

            if (search(*reg, data)) cout << "   1:   ";
            else                    cout << "   0:   ";

            cout << data << endl;
            }

         cout << endl;
         }
      }

   if (reg) delete reg;
}
