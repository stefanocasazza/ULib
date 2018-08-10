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
      uint32_t           timestamp;
      uint16_t           num_query, num_result;
   } query_info;

   UEventDB()
      {
      U_TRACE_CTOR(0, UEventDB, "")

      conn = U_NULLPTR;

      start = end = 0;

      (void) U_SYSCALL(memset, "%p,%d,%u", query, 0, sizeof(query));

      bmode = false;
      }

   ~UEventDB();

   // SERVICES

   void handlerResult(vPFpvu handler);
   void setConnection(void* connection);
   void handlerQuery(uint32_t num_query = 1);

   // define method VIRTUAL

   virtual int handlerRead();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
   void* conn;
   uint32_t start, end;
   query_info query[512];
   int fd_conn;
   bool bmode;

   static bool basync;
   static uint32_t nquery;

#ifdef DEBUG
   static uint32_t max_num_handler;
#endif

   uint32_t getResult(void* vresult);

   U_DISALLOW_COPY_AND_ASSIGN(UEventDB)
};
#endif
