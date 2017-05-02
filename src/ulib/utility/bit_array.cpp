// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    bit_array.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/bit_array.h>

void UBitArray::reserve(uint32_t nbits)
{
   U_TRACE(0, "UBitArray::reserve(%u)", nbits)

   U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

   uint32_t nuint = (nbits / 32);

   U_INTERNAL_DUMP("nuint = %u _capacity = %u", nuint, _capacity)

   U_INTERNAL_ASSERT_MAJOR(nuint, _capacity)

   uint32_t* old_vec = vec;

   vec = (uint32_t*) UMemoryPool::_malloc(&nuint, sizeof(uint32_t));

   U_MEMCPY(vec, old_vec, getNumBytes());

   UMemoryPool::_free(old_vec, _capacity, sizeof(uint32_t));

   (void) U_SYSCALL(memset, "%p,%d,%u", vec+_capacity, 0x00, (nuint-_capacity)*4);

   _capacity = nuint;
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UBitArray::dump(bool reset) const
{
   *UObjectIO::os << "vec       " << (void*)vec << '\n'
                  << "_capacity " << _capacity;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
