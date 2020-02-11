// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    flatbuffers.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_FLAT_BUFFERS_H
#define ULIB_FLAT_BUFFERS_H

#include <ulib/container/hash_map.h>

#ifndef _MSWINDOWS_
#  include <arpa/inet.h>
#endif

/**
 * Serialization is the process of translating data structures or object state into a format that can be stored (for example, in a file or
 * memory buffer) or transmitted (for example, across a network connection link) and reconstructed later (possibly in a different computer environment)
 *
 * @see http://google.github.io/flatbuffers/index.html
 *
 * All data is accessed over offsets, all scalars are aligned to their own size, and all data is always stored in little endian format.
 * Buffers are built front to back, so children are stored before parents, and the root of the data starts at the last byte.
 *
 * The scalar data is stored with a variable number of bits (8/16/32/64). The current width is always determined by the *parent*, i.e.
 * if the scalar sits in a vector, the vector determines the bit width for all elements at once. Selecting the minimum bit width for a
 * particular vector is something the encoder does automatically and thus is typically of no concern to the user, though being aware of
 * this feature (and not sticking a double in the same vector as a bunch of byte sized elements) is helpful for efficiency.
 *
 * There is only one kind of offset, and that is an unsigned integer indicating the number of bytes in a negative direction from the
 * address of itself (where the offset is stored). The root starts at the end of the buffer. The last uint8_t is the width in bytes of
 * the root (normally the parent determines the width, but the root has no parent). The uint8_t before this is the type of the root, and
 * the bytes before that are the root value (of the number of bytes specified by the last byte)
 *
 * Vectors
 *
 * The representation of the vector is at the core of how UFlatBuffer works (since maps are really just a combination of 2 vectors).
 * A vector is governed by a single bit width (supplied by its parent). This includes the size field. For example, a vector that
 * stores the integer values `1, 2, 3` is encoded as follows:
 *
 * uint8_t 3, 1, 2, 3, 4, 4, 4
 *
 * The first `3` is the size field, and is placed before the vector (an offset from the parent to this vector points to the first
 * element, not the size field, so the size field is effectively at index -1). Since this is an untyped vector `TYPE_VECTOR`, it
 * is followed by 3 type bytes (one per element of the vector), which are always following the vector, and are always a uint8_t
 * even if the vector is made up of bigger scalars. There are also the Typed Vectors that omit the type bytes. The type is instead
 * determined by the vector type supplied by the parent. Typed vectors are only available for a subset of types for which these
 * savings can be significant, namely inline signed/unsigned integers (`TYPE_VECTOR_INT` / `TYPE_VECTOR_UINT`), floats (`TYPE_VECTOR_FLOAT`),
 * string (`TYPE_VECTOR_STRING`) and bool (`TYPE_VECTOR_BOOL`). Additionally, for scalars, there are fixed length vectors of sizes 2 / 3 / 4
 * that don't store the size (`TYPE_VECTOR_INT2` etc.), for an additional savings in space when storing common vector or color data.
 *
 * Scalars
 *
 * UFlatBuffer supports integers (`TYPE_INT` and `TYPE_UINT`) and floats (`TYPE_FLOAT`), available in the bit-widths mentioned above.
 * They can be stored both inline and over an offset (`TYPE_INDIRECT_*`). The offset version is useful to encode costly 64bit (or even
 * 32bit) quantities into vectors / maps of smaller sizes, and to share / repeat a value multiple times.
 *
 * Booleans and Nulls
 *
 * Booleans (`TYPE_BOOL`) and nulls (`TYPE_NULL`) are encoded as inlined unsigned integers.
 *
 * Maps
 *
 * A map (`TYPE_MAP`) is like an (untyped) vector, but with 2 prefixes before the size field:
 *
 * | index | field                                                   |
 * | ----: | :-------------------------------------------------------|
 * | -3    | An offset to the keys vector                            |
 * | -2    | Byte width of the keys vector                           |
 * | -1    | Size (from here on it is compatible with `TYPE_VECTOR`) |
 * |  0    | Elements                                                |
 * | Size  | Types                                                   |
 *
 * Since a map is otherwise the same as a vector, it can be iterated like a vector (which is probably faster than lookup by key).
 * The keys vector is a typed vector of strings. The reason the key vector is a seperate structure from the value vector is such that
 * to allow it to be treated as its own individual vector in code. An example map { foo: 13, bar: 14 } would be encoded as:
 *
 * "\003foo\003bar\002\b\005\002\001\002\r\016\004\004\004$\001"
 *   0      4      8   9     11      13  14    16        19
 *
 * 0 : uint8_t 3, 'f', 'o', 'o'
 * 4 : uint8_t 3, 'b', 'a', 'r'
 * 8 : uint8_t 2 // key vector of size 2
 * <-------------------------------------> key vector offset points here
 * 9 : uint8_t 8, 5 // offsets to foo_key and bar_key
 * 11: uint8_t 2, 1 // offset to key vector, and its byte width
 * 13: uint8_t 2    // value vector of size
 * <-------------------------------------> value vector offset points here
 * 14: uint8_t 13, 14 // values
 * 16: uint8_t  4,  4 // types
 * 19: uint8_t 40     // root type
 *
 * The root
 *
 * The root starts at the end of the buffer. The last uint8_t is the width in bytes of the root (normally the parent determines
 * the width, but the root has no parent). The uint8_t before this is the type of the root, and the bytes before that are the
 * root value (of the number of bytes specified by the last byte). So for example, the integer value `13` as root would be:
 *
 * uint8_t 13, 4, 1 // root value, type and byte width
 */

class UFlatBuffer;

template <class T> class UFlatBufferTypeHandler;

class U_EXPORT UFlatBufferValue {
public:

   /**
    * A type byte is made up of 2 components:
    *
    * 2 lower bits representing the bit-width of the child (8, 16, 32, 64).
    * This is only used if the child is accessed over an offset, such as a child vector. It is ignored for inline types
    */

   enum BitWidth {
      BIT_WIDTH_8  = 0,
      BIT_WIDTH_16 = 1,
      BIT_WIDTH_32 = 2,
      BIT_WIDTH_64 = 3
   };

   /**
    * 6 bits representing the actual type
    *
    * for example 4 (100) means 8 bit child (value 0, unused, since the value is in-line), type `TYPE_INT` (value 1)
    */

   enum Type {
      TYPE_NULL = 0,
      TYPE_INT = 1,
      TYPE_UINT = 2,
      TYPE_FLOAT = 3,
      TYPE_BOOL = 4,
      // Types above stored inline, types below store an offset
      TYPE_STRING = 5,
      TYPE_INDIRECT_INT = 6,
      TYPE_INDIRECT_UINT = 7,
      TYPE_INDIRECT_FLOAT = 8,
      TYPE_MAP = 9,
      TYPE_VECTOR = 10, // Untyped
      TYPE_VECTOR_INT = 11, // Typed any size (stores no type table)
      TYPE_VECTOR_UINT = 12,
      TYPE_VECTOR_FLOAT = 13,
      TYPE_VECTOR_BOOL = 14, // To Allow the same type of conversion of type to vector type
      TYPE_VECTOR_STRING = 15,
      TYPE_VECTOR_INT2 = 16, // Typed tuple (no type table, no size field)
      TYPE_VECTOR_UINT2 = 17,
      TYPE_VECTOR_FLOAT2 = 18,
      TYPE_VECTOR_INT3 = 19, // Typed triple (no type table, no size field)
      TYPE_VECTOR_UINT3 = 20,
      TYPE_VECTOR_FLOAT3 = 21,
      TYPE_VECTOR_INT4 = 22, // Typed quad (no type table, no size field)
      TYPE_VECTOR_UINT4 = 23,
      TYPE_VECTOR_FLOAT4 = 24
      };

   static uint32_t size() { return sizeof(double)+(sizeof(uint8_t)*2); }

protected:
   union { uint32_t l_; int64_t i_; uint64_t u_; double f_; };
   uint8_t type_, min_bit_width_; // For scalars: of itself, for vector: of its elements, for string: length

   void reset()
      {
      U_TRACE_NO_PARAM(0, "UFlatBufferValue::reset()")

                  u_ = 0ULL;
               type_ = TYPE_NULL;
      min_bit_width_ = BIT_WIDTH_8;
      }

   void set(uint64_t u = 0ULL, uint8_t type = TYPE_NULL, uint8_t min_bit_width = BIT_WIDTH_8)
      {
      U_TRACE(0, "UFlatBufferValue::set(%llu,%u,%u)", u, type, min_bit_width)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
                      u_ = u;
#  else
      l_ = (uint32_t)(u_ = u);
#  endif

      U_INTERNAL_ASSERT_EQUALS(l_, (uint32_t)u_)

               type_ = type;
      min_bit_width_ = min_bit_width;
      }

   void set(float f)
      {
      U_TRACE(0, "UFlatBufferValue::set(%g)", f)

                  f_ = f;
               type_ = TYPE_FLOAT;
      min_bit_width_ = BIT_WIDTH_32;
      }

   void set(double f)
      {
      U_TRACE(0, "UFlatBufferValue::set(%g)", f)

                  f_ = f;
               type_ = TYPE_FLOAT;
      min_bit_width_ = WidthF(f);
      }

   uint8_t StoredWidth(uint8_t parent_bit_width) const
      {
      U_TRACE(0, "UFlatBufferValue::StoredWidth(%u)", parent_bit_width)

      if (IsInline(type_) &&
          min_bit_width_ < parent_bit_width)
         {
         U_RETURN(parent_bit_width);
         }

      U_RETURN(min_bit_width_);
      }

   uint8_t StoredPackedType(uint8_t parent_bit_width) const
      {
      U_TRACE(0, "UFlatBufferValue::StoredPackedType(%u)", parent_bit_width)

      uint8_t bit_width = PackedType(StoredWidth(parent_bit_width), type_);

      U_RETURN(bit_width);
      }

   uint8_t ElemWidth(uint32_t buf_size, uint32_t elem_index) const __pure;

   static uint8_t WidthB(uint32_t byte_width)
      {
      U_TRACE(0, "UFlatBufferValue::WidthB(%u)", byte_width)

      switch (byte_width)
         {
         case 1: U_RETURN(BIT_WIDTH_8);
         case 2: U_RETURN(BIT_WIDTH_16);
         case 4: U_RETURN(BIT_WIDTH_32);
         case 8: U_RETURN(BIT_WIDTH_64);

         default: U_INTERNAL_ASSERT(false);
         }

      U_RETURN(BIT_WIDTH_64);
      }

   static uint8_t WidthL(uint32_t u)
      {
      U_TRACE(0, "UFlatBufferValue::WidthL(%u)", u)

      if ((u & ~((1U <<  8) - 1U)) == 0) U_RETURN(BIT_WIDTH_8);
      if ((u & ~((1U << 16) - 1U)) == 0) U_RETURN(BIT_WIDTH_16);

      U_RETURN(BIT_WIDTH_32);
      }

   static uint8_t WidthU(uint64_t u)
      {
      U_TRACE(0, "UFlatBufferValue::WidthU(%llu)", u)

      if ((u & ~((1ULL <<  8) - 1ULL)) == 0) U_RETURN(BIT_WIDTH_8);
      if ((u & ~((1ULL << 16) - 1ULL)) == 0) U_RETURN(BIT_WIDTH_16);
      if ((u & ~((1ULL << 32) - 1ULL)) == 0) U_RETURN(BIT_WIDTH_32);

      U_RETURN(BIT_WIDTH_64);
      }

   static uint8_t WidthI(int64_t i)
      {
      U_TRACE(0, "UFlatBufferValue::WidthI(%lld)", i)

      U_INTERNAL_ASSERT_MINOR(i, 0)

      return WidthU(-i);
      }

   static uint8_t WidthF(double f)
      {
      U_TRACE(0, "UFlatBufferValue::WidthF(%g)", f)

      U_RETURN((double)((float)f) == f ? BIT_WIDTH_32 : BIT_WIDTH_64);
      }

   static bool IsInline(uint8_t t)
      {
      U_TRACE(0, "UFlatBufferValue::IsInline(%u)", t)

      if (t <= TYPE_BOOL) U_RETURN(true);

      U_RETURN(false);
      }

   static uint8_t PackedType(uint8_t bit_width, uint8_t type)
      {
      U_TRACE(0, "UFlatBufferValue::PackedType(%u,%u)", bit_width, type)

      return (bit_width | (type << 2));
      }

   // Computes how many bytes you'd have to pad to be able to write an
   // "scalar_size" scalar if the buffer had grown to "buf_size" (downwards in memory)

