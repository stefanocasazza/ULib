// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client_image_rdb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/rdb.h>
#include <ulib/net/rpc/rpc.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/client_image_rdb.h>

// 2xx indicates success of some kind
#define STR_200 "200 "  // OK

// 4xx try again: not found, EOF, no database open, permission denied
#define STR_400 "400 "  // Lookup failed: the entry was not in the database
#define STR_401 "401 "  // Store failed: flag was insertion of new entries only and the key already existed
#define STR_402 "402 "  // Remove failed: the entry was already marked deleted

// 5xx go away: parse error, catastrophic error, ...
#define STR_500 "500 "  // Server error: requested action not taken

URDB* URDBClientImage::rdb;

int URDBClientImage::handlerRead()
{
   U_TRACE_NO_PARAM(0, "URDBClientImage::handlerRead()")

   if (UClientImage_Base::manageRead() == U_NOTIFIER_DELETE) U_RETURN(U_NOTIFIER_DELETE);

   if (U_ClientImage_state == U_PLUGIN_HANDLER_GO_ON)
      {
#  ifndef U_LOG_DISABLE
      if (UClientImage_Base::logbuf)
         {
         *UClientImage_Base::request = *UClientImage_Base::rbuffer;

         UClientImage_Base::logRequest();
         }
#  endif

      // check for RPC request

      URPC::resetInfo();

      if (URPC::readRequest(UClientImage_Base::socket) == false) U_RETURN(U_NOTIFIER_DELETE);

      UClientImage_Base::wbuffer->setBuffer(U_CAPACITY);

      // Process the RPC message

      int result;
      const char* res = STR_200;
      const char* ptr = UClientImage_Base::rbuffer->data();

      enum {
         RPC_METHOD_FIND               = U_MULTICHAR_CONSTANT32('F','I','N','D'),
         RPC_METHOD_STORE              = U_MULTICHAR_CONSTANT32('S','T','R','0'),
         RPC_METHOD_REPLACE            = U_MULTICHAR_CONSTANT32('S','T','R','1'),
         RPC_METHOD_REMOVE             = U_MULTICHAR_CONSTANT32('R','E','M','V'),
         RPC_METHOD_SUBSTITUTE0        = U_MULTICHAR_CONSTANT32('S','U','B','0'),
         RPC_METHOD_SUBSTITUTE1        = U_MULTICHAR_CONSTANT32('S','U','B','1'),
         RPC_METHOD_PRINT0             = U_MULTICHAR_CONSTANT32('P','R','T','0'),
         RPC_METHOD_PRINT1             = U_MULTICHAR_CONSTANT32('P','R','T','1'),
         RPC_METHOD_REORGANIZE         = U_MULTICHAR_CONSTANT32('R','O','R','G'),
         RPC_METHOD_BEGIN_TRANSACTION  = U_MULTICHAR_CONSTANT32('B','T','R','N'),
         RPC_METHOD_ABORT_TRANSACTION  = U_MULTICHAR_CONSTANT32('A','T','R','N'),
         RPC_METHOD_COMMIT_TRANSACTION = U_MULTICHAR_CONSTANT32('C','T','R','N')
      };

      switch (u_get_unalignedp32(ptr))
         {
         case RPC_METHOD_FIND:
            {
            if (rdb->find((*URPC::rpc_info)[0]))
               {
               // Build the response: 200

               uint32_t size = rdb->data.dsize;

               (void) UClientImage_Base::wbuffer->reserve(U_TOKEN_LN + size);

               UStringExt::buildTokenInt(res = STR_200, size, *UClientImage_Base::wbuffer);

               (void) UClientImage_Base::wbuffer->append((const char*)rdb->data.dptr, size);
               }
            else
               {
               // Build the response: 400

               UStringExt::buildTokenInt(res = STR_400, 0, *UClientImage_Base::wbuffer);
               }
            }
         break;

         case RPC_METHOD_STORE:
         case RPC_METHOD_REPLACE:
            {
            // ------------------------------------------------------
            // Write a key/value pair to a reliable database
            // ------------------------------------------------------
            // RETURN VALUE
            // ------------------------------------------------------
            //  0: Everything was OK
            // -1: flag was RDB_INSERT and this key already existed
            // -3: disk full writing to the journal file
            // ------------------------------------------------------
            // #define RDB_INSERT  0 // Insertion of new entries only
            // #define RDB_REPLACE 1 // Allow replacing existing entries
            // ------------------------------------------------------

            result = rdb->store((*URPC::rpc_info)[0], (*URPC::rpc_info)[1], ptr[3] == '0' ? RDB_INSERT : RDB_REPLACE);

            switch (result)
               {
               case  0: res = STR_200; break; //  0: Everything was OK
               case -1: res = STR_401; break; // -1: flag was RDB_INSERT and this key already existed
               case -3: res = STR_500; break; // -3: disk full writing to the journal file
               }

            UStringExt::buildTokenInt(res, 0, *UClientImage_Base::wbuffer);
            }
         break;

         case RPC_METHOD_REMOVE:
            {
            // ---------------------------------------------------------
            // Mark a key/value as deleted
            // ---------------------------------------------------------
            // RETURN VALUE
            // ---------------------------------------------------------
            //  0: Everything was OK
            // -1: The entry was not in the database
            // -2: The entry was already marked deleted in the hash-tree
            // -3: disk full writing to the journal file
            // ---------------------------------------------------------

            result = rdb->remove((*URPC::rpc_info)[0]);

            switch (result)
               {
               case  0: res = STR_200; break; //  0: Everything was OK
               case -1: res = STR_400; break; // -1: The entry was not in the database
               case -2: res = STR_402; break; // -2: The entry was already marked deleted in the hash-tree
               case -3: res = STR_500; break; // -3: disk full writing to the journal file
               }

            UStringExt::buildTokenInt(res, 0, *UClientImage_Base::wbuffer);
            }
         break;

         case RPC_METHOD_SUBSTITUTE0:
         case RPC_METHOD_SUBSTITUTE1:
            {
            // ----------------------------------------------------------
            // Substitute a key/value with a new key/value (remove+store)
            // ----------------------------------------------------------
            // RETURN VALUE
            // ----------------------------------------------------------
            //  0: Everything was OK
            // -1: The entry was not in the database
            // -2: The entry was marked deleted in the hash-tree
            // -3: disk full writing to the journal file
            // -4: flag was RDB_INSERT and the new key already existed
            // ----------------------------------------------------------

            // #define RDB_INSERT  0 // Insertion of new entries only
            // #define RDB_REPLACE 1 // Allow replacing existing entries

            result = rdb->substitute((*URPC::rpc_info)[0], (*URPC::rpc_info)[1], (*URPC::rpc_info)[2], ptr[3] == '0' ? RDB_INSERT : RDB_REPLACE);

            switch (result)
               {
               case  0: res = STR_200; break; //  0: Everything was OK
               case -1: res = STR_400; break; // -1: The entry was not in the database
               case -2: res = STR_402; break; // -2: The entry was marked deleted in the hash-tree
               case -3: res = STR_500; break; // -3: disk full writing to the journal file
               case -4: res = STR_401; break; // -4: flag was RDB_INSERT and the new key already existed
               }

            UStringExt::buildTokenInt(res, 0, *UClientImage_Base::wbuffer);
            }
         break;

         case RPC_METHOD_PRINT0:
         case RPC_METHOD_PRINT1:
            {
            // Build the response: 200

            UString tmp = (ptr[3] == '0' ? rdb->print() : rdb->printSorted());

            UStringExt::buildTokenInt(res = STR_200, tmp.size(), *UClientImage_Base::wbuffer);

            UClientImage_Base::wbuffer->append(tmp);
            }
         break;

         case RPC_METHOD_REORGANIZE:
            {
            res = (rdb->reorganize() ? STR_200 : STR_500);

            UStringExt::buildTokenInt(res, 0, *UClientImage_Base::wbuffer);
            }
         break;

         case RPC_METHOD_BEGIN_TRANSACTION:
            {
            res = (rdb->beginTransaction() ? STR_200 : STR_500);

            UStringExt::buildTokenInt(res, 0, *UClientImage_Base::wbuffer);
            }
         break;

         case RPC_METHOD_ABORT_TRANSACTION:
            {
            rdb->abortTransaction();

            UStringExt::buildTokenInt(res = STR_200, 0, *UClientImage_Base::wbuffer);
            }
         break;

         case RPC_METHOD_COMMIT_TRANSACTION:
            {
            rdb->commitTransaction();

            UStringExt::buildTokenInt(res = STR_200, 0, *UClientImage_Base::wbuffer);
            }
         break;

         default:
            {
            UStringExt::buildTokenInt(res = STR_500, 0, *UClientImage_Base::wbuffer);
            }
         break;
         }

      U_SRV_LOG_WITH_ADDR("method %.4S return %s for", ptr, res);

      return UClientImage_Base::handlerResponse();
      }

   U_RETURN(U_NOTIFIER_OK);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URDBClientImage::dump(bool _reset) const
{
   UClientImage<UTCPSocket>::dump(false);

   *UObjectIO::os << '\n'
                  << "rdb             (URDB              " << (void*)rdb << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
