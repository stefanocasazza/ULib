// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    construct.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_CONSTRUCT_H
#define ULIB_CONSTRUCT_H 1

#include <ulib/string.h>

// default behaviour

template <class T> inline void u_construct(const T** ptr, bool stream_loading)
{
   U_TRACE(0, "u_construct<T>(%p,%b)", ptr, stream_loading)

   U_INTERNAL_ASSERT_POINTER(ptr)

#ifndef U_COVERITY_FALSE_POSITIVE // coverity[RESOURCE_LEAK]
   if (stream_loading) U_NEW_WITHOUT_CHECK_MEMORY(T, *ptr, T(**ptr))
#endif
}

template <class T> inline void u_construct(const T* ptr, uint32_t n)
{
   U_TRACE(0, "u_construct<T>(%p,%u)", ptr, n)
}

template <class T> inline void u_destroy(const T* ptr)
{
   U_TRACE(0, "u_destroy<T>(%p)", ptr)

#ifndef U_COVERITY_FALSE_POSITIVE // coverity[RESOURCE_LEAK]
   if (ptr <= (const void*)0x0000ffff) U_ERROR("u_destroy<T>(%p)", ptr);

   U_DELETE(ptr)
#endif
}

template <class T> inline void u_destroy(const T** ptr, uint32_t n)
{
   U_TRACE(0, "u_destroy<T>(%p,%u)", ptr, n)

#ifndef U_COVERITY_FALSE_POSITIVE // coverity[RESOURCE_LEAK]
   for (uint32_t i = 0; i < n; ++i)
      {
      U_INTERNAL_DUMP("ptr[%u] = %p", i, ptr[i])

      U_DELETE(ptr[i])
      }
#endif
}

template <> inline void u_construct(const UStringRep** prep, bool stream_loading)
{
   U_TRACE(0, "u_construct<UStringRep*>(%p,%b)", prep, stream_loading)

   U_VAR_UNUSED(stream_loading)

#ifndef U_COVERITY_FALSE_POSITIVE // coverity[RESOURCE_LEAK]
   ((UStringRep*)(*prep))->hold();
#endif
}

template <> inline void u_construct(const UStringRep* rep, uint32_t n)
{
   U_TRACE(0, "u_construct<UStringRep*>(%p,%u)", rep, n)

#ifndef U_COVERITY_FALSE_POSITIVE // coverity[RESOURCE_LEAK]
   ((UStringRep*)rep)->references += n;

   U_INTERNAL_DUMP("references = %d", rep->references + 1)
#endif
}

template <> inline void u_destroy(const UStringRep* rep)
{
   U_TRACE(0, "u_destroy<UStringRep*>(%V)", rep)

#ifndef U_COVERITY_FALSE_POSITIVE // coverity[RESOURCE_LEAK]
   ((UStringRep*)rep)->release();
#endif
}

template <> inline void u_destroy(const UStringRep** prep, uint32_t n)
{
   U_TRACE(0, "u_destroy<UStringRep*>(%p,%u)", prep, n)

   for (uint32_t i = 0; i < n; ++i) ((UStringRep*)prep[i])->release();
}
#endif
