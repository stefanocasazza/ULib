// test_serialize.cpp

#include <ulib/file.h>
#include <ulib/utility/hexdump.h>

#include "json_obj.h"

static void check(UFlatBuffer& fb, const UString& to_parse, const UString& expect)
{
   U_TRACE(5, "check(%p,%V,%V)", &fb, to_parse.rep, expect.rep)

   UString result = fb.getResult();

   U_ASSERT_EQUALS(result, expect)

   UValue json;
   bool ok = json.parse(to_parse);

   U_INTERNAL_ASSERT(ok)

   result = json.toFlatBuffer();

   U_ASSERT_EQUALS(result, expect)

   json.clear();
   json.fromFlatBuffer(fb);

   result = json.output();

   U_ASSERT_EQUALS(result, to_parse)
}

int U_EXPORT main(int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)

   uint64_t i, size;
   uint8_t stack[1024];
   UFlatBuffer fb, vec, vec0, map, map0;
   uint64_t vec1[] = {1, 2, 3, 7}; 
   bool     vec2[] = {true, false, true};
   double   vec3[] = {1.0, 0.2, 0.5, 5.6};

   UFlatBuffer::setStack(stack, sizeof(stack));

   Request().testFlatBuffer();
   Response().testFlatBuffer();
   ResponseLogin().testFlatBuffer();
   ResponseSearch().testFlatBuffer();

   // Write the json equivalent of: ""

   size = fb.encode([&]() { fb.String(U_CONSTANT_TO_PARAM("")); });

   fb.setRoot();

   U_ASSERT(fb.AsString().isNull())

   check(fb, U_STRING_FROM_CONSTANT("\"\""), U_HEXDUMP("00001401"));

   // Write the json equivalent of: []

   fb.StartBuild();
   fb.AddVectorEmpty();
   (void) fb.EndBuild();

   fb.setRoot();
   fb.AsTypedVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 0)

   check(fb, U_STRING_FROM_CONSTANT("[]"), U_HEXDUMP("00003801"));

   // Write the json equivalent of: {}

   fb.StartBuild();
   fb.AddMapEmpty();
   (void) fb.EndBuild();

   fb.setRoot();
   fb.AsMap(map);

   U_ASSERT_EQUALS(map.GetSize(), 0)

   check(fb, U_STRING_FROM_CONSTANT("{}"), U_HEXDUMP("00010100002401"));

   // Write the json equivalent of: 140737488355327 

   size = fb.encode([&]() { fb.UInt(U_VALUE_PAYLOAD_MASK); });

   fb.setRoot();
   i = fb.AsUInt64();

   U_INTERNAL_ASSERT_EQUALS(i, U_VALUE_PAYLOAD_MASK)

   check(fb, U_STRING_FROM_CONSTANT("140737488355327"), U_HEXDUMP("ffffffffff7f00000b08"));

   // Write the json equivalent of: "Fred" 

   size = fb.encode([&]() { fb.String(U_CONSTANT_TO_PARAM("Fred")); });

   fb.setRoot();
   UString x = fb.AsString();

   U_ASSERT_EQUALS(x, "Fred")

   check(fb, U_STRING_FROM_CONSTANT("\"Fred\""), U_STRING_FROM_CONSTANT("\004Fred\004\024\001"));

   // Write the json equivalent of: [ 1, 2 ]

   size = fb.encodeVector([&]() {
      fb.UInt(1);
      fb.UInt(2);
   }, true, true);

   fb.setRoot();
   fb.AsFixedTypedVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 2)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec.AsTypedOrFixedVectorIsEqual(vec1, 2))

   check(fb, U_STRING_FROM_CONSTANT("[1,2]"), U_STRING_FROM_CONSTANT("\001\002\002D\001"));

   // Write the json equivalent of: [ true, false, true ]

   size = fb.encodeVector([&]() {
      fb.Bool(true);
      fb.Bool(false);
      fb.Bool(true);
   }, true);

   fb.setRoot();
   fb.AsTypedVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<bool>(0), true)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<bool>(1), false)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<bool>(2), true)
   U_ASSERT(vec.AsTypedOrFixedVectorIsEqual(vec2, 3))

   check(fb, U_STRING_FROM_CONSTANT("[true,false,true]"), U_STRING_FROM_CONSTANT("\003\001\000\001\0038\001"));

   // Write the json equivalent of: [ 1, 2, 3 ]

   size = fb.encodeVector([&]() {
      fb.UInt(1);
      fb.UInt(2);
      fb.UInt(3);
   }, true, true);

   fb.setRoot();
   fb.AsFixedTypedVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<uint64_t>(2), 3)
   U_ASSERT(vec.AsTypedOrFixedVectorIsEqual(vec1, 3))

   check(fb, U_STRING_FROM_CONSTANT("[1,2,3]"), U_STRING_FROM_CONSTANT("\001\002\003\003P\001"));

   // Write the json equivalent of: [ 1.0, 0.2, 0.5, 5.6 ]

   size = fb.encodeVector([&]() {
      fb.Double(1.0);
      fb.Double(0.2);
      fb.Double(0.5);
      fb.Double(5.6);
   }, true, true);

   fb.setRoot();
   fb.AsFixedTypedVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 4)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<double>(0), 1.0)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<double>(1), 0.2)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<double>(2), 0.5)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<double>(3), 5.6)
   U_ASSERT(vec.AsTypedOrFixedVectorIsEqual<double>(vec3, 4))

   check(fb, U_STRING_FROM_CONSTANT("[1.0,0.2,0.5,5.6]"), U_HEXDUMP("000000000000f03f9a9999999999c93f000000000000e03f6666666666661640206301"));

   // Write the json equivalent of: [ "uno", "due", "tre" ]

   size = fb.encodeVector([&]() {
      fb.String(U_CONSTANT_TO_PARAM("uno"));
      fb.String(U_CONSTANT_TO_PARAM("due"));
      fb.String(U_CONSTANT_TO_PARAM("tre"));
   }, true);

   fb.setRoot();
   fb.AsTypedVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "uno")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(1), "due")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(2), "tre")

   check(fb, U_STRING_FROM_CONSTANT("[\"uno\",\"due\",\"tre\"]"), U_STRING_FROM_CONSTANT("\003uno\003due\003tre\003\f\t\006\003<\001"));

   size = fb.encodeVector([&]() {
      fb.String(U_CONSTANT_TO_PARAM("9cvxHmjzuQzzCaw2LMTvjmyMeRXM8mzXY"));
      fb.String(U_CONSTANT_TO_PARAM("vLivrQb9dqcRyiRa1LENgBnsSpEbFsOcN"));
      fb.String(U_CONSTANT_TO_PARAM("uELEMyIieNW86ruETPaISDBlnn5UOFTZr"));
      fb.String(U_CONSTANT_TO_PARAM("pSqtl0fITijC8BbGKvJTaSrqhgNBRDeJX"));
      fb.String(U_CONSTANT_TO_PARAM("sreDFtqY9a3aRU0y4PrnX4VLTJvNJUjh6"));
      fb.String(U_CONSTANT_TO_PARAM("1MKsg9wknND12SHLuM3NbXcO2hSxRhck8"));
      fb.String(U_CONSTANT_TO_PARAM("akItwhgG2JVXhUldOuSgBzrhQydIcBRRq"));
      fb.String(U_CONSTANT_TO_PARAM("RYF17ULTOh0l4NLazP9UGYewqLIWJcMk3"));
   }, true);

   fb.setRoot();
   fb.AsTypedVector(vec);

