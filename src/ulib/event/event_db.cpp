// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//   event_db.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/event/event_db.h>
#include <ulib/net/server/server.h>

#ifdef U_STATIC_ORM_DRIVER_PGSQL
#  include <ulib/orm/driver/orm_driver_pgsql.h>
#endif

void UEventDB::setConnection(void* connection)
{
   U_TRACE(0, "UEventDB::setConnection(%p)", connection)

   conn = connection;

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   UEventFd::fd = U_SYSCALL(PQsocket, "%p", (PGconn*)conn);

   (void) U_SYSCALL(PQsetnonblocking, "%p,%u", (PGconn*)conn, 1);
#endif
}

void UEventDB::addClientImage(vPFpvu handlerResult, uint32_t num_query)
{
   U_TRACE(0, "UEventDB::addClientImage(%p,%u)", handlerResult, num_query)

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   (void) U_SYSCALL(PQflush, "%p", (PGconn*)conn);
#endif

#if defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   UClientImage_Base::setRequestProcessed();
#endif

   U_INTERNAL_DUMP("start = %u end = %u", start, end)

   query_info* pquery = query+end;

   pquery->handlerResult = handlerResult;
   pquery->pClientImage  = UServer_Base::pClientImage;
   pquery->num_query     = num_query;
   pquery->num_result    = 0;

   end = (end+1) & 511;

   U_INTERNAL_DUMP("start = %u end = %u", start, end)

   U_INTERNAL_ASSERT_DIFFERS(end, start)
}

int UEventDB::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UEventDB::handlerRead()")

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   if (U_SYSCALL(PQconsumeInput, "%p", (PGconn*)conn))
      {
      bool bopen;
      PGresult* result;
      query_info* pquery;

loop: U_INTERNAL_DUMP("start = %u end = %u", start, end)

      pquery = query+start;

      U_INTERNAL_DUMP("pquery->pClientImage = %p pquery->num_query = %u pquery->num_result = %u", pquery->pClientImage, pquery->num_query, pquery->num_result)

      bopen = pquery->pClientImage->isOpen();

      while (U_SYSCALL(PQisBusy, "%p", (PGconn*)conn) == 0)
         {
         result = (PGresult*) U_SYSCALL(PQgetResult, "%p", (PGconn*)conn);

         if (result == U_NULLPTR) U_RETURN(U_NOTIFIER_OK);

         pquery->num_result++;

         U_INTERNAL_DUMP("Result status: %d (%s) for num_result(%u), tuples(%d)", PQresultStatus(result), PQresStatus(PQresultStatus(result)), pquery->num_result, PQntuples(result))

         if (bopen) pquery->handlerResult(result, pquery->num_result);

         U_SYSCALL_VOID(PQclear, "%p", result);

         U_INTERNAL_DUMP("pquery->num_query = %u pquery->num_result = %u", pquery->num_query, pquery->num_result)

         if (pquery->num_query == pquery->num_result)
            {
            pquery->pClientImage->writeResponseCompact();

            start = (start+1) & 511;

            if (start == end)
               {
               U_INTERNAL_ASSERT_EQUALS(PQgetResult((PGconn*)conn), U_NULLPTR)

               U_RETURN(U_NOTIFIER_OK);
               }

            goto loop;
            }

         continue;
         }
      }
#endif

   U_RETURN(U_NOTIFIER_OK);
}

// DEBUG

#ifdef DEBUG
const char* UEventDB::dump(bool _reset) const
{
   *UObjectIO::os << "end   " << end         << '\n' 
                  << "conn  " << (void*)conn << '\n' 
                  << "start " << start;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
