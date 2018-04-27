// HttpSetCookie.cpp

#include <HttpSetCookie.h>

/*
The header sends a cookie to the browser to maintain the client-side state.
The syntax for the Set-Cookie response header is

   set-cookie    =  "Set-Cookie2:" cookies
   cookies       =  1#cookie
   cookie        =  NAME "=" VALUE *(";" set-cookie-av)
   NAME          =  attr
   VALUE         =  value
   set-cookie-av =  "Comment" "=" value
                 |  "CommentURL" "=" <"> http_URL <">
                 |  "Discard"
                 |  "Domain" "=" value
                 |  "Max-Age" "=" value
                 |  "Path" "=" value
                 |  "Port" [ "=" <"> portlist <"> ]
                 |  "Secure"
                 |  "Version" "=" 1*DIGIT
   portlist      =  1#portnum
   portnum       =  1*DIGIT

For example:
  Set-Cookie: Part_Number="Riding_Rocket_0023"; Version="1";
           Path="/acme/ammo"
*/

HttpSetCookie::HttpSetCookie(const char* name_, unsigned name_len, const char* value_, unsigned value_len)
{
   U_TRACE_CTOR(5, HttpSetCookie, "%.*S,%u,%.*S,%u", name_len, name_, name_len, value_len, value_, value_len)

   U_INTERNAL_ASSERT(memcmp(name_, U_CONSTANT_TO_PARAM("Set-Cookie")) == 0)

   version2 = (name_[sizeof("Set-Cookie")-1] == '2');

   name.assign(name_, name_len);
   buffer.assign(value_, value_len);

   (void) U_VEC_SPLIT(vec, buffer, "=;, \r\n");

   UString tmp;
   unsigned i = 0;

   while (i < vec.size())
      {
      if (vec[i] == U_STRING_FROM_CONSTANT("Secure") ||
          vec[i] == U_STRING_FROM_CONSTANT("Discard"))
         {
         ++i;

         U_VEC_INSERT(vec, i, tmp);
         }

      ++i;
      }

   is_path = is_domain = is_max_age = is_comment = is_comment_url = is_port = is_version = is_secure = is_discard = false;
}

unsigned HttpSetCookie::count(const UString& name_)
{
   U_TRACE(5, "HttpSetCookie::count(%.*S)", U_STRING_TO_TRACE(name_))

   unsigned i, j;

   for (i = j = 0; i < vec.size(); i += 2)
      {
      if (name_ == vec[i])
         {
         ++j;
         }
      }

   U_RETURN(j);
}

void HttpSetCookie::add(const UString& name_, const UString& value_)
{
   U_TRACE(5, "HttpSetCookie::add(%.*S,%.*S)", U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_))

   vec.push_back(name_);
   vec.push_back(value_);
}

bool HttpSetCookie::check(unsigned i)
{
   U_TRACE(5, "HttpSetCookie::check(%u)", i)

   is_path        = (vec[i] == U_STRING_FROM_CONSTANT("Path"));
   is_domain      = (vec[i] == U_STRING_FROM_CONSTANT("Domain"));
   is_max_age     = (vec[i] == U_STRING_FROM_CONSTANT("Max-Age"));
   is_comment     = (vec[i] == U_STRING_FROM_CONSTANT("Comment"));
   is_comment_url = (vec[i] == U_STRING_FROM_CONSTANT("CommentURL"));
   is_port        = (vec[i] == U_STRING_FROM_CONSTANT("Port"));
   is_version     = (vec[i] == U_STRING_FROM_CONSTANT("Version"));
   is_secure      = (vec[i] == U_STRING_FROM_CONSTANT("Secure"));
   is_discard     = (vec[i] == U_STRING_FROM_CONSTANT("Discard"));

   U_RETURN(is_path || is_domain || is_max_age || is_comment || is_version || is_secure || is_comment_url || is_port || is_discard);
}

#define SET_VALUE(n) \
      if      (is_path)        path        = vec[i+n]; \
      if      (is_port)        port        = vec[i+n]; \
      else if (is_domain)      domain      = vec[i+n]; \
      else if (is_max_age)     max_age     = vec[i+n]; \
      else if (is_comment)     comment     = vec[i+n]; \
      else if (is_comment_url) comment_url = vec[i+n]; \
      else if (is_version)     version     = vec[i+n]; \
      else if (is_secure)      secure      = true; \
      else                     discard     = true;

