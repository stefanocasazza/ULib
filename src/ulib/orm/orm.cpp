// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/orm/orm.h>
#include <ulib/orm/orm_driver.h>

#if defined(U_STDCPP_ENABLE) && !defined(HAVE_OLD_IOSTREAM)
#  include <sstream>
#endif

__noreturn U_NO_EXPORT void UOrmSession::loadDriverFail(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UOrmSession::loadDriverFail(%.*S,%u)", len, ptr, len)

   U_WARNING("Load of ORM driver failed: %.*S", len, ptr);

   U_EXIT(EXIT_FAILURE);   
}

void UOrmSession::loadDriver(const char* backend, uint32_t len, const UString& option)
{
   U_TRACE(0, "UOrmSession::loadDriver(%.*S,%u,%V)", len, backend, len, option.rep)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::vdriver)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::vdriver_name)

   if (len == 0) U_ERROR("ORM driver not defined");

   uint32_t n = UOrmDriver::vdriver_name->find(backend, len);

   if (n != U_NOT_FOUND) pdrv = (*UOrmDriver::vdriver)[n];
   else
      {
      UString name(32U);

      name.snprintf(U_CONSTANT_TO_PARAM("orm_driver_%.*s"), len, backend);

      U_INTERNAL_ASSERT_POINTER(UOrmDriver::driver_dir)

      if (*UOrmDriver::driver_dir) UDynamic::setPluginDirectory(*UOrmDriver::driver_dir);

      pdrv = UPlugIn<UOrmDriver*>::create(U_STRING_TO_PARAM(name));

      if (pdrv == U_NULLPTR) goto err;

      UString _name(backend, len);

      UOrmDriver::vdriver->push(pdrv);
      UOrmDriver::vdriver_name->push_back(_name);
      }

   U_INTERNAL_ASSERT_POINTER(pdrv)

   if ((pdrv->connection == U_NULLPTR ||
        pdrv->opt != option)          &&
       connect(option) == false)
      {
err:  loadDriverFail(backend, len);
      }
}

UOrmSession::UOrmSession(const char* dbname, uint32_t len)
{
   U_TRACE_REGISTER_OBJECT(0, UOrmSession, "%.*S,%u", len, dbname, len)

   pdrv = U_NULLPTR;

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_DUMP("UOrmDriver::env_driver_len = %u", UOrmDriver::env_driver_len)

   if (UOrmDriver::env_driver_len == 0 &&
       UOrmDriver::loadDriver() == false)
      {
      U_ERROR("ORM drivers load failed");
      }

   if (UOrmDriver::env_driver_len)
      {
      if (UOrmDriver::env_option == U_NULLPTR) U_ERROR("The environment var ORM_OPTION is empty");

      UString option(200U);

      option.snprintf(UOrmDriver::env_option, strlen(UOrmDriver::env_option), len, dbname);

      loadDriver(UOrmDriver::env_driver, UOrmDriver::env_driver_len, option);
      }
#endif
}

UOrmSession::~UOrmSession()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOrmSession)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_DUMP("pdrv = %p", pdrv)

   if (pdrv)
      {
      pdrv->handlerDisConnect();

      if (UOrmDriver::vdriver->find(pdrv) != U_NOT_FOUND) pdrv->vopt.clear();
      else
         {
         delete pdrv;
                pdrv = U_NULLPTR;
         }
      }
#endif
}

