// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    file_config.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_FILECONFIG_H
#define ULIB_FILECONFIG_H 1

#include <ulib/file.h>
#include <ulib/container/hash_map.h>

class UServer_Base;
class UClient_Base;

class U_EXPORT UFileConfig : public UFile {
public:

    UFileConfig();
    UFileConfig(const UString& _data, bool _preprocessing);

   ~UFileConfig()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UFileConfig)
      }

   // SERVICES

   void destroy()
      {
      U_TRACE(0, "UFileConfig::destroy()")

           clear();
      data.clear();
      }

   // section management

   char peek() { return _start[0]; }

   void reset()
      {
      U_TRACE_NO_PARAM(0, "UFileConfig::reset()")

      _end  = data.pend();
      _size = (_end - _start);
      }

   bool searchForObjectStream(const char* section = 0, uint32_t len = 0);

   // Wrapper for table

   UHashMap<UString> table;

   bool empty() const { return table.empty(); }

   void clear() { table.clear(); table.deallocate(); }

   UString      erase(const UString& key)                  { return table.erase(key); }
   UString operator[](const UString& key)                  { return table[key]; }
   UString         at(const char*    key, uint32_t keylen) { return table.at(key, keylen); }

   // Facilities

   long readLong(const char* key, uint32_t keylen, long default_value = 0)
      {
      U_TRACE(0, "UFileConfig::readLong(%.*S,%u,%ld)", keylen, key, keylen, default_value)

      UString value = at(key, keylen);

      if (value) default_value = value.strtol(true);

      U_RETURN(default_value);
      }

   bool readBoolean(const char* key, uint32_t keylen, bool default_value = false)
      {
      U_TRACE(0, "UFileConfig::readBoolean(%.*S,%u,%b)", keylen, key, keylen, default_value)

      UString value = at(key, keylen);

      if (value) default_value = value.strtob();

      U_RETURN(default_value);
      }

   long readLong(   const UString& key, long default_value = 0) { return readLong(   U_STRING_TO_PARAM(key), default_value); }
   bool readBoolean(const UString& key)                         { return readBoolean(U_STRING_TO_PARAM(key)); }

   // Load a configuration file

   void load(const UString& path)
      {
      U_TRACE(0, "UFileConfig::load(%V)", path.rep)

      UFile::setPath(path);

      load();
      }

   bool isLoaded()
      {
      U_TRACE_NO_PARAM(0, "UFileConfig::isLoaded()")

      U_INTERNAL_DUMP("st_size = %I", UFile::st_size)

      if (UFile::st_size) U_RETURN(true);

      U_RETURN(false);
      }

   bool loadTable() { return loadTable(table); }

   bool loadSection(const char* section, uint32_t len);

   bool loadVector(UVector<UString>& vec, const char* name = 0);

   // EXT

   bool processData(bool bload);

   UString getData() const                                    { return  data; }
   void    setData(const UString& _data, bool _preprocessing) { data = _data; preprocessing = _preprocessing; }

   // ========================================================================
   // This implementation of a Configuration reads properties
   // from a legacy Windows initialization (.ini) file.
   //
   // The file syntax is implemented as follows.
   //   - a line starting with a semicolon is treated as a comment and ignored
   //   - a line starting with a square bracket denotes a section key [<key>]
   //   - every other line denotes a property assignment in the form
   //     <value key> = <value>
   //
   // The name of a property is composed of the section key and the value key,
   // separated by a period (<section key>.<value key>).
   //
   // Property names are not case sensitive. Leading and trailing whitespace is
   // removed from both keys and values
   // ========================================================================

   bool loadINI();

   // ========================================================================
   // This implementation of a Configuration reads properties
   // from a Java-style properties file.
   //
   // The file syntax is implemented as follows.
   //   - a line starting with a hash '#' or exclamation mark '!' is treated as
   //     a comment and ignored
   //   - every other line denotes a property assignment in the form
   //     <key> = <value> or
   //     <key> : <value>
   //
   // Property names are case sensitive. Leading and trailing whitespace is
   // removed from both keys and values. A property name can neither contain
   // a colon ':' nor an equal sign '=' character
   // ========================================================================

          bool loadProperties();
   static bool loadProperties(UHashMap<UString>& table, const char* start, const char* end);

   static bool loadProperties(UHashMap<UString>& table, const UString& content) { return loadProperties(table, U_STRING_TO_RANGE(content)); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString data;
   const char* _end;
   const char* _start;
   uint32_t _size;
   bool preprocessing;

   void load();
   bool loadTable(UHashMap<UString>& table);

private:
   void init() U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UFileConfig)

   friend class USSIPlugIn;
   friend class UServer_Base;
   friend class UClient_Base;
   friend class UModProxyService;
};

#endif