   static uint32_t PaddingBytes(uint32_t buf_size, uint32_t scalar_size)
      {
      U_TRACE(0, "UFlatBufferValue::PaddingBytes(%u,%u)", buf_size, scalar_size)

      U_RETURN(((~buf_size)+1) & (scalar_size-1));
      }

private:
   UFlatBufferValue()
      {
      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
#  endif
      }

   friend class UFlatBuffer;

   U_DISALLOW_COPY_AND_ASSIGN(UFlatBufferValue)
};

typedef void (*vPFpfbpv) (UFlatBuffer*,void*);

class U_EXPORT UFlatBuffer {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UFlatBuffer()
      {
      U_TRACE_CTOR(0, UFlatBuffer, "")

      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
      u_             = 0ULL;
      type_          =
      min_bit_width_ = BIT_WIDTH_8;

      reset();
#  endif
      }

   UFlatBuffer(const UString& str)
      {
      U_TRACE_CTOR(0, UFlatBuffer, "%V", str.rep)

      setRoot(str);
      }

   ~UFlatBuffer()
      {
      U_TRACE_DTOR(0, UFlatBuffer)
      }

   // SERVICES

   void reset()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::reset()")

      data_         = U_NULLPTR;
      buffer_idx    = 0;
      type_         = UFlatBufferValue::TYPE_NULL;
        byte_width_ =
      parent_width_ = UFlatBufferValue::BIT_WIDTH_8;
      }

   uint8_t GetType() const { return type_; }

   static bool IsBool(uint8_t type)      { return  type == UFlatBufferValue::TYPE_BOOL; }
   static bool IsInt(uint8_t type)       { return (type == UFlatBufferValue::TYPE_INT  || type == UFlatBufferValue::TYPE_INDIRECT_INT); }
   static bool IsUInt(uint8_t type)      { return (type == UFlatBufferValue::TYPE_UINT || type == UFlatBufferValue::TYPE_INDIRECT_UINT); }
   static bool IsIntOrUint(uint8_t type) { return (IsInt(type) || IsUInt(type)); }
   static bool IsFloat(uint8_t type)     { return (type == UFlatBufferValue::TYPE_FLOAT || type == UFlatBufferValue::TYPE_INDIRECT_FLOAT); }
   static bool IsVector(uint8_t type)    { return  type == UFlatBufferValue::TYPE_VECTOR; }
   static bool IsNumeric(uint8_t type)   { return (IsIntOrUint(type) || IsFloat(type)); }
   static bool IsScalar(uint8_t type)    { return (IsNumeric(type) || IsBool(type)); }

   bool IsNull() const      { return type_ == UFlatBufferValue::TYPE_NULL; }
   bool IsString() const    { return type_ == UFlatBufferValue::TYPE_STRING; }
   bool IsMap() const       { return type_ == UFlatBufferValue::TYPE_MAP; }
   bool IsBool() const      { return IsBool(type_); }
   bool IsInt() const       { return IsInt(type_); }
   bool IsUInt() const      { return IsUInt(type_); }
   bool IsIntOrUint() const { return IsIntOrUint(type_); }
   bool IsFloat() const     { return IsFloat(type_); }
   bool IsNumeric() const   { return IsNumeric(type_); }
   bool IsVector() const    { return IsVector(type_); } // Maps can be viewed as vectors too

   bool IsTypedVector() const      { return (type_ >= UFlatBufferValue::TYPE_VECTOR_INT  && type_ <= UFlatBufferValue::TYPE_VECTOR_STRING); }
   bool IsFixedTypedVector() const { return (type_ >= UFlatBufferValue::TYPE_VECTOR_INT2 && type_ <= UFlatBufferValue::TYPE_VECTOR_FLOAT4); }

   // All value constructing functions below have two versions: one that takes a key
   // (for placement inside a map) and one that doesn't (for inside vectors and elsewhere)

   static void Null()           { pushOnStack(); }
   static void Bool(bool b)     { pushOnStack((uint64_t)b, UFlatBufferValue::TYPE_BOOL, UFlatBufferValue::BIT_WIDTH_8); }
   static void Int(  int64_t i) { pushOnStack(-i, UFlatBufferValue::TYPE_INT,  UFlatBufferValue::WidthI(i)); }
   static void UInt(uint64_t u) { pushOnStack(u,  UFlatBufferValue::TYPE_UINT, UFlatBufferValue::WidthU(u)); }
   static void Float(float f)   { pushOnStack(f); }
   static void Double(double d) { pushOnStack(d); }

   static void IPAddress(uint32_t u) { UInt(ntohl(u)); }

   static void setNumber(int64_t l)
      {
      U_TRACE(0, "UFlatBuffer::setNumber(%lld)", l)

      if (l < 0)  Int(l);
      else       UInt(l);
      }

   void IndirectInt(  int64_t i) { PushIndirect((uint64_t)i, UFlatBufferValue::TYPE_INDIRECT_INT, UFlatBufferValue::WidthU((uint64_t)i)); }
   void IndirectUInt(uint64_t u) { PushIndirect(u, UFlatBufferValue::TYPE_INDIRECT_UINT, UFlatBufferValue::WidthU(u)); }
   void IndirectFloat(float f)   { PushIndirectFloat(f); }
   void IndirectDouble(double f) { PushIndirect(f, UFlatBufferValue::TYPE_INDIRECT_FLOAT, UFlatBufferValue::WidthF(f)); }

   void String(const UString& str)            { CreateString(U_STRING_TO_PARAM(str)); }
   void String(const char* str, uint32_t len) { CreateString(str, len); }

   void Null(  const char* key, uint32_t len)             { Key(key, len); Null(); }
   void Bool(  const char* key, uint32_t len, bool b)     { Key(key, len); Bool(b); }
   void  Int(  const char* key, uint32_t len,  int64_t i) { Key(key, len);  Int(i); }
   void UInt(  const char* key, uint32_t len, uint64_t u) { Key(key, len); UInt(u); }
   void Float( const char* key, uint32_t len, float f)    { Key(key, len); Float(f); }
   void Double(const char* key, uint32_t len, double d)   { Key(key, len); Double(d); }

   void IndirectInt(   const char* key, uint32_t len,  int64_t i) { Key(key, len); IndirectInt(i); } 
   void IndirectUInt(  const char* key, uint32_t len, uint64_t u) { Key(key, len); IndirectUInt(u); }
   void IndirectFloat( const char* key, uint32_t len, float f)    { Key(key, len); IndirectFloat(f); }
   void IndirectDouble(const char* key, uint32_t len, double d)   { Key(key, len); IndirectDouble(d); }

   void String(const char* key, uint32_t len1, const char* str, uint32_t len2) { Key(key, len1); CreateString(str, len2); }

   void StringNull()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::StringNull()")

      buffer_str[buffer_idx++] = 0;

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)

      pushOnStack(buffer_idx, UFlatBufferValue::TYPE_STRING, UFlatBufferValue::BIT_WIDTH_8);
      }

   // Overloaded Add that tries to call the correct function above

   static void Add()           { Null(); }
   static void Add(bool b)     { Bool(b); }
   static void Add( int64_t i) {  Int(i); }
   static void Add(uint64_t u) { UInt(u); }
   static void Add(float f)    { Float(f); }
   static void Add(double d)   { Double(d); }

   void Add(const char* str, uint32_t len) { CreateString(str, len); }

   void Key(const char* key, uint32_t len) { CreateString(key, len); }

   void Add(const char* key, uint32_t len, bool b)     { Key(key, len); Bool(b); }
   void Add(const char* key, uint32_t len,  int64_t i) { Key(key, len);  Int(i); }
   void Add(const char* key, uint32_t len, uint64_t u) { Key(key, len); UInt(u); }
   void Add(const char* key, uint32_t len, float f)    { Key(key, len); Float(f); }
   void Add(const char* key, uint32_t len, double d)   { Key(key, len); Double(d); }

   void Add(const char* key, uint32_t len1, const char* str, uint32_t len2) { Key(key, len1); CreateString(str, len2); }

   static uint32_t StartVector()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::StartVector()")

      U_RETURN(stack_idx);
      }

   uint32_t StartVector(const char* key, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::StartVector(%.*S,%u)", len, key, len)

      Key(key, len);
      
      U_RETURN(stack_idx);
      }

   static uint32_t StartMap()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::StartMap()")

      U_RETURN(stack_idx);
      }

   uint32_t StartMap(const char* key, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::StartMap(%.*S,%u)", len, key, len)

      uint32_t start = stack_idx;
      
      Key(key, len);
      
      U_RETURN(start);
      }

   void EndMap(   uint32_t start);
   void EndVector(uint32_t start, bool typed = true, bool fixed = false)
      {
      U_TRACE(0, "UFlatBuffer::EndVector(%u,%b,%b)", start, typed, fixed)

      CreateVector(start, stack_idx-start, 1, typed, fixed, U_NULLPTR);
      }

   template <typename T> void TypedVector(const T* elems, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::TypedVector<T>(%p,%u)", elems, len)

      uint32_t start = StartVector();

      for (uint32_t i = 0; i < len; ++i) Add(elems[i]);

      EndVector(start);
      }

   template <typename T> void TypedVector(const char* key, uint32_t klen, const T* elems, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::TypedVector<T>(%.*S,%u,%p,%u)", klen, key, klen, elems, len)

      uint32_t start = StartVector(key, klen);

      for (uint32_t i = 0; i < len; ++i) Add(elems[i]);

      EndVector(start);
      }

   template <typename T> void FixedTypedVector(const T* elems, uint32_t len, uint8_t vector_type = UFlatBufferValue::TYPE_UINT) // only scalar values
      {
      U_TRACE(0, "UFlatBuffer::FixedTypedVector<T>(%p,%u,%u)", elems, len, vector_type)

      U_INTERNAL_ASSERT_RANGE(2, len, 4) // We only support a few fixed vector lengths. Anything bigger use a regular typed vector

      ScalarVector(elems, len, vector_type, true);
      }

   template <typename T> void FixedTypedVector(const char* key, uint32_t klen, const T* elems, uint32_t len, uint8_t vector_type = UFlatBufferValue::TYPE_UINT) // only scalar values
      {
      U_TRACE(0, "UFlatBuffer::FixedTypedVector<T>(%.*S,%u,%p,%u,%u)", klen, key, klen, elems, len, vector_type)

      U_INTERNAL_ASSERT_RANGE(2, len, 4) // We only support a few fixed vector lengths. Anything bigger use a regular typed vector

      Key(key, klen);

      ScalarVector(elems, len, vector_type, true);
      }

   template <typename T> void ScalarVector(const T* elems, uint32_t len, uint8_t vector_type = UFlatBufferValue::TYPE_UINT, bool fixed = true)
      {
      U_TRACE(0, "UFlatBuffer::ScalarVector<T>(%p,%u,%u,%b)", elems, len, vector_type, fixed)

      U_INTERNAL_ASSERT(IsScalar(vector_type))

      uint8_t byte_width = sizeof(T), bit_width = UFlatBufferValue::WidthB(byte_width);

      // If you get this assert, you're trying to write a vector with a size field that is bigger than the scalars
      // you're trying to write (e.g. a byte vector > 255 elements). For such types, write a "string" instead

      U_INTERNAL_ASSERT(UFlatBufferValue::WidthU(len) <= bit_width)

      if (fixed == false) WriteScalar<uint32_t>(len, byte_width);
#  ifdef DEBUG
      else
         {
         U_INTERNAL_ASSERT(IsNumeric(vector_type))
         }
#  endif

      uint32_t vloc = buffer_idx;

      for (uint32_t i = 0; i < len; ++i) WriteScalar(elems[i], byte_width);

      pushOnStack(vloc, ToTypedVector(vector_type, fixed ? len : 0), bit_width);
      }

   template <typename F> void Vector(F f, bool typed = false, bool fixed = false)
      {
      U_TRACE(0, "UFlatBuffer::Vector<F>(%p,%b,%b)", f, typed, fixed)

      uint32_t start = StartVector();

      f();

      EndVector(start, typed, fixed);
      }

   template <typename F> void Vector(const char* key, uint32_t len, F f, bool typed = false, bool fixed = false)
      {
      U_TRACE(0, "UFlatBuffer::Vector<F>(%.*S,%u,%p,%b,%b)", len, key, len, f, typed, fixed)

      uint32_t start = StartVector(key, len);

      f();

      EndVector(start, typed, fixed);
      }

   template <typename F> void Map(F f)
      {
      U_TRACE(0, "UFlatBuffer::Map<F>(%p)", f)

      uint32_t start = StartMap();

      f();

      EndMap(start);
      }

   template <typename F> void Map(const char* key, uint32_t len, F f)
      {
      U_TRACE(0, "UFlatBuffer::Map<F>(%.*S,%u,%p)", len, key, len, f)

      uint32_t start = StartMap(key, len);

      f();

      EndMap(start);
      }

   // manage object <=> FlatBuffer representation

   template <typename T> void toFlatBuffer(UFlatBufferTypeHandler<T> member)
      {
      U_TRACE(0, "UFlatBuffer::toFlatBuffer<T>(%p)", &member)

      member.toFlatBuffer(*this);
      }

   template <typename T> void toFlatBuffer(const UString& key, UFlatBufferTypeHandler<T> member)
      {
      U_TRACE(0, "UFlatBuffer::toFlatBuffer<T>(%V, %p)", key.rep, &member)

      Key(U_STRING_TO_PARAM(key));

      member.toFlatBuffer(*this);
      }

   template <typename T> void fromFlatBuffer(uint32_t i, UFlatBufferTypeHandler<T> member)
      {
      U_TRACE(0, "UFlatBuffer::fromFlatBuffer<T>(%u,%p)", i, &member)

      UFlatBuffer fb;

      AsVectorGet(i, fb);

      member.fromFlatBuffer(fb);
      }

   template <class T> void   toObject(T& obj);
   template <class T> void fromObject(T& obj);

   template <class T> static void toObject(const UString& str, T& obj)
      {
      U_TRACE(0, "UFlatBuffer::toObject(%V,%p)", str.rep, &obj)

      UFlatBuffer(str).toObject<T>(obj);
      }

   // INIT

