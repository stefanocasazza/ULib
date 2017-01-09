// test_json.cpp

#include <ulib/file.h>
#include <ulib/json/value.h>
#include <ulib/debug/crono.h>

class Request {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString token, type, radius, location;

   Request()
      {
      U_TRACE_REGISTER_OBJECT(5, Request, "")
      }

   Request(const Request& r) : token(r.token), type(r.type), radius(r.radius), location(r.location)
      {
      U_TRACE_REGISTER_OBJECT(5, Request, "%p", &r)

      U_MEMORY_TEST_COPY(r)
      }

   ~Request()
      {
      U_TRACE_UNREGISTER_OBJECT(5, Request)
      }

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Request::clear()")

        token.clear();
         type.clear();
        radius.clear();
      location.clear();
      }

#ifdef DEBUG
   const char* dump(bool breset) const
      {
      *UObjectIO::os << "token    (UString " << (void*)&token    << ")\n"
                     << "type     (UString " << (void*)&type     << ")\n"
                     << "radius   (UString " << (void*)&radius   << ")\n"
                     << "location (UString " << (void*)&location << ')';

      if (breset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return 0;
      }
#endif

private:
   Request& operator=(const Request&) { return *this; }
};

// JSON TEMPLATE SPECIALIZATIONS

template <> class U_EXPORT UJsonTypeHandler<Request> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(Request& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<Request>::toJSON(%p)", &json)

      json.toJSON(U_JSON_TYPE_HANDLER(Request, token,    UString));
      json.toJSON(U_JSON_TYPE_HANDLER(Request, type,     UString));
      json.toJSON(U_JSON_TYPE_HANDLER(Request, radius,   UString));
      json.toJSON(U_JSON_TYPE_HANDLER(Request, location, UString));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<Request>::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_TYPE_HANDLER(Request, token,    UString));
      json.fromJSON(U_JSON_TYPE_HANDLER(Request, type,     UString));
      json.fromJSON(U_JSON_TYPE_HANDLER(Request, radius,   UString));
      json.fromJSON(U_JSON_TYPE_HANDLER(Request, location, UString));
      }
};

// Do a query and print the results

static void testQuery(const UString& json, const char* cquery, const UString& expected)
{
   U_TRACE(5, "testQuery(%V,%S,%V)", json.rep, cquery, expected.rep)

   char buffer[4096];
   UString result, query(cquery, strlen(cquery));
   int dataType = UValue::jread(json, query, result);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("dataType = (%d %S) query = %V result(%u) = %V UValue::jread_elements = %d UValue::jread_error = (%d %S)\n"),
              dataType, UValue::getDataTypeDescription(dataType), query.rep, result.size(), result.rep, UValue::jread_elements, UValue::jread_error,
              UValue::getJReadErrorDescription()));

   U_INTERNAL_ASSERT_EQUALS(result, expected)
}

static void testVector()
{
   U_TRACE_NO_PARAM(5, "testVector()")

   UValue json_vec;
   UVector<UString> y;
   UString result, vecJson = U_STRING_FROM_CONSTANT("[\"riga 1\",\"riga 2\",\"riga 3\",\"riga 4\"]");

   bool ok = JSON_parse(vecJson, y);

   U_INTERNAL_ASSERT(ok)

   ok = (y[0] == U_STRING_FROM_CONSTANT("riga 1"));
   U_INTERNAL_ASSERT(ok)
   ok = (y[1] == U_STRING_FROM_CONSTANT("riga 2"));
   U_INTERNAL_ASSERT(ok)
   ok = (y[2] == U_STRING_FROM_CONSTANT("riga 3"));
   U_INTERNAL_ASSERT(ok)
   ok = (y[3] == U_STRING_FROM_CONSTANT("riga 4"));
   U_INTERNAL_ASSERT(ok)

   y.clear();

   ok = JSON_parse(vecJson, y);

   U_INTERNAL_ASSERT(ok)

   ok = (y[0] == U_STRING_FROM_CONSTANT("riga 1"));
   U_INTERNAL_ASSERT(ok)
   ok = (y[1] == U_STRING_FROM_CONSTANT("riga 2"));
   U_INTERNAL_ASSERT(ok)
   ok = (y[2] == U_STRING_FROM_CONSTANT("riga 3"));
   U_INTERNAL_ASSERT(ok)
   ok = (y[3] == U_STRING_FROM_CONSTANT("riga 4"));
   U_INTERNAL_ASSERT(ok)

   JSON_stringify(result, json_vec, y);

   U_ASSERT_EQUALS( result, vecJson )
}

