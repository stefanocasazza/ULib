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

#ifdef DEBUG
uint32_t UEventDB::max_num_handler;
#endif

UEventDB::UEventDB()
{
   U_TRACE_CTOR(0, UEventDB, "")

    conn = U_NULLPTR;
   pbusy = U_NULLPTR;

   (void) U_SYSCALL(memset, "%p,%d,%u", query, 0, sizeof(query));

   num_result  =
   num_handler = 0;

   bsend     = false;
   bnotifier = true;
}

UEventDB::~UEventDB()
{
   U_TRACE_DTOR(0, UEventDB)

#ifdef DEBUG
   if (max_num_handler > 1)
      {
      (void) UFile::writeToTmp(u_buffer, u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("max_num_handler: %u"), max_num_handler),
                               O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("event_db.%P"), 0);
      }
#endif
}

void UEventDB::setConnection(void* connection)
{
   U_TRACE(0, "UEventDB::setConnection(%p)", connection)

   U_CHECK_MEMORY

   conn = connection;

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   UEventFd::fd = U_SYSCALL(PQsocket, "%p", (PGconn*)conn);

   (void) U_SYSCALL(PQsetnonblocking, "%p,%u", (PGconn*)conn, 1);
   (void) U_SYSCALL(PQenterQueueMode, "%p",    (PGconn*)conn);
#endif
}

