// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client_rdb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc_client.h>
#include <ulib/utility/socket_ext.h>
#include <ulib/net/client/client_rdb.h>

U_NO_EXPORT void URDBClient_Base::setStatus()
{
   U_TRACE_NO_PARAM(0, "URDBClient_Base::setStatus()")

   const char* descr;

   switch (nResponseCode)
      {
      // 2xx indicates success of some kind
      case 200: descr = "OK"; break;

      // 4xx try again: not found, EOF, no database open, permission denied
      case 400: descr = "Lookup failed: the entry was not in the database"; break;
      case 401: descr = "Store failed: flag was insertion of new entries only and the key already existed"; break;
      case 402: descr = "Remove failed: the entry was already marked deleted"; break;

      // 5xx go away: parse error, catastrophic error, ...
      case 500: descr = "Server error: requested action not taken"; break;

      default:  descr = "Code unknown"; break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("(%d, %s)"), nResponseCode, descr);
}

bool URDBClient_Base::readResponse()
{
   U_TRACE_NO_PARAM(0, "URDBClient_Base::readResponse()")

   response.setBuffer(U_CAPACITY);

   bool ok = URPCClient_Base::readResponse(socket, buffer, response);

   if (ok)
      {
      nResponseCode = strtol(buffer.data(), 0, 10);

#  ifdef DEBUG
      setStatus();

      U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

      u_buffer_len = 0;
#  endif
      }

   U_RETURN(ok);
}

bool URDBClient_Base::processRequest(const char* token)
{
   U_TRACE(0, "URDBClient_Base::processRequest(%S)", token)

   uint32_t size = U_TOKEN_LN;

   for (uint32_t i = 0, n = URPC::rpc_info->size(); i < n; ++i) size += U_TOKEN_LN + (*URPC::rpc_info)[i].size();

   UString req(size);

   UStringExt::buildTokenVector(token, *URPC::rpc_info, req);

   UClient_Base::prepareRequest(req);

   if (sendRequest() &&
       readResponse())
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool URDBClient_Base::closeReorganize()
{
   U_TRACE_NO_PARAM(0, "URDBClient_Base::closeReorganize()")

   reset();

   bool result = processRequest("RORG") && isOK();

   UClient_Base::socket->close();

   U_RETURN(result);
}

bool URDBClient_Base::beginTransaction()
{
   U_TRACE_NO_PARAM(0, "URDBClient_Base::beginTransaction()")

   reset();

   bool result = processRequest("BTRN") && isOK();

   U_RETURN(result);
}

bool URDBClient_Base::abortTransaction()
{
   U_TRACE_NO_PARAM(0, "URDBClient_Base::abortTransaction()")

   reset();

   bool result = processRequest("ATRN") && isOK();

   U_RETURN(result);
}

bool URDBClient_Base::commitTransaction()
{
   U_TRACE_NO_PARAM(0, "URDBClient_Base::commitTransaction()")

   reset();

   bool result = processRequest("CTRN") && isOK();

   U_RETURN(result);
}

// Write a key/value pair to a reliable database

int URDBClient_Base::store(const UString& key, const UString& data, int flag)
{
   U_TRACE(0, "URDBClient_Base::store(%V,%V,%d)", key.rep, data.rep, flag)

   int result    = -5;
   char token[5] = { 'S', 'T', 'R', '0', '\0' };

   if (flag == RDB_REPLACE) token[3] = '1';

   reset();

   URPC::rpc_info->push(key);
   URPC::rpc_info->push(data);

   if (processRequest(token))
      {
      switch (nResponseCode)
         {
         case 200: result =  0; break; //  0: Everything was OK
         case 401: result = -1; break; // -1: flag was RDB_INSERT and this key already existed
         case 500: result = -3; break; // -3: disk full writing to the journal file
         }
      }

   U_RETURN(result);
}

// Mark a key/value as deleted

int URDBClient_Base::remove(const UString& key)
{
   U_TRACE(0, "URDBClient_Base::remove(%V)", key.rep)

   int result = -5;

   reset();

   URPC::rpc_info->push(key);

   if (processRequest("REMV"))
      {
      switch (nResponseCode)
         {
         case 200: result =  0; break; //  0: Everything was OK
         case 400: result = -1; break; // -1: The entry was not in the database
         case 402: result = -2; break; // -2: The entry was already marked deleted in the hash-tree
         case 500: result = -3; break; // -3: disk full writing to the journal file
         }
      }

   U_RETURN(result);
}

// Substitute a key/value with a new key/value (remove+store)

int URDBClient_Base::substitute(const UString& key, const UString& new_key, const UString& data, int flag)
{
   U_TRACE(0, "URDBClient_Base::substitute(%V,%V,%V,%d)", key.rep, new_key.rep, data.rep, flag)

   int result    = -5;
   char token[5] = { 'S', 'U', 'B', '0', '\0' };

   if (flag == RDB_REPLACE) token[3] = '1';

   reset();

   URPC::rpc_info->push(key);
   URPC::rpc_info->push(new_key);
   URPC::rpc_info->push(data);

   if (processRequest(token))
      {
      switch (nResponseCode)
         {
         case 200: result =  0; break; //  0: Everything was OK
         case 400: result = -1; break; // -1: The entry was not in the database
         case 402: result = -2; break; // -2: The entry was marked deleted in the hash-tree
         case 500: result = -3; break; // -3: disk full writing to the journal file
         case 401: result = -4; break; // -4: flag was RDB_INSERT and the new key already existed
         }
      }

   U_RETURN(result);
}

// operator []

UString URDBClient_Base::operator[](const UString& key)
{
   U_TRACE(0, "URDBClient_Base::operator[](%V)", key.rep)

   reset();

   URPC::rpc_info->push(key);

   if (processRequest("FIND") && isOK()) U_RETURN_STRING(response);

   return UString::getStringNull();
}

// Call function for all entry

void URDBClient_Base::_callForAllEntry(vPFprpr function, bool sorted)
{
   U_TRACE(0, "URDBClient_Base::_callForAllEntry(%p,%b)", function, sorted)

   char token[5] = { 'P', 'R', 'T', '0', '\0' };

   if (sorted) token[3] = '1';

   reset();

   if (processRequest(token))
      {
      U_INTERNAL_ASSERT_EQUALS(nResponseCode, 200)

      UCDB::datum _key, _data;
      const char* ptr = response.data();

      while (*ptr == '+')
         {
         UStringRep* key;
         UStringRep* data;

         U_INTERNAL_ASSERT_MINOR(ptr, response.pend())

         ptr = URDB::parseLine(ptr, &_key, &_data);

         U_INTERNAL_ASSERT_MAJOR(_key.dsize,0)
         U_INTERNAL_ASSERT_MAJOR(_data.dsize,0)

         U_NEW(UStringRep, key,  UStringRep((const char*) _key.dptr,  _key.dsize));
         U_NEW(UStringRep, data, UStringRep((const char*)_data.dptr, _data.dsize));

         function(key, data);

          key->release();
         data->release();
         }
      }
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URDBClient_Base::dump(bool _reset) const
{
   UClient_Base::dump(false);

   *UObjectIO::os << '\n'
                  << "nResponseCode                       " << nResponseCode;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
