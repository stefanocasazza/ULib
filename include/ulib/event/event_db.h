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

#define U_QUERY_INFO_SZ 168

class UServer_Base;
class UClientImage_Base;

class U_EXPORT UEventDB : public UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

    UEventDB();
   ~UEventDB();

   // SERVICES

   void reset();
   void setConnection(void* connection);
   void handlerQuery(vPFpvu handler, uint32_t num_query = 1);

   bool isEmpty()
      {
      U_TRACE_NO_PARAM(0, "UEventDB::isEmpty()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("num_handler = %u", num_handler)

      U_INTERNAL_ASSERT_MINOR(num_handler, U_QUERY_INFO_SZ)

      if (num_handler) U_RETURN(false);

      U_RETURN(true);
      }

   // define method VIRTUAL

   virtual int handlerRead();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   typedef struct query_info {
      vPFpvu handlerResult;
      UClientImage_Base* pClientImage;
      uint16_t num_query;
#  ifdef DEBUG
      uint32_t timestamp;
#  endif
   } query_info;

   void* conn;
   uint8_t* pbusy;
   uint16_t num_result, num_handler;
   bool bsend, bnotifier;
   query_info query[U_QUERY_INFO_SZ];
#ifdef DEBUG
   const void* pthis;
#endif

#ifdef DEBUG
   static uint32_t max_num_handler;
#endif

private:
   friend class UServer_Base;

   U_DISALLOW_COPY_AND_ASSIGN(UEventDB)
};
#endif
