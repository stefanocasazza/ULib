// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver_pgsql.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_ORM_DRIVER_PGSQL_H
#define U_ORM_DRIVER_PGSQL_H 1

#include <ulib/orm/orm_driver.h>

#undef PACKAGE_URL
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT

extern "C" {
#include <postgres_fe.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h>
}

#undef  PACKAGE_NAME
#define PACKAGE_NAME    "ULib"
#undef  PACKAGE_VERSION
#define PACKAGE_VERSION ULIB_VERSION

/**
 * .................
 * #define BOOLOID         16
 * #define BYTEAOID        17
 * #define CHAROID         18
 * #define NAMEOID         19
 * #define INT8OID         20
 * #define INT2OID         21
 * #define INT2VECTOROID   22
 * #define INT4OID         23
 * #define REGPROCOID      24
 * #define TEXTOID         25
 * #define OIDOID          26
 * #define TIDOID          27
 * #define XIDOID          28
 * #define CIDOID          29
 * #define OIDVECTOROID    30
 * #define JSONOID        114
 * #define XMLOID         142
 * #define VARCHAROID    1043
 * .................
 */

class U_EXPORT UPgSqlStatementBindParam : public USqlStatementBindParam {
public:

   UPgSqlStatementBindParam()
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "", 0)

      type   = 0;
      length = 0;
      }

   explicit UPgSqlStatementBindParam(bool* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = BOOLOID;
      length = sizeof(bool);
      }

   explicit UPgSqlStatementBindParam(char* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = CHAROID;
      length = sizeof(char);
      }

   explicit UPgSqlStatementBindParam(unsigned char* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = CHAROID;
      length = sizeof(char);
      }

   explicit UPgSqlStatementBindParam(short* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = INT2OID;
      length = sizeof(short);
      }

   explicit UPgSqlStatementBindParam(unsigned short* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = INT2OID;
      length = sizeof(short);
      }

   explicit UPgSqlStatementBindParam(int* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = INT4OID;
      length = sizeof(int);
      }

   explicit UPgSqlStatementBindParam(unsigned int* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = INT4OID;
      length = sizeof(int);
      }

   explicit UPgSqlStatementBindParam(long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

#  if SIZEOF_LONG == 4
      type   = INT4OID;
#  else
      type   = INT8OID;
#  endif
      length = sizeof(long);
      }

   explicit UPgSqlStatementBindParam(unsigned long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

#  if SIZEOF_LONG == 4
      type   = INT4OID;
#  else
      type   = INT8OID;
#  endif
      length = sizeof(long);
      }

   explicit UPgSqlStatementBindParam(long long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = INT8OID;
      length = sizeof(long long);
      }

   explicit UPgSqlStatementBindParam(unsigned long long* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = INT8OID;
      length = sizeof(long long);
      }

   explicit UPgSqlStatementBindParam(float* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = FLOAT4OID;
      length = sizeof(float);
      }

   explicit UPgSqlStatementBindParam(double* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = FLOAT8OID;
      length = sizeof(double);
      }

   explicit UPgSqlStatementBindParam(long double* v) : USqlStatementBindParam(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%p", v)

      type   = FLOAT8OID;
      length = sizeof(double);
      }

   explicit UPgSqlStatementBindParam(const char* s, int n, bool bstatic) : USqlStatementBindParam(s, n, bstatic)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindParam, "%.*S,%u,%b", n, s, n, bstatic)

      type = TEXTOID;
      }

   virtual ~UPgSqlStatementBindParam()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPgSqlStatementBindParam)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USqlStatementBindParam::dump(reset); }
#endif

   char num2str[21]; // buffer to stringify numeric
};

class U_EXPORT UPgSqlStatementBindResult : public USqlStatementBindResult {
public:

   explicit UPgSqlStatementBindResult(bool* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = BOOLOID;
      length = sizeof(bool);
      }

   explicit UPgSqlStatementBindResult(char* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = CHAROID;
      length = sizeof(char);
      }

   explicit UPgSqlStatementBindResult(unsigned char* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = CHAROID;
      length = sizeof(char);
      }

   explicit UPgSqlStatementBindResult(short* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = INT2OID;
      length = sizeof(short);
      }

   explicit UPgSqlStatementBindResult(unsigned short* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = INT2OID;
      length = sizeof(short);
      }

   explicit UPgSqlStatementBindResult(int* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = INT4OID;
      length = sizeof(int);
      }

