// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    flatbuffers.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/serialize/flatbuffers.h>

uint8_t* UFlatBuffer::stack_str;
uint8_t* UFlatBuffer::buffer_str;
uint32_t UFlatBuffer::stack_idx;
uint32_t UFlatBuffer::stack_max;
uint32_t UFlatBuffer::buffer_max;

UFlatBufferValue* UFlatBuffer::pkeys;
UFlatBufferValue* UFlatBuffer::pvalue;

uint32_t UFlatBuffer::AsMapGetKeys(UVector<UString>& members) const
{
   U_TRACE(0, "UFlatBuffer::AsMapGetKeys(%p)", &members)

   UString key;
   UFlatBuffer vec;
   uint32_t len = members.size();

   AsMapGetKeys(vec);

   for (uint32_t i = 0, n = vec.GetSize(); i < n; ++i)
      {
#  ifndef HAVE_OLD_IOSTREAM
      key = vec.AsTypedOrFixedVectorGet<UString>(i);
#  endif

      U_INTERNAL_DUMP("key = %V", key.rep)

      members.push(key);
      }

   len = members.size() - len;

   U_RETURN(len);
}

void UFlatBuffer::WriteAny(uint8_t byte_width)
{
   U_TRACE(0, "UFlatBuffer::WriteAny(%u)", byte_width)

   U_DUMP("pvalue->type_ = (%u,%S) pvalue->i_ = %llu", pvalue->type_, getTypeDescription(pvalue->type_), pvalue->i_)

   switch (pvalue->type_)
      {
      case UFlatBufferValue::TYPE_NULL:
      case UFlatBufferValue::TYPE_INT:
      case UFlatBufferValue::TYPE_UINT:
      case UFlatBufferValue::TYPE_BOOL:  WriteScalar<uint64_t>(pvalue->u_, byte_width); break;
      case UFlatBufferValue::TYPE_FLOAT: WriteDouble(          pvalue->f_, byte_width); break;

      case UFlatBufferValue::TYPE_STRING:
      case UFlatBufferValue::TYPE_INDIRECT_INT:
      case UFlatBufferValue::TYPE_INDIRECT_UINT:
      case UFlatBufferValue::TYPE_INDIRECT_FLOAT:
      case UFlatBufferValue::TYPE_MAP:
      case UFlatBufferValue::TYPE_VECTOR:
      case UFlatBufferValue::TYPE_VECTOR_INT:
      case UFlatBufferValue::TYPE_VECTOR_UINT:
      case UFlatBufferValue::TYPE_VECTOR_FLOAT:
      case UFlatBufferValue::TYPE_VECTOR_BOOL:
      case UFlatBufferValue::TYPE_VECTOR_STRING:
      case UFlatBufferValue::TYPE_VECTOR_INT2:
      case UFlatBufferValue::TYPE_VECTOR_UINT2:
      case UFlatBufferValue::TYPE_VECTOR_FLOAT2:
      case UFlatBufferValue::TYPE_VECTOR_INT3:
      case UFlatBufferValue::TYPE_VECTOR_UINT3:
      case UFlatBufferValue::TYPE_VECTOR_FLOAT3:
      case UFlatBufferValue::TYPE_VECTOR_INT4:
      case UFlatBufferValue::TYPE_VECTOR_UINT4:
      case UFlatBufferValue::TYPE_VECTOR_FLOAT4: WriteOffset(pvalue->l_, byte_width);
      break;
      }
}

