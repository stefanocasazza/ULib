// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver_mysql.cpp - ORM MYSQL driver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/socket.h>
#include <ulib/orm/driver/orm_driver_mysql.h>

extern "C" {
#include <mysql/errmsg.h>
}

U_CREAT_FUNC(orm_driver_mysql, UOrmDriverMySql)

UOrmDriverMySql::~UOrmDriverMySql()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOrmDriverMySql)
}

void UOrmDriverMySql::handlerError()
{
   U_TRACE_NO_PARAM(0, "UOrmDriverMySql::UOrmDriverMySql()")

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Translation table for error status codes

   struct error_value_info {
      int value;        // The numeric value
      const char* name; // The equivalent symbolic value
   };

   static const struct error_value_info error_value_table[] = {
      U_ENTRY(CR_UNKNOWN_ERROR),
      U_ENTRY(CR_SOCKET_CREATE_ERROR),
      U_ENTRY(CR_CONNECTION_ERROR),
      U_ENTRY(CR_CONN_HOST_ERROR),
      U_ENTRY(CR_IPSOCK_ERROR),
      U_ENTRY(CR_UNKNOWN_HOST),
      U_ENTRY(CR_SERVER_GONE_ERROR),
      U_ENTRY(CR_VERSION_ERROR),
      U_ENTRY(CR_OUT_OF_MEMORY),
      U_ENTRY(CR_WRONG_HOST_INFO),
      U_ENTRY(CR_LOCALHOST_CONNECTION),
      U_ENTRY(CR_TCP_CONNECTION),
      U_ENTRY(CR_SERVER_HANDSHAKE_ERR),
      U_ENTRY(CR_SERVER_LOST),
      U_ENTRY(CR_COMMANDS_OUT_OF_SYNC),
      U_ENTRY(CR_NAMEDPIPE_CONNECTION),
      U_ENTRY(CR_NAMEDPIPEWAIT_ERROR),
      U_ENTRY(CR_NAMEDPIPEOPEN_ERROR),
      U_ENTRY(CR_NAMEDPIPESETSTATE_ERROR),
      U_ENTRY(CR_CANT_READ_CHARSET),
      U_ENTRY(CR_NET_PACKET_TOO_LARGE),
      U_ENTRY(CR_EMBEDDED_CONNECTION),
      U_ENTRY(CR_PROBE_SLAVE_STATUS),
      U_ENTRY(CR_PROBE_SLAVE_HOSTS),
      U_ENTRY(CR_PROBE_SLAVE_CONNECT),
      U_ENTRY(CR_PROBE_MASTER_CONNECT),
      U_ENTRY(CR_SSL_CONNECTION_ERROR),
      U_ENTRY(CR_MALFORMED_PACKET),
      U_ENTRY(CR_WRONG_LICENSE),
      U_ENTRY(CR_NULL_POINTER),
      U_ENTRY(CR_NO_PREPARE_STMT),
      U_ENTRY(CR_PARAMS_NOT_BOUND),
      U_ENTRY(CR_DATA_TRUNCATED),
      U_ENTRY(CR_NO_PARAMETERS_EXISTS),
      U_ENTRY(CR_INVALID_PARAMETER_NO),
      U_ENTRY(CR_INVALID_BUFFER_USE),
      U_ENTRY(CR_UNSUPPORTED_PARAM_TYPE),
      U_ENTRY(CR_SHARED_MEMORY_CONNECTION),
      U_ENTRY(CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_CONNECT_MAP_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_FILE_MAP_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_MAP_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_EVENT_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR),
      U_ENTRY(CR_SHARED_MEMORY_CONNECT_SET_ERROR),
      U_ENTRY(CR_CONN_UNKNOW_PROTOCOL),
      U_ENTRY(CR_INVALID_CONN_HANDLE),
#  ifdef CR_SECURE_AUTH
      U_ENTRY(CR_SECURE_AUTH),
#  endif
      U_ENTRY(CR_FETCH_CANCELED),
      U_ENTRY(CR_NO_DATA),
      U_ENTRY(CR_NO_STMT_METADATA),
      U_ENTRY(CR_NO_RESULT_SET),
      U_ENTRY(CR_NOT_IMPLEMENTED),
      U_ENTRY(CR_SERVER_LOST_EXTENDED),
      U_ENTRY(CR_STMT_CLOSED),
      U_ENTRY(CR_NEW_STMT_METADATA),
      U_ENTRY(CR_ALREADY_CONNECTED),
      U_ENTRY(CR_AUTH_PLUGIN_CANNOT_LOAD)
   };

   UOrmDriver::errcode  = U_SYSCALL(mysql_errno,    "%p", (MYSQL*)UOrmDriver::connection);
   UOrmDriver::SQLSTATE = U_SYSCALL(mysql_sqlstate, "%p", (MYSQL*)UOrmDriver::connection);

   if (UOrmDriver::errmsg == 0) UOrmDriver::errmsg = U_SYSCALL(mysql_error, "%p", (MYSQL*)UOrmDriver::connection);

   if (UOrmDriver::errcode >= CR_ERROR_FIRST) UOrmDriver::errcode -= CR_ERROR_FIRST; // 2000

   if (UOrmDriver::errcode >= 0                                     &&
       UOrmDriver::errcode < (int)U_NUM_ELEMENTS(error_value_table) &&
       UOrmDriver::errcode == error_value_table[UOrmDriver::errcode].value)
      {
      UOrmDriver::errname = error_value_table[UOrmDriver::errcode].name;
      }
   else
      {
      UOrmDriver::errname = "???";

      for (unsigned int i = 0; i < U_NUM_ELEMENTS(error_value_table); ++i)
         {
         if (UOrmDriver::errcode == error_value_table[i].value)
            {
            UOrmDriver::errname = error_value_table[i].name;

            break;
            }
         }
      }
}

