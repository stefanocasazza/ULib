// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event_db.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_EVENT_DB_H
#define ULIB_EVENT_DB_H 1

#include <ulib/event/event_fd.h>

class UClientImage_Base;

class U_EXPORT UEventDB : public UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   typedef struct query_info {
      vPFpvu             handlerResult;
      UClientImage_Base* pClientImage;
      uint32_t           num_query, num_result;
   } query_info;

   UEventDB()
      {
      U_TRACE_CTOR(0, UEventDB, "")

      conn = U_NULLPTR;

      start = end = 0;

      (void) U_SYSCALL(memset, "%p,%d,%u", query, 0, sizeof(query));
      }

   ~UEventDB()
      {
      U_TRACE_DTOR(0, UEventDB)
      }

   // SERVICES

   void setConnection(void* connection);
   void addClientImage(vPFpvu handlerResult, uint32_t num_query = 1);

   // define method VIRTUAL

   virtual int handlerRead();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
   void* conn;
   uint32_t start, end;
   query_info query[512];

   U_DISALLOW_COPY_AND_ASSIGN(UEventDB)
};
#endif
