// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver.cpp - ORM driver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/orm/orm_driver.h>
#include <ulib/net/server/server.h>

#ifdef U_STATIC_ORM_DRIVER_SQLITE
#  include <ulib/orm/driver/orm_driver_sqlite.h>
#endif
#ifdef U_STATIC_ORM_DRIVER_MYSQL
#  include <ulib/orm/driver/orm_driver_mysql.h>
#endif
#ifdef U_STATIC_ORM_DRIVER_PGSQL
#  include <ulib/orm/driver/orm_driver_pgsql.h>
#endif

bool                  UOrmDriver::bexit;
uint32_t              UOrmDriver::vdriver_size;
uint32_t              UOrmDriver::env_driver_len;
UString*              UOrmDriver::driver_dir;
const char*           UOrmDriver::env_driver;
const char*           UOrmDriver::env_option;
UVector<UString>*     UOrmDriver::vdriver_name;
UVector<UString>*     UOrmDriver::vdriver_name_static;
UVector<UOrmDriver*>* UOrmDriver::vdriver;

UOrmDriver::~UOrmDriver()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOrmDriver)
}

void UOrmDriver::clear()
{
   U_TRACE_NO_PARAM(0, "UOrmDriver::clear()")

   if (driver_dir) delete driver_dir;

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   if (vdriver)
      {
      delete vdriver;
      delete vdriver_name;
      }
#endif
}

// load driver modules

U_NO_EXPORT void UOrmDriver::loadStaticLinkedModules(const char* name)
{
   U_TRACE(0, "UOrmDriver::loadStaticLinkedModules(%S)", name)

#if defined(U_STATIC_ORM_DRIVER_SQLITE) || defined(U_STATIC_ORM_DRIVER_MYSQL) || defined(U_STATIC_ORM_DRIVER_PGSQL)
   UString x(name);
   UOrmDriver* _driver = 0;

# ifdef U_STATIC_ORM_DRIVER_SQLITE
   if (x.equal(U_CONSTANT_TO_PARAM("sqlite"))) { U_NEW(UOrmDriverSqlite, _driver, UOrmDriverSqlite);  goto next; }
# endif
# ifdef U_STATIC_ORM_DRIVER_MYSQL
   if (x.equal(U_CONSTANT_TO_PARAM("mysql")))  { U_NEW(UOrmDriverMySql, _driver, UOrmDriverMySql); goto next; }
# endif
# ifdef U_STATIC_ORM_DRIVER_PGSQL
   if (x.equal(U_CONSTANT_TO_PARAM("pgsql")))  { U_NEW(UOrmDriverPgSql, _driver, UOrmDriverPgSql); goto next; }
# endif
next:
   if (_driver)
      {
      vdriver->push_back(_driver);
      vdriver_name_static->push_back(x);

#  ifndef U_LOG_DISABLE
      if (UServer_Base::isLog()) ULog::log(U_CONSTANT_TO_PARAM("[%s] Link of static driver ok"), name);
#  endif
      }
#endif
}

void UOrmDriver::setDriverDirectory(const UString& dir)
{
   U_TRACE(0, "UOrmDriver::setDriverDirectory(%V)", dir.rep)

   U_INTERNAL_ASSERT_EQUALS(driver_dir, 0)

   U_NEW(UString, driver_dir, UString);

   // NB: we can't use relativ path because after we call chdir()...

   if (dir.first_char() != '.') (void) driver_dir->replace(dir);
   else
      {
      U_INTERNAL_ASSERT(dir.isNullTerminated())

      *driver_dir = UFile::getRealPath(dir.data());
      }
}

