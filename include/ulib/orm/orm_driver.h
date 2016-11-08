// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver.h - database independent abstraction layer
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_ORM_DRIVER_H
#define U_ORM_DRIVER_H 1

#include <ulib/dynamic/plugin.h>
#include <ulib/container/vector.h>

typedef enum ParamType {
      NULL_VALUE =  0, // null value
   BOOLEAN_VALUE =  1, // bool value
      CHAR_VALUE =  2, //   signed char value
     UCHAR_VALUE =  3, // unsigned char value
     SHORT_VALUE =  4, //   signed short integer value
    USHORT_VALUE =  5, // unsigned short integer value
       INT_VALUE =  6, //   signed integer value
      UINT_VALUE =  7, // unsigned integer value
      LONG_VALUE =  8, //   signed long value
     ULONG_VALUE =  9, // unsigned long value
     LLONG_VALUE = 10, //   signed long long value
    ULLONG_VALUE = 11, // unsigned long long value
     FLOAT_VALUE = 12, // float value
      REAL_VALUE = 13, // double value
     LREAL_VALUE = 14, // long double value
    STRING_VALUE = 15, // UTF-8 string value
} ParamType;

class U_EXPORT USqlStatementBindParam {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   USqlStatementBindParam()
      {
      U_TRACE_REGISTER_OBJECT(0, USqlStatementBindParam, "", 0)

      buffer = 0;
      pstr   = 0;
      length = 0;
      type   = 0;

      is_unsigned = false;
      }

   USqlStatementBindParam(void* v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqlStatementBindParam, "%p", v)

      buffer = v;
      pstr   = 0;
      length = 0;
      type   = 0;

      is_unsigned = false;
      }

   explicit USqlStatementBindParam(const char* s, int n, bool bstatic);

   virtual ~USqlStatementBindParam()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USqlStatementBindParam)

      if (pstr) delete pstr;
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   void* buffer;         // buffer to get data
   unsigned long length; // buffer length
   UString* pstr;
   int type;
   bool is_unsigned;
};

class U_EXPORT USqlStatementBindResult {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   USqlStatementBindResult(void* v)
      {
      U_TRACE_REGISTER_OBJECT(0, USqlStatementBindResult, "%p", v)

      buffer = v;
      pstr   = 0;
      length = 0;
      type   = 0;

      is_unsigned = is_null = false;
      }

   explicit USqlStatementBindResult(UStringRep& s)
      {
      U_TRACE_REGISTER_OBJECT(0, USqlStatementBindResult, "%V", &s)

      buffer = s.data();
      length = s.capacity(); // NB: after fetch become the length of the actual data value...
      pstr   = &s;
      type   = 0;

      is_unsigned = is_null = false;
      }

   virtual ~USqlStatementBindResult()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USqlStatementBindResult)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   void* buffer;         // buffer to put data
   unsigned long length; // output length
   UStringRep* pstr;
   int type;
   bool is_unsigned, is_null;
};

class UOrmDriver;

class U_EXPORT USqlStatement {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   USqlStatement(void* pstmt, uint32_t nbind, uint32_t nresult) : vparam(nbind), vresult(nresult)
      {
      U_TRACE_REGISTER_OBJECT(0, USqlStatement, "%p,%u,%u", nbind, nresult)

      pHandle         = pstmt;
      num_bind_param  = nbind;
      num_bind_result = nresult;
      num_row_result  = current_row = 0;
      }

   virtual ~USqlStatement()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USqlStatement)
      }

   // SERVICES

   void reset() // Resets a prepared statement on client and server to state after creation
      {
      U_TRACE_NO_PARAM(0, "USqlStatement::reset()")

       vparam.clear();
      vresult.clear();

      num_row_result = current_row = 0;
      }

   void bindParam(USqlStatementBindParam* ptr)
      {
      U_TRACE(0, "USqlStatement::bindParam(%p)", ptr)

      U_INTERNAL_ASSERT_POINTER(ptr)

      vparam.push_back(ptr);

      U_ASSERT(vparam.size() <= num_bind_param)
      }

   void bindResult(USqlStatementBindResult* ptr)
      {
      U_TRACE(0, "USqlStatement::bindResult(%p)", ptr)

      vresult.push_back(ptr);

      U_ASSERT(vresult.size() <= num_bind_result)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   void* pHandle;
   UVector<USqlStatementBindParam*> vparam;
   UVector<USqlStatementBindResult*> vresult;
   uint32_t num_bind_param, num_bind_result, num_row_result, current_row;

   friend class UOrmDriver;
};