static void testMap()
{
   U_TRACE_NO_PARAM(5, "testMap()")

   UValue json_obj;
   UHashMap<UString> x;
   UVector<UString> members;
   UString result, mapJson = U_STRING_FROM_CONSTANT("{\"key1\":\"riga 1\",\"key2\":\"riga 2\",\"key3\":\"riga 3\",\"key4\":\"riga 4\"}");

   bool ok = JSON_parse(mapJson, x);
   U_INTERNAL_ASSERT(ok)

   ok = (x["key1"] == U_STRING_FROM_CONSTANT("riga 1"));
   U_INTERNAL_ASSERT(ok)
   ok = (x["key2"] == U_STRING_FROM_CONSTANT("riga 2"));
   U_INTERNAL_ASSERT(ok)
   ok = (x["key3"] == U_STRING_FROM_CONSTANT("riga 3"));
   U_INTERNAL_ASSERT(ok)
   ok = (x["key4"] == U_STRING_FROM_CONSTANT("riga 4"));
   U_INTERNAL_ASSERT(ok)

   x.clear();

   ok = JSON_parse(mapJson, x);

   U_INTERNAL_ASSERT(ok)

   ok = (x["key1"] == U_STRING_FROM_CONSTANT("riga 1"));
   U_INTERNAL_ASSERT(ok)
   ok = (x["key2"] == U_STRING_FROM_CONSTANT("riga 2"));
   U_INTERNAL_ASSERT(ok)
   ok = (x["key3"] == U_STRING_FROM_CONSTANT("riga 3"));
   U_INTERNAL_ASSERT(ok)
   ok = (x["key4"] == U_STRING_FROM_CONSTANT("riga 4"));
   U_INTERNAL_ASSERT(ok)

   JSON_stringify(result, json_obj, x);

   U_ASSERT_EQUALS(result.size(), mapJson.size())

   uint32_t n = json_obj.getMemberNames(members);
   U_INTERNAL_ASSERT(n == 4)

   const char* str = UObject2String(members);
   U_INTERNAL_DUMP("UObject2String(members) = %S", str)

   ok = json_obj.isMemberExist(U_CONSTANT_TO_PARAM("key4"));
   U_INTERNAL_ASSERT(ok)
}