bool UOrmDriver::loadDriver(const UString& dir, const UString& driver_list)
{
   U_TRACE(0, "UOrmDriver::loadDriver(%V,%V)", dir.rep, driver_list.rep)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   if (vdriver) U_RETURN(true);

   if (dir) setDriverDirectory(dir);

   if (driver_dir) UDynamic::setPluginDirectory(*driver_dir);

   UString::str_allocate(STR_ALLOCATE_ORM);

   U_NEW(UVector<UOrmDriver*>, vdriver, UVector<UOrmDriver*>(10U));

   U_NEW(UVector<UString>, vdriver_name,        UVector<UString>(10U));
   U_NEW(UVector<UString>, vdriver_name_static, UVector<UString>(20U));

   uint32_t i, n;
   UOrmDriver* _driver;
   UString item, _name;

   // NB: we don't want to use substr() because of dependency from config var ORM_DRIVER...

   if (driver_list) vdriver_size = vdriver_name->split(U_STRING_TO_PARAM(driver_list));

   /**
    * I do know that to include code in the middle of a function is hacky and dirty,
    * but this is the best solution that I could figure out. If you have some idea to
    * clean it up, please, don't hesitate and let me know
    */

#  include "driver/loader.autoconf.cpp"

   for (i = 0; i < vdriver_size; ++i)
      {
      item = vdriver_name->at(i);

      uint32_t pos = vdriver_name_static->find(item);

      U_INTERNAL_DUMP("i = %u pos = %u item = %V", i, pos, item.rep)

      if (pos == U_NOT_FOUND)
         {
         _name.setBuffer(32U);

         _name.snprintf(U_CONSTANT_TO_PARAM("orm_driver_%v"), item.rep);

         _driver = UPlugIn<UOrmDriver*>::create(U_STRING_TO_PARAM(_name));

         if (_driver == 0)
            {
            U_SRV_LOG("WARNING: load of driver %v failed", item.rep);

            continue;
            }

         vdriver->push_back(_driver);
         vdriver_name_static->push_back(item);

#     ifndef U_LOG_DISABLE
         if (UServer_Base::isLog()) ULog::log(U_CONSTANT_TO_PARAM("[%v] Load of driver success"), item.rep);
#     endif
         }
      }

   if (vdriver_size && vdriver->empty()) U_RETURN(false);

   vdriver_name->clear();

   for (i = 0, n = vdriver_name_static->size(); i < n; ++i)
      {
      item = vdriver_name_static->at(i);

      vdriver_name->push_back(item);
      }

   delete vdriver_name_static;

   env_driver = (const char*) U_SYSCALL(getenv, "%S", "ORM_DRIVER");
   env_option = (const char*) U_SYSCALL(getenv, "%S", "ORM_OPTION");

   if (env_driver) env_driver_len = u__strlen(env_driver, __PRETTY_FUNCTION__);
#endif

   U_RETURN(true);
}

void UOrmDriver::printError(const char* function)
{
   U_TRACE(0, "UOrmDriver::printError(%S)", function)

   handlerError();

   const char* ptr1 = (errname  == 0 ? (errname = "")
                                     : " ");
   const char* ptr2 = (SQLSTATE == 0 ? (SQLSTATE = "")
                                     : " - SQLSTATE: ");

   char buffer[4096];

   buffer[0] = 0;

   uint32_t len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%V on %V at %S - %s%s(%d, %s)%s%s"),
                              name.rep, dbname.rep, function, errname, ptr1, errcode, errmsg, ptr2, SQLSTATE);

   if (UOrmDriver::bexit == false)
      {
      errmsg   = errname = 0;
      errcode  = 0;
      SQLSTATE = 0;

      U_WARNING("ORM: %.*s", len, buffer);

      return;
      }

   U_ERROR("ORM: %.*s", len, buffer);
}

