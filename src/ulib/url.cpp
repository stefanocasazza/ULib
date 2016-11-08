// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    url.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/utility/string_ext.h>

// gcc: call is unlikely and code size would grow

UString Url::getService() const
{
   U_TRACE_NO_PARAM(0, "Url::getService()")

   UString srv;

   if (service_end > 0) srv = url.substr(0U, (uint32_t)service_end);

   U_RETURN_STRING(srv);
}

void Url::setService(const char* service, uint32_t n)
{
   U_TRACE(0, "Url::setService(%S,%u)", service, n)

   U_INTERNAL_ASSERT_POINTER(service)

   if (service_end > 0) (void) url.replace(0, service_end, service, n);
   else
      {
      char buffer[32];

      (void) url.insert(0, buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%.*s://"), n, service));
      }

   findpos();
}

UString Url::getUser()
{
   U_TRACE_NO_PARAM(0, "Url::getUser()")

   UString usr;

   if (user_begin < user_end) usr = url.substr(user_begin, user_end - user_begin);

   U_RETURN_STRING(usr);
}

bool Url::setUser(const char* user, uint32_t n)
{
   U_TRACE(0, "Url::setUser(%S,%u)", user, n)

   U_INTERNAL_ASSERT_POINTER(user)

   // Only posible if there is a url

   if (host_begin < host_end)
      {
      if (user_begin < user_end) (void) url.replace(user_begin, user_end - user_begin, user, n);
      else
         {
         char buffer[128];

         (void) url.insert(user_begin, buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%.*s@"), n, user));
         }

      findpos();

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString Url::getHost()
{
   U_TRACE_NO_PARAM(0, "Url::getHost()")

   UString host;

   if (host_begin < host_end) host = url.substr(host_begin, host_end - host_begin);

   U_RETURN_STRING(host);
}

void Url::setHost(const char* _host, uint32_t n)
{
   U_TRACE(0, "Url::setHost(%S,%u)", _host, n)

   U_INTERNAL_ASSERT_POINTER(_host)

   if (host_begin < host_end) (void) url.replace(host_begin, host_end - host_begin, _host, n);
   else                       (void) url.insert( host_begin,                        _host, n);

   findpos();
}

unsigned int Url::getPort()
{
   U_TRACE_NO_PARAM(0, "Url::getPort()")

   if (host_end < path_begin)
      {
      int size = path_begin - host_end - 1;

      if (size > 0 &&
          size <= 5)
         {
         char buffer[6];

         url.copy(buffer, size, host_end + 1);

         unsigned int port = atoi(buffer);

         U_RETURN(port);
         }
      }

   if (service_end > 0)
      {
      enum {
         URL_FTP  = U_MULTICHAR_CONSTANT32('f','t','p',':'),
         URL_HTTP = U_MULTICHAR_CONSTANT32('h','t','t','p'),
         URL_LDAP = U_MULTICHAR_CONSTANT32('l','d','a','p'),
         URL_SMTP = U_MULTICHAR_CONSTANT32('s','m','t','p'),
         URL_POP3 = U_MULTICHAR_CONSTANT32('p','o','p','3'),
      };

      const char* ptr = url.data();

      switch (u_get_unalignedp32(ptr))
         {
         case URL_FTP:  U_RETURN(21);
         case URL_SMTP: U_RETURN(25);
         case URL_POP3: U_RETURN(110);

         case URL_HTTP:
            {
            if (ptr[4] == 's') U_RETURN(443);

            U_RETURN(80);
            }
         break;

         case URL_LDAP:
            {
            if (ptr[4] == 's') U_RETURN(636);

            U_RETURN(389);
            }
         break;
         }
      }

   U_RETURN(0);
}

bool Url::setPort(unsigned int port)
{
   U_TRACE(0, "Url::setPort(%u)", port)

   // Only posible if there is a url

   if (port <= 0xFFFF &&
       host_begin < host_end)
      {
      char buffer[10];

      buffer[0] = ':';

      char* ptr = buffer+1;

      (void) url.replace(host_end, path_begin - host_end, buffer, u_num2str32(port, ptr) - ptr);

      findpos();

      U_RETURN(true);
      }

   U_RETURN(false);
}

void Url::setPath(const char* path, uint32_t n)
{
   U_TRACE(0, "Url::setPath(%S,%u)", path, n)

   U_INTERNAL_ASSERT_POINTER(path)

   if (path_begin < path_end)
      {
      if (*path != '/') ++path_begin;

      (void) url.replace(path_begin, path_end - path_begin, path, n);
      }
   else
      {
      if (*path != '/')
         {
         (void) url.insert(path_begin, 1, '/');

         ++path_begin;
         }

      (void) url.insert(path_begin, path, n);
      }

   findpos();
}

UString Url::getPath()
{
   U_TRACE_NO_PARAM(0, "Url::getPath()")

   UString path(U_CAPACITY);

   if (path_begin < path_end) decode(url.c_pointer(path_begin), path_end - path_begin, path);
   else                       path.push_back('/');

   (void) path.shrink();

   U_RETURN_STRING(path);
}

UString Url::getQuery()
{
   U_TRACE_NO_PARAM(0, "Url::getQuery()")

   int _end = url.size() - 1;

   if (path_end < _end)
      {
      uint32_t sz = _end - path_end;

      UString _query(sz);

      decode(url.c_pointer(path_end + 1), sz, _query);

      U_RETURN_STRING(_query);
      }

   return UString::getStringNull();
}

uint32_t Url::getQuery(UVector<UString>& vec)
{
   U_TRACE(0, "Url::getQuery(%p)", &vec)

   int _end = url.size() - 1;

   if (path_end < _end) return UStringExt::getNameValueFromData(url.substr(path_end + 1, _end - path_end), vec, U_CONSTANT_TO_PARAM("&"));

   U_RETURN(0);
}

bool Url::setQuery(const char* query_, uint32_t query_len)
{
   U_TRACE(0, "Url::setQuery(%S,%u)", query_, query_len)

   U_INTERNAL_ASSERT_POINTER(query_)

   if (prepareForQuery())
      {
      if (*query_ == '?') ++query_;

      (void) url.replace(path_end + 1, url.size() - path_end - 1, query_, query_len);

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool Url::setQuery(UVector<UString>& vec)
{
   U_TRACE(0, "Url::setQuery(%p)", &vec)

   U_INTERNAL_ASSERT_EQUALS(vec.empty(), false)

   if (prepareForQuery())
      {
      UString name, value;

      for (int32_t i = 0, n = vec.size(); i < n; ++i)
         {
         name  = vec[i++];
         value = vec[i];

         addQuery(U_STRING_TO_PARAM(name), U_STRING_TO_PARAM(value));
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString Url::getQueryBody(UVector<UString>& vec)
{
   U_TRACE(0, "Url::getQueryBody(%p)", &vec)

   U_INTERNAL_ASSERT_EQUALS(vec.empty(), false)

   char buffer[4096];
   uint32_t sz, value_sz;
   UString name, value, query(U_CAPACITY);

   for (int32_t i = 0, n = vec.size(); i < n; ++i)
      {
      name  = vec[i++];
      value = vec[i];

      (void) query.reserve(3U +         name.size()  +
                           (      sz = query.size()) +
                           (value_sz = value.size()));

      uint32_t encoded_sz = u_url_encode((const unsigned char*)value.data(), value_sz, (unsigned char*)buffer);

      U_INTERNAL_ASSERT_MINOR(encoded_sz, sizeof(buffer))

      query.snprintf_add(U_CONSTANT_TO_PARAM("%.*s%v=%.*s"), (sz > 0), "&", name.rep, encoded_sz, buffer);
      }

   U_RETURN_STRING(query);
}

UString Url::getPathAndQuery()
{
   U_TRACE_NO_PARAM(0, "Url::getPathAndQuery()")

   UString file;

   if (path_begin < path_end) file = url.substr(path_begin);
   else                       file.push_back('/');

   U_RETURN_STRING(file);
}

void Url::findpos()
{
   U_TRACE_NO_PARAM(0, "Url::findpos()")

   // proto://[user[:password]@]hostname[:port]/[path]?[query]

   service_end = U_STRING_FIND(url, 0, "//");

   if (service_end < 0)
      {
      service_end =
       user_begin =
         user_end =
       host_begin =
         host_end =
       path_begin =
         path_end =
            query = 0;

      return;
      }

   U_INTERNAL_ASSERT(u_isUrlScheme(U_STRING_TO_PARAM(url)))

   user_begin = service_end + 2;

   --service_end; // cut ':'

   path_begin = url.find('/', user_begin);

   if (path_begin < 0)
      {
      path_begin = url.find('?', user_begin);

      if (path_begin < 0) path_begin = url.size();
      }

   int temp;

   if (service_end == 0 &&
       path_begin       &&
       (url.c_char(path_begin-1) == ':'))
      {
      temp = url.find('.');

      if (temp < 0 ||
          temp > path_begin)
         {
         service_end = path_begin - 1;
         user_begin  = service_end;
         user_end    = user_begin;
         }
      }

   user_end = url.find('@', user_begin);

   if (user_end < 0 ||
       user_end > path_begin)
      {
      user_end   = user_begin;
      host_begin = user_end;
      }
   else
      {
      host_begin = user_end + 1;
      }

   // find ipv6 adresses

   temp = url.find('[', host_begin);

   if (temp >= 0 &&
       temp < path_begin)
      {
      host_end = url.find(']', temp);

      if (host_end < path_begin) ++host_end;
      else                         host_end = host_begin;
      }
   else
      {
      host_end = url.find(':', host_begin);

      if (host_end < 0 ||
          host_end > path_begin)
         {
         host_end = path_begin;
         }
      }

   path_end = url.find('?', path_begin);

   if (path_end < path_begin) path_end = url.size();

   query = path_end;

   U_INTERNAL_DUMP("service_end = %d user_begin = %d user_end = %d host_begin = %d host_end = %d path_begin = %d path_end = %d query = %d",
                    service_end,     user_begin,     user_end,     host_begin,     host_end,     path_begin,     path_end,     query)

#ifdef DEBUG
   if (service_end > 0        &&
       user_begin == user_end &&
       strncmp(url.data(), U_CONSTANT_TO_PARAM("http")) == 0)
      {
      U_ASSERT(u_isURL(U_STRING_TO_PARAM(url)))
      }
#endif
}

U_NO_EXPORT bool Url::prepareForQuery()
{
   U_TRACE_NO_PARAM(0, "Url::prepareForQuery()")

   // NB: Only posible if there is a url

   if (host_begin < host_end)
      {
      if (path_begin == path_end)
         {
         (void) url.insert(path_begin, 1, '/');

         ++path_end;
         }

      if (path_end == (int)url.size()) url.push_back('?');

      U_RETURN(true);
      }

   U_RETURN(false);
}

void Url::addQuery(const char* entry, uint32_t entry_len, const char* value, uint32_t value_len)
{
   U_TRACE(0, "Url::addQuery(%.*S,%u,%.*S,%u)", entry_len, entry, entry_len, value_len, value, value_len)

   U_INTERNAL_ASSERT_POINTER(entry)

   if (prepareForQuery())
      {
      uint32_t v_size = 0,
               b_size = entry_len,
               e_size = b_size;

      if (value) v_size = value_len;

      if (e_size < v_size) b_size = v_size;

      if (url.last_char() != '?') url.push_back('&');

      if (u_isUrlEncodeNeeded(entry, e_size) == false) (void) url.append(entry, e_size);
      else
         {
         UString buffer(b_size * 3);

         encode(entry, e_size, buffer);

         (void) url.append(buffer);
         }

      if (value)
         {
         url.push_back('=');

         if (u_isUrlEncodeNeeded(value, v_size) == false) (void) url.append(value, v_size);
         else
            {
            UString buffer(v_size * 3);

            encode(value, v_size, buffer);

            (void) url.append(buffer);
            }
         }
      }
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, Url& u)
{
   U_TRACE(0+256, "Url::operator>>(%p,%p)", &is, &u)

   is >> u.url;

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, const Url& u)
{
   U_TRACE(0+256, "Url::operator<<(%p,%p)", &os, &u)

   os << u.url;

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* Url::dump(bool reset) const
{
   *UObjectIO::os << "service_end  " << service_end << '\n'
                  << "user_begin   " << user_begin  << '\n'
                  << "user_end     " << user_end    << '\n'
                  << "host_begin   " << host_begin  << '\n'
                  << "host_end     " << host_end    << '\n'
                  << "path_begin   " << path_begin  << '\n'
                  << "path_end     " << path_end    << '\n'
                  << "query        " << query       << '\n'
                  << "url (UString " << (void*)&url << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
