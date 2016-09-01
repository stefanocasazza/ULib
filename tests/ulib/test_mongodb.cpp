// test_mongodb.cpp

#include <ulib/container/vector.h>
#include <ulib/net/client/mongodb.h>

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UMongoDBClient x;
   const char* name_collection = argv[1];
   const char* json            = argv[2];

   if (x.connect(0,0) &&
       x.selectCollection("hello_world", name_collection))
      {
      if (U_STREQ(name_collection, strlen(name_collection), "Fortune")) (void) x.findAll();
      else
         {
         if (json) (void) x.findOne(json, strlen(json));
                   (void) x.findOne(8980);
                   (void) x.update(8980, "randomNumber", 8980);
         }
      }
}
