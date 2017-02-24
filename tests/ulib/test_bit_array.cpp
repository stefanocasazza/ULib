// test_bit_array.cpp

#include <ulib/utility/bit_array.h>

#ifndef U_HTTP2_DISABLE
#  include <ulib/utility/http2.h>
#endif

int U_EXPORT main(int argc, char** argv)
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "::main(%d,%p)", argc, argv)

   UBitArray addrmask;
   uint32_t i, nbits = addrmask.getNumBits();

   U_ASSERT_EQUALS( addrmask.count(), 0 )

   for (i = 0; i < nbits; ++i)
      {
      U_ASSERT_EQUALS( addrmask[i], false )
      }

   addrmask.setAll();

   U_ASSERT_EQUALS( addrmask.count(), nbits )

   for (i = 0; i < nbits; ++i)
      {
      U_ASSERT( addrmask[i] )
      }

   addrmask.clearAll();

   U_ASSERT_EQUALS( addrmask.count(), 0 )

   for (i = 0; i < nbits; ++i)
      {
      U_ASSERT_EQUALS( addrmask[i], false )
      }

   addrmask.setAll();
   addrmask.set(1024);

   ++nbits;

   U_ASSERT_EQUALS( addrmask.count(), nbits )

   for (i = 0; i < nbits; ++i)
      {
      U_ASSERT( addrmask[i] )
      }

   for (nbits = addrmask.getNumBits(); i < nbits; ++i)
      {
      U_ASSERT_EQUALS( addrmask[i], false )
      }

#if defined(DEBUG) && !defined(U_HTTP2_DISABLE)
   UHTTP2::testHpackDynTbl();
#endif
}
