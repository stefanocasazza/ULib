// =================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    magic.h - interface to the libmagic library (Magic Number Recognition Library)
//
// = AUTHOR
//    Stefano Casazza
//
// =================================================================================

#ifndef U_MAGIC_H
#define U_MAGIC_H 1

#include <ulib/string.h>

#include <magic.h>

class UHttpClient_Base;
class UMimeMultipartMsg;

// identify a file's format by scanning binary data for patterns

class U_EXPORT UMagic {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UMagic(int flags)
      {
      U_TRACE_REGISTER_OBJECT(0, UMagic, "%d", flags)

      if (magic == 0) (void) init();

      U_INTERNAL_ASSERT_POINTER(magic)

      (void) setFlags(flags);
      }

   ~UMagic()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMagic)
      }

   static void clear()
      {
      U_TRACE_NO_PARAM(1, "UMagic::clear()")

      if (magic)
         {
         U_SYSCALL_VOID(magic_close, "%p", magic);

         magic = 0;
         }
      }

   static const char* getError()
      {
      U_TRACE_NO_PARAM(1, "UMagic::getError()")

      U_INTERNAL_ASSERT_POINTER(magic)

      const char* result = (const char*) U_SYSCALL(magic_error, "%p", magic);

      U_RETURN(result);
      }

   static bool setFlags(int flags = MAGIC_NONE)
      {
      U_TRACE(1, "UMagic::setFlags(%d)", flags)

      U_INTERNAL_ASSERT_POINTER(magic)

      bool result = (U_SYSCALL(magic_setflags, "%p,%d", magic, flags) != -1);

      U_RETURN(result);
      }

   static bool init(int flags = MAGIC_MIME);

   static UString getType(const char* buffer, uint32_t buffer_len);

   static UString getType(const UString& buffer) { return getType(U_STRING_TO_PARAM(buffer)); }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static magic_t magic; /* pointer to magic :-) */

private:
   U_DISALLOW_COPY_AND_ASSIGN(UMagic)

   friend class UHttpClient_Base;
   friend class UMimeMultipartMsg;
};

#endif