class UOrmSession;
class UServer_Base;
class UOrmStatement;

class U_EXPORT UOrmDriver {
public:

   // Check for memory error
   U_MEMORY_TEST

   UOrmDriver()
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriver, "")

      errmsg     = errname = 0;
      errcode    = 0;
      SQLSTATE   = 0;
      connection = 0;
      }

   UOrmDriver(const UString& name_drv) : name(name_drv)
      {
      U_TRACE_REGISTER_OBJECT(0, UOrmDriver, "%V", name.rep)

      errmsg     = errname = 0;
      errcode    = 0;
      SQLSTATE   = 0;
      connection = 0;
      }

   virtual ~UOrmDriver();

   // SERVICES

   void    printError(const char* function);
   bool    setOption(const UString& option);
   UString getOptionValue(const char* name, uint32_t len);

   UString getOptionValue(const UString& _name) { return getOptionValue(U_STRING_TO_PARAM(_name)); }

   static void clear();
   static bool loadDriver(const UString& driver_dir, const UString& driver_list);

   static bool loadDriver()                           { return loadDriver(UString::getStringNull(), UString::getStringNull()); }
   static bool loadDriver(const UString& driver_list) { return loadDriver(UString::getStringNull(),              driver_list); }

   static bool isSQLite()
      {
      U_TRACE_NO_PARAM(0, "UOrmDriver::isSQLite()")

      U_INTERNAL_ASSERT_MAJOR(env_driver_len, 0)

      if (U_STREQ(env_driver, env_driver_len, "sqlite")) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isMYSQL()
      {
      U_TRACE_NO_PARAM(0, "UOrmDriver::isMYSQL()")

      U_INTERNAL_ASSERT_MAJOR(env_driver_len, 0)

      if (U_STREQ(env_driver, env_driver_len, "mysql")) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPGSQL()
      {
      U_TRACE_NO_PARAM(0, "UOrmDriver::isPGSQL()")

      U_INTERNAL_ASSERT_MAJOR(env_driver_len, 0)

      if (U_STREQ(env_driver, env_driver_len, "pgsql")) U_RETURN(true);

      U_RETURN(false);
      }

   void reset(USqlStatement* pstmt) // Resets a prepared statement on client and server to state after creation
      {
      U_TRACE(0, "UOrmDriver::reset(%p)", pstmt)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      handlerStatementReset(pstmt);

      pstmt->reset();
      }

   void remove(USqlStatement* pstmt)
      {
      U_TRACE(0, "UOrmDriver::remove(%p)", pstmt)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      handlerStatementRemove(pstmt);
      }

   // BIND

   void bindParam(USqlStatement* pstmt)
      {
      U_TRACE(0, "UOrmDriver::bindParam(%p)", pstmt)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      USqlStatementBindParam* ptr = creatSqlStatementBindParam();

      pstmt->bindParam(ptr);
      }

   void bindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind)
      {
      U_TRACE(0, "UOrmDriver::bindParam(%p,%.*S,%u,%b,%d)", pstmt, n, s, n, bstatic, rebind)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      USqlStatementBindParam* ptr = creatSqlStatementBindParam(pstmt, s, n, bstatic, rebind);

      if (ptr) pstmt->bindParam(ptr);
      }

   template <typename T> void bindParam(USqlStatement* pstmt, T* v)
      {
      U_TRACE(0, "UOrmDriver::bindParam<T>(%p,%p)", pstmt, v)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      USqlStatementBindParam* ptr = creatSqlStatementBindParam(v);

      pstmt->bindParam(ptr);
      }

   template <typename T> void bindParam(USqlStatement* pstmt, T& v)
      {
      U_TRACE(0, "UOrmDriver::bindParam<T>(%p,%p)", pstmt, &v)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      USqlStatementBindParam* ptr = creatSqlStatementBindParam(&v);

      pstmt->bindParam(ptr);
      }

   template <typename T> void bindResult(USqlStatement* pstmt, T* v)
      {
      U_TRACE(0, "UOrmDriver::bindResult<T>(%p,%p)", pstmt, v)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      USqlStatementBindResult* ptr = creatSqlStatementBindResult(v);

      pstmt->bindResult(ptr);
      }

   template <typename T> void bindResult(USqlStatement* pstmt, T& v)
      {
      U_TRACE(0, "UOrmDriver::bindResult<T>(%p,%p)", pstmt, &v)

      U_INTERNAL_ASSERT_POINTER(pstmt)

      USqlStatementBindResult* ptr = creatSqlStatementBindResult(&v);

      pstmt->bindResult(ptr);
      }

   // VIRTUAL METHOD

   virtual void handlerError()
      {
      U_TRACE_NO_PARAM(0, "UOrmDriver::handlerError()")

      // set error value
      }

   // ------------------------------------------------------------------------------------------------
   // option string format:
   // ------------------------------------------------------------------------------------------------
   // <str> == <assignment> | <assignment> ';' <str>
   // <assignment> == <name> '=' <value>
   // <name> == 'host' | 'port' | 'user' | 'password' | 'dbname' | 'compress' | 'auto-reconnect' | ...
   // <value> == [~;]*
   // ------------------------------------------------------------------------------------------------

   virtual UOrmDriver* handlerConnect(const UString& option)
      {
      U_TRACE(0, "UOrmDriver::handlerConnect(%V)", option.rep)

      U_RETURN_POINTER(0, UOrmDriver);
      } 

   virtual void handlerDisConnect()
      {
      U_TRACE_NO_PARAM(0, "UOrmDriver::handlerDisConnect()")
      }

   // Executes the SQL statement pointed to by the null-terminated string

   virtual bool handlerQuery(const char* query, uint32_t query_len)
      {
      U_TRACE(0, "UOrmDriver::handlerQuery(%.*S,%u)", query_len, query, query_len)

      U_RETURN(false);
      }

   virtual void execute(USqlStatement* pstmt)
      {
      U_TRACE(0, "UOrmDriver::execute(%p)", pstmt)
      }

   virtual bool nextRow(USqlStatement* pstmt)
      {
      U_TRACE(0, "UOrmDriver::nextRow(%p)", pstmt)

      U_RETURN(false);
      }

   // This function returns the number of database rows that were changed
   // or inserted or deleted by the most recently completed SQL statement

   virtual unsigned long long affected(USqlStatement* pstmt) { return 0ULL; }

   // This routine returns the rowid of the most recent successful INSERT into the database

   virtual unsigned long long last_insert_rowid(USqlStatement* pstmt, const char* sequence) { return 0ULL; }

   // Get number of columns in the row

   virtual unsigned int cols(USqlStatement* pstmt) { return 0; }

   /**
    * The string must consist of a single SQL statement. You should not add a terminating semicolon (;) or \g to the statement.
    * The application can include one or more parameter markers in the SQL statement by embedding question mark (?) characters into
    * the SQL string at the appropriate positions. The markers are legal only in certain places in SQL statements. For example,
    * they are permitted in the VALUES() list of an INSERT statement (to specify column values for a row), or in a comparison with
    * a column in a WHERE clause to specify a comparison value. However, they are not permitted for identifiers (such as table or
    * column names), or to specify both operands of a binary operator such as the = equal sign. The latter restriction is necessary
    * because it would be impossible to determine the parameter type. In general, parameters are legal only in Data Manipulation Language
    * (DML) statements, and not in Data Definition Language (DDL) statements 
    */

   virtual USqlStatement* handlerStatementCreation(const char* stmt, uint32_t len)
      {
      U_TRACE(0, "UOrmDriver::handlerStatementCreation(%.*S,%u)", len, stmt, len)

      U_RETURN_POINTER(0, USqlStatement);
      }

   virtual void handlerStatementReset(USqlStatement* pstmt)
      {
      U_TRACE(0, "UOrmDriver::handlerStatementReset(%p)", pstmt)

      // Resets a prepared statement on client and server to state after prepare
      }

   virtual void handlerStatementRemove(USqlStatement* pstmt)
      {
      U_TRACE(0, "UOrmDriver::handlerStatementRemove(%p)", pstmt)
      }

   // CREATE BIND PARAM

   virtual USqlStatementBindParam* creatSqlStatementBindParam()                      { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(int* v)                { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(bool* v)               { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(char* v)               { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long* v)               { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(short* v)              { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(float* v)              { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(double* v)             { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long long* v)          { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(long double* v)        { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned int* v)       { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned char* v)      { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned long* v)      { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned short* v)     { return 0; }
   virtual USqlStatementBindParam* creatSqlStatementBindParam(unsigned long long* v) { return 0; }

   virtual USqlStatementBindParam* creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind);

   // CREATE BIND RESULT

   virtual USqlStatementBindResult* creatSqlStatementBindResult(int* v)                { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(bool* v)               { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(char* v)               { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long* v)               { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(short* v)              { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(float* v)              { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(double* v)             { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long long* v)          { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(long double* v)        { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(UStringRep& str)       { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned char* v)      { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned short* v)     { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned int* v)       { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long* v)      { return 0; }
   virtual USqlStatementBindResult* creatSqlStatementBindResult(unsigned long long* v) { return 0; }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString name, opt;
   UVector<UString> vopt; // NB: must be here to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
public:
   UString dbname;
   void* connection; // will be typecast into conn-specific type
   const char* errmsg;
   const char* errname;
   const char* SQLSTATE;
   int errcode;

protected:
   static bool                  bexit;
   static uint32_t              vdriver_size, env_driver_len;
   static const char*           env_driver;
   static const char*           env_option;
   static UString*              driver_dir;
   static UVector<UString>*     vdriver_name;
   static UVector<UString>*     vdriver_name_static;
   static UVector<UOrmDriver*>* vdriver;

   static void setDriverDirectory(const UString& dir);

private:
   static void loadStaticLinkedModules(const char* name) U_NO_EXPORT;

   U_DISALLOW_ASSIGN(UOrmDriver)

   friend class UOrmSession;
   friend class UServer_Base;
   friend class UOrmStatement;
};

template <> void UOrmDriver::bindParam<int>(USqlStatement* pstmt, int& v);
template <> void UOrmDriver::bindParam<bool>(USqlStatement* pstmt, bool& v);
template <> void UOrmDriver::bindParam<char>(USqlStatement* pstmt, char& v);
template <> void UOrmDriver::bindParam<long>(USqlStatement* pstmt, long& v);
template <> void UOrmDriver::bindParam<short>(USqlStatement* pstmt, short& v);
template <> void UOrmDriver::bindParam<float>(USqlStatement* pstmt, float& v);
template <> void UOrmDriver::bindParam<double>(USqlStatement* pstmt, double& v);
template <> void UOrmDriver::bindParam<long long>(USqlStatement* pstmt, long long& v);
template <> void UOrmDriver::bindParam<long double>(USqlStatement* pstmt, long double& v);
template <> void UOrmDriver::bindParam<unsigned int>(USqlStatement* pstmt, unsigned int& v);
template <> void UOrmDriver::bindParam<unsigned char>(USqlStatement* pstmt, unsigned char& v);
template <> void UOrmDriver::bindParam<unsigned long>(USqlStatement* pstmt, unsigned long& v);
template <> void UOrmDriver::bindParam<unsigned short>(USqlStatement* pstmt, unsigned short& v);
template <> void UOrmDriver::bindParam<unsigned long long>(USqlStatement* pstmt, unsigned long long& v);

template <> void UOrmDriver::bindResult<int>(USqlStatement* pstmt, int& v);
template <> void UOrmDriver::bindResult<bool>(USqlStatement* pstmt, bool& v);
template <> void UOrmDriver::bindResult<char>(USqlStatement* pstmt, char& v);
template <> void UOrmDriver::bindResult<long>(USqlStatement* pstmt, long& v);
template <> void UOrmDriver::bindResult<short>(USqlStatement* pstmt, short& v);
template <> void UOrmDriver::bindResult<float>(USqlStatement* pstmt, float& v);
template <> void UOrmDriver::bindResult<double>(USqlStatement* pstmt, double& v);
template <> void UOrmDriver::bindResult<long long>(USqlStatement* pstmt, long long& v);
template <> void UOrmDriver::bindResult<UStringRep>(USqlStatement* pstmt, UStringRep& v);
template <> void UOrmDriver::bindResult<long double>(USqlStatement* pstmt, long double& v);
template <> void UOrmDriver::bindResult<unsigned int>(USqlStatement* pstmt, unsigned int& v);
template <> void UOrmDriver::bindResult<unsigned char>(USqlStatement* pstmt, unsigned char& v);
template <> void UOrmDriver::bindResult<unsigned long>(USqlStatement* pstmt, unsigned long& v);
template <> void UOrmDriver::bindResult<unsigned short>(USqlStatement* pstmt, unsigned short& v);
template <> void UOrmDriver::bindResult<unsigned long long>(USqlStatement* pstmt, unsigned long long& v);

#endif