   explicit UPgSqlStatementBindResult(unsigned int* v) : USqlStatementBindResult(v) 
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = INT4OID;
      length = sizeof(int);
      }

   explicit UPgSqlStatementBindResult(long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

#  if SIZEOF_LONG == 4
      type   = INT4OID;
#  else
      type   = INT8OID;
#  endif
      length = sizeof(long);
      }

   explicit UPgSqlStatementBindResult(unsigned long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

#  if SIZEOF_LONG == 4
      type   = INT4OID;
#  else
      type   = INT8OID;
#  endif
      length = sizeof(long);
      }

   explicit UPgSqlStatementBindResult(long long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = INT8OID;
      length = sizeof(long long);
      }

   explicit UPgSqlStatementBindResult(unsigned long long* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = INT8OID;
      length = sizeof(long long);
      }

   explicit UPgSqlStatementBindResult(float* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = FLOAT4OID;
      length = sizeof(float);
      }

   explicit UPgSqlStatementBindResult(double* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = FLOAT8OID;
      length = sizeof(double);
      }

   explicit UPgSqlStatementBindResult(long double* v) : USqlStatementBindResult(v)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%p", v)

      type   = FLOAT8OID;
      length = sizeof(double);
      }

   explicit UPgSqlStatementBindResult(UStringRep& s) : USqlStatementBindResult(s)
      {
      U_TRACE_REGISTER_OBJECT(0, UPgSqlStatementBindResult, "%V", &s)

      type = VARCHAROID;
      }

   virtual ~UPgSqlStatementBindResult()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPgSqlStatementBindResult)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USqlStatementBindResult::dump(reset); }
#endif
};

class U_EXPORT UPgSqlStatement : public USqlStatement {
public:

            UPgSqlStatement(const char* stmt, uint32_t len);
   virtual ~UPgSqlStatement();

   // SERVICES

   void reset();

   bool setBindParam( UOrmDriver* pdrv);
   void setBindResult(UOrmDriver* pdrv);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   Oid* paramTypes;          // Specifies, by OID, the data types to be assigned to the parameter symbols. If paramTypes is 0,
                             // or any particular element in the array is zero, the server infers a data type for the parameter symbol
                             // in the same way it would do for an untyped literal string
   int* paramLengths;        // Specifies the actual data lengths of binary-format parameters. It is ignored for null parameters and text-format
                             // parameters. The array pointer can be null when there are no binary parameters
   int* paramFormats;        // Specifies whether parameters are text (put a zero in the array entry for the corresponding parameter) or binary
                             // (put a one in the array entry for the corresponding parameter). If the array pointer is null then all parameters
                             // are presumed to be text strings
   const char** paramValues; // Specifies the actual values of the parameters. A null pointer in this array means the corresponding parameter
                             // is null; otherwise the pointer points to a zero-terminated text string (for text format) or binary data in the
                             // format expected by the server (for binary format)

   UString stmt;
   PGresult* res;
   char stmtName[13];
   bool resultFormat;        // is zero to obtain results in text format, or one to obtain results in binary format
};

class U_EXPORT UOrmDriverPgSql : public UOrmDriver {
public:

   UOrmDriverPgSql()
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriverPgSql, "")

      U_INTERNAL_ASSERT_POINTER(UString::str_pgsql_name)

      UOrmDriver::name = *UString::str_pgsql_name;
      }

   UOrmDriverPgSql(const UString& name_drv) : UOrmDriver(name_drv)
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriverPgSql, "%V", name_drv.rep)
      }

   virtual ~UOrmDriverPgSql();

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
   virtual USqlStatement* handlerStatementCreation(const char* stmt, uint32_t len) U_DECL_FINAL
      {
      U_TRACE(0, "UOrmDriverPgSql::handlerStatementCreation(%.*S,%u)", len, stmt, len)

      U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

      USqlStatement* pstmt;

      U_NEW(UPgSqlStatement, pstmt, UPgSqlStatement(stmt, len));

      U_RETURN_POINTER(pstmt, USqlStatement);
      }

   // CREATE BIND PARAM

   virtual USqlStatementBindParam* creatSqlStatementBindParam()
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam()); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(int* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(bool* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(char* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(short* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(float* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(double* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long long* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long double* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  int* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  char* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  long* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned  short* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned long long* v)
      { UPgSqlStatementBindParam* r; U_NEW(UPgSqlStatementBindParam, r, UPgSqlStatementBindParam(v)); return r; }

   virtual USqlStatementBindParam* creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind);

   // CREATE BIND RESULT

   virtual USqlStatementBindResult* creatSqlStatementBindResult(int* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(bool* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(char* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(short* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(float* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(double* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(UStringRep& v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long long* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long double* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned char* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned short* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned int* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long long* v)
      { UPgSqlStatementBindResult* r; U_NEW(UPgSqlStatementBindResult, r, UPgSqlStatementBindResult(v)); return r; }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
   bool checkExecution(PGresult* res);

   U_DISALLOW_COPY_AND_ASSIGN(UOrmDriverPgSql)

   friend class UPgSqlStatement;
};

#endif
