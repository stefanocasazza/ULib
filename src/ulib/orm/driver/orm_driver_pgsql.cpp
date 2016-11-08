// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver_pgsql.cpp - ORM PostGres driver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/socket.h>
#include <ulib/orm/driver/orm_driver_pgsql.h>

U_CREAT_FUNC(orm_driver_pgsql, UOrmDriverPgSql)

UOrmDriverPgSql::~UOrmDriverPgSql()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOrmDriverPgSql)
}

void UOrmDriverPgSql::handlerError()
{
   U_TRACE_NO_PARAM(0, "UOrmDriverPgSql::UOrmDriverPgSql()")

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Translation table for error status codes

   /*
   struct error_value_info {
      int value;        // The numeric value
      const char* name; // The equivalent symbolic value
   };

   static const struct error_value_info error_value_table[] = {
   };
   */

   if (UOrmDriver::errmsg  == 0) UOrmDriver::errmsg  = U_SYSCALL(PQerrorMessage, "%p", (PGconn*)UOrmDriver::connection);
                                 UOrmDriver::errname = "???";

   /*
   if (UOrmDriver::errcode < (int)U_NUM_ELEMENTS(error_value_table) &&
       UOrmDriver::errcode == error_value_table[UOrmDriver::errcode].value)
      {
      UOrmDriver::errname = error_value_table[UOrmDriver::errcode].name;
      }
   */
}