// (void) UFile::writeToTmp(U_STRING_TO_PARAM(fb.getResult()), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("test_serialize.%P"), 0);

   U_ASSERT_EQUALS(vec.GetSize(), 8)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "9cvxHmjzuQzzCaw2LMTvjmyMeRXM8mzXY")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(1), "vLivrQb9dqcRyiRa1LENgBnsSpEbFsOcN")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(2), "uELEMyIieNW86ruETPaISDBlnn5UOFTZr")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(3), "pSqtl0fITijC8BbGKvJTaSrqhgNBRDeJX")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(4), "sreDFtqY9a3aRU0y4PrnX4VLTJvNJUjh6")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(5), "1MKsg9wknND12SHLuM3NbXcO2hSxRhck8")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(7), "RYF17ULTOh0l4NLazP9UGYewqLIWJcMk3")

   // Write the json equivalent of: [ 100, true, 4.0, "Fred" ]

   size = fb.encodeVector([&]() {
      fb.UInt(100);
      fb.Bool(true);
      fb.Double(4.0);
      fb.String(U_CONSTANT_TO_PARAM("Fred"));
   });

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 4)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(0), 100)
   U_ASSERT_EQUALS(vec.AsVectorGet<bool>(1), true)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(2), 4.0)
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(3), "Fred")

   check(fb, U_STRING_FROM_CONSTANT("[100,true,4.0,\"Fred\"]"), U_HEXDUMP("044672656404000000640000000100000000008040140000000a120e14142a01"));

   // Write the json equivalent of: [ [ 1, 2 ], [ 1, 2 ], [ 1, 2 ] ]

   size = fb.encodeVector([&]() {
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
   });

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)

   vec.AsVectorGetFixedTypedVector(0, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   vec.AsVectorGetFixedTypedVector(1, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   vec.AsVectorGetFixedTypedVector(2, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   check(fb, U_STRING_FROM_CONSTANT("[[1,2],[1,2],[1,2]]"), U_HEXDUMP("01020102010203070605444444062801"));

   // Write the json equivalent of: [ 1.0, [ ], [ 1, 2 ], [ ], [ 1, 2 ], [ ] ]

   size = fb.encodeVector([&]() {
      fb.Double(1.0);
      fb.AddVectorEmpty();
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
      fb.AddVectorEmpty();
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
      fb.AddVectorEmpty();
   });

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 6)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(0), 1.0)

   vec.AsVectorGetFixedTypedVector(2, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   vec.AsVectorGetFixedTypedVector(4, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   check(fb, U_STRING_FROM_CONSTANT("[1.0,[],[1,2],[],[1,2],[]]"), U_HEXDUMP("00010200010200060000000000803f0e000000120000001300000017000000180000000e38443844381e2a01"));

   // Write the json equivalent of: [ 1.0, [], [], [ "uno", "due", "tre" ] ]

   size = fb.encodeVector([&]() {
      fb.Double(1.0);
      fb.AddVectorEmpty();
      fb.AddVectorEmpty();
      fb.Vector([&]() {
         fb.String(U_CONSTANT_TO_PARAM("uno"));
         fb.String(U_CONSTANT_TO_PARAM("due"));
         fb.String(U_CONSTANT_TO_PARAM("tre"));
      }, true);
   });

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 4)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(0), 1.0)
   vec.AsVectorGetTypedVector(1, vec0);
   U_ASSERT_EQUALS(vec0.GetSize(), 0)
   vec.AsVectorGetTypedVector(2, vec0);
   U_ASSERT_EQUALS(vec0.GetSize(), 0)

   vec.AsVectorGetTypedVector(3, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(0), "uno")
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(1), "due")
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(2), "tre")

   check(fb, U_STRING_FROM_CONSTANT("[1.0,[],[],[\"uno\",\"due\",\"tre\"]]"), U_HEXDUMP("000003756e6f0364756503747265030c0906040000000000803f190000001c000000130000000e38383c142a01"));

   // Write the json equivalent of: [ 1.0, {}, [ 1, 2 ], {}, [ 1, 2 ], {} ]

   size = fb.encodeVector([&]() {
      fb.Double(1.0);
      fb.AddMapEmpty();
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
      fb.AddMapEmpty();
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(2);
      }, true, true);
      fb.AddMapEmpty();
   });

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 6)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(0), 1.0)

   vec.AsVectorGetFixedTypedVector(2, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   vec.AsVectorGetFixedTypedVector(4, vec0);

   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)
   U_ASSERT(vec0.AsTypedOrFixedVectorIsEqual(vec1, 2))

   check(fb, U_STRING_FROM_CONSTANT("[1.0,{},[1,2],{},[1,2],{}]"), U_HEXDUMP("00010100010200010100010200010100060000000000803f1400000018000000160000001a000000180000000e24442444241e2a01"));

   // Write the json equivalent of: { foo: 13, bar: 14 }

   size = fb.encodeMap([&]() {
      fb.UInt(U_CONSTANT_TO_PARAM("foo"), 13);
      fb.UInt(U_CONSTANT_TO_PARAM("bar"), 14);
   });

   fb.setRoot();
   fb.AsMap(map);

   U_ASSERT_EQUALS(map.GetSize(), 2)

   map.AsMapGetKeys(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 2)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "foo")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(1), "bar")

   UVector<UString> v;

   U_ASSERT_EQUALS(map.AsMapGetKeys(v), 2)
   U_ASSERT_EQUALS(v[0], "foo")
   U_ASSERT_EQUALS(v[1], "bar")

   map.AsMapGetValues(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 2)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(0), 13)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(1), 14)

   U_ASSERT_EQUALS(map.AsMapGet<uint64_t>(0), 13)
   U_ASSERT_EQUALS(map.AsMapGet<uint64_t>(1), 14)
   U_ASSERT_EQUALS(map.AsMapGet<uint64_t>(U_CONSTANT_TO_PARAM("foo")), 13)
   U_ASSERT_EQUALS(map.AsMapGet<uint64_t>(U_CONSTANT_TO_PARAM("bar")), 14)

   check(fb, U_STRING_FROM_CONSTANT("{\"foo\":13,\"bar\":14}"), U_HEXDUMP("03666f6f036261720208050201020d0e0808042401"));

   // Write the json equivalent of: { vec: [ -100, "Fred", 4.0 ], foo: 100 }

   size = fb.encodeMap([&]() {
      fb.Key(U_CONSTANT_TO_PARAM("vec"));
      fb.Vector([&]() {
         fb.Int(-100);
         fb.String(U_CONSTANT_TO_PARAM("Fred"));
         fb.Double(4.0);
      });
      fb.UInt(U_CONSTANT_TO_PARAM("foo"), 100);
   });

   fb.setRoot();
   fb.AsMap(map);

   U_ASSERT_EQUALS(map.GetSize(), 2)

   map.AsMapGetKeys(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 2)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "vec")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(1), "foo")

   map.AsMapGetValues(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 2)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(1), 100)

   U_ASSERT_EQUALS(map.AsMapGet<uint64_t>(1), 100)
   U_ASSERT_EQUALS(map.AsMapGet<uint64_t>(U_CONSTANT_TO_PARAM("foo")), 100)

   map.AsMapGetVector(0, vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsVectorGet<int8_t>(0), -100)
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(1), "Fred")
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(2), 4.0)

   map.AsMapGetVector(U_CONSTANT_TO_PARAM("vec"), vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsVectorGet<int8_t>(0), -100)
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(1), "Fred")
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(2), 4.0)

   check(fb, U_STRING_FROM_CONSTANT("{\"vec\":[-100,\"Fred\",4.0],\"foo\":100}"), U_HEXDUMP("03766563044672656403000000640000000c0000000000804006140e03666f6f02200502010219642a08042401"));

   size = fb.encodeVector([&]() {
      fb.String(U_CONSTANT_TO_PARAM("hello"));
      fb.Map([&]() {
         fb.UInt(U_CONSTANT_TO_PARAM("a"), 12);
         fb.UInt(U_CONSTANT_TO_PARAM("b"), 4);
      });
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(3);
      });
      fb.Map([&]() {
         fb.UInt(U_CONSTANT_TO_PARAM("a"), 12);
         fb.UInt(U_CONSTANT_TO_PARAM("b"), 4);
      });
   });

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 4)
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(0), "hello")

   vec.AsVectorGetVector(2, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(1), 3)

   vec.AsVectorGetMap(1, map0);

   U_ASSERT_EQUALS(map0.GetSize(), 2)

   map0.AsMapGetKeys(vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(0), "a")
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(1), "b")

   map0.AsMapGetValues(vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(0), 12)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(1), 4)

   vec.AsVectorGetMap(3, map0);

   U_ASSERT_EQUALS(map0.GetSize(), 2)

   map0.AsMapGetKeys(vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(0), "a")
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(1), "b")

   map0.AsMapGetValues(vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(0), 12)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(1), 4)

   size = fb.encodeVector([&]() {
      fb.UInt(1);
      fb.Int(INT_MIN);
      fb.UInt(2);
      fb.UInt(UINT_MAX);
      fb.Add(3.0);
      fb.Add(true);
      fb.String(U_CONSTANT_TO_PARAM("Hello"));
      fb.String(U_CONSTANT_TO_PARAM("Hello!"));
      fb.Add();
      fb.String(U_CONSTANT_TO_PARAM("a"));
      fb.String(U_CONSTANT_TO_PARAM("b"));
      fb.String(U_CONSTANT_TO_PARAM("a!"));
      fb.String(U_CONSTANT_TO_PARAM("b!"));
      fb.FixedTypedVector(vec1, 3);
      fb.FixedTypedVector(vec1, 3);
      fb.TypedVector(vec2, 2);
      fb.TypedVector(vec3, 3);
      vec3[1] = 2.3;
      vec3[2] = 4.5;
      fb.TypedVector(vec3, 2);
      fb.TypedVector(vec3, 3);
      fb.TypedVector(vec3, 4);
      fb.FixedTypedVector(vec1, 2);
      vec1[2] = 4;
      fb.FixedTypedVector(vec1, 3);
      fb.FixedTypedVector(vec1, 4);
      fb.FixedTypedVector(vec1, 2);
      fb.FixedTypedVector(vec1, 3);
      fb.FixedTypedVector(vec1, 4);
      fb.String(U_CONSTANT_TO_PARAM("hello"));
      fb.Map([&]() {
         fb.UInt(U_CONSTANT_TO_PARAM("a"), 12);
         fb.UInt(U_CONSTANT_TO_PARAM("b"), 4);
      });
      fb.Vector([&]() {
         fb.UInt(1);
         fb.UInt(3);
      });
      vec1[0] = INT_MIN;
      vec1[1] = INT_MAX;
      fb.FixedTypedVector(vec1, 2);
      vec1[0] = 0ULL;
      vec1[1] = UINT_MAX;
      fb.FixedTypedVector(vec1, 2);
      fb.IndirectUInt(23);
      fb.IndirectUInt(UINT_MAX);
      fb.IndirectUInt(INT_MAX);
      fb.IndirectUInt(UINT_MAX);
      fb.IndirectFloat(3.5);
      });

   U_INTERNAL_ASSERT_EQUALS(size, 607)

   fb.setRoot();
   fb.AsVector(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 36)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<int64_t>(1), INT_MIN)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(2), 2)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(3), UINT_MAX)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(4), 3.0)
   U_ASSERT_EQUALS(vec.AsVectorGet<bool>(5), true)
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(6), "Hello")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(7), "Hello!")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(9), "a")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(10), "b")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(11), "a!")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(12), "b!")

   vec.AsVectorGetFixedTypedVector(13, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 3)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(2), 3)

   vec.AsVectorGetFixedTypedVector(14, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 3)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(2), 3)

   vec.AsVectorGetTypedVector(15, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<bool>(1), false)

   vec.AsVectorGetTypedVector(16, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 3)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<double>(2), 0.5)

   vec.AsVectorGetTypedVector(17, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<double>(1), 2.3)

   vec.AsVectorGetTypedVector(18, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 3)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<double>(2), 4.5)

   vec.AsVectorGetTypedVector(19, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 4)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<double>(3), 5.6)

   vec.AsVectorGetFixedTypedVector(20, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)

   vec.AsVectorGetFixedTypedVector(21, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 3)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(2), 4)

   vec.AsVectorGetFixedTypedVector(22, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 4)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(2), 4)

   vec.AsVectorGetFixedTypedVector(23, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), 2)

   vec.AsVectorGetFixedTypedVector(24, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 3)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(2), 4)

   vec.AsVectorGetFixedTypedVector(25, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 4)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(2), 4)

   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(26), "hello")

   vec.AsVectorGetMap(27, map0);

   U_ASSERT_EQUALS(map0.GetSize(), 2)

   map0.AsMapGetKeys(vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(0), "a")
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<UString>(1), "b")

   map0.AsMapGetValues(vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(0), 12)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(1), 4)

   vec.AsVectorGetVector(28, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec0.AsVectorGet<uint64_t>(1), 3)

   vec.AsVectorGetFixedTypedVector(29, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(1), INT_MAX)

   vec.AsVectorGetFixedTypedVector(30, vec0);

   U_ASSERT_EQUALS(vec0.GetSize(), 2)
   U_ASSERT_EQUALS(vec0.AsTypedOrFixedVectorGet<uint64_t>(0), 0)

   U_ASSERT_EQUALS(vec.AsVectorGetIndirect<uint64_t>(31), 23)
   U_ASSERT_EQUALS(vec.AsVectorGetIndirect<uint64_t>(32), UINT_MAX)
   U_ASSERT_EQUALS(vec.AsVectorGetIndirect<uint64_t>(33), INT_MAX)
   U_ASSERT_EQUALS(vec.AsVectorGetIndirect<uint64_t>(34), UINT_MAX)
   U_ASSERT_EQUALS(vec.AsVectorGetIndirect<double>(35), 3.5)

   size = fb.encodeMap([&]() {
      fb.UInt(U_CONSTANT_TO_PARAM("a"), 1);
      fb.UInt(U_CONSTANT_TO_PARAM("b"), 2);
      fb.UInt(U_CONSTANT_TO_PARAM("c"), (uint64_t)UINT_MAX);
      fb.Int(U_CONSTANT_TO_PARAM("d"), (uint64_t)INT_MIN);
      fb.UInt(U_CONSTANT_TO_PARAM("e"), 1);
      fb.UInt(U_CONSTANT_TO_PARAM("f"), 2);
      fb.UInt(U_CONSTANT_TO_PARAM("g"), (uint64_t)UINT_MAX);
      fb.UInt(U_CONSTANT_TO_PARAM("h"), 0);
      fb.Add(U_CONSTANT_TO_PARAM("i"), 1.1);
      fb.Add(U_CONSTANT_TO_PARAM("j"), 2.2);
      fb.Add(U_CONSTANT_TO_PARAM("k"), U_CONSTANT_TO_PARAM("Hello"));
      fb.Add(U_CONSTANT_TO_PARAM("l"), U_CONSTANT_TO_PARAM("Hello2"));
      fb.Add(U_CONSTANT_TO_PARAM("m"), U_CONSTANT_TO_PARAM("Hello3"));
      fb.Add(U_CONSTANT_TO_PARAM("n"), U_CONSTANT_TO_PARAM("Hello4"));
      fb.Add(U_CONSTANT_TO_PARAM("o"), true);
      fb.Add(U_CONSTANT_TO_PARAM("p"), false);
      fb.Add(U_CONSTANT_TO_PARAM("q"), U_CONSTANT_TO_PARAM("Bla"));
      fb.Add(U_CONSTANT_TO_PARAM("r"), U_CONSTANT_TO_PARAM("BlaBla"));
      fb.IndirectInt(U_CONSTANT_TO_PARAM("u"), (uint64_t)INT_MAX);
      fb.IndirectInt(U_CONSTANT_TO_PARAM("v"), (uint64_t)INT_MIN);
      fb.IndirectUInt(U_CONSTANT_TO_PARAM("w"), 1);
      fb.IndirectUInt(U_CONSTANT_TO_PARAM("x"), 2);
      fb.IndirectUInt(U_CONSTANT_TO_PARAM("y"), (uint64_t)UINT_MAX);
      fb.IndirectUInt(U_CONSTANT_TO_PARAM("z"), 0);
      fb.IndirectFloat(U_CONSTANT_TO_PARAM("aa"), 1.1f);
      fb.IndirectFloat(U_CONSTANT_TO_PARAM("ab"), 2.2f);
      fb.IndirectUInt(U_CONSTANT_TO_PARAM("ac"), 1);
      fb.IndirectUInt(U_CONSTANT_TO_PARAM("ad"), 2);
      fb.FixedTypedVector(U_CONSTANT_TO_PARAM("ae"), vec1, 3);
      fb.TypedVector(U_CONSTANT_TO_PARAM("af"), vec2, 2);
      fb.FixedTypedVector(U_CONSTANT_TO_PARAM("ag"), vec3, 3);
      fb.Add(U_CONSTANT_TO_PARAM("bk"), U_CONSTANT_TO_PARAM("ef"));
      fb.Vector(U_CONSTANT_TO_PARAM("bm"), [&]() {
          fb.UInt(1);
          fb.UInt(3);
          fb.UInt(4);
      });
      fb.Vector(U_CONSTANT_TO_PARAM("bn"), [&]() {
          fb.UInt(1);
          fb.UInt(3);
          fb.UInt(5);
      });
      fb.Key(U_CONSTANT_TO_PARAM("map1"));
      fb.Map(U_CONSTANT_TO_PARAM("bo"), [&]() {
          fb.UInt(12);
      });
      fb.Key(U_CONSTANT_TO_PARAM("map2"));
      fb.Map(U_CONSTANT_TO_PARAM("bp"), [&]() {
          fb.Add(true);
      });
   });

   U_INTERNAL_ASSERT_EQUALS(size, 632)

   fb.setRoot();
   fb.AsMap(map);

   U_ASSERT_EQUALS(map.GetSize(), 36)

   map.AsMapGetKeys(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 36)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "a")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(1), "b")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(32), "bm")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(33), "bn")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(34), "map1")
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(35), "map2")

   map.AsMapGetValues(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 36)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(0), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(1), 2)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(2), UINT_MAX)
   U_ASSERT_EQUALS(vec.AsVectorGet<int64_t>(3), INT_MIN)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(4), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(5), 2)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(6), UINT_MAX)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(7), 0)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(8), 1.1)
   U_ASSERT_EQUALS(vec.AsVectorGet<double>(9), 2.2)
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(10), "Hello")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(11), "Hello2")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(12), "Hello3")
   U_ASSERT_EQUALS(vec.AsVectorGet<UString>(13), "Hello4")

   map.AsMapGetVector(U_CONSTANT_TO_PARAM("bm"), vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint8_t>(0), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint8_t>(1), 3)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint8_t>(2), 4)

   map.AsMapGetVector(U_CONSTANT_TO_PARAM("bn"), vec);

   U_ASSERT_EQUALS(vec.GetSize(), 3)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint8_t>(0), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint8_t>(1), 3)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint8_t>(2), 5)

   map.AsMapGetMap(U_CONSTANT_TO_PARAM("map1"), map0);

   U_ASSERT_EQUALS(map0.GetSize(), 1)

   map0.AsMapGetKeys(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 1)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "bo")

   map0.AsMapGetValues(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<uint64_t>(0), 12)

   map.AsMapGetMap(U_CONSTANT_TO_PARAM("map2"), map0);

   U_ASSERT_EQUALS(map0.GetSize(), 1)

   map0.AsMapGetKeys(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 1)
   U_ASSERT_EQUALS(vec.AsTypedOrFixedVectorGet<UString>(0), "bp")

   map0.AsMapGetValues(vec);

   U_ASSERT_EQUALS(vec.GetSize(), 1)
   U_ASSERT_EQUALS(vec.AsVectorGet<bool>(0), true)

   Multiple().testFlatBuffer();
}
