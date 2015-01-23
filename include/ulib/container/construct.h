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

   if (stream_loading) *ptr = U_NEW(T(**ptr));
}

template <class T> inline void u_construct(const T* ptr, uint32_t n)
{
   U_TRACE(0, "u_construct<T>(%p,%u)", ptr, n)
}

template <class T> inline void u_destroy(const T* ptr)
{
   U_TRACE(0, "u_destroy<T>(%p)", ptr)

   if (ptr <= (const void*)0x0000ffff) U_ERROR("u_destroy<T>(%p)", ptr);

   delete ptr;
}

template <class T> inline void u_destroy(const T** ptr, uint32_t n)
{
   U_TRACE(0, "u_destroy<T>(%p,%u)", ptr, n)

   for (uint32_t i = 0; i < n; ++i) delete ptr[i];
}

template <> inline void u_construct(const UStringRep** prep, bool stream_loading)
{
   U_TRACE(0, "u_construct<UStringRep*>(%p,%b)", prep, stream_loading)

   ((UStringRep*)(*prep))->hold();
}

template <> inline void u_construct(const UStringRep* rep, uint32_t n)
{
   U_TRACE(0, "u_construct<UStringRep*>(%p,%u)", rep, n)

   ((UStringRep*)rep)->references += n;

   U_INTERNAL_DUMP("references = %d", rep->references + 1)
}

template <> inline void u_destroy(const UStringRep* rep)
{
   U_TRACE(0, "u_destroy<UStringRep*>(%p)", rep)

   ((UStringRep*)rep)->release();
}

template <> inline void u_destroy(const UStringRep** rep, uint32_t n)
{
   U_TRACE(0, "u_destroy<UStringRep*>(%p,%u)", rep, n)

   for (uint32_t i = 0; i < n; ++i) ((UStringRep*)rep[i])->release();
}

#endif