UOrmDriver* UOrmDriverPgSql::handlerConnect(const UString& option)
{
   U_TRACE(0, "UOrmDriverPgSql::handlerConnect(%V)", option.rep)

   UOrmDriver* pdrv;

   if (UOrmDriver::connection == 0) pdrv = this;
   else U_NEW(UOrmDriverPgSql, pdrv, UOrmDriverPgSql(*UString::str_pgsql_name));

   // PQconnectdb accepts additional options as a string of "key=value" pairs

   U_INTERNAL_ASSERT(option.isNullTerminated())

   pdrv->connection = U_SYSCALL(PQconnectdb, "%S", option.data());

   if (pdrv->connection == 0)
      {
      pdrv->errmsg = "ran out of memory";

      pdrv->printError(__PRETTY_FUNCTION__);

      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

   if (PQstatus((PGconn*)pdrv->connection) != CONNECTION_OK)
      {
      pdrv->printError(__PRETTY_FUNCTION__);

      U_SYSCALL_VOID(PQfinish, "%p", (PGconn*)pdrv->connection);

      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

// (void) U_SYSCALL(PQsetClientEncoding, "%p,%S", (PGconn*)pdrv->connection, "UTF8");

   U_RETURN_POINTER(pdrv, UOrmDriver);
}

void UOrmDriverPgSql::handlerDisConnect()
{
   U_TRACE_NO_PARAM(0, "UOrmDriverPgSql::handlerDisConnect()")

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   U_SYSCALL_VOID(PQfinish, "%p", (PGconn*)UOrmDriver::connection);

   UOrmDriver::connection = 0;
}

bool UOrmDriverPgSql::checkExecution(PGresult* res)
{
   U_TRACE(0, "UOrmDriverPgSql::checkExecution(%p)", res)

   int resstatus;

   if (res == 0) goto fail;

   resstatus = U_SYSCALL(PQresultStatus, "%p", res);

   if (resstatus != PGRES_COMMAND_OK &&
       resstatus != PGRES_TUPLES_OK)
      {
fail:
      if (res) UOrmDriver::SQLSTATE = U_SYSCALL(PQresultErrorField, "%p,%d", res, PG_DIAG_SQLSTATE);

      UOrmDriver::printError(__PRETTY_FUNCTION__);

      U_SYSCALL_VOID(PQclear, "%p", res);

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool UOrmDriverPgSql::handlerQuery(const char* query, uint32_t query_len)
{
   U_TRACE(0, "UOrmDriverPgSql::handlerQuery(%.*S,%u)", query_len, query, query_len)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   PGresult* res = (PGresult*) U_SYSCALL(PQexec, "%p,%S", (PGconn*)UOrmDriver::connection, query);

   if (checkExecution(res)) U_RETURN(true);

   U_RETURN(false);
}

UPgSqlStatement::UPgSqlStatement(const char* s, uint32_t n) : USqlStatement(0, 0, 10), stmt(U_CAPACITY)
{
   U_TRACE_REGISTER_OBJECT(0, UPgSqlStatement, "%.*S,%u", n, s, n)

   char* p = stmtName;
   uint32_t start = 0, len;

   u_put_unalignedp32(p, U_MULTICHAR_CONSTANT32('S','Q','L','_'));
            u_int2hex(p+4, U_PTR2INT(this));
                      p[12] = '\0';

   while ((p = (char*)memchr(s + start, '?', n - start)))
      {
      uint32_t end = p - s;

      if (end > start)
         {
         len = end - start;

         if (len) (void) stmt.append(s + start, len);
         }

      U_INTERNAL_DUMP("start = %u end = %u num_bind_param = %u", start, end, num_bind_param)

      (void) stmt.append(1U, '$');
      (void) stmt.append(1U, '0' + ++num_bind_param);

      U_INTERNAL_ASSERT_MINOR(num_bind_param, 10)

      start = end + 1;
      }

   len = n - start;

   if (len) (void) stmt.append(s + start, len);

   (void) stmt.shrink();

   res          = 0;
   paramValues  = 0;
   paramFormats = paramLengths = 0;
   resultFormat = true; // is zero to obtain results in text format, or one to obtain results in binary format

   if (num_bind_param == 0) paramTypes = 0;
   else
      {
      vparam.reserve(num_bind_param);

      paramTypes = (Oid*) UMemoryPool::_malloc(num_bind_param * 3, sizeof(int), false);
      }
}

UPgSqlStatement::~UPgSqlStatement()
{
   U_TRACE_UNREGISTER_OBJECT(0, UPgSqlStatement)

   reset();

   if (paramTypes) UMemoryPool::_free(paramTypes, num_bind_param * 3, sizeof(int));

   if (res)     U_SYSCALL_VOID(PQclear, "%p", res);
   if (pHandle) U_SYSCALL_VOID(PQclear, "%p", (PGresult*)pHandle);
}

void UPgSqlStatement::reset()
{
   U_TRACE_NO_PARAM(0, "UPgSqlStatement::reset()")

   U_ASSERT_EQUALS(num_bind_param,  vparam.size())
   U_ASSERT_EQUALS(num_bind_result, vresult.size())

   if (paramValues)
      {
      UMemoryPool::_free(paramValues, num_bind_param, sizeof(const char*));

      paramValues  = 0;
      paramLengths = 0; 
      }
}

void UOrmDriverPgSql::handlerStatementReset(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverPgSql::handlerStatementReset(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Resets a prepared statement on client and server to state after creation

   ((UPgSqlStatement*)pstmt)->reset();
}

void UOrmDriverPgSql::handlerStatementRemove(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverPgSql::handlerStatementRemove(%p)", pstmt)

   /* If you do not explicitly deallocate a prepared statement, it is deallocated when the session ends

   char query[32];

   (void) u__snprintf(query, sizeof(query), U_CONSTANT_TO_PARAM("DEALLOCATE %.12s"), ((UPgSqlStatement*)pstmt)->stmtName);

   (void) handlerQuery(query, 0);
   */

   delete (UPgSqlStatement*)pstmt;
}

bool UPgSqlStatement::setBindParam(UOrmDriver* pdrv)
{
   U_TRACE(0, "UPgSqlStatement::setBindParam(%p)", pdrv)

   U_INTERNAL_DUMP("num_bind_param = %u paramTypes = %p paramFormats = %p paramValues = %p", num_bind_param, paramTypes, paramFormats, paramValues)

   U_ASSERT_EQUALS(num_bind_param, vparam.size())

   int i;

   if (paramTypes &&
       paramFormats == 0)
      {
      U_INTERNAL_ASSERT_MAJOR(num_bind_result, 0)

      paramFormats = (int*)paramTypes + num_bind_param;

      for (i = 0; i < (int)num_bind_param; ++i)
         {
         paramTypes[i]   = vparam[i]->type;
         paramFormats[i] = resultFormat; // 1 => binary
         }
      }

   if (pHandle == 0)
      {
      /**
       * The function creates a prepared statement named stmtName from the query string, which must contain a single SQL command.
       * stmtName can be "" to create an unnamed statement, in which case any pre-existing unnamed statement is automatically replaced;
       * otherwise it is an error if the statement name is already defined in the current session. If any parameters are used, they are
       * referred to in the query as $1, $2, etc. nParams is the number of parameters for which types are pre-specified in the array
       * paramTypes[]. (The array pointer can be 0 when nParams is zero) paramTypes[] specifies, by OID, the data types to be assigned
       * to the parameter symbols. If paramTypes is 0, or any particular element in the array is zero, the server assigns a data type
       * to the parameter symbol in the same way it would do for an untyped literal string. Also, the query can use parameter symbols with
       * numbers higher than nParams; data types will be inferred for these symbols as well
       */

      pHandle = (PGresult*) U_SYSCALL(PQprepare, "%p,%S,%S,%d,%p",
                                      (PGconn*)pdrv->UOrmDriver::connection,
                                      stmtName, stmt.data(), num_bind_param, paramTypes);

      if (pHandle == 0 ||
          U_SYSCALL(PQresultStatus, "%p", (PGresult*)pHandle) != PGRES_COMMAND_OK)
         {
         pdrv->UOrmDriver::printError(__PRETTY_FUNCTION__);

         if (pHandle)
            {
            U_SYSCALL_VOID(PQclear, "%p", (PGresult*)pHandle);
                                                     pHandle = 0;
            }

         U_RETURN(false);
         }

      res = (PGresult*) U_SYSCALL(PQdescribePrepared, "%p,%S", (PGconn*)pdrv->UOrmDriver::connection, stmtName);

      if (res == 0 ||
          U_SYSCALL(PQresultStatus, "%p", res) != PGRES_COMMAND_OK)
         {
         pdrv->UOrmDriver::printError(__PRETTY_FUNCTION__);

         if (res) U_SYSCALL_VOID(PQclear, "%p", res);
                                                res = 0;

         if (pHandle) U_SYSCALL_VOID(PQclear, "%p", (PGresult*)pHandle);
                                                               pHandle = 0;

         U_RETURN(false);
         }

      /**
       * The functions PQnparams and PQparamtype can be applied to the PGresult to obtain information about the parameters
       * of the prepared statement, and the functions PQnfields, PQfname, PQftype, etc provide information about the result
       * columns (if any) of the statement
       */

      U_INTERNAL_ASSERT_EQUALS(num_bind_param, (uint32_t)PQnparams(res))

      num_bind_result = U_SYSCALL(PQnfields, "%p", res);

#  ifdef DEBUG
      Oid paramtype;

      for (i = 0; i < (int)num_bind_param; ++i)
         {
         paramtype = PQparamtype(res, i);

         U_INTERNAL_DUMP("paramTypes[%u] = %u PQparamtype() = %u", i, paramTypes[i], paramtype)
         }

      for (i = 0; i < (int)num_bind_result; ++i)
         {
         char* fname = PQfname(res, i);
         int fformat = PQfformat(res, i),
             fsize   = PQfsize(res, i);

         paramtype = PQftype(res, i);

         U_INTERNAL_DUMP("result[%u] (%s): size = %d type = %d format = %d", i, fname, fsize, paramtype, fformat)
         }
#  endif
      }

   if (num_bind_param)
      {
      if (paramValues == 0)
         {
         paramValues = (const char**) UMemoryPool::_malloc(num_bind_param, sizeof(const char*), false);

         U_INTERNAL_ASSERT_EQUALS(paramLengths, 0)

         paramLengths = (int*)paramTypes + num_bind_param + num_bind_param;
         }

      for (i = 0; i < (int)num_bind_param; ++i)
         {
         USqlStatementBindParam* param = vparam[i];

#     ifndef U_COVERITY_FALSE_POSITIVE // Dereference after null check (FORWARD_NULL)
         paramLengths[i] = param->length;
#     endif

         char* ptr = (char*)(paramValues[i] = ((UPgSqlStatementBindParam*)param)->num2str);

         switch (param->type)
            {
            case BOOLOID: *(bool*)ptr = *(bool*)param->buffer; U_RETURN(true);
            case CHAROID: *(char*)ptr = *(char*)param->buffer; U_RETURN(true);
            }

         if (resultFormat) // is zero/one to obtain results in text/binary format
            {
            switch (param->type)
               {
               case INT2OID:     *(short*)ptr =   htons( *(uint16_t*)param->buffer); break;
               case INT4OID:       *(int*)ptr =   htonl( *(uint32_t*)param->buffer); break;
               case INT8OID: *(long long*)ptr = u_htonll(*(uint64_t*)param->buffer); break;
               }
            }
         else
            {
            switch (param->type)
               {
               case INT2OID: ptr[u_num2str32s(    *(short*)param->buffer, ptr)-ptr] = '\0'; break;
               case INT4OID: ptr[u_num2str32s(      *(int*)param->buffer, ptr)-ptr] = '\0'; break;
               case INT8OID: ptr[u_num2str64s(*(long long*)param->buffer, ptr)-ptr] = '\0'; break;
               }
            }

         U_INTERNAL_DUMP("paramValues[%u] = %u paramLengths[%u] = %u", i, paramValues[i], i, paramLengths[i])
         }
      }

   U_RETURN(true);
}

USqlStatementBindParam* UOrmDriverPgSql::creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind)
{
   U_TRACE(0, "UOrmDriverPgSql::creatSqlStatementBindParam(%p,%.*S,%u,%b,%d)", pstmt, n, s, n, bstatic, rebind)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   if (rebind == -1)
      {
      USqlStatementBindParam* ptr;
      
      U_NEW(UPgSqlStatementBindParam, ptr, UPgSqlStatementBindParam(s, n, bstatic));

      U_RETURN_POINTER(ptr, USqlStatementBindParam);
      }

   USqlStatementBindParam* param = UOrmDriver::creatSqlStatementBindParam(pstmt, s, n, bstatic, rebind);

   U_INTERNAL_ASSERT_POINTER(((UPgSqlStatement*)pstmt)->paramValues)
   U_INTERNAL_ASSERT_EQUALS( ((UPgSqlStatement*)pstmt)->vparam[rebind]->type, TEXTOID)

   ((UPgSqlStatement*)pstmt)->paramValues[rebind]  = (const char*)param->buffer;
   ((UPgSqlStatement*)pstmt)->paramLengths[rebind] =              param->length;

   U_RETURN_POINTER(0, USqlStatementBindParam);
}

void UPgSqlStatement::setBindResult(UOrmDriver* pdrv)
{
   U_TRACE(0, "UPgSqlStatement::setBindResult(%p)", pdrv)

   U_INTERNAL_DUMP("num_bind_result = %u vresult.size() = %u", num_bind_result, vresult.size())

   U_ASSERT_EQUALS(num_bind_result, vresult.size())

   if (num_bind_result == 0) return; // NB: statement is one that NOT produces a result set...

   if (num_row_result == 0)
      {
      current_row    = 1;
      num_row_result = U_SYSCALL(PQntuples, "%p", res);

      if (num_row_result == 0) return;
      }


   for (uint32_t i = 0; i < num_bind_result; ++i)
      {
      USqlStatementBindResult* result = vresult[i];

      /**
       * PQgetvalue() returns a single field value of one row of a PGresult. Row and column numbers start at 0.
       * The caller should not free the result directly. It will be freed when the associated PGresult handle is passed to PQclear.
       * For data in text format, the value returned by PQgetvalue is a null-terminated character string representation of the field value.
       * For data in binary format, the value is in the binary representation determined by the data type's typsend and typreceive functions.
       * (The value is actually followed by a zero byte in this case too, but that is not ordinarily useful, since the value is likely to
       * contain embedded nulls) An empty string is returned if the field value is null. See PQgetisnull to distinguish null values from
       * empty-string values. The pointer returned by PQgetvalue points to storage that is part of the PGresult structure. One should not
       * modify the data it points to, and one must explicitly copy the data into other storage if it is to be used past the lifetime of
       * the PGresult structure itself
       */

      char* ptr = U_SYSCALL(PQgetvalue,  "%p,%d,%d", res, current_row-1, i);
      int sz    = U_SYSCALL(PQgetlength, "%p,%d,%d", res, current_row-1, i);

#  ifdef DEBUG
      char buffer[4096];

      U_INTERNAL_DUMP("field value:\n"
                      "--------------------------------------\n"
                      "%s", u_memoryDump(buffer, (unsigned char*)ptr, sz))
#  endif

      switch (result->type)
         {
         case CHAROID: *(char*)result->buffer = *(char*)ptr;                                        return;
         case BOOLOID: *(bool*)result->buffer = *(char*)ptr == 'f' ? false : (*(char*)ptr != '\0'); return; // booleans are mapped to int values
         case TEXTOID:
         case BYTEAOID:
         case VARCHAROID:
            {
            U_INTERNAL_ASSERT_POINTER(result->pstr)

            if (sz > 0)
               {
               (void) result->pstr->replace(ptr, sz);

               U_INTERNAL_DUMP("result->pstr(%u) = %V", sz, result->pstr)
               }

            return;
            }
         }

      if (resultFormat) // is zero/one to obtain results in text/binary format
         {
         switch (result->type)
            {
            case INT2OID:     *(short*)result->buffer =   ntohs( *(uint16_t*)ptr); break;
            case INT4OID:       *(int*)result->buffer =   ntohl( *(uint32_t*)ptr); break;
            case INT8OID: *(long long*)result->buffer = u_ntohll(*(uint64_t*)ptr); break;
            case FLOAT4OID:   *(float*)result->buffer =             *(float*)ptr;  break;
            case FLOAT8OID:  *(double*)result->buffer =            *(double*)ptr;  break;
            }
         }
      else
         {
         switch (result->type)
            {
            case INT2OID:     *(short*)result->buffer = strtol( ptr, 0, 10); break;
            case INT4OID:       *(int*)result->buffer = strtol( ptr, 0, 10); break;
            case INT8OID: *(long long*)result->buffer = strtoll(ptr, 0, 10); break;
            case FLOAT4OID:  *(float*) result->buffer = strtof( ptr, 0);     break;
            case FLOAT8OID:  *(double*)result->buffer = strtod( ptr, 0);     break;
            }
         }
      }
}

void UOrmDriverPgSql::execute(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverPgSql::execute(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   if (((UPgSqlStatement*)pstmt)->setBindParam(this))
      {
      PGresult* res = (PGresult*) U_SYSCALL(PQexecPrepared, "%p,%S,%d,%p,%p,%p,%d",
                                    (PGconn*)UOrmDriver::connection,
                                    ((UPgSqlStatement*)pstmt)->stmtName,
                                    pstmt->num_bind_param,
                                    ((UPgSqlStatement*)pstmt)->paramValues,
                                    ((UPgSqlStatement*)pstmt)->paramLengths,
                                    ((UPgSqlStatement*)pstmt)->paramFormats,
                                    ((UPgSqlStatement*)pstmt)->resultFormat); // is zero/one to obtain results in text/binary format

      if (checkExecution(res) == false) return;

      U_INTERNAL_ASSERT_POINTER(((UPgSqlStatement*)pstmt)->res)

      U_SYSCALL_VOID(PQclear, "%p", ((UPgSqlStatement*)pstmt)->res);

      ((UPgSqlStatement*)pstmt)->res = res;
      }

   pstmt->current_row    =
   pstmt->num_row_result = 0;

   ((UPgSqlStatement*)pstmt)->setBindResult(this);
}

bool UOrmDriverPgSql::nextRow(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverPgSql::nextRow(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   U_INTERNAL_DUMP("current_row = %u num_row_result = %u", pstmt->current_row, pstmt->num_row_result)

   if (pstmt->current_row >= pstmt->num_row_result) U_RETURN(false);

   U_INTERNAL_ASSERT_MAJOR(pstmt->num_bind_result, 0)

   pstmt->current_row++;

   ((UPgSqlStatement*)pstmt)->setBindResult(this);

   U_RETURN(true);
}

unsigned long long UOrmDriverPgSql::affected(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverPgSql::affected(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   unsigned long long n = 0;

   /**
    * This function returns a string containing the number of rows affected by the SQL statement that generated the PGresult.
    * This function can only be used following the execution of an INSERT, UPDATE, DELETE, MOVE, FETCH, or COPY statement, or
    * an EXECUTE of a prepared query that contains an INSERT, UPDATE, or DELETE statement. If the command that generated the
    * PGresult was anything else, PQcmdTuples returns an empty string. The caller should not free the return value directly.
    * It will be freed when the associated PGresult handle is passed to PQclear
    */

   if (pstmt)
      {
      if (U_SYSCALL(PQresultStatus, "%p", ((UPgSqlStatement*)pstmt)->res) == PGRES_COMMAND_OK)
         {
         char const* s = U_SYSCALL(PQcmdTuples, "%p", ((UPgSqlStatement*)pstmt)->res);

         n = (s ? (unsigned long long) atoll(s) : pstmt->num_row_result);
         }
      else if (U_SYSCALL(PQresultStatus, "%p", ((UPgSqlStatement*)pstmt)->res) == PGRES_TUPLES_OK)
         {
         n = U_SYSCALL(PQntuples, "%p", ((UPgSqlStatement*)pstmt)->res);
         }
      }

   U_RETURN(n);
}

unsigned long long UOrmDriverPgSql::last_insert_rowid(USqlStatement* pstmt, const char* sequence)
{
   U_TRACE(0, "UOrmDriverPgSql::last_insert_rowid(%p,%S)", pstmt, sequence)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   /**
    * If you create a column as serial PostgreSQL automatically creates a sequence for that.
    * The name of the sequence is autogenerated and is always tablename_columnname_seq, Ex. names_id_seq
    */

   unsigned long long n = 0;

   PGresult* res = (PGresult*) U_SYSCALL(PQexecParams, "%p,%S,%d,%p,%p,%p,%d",
                                          (PGconn*)UOrmDriver::connection,
                                          "SELECT currval($1)",
                                          1, // 1 param
                                          0, // types
                                          &sequence, // param values
                                          0,  // lengths
                                          0,  // formats
                                          0); // string format

   if (checkExecution(res))
      {
      const char* val = U_SYSCALL(PQgetvalue, "%p,%d,%d", res, 0, 0);

      U_SYSCALL_VOID(PQclear, "%p", res);

      n = (unsigned long long) atoll(val);
      }

   U_RETURN(n);
}

unsigned int UOrmDriverPgSql::cols(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverPgSql::cols(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Return the number of columns in the result set returned by the prepared statement

   unsigned int n = U_SYSCALL(PQnfields, "%p", ((UPgSqlStatement*)pstmt)->res);

   U_RETURN(n);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UPgSqlStatement::dump(bool _reset) const
{
   USqlStatement::dump(false);

   *UObjectIO::os << '\n'
                  << "stmtName                                   " << stmtName;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UOrmDriverPgSql::dump(bool _reset) const
{
   UOrmDriver::dump(false);

   *UObjectIO::os << '\n';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
