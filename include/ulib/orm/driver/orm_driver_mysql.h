// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver_mysql.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_ORM_DRIVER_MYSQL_H
#define U_ORM_DRIVER_MYSQL_H 1

#include <ulib/orm/orm_driver.h>

extern "C" {
#include <mysql/mysql.h>
}

class U_EXPORT UMySqlStatementBindParam : public USqlStatementBindParam {
public:

   UMySqlStatementBindParam()
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "", 0)

      type        = MYSQL_TYPE_NULL;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(bool* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_TINY;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(char* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_TINY;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(unsigned char* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_TINY;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindParam(short* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_SHORT;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(unsigned short* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_SHORT;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindParam(int* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_LONG;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(unsigned int* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_LONG;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindParam(long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

#  if SIZEOF_LONG == 4
      type        = MYSQL_TYPE_LONG;
#  else
      type        = MYSQL_TYPE_LONGLONG;
#  endif
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(unsigned long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

#  if SIZEOF_LONG == 4
      type        = MYSQL_TYPE_LONG;
#  else
      type        = MYSQL_TYPE_LONGLONG;
#  endif
      is_unsigned = true;
      }

   explicit UMySqlStatementBindParam(long long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_LONGLONG;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(unsigned long long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_LONGLONG;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindParam(float* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_FLOAT;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(double* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_DOUBLE;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(long double* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%p", v)

      type        = MYSQL_TYPE_DOUBLE;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindParam(const char* s, int n, bool bstatic) : USqlStatementBindParam(s, n, bstatic)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindParam, "%.*S,%u,%b", n, s, n, bstatic)

      type = MYSQL_TYPE_STRING;
      }

   virtual ~UMySqlStatementBindParam()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMySqlStatementBindParam)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USqlStatementBindParam::dump(reset); }
#endif
};

class U_EXPORT UMySqlStatementBindResult : public USqlStatementBindResult {
public:

   explicit UMySqlStatementBindResult(bool* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_TINY;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(char* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_TINY;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(unsigned char* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_TINY;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindResult(short* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_SHORT;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(unsigned short* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_SHORT;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindResult(int* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_LONG;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(unsigned int* v) : USqlStatementBindResult(v) 
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_LONG;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindResult(long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

#  if SIZEOF_LONG == 4
      type        = MYSQL_TYPE_LONG;
#  else
      type        = MYSQL_TYPE_LONGLONG;
#  endif
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(unsigned long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

#  if SIZEOF_LONG == 4
      type        = MYSQL_TYPE_LONG;
#  else
      type        = MYSQL_TYPE_LONGLONG;
#  endif
      is_unsigned = true;
      }

   explicit UMySqlStatementBindResult(long long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_LONGLONG;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(unsigned long long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_LONGLONG;
      is_unsigned = true;
      }

   explicit UMySqlStatementBindResult(float* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_FLOAT;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(double* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_DOUBLE;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(long double* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%p", v)

      type        = MYSQL_TYPE_DOUBLE;
      is_unsigned = false;
      }

   explicit UMySqlStatementBindResult(UStringRep& s) : USqlStatementBindResult(s)
      {
      U_TRACE_REGISTER_OBJECT(0, UMySqlStatementBindResult, "%V", &s)

      type = MYSQL_TYPE_STRING;
      }

   virtual ~UMySqlStatementBindResult()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMySqlStatementBindResult)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USqlStatementBindResult::dump(reset); }
#endif
};

class U_EXPORT UMySqlStatement : public USqlStatement {
public:

            UMySqlStatement(MYSQL_STMT* ptr, uint32_t nbind, uint32_t nresult) : USqlStatement(ptr, nbind, nresult) { string_type = 0; mysql_vparam = mysql_vresult = 0; }
   virtual ~UMySqlStatement()                                                                                       { reset(); }

   // SERVICES

   void reset();
   void setStringBindedAsResult();

   bool setBindParam( UOrmDriver* pdrv);
   bool setBindResult(UOrmDriver* pdrv);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   int string_type;
   MYSQL_BIND* mysql_vparam;
   MYSQL_BIND* mysql_vresult;
};

class U_EXPORT UOrmDriverMySql : public UOrmDriver {
public:

   UOrmDriverMySql()
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriverMySql, "")

      U_INTERNAL_ASSERT_POINTER(UString::str_mysql_name)

      UOrmDriver::name = *UString::str_mysql_name;
      }

   UOrmDriverMySql(const UString& name_drv) : UOrmDriver(name_drv)
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriverMySql, "%V", name_drv.rep)
      }

   virtual ~UOrmDriverMySql();

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
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam()); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(int* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(bool* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(char* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(short* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(float* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(double* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long long* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long double* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  int* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  char* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  long* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  short* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned long long* v)
      { UMySqlStatementBindParam* r; U_NEW(UMySqlStatementBindParam, r, UMySqlStatementBindParam(v)); return r; }

   virtual USqlStatementBindParam* creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind);

   // CREATE BIND RESULT

   virtual USqlStatementBindResult* creatSqlStatementBindResult(int* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(bool* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(char* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(short* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(float* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(double* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(UStringRep& v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long long* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long double* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned char* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned short* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned int* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long long* v)
      { UMySqlStatementBindResult* r; U_NEW(UMySqlStatementBindResult, r, UMySqlStatementBindResult(v)); return r; }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UOrmDriverMySql)

   friend class UMySqlStatement;
};

#endif
