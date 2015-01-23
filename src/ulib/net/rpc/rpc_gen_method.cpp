// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_gen_method.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/utility/services.h>
#include <ulib/net/rpc/rpc_object.h>
#include <ulib/net/rpc/rpc_envelope.h>

bool URPCGenericMethod::execute(URPCEnvelope& theCall)
{
   U_TRACE(0, "URPCGenericMethod::execute(%p)", &theCall)

   U_ASSERT(theCall.getMethodName() == getMethodName())

   static int fd_stderr;

   bool result;
   UString input;
   UString* pinput;
   int32_t old_ncmd = command->getNumArgument();
   uint32_t i = 0, num_arguments = theCall.getNumArgument();

   U_INTERNAL_DUMP("old_ncmd = %u num_arguments = %u", old_ncmd, num_arguments)

   if (URPCObject::isStdInput(response_type) == false) pinput = 0;
   else
      {
      U_INTERNAL_ASSERT_MAJOR(num_arguments, 0)

      input = theCall.getArgument(i++);

      U_INTERNAL_ASSERT(input)

      pinput = &input;
      }

   for (; i < num_arguments; ++i) command->addArgument(theCall.getArgumentCStr(i));

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/URPCGenericMethod.err");

   if (URPCObject::isSuccessOrFailure(response_type)) result = command->executeAndWait(pinput, -1, fd_stderr);
   else
      {
      response.setEmpty();

      result = command->execute(pinput, &response, -1, fd_stderr);
      }

   command->setNumArgument(old_ncmd, true); // we need to free strndup() malloc...

   if (result == false)
      {
      uint32_t n;
      const char* s;

      if (UCommand::isStarted())
         {
         s =                 "command failed";
         n = U_CONSTANT_SIZE("command failed");
         }
      else
         {
         s =                 "command not started";
         n = U_CONSTANT_SIZE("command not started");
         }

      (void) UCommand::setMsgError(command->getCommand(), false);

      URPCObject::dispatcher->setFailed();

      URPCMethod::pFault->setDetail();
      URPCMethod::pFault->setFaultCode();
      URPCMethod::pFault->setFaultReason(s, n);
      URPCMethod::pFault->encode(response);

      U_RETURN(false);
      }

   if (URPCObject::isSuccessOrFailure(response_type)) response.replace(UCommand::exit_value == 0 ? '1' : '0');

   U_RETURN(true);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCGenericMethod::dump(bool reset) const
{
   URPCMethod::dump(false);

   *UObjectIO::os << '\n'
                  << "response_type            " << response_type    << '\n'
                  << "command     (UCommand    " << (void*)command   << ")\n"
                  << "response    (UString     " << (void*)&response << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
