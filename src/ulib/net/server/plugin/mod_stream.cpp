// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_stream.cpp - distributing realtime input
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/plugin/mod_stream.h>

U_CREAT_FUNC(server_plugin_stream, UStreamPlugIn)

pid_t                   UStreamPlugIn::pid = (pid_t)-1;
UString*                UStreamPlugIn::uri_path;
UString*                UStreamPlugIn::metadata;
UString*                UStreamPlugIn::content_type;
UCommand*               UStreamPlugIn::command;
URingBuffer*            UStreamPlugIn::rbuf;
URingBuffer::rbuf_data* UStreamPlugIn::ptr;

// 1M size ring buffer
#define U_RING_BUFFER_SIZE (1 * 1024 * 1024)

RETSIGTYPE UStreamPlugIn::handlerForSigTERM(int signo)
{
   U_TRACE(0, "[SIGTERM] UStreamPlugIn::handlerForSigTERM(%d)", signo)

   if (pid != -1) UProcess::kill(pid, SIGTERM);

   UInterrupt::sendOurselves(SIGTERM);
}

UStreamPlugIn::UStreamPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UStreamPlugIn, "")

   U_NEW(UString, uri_path,     UString);
   U_NEW(UString, content_type, UString);
}

UStreamPlugIn::~UStreamPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UStreamPlugIn)

   delete uri_path;
   delete content_type;

   if (command)
      {
                delete command;
      if (rbuf) delete rbuf;

      if (pid != -1) UProcess::kill(pid, SIGTERM);
      }

   if (metadata) delete metadata;
}

// Server-wide hooks

int UStreamPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UStreamPlugIn::handlerConfig(%p)", &cfg)

   // stream - plugin parameters
   // ------------------------------------------------------------------------------------------------------------------------
   // URI_PATH     specifies the local part of the URL path at which you would like the content to appear (Ex. /my/video.mjpeg)
   // METADATA     specifies the needs to have setup headers prepended for each codec stream (Ex. /my/audio.ogg)
   // CONTENT_TYPE specifies the Internet media type of the stream, which will appear in the Content-Type HTTP response header
   //
   // COMMAND                      command to execute
   // ENVIRONMENT  environment for command to execute
   // ------------------------------------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UString x = cfg.at(U_CONSTANT_TO_PARAM("METADATA"));

      if (x) U_NEW(UString, metadata, UString(x));

      *uri_path     = cfg.at(U_CONSTANT_TO_PARAM("URI_PATH"));
      *content_type = cfg.at(U_CONSTANT_TO_PARAM("CONTENT_TYPE"));

      command = UServer_Base::loadConfigCommand();

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UStreamPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UStreamPlugIn::handlerInit()")

   static int fd_stderr;

   if (command == 0) U_RETURN(U_PLUGIN_HANDLER_ERROR);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/UStreamPlugIn.err");

   bool result = command->execute(0, (UString*)-1, -1, fd_stderr);

#ifndef U_LOG_DISABLE
   UServer_Base::logCommandMsgError(command->getCommand(), true);
#endif

   if (result == false)
      {
      delete command;
             command = 0;

      U_RETURN(U_PLUGIN_HANDLER_ERROR);
      }

   ptr = (URingBuffer::rbuf_data*) UServer_Base::getOffsetToDataShare(sizeof(URingBuffer::rbuf_data) + U_RING_BUFFER_SIZE);

   (void) content_type->append(U_CONSTANT_TO_PARAM(U_CRLF));

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UStreamPlugIn::handlerRun()
{
   U_TRACE_NO_PARAM(0, "UStreamPlugIn::handlerRun()")

   U_INTERNAL_ASSERT_EQUALS(pid,-1)

   U_NEW(URingBuffer, rbuf, URingBuffer((URingBuffer::rbuf_data*) UServer_Base::getPointerToDataShare(ptr), U_RING_BUFFER_SIZE));

   // NB: we are feeding by a child of us...

   UProcess proc;

   if (proc.fork() &&
       proc.parent())
      {
      pid = proc.pid();

      /*
      pid_t pgid = U_SYSCALL_NO_PARAM(getpgrp);

      UProcess::setProcessGroup(pid, pgid);
      */

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   if (proc.child())
      {
      UTimeVal to_sleep(0L, 50 * 1000L);

      pid = UCommand::pid;

      UInterrupt::insert(SIGTERM, (sighandler_t)UStreamPlugIn::handlerForSigTERM); // async signal

      while (UNotifier::waitForRead(UProcess::filedes[2]) >= 1)
         {
         int nread = rbuf->readFromFdAndWrite(UProcess::filedes[2]);

         if (nread == 0) break;                // EOF
         if (nread  < 0) to_sleep.nanosleep(); // EAGAIN
         }

      handlerForSigTERM(SIGTERM);
      }

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int UStreamPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UStreamPlugIn::handlerRequest()")

   if (U_HTTP_URI_EQUAL(*uri_path) == false) U_RETURN(U_PLUGIN_HANDLER_GO_ON);

   U_http_info.nResponseCode = HTTP_OK;

   UHTTP::setResponse(*content_type, 0);

   UClientImage_Base::setCloseConnection();

   if (USocketExt::write(UServer_Base::csocket, *UClientImage_Base::wbuffer, UServer_Base::timeoutMS) &&
       UHTTP::isHEAD() == false)
      {
      int readd = rbuf->open();

      if (readd != -1)
         {
         if (UServer_Base::startParallelization())
            {
            // parent

            rbuf->close(readd);

            U_RETURN(U_PLUGIN_HANDLER_ERROR);
            }

         UTimeVal to_sleep(0L, 10 * 1000L);

         if (metadata) (void) USocketExt::write(UServer_Base::csocket, *metadata, UServer_Base::timeoutMS);

         while (UServer_Base::flag_loop)
            {
            if ( rbuf->isEmpty(readd) == false &&
                (rbuf->readAndWriteToFd(readd, UServer_Base::csocket->iSockDesc) <= 0 && errno != EAGAIN)) break;

            to_sleep.nanosleep();
            }

         rbuf->close(readd);
         }
      }

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UStreamPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "pid                       " << pid                 << '\n'
                  << "uri_path     (UString     " << (void*)uri_path     << ")\n"
                  << "content_type (UString     " << (void*)content_type << ")\n"
                  << "command      (UCommand    " << (void*)command      << ")\n"
                  << "rbuf         (URingBuffer " << (void*)rbuf         << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
