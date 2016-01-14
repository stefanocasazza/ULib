// test_json.cpp

#include <ulib/file.h>
#include <ulib/json/value.h>
#include <ulib/debug/crono.h>

// Do a query and print the results

static void testQuery(const UString& json, const char* cquery, const UString& expected)
{
   U_TRACE(5, "testQuery(%V,%S,%V)", json.rep, cquery, expected.rep)

   char buffer[4096];
   UString result, query(cquery);
   int dataType = UValue::jread(json, query, result);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "dataType = (%d %S) query = %V result(%u) = %V UValue::jread_elements = %d UValue::jread_error = (%d %S)\n",
              dataType, UValue::getDataTypeDescription(dataType), query.rep, result.size(), result.rep, UValue::jread_elements, UValue::jread_error, UValue::getJReadErrorDescription()));

   U_INTERNAL_ASSERT_EQUALS(result, expected)
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
   UString filename, content, array, result,
           exampleJson = U_STRING_FROM_CONSTANT("{"
                                                "  \"astring\": \"This is a string\",\n"
                                                "  \"number1\": 42,\n"
                                                "  \"number2\":  -123.45,\n"
                                                "  \"anObject\":{\"one\":1,\"two\":{\"obj2.1\":21,\"obj2.2\":22},\"three\":333},\n"
                                                "  \"anArray\":[0, \"one\", {\"two.0\":20,\"two.1\":21}, 3, [4,44,444]],\n"
                                                "  \"isnull\":null,\n"
                                                "  \"yes\": true,\n"
                                                "  \"no\":  false\n"
                                                "}");

   while (cin >> filename)
      {
      content = UFile::contentOf(filename);

      if (json.parse(content))
         {
         result.setBuffer(U_CAPACITY);

         UValue::stringify(result, json);

         cout << result << '\n';
         }

      json.clear();
      }

   cout << '\n';

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

   // locate "anArray"...

   (void) UValue::jread(exampleJson, U_STRING_FROM_CONSTANT("{'anArray'"), array);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "\n\"anArray\": = %V\n", array.rep));

   // do queries within "anArray"...

   for (i = 0, n = UValue::jread_elements; i < n; ++i)
      {
      // index the array using queryParam

      (void) UValue::jread(array, U_STRING_FROM_CONSTANT("[*"), result, &i); 

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "anArray[%d] = %V\n", i, result.rep));
      }

   // example using a parameter array

   (void) UValue::jread(array, U_STRING_FROM_CONSTANT("[*{*"), result, params);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "\nanArray[%d] objectKey[%d] = %V\n\n", params[0], params[1], result.rep));

   // identify the whole JSON element

   array = UFile::contentOf("inp/TESTJSON.json");

   (void) UValue::jread(array, UString::getStringNull(), result);

   U_INTERNAL_ASSERT_EQUALS(UValue::jread_elements, 1000)

   // perform query on JSON file - access each array by indexing

   crono.start();

   for (i = 0, n = UValue::jread_elements; i < n; ++i)
      {
      (void) UValue::jread(array, U_STRING_FROM_CONSTANT("[*{'Users'"), result, &i);

   // cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "array[%d] \"Users\": = %V\n", i, result.rep));
      }

   crono.stop();

   cerr.write(buffer, u__snprintf(buffer, sizeof(buffer), "\n# Time Consumed with ACCESS EACH ARRAY BY INDEXING = %4ld ms\n", crono.getTimeElapsed()));

   // now using jreadArrayStep()...

   crono.start();

   UValue::jreadArrayStepInit();

   for (i = 0; i < n; ++i)
      {
      if (UValue::jreadArrayStep(array, result) != OBJECT_VALUE)
         {
         U_ERROR("Array element wasn't an object! i = %d UValue::jread_pos = %u", i, UValue::jread_pos);
         }

      (void) UValue::jread(result, U_STRING_FROM_CONSTANT("{'Users'"), result);

   // cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "array[%d] \"Users\": = %V\n", i, result.rep));
      }

   crono.stop();

   cerr.write(buffer, u__snprintf(buffer, sizeof(buffer),   "# Time Consumed with              jreadArrayStep() = %4ld ms\n", crono.getTimeElapsed()));
}
