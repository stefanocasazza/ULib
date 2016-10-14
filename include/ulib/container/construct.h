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

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   if (stream_loading) U_NEW(T, *ptr, T(**ptr));
#endif
}

template <class T> inline void u_construct(const T* ptr, uint32_t n)
{
   U_TRACE(0, "u_construct<T>(%p,%u)", ptr, n)
}

template <class T> inline void u_destroy(const T* ptr)
{
   U_TRACE(0, "u_destroy<T>(%p)", ptr)

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   if (ptr <= (const void*)0x0000ffff) U_ERROR("u_destroy<T>(%p)", ptr);

   delete ptr;
#endif
}

template <class T> inline void u_destroy(const T** ptr, uint32_t n)
{
   U_TRACE(0, "u_destroy<T>(%p,%u)", ptr, n)

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   for (uint32_t i = 0; i < n; ++i) delete ptr[i];
#endif
}

template <> inline void u_construct(const UStringRep** prep, bool stream_loading)
{
   U_TRACE(0, "u_construct<UStringRep*>(%p,%b)", prep, stream_loading)

   U_VAR_UNUSED(stream_loading)

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   ((UStringRep*)(*prep))->hold();
#endif
}

template <> inline void u_construct(const UStringRep* rep, uint32_t n)
{
   U_TRACE(0, "u_construct<UStringRep*>(%p,%u)", rep, n)

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   ((UStringRep*)rep)->references += n;

   U_INTERNAL_DUMP("references = %d", rep->references + 1)
#endif
}

template <> inline void u_destroy(const UStringRep* rep)
{
   U_TRACE(0, "u_destroy<UStringRep*>(%p)", rep)

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   ((UStringRep*)rep)->release();
#endif
}

template <> inline void u_destroy(const UStringRep** rep, uint32_t n)
{
   U_TRACE(0, "u_destroy<UStringRep*>(%p,%u)", rep, n)

// coverity[RESOURCE_LEAK]
#ifndef U_COVERITY_FALSE_POSITIVE
   for (uint32_t i = 0; i < n; ++i) ((UStringRep*)rep[i])->release();
#endif
}

#endif
