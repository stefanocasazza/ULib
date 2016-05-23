// HttpCookie.cpp

#include <HttpCookie.h>

/*
The following grammar uses the notation, and tokens DIGIT (decimal digits) and
token (informally, a sequence of non-special, non-white space characters).

cookie          =  "Cookie:" cookie-version 1*((";" | ",") cookie-value)  
cookie-value    =  NAME "=" VALUE [";" path] [";" domain] [";" port]
cookie-version  =  "$Version" "=" value
NAME            =  attr
VALUE           =  value
path            =  "$Path" "=" value
domain          =  "$Domain" "=" value
port            =  "$Port" [ "=" <"> value <"> ]
attr            =  token
value           =  word
word            =  token | quoted-string

Example:
   Cookie: $Version="1";
          Part_Number="Riding_Rocket_0023"; $Path="/acme/ammo";
          Part_Number="Rocket_Launcher_0001"; $Path="/acme"
*/

// #define SERGIO

#ifdef SERGIO

static unsigned cookie_split(UVector<UString>& vec, const UString& buffer, const char* delim)
{
   U_TRACE(5, "cookie_split(%p,%.*S,%S)", &vec, U_STRING_TO_TRACE(buffer), delim)

   UString x;

   unsigned r, n   = vec.size();
   const char* s   = buffer.data();
   const char* ss  = s;
   const char* end = s + buffer.size();
   const char* p;
   const char* b = s;

loop:
   if (s >= end) goto done;

   if (strchr(delim, *s))
      {
      ++s; 

      goto loop;
      }
   else
      {
      while (isspace(*s)) s++;

      p = s++;

      if (*(s-1) == '"')
         {
         while (s < end && *s != '"') ++s;
         }

      ss = s;

      while (s < end && strchr(delim,*s) == 0)
         {
         ++s;

         if (!isspace(*(s-1))) ss = s;
         }
      }

   if (*p == '"' && *(ss-1) == '"')
      {
      p++;
      ss--;
      }

   x = buffer.substr(p - b, ss - p);

   vec.push_back(x);

   ++s;

   goto loop;

done:
   r = vec.size() - n;

#ifdef DEBUG
   for (unsigned i = 0; i < r; ++i)
      {
      U_DUMP("vec[%d] = %.*S", n+i, U_STRING_TO_TRACE(vec[n+i]))
      }
#endif

   U_RETURN(r);
}

#endif

HttpCookie::HttpCookie(const char* name_, unsigned name_len, const char* value_, unsigned value_len)
{
   U_TRACE_REGISTER_OBJECT(5, HttpCookie, "%.*S,%u,%.*S,%u", name_len, name_, name_len, value_len, value_, value_len)

   U_INTERNAL_ASSERT(memcmp(name_,  U_CONSTANT_TO_PARAM("Cookie")) == 0)

     name.assign(name_,   name_len);
   buffer.assign(value_, value_len);

   is_path = is_domain = is_port = false;

#ifdef SERGIO
   // NOTA BENE: La virgola come separatore non viene supportata per consentire l'utilizzo di COOKIE_SPECIAL_VALUE

   UVector<UString> tvec,tvec1;

   (void) cookie_split(tvec, buffer, "=");

   const char* pc;
   unsigned i, num;
   const char* delim;

   for (i = 0; i < tvec.size(); i++)
      {
      const char* s = tvec[i].data();

      /* modifica stefano */
      unsigned size = tvec[i].size();

      pc = (const char*) memrchr(s, ',', size);

      const char* p = (const char*) memrchr(s, ';', size);

      if (pc > p)
         {
         p = pc;

         delim = ",";
         }
      else
         {
         delim = ";";
         }

      if (p != NULL)
         {
         /* modifica stefano */

         num = cookie_split(tvec1, tvec[i], delim);

         U_INTERNAL_ASSERT(num > 1)

         vec.push_back(tvec1[0]);
         vec.push_back(tvec1[1]);

         tvec1.clear();
         }
      else
         {
         vec.push_back(tvec[i]);
         }
      }

   tvec.clear();
#else
   (void) U_VEC_SPLIT(vec, buffer, "=;, \r\n");
#endif
}

unsigned HttpCookie::count(const UString& name_)
{
   U_TRACE(5, "HttpCookie::count(%.*S)", U_STRING_TO_TRACE(name_))

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

void HttpCookie::add(const UString& name_, const UString& value_)
{
   U_TRACE(5, "HttpCookie::add(%.*S,%.*S)", U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_))

   vec.push_back(name_);
   vec.push_back(value_);
}

bool HttpCookie::check(unsigned i)
{
   U_TRACE(5, "HttpCookie::check(%u)", i)

   is_path   = (vec[i] == U_STRING_FROM_CONSTANT("$Path"));
   is_port   = (vec[i] == U_STRING_FROM_CONSTANT("$Port"));
   is_domain = (vec[i] == U_STRING_FROM_CONSTANT("$Domain"));

   U_RETURN(is_path || is_port || is_domain);
}

#define SET_VALUE(n) \
      if      (is_path)    path    = vec[i+n]; \
      else if (is_domain)  domain  = vec[i+n]; \
      else                 port    = vec[i+n];

bool HttpCookie::find(const UString& name_, UString& value_, UString& path, UString& domain, UString& port, unsigned index)
{
   U_TRACE(5, "HttpCookie::find(%.*S,%.*S,%.*S,%.*S,%d)",
            U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_), U_STRING_TO_TRACE(path), U_STRING_TO_TRACE(domain), index)

   unsigned i, j;

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

            U_RETURN(true);
            }

         ++j;
         }
      }

   U_RETURN(false);
}

bool HttpCookie::del(const UString& name_, unsigned index)
{
   U_TRACE(5, "HttpCookie::del(%.*S,%d)", U_STRING_TO_TRACE(name_), index)

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

            U_VEC_ERASE2(vec, first, last);

            U_RETURN(true);
            }

         ++j;
         }
      }

   U_RETURN(false);
}

bool HttpCookie::del_all(const UString& name_)
{
   U_TRACE(5, "HttpCookie::del_all(%.*S)", U_STRING_TO_TRACE(name_))

   if (del(name_) == false)
      {
      U_RETURN(false);
      }

   while (del(name_))
      {
      }

   U_RETURN(true);
}

void HttpCookie::stringify(UString& field)
{
   U_TRACE(5, "HttpCookie::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += name;
   field.append(U_CONSTANT_TO_PARAM(": "));

   /*
   unsigned i = 0;

   while (i < vec.size())
      {
      field += vec[i];
      field.append(1, '=');
      field += vec[i+1];

      i += 2;

      if (i < vec.size())
         {
         field.append(U_CONSTANT_TO_PARAM("; "));
         }
      }
   */

   field.append(buffer);

   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpCookie::dump(bool reset) const
{
   HttpField::dump(false);

   *UObjectIO::os << "\n"
                  << "is_path            " << is_path        << "\n"
                  << "is_port            " << is_port        << "\n"
                  << "is_domain          " << is_domain      << "\n"
                  << "vec       (UVector " << (void*)&vec    << ")\n"
                  << "buffer    (UString " << (void*)&buffer << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
