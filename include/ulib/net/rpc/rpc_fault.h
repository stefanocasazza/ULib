// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_fault.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_FAULT_H
#define ULIB_RPC_FAULT_H 1

#include <ulib/string.h>

/**
 * @class URPCFault
 */

class U_EXPORT URPCFault {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum FaultCode { Sender, Receiver, DataEncodingUnknown, MustUnderstand, VersionMismatch };

   URPCFault() : detail(U_CAPACITY)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCFault, "", 0)

      faultCode = Sender;
      }

   virtual ~URPCFault()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCFault)
      }

   // Gets the general reason why the call failed.
   // Returns a string combining the generic fault and any more specific fault data

   UString getFaultCode();

   // Sets the generic fault code

   void setFaultCode(FaultCode code = Receiver) { faultCode = code; }

   // Allows the caller to set the generic and specific reasons for the failure of a call

   void setDetail()
      {
      U_TRACE_NO_PARAM(0, "URPCFault::setDetail()")

      U_INTERNAL_ASSERT_MAJOR(u_buffer_len, 0)

      detail.snprintf(U_CONSTANT_TO_PARAM("%.*s"), u_buffer_len, u_buffer);

      u_buffer_len = 0;
      }

   void setDetail(const char* format, uint32_t fmt_size, ...);

   UString& getDetail()      { return detail; }
   UString& getFaultReason() { return faultReason; }

   void setFaultReason(const char* s, uint32_t n)
      {
      U_TRACE(0, "URPCFault::setFaultReason(%.*S,%u)", n, s, n)

      U_INTERNAL_ASSERT_MAJOR(n, 0)

      (void) faultReason.assign(s, n);
      }

   // Encodes the complete fault into a string

   virtual void encode(UString& response);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString detail,         // Details behind why the fault occurred
           faultReason;    // Description of the fault
   FaultCode faultCode;    // Generic reason the call failed

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCFault)
};

#endif
