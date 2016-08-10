// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_stream.h - distributing realtime input
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_STREAM_H
#define U_MOD_STREAM_H 1

#include <ulib/utility/ring_buffer.h>
#include <ulib/net/server/server_plugin.h>

class UCommand;

class U_EXPORT UStreamPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            UStreamPlugIn();
   virtual ~UStreamPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;
   virtual int handlerRun() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static pid_t pid;
   static URingBuffer* rbuf;
   static UCommand* command;
   static UString* uri_path;
   static UString* metadata;
   static UString* content_type;
   static URingBuffer::rbuf_data* ptr;

   static RETSIGTYPE handlerForSigTERM(int signo);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UStreamPlugIn)
};

#endif
