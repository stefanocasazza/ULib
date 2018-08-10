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

bool     UEventDB::basync;
uint32_t UEventDB::nquery;
#ifdef DEBUG
uint32_t UEventDB::max_num_handler;
#endif

UEventDB::~UEventDB()
{
   U_TRACE_DTOR(0, UEventDB)

#ifdef DEBUG
   (void) UFile::writeToTmp(u_buffer, u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("max_num_handler: %u"), max_num_handler),
                            O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("event_db.%P"), 0);
#endif
}

void UEventDB::setConnection(void* connection)
{
   U_TRACE(0, "UEventDB::setConnection(%p)", connection)

   U_CHECK_MEMORY

   conn = connection;

   start = end = 0;

   bmode = false;

   U_INTERNAL_DUMP("basync = %b", basync)

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   fd_conn = U_SYSCALL(PQsocket, "%p", (PGconn*)conn);

   if (basync) UEventFd::fd = fd_conn;
#endif
}

void UEventDB::handlerQuery(uint32_t num_query)
{
   U_TRACE(0, "UEventDB::handlerQuery(%u)", num_query)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("start = %u end = %u", start, end)

   U_INTERNAL_ASSERT_MINOR(end,   512)
   U_INTERNAL_ASSERT_MINOR(start, 512)

#if defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   UClientImage_Base::setRequestProcessed();
#endif

   U_INTERNAL_DUMP("basync = %b bmode = %b", basync, bmode)

   nquery = num_query;

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   if (bmode)
      {
      if (num_query == 1)
         {
         bmode = false;

         (void) U_SYSCALL(PQexitQueueMode, "%p",     (PGconn*)conn);
         (void) U_SYSCALL(PQsetnonblocking, "%p,%u", (PGconn*)conn, 0);
         }
      }
   else
      {
      if (num_query > 1)
         {
         bmode = true;

         (void) U_SYSCALL(PQsetnonblocking, "%p,%u", (PGconn*)conn, 1);
         (void) U_SYSCALL(PQenterQueueMode, "%p",    (PGconn*)conn);
         }
      }

   if (basync)
      {
      query_info* pquery = query+end;

      pquery->pClientImage = UServer_Base::pClientImage;
      pquery->num_query    = num_query;
      pquery->num_result   = 0;
      pquery->timestamp    = u_now->tv_sec;
      }
#endif
}

uint32_t UEventDB::getResult(void* vresult)
{
   U_TRACE(0, "UEventDB::getResult(%p)", vresult)

   U_CHECK_MEMORY

   uint32_t num_result = 0;

   U_INTERNAL_DUMP("basync = %b bmode = %b", basync, bmode)

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   if (U_SYSCALL(PQconsumeInput, "%p", (PGconn*)conn) == 1)
      {
      PGresult* result;

      while (U_SYSCALL(PQisBusy, "%p", (PGconn*)conn) == 0)
         {
         if (bmode) (void) U_SYSCALL(PQprocessQueue, "%p", (PGconn*)conn);

         result = (PGresult*) U_SYSCALL(PQgetResult, "%p", (PGconn*)conn);

         if (result == U_NULLPTR) break;

         U_INTERNAL_DUMP("Result[%u] status: %d (%s), tuples(%d), fields(%d)", num_result, PQresultStatus(result), PQresStatus(PQresultStatus(result)),
                           PQntuples(result), PQnfields(result))

         U_INTERNAL_ASSERT_EQUALS(PQresultStatus(result), PGRES_TUPLES_OK)

         ((PGresult**)vresult)[num_result++] = result;

         U_INTERNAL_ASSERT_RANGE(1,num_result,1023)
         }
      }
#endif

   U_RETURN(num_result);
}

void UEventDB::handlerResult(vPFpvu handler)
{
   U_TRACE(0, "UEventDB::handlerQuery(%p)", handler)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("start = %u end = %u", start, end)

   U_INTERNAL_ASSERT_MINOR(end,   512)
   U_INTERNAL_ASSERT_MINOR(start, 512)

   U_INTERNAL_DUMP("basync = %b bmode = %b", basync, bmode)

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   if (bmode) (void) U_SYSCALL(PQsendQueue, "%p", (PGconn*)conn);

   if (basync)
      {
      query[end].handlerResult = handler;

      end = (end+1) & 511;

      U_INTERNAL_DUMP("start = %u end = %u", start, end)

      U_INTERNAL_ASSERT_MINOR(end, 512)
      U_INTERNAL_ASSERT_DIFFERS(end, start)
      }
   else
      {
      if (UNotifier::waitForRead(fd_conn))
         {
         PGresult*  result;
         PGresult* vresult[1024];

         uint32_t num_result = getResult(vresult);

         U_INTERNAL_ASSERT_EQUALS(nquery, num_result)

         for (uint32_t i = 0; i < num_result; ++i)
            {
            handler(result = vresult[i], i+1);

            U_SYSCALL_VOID(PQclear, "%p", result);
            }

         UServer_Base::pClientImage->writeResponseCompact();
         }
      }
#endif
}

