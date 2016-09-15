// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    file_config.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/escape.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>

#if defined(__NetBSD__) || defined(__UNIKERNEL__) || defined(__OSX__)
extern char** environ;
#endif

U_NO_EXPORT void UFileConfig::init()
{
   U_TRACE_NO_PARAM(0, "UFileConfig::init()")

   UFile::map      = (char*)MAP_FAILED;
   UFile::st_size  = 0;
   UFile::map_size = 0;

   _start = data.data();

   reset();
}

UFileConfig::UFileConfig()
{
   U_TRACE_REGISTER_OBJECT(0, UFileConfig, "")

   init();

   preprocessing = false;
}

UFileConfig::UFileConfig(const UString& _data, bool _preprocessing) : data(_data)
{
   U_TRACE_REGISTER_OBJECT(0, UFileConfig, "%V,%b", _data.rep, _preprocessing)

   init();

   preprocessing = _preprocessing;
}

bool UFileConfig::processData(bool bload)
{
   U_TRACE(0, "UFileConfig::processData(%b)", bload)

   U_CHECK_MEMORY

   // manage if we need preprocessing...

#if defined(HAVE_CPP) || defined(HAVE_MCPP)
   if (preprocessing ||
       UServices::dosMatch(data, U_CONSTANT_TO_PARAM("*#include *\n*"), 0))
      {
      static int fd_stderr;

      UString command(200U),
              _dir = UStringExt::dirname(pathname);

#  ifdef DEBUG
#     define DBG_DEF " -DDEBUG "
#  else
#     define DBG_DEF
#  endif

      if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/cpp.err");

      command.snprintf(U_CONSTANT_TO_PARAM("cpp -undef -nostdinc -w -P -C " DBG_DEF "-I%v -"), _dir.rep);

      if (UFile::isOpen())
         {
         (void) UFile::lseek(U_SEEK_BEGIN, SEEK_SET);

         data = UCommand::outputCommand(command, environ, UFile::getFd(), fd_stderr);

#     ifdef HAVE_MCPP
         if (data.empty())
            {
            command.snprintf(U_CONSTANT_TO_PARAM("mcpp -P -C " DBG_DEF "-I%v -"), _dir.rep);

            (void) UFile::lseek(U_SEEK_BEGIN, SEEK_SET);

            data = UCommand::outputCommand(command, environ, UFile::getFd(), fd_stderr);
            }
#     endif
         }
      else
         {
         UCommand cmd(command);
         UString output(U_CAPACITY);

         (void) cmd.execute(&data, &output, -1, fd_stderr);

#     ifndef U_LOG_DISABLE
         UServer_Base::logCommandMsgError(cmd.getCommand(), true);
#     endif

#     ifdef HAVE_MCPP
         if (data.empty())
            {
            UCommand _cmd(command);

            command.snprintf(U_CONSTANT_TO_PARAM("mcpp -P -C " DBG_DEF "-I%v -"), _dir.rep);

            (void) _cmd.execute(&data, &output, -1, fd_stderr);

#        ifndef U_LOG_DISABLE
            UServer_Base::logCommandMsgError(_cmd.getCommand(), true);
#        endif
            }
#     endif

         data = output;
         }
      }
#endif

   if (data.empty())   U_RETURN(false);
   if (bload == false) U_RETURN(true);

   _end   = data.pend();
   _start = data.data();
   _size  = data.size();

   if (UFile::isPath())
      {
      //------------ -------------------------------------------------------------
      // Loads configuration information from the file. The file type is
      // determined by the file extension. The following extensions are supported:
      // -------------------------------------------------------------------------
      // .ini        - initialization file (Windows INI)
      // .properties - properties file (JAVA Properties)
      // -------------------------------------------------------------------------

      UString suffix = UFile::getSuffix();

      if (suffix)
         {
         if (suffix.equal(U_CONSTANT_TO_PARAM("ini")))
            {
            if (loadINI()) U_RETURN(true);

            U_RETURN(false);
            }

         if (suffix.equal(U_CONSTANT_TO_PARAM("properties")))
            {
            if (loadProperties()) U_RETURN(true);

            U_RETURN(false);
            }
         }
      }

   if (loadSection(0, 0)) U_RETURN(true);

   U_RETURN(false);
}

void UFileConfig::load()
{
   U_TRACE_NO_PARAM(0, "UFileConfig::load()")

   U_CHECK_MEMORY

   U_ASSERT_EQUALS(isLoaded(), false)

   if (UFile::open()                   &&
       UFile::size() > 0               &&
       UFile::memmap(PROT_READ, &data) &&
       processData(true))
      {
      if (UFile::isOpen()) UFile::close();
      }
   else
      {
      U_ERROR("Configuration file %.*S processing failed", U_FILE_TO_TRACE(*this));
      }
}