bool UOrmDriver::setOption(const UString& option)
{
   U_TRACE(0, "UOrmDriver::setOption(%V)", option.rep)

   // ---------------------------------------------------------------------------------------------
   // option string format:
   // ---------------------------------------------------------------------------------------------
   // <str> == <assignment> | <assignment> ';' <str>
   // <assignment> == <name> '=' <value>
   // <name> == 'host' | 'port' | 'user' | 'password' | 'db' | 'compress' | 'auto-reconnect' | ...
   // <value> == [~;]*
   // ---------------------------------------------------------------------------------------------

   U_ASSERT(vopt.empty())

   opt = option;

   uint32_t n = vopt.split(opt, "=; ");

   if (n == 0)
      {
      errmsg  = "wrong option string format";
      errcode = -1;

      printError(__PRETTY_FUNCTION__);

      U_RETURN(false);
      }

   if (n == 1) dbname = vopt[0];
   else
      {
      n = vopt.find(*UString::str_dbname, false);

      if (n != U_NOT_FOUND) dbname = vopt[n+1];
      }

   if (dbname)
      {
      dbname.setNullTerminated();

      U_RETURN(true);
      }

   errmsg  = "no database specified";
   errcode = -1;

   printError(__PRETTY_FUNCTION__);

   U_RETURN(false);
}

UString UOrmDriver::getOptionValue(const char* _name, uint32_t len)
{
   U_TRACE(0, "UOrmDriver::getOptionValue(%*S,%u)", len, _name, len)

   if (vopt.size() == 1 &&
       UString::str_dbname->equal(_name, len))
      {
      U_RETURN_STRING(dbname);
      }

   uint32_t n = vopt.find(_name, len);

   if (n != U_NOT_FOUND)
      {
      UString result = vopt[n+1];

      U_RETURN_STRING(result);
      }

   return UString::getStringNull();
}

USqlStatementBindParam::USqlStatementBindParam(const char* s, int n, bool bstatic)
{
   U_TRACE_REGISTER_OBJECT(0, USqlStatementBindParam, "%.*S,%u,%b", n, s, n, bstatic)

   type        = 0;
   length      = n;
   is_unsigned = false;

   if (bstatic)
      {
      buffer = (void*)s;
      pstr   = 0;
      }
   else
      {
      U_NEW(UString, pstr, UString((void*)s, n));

      buffer = pstr->data();
      }
}

USqlStatementBindParam* UOrmDriver::creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind)
{
   U_TRACE(0, "UOrmDriver::creatSqlStatementBindParam(%p,%.*S,%u,%b,%d)", pstmt, n, s, n, bstatic, rebind)

   U_INTERNAL_ASSERT(rebind >= 0)
   U_INTERNAL_ASSERT_POINTER(pstmt)

   USqlStatementBindParam* param = pstmt->vparam[rebind];

   param->length = n;

   if (param->pstr)
      {
      if (bstatic == false)
         {
         (void) param->pstr->replace(s, n);

         param->buffer = param->pstr->data();

         U_RETURN_POINTER(0, USqlStatementBindParam);
         }

      bstatic = false;

      delete param->pstr;
      }

   if (bstatic) param->buffer = (void*)s;
   else
      {
      U_NEW(UString, param->pstr, UString((void*)s, n));

      param->buffer = param->pstr->data();
      }

   U_RETURN_POINTER(param, USqlStatementBindParam);
}

// BIND PARAM