static void testObject()
{
   U_TRACE_NO_PARAM(5, "testObject()")

   UValue json_obj;
   Request request;
   UString result, reqJson = U_STRING_FROM_CONSTANT("{\"token\":\"A619828KAIJ6D3\",\"type\":\"localesData\",\"radius\":\"near\",\"location\":\"40.7831 N, 73.9712 W\"}");

   bool ok = JSON_parse(reqJson, request);

   U_INTERNAL_ASSERT(ok)

   U_DUMP_OBJECT(request)

   U_INTERNAL_ASSERT_EQUALS(request.token,    "A619828KAIJ6D3")
   U_INTERNAL_ASSERT_EQUALS(request.type,     "localesData")
   U_INTERNAL_ASSERT_EQUALS(request.radius,   "near")
   U_INTERNAL_ASSERT_EQUALS(request.location, "40.7831 N, 73.9712 W")

   ok = JSON_parse(reqJson, request);

   U_INTERNAL_ASSERT(ok)

   U_DUMP_OBJECT(request)

   U_INTERNAL_ASSERT_EQUALS(request.token,    "A619828KAIJ6D3")
   U_INTERNAL_ASSERT_EQUALS(request.type,     "localesData")
   U_INTERNAL_ASSERT_EQUALS(request.radius,   "near")
   U_INTERNAL_ASSERT_EQUALS(request.location, "40.7831 N, 73.9712 W")

   JSON_stringify(result, json_obj, request);

   U_ASSERT_EQUALS( result, reqJson )

   request.clear();

   ok = JSON_parse((reqJson = U_STRING_FROM_CONSTANT("{\"type\":\"localesData\",\"radius\":\"near\",\"location\":\"40.7831 N, 73.9712 W\"}")), request);

   U_INTERNAL_ASSERT(ok)

   U_DUMP_OBJECT(request)

   U_INTERNAL_ASSERT_EQUALS(request.token,    "")
   U_INTERNAL_ASSERT_EQUALS(request.type,     "localesData")
   U_INTERNAL_ASSERT_EQUALS(request.radius,   "near")
   U_INTERNAL_ASSERT_EQUALS(request.location, "40.7831 N, 73.9712 W")

   result.clear();

   JSON_stringify(result, json_obj, request);

   U_ASSERT_EQUALS( result, "{\"token\":\"\",\"type\":\"localesData\",\"radius\":\"near\",\"location\":\"40.7831 N, 73.9712 W\"}" )
}

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5, "main(%d)", argc)
 
   UValue json;
   UCrono crono;
   char buffer[4096];
   uint32_t i, n, params[2] = { 2, 1 };
   UString result(U_CAPACITY), result1, filename, content, array;

   UValue::jsonParseFlags = 2;

   /*
   content = UFile::contentOf(U_STRING_FROM_CONSTANT("inp/json/pass01.json")); // canada.json citm_catalog.json inp/json/pass01.json

   if (json.parse(content)) cout << json.output() << '\n';

   json.clear();

   return -1;
   */

   testObject();

   content = UFile::contentOf(U_STRING_FROM_CONSTANT("inp/json/prova.json"));

   bool ok = json.parse(content);

   U_INTERNAL_ASSERT(ok)

   result1 = json.prettify();
   
   U_INTERNAL_ASSERT_EQUALS(content, result1)

   json.clear();

   UValue::stringify(result, UValue(U_STRING_FROM_CONSTANT("message"), U_STRING_FROM_CONSTANT("Hello, World!")));

   cout << result << '\n';

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"Hello\\nWorld\"]"));

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"Hello\\u0000World\"]"));

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"\\\"\\\\/\\b\\f\\n\\r\\t\"]")); // expect: `""\/^H^L ^M "` (length: 8)

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"\\u0024\"]")); // "\x24" // Dollar sign U+0024

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"\\u00A2\"]")); // "\xC2\xA2" Cents sign U+00A2

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"\\u20AC\"]")); // "\xE2\x82\xAC" Euro sign U+20AC

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   ok = json.parse(U_STRING_FROM_CONSTANT("[\"\\uD834\\uDD1E\"]")); // "\xF0\x9D\x84\x9E" G clef sign U+1D11E 

   U_INTERNAL_ASSERT(ok)

   cout << json.at(0)->getString() << '\n';

   json.clear();

   // locate "anArray"...

   UString exampleJson = U_STRING_FROM_CONSTANT("{"
                                                "  \"astring\": \"This is a string\",\n"
                                                "  \"number1\": 42,\n"
                                                "  \"number2\":  -123.45,\n"
                                                "  \"anObject\":{\"one\":1,\"two\":{\"obj2.1\":21,\"obj2.2\":22},\"three\":333},\n"
                                                "  \"anArray\":[0, \"one\", {\"two.0\":20,\"two.1\":21}, 3, [4,44,444]],\n"
                                                "  \"isnull\":null,\n"
                                                "  \"yes\": true,\n"
                                                "  \"no\":  false\n"
                                                "}");

   (void) UValue::jread(exampleJson, U_STRING_FROM_CONSTANT("{'anArray'"), array);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("\n\"anArray\": = %V\n"), array.rep));

   // do queries within "anArray"...

   for (i = 0, n = UValue::jread_elements; i < n; ++i)
      {
      // index the array using queryParam

      result.clear();

      (void) UValue::jread(array, U_STRING_FROM_CONSTANT("[*"), result, &i); 

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("anArray[%d] = %V\n"), i, result.rep));
      }

   // example using a parameter array

   result.clear();

   (void) UValue::jread(array, U_STRING_FROM_CONSTANT("[*{*"), result, params);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("\nanArray[%d] objectKey[%d] = %V\n\n"), params[0], params[1], result.rep));

   // identify the whole JSON element

   array = UFile::contentOf(U_STRING_FROM_CONSTANT("inp/TESTJSON.json"));

   result.clear();

   (void) UValue::jread(array, UString::getStringNull(), result);

   U_INTERNAL_ASSERT_EQUALS(UValue::jread_elements, 1000)

   // perform query on JSON file - access each array by indexing

   crono.start();

   for (i = 0, n = UValue::jread_elements; i < n; ++i)
      {
      result.clear();

      (void) UValue::jread(array, U_STRING_FROM_CONSTANT("[*{'Users'"), result, &i);

   // cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("array[%d] \"Users\": = %V\n"), i, result.rep));
      }

   crono.stop();

   cerr.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("\n# Time Consumed with ACCESS EACH ARRAY BY INDEXING = %4ld ms\n"), crono.getTimeElapsed()));

   // now using jreadArrayStep()...

   crono.start();

   UValue::jreadArrayStepInit();

   for (i = 0; i < n; ++i)
      {
      result1.clear();

      if (UValue::jreadArrayStep(array, result1) != UValue::OBJECT_VALUE)
         {
         U_ERROR("Array element wasn't an object! i = %d UValue::jread_pos = %u", i, UValue::jread_pos);
         }

      result.clear();

      (void) UValue::jread(result1, U_STRING_FROM_CONSTANT("{'Users'"), result);

   // cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("array[%d] \"Users\": = %V\n"), i, result.rep));
      }

   crono.stop();

   cerr.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("# Time Consumed with              jreadArrayStep() = %4ld ms\n"), crono.getTimeElapsed()));

   UString searchJson = U_STRING_FROM_CONSTANT("{\"took\":1,\"timed_out\":false,\"_shards\":{\"total\":1,\"successful\":1,\"failed\":0},"
                                               "\"hits\":{\"total\":1,\"max_score\":1.0,\"hits\":[{\"_index\":\"tfb\",\"_type\":\"world\",\"_id\":\"6464\",\"_score\":1.0,"
                                               "\"_source\":{ \"randomNumber\" : 9342 }}]}}");

   result.clear();

   (void) U_JFIND(searchJson, "randomNumber", result);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("randomNumber = %V\n"), result.rep));

   int city;
   double pricePoint;
   UString workingString, query(U_STRING_FROM_CONSTANT("{ \"colorShifts\" : { \"H67\" : -1 }, \"name\" : \"Mr. Taka Ramen\", \"category\" : 39, \"grouping\" : 0,"
                                                       " \"bumpUp\" : false, \"businessID\" : \"B5401\", \"foundationColor\" : 3, \"coordinates\" : [ -73.9888983, 40.7212405 ] }"));

   (void) U_JFIND(U_STRING_FROM_CONSTANT("{ \"pricePoint\" : 2.48333333333333, \"socialWeight\" : 8.75832720587083, \"gender\" : 0, \"lessThan16\" : false }"), "pricePoint", pricePoint);

   U_INTERNAL_ASSERT_EQUALS(pricePoint, 2.48333333333333)

   (void) U_JFIND(U_STRING_FROM_CONSTANT("{ \"cityKey\" : 0 }"), "cityKey", city);

   U_INTERNAL_ASSERT_EQUALS(city, 0)

   (void) UValue::jread(query, U_STRING_FROM_CONSTANT("{'coordinates' [0"), workingString);

   U_INTERNAL_ASSERT_EQUALS(workingString, "-73.9888983")

   workingString.clear();

   (void) U_JFIND(query, "coordinates", workingString);

   U_INTERNAL_ASSERT_EQUALS(workingString, "[ -73.9888983, 40.7212405 ]")

   testQuery( U_STRING_FROM_CONSTANT("{ \"_id\" : 3457, \"id\" : 3457, \"randomNumber\" : 8427 }"), "{'randomNumber'", U_STRING_FROM_CONSTANT("8427") );
   testQuery( exampleJson, "", exampleJson );
   testQuery( exampleJson, "[1", U_STRING_FROM_CONSTANT("") );
   testQuery( exampleJson, "{'astring'", U_STRING_FROM_CONSTANT("This is a string") );
   testQuery( exampleJson, "{'number1'", U_STRING_FROM_CONSTANT("42") );
   testQuery( exampleJson, "{'number2'", U_STRING_FROM_CONSTANT("-123.45") );
   testQuery( exampleJson, "{'anObject'", U_STRING_FROM_CONSTANT("{\"one\":1,\"two\":{\"obj2.1\":21,\"obj2.2\":22},\"three\":333}") );
   testQuery( exampleJson, "{'anArray'", U_STRING_FROM_CONSTANT("[0, \"one\", {\"two.0\":20,\"two.1\":21}, 3, [4,44,444]]") );
   testQuery( exampleJson, "{'isnull'", U_STRING_FROM_CONSTANT("null") );
   testQuery( exampleJson, "{'yes'", U_STRING_FROM_CONSTANT("true") );
   testQuery( exampleJson, "{'no'", U_STRING_FROM_CONSTANT("false") );
   testQuery( exampleJson, "{'missing'", U_STRING_FROM_CONSTANT("") );
   testQuery( exampleJson, "{'anObject'{'two'", U_STRING_FROM_CONSTANT("{\"obj2.1\":21,\"obj2.2\":22}") );
   testQuery( exampleJson, "{'anObject' {'two' {'obj2.2'", U_STRING_FROM_CONSTANT("22") );
   testQuery( exampleJson, "{'anObject'{'three'", U_STRING_FROM_CONSTANT("333") );
   testQuery( exampleJson, "{'anArray' [1", U_STRING_FROM_CONSTANT("one") );
   testQuery( exampleJson, "{'anArray' [2 {'two.1'", U_STRING_FROM_CONSTANT("21") );
   testQuery( exampleJson, "{'anArray' [4 [2", U_STRING_FROM_CONSTANT("444") );
   testQuery( exampleJson, "{'anArray' [999", U_STRING_FROM_CONSTANT("") );
   testQuery( exampleJson, "{3", U_STRING_FROM_CONSTANT("anObject") );
   testQuery( exampleJson, "{'anObject' {1", U_STRING_FROM_CONSTANT("two") );
   testQuery( exampleJson, "{999", U_STRING_FROM_CONSTANT("") );

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11) && defined(U_COMPILER_RANGE_FOR)
   UValue json_vec;
   std::vector<unsigned int> v = {0, 1, 2, 3, 4, 5};
   UString vecJson = U_STRING_FROM_CONSTANT("[0,1,2,3,4,5]");

   result1.clear();

   JSON_stringify(result1, json_vec, v);

   U_ASSERT_EQUALS(result1, vecJson)

   v.clear();

   ok = JSON_parse(vecJson, v);
   U_INTERNAL_ASSERT(ok)

   ok = (v[0] == 0);
   U_INTERNAL_ASSERT(ok)
   ok = (v[1] == 1);
   U_INTERNAL_ASSERT(ok)
   ok = (v[2] == 2);
   U_INTERNAL_ASSERT(ok)
   ok = (v[3] == 3);
   U_INTERNAL_ASSERT(ok)
   ok = (v[4] == 4);
   U_INTERNAL_ASSERT(ok)
   ok = (v[5] == 5);
   U_INTERNAL_ASSERT(ok)
#endif

   testMap();
   testVector();

   while (cin >> filename)
      {
      content = UFile::contentOf(filename);

      if (json.parse(content)) cout << json.output() << '\n';

      json.clear();
      }
}