int UEventDB::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UEventDB::handlerRead()")

   U_CHECK_MEMORY

#ifdef U_STATIC_ORM_DRIVER_PGSQL
   PGresult* result;
   query_info* pquery;
   PGresult* vresult[1024];
   bool bopen[1024], bcheck;
   uint32_t i, index = start, num_open = 0, num_handler = (end >= start ? end-start : (end+512)-start);

   U_INTERNAL_DUMP("start = %u end = %u num_handler = %u", start, end, num_handler)

   U_INTERNAL_ASSERT_MINOR(end,         512)
   U_INTERNAL_ASSERT_MINOR(start,       512)
   U_INTERNAL_ASSERT_MINOR(num_handler, 512)

#ifdef DEBUG
   if (max_num_handler < num_handler) max_num_handler = num_handler;
#endif

   uint32_t num_result = getResult(vresult);

   U_INTERNAL_ASSERT_RANGE(1,num_result,1023)
   U_INTERNAL_ASSERT(num_result <= num_handler)

   for (i = 0; i < num_handler; ++i)
      {
      pquery = query+index;

      if ((bopen[i] = pquery->pClientImage->isOpen())) ++num_open;

      U_INTERNAL_DUMP("bopen[%u] = %b pquery->num_query = %u pquery->num_result = %u (now-timestamp) = %#2D",
                         i, bopen[i], pquery->num_query,     pquery->num_result, u_now->tv_sec - pquery->timestamp)

      index = (index+1) & 511;

      U_INTERNAL_ASSERT_MINOR(index, 512)
      }

   U_INTERNAL_DUMP("num_open = %u", num_open)

   if (num_open == 0)
      {
      start = end = 0;

      for (i = 0; i < num_result; ++i) U_SYSCALL_VOID(PQclear, "%p", vresult[i]);

      U_RETURN(U_NOTIFIER_OK);
      }

   U_INTERNAL_ASSERT_RANGE(1,num_open,1023)
   U_INTERNAL_ASSERT(num_open <= num_handler)

        i = 0;
   bcheck = (num_open != num_result);

   U_INTERNAL_DUMP("bcheck = %b", bcheck)

loop:
   if (bopen[i] == false) goto next;

   pquery = query+start;

   if (bcheck &&
      ((u_now->tv_sec - pquery->timestamp) > 10)) // 10 second connection/read timeout
      {
      U_INTERNAL_DUMP("num_open = %u num_result = %u", num_open, num_result)

      if (--num_open == num_result) bcheck = false;

      pquery->pClientImage->abortive_close();

      goto next;
      }

   pquery->handlerResult(result = vresult[i++], pquery->num_result += 1);

   U_SYSCALL_VOID(PQclear, "%p", result);

   if (pquery->num_query == pquery->num_result)
      {
      pquery->pClientImage->writeResponseCompact();

next: start = (start+1) & 511;

      U_INTERNAL_ASSERT_MINOR(start, 512)

      if (start == end)
         {
         U_INTERNAL_DUMP("i = %u num_result = %u", i, num_result)

         U_INTERNAL_ASSERT_EQUALS(i, num_result)

         U_RETURN(U_NOTIFIER_OK);
         }
      }

   U_INTERNAL_DUMP("i = %u num_result = %u", i, num_result)

   if (i < num_result) goto loop;
#endif

   U_RETURN(U_NOTIFIER_OK);
}

// DEBUG

#ifdef DEBUG
const char* UEventDB::dump(bool _reset) const
{
   *UObjectIO::os << "end     " << end         << '\n' 
                  << "conn    " << (void*)conn << '\n' 
                  << "bmode   " << bmode       << '\n' 
                  << "start   " << start       << '\n'
                  << "fd_conn " << fd_conn;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