void UEventDB::reset()
{
   U_TRACE_NO_PARAM(0, "UEventDB::reset()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("num_handler = %u", num_handler)

   U_INTERNAL_ASSERT_MINOR(num_handler, U_QUERY_INFO_SZ)

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   bsend = false;

   num_result = 0;

   if (num_handler)
      {
      U_DEBUG("UEventDB::reset(): num_handler = %u", num_handler);

      for (uint32_t i = 0; i < num_handler; ++i)
         {
         query_info* pquery = query+i;

         if (pquery->pClientImage->isOpen())
            {
            U_DEBUG("UEventDB::reset(): pquery->num_query = %u (now-timestamp) = %#2D", pquery->num_query, u_now->tv_sec - pquery->timestamp);

            U_INTERNAL_DUMP("pquery->num_query = %u (now-timestamp) = %#2D", pquery->num_query, u_now->tv_sec - pquery->timestamp)

            pquery->pClientImage->UEventFd::fd = -1;

            pquery->pClientImage->socket->abortive_close();
            }
         }

      num_handler = 0;

      U_SYSCALL_VOID(PQresetQueue, "%p", (PGconn*)conn);
      }
#endif
}

void UEventDB::handlerQuery(vPFpvu handler, uint32_t num_query)
{
   U_TRACE(0, "UEventDB::handlerQuery(%p,%u)", handler, num_query)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("num_handler = %u", num_handler)

   U_INTERNAL_ASSERT_POINTER(pbusy)
   U_INTERNAL_ASSERT_MINOR(num_handler, U_QUERY_INFO_SZ)

#if defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   UClientImage_Base::setRequestProcessed();
#endif

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   query_info* pquery = query+num_handler++;

   pquery->handlerResult = handler;
   pquery->pClientImage  = UServer_Base::pClientImage;
   pquery->num_query     = num_query;
#ifdef DEBUG
   pquery->timestamp     = u_now->tv_sec;
#endif

   num_result += num_query;

   U_INTERNAL_DUMP("num_result = %u bsend = %b bnotifier = %b busy = %b", num_result, bsend, bnotifier, *pbusy)

   U_INTERNAL_ASSERT_MINOR(num_result, 4096)

   if (num_query > 20       ||
       (num_result  >= 2048 ||
        num_handler == U_QUERY_INFO_SZ))
      {
      U_DEBUG("UEventDB::handlerQuery(%u): num_result(%u), num_handler(%u)", num_query, num_result, num_handler);

      if (bsend == false)
         {
         bsend = true;

         (void) U_SYSCALL(PQsendQueue, "%p", (PGconn*)conn);
         }

      while (UNotifier::waitForRead(UEventFd::fd))
         {
         if (*pbusy)
            {
            U_DEBUG("UEventDB::handlerQuery(%u): num_result(%u), num_handler(%u), busy == true", num_query, num_result, num_handler);

            continue;
            }

         *pbusy = true;

         handlerRead();

         *pbusy = false;

         return;
         }

      U_ERROR("UEventDB::handlerQuery(%u): num_result(%u), num_handler(%u), queue is full, I must exit...", num_query, num_result, num_handler);
      }

   if (bnotifier == false &&
          *pbusy == false)
      {
      bsend = false;
      }

   if (bsend == false)
      {
      bsend = true;

      if (bnotifier == false)
         {
         bnotifier = true;

         UNotifier::resume(this, EPOLLIN|EPOLLET);
         }

      (void) U_SYSCALL(PQsendQueue, "%p", (PGconn*)conn);
      }
#endif
}

int UEventDB::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UEventDB::handlerRead()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("num_handler = %u", num_handler)

   U_INTERNAL_ASSERT_POINTER(pbusy)
   U_INTERNAL_ASSERT(num_handler <= U_QUERY_INFO_SZ)

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   U_INTERNAL_DUMP("bsend = %b bnotifier = %b busy = %b", bsend, bnotifier, *pbusy)

   U_INTERNAL_ASSERT(bnotifier)

#ifdef DEBUG
   if (max_num_handler < num_handler) max_num_handler = num_handler;
#endif

   pid_t pid = (num_handler <= 1 || *pbusy ? 1 : (*pbusy = true, UServer_Base::startNewChild()));

   if (pid > 1) // parent
      {
      num_result  =
      num_handler = 0;

      bnotifier = false;

      UNotifier::suspend(this);

      U_SYSCALL_VOID(PQresetQueue, "%p", (PGconn*)conn);

      U_RETURN(U_NOTIFIER_OK);
      }

   bool bopen;
   PGresult* result;
   query_info* pquery;
   PGresult* vresult[4096];
   uint32_t i, j, k, n, status, vresult_size = 0;

   U_INTERNAL_DUMP("num_result = %u", num_result)

   U_INTERNAL_ASSERT_MINOR(num_result, 4096)

read:
   if (U_SYSCALL(pqReadData, "%p", (PGconn*)conn) == 1)
      {
      U_SYSCALL_VOID(pqParseInput3, "%p", (PGconn*)conn);

      while (U_SYSCALL(PQprocessQueue, "%p", (PGconn*)conn) == 1)
         {
         result = (PGresult*) U_SYSCALL(PQgetResult, "%p", (PGconn*)conn);

         U_INTERNAL_ASSERT_POINTER(result)

         status = PQresultStatus(result);

         U_INTERNAL_DUMP("Result[%u] status: %d (%s), tuples(%d), fields(%d)", vresult_size, status, PQresStatus(PQresultStatus(result)), PQntuples(result), PQnfields(result))

         if (status == PGRES_TUPLES_OK)
            {
            ((PGresult**)vresult)[vresult_size++] = result;

            U_INTERNAL_ASSERT_MINOR(vresult_size, 4096)
            }
         else if (status == 10)
            {
            if (vresult_size < num_result)
               {
               (void) U_SYSCALL(PQsendQueue, "%p", (PGconn*)conn);

               if (UNotifier::waitForRead(UEventFd::fd)) goto read;
               }

#        ifdef DEBUG
            result = (PGresult*) U_SYSCALL(PQgetResult, "%p", (PGconn*)conn);

            U_INTERNAL_ASSERT_EQUALS(result, U_NULLPTR)
#        endif

            goto next;
            }
         }
      }

   U_DEBUG("UEventDB::handlerRead(): vresult_size = %u num_result = %u pid = %u conn->errorMessage = %S", vresult_size, num_result, pid, PQerrorMessage((PGconn*)conn));

next:
   *pbusy = false;

   n = U_min(num_handler, vresult_size);

   U_INTERNAL_DUMP("n = %u", n)

   for (k = i = 0; k < n; ++i)
      {
      bopen = (pquery = (query+i))->pClientImage->isOpen();

#  ifdef DEBUG
      if (((u_now->tv_sec - pquery->timestamp) > 10)) // 10 second connection/read timeout
         {
         U_WARNING("UEventDB::handlerRead(): (now-timestamp) = %#2D > 10 isOpen() = %b", u_now->tv_sec - pquery->timestamp, bopen);

         /*
         if (bopen)
            {
            bopen = false;

            pquery->pClientImage->UEventFd::fd = -1;

            pquery->pClientImage->socket->abortive_close();
            }
         */
         }
#  endif

      if (bopen)
         {
         for (j = 0; j < pquery->num_query; ++j, ++k)
            {
            pquery->handlerResult(result = vresult[k], j+1);

            if (pid) U_SYSCALL_VOID(PQclear, "%p", result);
            }

         pquery->pClientImage->writeResponseCompact();
         }
      else
         {
         if (pid == 0) k += pquery->num_query;
         else
            {
            for (j = 0; j < pquery->num_query; ++j, ++k)
               {
               U_SYSCALL_VOID(PQclear, "%p", vresult[k]);
               }
            }
         }

      U_INTERNAL_DUMP("i = %u k = %u vresult_size = %u num_handler = %u", i+1, k, vresult_size, num_handler)
      }

   for (; i < num_handler; ++i)
      {
      if ((pquery = (query+i))->pClientImage->isOpen())
         {
         pquery->pClientImage->UEventFd::fd = -1;

         pquery->pClientImage->socket->abortive_close();
         }
      }

   if (pid)
      {
      for (; k < vresult_size; ++k)
         {
         U_SYSCALL_VOID(PQclear, "%p", vresult[k]);
         }
      }

        if (pid == 0) UServer_Base::endNewChild();
   else if (pid == 1)
      {
      bsend = false;

      num_result  =
      num_handler = 0;
      }
#endif

   U_RETURN(U_NOTIFIER_OK);
}

// DEBUG

#ifdef DEBUG
const char* UEventDB::dump(bool _reset) const
{
   *UObjectIO::os << "bsend       " << bsend      << '\n'
                  << "bnotifier   " << bnotifier  << '\n'
                  << "num_result  " << num_result << '\n'
                  << "num_handler " << num_handler;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
