// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    orm_driver_sqlite.cpp - ORM SQLITE driver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/socket.h>
#include <ulib/orm/driver/orm_driver_sqlite.h>

extern "C" {
#include <sqlite3.h>
}

U_CREAT_FUNC(orm_driver_sqlite, UOrmDriverSqlite)

UOrmDriverSqlite::~UOrmDriverSqlite()
{
   U_TRACE_UNREGISTER_OBJECT(0, UOrmDriverSqlite)
}

void UOrmDriverSqlite::handlerError()
{
   U_TRACE_NO_PARAM(0, "UOrmDriverSqlite::handlerError()")

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Translation table for error status codes

   struct error_value_info {
      int value;        // The numeric value
      const char* name; // The equivalent symbolic value
   };

   static const struct error_value_info error_value_table[] = {
      U_ENTRY(SQLITE_OK),           /* Successful result */
      U_ENTRY(SQLITE_ERROR),        /* SQL error or missing database */
      U_ENTRY(SQLITE_INTERNAL),     /* Internal logic error in SQLite */
      U_ENTRY(SQLITE_PERM),         /* Access permission denied */
      U_ENTRY(SQLITE_ABORT),        /* Callback routine requested an abort */
      U_ENTRY(SQLITE_BUSY),         /* The database file is locked */
      U_ENTRY(SQLITE_LOCKED),       /* A table in the database is locked */
      U_ENTRY(SQLITE_NOMEM),        /* A malloc() failed */
      U_ENTRY(SQLITE_READONLY),     /* Attempt to write a readonly database */
      U_ENTRY(SQLITE_INTERRUPT),    /* Operation terminated by sqlite3_interrupt()*/
      U_ENTRY(SQLITE_IOERR),        /* Some kind of disk I/O error occurred */
      U_ENTRY(SQLITE_CORRUPT),      /* The database disk image is malformed */
      U_ENTRY(SQLITE_NOTFOUND),     /* Unknown opcode in sqlite3_file_control() */
      U_ENTRY(SQLITE_FULL),         /* Insertion failed because database is full */
      U_ENTRY(SQLITE_CANTOPEN),     /* Unable to open the database file */
      U_ENTRY(SQLITE_PROTOCOL),     /* Database lock protocol error */
      U_ENTRY(SQLITE_EMPTY),        /* Database is empty */
      U_ENTRY(SQLITE_SCHEMA),       /* The database schema changed */
      U_ENTRY(SQLITE_TOOBIG),       /* String or BLOB exceeds size limit */
      U_ENTRY(SQLITE_CONSTRAINT),   /* Abort due to constraint violation */
      U_ENTRY(SQLITE_MISMATCH),     /* Data type mismatch */
      U_ENTRY(SQLITE_MISUSE),       /* Library used incorrectly */
      U_ENTRY(SQLITE_NOLFS),        /* Uses OS features not supported on host */
      U_ENTRY(SQLITE_AUTH),         /* Authorization denied */
      U_ENTRY(SQLITE_FORMAT),       /* Auxiliary database format error */
      U_ENTRY(SQLITE_RANGE),        /* 2nd parameter to sqlite3_bind out of range */
      U_ENTRY(SQLITE_NOTADB),       /* File opened that is not a database file */
#  ifdef SQLITE_NOTICE
      U_ENTRY(SQLITE_NOTICE),       /* Notifications from sqlite3_log() */
#  endif
#  ifdef SQLITE_WARNING
      U_ENTRY(SQLITE_WARNING),      /* Warnings from sqlite3_log() */
#  endif
      U_ENTRY(SQLITE_ROW),          /* sqlite3_step() has another row ready */
      U_ENTRY(SQLITE_DONE)          /* sqlite3_step() has finished executing */
   };

   if (UOrmDriver::errmsg  == 0) UOrmDriver::errmsg  = U_SYSCALL(sqlite3_errmsg,  "%p", (sqlite3*)UOrmDriver::connection);
   if (UOrmDriver::errcode == 0) UOrmDriver::errcode = U_SYSCALL(sqlite3_errcode, "%p", (sqlite3*)UOrmDriver::connection);

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

   UOrmDriver::bexit = (UOrmDriver::errcode == SQLITE_CORRUPT);
}