// Perform search of first caracter '{' and check section name before...

bool UFileConfig::searchForObjectStream(const char* section, uint32_t len)
{
   U_TRACE(0, "UFileConfig::searchForObjectStream(%.*S,%u)", len, section, len)

   U_INTERNAL_DUMP("_size = %u _start = %.*S", _size, 10, _start)

   if (len == 0) section = "{";

   bool bretry            = len && (_start != data.data());
   const char* save_start = _start;

retry:
   while (_start < _end)
      {
      _start = u_skip(_start, _end, 0, '#');

      if (_start == _end) break;

      U_INTERNAL_ASSERT_EQUALS(u__isspace(_start[0]), false)

      U_INTERNAL_DUMP("_start = %.*S", 10, _start)

      if (_start[0] != section[0]               ||
          (len && memcmp(_start, section, len)) ||
          (u__isspace(_start[(len ? len : 1)]) == false)) // check for partial match of the name section...
         {
         while (u__isspace(*_start) == false) ++_start;

         continue;
         }

      _start += (len ? len : 1);

      U_INTERNAL_ASSERT(u__isspace(_start[0]))

      if (len)
         {
         // check the caracter after the name of the section...

         while (u__isspace(*_start)) ++_start;

         if (*_start != '{') continue;

         // find the end of the section and consider that as EOF... (call reset() when done with this section)

         _end = u_strpend(_start, data.remain(_start), U_CONSTANT_TO_PARAM("{}"), '#');

         U_INTERNAL_DUMP("_end = %p", _end)

         if (_end == 0) break;

         _size = (_end - ++_start); // NB: we advance one char (to call u_skip() after...)

         U_INTERNAL_DUMP("_size = %u _end = %.*S", _size, 10, _end)
         }

      // FOUND

      U_RETURN(true);
      }

   if (bretry)
      {
      bretry = false;
      _start = data.data();

      goto retry;
      }

   _start = save_start;

   U_INTERNAL_DUMP("_size = %u _start = %.*S", _size, 10, _start)

   U_RETURN(false);
}

bool UFileConfig::loadTable(UHashMap<UString>& tbl)
{
   U_TRACE(0, "UFileConfig::loadTable(%p)", &tbl) // problem with sanitize address

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_size = %u _start = %.*S", _size, 10, _start)

   if (_size)
      {
      uint32_t len = tbl.loadFromData(_start, _size);

      _start += len;
      _size  -= len;

      U_INTERNAL_DUMP("_size = %u _start = %.*S", _size, 10, _start)

      if (tbl.empty() == false) U_RETURN(true);
      }

   U_RETURN(false);
}

