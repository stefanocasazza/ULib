// test_elasticsearch.cpp

#include <ulib/net/client/elasticsearch.h>

// Basic use elasticsearch wrapper

int U_EXPORT main(int argc, char* argv[], char* env[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UElasticSearchClient es;

   if (es.connect())
      {
      // Search document

      /**
       * {
       * "took" : 4,
       * "timed_out" : false,
       * "_shards" : {
       * "total" : 1,
       * "successful" : 1,
       * "failed" : 0
       * },
       * "hits" : {
       * "total" : 10000,
       * "max_score" : 1.0,
       * "hits" : [ {
       * "_index" : "tfb",
       * "_type" : "world",
       * "_id" : "1",
       * "_score" : 1.0,
       * "_source" : {
       * "randomNumber" : 2203
       * }
       * }, {
       * ...
       * }
       * } ]
       * }
       */

      UValue json;
      UValue* hits;
      UValue* total;
      bool ok = es.search(U_CONSTANT_TO_PARAM("tfb"), U_CONSTANT_TO_PARAM("world"), U_CONSTANT_TO_PARAM("{\"query\":{\"match_all\":{}}}")) &&
                es.parseResponse(&json);

      U_INTERNAL_ASSERT(ok)

      UValue* timed_out = json.at(U_CONSTANT_TO_PARAM("timed_out"));

      if (timed_out)
         {
         U_ASSERT(timed_out->isNumeric())

         if (timed_out->getBool()) cout << "Search timed out" << std::endl;
         else
            {
            hits = json.at(U_CONSTANT_TO_PARAM("hits"));

            U_INTERNAL_ASSERT_POINTER(hits)
            U_ASSERT(hits->isObject())

            total = hits->at(U_CONSTANT_TO_PARAM("total"));

            U_INTERNAL_ASSERT_POINTER(total)
            U_ASSERT(total->isNumeric())

            cout << "We found " << total->getNumber() << std::endl;
            }
         }

      // Index one document

      UValue jdata;
      UString data = U_STRING_FROM_CONSTANT("{\"user\": \"kimchy\", \"post_date\": \"2009-11-15T13:12:00\", \"message\": \"Trying out Elasticsearch, so far so good?\" }");

      json.clear();

      ok = jdata.parse(data)                                                                                                         &&
           es.index(U_CONSTANT_TO_PARAM("twitter"), U_CONSTANT_TO_PARAM("tweet"), U_CONSTANT_TO_PARAM("1"), U_STRING_TO_PARAM(data)) &&
           es.parseResponse(&json);

      U_INTERNAL_ASSERT(ok)

      if (json.isMemberExist(U_CONSTANT_TO_PARAM("created")) == false) cout << "Index failed" << std::endl;

      // Get document

      json.clear();

      ok = es.getDocument(U_CONSTANT_TO_PARAM("twitter"), U_CONSTANT_TO_PARAM("tweet"), U_CONSTANT_TO_PARAM("1")) &&
           es.parseResponse(&json);

      U_INTERNAL_ASSERT(ok)

      if (json.isMemberExist(U_CONSTANT_TO_PARAM("found")) == false) cout << "Failed to get document" << std::endl;

      UValue* result = json.at(U_CONSTANT_TO_PARAM("_source"));

      U_INTERNAL_ASSERT_POINTER(result)
      U_ASSERT(result->isObject())

      if (jdata != *result) cout << "Oups, something did not work" << std::endl;

      json.clear();

      ok = es.getDocument(U_CONSTANT_TO_PARAM("twitter"), U_CONSTANT_TO_PARAM("tweet"), U_CONSTANT_TO_PARAM("user"), U_CONSTANT_TO_PARAM("kimchy")) &&
           es.parseResponse(&json);

      U_INTERNAL_ASSERT(ok)

      timed_out = json.at(U_CONSTANT_TO_PARAM("timed_out"));

      if (timed_out)
         {
         U_ASSERT(timed_out->isNumeric())

         if (timed_out->getBool()) cout << "Search timed out" << std::endl;
         else
            {
            hits = json.at(U_CONSTANT_TO_PARAM("hits"));

            U_INTERNAL_ASSERT_POINTER(hits)
            U_ASSERT(hits->isObject())

            total = hits->at(U_CONSTANT_TO_PARAM("total"));

            U_INTERNAL_ASSERT_POINTER(total)
            U_ASSERT(total->isNumeric())

            cout << "We found " << total->getNumber() << std::endl;
            }
         }

      // Update document

      json.clear();

      ok = es.update(U_CONSTANT_TO_PARAM("twitter"), U_CONSTANT_TO_PARAM("tweet"), U_CONSTANT_TO_PARAM("1"), U_CONSTANT_TO_PARAM("user"), U_CONSTANT_TO_PARAM("cpp-elasticsearch")) &&
           es.parseResponse(&json);

      if (ok == false) cout << "Failed to update document" << std::endl;

      // Delete document

      json.clear();

      ok = es.deleteDocument(U_CONSTANT_TO_PARAM("twitter"), U_CONSTANT_TO_PARAM("tweet"), U_CONSTANT_TO_PARAM("1")) &&
           es.parseResponse(&json);

      if (ok == false) cout << "Failed to delete document" << std::endl;
      }
}