bool HttpSetCookie::find(const UString& name_, UString& value_, UString& path, UString& domain,
                         UString& max_age, UString& comment, UString& comment_url, UString& port,
                         UString& version, bool& secure, bool& discard, unsigned index)
{
   U_TRACE(5, "HttpSetCookie::find(%.*S,%.*S,%.*S,%.*S,%d)",
            U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_), U_STRING_TO_TRACE(path), U_STRING_TO_TRACE(domain), index)

   unsigned i, j;

   secure = discard = false;

   for (i = j = 0; i < vec.size(); i += 2)
      {
      if (name_ == vec[i])
         {
         if (index == j && (i + 1) < vec.size())
            {
            value_ = vec[i+1];

            if ((i + 3) < vec.size() && check(i+2))
               {
               SET_VALUE(3);
               }

            if ((i + 5) < vec.size() && check(i+4))
               {
               SET_VALUE(5);
               }

            if ((i + 7) < vec.size() && check(i+6))
               {
               SET_VALUE(7);
               }

            if ((i + 9) < vec.size() && check(i+8))
               {
               SET_VALUE(9);
               }

            if ((i + 11) < vec.size() && check(i+10))
               {
               SET_VALUE(11);
               }

            if ((i + 13) < vec.size() && check(i+12))
               {
               SET_VALUE(13);
               }

            if ((i + 15) < vec.size() && check(i+14))
               {
               SET_VALUE(15);
               }

            if ((i + 17) < vec.size() && check(i+16))
               {
               SET_VALUE(17);
               }

            if ((i + 19) < vec.size() && check(i+18))
               {
               SET_VALUE(19);
               }

            U_RETURN(true);
            }

         ++j;
         }
      }

   U_RETURN(false);
}

bool HttpSetCookie::del(const UString& name_, unsigned index)
{
   U_TRACE(5, "HttpSetCookie::del(%.*S,%d)", U_STRING_TO_TRACE(name_), index)

   unsigned i, j;

   for (i = j = 0; i < vec.size(); i += 2)
      {
      if (name_ == vec[i])
         {
         if (index == j)
            {
            // erase [first,last[

            unsigned first = i, last = i+2;

            if ((i + 3) < vec.size() && check(i+2))
               {
               last = i+4;
               }

            if ((i + 5) < vec.size() && check(i+4))
               {
               last = i+6;
               }

            if ((i + 7) < vec.size() && check(i+6))
               {
               last = i+8;
               }

            if ((i + 9) < vec.size() && check(i+8))
               {
               last = i+10;
               }

            if ((i + 11) < vec.size() && check(i+10))
               {
               last = i+12;
               }

            if ((i + 13) < vec.size() && check(i+12))
               {
               last = i+14;
               }

            if ((i + 15) < vec.size() && check(i+14))
               {
               last = i+16;
               }

            if ((i + 17) < vec.size() && check(i+16))
               {
               last = i+18;
               }

            if ((i + 19) < vec.size() && check(i+18))
               {
               last = i+20;
               }

            if ((i + 21) < vec.size() && check(i+20))
               {
               last = i+22;
               }

            U_VEC_ERASE2(vec, first, last);

            U_RETURN(true);
            }

         ++j;
         }
      }

   U_RETURN(false);
}

bool HttpSetCookie::del_all(const UString& name_)
{
   U_TRACE(5, "HttpSetCookie::del_all(%.*S)", U_STRING_TO_TRACE(name_))

   if (del(name_) == false)
      {
      U_RETURN(false);
      }

   while (del(name_))
      {
      }

   U_RETURN(true);
}

void HttpSetCookie::stringify(UString& field)
{
   U_TRACE(5, "HttpSetCookie::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += name;
   field.append(U_CONSTANT_TO_PARAM(": "));

   unsigned i = 0;

   while (i < vec.size())
      {
      field += vec[i];

      if (vec[i] != U_STRING_FROM_CONSTANT("Secure") &&
          vec[i] != U_STRING_FROM_CONSTANT("Discard"))
         {
         field.append(1, '=');

         field += vec[i+1];
         }

      i += 2;

      if (i < vec.size())
         {
         field.append(U_CONSTANT_TO_PARAM("; "));
         }
      }

   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpSetCookie::dump(bool reset) const
{
   HttpField::dump(false);

   *UObjectIO::os << "\n"
                  << "is_path            " << is_path        << "\n"
                  << "is_port            " << is_port        << "\n"
                  << "version2           " << version2       << "\n"
                  << "is_domain          " << is_domain      << "\n"
                  << "is_secure          " << is_secure      << "\n"
                  << "is_discard         " << is_discard     << "\n"
                  << "is_max_age         " << is_max_age     << "\n"
                  << "is_version         " << is_version     << "\n"
                  << "is_comment         " << is_comment     << "\n"
                  << "is_comment_url     " << is_comment_url << "\n"
                  << "vec       (UVector " << (void*)&vec    << ")\n"
                  << "buffer    (UString " << (void*)&buffer << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