bool UFileConfig::loadVector(UVector<UString>& vec, const char* name)
{
   U_TRACE(0, "UFileConfig::loadVector(%p,%S)", &vec, name)

   U_CHECK_MEMORY

   _start = u_skip(_start, _end, 0, '#');

   if (_start == _end) U_RETURN(false);

   U_INTERNAL_ASSERT_EQUALS(u__isspace(_start[0]), false)

   U_INTERNAL_DUMP("_start = %.*S", 10, _start)

   uint32_t len = (name ? u__strlen(name, __PRETTY_FUNCTION__) : 0);

   if (len)
      {
      if (_start[0] != name[0] ||
          memcmp(_start, name, len))
         {
         U_RETURN(false);
         }

      _start += len;

      U_INTERNAL_ASSERT(u__isspace(_start[0]))

      while (u__isspace(*_start)) ++_start;
      }

   if (_start[0] == '[' ||
       _start[0] == '(')
      {
      len = vec.loadFromData(_start, _size);

      _start += len;
      _size  -= len;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UFileConfig::loadSection(const char* section, uint32_t len)
{
   U_TRACE(0, "UFileConfig::loadSection(%.*S,%u)", len, section, len)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MAJOR(_size, 0)

   if (searchForObjectStream(section, len))
      {
      table.clear();
      
      if (loadTable(table)) U_RETURN(true);
      }

   U_RETURN(false);
}

bool UFileConfig::loadINI()
{
   U_TRACE_NO_PARAM(0, "UFileConfig::loadINI()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_size = %u _start = %.*S", _size, 10, _start)

   uint32_t len;
   const char* ptr;
   UString sectionKey, fullKey(U_CAPACITY), key, value;

   while (_start < _end)
      {
      _start = u_skip(_start, _end, 0, ';');

      if (_start == _end) break;

      U_INTERNAL_ASSERT_EQUALS(u__isspace(_start[0]),false)

      U_INTERNAL_DUMP("_start = %.*S", 10, _start)

      if (_start[0] == '[') // a line starting with a square bracket denotes a section key [<key>]
         {
         ++_start;

         ptr = u__strpbrk(_start, _end - _start, "]\n");

         if (ptr == 0)
            {
            _start = _end;

            break;
            }

         sectionKey = UStringExt::trim(_start, ptr - _start);

         _start = ptr;
         }
      else // every other line denotes a property assignment in the form <value key> = <value>
         {
         ptr = u__strpbrk(_start, _end - _start, "=\n");

         if (ptr == 0)
            {
            _start = _end;

            break;
            }

         key = UStringExt::trim(_start, ptr - _start);

         _start = ptr;

         if (*_start == '=')
            {
            ++_start;

            ptr = (const char*) memchr(_start, '\n', _end - _start);

            if (ptr == 0)
               {
               _start = _end;

               break;
               }

            value = UStringExt::trim(_start, ptr - _start);

            _start = ptr;
            }

         // The name of a property is composed of the section key and the value key, separated by a period (<section key>.<value key>)

         len = sectionKey.size();

         fullKey.setBuffer(len + 1 + key.size());

         if (len == 0) fullKey.snprintf(U_CONSTANT_TO_PARAM(   "%v"),                 key.rep);
         else          fullKey.snprintf(U_CONSTANT_TO_PARAM("%v.%v"), sectionKey.rep, key.rep);

         (void) fullKey.shrink();

         table.insert(fullKey, value);
         }

      if (_start >= _end) break;

      ++_start;
      }

   _size = (_end - _start);

   U_INTERNAL_DUMP("_size = %u", _size)

   bool result = (table.empty() == false);

   U_RETURN(result);
}

bool UFileConfig::loadProperties(UHashMap<UString>& table, const char* _start, const char* _end)
{
   U_TRACE(0, "UFileConfig::loadProperties(%p,%p,%p)", &table, _start, _end)

   U_INTERNAL_DUMP("_start = %.*S", 10, _start)

   char c;
   const char* ptr;
   UString key, value;

   while (_start < _end)
      {
      // skip white space

      if (u__isspace(*_start))
         {
         ++_start;

         continue;
         }

      // a line starting with a hash '#' or exclamation mark '!' is treated as a comment and ignored

      c = *_start;

      if (c == '#' ||
          c == '!')
         {
         // skip line comment

         _start = (const char*) memchr(_start, '\n', _end - _start);

         if (_start == 0) _start = _end;

         continue;
         }

      U_INTERNAL_ASSERT_EQUALS(u__isspace(_start[0]),false)

      U_INTERNAL_DUMP("_start = %.*S", 10, _start)

      // every other line denotes a property assignment in the form <key> = <value>

      ptr = u__strpbrk(_start, _end - _start, "=:\r\n");

      if (ptr == 0) break;

      key = UStringExt::trim(_start, ptr - _start);

      _start = ptr;

      c = *_start;

      if (c == '=' ||
          c == ':')
         {
         ++_start;

         ptr = (const char*) memchr(_start, '\n', _end - _start);

         if (ptr == 0) break;

         value = UStringExt::trim(_start, ptr - _start);

         // NB: var shell often need to be quoted...

         if (c == '=' &&
             value.isQuoted())
            {
            value.unQuote();
            }

         _start = ptr;
         }

      table.insert(key, value);

      if (_start >= _end) break;

      ++_start;
      }

   if (table.empty()) U_RETURN(false);

   U_RETURN(true);
}

bool UFileConfig::loadProperties()
{
   U_TRACE_NO_PARAM(0, "UFileConfig::loadProperties()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_size = %u _start = %.*S", _size, 10, _start)

   bool result = loadProperties(table, _start, _end);

   if (result)
      {
      _size = (_end - _start);

      U_INTERNAL_DUMP("_size = %u", _size)

      U_RETURN(true);
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UFileConfig::dump(bool _reset) const
{
   UFile::dump(false);

   *UObjectIO::os << '\n'
                  << "_end                      " << (void*)_end   << '\n'
                  << "_size                     " << _size         << '\n'
                  << "_start                    " << (void*)_start << '\n'
                  << "preprocessing             " << preprocessing << '\n'
                  << "data  (UString            " << (void*)&data  << ")\n"
                  << "table (UHashMap<UString>  " << (void*)&table << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