__pure bool UOrmSession::isReady() const
{
   U_TRACE_NO_PARAM(0, "UOrmSession::isReady()")

   if (pdrv != U_NULLPTR &&
       UOrmDriver::env_driver_len)
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UOrmSession::connect(const UString& option)
{
   U_TRACE(0, "UOrmSession::connect(%V)", option.rep)

   U_INTERNAL_ASSERT_POINTER(pdrv)

   if ((pdrv = pdrv->handlerConnect(option))) U_RETURN(true);

   U_RETURN(false);
}

// will be typecast into conn-specific type

__pure void* UOrmSession::getConnection()
{
   U_TRACE_NO_PARAM(0, "UOrmSession::getConnection()")

   U_INTERNAL_ASSERT_POINTER(pdrv)

   U_RETURN_POINTER(pdrv->connection, void);
}

// statement that should only be executed once and immediately

bool UOrmSession::query(const char* stmt, uint32_t len)
{
   U_TRACE(0, "UOrmSession::query(%.*S,%u)", len, stmt, len)

   U_INTERNAL_ASSERT_POINTER(pdrv)

   if (pdrv->handlerQuery(stmt, len)) U_RETURN(true);

   U_RETURN(false);
}

// This function returns the number of database rows that were changed
// or inserted or deleted by the most recently completed SQL statement

unsigned long long UOrmSession::affected()
{
   U_TRACE_NO_PARAM(0, "UOrmSession::affected()")

   U_INTERNAL_ASSERT_POINTER(pdrv)

   unsigned long long result = pdrv->affected(U_NULLPTR);

   U_RETURN(result);
}

// This routine returns the rowid of the most recent successful INSERT into the database

unsigned long long UOrmSession::last_insert_rowid(const char* sequence)
{
   U_TRACE(0, "UOrmSession::last_insert_rowid(%S)", sequence)

   U_INTERNAL_ASSERT_POINTER(pdrv)

   unsigned long long result = pdrv->last_insert_rowid(U_NULLPTR, sequence);

   U_RETURN(result);
}

// creat the SQL statement string with some placeholder (?) 

UOrmStatement::UOrmStatement(UOrmSession& session, const char* stmt, uint32_t len)
{
   U_TRACE_REGISTER_OBJECT(0, UOrmStatement, "%p,%.*S,%u", &session, len, stmt, len)

   pdrv     = U_NULLPTR;
   pstmt    = U_NULLPTR;
   psession = U_NULLPTR;

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   if (session.pdrv)
      {
      pdrv  = (psession = &session)->pdrv;
      pstmt = pdrv->handlerStatementCreation(stmt, len);

      U_INTERNAL_DUMP("psession = %p pdrv = %p pstmt = %p", psession, pdrv, pstmt)

      if (pstmt) return;
      }

   if (UOrmDriver::env_driver_len) UOrmSession::loadDriverFail(UOrmDriver::env_driver, UOrmDriver::env_driver_len);
#endif
}

UOrmStatement::~UOrmStatement()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOrmStatement)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_DUMP("pstmt = %p", pstmt)

   if (pstmt)
      {
      U_INTERNAL_DUMP("psession = %p", psession)

      U_INTERNAL_ASSERT_POINTER(psession)

      if (psession->pdrv == U_NULLPTR) delete pstmt;
      else
         {
         U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

         pdrv->remove(pstmt);
         }
      }
#endif
}

void UOrmStatement::execute()
{
   U_TRACE_NO_PARAM(0, "UOrmStatement::execute()")

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->execute(pstmt);
#endif
}

// This function returns the number of database rows that were changed
// or inserted or deleted by the most recently completed SQL statement

unsigned long long UOrmStatement::affected()
{
   U_TRACE_NO_PARAM(0, "UOrmStatement::affected()")

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   unsigned long long result = pdrv->affected(pstmt);

   U_RETURN(result);
}

// This routine returns the rowid of the most recent successful INSERT into the database

unsigned long long UOrmStatement::last_insert_rowid(const char* sequence)
{
   U_TRACE(0, "UOrmStatement::last_insert_rowid(%S)", sequence)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   unsigned long long result = pdrv->last_insert_rowid(pstmt, sequence);

   U_RETURN(result);
}

// Get number of columns in the row

unsigned int UOrmStatement::cols()
{
   U_TRACE_NO_PARAM(0, "UOrmStatement::cols()")

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   unsigned int result = pdrv->cols(pstmt);

   U_RETURN(result);
}