template <> void UOrmDriver::bindParam<int>(USqlStatement* pstmt, int& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<int>(%p,%d)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<bool>(USqlStatement* pstmt, bool& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<bool>(%p,%b)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<char>(USqlStatement* pstmt, char& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<char>(%p,%C)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<long>(USqlStatement* pstmt, long& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<long>(%p,%ld)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<short>(USqlStatement* pstmt, short& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<short>(%p,%d)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<float>(USqlStatement* pstmt, float& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<float>(%p,%g)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<double>(USqlStatement* pstmt, double& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<double>(%p,%g)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<long long>(USqlStatement* pstmt, long long& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<long long>(%p,%lld)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<long double>(USqlStatement* pstmt, long double& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<long double>(%p,%g)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<unsigned int>(USqlStatement* pstmt, unsigned int& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<unsigned int>(%p,%u)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<unsigned char>(USqlStatement* pstmt, unsigned char& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<unsigned char>(%p,%C)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<unsigned long>(USqlStatement* pstmt, unsigned long& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<unsigned long>(%p,%lu)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<unsigned short>(USqlStatement* pstmt, unsigned short& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<unsigned short>(%p,%d)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

template <> void UOrmDriver::bindParam<unsigned long long>(USqlStatement* pstmt, unsigned long long& v)
{
   U_TRACE(0, "UOrmDriver::bindParam<unsigned long long>(%p,%llu)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindParam(creatSqlStatementBindParam(&v));
}

// BIND RESULT

template <> void UOrmDriver::bindResult<int>(USqlStatement* pstmt, int& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<int>(%p,%d)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<bool>(USqlStatement* pstmt, bool& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<bool>(%p,%b)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<char>(USqlStatement* pstmt, char& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<char>(%p,%C)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<long>(USqlStatement* pstmt, long& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<long>(%p,%ld)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<short>(USqlStatement* pstmt, short& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<short>(%p,%d)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<float>(USqlStatement* pstmt, float& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<float>(%p,%g)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<double>(USqlStatement* pstmt, double& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<double>(%p,%g)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<long long>(USqlStatement* pstmt, long long& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<long long>(%p,%lld)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<long double>(USqlStatement* pstmt, long double& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<long double>(%p,%g)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<unsigned int>(USqlStatement* pstmt, unsigned int& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<unsigned int>(%p,%u)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<unsigned char>(USqlStatement* pstmt, unsigned char& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<unsigned char>(%p,%C)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<unsigned long>(USqlStatement* pstmt, unsigned long& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<unsigned long>(%p,%lu)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<unsigned short>(USqlStatement* pstmt, unsigned short& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<unsigned short>(%p,%d)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<unsigned long long>(USqlStatement* pstmt, unsigned long long& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<unsigned long long>(%p,%llu)", pstmt, v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   pstmt->bindResult(creatSqlStatementBindResult(&v));
}

template <> void UOrmDriver::bindResult<UStringRep>(USqlStatement* pstmt, UStringRep& v)
{
   U_TRACE(0, "UOrmDriver::bindResult<UString>(%p,%V)", pstmt, &v)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   USqlStatementBindResult* ptr = creatSqlStatementBindResult(v);

   pstmt->bindResult(ptr);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USqlStatementBindParam::dump(bool _reset) const
{
   *UObjectIO::os << "type          " << type        << '\n'
                  << "buffer        " << buffer      << '\n'
                  << "length        " << length      << '\n'
                  << "is_unsigned   " << is_unsigned << '\n'
                  << "pstr (UString " << (void*)pstr << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* USqlStatementBindResult::dump(bool _reset) const
{
   *UObjectIO::os << "type             " << type        << '\n'
                  << "buffer           " << buffer      << '\n'
                  << "length           " << length      << '\n'
                  << "is_null          " << is_null     << '\n'
                  << "is_unsigned      " << is_unsigned << '\n'
                  << "pstr (UStringRep " << (void*)pstr << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* USqlStatement::dump(bool _reset) const
{
   *UObjectIO::os << "pHandle                                    " << pHandle         << '\n'
                  << "current_row                                " << current_row     << '\n'
                  << "num_row_result                             " << num_row_result  << '\n'
                  << "num_bind_param                             " << num_bind_param  << '\n'
                  << "num_bind_result                            " << num_bind_result << '\n'
                  << "vparam  (UVector<USqlStatementBindParam*>  " << (void*)&vparam  << ")\n"
                  << "vresult (UVector<USqlStatementBindResult*> " << (void*)&vresult << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UOrmDriver::dump(bool _reset) const
{
   *UObjectIO::os << "errmsg                   " << (void*)errmsg     << '\n'
                  << "errcode                  " << errcode           << '\n'
                  << "connection               " << (void*)connection << '\n'
                  << "opt    (UString          " << (void*)&opt       << ")\n"
                  << "name   (UString          " << (void*)&name      << ")\n"
                  << "dbname (UString          " << (void*)&dbname    << ")\n"
                  << "vopt   (UVector<UString> " << (void*)&vopt      << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