#ifdef GCC_IS_GNU
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

   static void setStack(uint8_t* ptr, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::setStack(%p,%u)", ptr, len)

      stack_str = ptr;
      stack_max = len / UFlatBufferValue::size();
      }
      
#ifdef GCC_IS_GNU
#pragma GCC diagnostic pop
#endif

   static void setBuffer(uint8_t* ptr, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::setBuffer(%p,%u)", ptr, len)

      buffer_str = ptr;
      buffer_max = len;
      }

   UString  getResult() const     { return UString(buffer_str, buffer_idx); }
   uint8_t* getPointer() const    { return buffer_str+buffer_idx; }
   uint32_t getBufferSize() const { return buffer_idx; }

   static uint8_t* getStack()     { return stack_str; }
   static uint8_t* getBuffer()    { return buffer_str; }
   static uint32_t getStackMax()  { return stack_max * UFlatBufferValue::size(); }
   static uint32_t getBufferMax() { return buffer_max; }

   // BUILD

   void StartBuild()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::StartBuild()")

      U_INTERNAL_ASSERT_POINTER( stack_str)
      U_INTERNAL_ASSERT_POINTER(buffer_str)

      // Reset all state so we can re-use the buffers

      reset();

      setStackPointer((stack_idx = 0));

      pvalue->reset();
      }

   uint32_t EndBuild()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::EndBuild()")

      // If you hit this assert, you likely have objects that were never included in a parent.
      // You need to have exactly one root to finish a buffer. Check your Start/End calls are
      // matched, and all objects are inside some other object

      U_INTERNAL_ASSERT_EQUALS(stack_idx, 1)

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)

      uint8_t byte_width = Align(((UFlatBufferValue*)stack_str)->ElemWidth(buffer_idx, 0));

      WriteAny(byte_width); // Write root value

      WriteScalar8(((UFlatBufferValue*)stack_str)->StoredPackedType(UFlatBufferValue::BIT_WIDTH_8)); // Write root type

      WriteScalar8(byte_width); // Write root size. Normally determined by parent, but root has no parent :)

      U_INTERNAL_DUMP("buffer(%u) = %#.*S", buffer_idx, buffer_idx, buffer_str)

      U_RETURN(buffer_idx);
      }

   template <typename F> uint32_t encode(F f)
      {
      U_TRACE(0, "UFlatBuffer::encode<F>(%p)", f)

      StartBuild();

      f();

      return EndBuild();
      }

   template <typename F> uint32_t encodeVector(F f, bool typed = false, bool fixed = false)
      {
      U_TRACE(0, "UFlatBuffer::encodeVector<F>(%p,%b,%b)", f, typed, fixed)

      StartBuild();

      (void) StartVector();

      f();

      EndVector(0, typed, fixed);

      return EndBuild();
      }

   template <typename F> uint32_t encodeMap(F f)
      {
      U_TRACE(0, "UFlatBuffer::encodeMap<F>(%p)", f)

      StartBuild();

      (void) StartMap();

      f();

      EndMap(0);

      return EndBuild();
      }

   void AddVectorEmpty()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::AddVectorEmpty()")

      buffer_str[buffer_idx++] = 0;

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)

      pushOnStack(buffer_idx, UFlatBufferValue::TYPE_VECTOR_BOOL, UFlatBufferValue::BIT_WIDTH_8);
      }

   void AddMapEmpty()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::AddMapEmpty()")

      u_put_unalignedp32(buffer_str+buffer_idx, U_MULTICHAR_CONSTANT32(0,1,1,0));

      pushOnStack((buffer_idx += 4), UFlatBufferValue::TYPE_MAP, UFlatBufferValue::BIT_WIDTH_8);

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)
      }

   // PARSE

   void setRoot(const uint8_t* ptr, uint32_t len) // See EndBuild() below for the serialization counterpart of this
      {
      U_TRACE(0, "UFlatBuffer::setRoot(%p,%u)", ptr, len)

      data_ = (buffer_str = (uint8_t*)ptr) + len; // The root starts at the end of the buffer, so we parse backwards from there

      parent_width_ = *--data_;

      setTypeAndWidth(*--data_);

      data_ -= parent_width_; // The root data item

      U_DUMP("type_ = (%u,%S) byte_width_ = %u parent_width_ = %u data_(%u) = %#.*S", type_, getTypeDescription(type_), byte_width_, parent_width_, 2+byte_width_, 2+byte_width_, data_)
      }

   void setRoot()                   { setRoot(buffer_str, buffer_idx); }
   void setRoot(const UString& str) { setRoot((const uint8_t*)U_STRING_TO_PARAM(str)); }

   bool     AsBool() const   { return AsBool(data_); }
    int64_t AsInt64() const  { return AsInt64(data_); }
   uint64_t AsUInt64() const { return AsUInt64(data_); }
   float    AsFloat() const  { return AsFloat(data_); }
   double   AsDouble() const { return AsDouble(data_); }
   UString  AsString() const { return AsString(data_); }

   int8_t  AsInt8()  const { return (int8_t) AsInt64(data_); }
   int16_t AsInt16() const { return (int16_t)AsInt64(data_); }
   int32_t AsInt32() const { return (int32_t)AsInt64(data_); }

    int64_t AsIndirectInt64() const  { return AsIndirectInt64(data_); }
   uint64_t AsIndirectUInt64() const { return AsIndirectUInt64(data_); }
   float    AsIndirectFloat() const  { return AsIndirectFloat(data_); }
   double   AsIndirectDouble() const { return AsIndirectDouble(data_); }

   int8_t  AsIndirectInt8()  const { return (int8_t) AsIndirectInt64(data_); }
   int16_t AsIndirectInt16() const { return (int16_t)AsIndirectInt64(data_); }
   int32_t AsIndirectInt32() const { return (int32_t)AsIndirectInt64(data_); }

   int64_t AsNumber() const
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::AsNumber()")

      if (type_ == UFlatBufferValue::TYPE_INT) return AsInt64();

      U_INTERNAL_ASSERT_EQUALS(type_, UFlatBufferValue::TYPE_UINT)

      return AsUInt64();
      }

   int64_t AsIndirectNumber() const
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::AsIndirectNumber()")

      if (type_ == UFlatBufferValue::TYPE_INDIRECT_INT) return AsIndirectInt64();

      U_INTERNAL_ASSERT_EQUALS(type_, UFlatBufferValue::TYPE_INDIRECT_UINT)

      return AsIndirectUInt64();
      }

   uint32_t GetSize() const
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::GetSize()")

      U_RETURN(buffer_idx);
      }

   // VECTOR

   void AsVector(UFlatBuffer& fb) const           { AsVector(data_, fb); }
   void AsTypedVector(UFlatBuffer& fb) const      { AsTypedVector(data_, fb); }
   void AsFixedTypedVector(UFlatBuffer& fb) const { AsFixedTypedVector(data_, fb); }

   template <typename T> T AsVectorGet(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGet<T>(%u)", i)

      T value = Get<T>(AsVectorSetIndex(i));

      type_ = UFlatBufferValue::TYPE_VECTOR;

      return value;
      }

   inline uint32_t AsVectorGetIPAddress(uint32_t i);

   template <typename T> T AsVectorGetIndirect(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGetIndirect<T>(%u)", i)

      T value = GetIndirect<T>(AsVectorSetIndex(i));

      type_ = UFlatBufferValue::TYPE_VECTOR;

      return value;
      }

   template <typename T> T AsTypedOrFixedVectorGet(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGet<T>(%u)", i)

      return Get<T>(AsTypedOrFixedVectorSetIndex(i));
      }

   template <typename T> T AsTypedOrFixedVectorGetIndirect(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGetIndirect<T>(%u)", i)

      return GetIndirect<T>(AsTypedOrFixedVectorSetIndex(i));
      }

   template <typename T> bool AsVectorIsEqual(const T* elems, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorIsEqual<T>(%p,%u)", elems, len)

      for (uint32_t i = 0; i < len; ++i)
         {
         if (AsVectorGet<T>(i) != elems[i]) U_RETURN(false); 
         }

      U_RETURN(true);
      }

   template <typename T> bool AsTypedOrFixedVectorIsEqual(const T* elems, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorIsEqual<T>(%p,%u)", elems, len)

      for (uint32_t i = 0; i < len; ++i)
         {
         if (AsTypedOrFixedVectorGet<T>(i) != elems[i]) U_RETURN(false); 
         }

      U_RETURN(true);
      }

   // MAP

   void AsMap(UFlatBuffer& fb) const          { AsMap(data_, fb); }
   void AsMapGetKeys(UFlatBuffer& fb) const   { AsMapGetKeys(data_, fb); }
   void AsMapGetValues(UFlatBuffer& fb) const { AsMapGetValues(data_, fb); }

   uint32_t AsMapGetKeys(UVector<UString>& members) const;

   template <typename T> T AsMapGet(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGet<T>(%u)", i)

      U_INTERNAL_ASSERT(IsMap())

      T value = Get<T>(AsVectorSetIndex(i));

      type_ = UFlatBufferValue::TYPE_MAP;

      return value;
      }

   template <typename T> T AsMapGetIndirect(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetIndirect<T>(%u)", i)

      U_INTERNAL_ASSERT(IsMap())

      T value = GetIndirect<T>(AsVectorSetIndex(i));

      type_ = UFlatBufferValue::TYPE_MAP;

      return value;
      }

   template <typename T> T AsMapGet(const char* key, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGet<T>(%.*S,%u)", len, key, len)

      U_INTERNAL_ASSERT(IsMap())

      T value = Get<T>(AsMapSetIndex(key, len));

      type_ = UFlatBufferValue::TYPE_MAP;

      return value;
      }

   template <typename T> T AsMapGetIndirect(const char* key, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetIndirect<T>(%.*S,%u)", len, key, len)

      U_INTERNAL_ASSERT(IsMap())

      T value = GetIndirect<T>(AsMapSetIndex(key, len));

      type_ = UFlatBufferValue::TYPE_MAP;

      return value;
      }

   // VECTOR <=> MAP

   void AsVectorGetVector(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGetVector(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsVector())

      AsVector(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_VECTOR;
      }

   void AsVectorGetTypedVector(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGetTypedVector(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsVector())

      AsTypedVector(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_VECTOR;
      }

   void AsVectorGetFixedTypedVector(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGetFixedTypedVector(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsVector())

      AsFixedTypedVector(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_VECTOR;
      }

   void AsVectorGetMap(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGetMap(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsVector())

      AsMap(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_VECTOR;
      }

   void AsTypedOrFixedVectorGetMap(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGetMap(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsTypedVector() || IsTypedVectorElementType())

      AsMap(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;
      }

   void AsMapGetVector(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetVector(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsVector(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetVector(const char* key, uint32_t len, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetVector(%.*S,%u,%p)", len, key, len, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsVector(AsMapSetIndex(key, len), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetTypedVector(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetTypedVector(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsTypedVector(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetTypedVector(const char* key, uint32_t len, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetTypedVector(%.*S,%u,%p)", len, key, len, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsTypedVector(AsMapSetIndex(key, len), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetFixedTypedVector(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetFixedTypedVector(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsFixedTypedVector(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetFixedTypedVector(const char* key, uint32_t len, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetFixedTypedVector(%.*S,%u,%p)", len, key, len, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsFixedTypedVector(AsMapSetIndex(key, len), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetMap(uint32_t i, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetMap(%u,%p)", i, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsMap(AsVectorSetIndex(i), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   void AsMapGetMap(const char* key, uint32_t len, UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetMap(%.*S,%u,%p)", len, key, len, &fb)

      U_INTERNAL_ASSERT(IsMap())

      AsMap(AsMapSetIndex(key, len), fb);

      byte_width_ = parent_width_;

      type_ = UFlatBufferValue::TYPE_MAP;
      }

   static UString fromVectorInt(uint32_t* vec)
      {
      U_TRACE(0, "UFlatBuffer::fromVectorInt(%p)", vec)

      return toVector(UFlatBuffer::setFromVectorInt, true, vec);
      }

   static void setFromVectorInt(UFlatBuffer* pfb, void* param)
      {
      uint32_t* vec = (uint32_t*)param;

      for (uint32_t i = 0; vec[i]; ++i) pfb->UInt(vec[i]);
      }

   static UString fromVectorString(UVector<UString>& vec)
      {
      U_TRACE(0, "UFlatBuffer::fromVectorString(%p)", &vec)

      return toVector(UFlatBuffer::setFromVectorString, true, &vec);
      }

   static void setFromVectorString(UFlatBuffer* pfb, void* param)
      {
      UVector<UString>* pvec = (UVector<UString>*)param;

      for (uint32_t i = 0, n = pvec->size(); i < n; ++i) pfb->String((*pvec)[i]);
      }

   static inline uint32_t toVectorInt(   const UString& data, uint32_t*         vec);
   static inline void     toVectorString(const UString& data, UVector<UString>& vec);

   static void    toVector(UString& result, vPFpfbpv func, bool typed = false, void* param = U_NULLPTR);
   static UString toVector(                 vPFpfbpv func, bool typed = false, void* param = U_NULLPTR);

#ifdef DEBUG
   const char* dump(bool reset) const;

   static const char* getTypeDescription(uint8_t type);
#else
   static const char* getTypeDescription(uint8_t) { return ""; }
#endif

protected:
   uint8_t* data_;
   uint32_t buffer_idx;
   uint8_t type_, byte_width_, parent_width_;

   static uint8_t*  stack_str;
   static uint8_t* buffer_str;
   static uint32_t stack_idx, stack_max, buffer_max;

   static UFlatBufferValue* pkeys;
   static UFlatBufferValue* pvalue;

   static void setStackPointer(uint32_t idx)
      {
      U_TRACE(0, "UFlatBuffer::setStackPointer(%u)", idx)

      U_INTERNAL_DUMP("stack_idx = %u stack_max = %u", stack_idx, stack_max)

      U_INTERNAL_ASSERT_MINOR(idx, stack_max)

      pvalue = (UFlatBufferValue*)(stack_str + (idx * UFlatBufferValue::size()));
      }

#ifdef DEBUG
   static void checkStackPointer()
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::checkStackPointer()")

      U_INTERNAL_DUMP("idx = %u stack_max = %u", ((uint8_t*)pvalue - stack_str) / UFlatBufferValue::size(), stack_max)

      U_INTERNAL_ASSERT_MINOR(((uint8_t*)pvalue - stack_str) / UFlatBufferValue::size(), stack_max)
      }
#endif

   static void nextStackPointer()
      {
      U_TRACE_NO_PARAM(0+256, "UFlatBuffer::nextStackPointer()")

      pvalue = (UFlatBufferValue*)((uint8_t*)pvalue + UFlatBufferValue::size());

#  ifdef DEBUG
      checkStackPointer();
#  endif
      }

   static void next2StackPointer()
      {
      U_TRACE_NO_PARAM(0+256, "UFlatBuffer::next2StackPointer()")

      pvalue = (UFlatBufferValue*)((uint8_t*)pvalue + (UFlatBufferValue::size()*2));

#  ifdef DEBUG
      checkStackPointer();
#  endif
      }

   static void pushOnStack(uint64_t u = 0ULL, uint8_t type = UFlatBufferValue::TYPE_NULL, uint8_t min_bit_width = UFlatBufferValue::BIT_WIDTH_8)
      {
      U_TRACE(0, "UFlatBuffer::pushOnStack(%llu,%u,%u)", u, type, min_bit_width)

      setStackPointer(stack_idx++);

      pvalue->set(u, type, min_bit_width);

      U_INTERNAL_DUMP("pvalue->l_ = %u pvalue->type_ = %u pvalue->min_bit_width_ = %u", pvalue->l_, pvalue->type_, pvalue->min_bit_width_)
      }

   static void pushOnStack(float f)
      {
      U_TRACE(0, "UFlatBufferValue::pushOnStack(%g)", f)

      setStackPointer(stack_idx++);

      pvalue->set(f);
      }

   static void pushOnStack(double f)
      {
      U_TRACE(0, "UFlatBufferValue::pushOnStack(%g)", f)

      setStackPointer(stack_idx++);

      pvalue->set(f);
      }

   static uint8_t GetByteWidth(uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::GetByteWidth(%u)", len)

      uint8_t bit_width = UFlatBufferValue::WidthL(len);

      U_RETURN(Align(bit_width));
      }

   void CreateString(const char* data, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::CreateString(%.*S,%u)", len, data, len)

      if (len == 0) StringNull();
      else
         {
         uint8_t bit_width = UFlatBufferValue::WidthL(len);

         WriteScalar<uint32_t>(len, Align(bit_width));

         uint32_t sloc = buffer_idx;

         WriteBytes(data, len);

         pushOnStack(sloc, UFlatBufferValue::TYPE_STRING, bit_width);
         }
      }

   void CreateVector(uint32_t start, uint32_t vec_len, uint32_t step, bool typed, bool fixed, UFlatBufferValue* pval);

   uint8_t ToTypedVectorElementType() const
      {
      U_TRACE_NO_PARAM(0, "UFlatBuffer::ToTypedVectorElementType()")

      U_INTERNAL_ASSERT(IsTypedVector())

      U_RETURN(type_ - UFlatBufferValue::TYPE_VECTOR_INT + UFlatBufferValue::TYPE_INT);
      }

   uint8_t ToFixedTypedVectorElementType(UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::ToFixedTypedVectorElementType(%p)", &fb)

      uint8_t fixed_type = type_ - UFlatBufferValue::TYPE_VECTOR_INT2;

      fb.buffer_idx = (uint8_t)(fixed_type / 3 + 2); // 3 types each, starting from length 2

      U_RETURN(fixed_type % 3 + UFlatBufferValue::TYPE_INT);
      }

   static bool IsTypedVectorElementType(uint8_t t)
      {
      U_TRACE(0, "UFlatBuffer::IsTypedVectorElementType(%u)", t)

      U_DUMP("t = %S", getTypeDescription(t))

      if (t >= UFlatBufferValue::TYPE_INT && t <= UFlatBufferValue::TYPE_STRING) U_RETURN(true);

      U_RETURN(false);
      }

   static uint8_t ToTypedVector(uint8_t t, uint8_t fixed_len)
      {
      U_TRACE(0, "UFlatBuffer::ToTypedVector(%u,%u)", t, fixed_len)

      U_INTERNAL_ASSERT(IsVector(t) || IsTypedVectorElementType(t))

      if (fixed_len == 0) U_RETURN(t - UFlatBufferValue::TYPE_INT + UFlatBufferValue::TYPE_VECTOR_INT);
      if (fixed_len == 2) U_RETURN(t - UFlatBufferValue::TYPE_INT + UFlatBufferValue::TYPE_VECTOR_INT2);
      if (fixed_len == 3) U_RETURN(t - UFlatBufferValue::TYPE_INT + UFlatBufferValue::TYPE_VECTOR_INT3);

      U_INTERNAL_ASSERT_EQUALS(fixed_len, 4)

      U_RETURN(t - UFlatBufferValue::TYPE_INT + UFlatBufferValue::TYPE_VECTOR_INT4);
      }

   bool IsTypedVectorElementType() const { return IsTypedVectorElementType(type_); }

   // WRITE

   void WriteBytes(const void* val, uint32_t len)
      {
      U_TRACE(0, "UFlatBuffer::WriteBytes<T>(%#.*S,%u)", len, val, len)

      U_INTERNAL_ASSERT_MAJOR(len, 0)

      (void) memcpy(getPointer(), val, len);

      buffer_idx += len;

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)
      }

   void WriteScalar8(uint8_t i)
      {
      U_TRACE(0, "UFlatBuffer::WriteScalar8(%u)", i)

      buffer_str[buffer_idx++] = i;

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)
      }

   void WriteScalar16(uint16_t i)
      {
      U_TRACE(0, "UFlatBuffer::WriteScalar16(%u)", i)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      u_put_unalignedp16(getPointer(),              i);
#  else
      u_put_unalignedp16(getPointer(), U_BYTESWAP16(i));
#  endif

      buffer_idx += sizeof(uint16_t);

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)
      }

   void WriteScalar32(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::WriteScalar32(%u)", i)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      u_put_unalignedp32(getPointer(),              i);
#  else
      u_put_unalignedp32(getPointer(), U_BYTESWAP32(i));
#  endif

      buffer_idx += sizeof(uint32_t);

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)
      }

   void WriteScalar64(uint64_t i)
      {
      U_TRACE(0, "UFlatBuffer::WriteScalar64(%llu)", i)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      u_put_unalignedp64(getPointer(),              i);
#  else
      u_put_unalignedp64(getPointer(), U_BYTESWAP64(i));
#  endif

      buffer_idx += sizeof(uint64_t);

      U_INTERNAL_DUMP("buffer_idx = %u buffer_max = %u", buffer_idx, buffer_max)

      U_INTERNAL_ASSERT_MINOR(buffer_idx, buffer_max)
      }

   template <typename T> void WriteScalar(T val, uint8_t byte_width)
      {
      U_TRACE(0, "UFlatBuffer::WriteScalar<T>(%p,%u)", &val, byte_width)

      if (byte_width < 4)
         {
         if (byte_width < 2) WriteScalar8(val);
         else                WriteScalar16(val);
         }
      else if (byte_width < 8) WriteScalar32(val);
      else                     WriteScalar64(val);
      }

   void WriteFloat(float f)
      {
      U_TRACE(0, "UFlatBuffer::WriteFloat(%g)", f)

      char* p = (char*)&f;

      WriteScalar32(*(uint32_t*)p);

      U_INTERNAL_DUMP("writeFloat = %#.4S", getPointer()-4)
      }

   void WriteDouble(double f, uint8_t byte_width)
      {
      U_TRACE(0, "UFlatBuffer::WriteDouble(%g,%u)", f, byte_width)

      if (byte_width == 4) WriteFloat(f);
      else
         {
         U_INTERNAL_ASSERT_EQUALS(byte_width, 8)
      
         char* p = (char*)&f;

         WriteScalar64(*(uint64_t*)p);

         U_INTERNAL_DUMP("writeDouble = %#.8S", getPointer()-8)
         }
      }

   void WriteAny(uint8_t byte_width);

   void WriteOffset(uint32_t o, uint8_t byte_width)
      {
      U_TRACE(0, "UFlatBuffer::WriteOffset(%u,%u)", o, byte_width)

      uint32_t reloff = buffer_idx - o;

      U_INTERNAL_DUMP("buffer_idx = %u reloff(%u) = %#.4S", buffer_idx, reloff, buffer_str + o)

      U_INTERNAL_ASSERT(byte_width == 8 || reloff < (1ULL << (byte_width * 8)))

      WriteScalar<uint32_t>(reloff, byte_width);
      }

   static uint8_t Align(uint8_t alignment) { return (1U << alignment); } // Align to prepare for writing a scalar with a certain size

   // READ

   static uint8_t ReadScalar8(const void* p)
      {
      U_TRACE(0, "UFlatBuffer::ReadScalar8(%p)", p)

      return *(const uint8_t*)p;
      }

   static uint16_t ReadScalar16(const void* p)
      {
      U_TRACE(0, "UFlatBuffer::ReadScalar16(%p)", p)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      return              u_get_unalignedp16(p);
#  else
      return U_BYTESWAP16(u_get_unalignedp16(p));
#  endif
      }

   static uint32_t ReadScalar32(const void* p)
      {
      U_TRACE(0, "UFlatBuffer::ReadScalar32(%p)", p)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      return              u_get_unalignedp32(p);
#  else
      return U_BYTESWAP32(u_get_unalignedp32(p));
#  endif
      }

   static uint64_t ReadScalar64(const void* p)
      {
      U_TRACE(0, "UFlatBuffer::ReadScalar64(%p)", p)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      return              u_get_unalignedp64(p);
#  else
      return U_BYTESWAP64(u_get_unalignedp64(p));
#  endif
      }

   template <typename T> static T ReadScalar(const void* p, uint8_t byte_width)
      {
      U_TRACE(0, "UFlatBuffer::ReadScalar<T>(%#.*S,%u)", sizeof(T), p, byte_width)

      if (byte_width < 4)
         {
         if (byte_width < 2) return ReadScalar8(p);

         return ReadScalar16(p);
         }

      if (byte_width < 8) return ReadScalar32(p);

      return ReadScalar64(p);
      }

   static  int64_t ReadInt64( const uint8_t* data, uint8_t byte_width) { return -ReadScalar<uint64_t>(data, byte_width); }
   static uint32_t ReadUInt32(const uint8_t* data, uint8_t byte_width) { return  ReadScalar<uint32_t>(data, byte_width); }
   static uint64_t ReadUInt64(const uint8_t* data, uint8_t byte_width) { return  ReadScalar<uint64_t>(data, byte_width); }

   static float ReadFloat(const void* p)
      {
      U_TRACE(0, "UFlatBuffer::ReadFloat(%p)", p)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      return              *(float*)p;
#  else
      return U_BYTESWAP32(*(float*)p);
#  endif
      }

   static double ReadDouble(const void* p)
      {
      U_TRACE(0, "UFlatBuffer::ReadDouble(%p)", p)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      return              *(double*)p;
#  else
      return U_BYTESWAP64(*(double*)p);
#  endif
      }

   static double ReadDouble(const uint8_t* data, uint8_t byte_width)
      {
      U_TRACE(0, "UFlatBuffer::ReadDouble(%#.8S,%u)", data, byte_width)

      if (byte_width == 4) return ReadFloat(data);

      U_INTERNAL_ASSERT_EQUALS(byte_width, 8)

      return ReadDouble(data);
      }

   static uint8_t* Indirect(const uint8_t* data, uint8_t parent_width)
      {
      U_TRACE(0, "UFlatBuffer::Indirect(%#.12S,%u)", data, parent_width)

      U_INTERNAL_DUMP("data-buffer_str = %u", data-buffer_str)

      uint32_t o = ReadUInt32(data, parent_width);

      U_INTERNAL_DUMP("o = %u", o)

      U_INTERNAL_DUMP("Indirect = %#.4S", (uint8_t*)data-o)

      return ((uint8_t*)data - o);
      }

   uint8_t* Indirect() { return Indirect(data_, parent_width_); }

   template <typename T> void PushIndirect(T val, uint8_t type, uint8_t bit_width)
      {
      U_TRACE(0, "UFlatBuffer::PushIndirect<T>(%p,%u,%u)", &val, type, bit_width)

      uint32_t iloc = buffer_idx;

      uint8_t byte_width = Align(bit_width);

      WriteScalar<T>(val, byte_width);

      pushOnStack(iloc, type, bit_width);
      }

   void PushIndirectFloat(float f)
      {
      U_TRACE(0, "UFlatBuffer::PushIndirectFloat(%g)", f)

      uint32_t iloc = buffer_idx;

      WriteFloat(f);

      pushOnStack(iloc, UFlatBufferValue::TYPE_INDIRECT_FLOAT, UFlatBufferValue::BIT_WIDTH_32);
      }

   uint32_t getSize(const uint8_t* data) const
      {
      U_TRACE(0, "UFlatBuffer::getSize(%#.4S)", data)

      U_INTERNAL_DUMP("byte_width_ = %u", byte_width_)

      uint32_t o = ReadUInt32(data - byte_width_, byte_width_);

      U_RETURN(o);
      }

   void setIndirect(uint8_t* ptr, UFlatBuffer& fb, uint8_t type) const
      {
      U_TRACE(0, "UFlatBuffer::setIndirect(%p,%p,%u)", ptr, &fb, type)

      U_INTERNAL_DUMP("type_ = %u byte_width_ = %u parent_width_ = %u", type_, byte_width_, parent_width_)

      fb.byte_width_   =
      fb.parent_width_ = byte_width_;
      fb.type_         = type;
      fb.buffer_idx    = fb.getSize((fb.data_ = Indirect(ptr, parent_width_)));

      U_INTERNAL_DUMP("fb.data(%u) = %#.*S", fb.buffer_idx, fb.buffer_idx, fb.data_)
      }

   void AsVector(uint8_t* ptr, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsVector(%p,%p)", ptr, &fb)

      U_INTERNAL_ASSERT(IsVector())

      setIndirect(ptr, fb, UFlatBufferValue::TYPE_VECTOR);
      }

   void AsTypedVector(uint8_t* ptr, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsTypedVector(%p,%p)", ptr, &fb)

      U_INTERNAL_ASSERT(IsTypedVector())

      setIndirect(ptr, fb, ToTypedVectorElementType());
      }

   void AsFixedTypedVector(uint8_t* ptr, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsFixedTypedVector(%p,%p)", ptr, &fb)

      U_INTERNAL_ASSERT(IsFixedTypedVector())

      fb.data_         = Indirect(ptr, parent_width_);
      fb.byte_width_   =
      fb.parent_width_ = byte_width_;
      fb.type_         = ToFixedTypedVectorElementType(fb);

      U_INTERNAL_DUMP("fb.data(%u) = %#.*S", fb.buffer_idx, fb.buffer_idx, fb.data_)
      }

   void AsMap(uint8_t* ptr, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsMap(%p,%p)", ptr, &fb)

      U_INTERNAL_ASSERT(IsMap())

      setIndirect(ptr, fb, UFlatBufferValue::TYPE_MAP);
      }

   void AsMapGetKeys(uint8_t* ptr, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetKeys(%p,%p)", ptr, &fb)

      U_DUMP("type_ = (%u,%S) byte_width_ = %u parent_width_ = %u", type_, getTypeDescription(type_), byte_width_, parent_width_)

      U_INTERNAL_ASSERT(IsMap())

      uint8_t* keys_offset = ptr - (byte_width_ * 3);

      fb.byte_width_   =
      fb.parent_width_ = ReadUInt32(keys_offset + byte_width_, byte_width_);
      fb.type_         = UFlatBufferValue::TYPE_STRING;
      fb.buffer_idx    = fb.getSize((fb.data_ = Indirect(keys_offset, byte_width_)));

      U_INTERNAL_DUMP("fb.data(%u) = %#.*S", fb.buffer_idx, fb.buffer_idx, fb.data_)
      }

   void AsMapGetValues(uint8_t* ptr, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsMapGetValues(%p,%p)", ptr, &fb)

      U_DUMP("type_ = (%u,%S) byte_width_ = %u parent_width_ = %u", type_, getTypeDescription(type_), byte_width_, parent_width_)

      U_INTERNAL_ASSERT(IsMap())

      fb.byte_width_   =
      fb.parent_width_ = byte_width_;
      fb.type_         = UFlatBufferValue::TYPE_VECTOR;
      fb.buffer_idx    = fb.getSize((fb.data_ = ptr));

      U_INTERNAL_DUMP("fb.data(%u) = %#.*S", fb.buffer_idx, fb.buffer_idx, fb.data_)
      }

   void setTypeAndWidth(uint8_t packed_type)
      {
      U_TRACE(0, "UFlatBuffer::setTypeAndWidth(%u)", packed_type)

      byte_width_ = 1U << (packed_type & 3);

      type_ = packed_type >> 2;

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S) packed_type = %u", parent_width_, byte_width_, type_, getTypeDescription(type_), packed_type)
      }

   uint8_t* AsVectorSetIndex(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsVectorSetIndex(%u)", i)

      U_DUMP("type_ = (%u,%S)", type_, getTypeDescription(type_))

      U_ASSERT_MINOR(i, buffer_idx)
      U_INTERNAL_ASSERT(IsVector() || IsMap())

      uint8_t* ptr = data_;
      uint32_t offset = i * (parent_width_ = byte_width_);

      setTypeAndWidth(ptr[(buffer_idx * byte_width_) + i]);

      return (ptr + offset);
      }

   uint8_t* AsTypedOrFixedVectorSetIndex(uint32_t i)
      {
      U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorSetIndex(%u)", i)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_ASSERT_MINOR(i, buffer_idx)
      U_INTERNAL_ASSERT(IsTypedVector() || IsTypedVectorElementType())

      uint8_t* ptr = (data_ + i * (parent_width_ = byte_width_));

      byte_width_ = 1;

      return ptr;
      }

   uint8_t* AsMapSetIndex(const char* key, uint32_t len);

   void AsVectorGet(uint32_t i, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsVectorGet(%u,%p)", i, &fb)

      U_ASSERT_MINOR(i, buffer_idx)
      U_INTERNAL_ASSERT(IsVector())

      fb.data_         = data_ + (i * byte_width_);
      fb.parent_width_ = parent_width_;

      fb.setTypeAndWidth(data_[(buffer_idx * byte_width_) + i]);

      fb.buffer_idx = fb.byte_width_;

      U_INTERNAL_DUMP("fb.data(%u) = %#.*S", fb.buffer_idx, fb.buffer_idx, fb.data_)
      }

   void AsTypedOrFixedVectorGet(uint32_t i, UFlatBuffer& fb) const
      {
      U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGet(%u,%p)", i, &fb)

      U_ASSERT_MINOR(i, buffer_idx)
      U_INTERNAL_ASSERT(IsTypedVector() || IsTypedVectorElementType())

      fb.buffer_idx  =
      fb.byte_width_ = 1;

      fb.data_         = data_ + (i * byte_width_);
      fb.parent_width_ = parent_width_;

      U_INTERNAL_DUMP("fb.data(%u) = %#.*S", fb.buffer_idx, fb.buffer_idx, fb.data_)
      }

   bool AsBool(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsBool(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsBool())

      if (ReadUInt64(ptr, parent_width_)) U_RETURN(true);

      U_RETURN(false);
      }

   uint64_t AsUInt64(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsUInt64(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsIntOrUint())

      return ReadUInt64(ptr, parent_width_);
      }

   uint64_t AsIndirectUInt64(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsIndirectUInt64(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsIntOrUint())

      return ReadUInt64(Indirect(ptr, parent_width_), byte_width_);
      }

   int64_t AsInt64(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsInt64(%p)", ptr)

      U_INTERNAL_ASSERT(IsInt())

      return ReadInt64(ptr, parent_width_);
      }

   int64_t AsIndirectInt64(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsIndirectInt64(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsInt())

      return ReadInt64(Indirect(ptr, parent_width_), byte_width_);
      }

   float AsFloat(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsFloat(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsFloat())

      U_INTERNAL_ASSERT_EQUALS(parent_width_, 4)

      return *(float*)ptr;
      }

   double AsIndirectFloat(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsIndirectFloat(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsFloat())

      U_INTERNAL_ASSERT_EQUALS(byte_width_, 4)

      return *(float*)Indirect(ptr, parent_width_);
      }

   double AsDouble(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsDouble(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsFloat())

      return ReadDouble(ptr, parent_width_);
      }

   double AsIndirectDouble(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsIndirectDouble(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      U_INTERNAL_ASSERT(IsFloat())

      return ReadDouble(Indirect(ptr, parent_width_), byte_width_);
      }

   bool GetBool(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetBool(%p)", ptr)

      bool value = AsBool(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   uint64_t GetUInt64(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetUInt64(%p)", ptr)

      uint64_t value = AsUInt64(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   uint64_t GetIndirectUInt64(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetIndirectUInt64(%p)", ptr)

      uint64_t value = AsIndirectUInt64(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   float GetFloat(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetFloat(%p)", ptr)

      float value = AsFloat(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   float GetIndirectFloat(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetIndirectFloat(%p)", ptr)

      float value = AsIndirectFloat(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   double GetDouble(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetDouble(%p)", ptr)

      double value = AsDouble(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   double GetIndirectDouble(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetIndirectDouble(%p)", ptr)

      double value = AsIndirectDouble(ptr);

      byte_width_ = parent_width_;

      U_RETURN(value);
      }

   UString AsString(const uint8_t* ptr) const
      {
      U_TRACE(0, "UFlatBuffer::AsString(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      if (IsString())
         {
         uint8_t* str = Indirect(ptr, parent_width_);

         uint32_t str_len = getSize(str);

         if (str_len)
            {
            UString x((const char*)str, str_len);

            U_RETURN_STRING(x);
            }
         }

      return UString::getStringNull();
      }

   UString GetString(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetString(%p)", ptr)

      UString x = AsString(ptr);

      byte_width_ = parent_width_;

      U_RETURN_STRING(x);
      }

   template <typename T> T As(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::As<T>(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      return ReadUInt64(ptr, parent_width_);
      }

   template <typename T> T AsIndirect(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::AsIndirect<T>(%p)", ptr)

      U_DUMP("parent_width_ = %u byte_width_ = %u type_ = (%u,%S)", parent_width_, byte_width_, type_, getTypeDescription(type_))

      return ReadUInt64(Indirect(ptr, parent_width_), byte_width_);
      }

   template <typename T> T Get(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::Get<T>(%p)", ptr)

      T value = As<T>(ptr);

      byte_width_ = parent_width_;

      return value;
      }

   template <typename T> T GetIndirect(const uint8_t* ptr)
      {
      U_TRACE(0, "UFlatBuffer::GetIndirect<T>(%p)", ptr)

      T value = AsIndirect<T>(ptr);

      byte_width_ = parent_width_;

      return value;
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UFlatBuffer)

   friend class UValue;

   template <class T> friend class UFlatBufferTypeHandler;
};

class U_EXPORT UFlatBufferSpaceShort {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UFlatBufferSpaceShort()
      {
      U_TRACE_CTOR(0, UFlatBufferSpaceShort, "")

      prev_stack       = UFlatBuffer::getStack();
      prev_buffer      = UFlatBuffer::getBuffer();
      prev_stack_size  = UFlatBuffer::getStackMax();
      prev_buffer_size = UFlatBuffer::getBufferMax();

      UFlatBuffer::setStack(  stack, sizeof(stack));
      UFlatBuffer::setBuffer(buffer, sizeof(buffer));
      }

   ~UFlatBufferSpaceShort()
      {
      U_TRACE_DTOR(0, UFlatBufferSpaceShort)

      UFlatBuffer::setStack( prev_stack,  prev_stack_size);
      UFlatBuffer::setBuffer(prev_buffer, prev_buffer_size);
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return ""; }
#endif

protected:
   uint8_t* prev_stack;
   uint8_t* prev_buffer;
   uint32_t prev_stack_size,
            prev_buffer_size;

   uint8_t stack[ 8U * 1024U],
          buffer[64U * 1024U];

private:
   U_DISALLOW_COPY_AND_ASSIGN(UFlatBufferSpaceShort)
};

class U_EXPORT UFlatBufferSpaceMedium {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UFlatBufferSpaceMedium()
      {
      U_TRACE_CTOR(0, UFlatBufferSpaceMedium, "")

      prev_stack       = UFlatBuffer::getStack();
      prev_buffer      = UFlatBuffer::getBuffer();
      prev_stack_size  = UFlatBuffer::getStackMax();
      prev_buffer_size = UFlatBuffer::getBufferMax();

      UFlatBuffer::setStack(  stack, sizeof(stack));
      UFlatBuffer::setBuffer(buffer, sizeof(buffer));
      }

   ~UFlatBufferSpaceMedium()
      {
      U_TRACE_DTOR(0, UFlatBufferSpaceMedium)

      UFlatBuffer::setStack( prev_stack,  prev_stack_size);
      UFlatBuffer::setBuffer(prev_buffer, prev_buffer_size);
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return ""; }
#endif

protected:
   uint8_t* prev_stack;
   uint8_t* prev_buffer;
   uint32_t prev_stack_size,
            prev_buffer_size;

   uint8_t stack[ 80U * 1024U],
          buffer[640U * 1024U];

private:
   U_DISALLOW_COPY_AND_ASSIGN(UFlatBufferSpaceMedium)
};

class U_EXPORT UFlatBufferSpaceLarge {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UFlatBufferSpaceLarge()
      {
      U_TRACE_CTOR(0, UFlatBufferSpaceLarge, "")

      prev_stack       = UFlatBuffer::getStack();
      prev_buffer      = UFlatBuffer::getBuffer();
      prev_stack_size  = UFlatBuffer::getStackMax();
      prev_buffer_size = UFlatBuffer::getBufferMax();

      UFlatBuffer::setStack(  stack, sizeof(stack));
      UFlatBuffer::setBuffer(buffer, sizeof(buffer));
      }

   ~UFlatBufferSpaceLarge()
      {
      U_TRACE_DTOR(0, UFlatBufferSpaceLarge)

      UFlatBuffer::setStack( prev_stack,  prev_stack_size);
      UFlatBuffer::setBuffer(prev_buffer, prev_buffer_size);
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return ""; }
#endif

protected:
   uint8_t* prev_stack;
   uint8_t* prev_buffer;
   uint32_t prev_stack_size,
            prev_buffer_size;

   uint8_t stack[1U * 1024U * 1024U],
          buffer[8U * 1024U * 1024U];

private:
   U_DISALLOW_COPY_AND_ASSIGN(UFlatBufferSpaceLarge)
};

#ifndef U_FLAT_BUFFERS_SPACE_STACK
#define U_FLAT_BUFFERS_SPACE_STACK  (1U * 1024U)
#endif
#ifndef U_FLAT_BUFFERS_SPACE_BUFFER
#define U_FLAT_BUFFERS_SPACE_BUFFER (8U * 1024U)
#endif

class U_EXPORT UFlatBufferSpaceUser {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UFlatBufferSpaceUser()
      {
      U_TRACE_CTOR(0, UFlatBufferSpaceUser, "")

      prev_stack       = UFlatBuffer::getStack();
      prev_buffer      = UFlatBuffer::getBuffer();
      prev_stack_size  = UFlatBuffer::getStackMax();
      prev_buffer_size = UFlatBuffer::getBufferMax();

      UFlatBuffer::setStack(  stack, sizeof(stack));
      UFlatBuffer::setBuffer(buffer, sizeof(buffer));
      }

   ~UFlatBufferSpaceUser()
      {
      U_TRACE_DTOR(0, UFlatBufferSpaceUser)

      UFlatBuffer::setStack( prev_stack,  prev_stack_size);
      UFlatBuffer::setBuffer(prev_buffer, prev_buffer_size);
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return ""; }
#endif

protected:
   uint8_t* prev_stack;
   uint8_t* prev_buffer;
   uint32_t prev_stack_size,
            prev_buffer_size;

   uint8_t stack[U_FLAT_BUFFERS_SPACE_STACK],
          buffer[U_FLAT_BUFFERS_SPACE_BUFFER];

private:
   U_DISALLOW_COPY_AND_ASSIGN(UFlatBufferSpaceUser)
};

// Template specialization

template<> inline bool     UFlatBuffer::As<bool>(    const uint8_t* ptr) { return AsBool(ptr); }
template<> inline uint64_t UFlatBuffer::As<uint64_t>(const uint8_t* ptr) { return AsUInt64(ptr); }
template<> inline float    UFlatBuffer::As<float>(   const uint8_t* ptr) { return AsFloat(ptr); }
template<> inline double   UFlatBuffer::As<double>(  const uint8_t* ptr) { return AsDouble(ptr); }
template<> inline UString  UFlatBuffer::As<UString>( const uint8_t* ptr) { return AsString(ptr); }

template<> inline uint64_t UFlatBuffer::AsIndirect<uint64_t>(const uint8_t* ptr) { return AsIndirectUInt64(ptr); }
template<> inline float    UFlatBuffer::AsIndirect<float>(   const uint8_t* ptr) { return AsIndirectFloat(ptr); }
template<> inline double   UFlatBuffer::AsIndirect<double>(  const uint8_t* ptr) { return AsIndirectDouble(ptr); }

template<> inline bool     UFlatBuffer::Get<bool>(    const uint8_t* ptr) { return GetBool(ptr); }
template<> inline uint64_t UFlatBuffer::Get<uint64_t>(const uint8_t* ptr) { return GetUInt64(ptr); }
template<> inline float    UFlatBuffer::Get<float>(   const uint8_t* ptr) { return GetFloat(ptr); }
template<> inline double   UFlatBuffer::Get<double>(  const uint8_t* ptr) { return GetDouble(ptr); }
template<> inline UString  UFlatBuffer::Get<UString>( const uint8_t* ptr) { return GetString(ptr); }

template<> inline uint64_t UFlatBuffer::GetIndirect<uint64_t>(const uint8_t* ptr) { return GetIndirectUInt64(ptr); }
template<> inline float    UFlatBuffer::GetIndirect<float>(   const uint8_t* ptr) { return GetIndirectFloat(ptr); }
template<> inline double   UFlatBuffer::GetIndirect<double>(  const uint8_t* ptr) { return GetIndirectDouble(ptr); }

// VECTOR

template<> inline bool UFlatBuffer::AsVectorGet<bool>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGet<bool>(%u)", i)

   bool value = Get<bool>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN(value);
}

template<> inline bool UFlatBuffer::AsTypedOrFixedVectorGet<bool>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGet<bool>(%u)", i)

   return Get<bool>(AsTypedOrFixedVectorSetIndex(i));
}

template<> inline uint32_t UFlatBuffer::AsVectorGet<uint32_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGet<uint32_t>(%u)", i)

   uint32_t value = Get<uint32_t>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN(value);
}

template<> inline uint64_t UFlatBuffer::AsVectorGet<uint64_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGet<uint64_t>(%u)", i)

   uint64_t value = Get<uint64_t>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN(value);
}

inline uint32_t UFlatBuffer::AsVectorGetIPAddress(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGetIPAddress(%u)", i)

   uint32_t value = Get<uint32_t>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   return htonl(value);
}

template<> inline uint64_t UFlatBuffer::AsTypedOrFixedVectorGet<uint64_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGet<uint64_t>(%u)", i)

   return Get<uint64_t>(AsTypedOrFixedVectorSetIndex(i));
}

#ifndef HAVE_OLD_IOSTREAM
template<> inline int8_t  UFlatBuffer::AsVectorGet<int8_t>( uint32_t i) { return -(int8_t) AsVectorGet<uint64_t>(i); }
template<> inline int16_t UFlatBuffer::AsVectorGet<int16_t>(uint32_t i) { return -(int16_t)AsVectorGet<uint64_t>(i); }
template<> inline int32_t UFlatBuffer::AsVectorGet<int32_t>(uint32_t i) { return -(int32_t)AsVectorGet<uint64_t>(i); }
template<> inline int64_t UFlatBuffer::AsVectorGet<int64_t>(uint32_t i) { return -(int64_t)AsVectorGet<uint64_t>(i); }
#endif

template<> inline uint64_t UFlatBuffer::AsVectorGetIndirect<uint64_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGetIndirect<uint64_t>(%u)", i)

   uint64_t value = GetIndirect<uint64_t>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN(value);
}

template<> inline uint64_t UFlatBuffer::AsTypedOrFixedVectorGetIndirect<uint64_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGetIndirect<uint64_t>(%u)", i)

   return GetIndirectUInt64(AsTypedOrFixedVectorSetIndex(i));
}

template<> inline int8_t  UFlatBuffer::AsVectorGetIndirect<int8_t>( uint32_t i) { return -(int8_t) AsVectorGetIndirect<uint64_t>(i); }
template<> inline int16_t UFlatBuffer::AsVectorGetIndirect<int16_t>(uint32_t i) { return -(int16_t)AsVectorGetIndirect<uint64_t>(i); }
template<> inline int32_t UFlatBuffer::AsVectorGetIndirect<int32_t>(uint32_t i) { return -(int32_t)AsVectorGetIndirect<uint64_t>(i); }
template<> inline int64_t UFlatBuffer::AsVectorGetIndirect<int64_t>(uint32_t i) { return -(int64_t)AsVectorGetIndirect<uint64_t>(i); }

#ifndef HAVE_OLD_IOSTREAM
template<> inline int8_t  UFlatBuffer::AsTypedOrFixedVectorGet<int8_t>( uint32_t i) { return -(int8_t) AsTypedOrFixedVectorGet<uint64_t>(i); }
template<> inline int16_t UFlatBuffer::AsTypedOrFixedVectorGet<int16_t>(uint32_t i) { return -(int16_t)AsTypedOrFixedVectorGet<uint64_t>(i); }
template<> inline int32_t UFlatBuffer::AsTypedOrFixedVectorGet<int32_t>(uint32_t i) { return -(int32_t)AsTypedOrFixedVectorGet<uint64_t>(i); }
template<> inline int64_t UFlatBuffer::AsTypedOrFixedVectorGet<int64_t>(uint32_t i) { return -(int64_t)AsTypedOrFixedVectorGet<uint64_t>(i); }
#endif

template<> inline int8_t  UFlatBuffer::AsTypedOrFixedVectorGetIndirect<int8_t>( uint32_t i) { return -(int8_t) AsTypedOrFixedVectorGetIndirect<uint64_t>(i); }
template<> inline int16_t UFlatBuffer::AsTypedOrFixedVectorGetIndirect<int16_t>(uint32_t i) { return -(int16_t)AsTypedOrFixedVectorGetIndirect<uint64_t>(i); }
template<> inline int32_t UFlatBuffer::AsTypedOrFixedVectorGetIndirect<int32_t>(uint32_t i) { return -(int32_t)AsTypedOrFixedVectorGetIndirect<uint64_t>(i); }
template<> inline int64_t UFlatBuffer::AsTypedOrFixedVectorGetIndirect<int64_t>(uint32_t i) { return -(int64_t)AsTypedOrFixedVectorGetIndirect<uint64_t>(i); }

template<> inline double UFlatBuffer::AsVectorGet<double>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGet<double>(%u)", i)

   double value = Get<double>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN(value);
}

template<> inline double UFlatBuffer::AsTypedOrFixedVectorGet<double>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGet<double>(%u)", i)

   return Get<double>(AsTypedOrFixedVectorSetIndex(i));
}

template<> inline double UFlatBuffer::AsVectorGetIndirect<double>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGetIndirect<double>(%u)", i)

   double value = GetIndirect<double>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN(value);
}

template<> inline double UFlatBuffer::AsTypedOrFixedVectorGetIndirect<double>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGetIndirect<double>(%u)", i)

   return GetIndirect<double>(AsTypedOrFixedVectorSetIndex(i));
}

template<> inline UString UFlatBuffer::AsVectorGet<UString>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsVectorGet<UString>(%u)", i)

   UString value = Get<UString>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_VECTOR;

   U_RETURN_STRING(value);
}

template<> inline UString UFlatBuffer::AsTypedOrFixedVectorGet<UString>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsTypedOrFixedVectorGet<UString>(%u)", i)

   return Get<UString>(AsTypedOrFixedVectorSetIndex(i));
}

// MAP

template<> inline bool UFlatBuffer::AsMapGet<bool>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<bool>(%u)", i)

   U_INTERNAL_ASSERT(IsMap())

   bool value = Get<bool>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline bool UFlatBuffer::AsMapGet<bool>(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<bool>(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   bool value = Get<bool>(AsMapSetIndex(key, len));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline uint64_t UFlatBuffer::AsMapGet<uint64_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<uint64_t>(%u)", i)

   U_INTERNAL_ASSERT(IsMap())

   uint64_t value = Get<uint64_t>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline uint64_t UFlatBuffer::AsMapGet<uint64_t>(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<uint64_t>(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   uint64_t value = Get<uint64_t>(AsMapSetIndex(key, len));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline int8_t  UFlatBuffer::AsMapGet<int8_t>( uint32_t i) { return -(int8_t) AsMapGet<uint64_t>(i); }
template<> inline int16_t UFlatBuffer::AsMapGet<int16_t>(uint32_t i) { return -(int16_t)AsMapGet<uint64_t>(i); }
template<> inline int32_t UFlatBuffer::AsMapGet<int32_t>(uint32_t i) { return -(int32_t)AsMapGet<uint64_t>(i); }
template<> inline int64_t UFlatBuffer::AsMapGet<int64_t>(uint32_t i) { return -(int64_t)AsMapGet<uint64_t>(i); }

template<> inline int8_t  UFlatBuffer::AsMapGet<int8_t>( const char* key, uint32_t len) { return -(int8_t) AsMapGet<uint64_t>(key, len); }
template<> inline int16_t UFlatBuffer::AsMapGet<int16_t>(const char* key, uint32_t len) { return -(int16_t)AsMapGet<uint64_t>(key, len); }
template<> inline int32_t UFlatBuffer::AsMapGet<int32_t>(const char* key, uint32_t len) { return -(int32_t)AsMapGet<uint64_t>(key, len); }
template<> inline int64_t UFlatBuffer::AsMapGet<int64_t>(const char* key, uint32_t len) { return -(int64_t)AsMapGet<uint64_t>(key, len); }

template<> inline uint64_t UFlatBuffer::AsMapGetIndirect<uint64_t>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsMapGetIndirect<uint64_t>(%u)", i)

   U_INTERNAL_ASSERT(IsMap())

   uint64_t value = GetIndirect<uint64_t>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline uint64_t UFlatBuffer::AsMapGetIndirect<uint64_t>(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapGetIndirect<uint64_t>(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   uint64_t value = GetIndirect<uint64_t>(AsMapSetIndex(key, len));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline int8_t  UFlatBuffer::AsMapGetIndirect<int8_t>( uint32_t i) { return -(int8_t) AsMapGetIndirect<uint64_t>(i); }
template<> inline int16_t UFlatBuffer::AsMapGetIndirect<int16_t>(uint32_t i) { return -(int16_t)AsMapGetIndirect<uint64_t>(i); }
template<> inline int32_t UFlatBuffer::AsMapGetIndirect<int32_t>(uint32_t i) { return -(int32_t)AsMapGetIndirect<uint64_t>(i); }
template<> inline int64_t UFlatBuffer::AsMapGetIndirect<int64_t>(uint32_t i) { return -(int64_t)AsMapGetIndirect<uint64_t>(i); }

template<> inline int8_t  UFlatBuffer::AsMapGetIndirect<int8_t>( const char* key, uint32_t len) { return -(int8_t) AsMapGetIndirect<uint64_t>(key, len); }
template<> inline int16_t UFlatBuffer::AsMapGetIndirect<int16_t>(const char* key, uint32_t len) { return -(int16_t)AsMapGetIndirect<uint64_t>(key, len); }
template<> inline int32_t UFlatBuffer::AsMapGetIndirect<int32_t>(const char* key, uint32_t len) { return -(int32_t)AsMapGetIndirect<uint64_t>(key, len); }
template<> inline int64_t UFlatBuffer::AsMapGetIndirect<int64_t>(const char* key, uint32_t len) { return -(int64_t)AsMapGetIndirect<uint64_t>(key, len); }

template<> inline double UFlatBuffer::AsMapGet<double>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<double>(%u)", i)

   U_INTERNAL_ASSERT(IsMap())

   double value = Get<double>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline double UFlatBuffer::AsMapGet<double>(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<double>(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   double value = Get<double>(AsMapSetIndex(key, len));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline double UFlatBuffer::AsMapGetIndirect<double>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsMapGetIndirect<double>(%u)", i)

   U_INTERNAL_ASSERT(IsMap())

   double value = GetIndirect<double>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline double UFlatBuffer::AsMapGetIndirect<double>(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapGetIndirect<double>(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   double value = GetIndirect<double>(AsMapSetIndex(key, len));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN(value);
}

template<> inline UString UFlatBuffer::AsMapGet<UString>(uint32_t i)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<UString>(%u)", i)

   U_INTERNAL_ASSERT(IsMap())

   UString value = Get<UString>(AsVectorSetIndex(i));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN_STRING(value);
}

template<> inline UString UFlatBuffer::AsMapGet<UString>(const char* key, uint32_t len)
{
   U_TRACE(0, "UFlatBuffer::AsMapGet<UString>(%.*S,%u)", len, key, len)

   U_INTERNAL_ASSERT(IsMap())

   UString value = Get<UString>(AsMapSetIndex(key, len));

   type_ = UFlatBufferValue::TYPE_MAP;

   U_RETURN_STRING(value);
}

inline uint32_t UFlatBuffer::toVectorInt(const UString& data, uint32_t* vec)
{
   U_TRACE(0, "UFlatBuffer::toVectorInt(%V,%p)", data.rep, vec)

   UFlatBuffer fb, vec1;

   fb.setRoot(data);
   fb.AsTypedVector(vec1);

   uint32_t n = vec1.GetSize();

#ifndef HAVE_OLD_IOSTREAM
   for (uint32_t i = 0; i < n; ++i) vec[i] = vec1.AsTypedOrFixedVectorGet<uint32_t>(i);
#endif

   U_RETURN(n);
}

inline void UFlatBuffer::toVectorString(const UString& data, UVector<UString>& vec)
{
   U_TRACE(0, "UFlatBuffer::toVectorString(%V,%p)", data.rep, &vec)

   UFlatBuffer fb, vec1;

   fb.setRoot(data);
   fb.AsTypedVector(vec1);

#ifndef HAVE_OLD_IOSTREAM
   for (uint32_t i = 0, n = vec1.GetSize(); i < n; ++i) vec.push_back(vec1.AsTypedOrFixedVectorGet<UString>(i));
#endif
}

// manage object <=> FlatBuffer representation

class U_EXPORT UFlatBufferTypeHandler_Base {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UFlatBufferTypeHandler_Base(const void* ptr) : pval((void*)ptr)
      {
      U_TRACE_CTOR(0, UFlatBufferTypeHandler_Base, "%p", ptr)

      U_INTERNAL_ASSERT_POINTER(pval)
      }

   ~UFlatBufferTypeHandler_Base()
      {
      U_TRACE_DTOR(0, UFlatBufferTypeHandler_Base)

      U_INTERNAL_ASSERT_POINTER(pval)
      }

#ifdef DEBUG
   const char* dump(bool _reset) const;
#endif

protected:
   void* pval;

private:
   U_DISALLOW_ASSIGN(UFlatBufferTypeHandler_Base)
};

#define FLATBUFFER(name_object_member, type_object_member) UFlatBufferTypeHandler<type_object_member>(name_object_member)

/**
 * Provide template specializations to support your own complex types
 *
 * Take as example the following (simplified) class:
 *
 * class Person {
 *   public:
 *      int age;
 *      UString  lastName;
 *      UString firstName;
 *   };
 *
 * add this methods to the (simplified) class:
 *
 * void Person::toFlatBuffer(UFlatBuffer& fb)
 *    {
 *    fb.toFlatBuffer(FLATBUFFER(age,       int));
 *    fb.toFlatBuffer(FLATBUFFER( lastName, UString));
 *    fb.toFlatBuffer(FLATBUFFER(firstName, UString));
 *    }
 *
 * void Person::fromFlatBuffer(UFlatBuffer& fb)
 *    {
 *    fb.fromFlatBuffer(0, FLATBUFFER(age,       int));
 *    fb.fromFlatBuffer(1, FLATBUFFER( lastName, UString));
 *    fb.fromFlatBuffer(2, FLATBUFFER(firstName, UString));
 *    }
 */

template <class T> class U_EXPORT UFlatBufferTypeHandler : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(T* val) : UFlatBufferTypeHandler_Base( val) {}
   explicit UFlatBufferTypeHandler(T& val) : UFlatBufferTypeHandler_Base(&val) {}

   // SERVICES

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<T>::toFlatBuffer(%p)", &fb)

      uint32_t start = fb.StartVector();

      ((T*)pval)->toFlatBuffer(fb);

      fb.EndVector(start, false);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<T>::fromFlatBuffer(%p)", &fb)

      UFlatBuffer vec;

      fb.AsVector(vec);

      ((T*)pval)->fromFlatBuffer(vec);
      }

private:
   U_DISALLOW_ASSIGN(UFlatBufferTypeHandler)
};

template <class T> void UFlatBuffer::fromObject(T& obj)
{
   U_TRACE(0, "UFlatBuffer::fromObject<T>(%p)", &obj)

   StartBuild();

   UFlatBufferTypeHandler<T>(obj).toFlatBuffer(*this);

   setRoot(buffer_str, EndBuild());
}

template <class T> void UFlatBuffer::toObject(T& obj)
{
   U_TRACE(0, "UFlatBuffer::toObject<T>(%p)", &obj)

   UFlatBufferTypeHandler<T>(obj).fromFlatBuffer(*this);
}

// TEMPLATE SPECIALIZATIONS

template <> class U_EXPORT UFlatBufferTypeHandler<null> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(null&) : UFlatBufferTypeHandler_Base(U_NULLPTR) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<null>::toFlatBuffer(%p)", &fb)

      fb.Null();
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<null>::fromFlatBuffer(%p)", &fb)

      U_VAR_UNUSED(fb)
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<bool> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(bool& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<bool>::toFlatBuffer(%p)", &fb)

      fb.Bool(*(bool*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<bool>::fromFlatBuffer(%p)", &fb)

      *(bool*)pval = fb.AsBool();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<char> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(char& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<char>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(char*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<char>::fromFlatBuffer(%p)", &fb)

      *(char*)pval = fb.AsUInt64();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<unsigned char> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(unsigned char& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned char>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(unsigned char*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned char>::fromFlatBuffer(%p)", &fb)

      *(unsigned char*)pval = fb.AsUInt64();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<short> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(short& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<short>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(short*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<short>::fromFlatBuffer(%p)", &fb)

      *(short*)pval = fb.AsUInt64();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<unsigned short> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(unsigned short& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned short>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(unsigned short*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned short>::fromFlatBuffer(%p)", &fb)

      *(unsigned short*)pval = fb.AsUInt64();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<int> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(int& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<int>::toFlatBuffer(%p)", &fb)

      fb.setNumber(*(int*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<int>::fromFlatBuffer(%p)", &fb)

      *(int*)pval = fb.AsNumber();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<unsigned int> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(unsigned int& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned int>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(unsigned int*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned int>::fromFlatBuffer(%p)", &fb)

      *(unsigned int*)pval = fb.AsUInt64();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<long> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(long& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<long>::toFlatBuffer(%p)", &fb)

      fb.setNumber(*(long*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<long>::fromFlatBuffer(%p)", &fb)

      *(long*)pval = fb.AsNumber();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<unsigned long> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(unsigned long& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned long>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(unsigned long*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned long>::fromFlatBuffer(%p)", &fb)

      *(unsigned long*)pval = fb.AsUInt64();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<long long> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(long long& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<long long>::toFlatBuffer(%p)", &fb)

      fb.setNumber(*(int64_t*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<long long>::fromFlatBuffer(%p)", &fb)

      *(long long*)pval = fb.AsNumber();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<unsigned long long> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(unsigned long long& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned long long>::toFlatBuffer(%p)", &fb)

      fb.UInt(*(uint64_t*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<unsigned long long>::fromFlatBuffer(%p)", &fb)

      *(unsigned long long*)pval = fb.AsNumber();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<float> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(float& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<float>::toFlatBuffer(%p)", &fb)

      fb.Double(*(float*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<float>::fromFlatBuffer(%p)", &fb)

      *(float*)pval = fb.AsDouble();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<double> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(double& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<double>::toFlatBuffer(%p)", &fb)

      fb.Double(*(double*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<double>::fromFlatBuffer(%p)", &fb)

      *(double*)pval = fb.AsDouble();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<long double> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(long double& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<long double>::toFlatBuffer(%p)", &fb)

      fb.Double(*(long double*)pval);
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<long double>::fromFlatBuffer(%p)", &fb)

      *(long double*)pval = fb.AsDouble();
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<UStringRep> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(UStringRep& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UStringRep>::toFlatBuffer(%p)", &fb)

      fb.String(U_STRING_TO_PARAM(*(UStringRep*)pval));
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UStringRep>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(fb.IsString())

      U_ERROR("UFlatBufferTypeHandler<UStringRep>::fromFlatBuffer(): sorry, we cannot use UStringRep type from FLATBUFFER handler...");
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<UString> : public UFlatBufferTypeHandler_Base {
public:
   explicit UFlatBufferTypeHandler(UString& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UString>::toFlatBuffer(%p)", &fb)

      fb.String(U_STRING_TO_PARAM(*(UString*)pval));
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UString>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(fb.IsString())

      *(UString*)pval = fb.AsString();
      }
};

// TEMPLATE SPECIALIZATIONS FOR CONTAINERS

template <class T> class U_EXPORT UFlatBufferTypeHandler<UVector<T*> > : public UFlatBufferTypeHandler_Base {
public:
   typedef UVector<T*> uvector;

   explicit UFlatBufferTypeHandler(uvector& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<uvector>::toFlatBuffer(%p)", &fb)

      uvector* pvec = (uvector*)pval;

      if (pvec->_length == 0) fb.AddVectorEmpty();
      else
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         uint32_t start = fb.StartVector();

         do { UFlatBufferTypeHandler<T>(*(T*)(*ptr)).toFlatBuffer(fb); } while (++ptr < end);

         fb.EndVector(start, false);
         }
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<uvector>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(((uvector*)pval)->empty())

      bool typed, fixed;
      UFlatBuffer vec, fbb;

      fixed = ((typed = fb.IsTypedVector()) ? false : fb.IsFixedTypedVector());

           if (typed) fb.AsTypedVector(vec);
      else if (fixed) fb.AsFixedTypedVector(vec), typed = true;
      else            fb.AsVector(vec);

      for (uint32_t i = 0, n = vec.GetSize(); i < n; ++i)
         {
         T* _pitem;

         U_NEW_WITHOUT_CHECK_MEMORY(T, _pitem, T);

         if (typed == false) vec.AsVectorGet(i, fbb);
         else                vec.AsTypedOrFixedVectorGet(i, fbb);

         UFlatBufferTypeHandler<T>(*_pitem).fromFlatBuffer(fbb);

         ((UVector<void*>*)pval)->push_back(_pitem);
         }
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<UVector<UString> > : public UFlatBufferTypeHandler<UVector<UStringRep*> > {
public:
   typedef UVector<UStringRep*> uvectorbase;

   explicit UFlatBufferTypeHandler(UVector<UString>& val) : UFlatBufferTypeHandler<uvectorbase>(*((uvector*)&val)) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UVector<UString>>::toFlatBuffer(%p)", &fb)

      uvectorbase* pvec = (uvectorbase*)pval;

      if (pvec->_length == 0) fb.AddVectorEmpty();
      else
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         uint32_t start = fb.StartVector();

         do { fb.String(U_STRING_TO_PARAM(*(const UStringRep*)(*ptr))); } while (++ptr < end);

         fb.EndVector(start);
         }
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UVector<UString>>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(((UVector<UString>*)pval)->empty())

      UFlatBuffer vec;

      fb.AsTypedVector(vec);

      for (uint32_t i = 0, n = vec.GetSize(); i < n; ++i)
         {
         ((UVector<UString>*)pval)->push_back(vec.AsTypedOrFixedVectorGet<UString>(i));
         }
      }
};

template <class T> class U_EXPORT UFlatBufferTypeHandler<UHashMap<T*> > : public UFlatBufferTypeHandler_Base {
public:
   typedef UHashMap<T*> uhashmap;

   explicit UFlatBufferTypeHandler(uhashmap& map) : UFlatBufferTypeHandler_Base(&map) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<uhashmap>::toFlatBuffer(%p)", &fb)

      uhashmap* pmap = (uhashmap*)pval;

      if (pmap->first() == false) fb.AddMapEmpty();
      else
         {
         uint32_t start = fb.StartMap();

         do {
            fb.Key(U_STRING_TO_PARAM(pmap->getKey()));

            UFlatBufferTypeHandler<T>(*(pmap->elem())).toFlatBuffer(fb);
            }
         while (pmap->next());

         fb.EndMap(start);
         }
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<uhashmap>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(((uhashmap*)pval)->empty())

      UFlatBuffer map, keys, values, fbb;

      fb.AsMap(map);

      map.AsMapGetKeys(keys);
      map.AsMapGetValues(values);

      for (uint32_t i = 0, n = map.GetSize(); i < n; ++i)
         {
         T* _pitem;

         U_NEW_WITHOUT_CHECK_MEMORY(T, _pitem, T);

         values.AsVectorGet(i, fbb);

         UFlatBufferTypeHandler<T>(*_pitem).fromFlatBuffer(fbb);

#     ifndef HAVE_OLD_IOSTREAM
         ((uhashmap*)pval)->insert(keys.AsTypedOrFixedVectorGet<UString>(i), _pitem);
#     endif
         }
      }
};

template <> class U_EXPORT UFlatBufferTypeHandler<UHashMap<UString> > : public UFlatBufferTypeHandler<UHashMap<UStringRep*> > {
public:
   typedef UHashMap<UStringRep*> uhashmapbase;

   explicit UFlatBufferTypeHandler(UHashMap<UString>& val) : UFlatBufferTypeHandler<uhashmapbase>(*((uhashmap*)&val)) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UHashMap<UString>>::toFlatBuffer(%p)", &fb)

      UHashMap<UString>* pmap = (UHashMap<UString>*)pval;

      if (pmap->first() == false) fb.AddMapEmpty();
      else
         {
         uint32_t start = fb.StartMap();

         do {
            fb.Key(U_STRING_TO_PARAM(pmap->getKey()));

            fb.String(U_STRING_TO_PARAM(*(const UStringRep*)pmap->elem()));
            }
         while (pmap->next());

         fb.EndMap(start);
         }
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<UHashMap<UString>>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(((UHashMap<UString>*)pval)->empty())

      UFlatBuffer map, keys, values, fbb;

      fb.AsMap(map);

      map.AsMapGetKeys(keys);
      map.AsMapGetValues(values);

      for (uint32_t i = 0, n = map.GetSize(); i < n; ++i)
         {
         ((UHashMap<UString>*)pval)->insert(keys.AsTypedOrFixedVectorGet<UString>(i), values.AsVectorGet<UString>(i));
         }
      }
};

#ifdef U_STDCPP_ENABLE
#  include <vector>

template <class T> class U_EXPORT UFlatBufferTypeHandler<std::vector<T> > : public UFlatBufferTypeHandler_Base {
public:
   typedef std::vector<T> stdvector;

   explicit UFlatBufferTypeHandler(stdvector& val) : UFlatBufferTypeHandler_Base(&val) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<stdvector>::toFlatBuffer(%p)", &fb)

      stdvector* pvec = (stdvector*)pval;

      uint32_t n = pvec->size();

      if (n == 0) fb.AddVectorEmpty();
      else
         {
         uint32_t start = fb.StartVector();

         for (uint32_t i = 0; i < n; ++i) UFlatBufferTypeHandler<T>(pvec->at(i)).toFlatBuffer(fb);

         fb.EndVector(start);
         }
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<stdvector>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(((stdvector*)pval)->empty())

      bool typed, fixed;
      UFlatBuffer vec, fbb;

      fixed = ((typed = fb.IsTypedVector()) ? false : fb.IsFixedTypedVector());

           if (typed) fb.AsTypedVector(vec);
      else if (fixed) fb.AsFixedTypedVector(vec), typed = true;
      else            fb.AsVector(vec);

      for (uint32_t i = 0, n = vec.GetSize(); i < n; ++i)
         {
         T item;

         if (typed == false) vec.AsVectorGet(i, fbb);
         else                vec.AsTypedOrFixedVectorGet(i, fbb);

         UFlatBufferTypeHandler<T>(item).fromFlatBuffer(fbb);

         ((stdvector*)pval)->push_back(item);
         }
      }
};

# if defined(HAVE_CXX17) && !defined(__clang__)
#  include <unordered_map>

template <class T> class U_EXPORT UFlatBufferTypeHandler<std::unordered_map<UString, T> > : public UFlatBufferTypeHandler_Base {
public:
   typedef std::unordered_map<UString, T> stringtobitmaskmap;

   explicit UFlatBufferTypeHandler(stringtobitmaskmap& map) : UFlatBufferTypeHandler_Base(&map) {}

   void toFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<stringtobitmaskmap>::toFlatBuffer(%p)", &fb)

      stringtobitmaskmap* pmap = (stringtobitmaskmap*)pval;

      if (pmap->empty()) fb.AddMapEmpty(); 
      else
         {
         uint32_t start = fb.StartMap();

         // this is is C++17 vvv
         for (const auto & [ key, value ] : *pmap) fb.toFlatBuffer<T>(key, UFlatBufferTypeHandler<T>(value)); 

         fb.EndMap(start);
         }
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(0, "UFlatBufferTypeHandler<stringtobitmaskmap>::fromFlatBuffer(%p)", &fb)

      U_ASSERT(((stringtobitmaskmap*)pval)->empty())

      UFlatBuffer map, keys, values, fbb;

      fb.AsMap(map);

      map.AsMapGetKeys(keys);
      map.AsMapGetValues(values);

      for (uint32_t i = 0, n = map.GetSize(); i < n; ++i)
         {
         T* _pitem;

         U_NEW_WITHOUT_CHECK_MEMORY(T, _pitem, T);

         values.AsVectorGet(i, fbb);

         UFlatBufferTypeHandler<T>(*_pitem).fromFlatBuffer(fbb);

         ((stringtobitmaskmap*)pval)->insert_or_assign(keys.AsTypedOrFixedVectorGet<UString>(i), _pitem); // insert_or_assign is C++17 
         }
      }
};
# endif
#endif
#endif
