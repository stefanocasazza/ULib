// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver_sqlite.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_ORM_DRIVER_SQLITE_H
#define U_ORM_DRIVER_SQLITE_H 1

#include <ulib/orm/orm_driver.h>

class U_EXPORT USqliteStatementBindParam : public USqlStatementBindParam {
public:

   USqliteStatementBindParam()
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "", 0)

      type = NULL_VALUE;
      }

   explicit USqliteStatementBindParam(bool* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = BOOLEAN_VALUE;
      }

   explicit USqliteStatementBindParam(char* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = CHAR_VALUE;
      }

   explicit USqliteStatementBindParam(unsigned char* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = CHAR_VALUE;
      }

   explicit USqliteStatementBindParam(short* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = SHORT_VALUE;
      }

   explicit USqliteStatementBindParam(unsigned short* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = SHORT_VALUE;
      }

   explicit USqliteStatementBindParam(int* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = INT_VALUE;
      }

   explicit USqliteStatementBindParam(unsigned int* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = INT_VALUE;
      }

   explicit USqliteStatementBindParam(long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

#  if SIZEOF_LONG == 4
      type = INT_VALUE;
#  else
      type = LLONG_VALUE;
#  endif
      }

   explicit USqliteStatementBindParam(unsigned long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

#  if SIZEOF_LONG == 4
      type = INT_VALUE;
#  else
      type = LLONG_VALUE;
#  endif
      }

   explicit USqliteStatementBindParam(long long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = LLONG_VALUE;
      }

   explicit USqliteStatementBindParam(unsigned long long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = LLONG_VALUE;
      }

   explicit USqliteStatementBindParam(float* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = FLOAT_VALUE;
      }

   explicit USqliteStatementBindParam(double* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = REAL_VALUE;
      }

   explicit USqliteStatementBindParam(long double* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%p", v)

      type = REAL_VALUE;
      }

   explicit USqliteStatementBindParam(const char* s, int n, bool bstatic) : USqlStatementBindParam(s, n, bstatic)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindParam, "%.*S,%u,%b", n, s, n, bstatic)

      type = STRING_VALUE;
      }

   virtual ~USqliteStatementBindParam()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USqliteStatementBindParam)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USqlStatementBindParam::dump(reset); }
#endif
};

class U_EXPORT USqliteStatementBindResult : public USqlStatementBindResult {
public:

   explicit USqliteStatementBindResult(bool* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = BOOLEAN_VALUE;
      }

   explicit USqliteStatementBindResult(char* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = CHAR_VALUE;
      }

   explicit USqliteStatementBindResult(unsigned char* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = CHAR_VALUE;
      }

   explicit USqliteStatementBindResult(short* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = SHORT_VALUE;
      }

   explicit USqliteStatementBindResult(unsigned short* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = SHORT_VALUE;
      }

   explicit USqliteStatementBindResult(int* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = INT_VALUE;
      }

   explicit USqliteStatementBindResult(unsigned int* v) : USqlStatementBindResult(v) 
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = INT_VALUE;
      }

   explicit USqliteStatementBindResult(long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

#  if SIZEOF_LONG == 4
      type = INT_VALUE;
#  else
      type = LLONG_VALUE;
#  endif
      }

   explicit USqliteStatementBindResult(unsigned long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

#  if SIZEOF_LONG == 4
      type = INT_VALUE;
#  else
      type = LLONG_VALUE;
#  endif
      }

   explicit USqliteStatementBindResult(long long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = LLONG_VALUE;
      }

   explicit USqliteStatementBindResult(unsigned long long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = LLONG_VALUE;
      }

   explicit USqliteStatementBindResult(float* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = FLOAT_VALUE;
      }

   explicit USqliteStatementBindResult(double* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = REAL_VALUE;
      }

   explicit USqliteStatementBindResult(long double* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%p", v)

      type = REAL_VALUE;
      }

   explicit USqliteStatementBindResult(UStringRep& s) : USqlStatementBindResult(s)
      {
      U_TRACE_REGISTER_OBJECT(0, USqliteStatementBindResult, "%V", &s)

      type = STRING_VALUE;
      }

   virtual ~USqliteStatementBindResult()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USqliteStatementBindResult)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USqlStatementBindResult::dump(reset); }
#endif
};