UOrmDriver* UOrmDriverMySql::handlerConnect(const UString& option)
{
   U_TRACE(0, "UOrmDriverMySql::handlerConnect(%V)", option.rep)

   UOrmDriver* pdrv;

   if (UOrmDriver::connection == 0) pdrv = this;
   else U_NEW(UOrmDriverMySql, pdrv, UOrmDriverMySql(*UString::str_mysql_name));

   if (pdrv->setOption(option) == false)
      {
      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

   pdrv->connection = U_SYSCALL(mysql_init, "%p", (MYSQL*)pdrv->connection);

   if (pdrv->connection == 0)
      {
      pdrv->errmsg = "ran out of memory";

      pdrv->printError(__PRETTY_FUNCTION__);

      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

   int timeout = pdrv->getOptionValue(*UString::str_timeout).strtoul(); // generic timeout is specified in seconds

   (void) U_SYSCALL(mysql_options, "%p", (MYSQL*)pdrv->connection, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&timeout);

   int port = pdrv->getOptionValue(*UString::str_port).strtoul();

   if (port <= 0 || port > 65535) port = 3306; // if no port was specified, use MySQL's default port to let things run gracefully

   UString host = pdrv->getOptionValue(*UString::str_host);

   if (host) host.setNullTerminated();
   else      host = *UString::str_localhost;

   UString user           = pdrv->getOptionValue(U_CONSTANT_TO_PARAM("user")),
           encoding       = pdrv->getOptionValue(U_CONSTANT_TO_PARAM("encoding")),
           password       = pdrv->getOptionValue(U_CONSTANT_TO_PARAM("password")),
           compress       = pdrv->getOptionValue(*UString::str_compress),
           secure_auth    = pdrv->getOptionValue(*UString::str_secure_auth),
           character_set  = pdrv->getOptionValue(*UString::str_character_set),
           auto_reconnect = pdrv->getOptionValue(*UString::str_auto_reconnect);

   /**
    * mysql_options() should be called after mysql_init() and before mysql_connect() or mysql_real_connect().
    * Can be used to set extra connect options and affect behavior for a connection.
    * This function may be called multiple times to set several options
    *
    * see http://dev.mysql.com/doc/refman/5.0/en/mysql-options.html
    */

   if (      compress.strtob()) (void) U_SYSCALL(mysql_options, "%p,%d,%p", (MYSQL*)pdrv->connection, MYSQL_OPT_COMPRESS,        compress.c_str());
   if (   secure_auth.strtob()) (void) U_SYSCALL(mysql_options, "%p,%d,%p", (MYSQL*)pdrv->connection, MYSQL_SECURE_AUTH,      secure_auth.c_str());
   if (auto_reconnect.strtob()) (void) U_SYSCALL(mysql_options, "%p,%d,%p", (MYSQL*)pdrv->connection, MYSQL_OPT_RECONNECT, auto_reconnect.c_str());

   if (user)         user.setNullTerminated();
   if (password) password.setNullTerminated();

   U_INTERNAL_ASSERT(pdrv->dbname.isNullTerminated())

   if (U_SYSCALL(mysql_real_connect,"%p,%S,%S,%S,%S,%u,%S,%lu",(MYSQL*)pdrv->connection,host.data(),user.data(),password.data(),pdrv->dbname.data(),(unsigned int)port,0,0L) == 0)
      {
      pdrv->printError(__PRETTY_FUNCTION__);

      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

   /**
    * You are not allowed to call the mysql_set_character_set() function before the mysql_real_connect().
    * The purpose of the function is to tune the current connection
    *
    * see http://dev.mysql.com/doc/refman/5.0/en/mysql-set-character-set.html
    */

   if (character_set)
      {
      const char* ptr = character_set.c_str();

      (void) U_SYSCALL(mysql_set_character_set, "%p,%S", (MYSQL*)pdrv->connection, ptr);
      }

   U_RETURN_POINTER(pdrv, UOrmDriver);
}

void UOrmDriverMySql::handlerDisConnect()
{
   U_TRACE_NO_PARAM(0, "UOrmDriverMySql::handlerDisConnect()")

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   U_SYSCALL_VOID(mysql_close, "%p", (MYSQL*)UOrmDriver::connection);

   UOrmDriver::connection = 0;
}

bool UOrmDriverMySql::handlerQuery(const char* query, uint32_t query_len)
{
   U_TRACE(0, "UOrmDriverMySql::handlerQuery(%.*S,%u)", query_len, query, query_len)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   UOrmDriver::errcode = U_SYSCALL(mysql_real_query, "%p,%S,%lu", (MYSQL*)UOrmDriver::connection, query, (unsigned long)query_len);

   if (UOrmDriver::errcode)
      {
      UOrmDriver::printError(__PRETTY_FUNCTION__);

      U_RETURN(false);
      }

   U_RETURN(true);
}

USqlStatement* UOrmDriverMySql::handlerStatementCreation(const char* stmt, uint32_t len)
{
   U_TRACE(0, "UOrmDriverMySql::handlerStatementCreation(%.*S,%u)", len, stmt, len)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   MYSQL_STMT* pHandle = (MYSQL_STMT*) U_SYSCALL(mysql_stmt_init, "%p", (MYSQL*)UOrmDriver::connection);

   if (pHandle == 0)
      {
      UOrmDriver::errmsg = "ran out of memory";

      UOrmDriver::printError(__PRETTY_FUNCTION__);

      U_RETURN_POINTER(0, USqlStatement);
      }

   UOrmDriver::errcode = U_SYSCALL(mysql_stmt_prepare, "%p,%S,%u", pHandle, stmt, len);

   if (UOrmDriver::errcode)
      {
      UOrmDriver::errmsg = U_SYSCALL(mysql_stmt_error, "%p", pHandle);

      UOrmDriver::printError(__PRETTY_FUNCTION__);

      (void) U_SYSCALL(mysql_stmt_close, "%p", pHandle);

      U_RETURN_POINTER(0, USqlStatement);
      }

   uint32_t num_bind_param  = U_SYSCALL(mysql_stmt_param_count, "%p", pHandle),
            num_bind_result = U_SYSCALL(mysql_stmt_field_count, "%p", pHandle);

   USqlStatement* pstmt;

   U_NEW(UMySqlStatement, pstmt, UMySqlStatement(pHandle, num_bind_param, num_bind_result));

   U_RETURN_POINTER(pstmt, USqlStatement);
}

void UMySqlStatement::reset()
{
   U_TRACE_NO_PARAM(0, "UMySqlStatement::reset()")

   U_ASSERT_EQUALS(num_bind_param,  vparam.size())
   U_ASSERT_EQUALS(num_bind_result, vresult.size())

   string_type = 0;

   if (mysql_vparam)
      {
      UMemoryPool::_free(mysql_vparam, num_bind_param, sizeof(MYSQL_BIND));
                         mysql_vparam = 0;
      }

   if (mysql_vresult)
      {
      UMemoryPool::_free(mysql_vresult, num_bind_result, sizeof(MYSQL_BIND));
                         mysql_vresult = 0;
      }
}

void UOrmDriverMySql::handlerStatementReset(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverMySql::handlerStatementReset(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Resets a prepared statement on client and server to state after creation

   UOrmDriver::errcode = U_SYSCALL(mysql_stmt_reset, "%p", (MYSQL_STMT*)pstmt->pHandle);

   if (UOrmDriver::errcode) UOrmDriver::printError(__PRETTY_FUNCTION__);

   ((UMySqlStatement*)pstmt)->reset();
}

void UOrmDriverMySql::handlerStatementRemove(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverMySql::handlerStatementRemove(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   UOrmDriver::errcode = U_SYSCALL(mysql_stmt_close, "%p", (MYSQL_STMT*)pstmt->pHandle);

   if (UOrmDriver::errcode) UOrmDriver::printError(__PRETTY_FUNCTION__);

   delete (UMySqlStatement*)pstmt;
}

bool UMySqlStatement::setBindParam(UOrmDriver* pdrv)
{
   U_TRACE(0, "UMySqlStatement::setBindParam(%p)", pdrv)

   U_ASSERT_EQUALS(num_bind_param, vparam.size())

   if (num_bind_param &&
       mysql_vparam == 0)
      {
      USqlStatementBindParam* param;

      mysql_vparam = (MYSQL_BIND*) UMemoryPool::_malloc(num_bind_param, sizeof(MYSQL_BIND), true);

      for (uint32_t i = 0; i < num_bind_param; ++i)
         {
         param = vparam[i];

         MYSQL_BIND* mysql_param = mysql_vparam+i;

         mysql_param->buffer        = param->buffer;
         mysql_param->buffer_length = param->length;
         mysql_param->buffer_type   = (enum_field_types)param->type;
         mysql_param->is_unsigned   = param->is_unsigned;
         }

      pdrv->errcode = U_SYSCALL(mysql_stmt_bind_param, "%p,%p", (MYSQL_STMT*)pHandle, mysql_vparam);

      if (pdrv->errcode) U_RETURN(false);
      }

   U_RETURN(true);
}

USqlStatementBindParam* UOrmDriverMySql::creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind)
{
   U_TRACE(0, "UOrmDriverMySql::creatSqlStatementBindParam(%p,%.*S,%u,%b,%d)", pstmt, n, s, n, bstatic, rebind)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   if (rebind == -1)
      {
      USqlStatementBindParam* ptr;

      U_NEW(UMySqlStatementBindParam, ptr, UMySqlStatementBindParam(s, n, bstatic));

      U_RETURN_POINTER(ptr, USqlStatementBindParam);
      }

   USqlStatementBindParam* param = UOrmDriver::creatSqlStatementBindParam(pstmt, s, n, bstatic, rebind);

   U_INTERNAL_ASSERT_POINTER(((UMySqlStatement*)pstmt)->mysql_vparam)

   MYSQL_BIND* mysql_param = ((UMySqlStatement*)pstmt)->mysql_vparam+rebind;

   U_INTERNAL_ASSERT_EQUALS(mysql_param->buffer_type, MYSQL_TYPE_STRING)

   mysql_param->buffer        = param->buffer;
   mysql_param->buffer_length = param->length;

   UOrmDriver::errcode = U_SYSCALL(mysql_stmt_bind_param, "%p,%p", (MYSQL_STMT*)pstmt->pHandle, ((UMySqlStatement*)pstmt)->mysql_vparam);

   if (UOrmDriver::errcode) UOrmDriver::printError(__PRETTY_FUNCTION__);

   U_RETURN_POINTER(0, USqlStatementBindParam);
}

void UMySqlStatement::setStringBindedAsResult()
{
   U_TRACE_NO_PARAM(0, "UMySqlStatement::setStringBindedAsResult()")

   U_INTERNAL_ASSERT_MAJOR(num_bind_result, 0)
   U_ASSERT_EQUALS(num_bind_result, vresult.size())

   USqlStatementBindResult* result;

   for (uint32_t i = 0; i < num_bind_result; ++i)
      {
      result = vresult[i];

      if (result->type == string_type)
         {
         U_INTERNAL_ASSERT_POINTER(result->pstr)

         result->pstr->size_adjust_force(result->length); // output length

         U_INTERNAL_DUMP("result->pstr = %V", result->pstr)
         }
      }
}

bool UMySqlStatement::setBindResult(UOrmDriver* pdrv)
{
   U_TRACE(0, "UMySqlStatement::setBindResult(%p)", pdrv)

   U_ASSERT_EQUALS(num_bind_result, vresult.size())

   if (num_bind_result == 0) U_RETURN(false); // NB: statement is one that NOT produces a result set...

   if (num_row_result == 0)
      {
      current_row    = 1;
      num_row_result = U_NOT_FOUND;
      // --------------------------------------------------------------------------------------------------
      // NB: the function is only useful when the database is changed (after insert, delete or update).
      //
      // num_row_result = U_SYSCALL(mysql_stmt_affected_rows, "%p", (MYSQL_STMT*)pHandle);
      // --------------------------------------------------------------------------------------------------

      pdrv->errcode = U_SYSCALL(mysql_stmt_store_result, "%p", (MYSQL_STMT*)pHandle);

      if (pdrv->errcode) U_RETURN(false);

#  ifdef DEBUG
      MYSQL_RES* result = (MYSQL_RES*) U_SYSCALL(mysql_stmt_result_metadata, "%p", (MYSQL_STMT*)pHandle);

      if (result)
         {
         uint32_t columns_count = U_SYSCALL(mysql_num_fields, "%p", result);

         if (columns_count != num_bind_result)
            {
            U_ERROR("Invalid column count (%u) returned by MySQL - total columns (%u) in SELECT statement", columns_count, num_bind_result);
            }

         U_SYSCALL_VOID(mysql_free_result, "%p", result);
         }
#  endif
      }

   if (mysql_vresult == 0)
      {
      USqlStatementBindResult* result;

      mysql_vresult = (MYSQL_BIND*) UMemoryPool::_malloc(num_bind_result, sizeof(MYSQL_BIND), true);

      for (uint32_t i = 0; i < num_bind_result; ++i)
         {
         result = vresult[i];

         MYSQL_BIND* mysql_result = mysql_vresult+i;

         mysql_result->is_null     = (my_bool*)&(result->is_null);
         mysql_result->buffer      = result->buffer;
         mysql_result->buffer_type = (enum_field_types)result->type;

         if (result->type != MYSQL_TYPE_STRING) mysql_result->is_unsigned = result->is_unsigned;
         else
            {
            string_type = MYSQL_TYPE_STRING;

            mysql_result->length        = &(result->length); // output length pointer
            mysql_result->buffer_length =   result->length;
            }
         }

      U_INTERNAL_DUMP("string_type = %u", string_type)

      pdrv->errcode = U_SYSCALL(mysql_stmt_bind_result, "%p,%p", (MYSQL_STMT*)pHandle, mysql_vresult);

      if (pdrv->errcode) U_RETURN(false);
      }

   U_RETURN(true);
}

void UOrmDriverMySql::execute(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverMySql::execute(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   /**
    * mysql_stmt_execute() executes the prepared query associated with the statement handle.
    * The currently bound parameter marker values are sent to server during this call, and the
    * server replaces the markers with this newly supplied data
    */

   if (((UMySqlStatement*)pstmt)->setBindParam(this))
      {
//retry:
      UOrmDriver::errcode = U_SYSCALL(mysql_stmt_execute, "%p", (MYSQL_STMT*)pstmt->pHandle);
      }

   if (UOrmDriver::errcode)
      {
#  ifndef U_LOG_DISABLE
      UOrmDriver::printError(__PRETTY_FUNCTION__);
#  endif

      return;
      }

   if ((pstmt->current_row = pstmt->num_row_result = 0, ((UMySqlStatement*)pstmt)->setBindResult(this)))
      {
      U_INTERNAL_ASSERT_MAJOR(pstmt->num_bind_result, 0)

      /**
       * mysql_stmt_fetch() returns the next row in the result set. It can be called only while the result set exists;
       * that is, after a call to mysql_stmt_execute() for a statement such as SELECT that produces a result set.
       *
       * mysql_stmt_fetch() returns row data using the buffers bound by mysql_stmt_bind_result(). It returns the data
       * in those buffers for all the columns in the current row set and the lengths are returned to the length pointer.
       * All columns must be bound by the application before it calls mysql_stmt_fetch(). By default, result sets are
       * fetched unbuffered a row at a time from the server. To buffer the entire result set on the client, call
       * mysql_stmt_store_result() after binding the data buffers and before calling mysql_stmt_fetch().
       *
       * If a fetched data value is a NULL value, the *is_null value of the corresponding MYSQL_BIND structure contains
       * TRUE (1). Otherwise, the data and its length are returned in the *buffer and *length elements based on the buffer
       * type specified by the application. Each numeric and temporal type has a fixed length. The length of the string types
       * depends on the length of the actual data value, as indicated by data_length
       */

      UOrmDriver::errcode = U_SYSCALL(mysql_stmt_fetch, "%p", (MYSQL_STMT*)pstmt->pHandle);

      /**
       * (CR_COMMANDS_OUT_OF_SYNC - Commands were executed in an improper order) can happen, for example,
       * if you are using mysql_use_result() and try to execute a new query before you have called mysql_free_result().
       * It can also happen if you try to execute two queries that return data without calling mysql_use_result() or mysql_store_result() in between
       */

      if (UOrmDriver::errcode &&
          UOrmDriver::errcode != MYSQL_NO_DATA)
         {
#     ifndef U_LOG_DISABLE
         UOrmDriver::printError(__PRETTY_FUNCTION__);
#     endif

         return;
         }

      if (((UMySqlStatement*)pstmt)->string_type) ((UMySqlStatement*)pstmt)->setStringBindedAsResult();
      }
}

bool UOrmDriverMySql::nextRow(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverMySql::nextRow(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   U_INTERNAL_DUMP("current_row = %u num_row_result = %u", pstmt->current_row, pstmt->num_row_result)

   if (pstmt->current_row >= pstmt->num_row_result)
      {
      U_INTERNAL_ASSERT_EQUALS(U_SYSCALL(mysql_stmt_fetch, "%p", (MYSQL_STMT*)pstmt->pHandle), MYSQL_NO_DATA)

      U_RETURN(false);
      }

   U_INTERNAL_ASSERT_MAJOR(pstmt->num_bind_result, 0)

   UOrmDriver::errcode = U_SYSCALL(mysql_stmt_fetch, "%p", (MYSQL_STMT*)pstmt->pHandle);

   if (UOrmDriver::errcode &&
       UOrmDriver::errcode != MYSQL_NO_DATA)
      {
#  ifndef U_LOG_DISABLE
      UOrmDriver::printError(__PRETTY_FUNCTION__);
#  endif

      U_RETURN(false);
      }

   if (UOrmDriver::errcode == MYSQL_NO_DATA)
      {
      if (pstmt->num_row_result == U_NOT_FOUND) pstmt->num_row_result = pstmt->current_row;

      U_RETURN(false);
      }

   pstmt->current_row++;

   if (((UMySqlStatement*)pstmt)->string_type) ((UMySqlStatement*)pstmt)->setStringBindedAsResult();

   U_RETURN(true);
}

unsigned long long UOrmDriverMySql::affected(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverMySql::affected(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // This function returns the number of database rows that were changed or inserted or deleted by the most recently completed SQL statement

   unsigned long long n = (pstmt ? U_SYSCALL(mysql_stmt_affected_rows, "%p", (MYSQL_STMT*)pstmt->pHandle)
                                 : U_SYSCALL(mysql_affected_rows,      "%p", (MYSQL*)UOrmDriver::connection));

   U_RETURN(n);
}

unsigned long long UOrmDriverMySql::last_insert_rowid(USqlStatement* pstmt, const char* sequence)
{
   U_TRACE(0, "UOrmDriverMySql::last_insert_rowid(%p,%S)", pstmt, sequence)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Returns the value generated for an AUTO_INCREMENT column by the prepared INSERT or UPDATE statement.
   // Use this function after you have executed a prepared INSERT statement on a table which contains an AUTO_INCREMENT field 

   unsigned long long n = (pstmt ? U_SYSCALL(mysql_stmt_insert_id, "%p", (MYSQL_STMT*)pstmt->pHandle)
                                 : U_SYSCALL(mysql_insert_id,      "%p", (MYSQL*)UOrmDriver::connection));

   U_RETURN(n);
}

unsigned int UOrmDriverMySql::cols(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverMySql::cols(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Return the number of columns in the result set returned by the prepared statement

   unsigned int n = (pstmt ? U_SYSCALL(mysql_stmt_field_count, "%p", (MYSQL_STMT*)pstmt->pHandle)
                           : U_SYSCALL(mysql_field_count,      "%p", (MYSQL*)UOrmDriver::connection));

   U_RETURN(n);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UMySqlStatement::dump(bool _reset) const
{
   USqlStatement::dump(false);

   *UObjectIO::os << '\n'
                  << "string_type                                " << string_type  << '\n'
                  << "mysql_vparam                               " << mysql_vparam << '\n'
                  << "mysql_vresult                              " << num_bind_param;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UOrmDriverMySql::dump(bool _reset) const
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