uint8_t* UFlatBuffer::AsMapSetIndex(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapSetIndex(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   UString item;
   UFlatBuffer keys;

   AsMapGetKeys(keys);

   for (uint32_t i = 0; i < keys.buffer_idx; ++i)
      {
#  ifndef HAVE_OLD_IOSTREAM
      item = keys.AsTypedOrFixedVectorGet<UString>(i);
#  endif

      U_INTERNAL_DUMP("item[%u] = %V", i, item.rep)

      if (item.equal(key, len)) return AsVectorSetIndex(i);
      }

   U_INTERNAL_ASSERT(false)

   return U_NULLPTR;
}

void UFlatBuffer::CreateVector(uint32_t start, uint32_t vec_len, uint32_t step, bool typed, bool fixed, UFlatBufferValue* pval)
{
   U_TRACE(0, "UFlatBuffer::CreateVector(%u,%u,%u,%b,%b,%p)", start, vec_len, step, typed, fixed, pval)

   U_INTERNAL_ASSERT_MAJOR(vec_len, 0)

   // Figure out smallest bit width we can store this vector with

   uint32_t i, vloc, prefix_elems = 1;
   vPF nextStackPtr = (step == 1 ? nextStackPointer : next2StackPointer);
   uint8_t byte_width, bit_width = UFlatBufferValue::WidthL(vec_len), elem_width, vector_type;

   if (pkeys)
      {
      // If this vector is part of a map, we will pre-fix an offset to the keys to this vector

      elem_width = pkeys->ElemWidth(buffer_idx, 0);

      if (bit_width < elem_width) bit_width = elem_width;

      prefix_elems += 2;
      }

   // Check bit widths and types for all elements

   setStackPointer((i = start));

   vector_type = pvalue->type_;

loop1:
   elem_width = pvalue->ElemWidth(buffer_idx, i + prefix_elems);

   U_INTERNAL_DUMP("bit_width = %u elem_width = %u", bit_width, elem_width)

   if (bit_width < elem_width) bit_width = elem_width;

#ifdef DEBUG
   if (typed)
      {
      U_INTERNAL_ASSERT_EQUALS(vector_type, pvalue->type_)       // you are writing a typed vector with elements that are not all the same type
      U_INTERNAL_ASSERT(IsTypedVectorElementType(pvalue->type_)) // your type are not one of: Int / UInt / Float / Bool / String
      }
#endif

   if ((i += step) < stack_idx)
      {
      nextStackPtr();

      goto loop1;
      }

   byte_width = Align(bit_width);

   U_INTERNAL_DUMP("byte_width = %u vector_type = %u", byte_width, vector_type)

   // Write vector. First the keys offset/width if available, and size

   if (pkeys)
      {
      WriteOffset(pkeys->l_, byte_width);

      WriteScalar<uint32_t>(1ULL << pkeys->min_bit_width_, byte_width);
      }

   if (fixed == false) WriteScalar<uint32_t>(vec_len, byte_width);

   // Then the actual data

   vloc = buffer_idx;

   setStackPointer((i = start));

loop2:
   WriteAny(byte_width);

   if ((i += step) < stack_idx)
      {
      nextStackPtr();

      goto loop2;
      }

   if (typed == false) // Then the types
      {
      setStackPointer((i = start));

loop3:
      WriteScalar8(pvalue->StoredPackedType(bit_width));

      if ((i += step) < stack_idx)
         {
         nextStackPtr();

         goto loop3;
         }
      }

   if (pval)
      {
      U_INTERNAL_ASSERT(typed)
      U_INTERNAL_ASSERT_EQUALS(fixed, false)
      U_INTERNAL_ASSERT_EQUALS(pkeys, U_NULLPTR)
      U_INTERNAL_ASSERT_EQUALS(vector_type, UFlatBufferValue::TYPE_STRING)

      pval->set(vloc, ToTypedVector(vector_type, 0), bit_width);
      }
   else if (pkeys == U_NULLPTR)
      {
      stack_idx = start; // Remove temp elements

      pushOnStack(vloc, (typed ? ToTypedVector(vector_type, fixed ? vec_len : 0) : (uint8_t)UFlatBufferValue::TYPE_VECTOR), bit_width);
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(typed, false)
      U_INTERNAL_ASSERT_EQUALS(fixed, false)

      stack_idx = start-1; // Remove temp elements

      pushOnStack(vloc, UFlatBufferValue::TYPE_MAP, bit_width);
      }

   U_INTERNAL_DUMP("buffer_str+vloc = %#.8S", buffer_str+vloc)
}

void UFlatBuffer::EndMap(uint32_t start)
{
   U_TRACE(0, "UFlatBuffer::EndMap(%u)", start)

   // We should have interleaved keys and values on the stack

   U_INTERNAL_DUMP("stack_idx = %u", stack_idx)

   U_INTERNAL_ASSERT_EQUALS((stack_idx-start) & 1, false) // Make sure it is an even number

   uint32_t len = (stack_idx-start) / 2;

   U_INTERNAL_ASSERT_MAJOR(len, 0)

#ifdef DEBUG // Make sure keys are all strings
   int32_t key;

   setStackPointer((key = start));
loop:
   U_DUMP("key = %u pvalue->type_ = (%u,%S)", key, pvalue->type_, getTypeDescription(pvalue->type_))

// U_INTERNAL_ASSERT_EQUALS(pvalue->type_, UFlatBufferValue::TYPE_STRING)

   if ((key += 2) < (int32_t)stack_idx)
      {
      next2StackPointer();

      goto loop;
      }
#endif

   // First create a vector out of all keys

   uint8_t keys[20];

   CreateVector(start, len, 2, true, false, (UFlatBufferValue*)keys);

   pkeys = (UFlatBufferValue*)keys;

   CreateVector(start+1, len, 2, false, false, U_NULLPTR);

   pkeys = U_NULLPTR;
}

// DEBUG

#ifdef DEBUG
const char* UFlatBuffer::getTypeDescription(uint8_t type)
{
   U_TRACE(0, "UFlatBuffer::getTypeDescription(%u)", type)

   struct type_info {
      int value;        // The numeric value
      const char* name; // The equivalent symbolic value
   };

   static const struct type_info type_table[] = {
      U_ENTRY(UFlatBufferValue::TYPE_NULL),
      U_ENTRY(UFlatBufferValue::TYPE_INT),
      U_ENTRY(UFlatBufferValue::TYPE_UINT),
      U_ENTRY(UFlatBufferValue::TYPE_FLOAT),
      U_ENTRY(UFlatBufferValue::TYPE_BOOL),
      U_ENTRY(UFlatBufferValue::TYPE_STRING),
      U_ENTRY(UFlatBufferValue::TYPE_INDIRECT_INT),
      U_ENTRY(UFlatBufferValue::TYPE_INDIRECT_UINT),
      U_ENTRY(UFlatBufferValue::TYPE_INDIRECT_FLOAT),
      U_ENTRY(UFlatBufferValue::TYPE_MAP),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_INT),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_UINT),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_FLOAT),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_BOOL),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_STRING),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_INT2),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_UINT2),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_FLOAT2),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_INT3),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_UINT3),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_FLOAT3),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_INT4),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_UINT4),
      U_ENTRY(UFlatBufferValue::TYPE_VECTOR_FLOAT4)
   };

   const char* descr = (type < (int)U_NUM_ELEMENTS(type_table) ? type_table[type].name : "type unknown");

   U_RETURN(descr);
}

const char* UFlatBuffer::dump(bool reset) const
{
#ifdef U_STDCPP_ENABLE
   *UObjectIO::os << "type_         " << type_                 << '\n'
                  << "buffer_idx    " << buffer_idx            << '\n'
                  << "byte_width_   " << (uint32_t)byte_width_ << '\n'
                  << "parent_width_ " << (uint32_t)parent_width_;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }
#endif

   return U_NULLPTR;
}

const char* UFlatBufferTypeHandler_Base::dump(bool _reset) const
{
#ifdef U_STDCPP_ENABLE
   *UObjectIO::os << "pval " << pval;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }
#endif

   return U_NULLPTR;
}
#endif