struct sqlite3_stmt;

class U_EXPORT USqliteStatement : public USqlStatement {
public:

            USqliteStatement(sqlite3_stmt* ptr, uint32_t nbind, uint32_t nresult) : USqlStatement(ptr, nbind, nresult) { param_binded = false; }
   virtual ~USqliteStatement()                                                                                         { USqlStatement::reset(); }

   // SERVICES

   void reset()
      {
      U_TRACE_NO_PARAM(0, "USqliteStatement::reset()")

      U_ASSERT_EQUALS(num_bind_param,  vparam.size())
      U_ASSERT_EQUALS(num_bind_result, vresult.size())

      param_binded = false;
      }

   bool setBindParam( UOrmDriver* pdrv);
   bool setBindResult(UOrmDriver* pdrv);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   bool param_binded;
};

/**
 * SQLite is a smart library that implements an embeddable SQL database engine. No need for an external database server
 * an application linked against libsqlite can do it all by itself. Of course there are a few limitations of this approach
 * compared to "real" SQL database servers, mostly for massively parallel high-throughput database applications, but on the
 * other hand, installation and administration are a breeze. 
 *
 * SQLite3 is a redesign of SQLite which is incompatible with the older 2.x versions.
 * Your application should support the sqlite3 driver if one of the following applies:
 *
 * You want to offer the simplest possible installation of your application.
 * You want to support potential users of your application who don't have the skills to administer a database server.
 * You want to let users test-drive your application without the need to fiddle with their production database servers
 */
 
class U_EXPORT UOrmDriverSqlite : public UOrmDriver {
public:

   UOrmDriverSqlite()
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriverSqlite, "")

      U_INTERNAL_ASSERT_POINTER(UString::str_sqlite_name)

      encoding_UTF16 = false;

      UOrmDriver::name = *UString::str_sqlite_name;
      }

   UOrmDriverSqlite(const UString& name_drv) : UOrmDriver(name_drv)
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriverSqlite, "%V", name_drv.rep)

      encoding_UTF16 = false;
      }

   virtual ~UOrmDriverSqlite();

   // define method VIRTUAL of class UOrmDriver

   virtual void handlerError() U_DECL_FINAL;
   virtual void handlerDisConnect() U_DECL_FINAL;
   virtual void execute(USqlStatement* pstmt) U_DECL_FINAL;
   virtual bool nextRow(USqlStatement* pstmt) U_DECL_FINAL;
   virtual void handlerStatementReset(USqlStatement* pstmt) U_DECL_FINAL;
   virtual void handlerStatementRemove(USqlStatement* pstmt) U_DECL_FINAL;
   virtual bool handlerQuery(const char* query, uint32_t query_len) U_DECL_FINAL;

   virtual unsigned int cols(USqlStatement* pstmt) U_DECL_FINAL;
   virtual unsigned long long affected(USqlStatement* pstmt) U_DECL_FINAL;
   virtual unsigned long long last_insert_rowid(USqlStatement* pstmt, const char* sequence) U_DECL_FINAL;

   virtual UOrmDriver*    handlerConnect(const UString& option) U_DECL_FINAL;
   virtual USqlStatement* handlerStatementCreation(const char* stmt, uint32_t len) U_DECL_FINAL;

   // CREATE BIND PARAM

   virtual USqlStatementBindParam* creatSqlStatementBindParam()
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam()); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(int* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(bool* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(char* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(short* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(float* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(double* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long long* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long double* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  int* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  char* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  long* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  short* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned long long* v)
      { USqliteStatementBindParam* r; U_NEW(USqliteStatementBindParam, r, USqliteStatementBindParam(v)); return r; }

   virtual USqlStatementBindParam* creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind);

   // CREATE BIND RESULT

   virtual USqlStatementBindResult* creatSqlStatementBindResult(int* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(bool* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(char* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(short* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(float* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(double* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(UStringRep& v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long long* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long double* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned char* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned short* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned int* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long long* v)
      { USqliteStatementBindResult* r; U_NEW(USqliteStatementBindResult, r, USqliteStatementBindResult(v)); return r; }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   bool encoding_UTF16;

   U_DISALLOW_COPY_AND_ASSIGN(UOrmDriverSqlite)

   friend class USqliteStatement;
};

#endif
