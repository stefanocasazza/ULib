// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    bit_array.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_BIT_ARRAY_H
#define ULIB_BIT_ARRAY_H 1

#include <ulib/internal/common.h>

class U_EXPORT UBitArray {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // allocate and deallocate methods

   UBitArray(uint32_t nbits = 1024U)
      {
      U_TRACE_REGISTER_OBJECT(0, UBitArray, "%u", nbits)

      allocate((nbits+31)/32);
      }

   ~UBitArray()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UBitArray)

      deallocate();
      }

   // Capacity

   uint32_t capacity() const
      {
      U_TRACE_NO_PARAM(0, "UBitArray::capacity()")

      U_RETURN(_capacity);
      }

   // SERVICES

   uint32_t count() const __pure // get the number of bits set
      {
      U_TRACE_NO_PARAM(0, "UBitArray::count()")

      U_CHECK_MEMORY

      uint32_t num_of_bits_set = 0;

      for (uint32_t v, i = 0; i < _capacity; ++i)
         {
         v = vec[i];

#     if !defined(HAVE_OLD_IOSTREAM) && !defined(_MSWINDOWS_)
         num_of_bits_set += __builtin_popcountl(v);
#     else
         uint32_t c = v - ((v >> 1) & 0x55555555);

         c  = ((c >> 2) & 0x33333333) + (c & 0x33333333);
         c  = ((c >> 4) + c) & 0x0F0F0F0F;
         c  = ((c >> 8) + c) & 0x00FF00FF;

         num_of_bits_set += ((c >> 16) + c) & 0x0000FFFF;
   #  endif
         }

      U_RETURN(num_of_bits_set);
      }

   uint32_t getNumBits() const  { return _capacity * 4 * 8; }
   uint32_t getNumBytes() const { return _capacity * 4; }

   void setAll() // set all elements of data to one
      {
      U_TRACE_NO_PARAM(0, "UBitArray:setAll()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

      (void) U_SYSCALL(memset, "%p,%d,%u", vec, 0xff, getNumBytes());
      }

   void clearAll() // set all elements of data to zero
      {
      U_TRACE_NO_PARAM(0, "UBitArray:clearAll()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

      (void) U_SYSCALL(memset, "%p,%d,%u", vec, 0x00, getNumBytes());
      }

   // ELEMENT ACCESS

   uint32_t bindex( uint32_t i) const { return (i / 32); }
   uint32_t boffset(uint32_t i) const { return (i % 32); }

   void set(uint32_t i)
      {
      U_TRACE(0, "UBitArray:set(%u)", i)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

      uint32_t idx = bindex(i);

      U_INTERNAL_DUMP("idx = %u _capacity = %u", idx, _capacity)

      if (idx >= _capacity)
         {
         reserve(getNumBits() * 2);

         U_INTERNAL_ASSERT_MAJOR(_capacity, idx)
         }

      vec[idx] |= (1U << boffset(i));
      }

   void clear(uint32_t i)
      {
      U_TRACE(0, "UBitArray::clear(%u)", i)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_MINOR(i, getNumBits())

      vec[bindex(i)] &= ~(1U << boffset(i));
      }

   void toggle(uint32_t i)
      {
      U_TRACE(0, "UBitArray:toggle(%u)", i)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_MINOR(i, getNumBits())

      vec[bindex(i)] ^= (1U << boffset(i));
      }

   bool isSet(uint32_t i) const
      {
      U_TRACE(0, "UBitArray::isSet(%u)", i)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_MINOR(i, getNumBits())

      if ((vec[bindex(i)] & (1U << boffset(i))) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool operator[](uint32_t i) const { return isSet(i); }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const;
#endif

protected:
   uint32_t* vec;
   uint32_t _capacity;

   void reserve(uint32_t nbits);

   void allocate(uint32_t nuint)
      {
      U_TRACE(0, "UBitArray::allocate(%u)", nuint)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(nuint, 0)

      vec       = (uint32_t*) UMemoryPool::_malloc(&nuint, sizeof(uint32_t), true);
      _capacity = nuint;
      }

   void deallocate()
      {
      U_TRACE_NO_PARAM(0, "UBitArray::deallocate()")

      U_CHECK_MEMORY

      UMemoryPool::_free(vec, _capacity, sizeof(uint32_t));
      }

private:
   U_DISALLOW_ASSIGN(UBitArray)
};

#endif