bool UOrmStatement::nextRow()
{
   U_TRACE_NO_PARAM(0, "UOrmStatement::nextRow()")

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   if (pdrv->nextRow(pstmt)) U_RETURN(true);

   U_RETURN(false);
}

// Resets a prepared statement on client and server to state after creation

void UOrmStatement::reset()
{
   U_TRACE_NO_PARAM(0, "UOrmStatement::reset()")

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->reset(pstmt);
}

// BIND PARAM

void UOrmStatement::bindParam()
{
   U_TRACE_NO_PARAM(0, "UOrmStatement::bindParam()")

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt);
#endif
}

void UOrmStatement::bindParam(bool& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%b)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(char& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%C)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(unsigned char& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%C)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(short& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%d)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(unsigned short& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%d)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(int& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%d)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(unsigned int& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%u)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(long& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%ld)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(unsigned long& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%lu)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(long long& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%lld)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(unsigned long long& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%llu)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(float& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%g)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(double& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%g)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(long double& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%g)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v);
#endif
}

void UOrmStatement::bindParam(const char* s, int n, bool bstatic, int rebind)
{
   U_TRACE(0, "UOrmStatement::bindParam(%.*S,%u,%b,%d)", n, s, n, bstatic, rebind)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, s, n, bstatic, rebind);
#endif
}

void UOrmStatement::bindParam(const char* v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%S)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, v, u__strlen(v, __PRETTY_FUNCTION__), true, -1);
#endif
}

void UOrmStatement::bindParam(const char* b, const char* e)
{
   U_TRACE(0, "UOrmStatement::bindParam(%S,%S)", b, e)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, b, e-b, false, -1);
#endif
}

void UOrmStatement::bindParam(UStringRep& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%V)", &v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindParam(pstmt, U_STRING_TO_PARAM(v), true, -1);
#endif
}

void UOrmStatement::bindParam(struct tm& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%p)", &v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   char buffer[32];

   buffer[0]     = 0;
   u_strftime_tm = v;

   uint32_t n = u_strftime1(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%Y-%m-%d %T"));

   pdrv->bindParam(pstmt, buffer, n, false, -1);
#endif
}

#ifdef U_STDCPP_ENABLE
void UOrmStatement::bindParam(istream& v)
{
   U_TRACE(0, "UOrmStatement::bindParam(%p)", &v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

# ifndef HAVE_OLD_IOSTREAM
   ostringstream ss;

   ss << v.rdbuf();

   string s = ss.str();

   pdrv->bindParam(pstmt, s.c_str(), s.size(), false, -1);
# endif
#endif
}
#endif

// BIND RESULT

void UOrmStatement::bindResult(bool& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%b)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(char& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%C)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(unsigned char& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%C)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(short& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%d)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(unsigned short& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%d)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(int& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%d)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(unsigned int& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%u)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(long& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%ld)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(unsigned long& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%lu)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(long long& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%lld)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(unsigned long long& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%llu)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(float& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%g)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(double& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%g)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(long double& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%g)", v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

void UOrmStatement::bindResult(UStringRep& v)
{
   U_TRACE(0, "UOrmStatement::bindResult(%V)", &v)

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(psession->pdrv)
   U_INTERNAL_ASSERT_EQUALS(pdrv, psession->pdrv)

   pdrv->bindResult(pstmt, v);
#endif
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UOrmSession::dump(bool _reset) const
{
   *UObjectIO::os << "pdrv (UOrmDriver " << (void*)pdrv << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UOrmStatement::dump(bool _reset) const
{
   *UObjectIO::os << "pdrv     (UOrmDriver    " << (void*)pdrv     << ")\n"
                  << "pstmt    (USqlStatement " << (void*)pstmt    << ")\n"
                  << "psession (UOrmSession   " << (void*)psession << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UOrmTypeHandler_Base::dump(bool _reset) const
{
   *UObjectIO::os << "pval " << pval;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