UOrmDriver* UOrmDriverSqlite::handlerConnect(const UString& option)
{
   U_TRACE(1, "UOrmDriverSqlite::handlerConnect(%V)", option.rep)

   UOrmDriver* pdrv;

   if (UOrmDriver::connection == 0) pdrv = this;
   else U_NEW(UOrmDriverSqlite, pdrv, UOrmDriverSqlite(*UString::str_sqlite_name));

   if (pdrv->setOption(option) == false)
      {
      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

   // -------------------------------------------------------------------------------------------
   // An SQLite database is normally stored in a single ordinary disk file.
   // However, in certain circumstances, the database might be stored in memory.
   //
   // The most common way to force an SQLite database to exist purely in memory
   // is to open the database using the special filename ":memory:". In other words,
   // instead of passing the name of a real disk file into one of the sqlite3_open(),
   // sqlite3_open16(), or sqlite3_open_v2() functions, pass in the string ":memory:".
   //
   // When this is done, no disk file is opened. Instead, a new database is created purely
   // in memory. The database ceases to exist as soon as the database connection is closed.
   // Every :memory: database is distinct from every other. So, opening two database connections
   // each with the filename ":memory:" will create two independent in-memory databases
   // -------------------------------------------------------------------------------------------

   const char* fullpath;
   char buffer[U_PATH_MAX];

   if (pdrv->dbname == *UString::str_memory) fullpath = UString::str_memory->data();
   else
      {
      UString dbdir = pdrv->getOptionValue(*UString::str_dbdir);

      uint32_t sz = dbdir.size();

      U_INTERNAL_DUMP("sz = %u", sz)

      if (sz) sz = u__snprintf(buffer,    sizeof(buffer),    U_CONSTANT_TO_PARAM("%v/"), dbdir.rep);
            (void) u__snprintf(buffer+sz, sizeof(buffer)-sz, U_CONSTANT_TO_PARAM("%v.db"), pdrv->dbname.rep);

      fullpath = buffer;
      }

   // create a database connection

   ((UOrmDriverSqlite*)pdrv)->encoding_UTF16 = (pdrv->getOptionValue(U_CONSTANT_TO_PARAM("encoding")) == *UString::str_UTF16);

   union uusqlite3 {
      void** p1;
   sqlite3** p2;
   };

   union uusqlite3 pconnection = { &(pdrv->connection) };

   pdrv->errcode = (((UOrmDriverSqlite*)pdrv)->encoding_UTF16 ? U_SYSCALL(sqlite3_open16, "%S,%p", fullpath, pconnection.p2)
                                                              : U_SYSCALL(sqlite3_open,   "%S,%p", fullpath, pconnection.p2));

   if (pdrv->errcode)
      {
      // sqlite3 creates a database the first time we try to access it. If this function fails,
      // there's usually a problem with access rights or an existing database is corrupted or
      // created with an incompatible version...

      pdrv->errmsg = "could not open database";

      pdrv->printError(__PRETTY_FUNCTION__);

      if (UOrmDriver::connection) delete pdrv;

      U_RETURN_POINTER(0, UOrmDriver);
      }

   // set options

   UString x = pdrv->getOptionValue(*UString::str_timeout); // timeout is specified in milliseconds

   // Calling this routine with an argument less than or equal to zero turns off all busy handlers

   (void) U_SYSCALL(sqlite3_busy_timeout, "%p,%d", (sqlite3*)pdrv->connection, x ? x.strtoul() : 8); // 8ms

   (void) U_SYSCALL(sqlite3_exec, "%p,%S,%p,%p,%p", (sqlite3*)pdrv->connection, "PRAGMA journal_mode=OFF", 0, 0, 0);
   (void) U_SYSCALL(sqlite3_exec, "%p,%S,%p,%p,%p", (sqlite3*)pdrv->connection, "PRAGMA mmap_size=44040192", 0, 0, 0);
   (void) U_SYSCALL(sqlite3_exec, "%p,%S,%p,%p,%p", (sqlite3*)pdrv->connection, "PRAGMA locking_mode=EXCLUSIVE", 0, 0, 0);

   U_RETURN_POINTER(pdrv, UOrmDriver);
}

void UOrmDriverSqlite::handlerDisConnect()
{
   U_TRACE_NO_PARAM(0, "UOrmDriverSqlite::handlerDisConnect()")

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   (void) U_SYSCALL(sqlite3_close, "%p", (sqlite3*)UOrmDriver::connection);

   UOrmDriver::connection = 0;
}

bool UOrmDriverSqlite::handlerQuery(const char* query, uint32_t query_len)
{
   U_TRACE(0, "UOrmDriverSqlite::handlerQuery(%.*S,%u)", query_len, query, query_len)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   UOrmDriver::errcode = U_SYSCALL(sqlite3_exec, "%p,%S,%p,%p,%p",
                                       (sqlite3*)UOrmDriver::connection, // An open database
                                       query,                            // SQL to be evaluated
                                       0,                                // Callback function
                                       0,                                // 1st argument to callback
                                       0);                               // Error msg written here

   if (UOrmDriver::errcode)
      {
      UOrmDriver::printError(__PRETTY_FUNCTION__);

      U_RETURN(false);
      }

   U_RETURN(true);
}

USqlStatement* UOrmDriverSqlite::handlerStatementCreation(const char* stmt, uint32_t len)
{
   U_TRACE(0, "UOrmDriverSqlite::handlerStatementCreation(%.*S,%u)", len, stmt, len)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   /**
    * If the nByte argument is less than zero, then zSql is read up to the first zero terminator.
    * If nByte is non-negative, then it is the maximum number of bytes read from Sql. When nByte
    * is non-negative, the Sql string ends at either the first '\000' or '\u0000' character or
    * the nByte-th byte, whichever comes first. If the caller knows that the supplied string is
    * nul-terminated, then there is a small performance advantage to be gained by passing an nByte
    * parameter that is equal to the number of bytes in the input string including the nul-terminator
    * bytes as this saves SQLite from having to make a copy of the input string
    */

   sqlite3_stmt* pHandle;

   UOrmDriver::errcode = (encoding_UTF16 ? U_SYSCALL(sqlite3_prepare16_v2, "%p,%S,%p,%p,%p",
                                                     (sqlite3*)UOrmDriver::connection, // An open database
                                                     stmt,                    // SQL statement, UTF-16 encoded
                                                     len+1,                   // Maximum length of Sql in bytes
                                                     &pHandle,                // OUT: Statement handle
                                                     0)                       // OUT: Pointer to unused portion of Sql
                                         : U_SYSCALL(sqlite3_prepare_v2, "%p,%S,%p,%p,%p",
                                                     (sqlite3*)UOrmDriver::connection, // An open database
                                                     stmt,                    // SQL statement, UTF-8 encoded
                                                     len+1,                   // Maximum length of Sql in bytes
                                                     &pHandle,                // OUT: Statement handle
                                                     0));                     // OUT: Pointer to unused portion of Sql

   if (UOrmDriver::errcode)
      {
      UOrmDriver::printError(__PRETTY_FUNCTION__);

      U_RETURN_POINTER(0, USqlStatement);
      }

   uint32_t num_bind_param  = U_SYSCALL(sqlite3_bind_parameter_count, "%p", pHandle),
            num_bind_result = U_SYSCALL(sqlite3_column_count,         "%p", pHandle);

   USqlStatement* pstmt;

   U_NEW(USqliteStatement, pstmt, USqliteStatement(pHandle, num_bind_param, num_bind_result));

   U_RETURN_POINTER(pstmt, USqlStatement);
}

void UOrmDriverSqlite::handlerStatementReset(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverSqlite::handlerStatementReset(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // reset a prepared statement object back to its initial state, ready to be re-executed

   UOrmDriver::errcode = U_SYSCALL(sqlite3_reset,          "%p", (sqlite3_stmt*)pstmt->pHandle) ||
                         U_SYSCALL(sqlite3_clear_bindings, "%p", (sqlite3_stmt*)pstmt->pHandle);

   if (UOrmDriver::errcode) UOrmDriver::printError(__PRETTY_FUNCTION__);

   ((USqliteStatement*)pstmt)->reset();
}

void UOrmDriverSqlite::handlerStatementRemove(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverSqlite::handlerStatementRemove(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   UOrmDriver::errcode = U_SYSCALL(sqlite3_finalize, "%p", (sqlite3_stmt*)pstmt->pHandle);

   if (UOrmDriver::errcode) UOrmDriver::printError(__PRETTY_FUNCTION__);

   delete (USqliteStatement*)pstmt;
}

bool USqliteStatement::setBindParam(UOrmDriver* pdrv)
{
   U_TRACE(0, "USqliteStatement::setBindParam(%p)", pdrv)

   U_ASSERT_EQUALS(num_bind_param, vparam.size())

   if (param_binded)
      {
      pdrv->errcode = U_SYSCALL(sqlite3_reset, "%p", (sqlite3_stmt*)pHandle);

      if (pdrv->errcode) U_RETURN(false);
      }

   int             int_value;
   double       double_value;
   long long long_long_value;

   for (uint32_t i = 0; i < num_bind_param; ++i)
      {
      USqlStatementBindParam* param = vparam[i];

      switch (param->type)
         {
         case NULL_VALUE: pdrv->errcode = U_SYSCALL(sqlite3_bind_null, "%p,%d", (sqlite3_stmt*)pHandle, i+1); break;

         case BOOLEAN_VALUE:
            {
            int_value = *(bool*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_int, "%p,%d,%u", (sqlite3_stmt*)pHandle, i+1, int_value);
            }
         break;

         case CHAR_VALUE:
            {
            int_value = *(char*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_int, "%p,%d,%u", (sqlite3_stmt*)pHandle, i+1, int_value);
            }
         break;

         case SHORT_VALUE:
            {
            int_value = *(short*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_int, "%p,%d,%u", (sqlite3_stmt*)pHandle, i+1, int_value);
            }
         break;

         case INT_VALUE:
            {
            int_value = *(int*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_int, "%p,%d,%u", (sqlite3_stmt*)pHandle, i+1, int_value);
            }
         break;

         case LLONG_VALUE:
            {
            long_long_value = *(long long*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_int64,  "%p,%d,%lu", (sqlite3_stmt*)pHandle, i+1, long_long_value);
            }
         break;

         case FLOAT_VALUE:
            {
            double_value = *(float*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_double, "%p,%d,%g", (sqlite3_stmt*)pHandle, i+1, double_value);
            }
         break;

         case REAL_VALUE:
            {
            double_value = *(double*)param->buffer;

            pdrv->errcode = U_SYSCALL(sqlite3_bind_double, "%p,%d,%g", (sqlite3_stmt*)pHandle, i+1, double_value);
            }
         break;

         case STRING_VALUE:
            {
            /**
             * The fourth argument is the number of bytes in the parameter. To be clear: the value is the number of bytes
             * in the value, not the number of characters. If the fourth parameter to sqlite3_bind_text() or sqlite3_bind_text16()
             * is negative, then the length of the string is the number of bytes up to the first zero terminator. If the fourth
             * parameter to sqlite3_bind_blob() is negative, then the behavior is undefined. If a non-negative fourth parameter is
             * provided to sqlite3_bind_text() or sqlite3_bind_text16() then that parameter must be the byte offset where the NUL
             * terminator would occur assuming the string were NUL terminated. If any NUL characters occur at byte offsets less than
             * the value of the fourth parameter then the resulting string value will contain embedded NULs. The result of expressions
             * involving strings with embedded NULs is undefined
             */

            pdrv->errcode = (((UOrmDriverSqlite*)pdrv)->encoding_UTF16
               ? U_SYSCALL(sqlite3_bind_text16, "%p,%d,%S,%d,%p", (sqlite3_stmt*)pHandle, i+1,              param->buffer, param->length, SQLITE_STATIC)
               : U_SYSCALL(sqlite3_bind_text,   "%p,%d,%S,%d,%p", (sqlite3_stmt*)pHandle, i+1, (const char*)param->buffer, param->length, SQLITE_STATIC));
            }
         break;
         }

      if (pdrv->errcode) U_RETURN(false);
      }

   param_binded = true;

   U_RETURN(true);
}

USqlStatementBindParam* UOrmDriverSqlite::creatSqlStatementBindParam(USqlStatement* pstmt, const char* s, int n, bool bstatic, int rebind)
{
   U_TRACE(0, "UOrmDriverSqlite::creatSqlStatementBindParam(%p,%.*S,%u,%b,%d)", pstmt, n, s, n, bstatic, rebind)

   U_INTERNAL_ASSERT_POINTER(pstmt)

   if (rebind == -1)
      {
      USqlStatementBindParam* ptr;
      
      U_NEW(USqliteStatementBindParam, ptr, USqliteStatementBindParam(s, n, bstatic));

      U_RETURN_POINTER(ptr, USqlStatementBindParam);
      }

   U_INTERNAL_ASSERT(((USqliteStatement*)pstmt)->param_binded)

   (void) UOrmDriver::creatSqlStatementBindParam(pstmt, s, n, bstatic, rebind);

   U_RETURN_POINTER(0, USqlStatementBindParam);
}

bool USqliteStatement::setBindResult(UOrmDriver* pdrv)
{
   U_TRACE(0, "USqliteStatement::setBindResult(%p)", pdrv)

   U_DUMP("num_bind_result = %u vresult.size() = %u", num_bind_result, vresult.size())

   U_ASSERT_EQUALS(num_bind_result, vresult.size())

   if (num_bind_result == 0) U_RETURN(false); // NB: statement is one that NOT produces a result set...

   if (num_row_result == 0)
      {
      current_row    = 1;
      num_row_result = U_NOT_FOUND;
      // --------------------------------------------------------------------------------------------------
      // NB: I've checked the source code of sqlite3, and I found the function of sqlite3_changes().
      //     But the function is only useful when the database is changed (after insert, delete or update).
      //
      // num_row_result = U_SYSCALL(sqlite3_changes, "%p", (sqlite3*)pdrv->connection);
      // --------------------------------------------------------------------------------------------------
      }

   int sz;
   const char* ptr;

   for (int i = 0; i < (int)num_bind_result; ++i)
      {
      USqlStatementBindResult* result = vresult[i];

      switch (result->type)
         {
         case BOOLEAN_VALUE:      *(bool*)result->buffer = U_SYSCALL(sqlite3_column_int,    "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case    CHAR_VALUE:      *(char*)result->buffer = U_SYSCALL(sqlite3_column_int,    "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case   SHORT_VALUE:     *(short*)result->buffer = U_SYSCALL(sqlite3_column_int,    "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case     INT_VALUE:       *(int*)result->buffer = U_SYSCALL(sqlite3_column_int,    "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case   LLONG_VALUE: *(long long*)result->buffer = U_SYSCALL(sqlite3_column_int64,  "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case   FLOAT_VALUE:     *(float*)result->buffer = U_SYSCALL(sqlite3_column_double, "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case    REAL_VALUE:    *(double*)result->buffer = U_SYSCALL(sqlite3_column_double, "%p,%d", (sqlite3_stmt*)pHandle, i); break;
         case  STRING_VALUE:
            {
            U_INTERNAL_ASSERT_POINTER(result->pstr)

            if (((UOrmDriverSqlite*)pdrv)->encoding_UTF16)
               {
               sz  =               U_SYSCALL(sqlite3_column_bytes16, "%p,%d", (sqlite3_stmt*)pHandle, i);
               ptr = (const char*) U_SYSCALL(sqlite3_column_text16,  "%p,%d", (sqlite3_stmt*)pHandle, i);
               }
            else
               {
               sz  =               U_SYSCALL(sqlite3_column_bytes, "%p,%d", (sqlite3_stmt*)pHandle, i);
               ptr = (const char*) U_SYSCALL(sqlite3_column_text,  "%p,%d", (sqlite3_stmt*)pHandle, i);
               }

            if (sz > 0) (void) result->pstr->replace(ptr, sz);
            }
         break;
         }
      }

   U_RETURN(true);
}

void UOrmDriverSqlite::execute(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverSqlite::execute(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   int numberOfRetries = 0;

   if (((USqliteStatement*)pstmt)->setBindParam(this))
      {
retry:
      UOrmDriver::errcode = U_SYSCALL(sqlite3_step, "%p", (sqlite3_stmt*)pstmt->pHandle);
      }

   if (UOrmDriver::errcode == SQLITE_ROW && // (100) sqlite3_step() has another row ready
       (pstmt->current_row = pstmt->num_row_result = 0, ((USqliteStatement*)pstmt)->setBindResult(this)))
      {
      U_INTERNAL_ASSERT_MAJOR(pstmt->num_bind_result, 0)

      return;
      }

   if (UOrmDriver::errcode &&
       UOrmDriver::errcode != SQLITE_DONE)
      {
      if (UOrmDriver::errcode == SQLITE_BUSY) // (5) The database file is locked
         {
         /**
          * SQLITE_BUSY means that the database engine was unable to acquire the database locks it needs to do its job.
          * If the statement is a COMMIT or occurs outside of an explicit transaction, then you can retry the statement.
          * If the statement is not a COMMIT and occurs within an explicit transaction then you should rollback the
          * transaction before continuing
          */

         struct timespec ts = { 0L, 10000000L }; // 10ms

         while (numberOfRetries++ < 3)
            {
            (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);

            goto retry;
            }

         // The sqlite3_reset() function is called to reset a prepared statement object back to its initial
         // state, ready to be re-executed and does not change the values of any bindings on the prepared statement 

         UOrmDriver::errcode = U_SYSCALL(sqlite3_reset, "%p", (sqlite3_stmt*)pstmt->pHandle);

         if (UOrmDriver::errcode == SQLITE_OK) goto retry;
         }

      UOrmDriver::printError(__PRETTY_FUNCTION__);
      }
}

bool UOrmDriverSqlite::nextRow(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverSqlite::nextRow(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   U_INTERNAL_DUMP("current_row = %u num_row_result = %u", pstmt->current_row, pstmt->num_row_result)

   if (pstmt->current_row >= pstmt->num_row_result)
      {
   // U_INTERNAL_ASSERT_EQUALS(U_SYSCALL(sqlite3_step, "%p", (sqlite3_stmt*)pstmt->pHandle), SQLITE_DONE)

      U_RETURN(false);
      }

   U_INTERNAL_ASSERT_MAJOR(pstmt->num_bind_result, 0)

   UOrmDriver::errcode = U_SYSCALL(sqlite3_step, "%p", (sqlite3_stmt*)pstmt->pHandle);

   if (UOrmDriver::errcode != SQLITE_ROW && // (100) sqlite3_step() has another row ready
       UOrmDriver::errcode != SQLITE_DONE)  // (101) sqlite3_step() has finished executing
      {
      UOrmDriver::printError(__PRETTY_FUNCTION__);

      U_RETURN(false);
      }

   if (UOrmDriver::errcode == SQLITE_DONE)
      {
      if (pstmt->num_row_result == U_NOT_FOUND) pstmt->num_row_result = pstmt->current_row;

      U_RETURN(false);
      }

   pstmt->current_row++;

   (void) ((USqliteStatement*)pstmt)->setBindResult(this);

   U_RETURN(true);
}

unsigned long long UOrmDriverSqlite::affected(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverSqlite::affected(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // NB: I've checked the source code of sqlite3, and I found the function of sqlite3_changes().
   //     But the function is only useful when the database is changed (after insert, delete or update)

   if (pstmt &&
       pstmt->num_row_result != U_NOT_FOUND)
      {
      U_RETURN(pstmt->num_row_result);
      }

   // This function returns the number of database rows that were changed or inserted or deleted by the most recently completed SQL statement

   unsigned long long n = U_SYSCALL(sqlite3_changes, "%p", (sqlite3*)UOrmDriver::connection);

   U_RETURN(n);
}

unsigned long long UOrmDriverSqlite::last_insert_rowid(USqlStatement* pstmt, const char* sequence)
{
   U_TRACE(0, "UOrmDriverSqlite::last_insert_rowid(%p,%S)", pstmt, sequence)

   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // This routine returns the rowid of the most recent successful INSERT into the database

   unsigned long long n = U_SYSCALL(sqlite3_last_insert_rowid, "%p", (sqlite3*)UOrmDriver::connection);

   U_RETURN(n);
}

unsigned int UOrmDriverSqlite::cols(USqlStatement* pstmt)
{
   U_TRACE(0, "UOrmDriverSqlite::cols(%p)", pstmt)

   U_INTERNAL_ASSERT_POINTER(pstmt)
   U_INTERNAL_ASSERT_POINTER(UOrmDriver::connection)

   // Return the number of columns in the result set returned by the prepared statement

   unsigned int n = U_SYSCALL(sqlite3_column_count, "%p", (sqlite3_stmt*)pstmt->pHandle);

   U_RETURN(n);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USqliteStatement::dump(bool _reset) const
{
   USqlStatement::dump(false);

   *UObjectIO::os << '\n'
                  << "param_binded                               " << param_binded;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UOrmDriverSqlite::dump(bool _reset) const
{
   UOrmDriver::dump(false);

   *UObjectIO::os << '\n'
                  << "encoding_UTF16                             " << encoding_UTF16;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
