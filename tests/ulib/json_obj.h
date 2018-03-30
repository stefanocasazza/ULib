// json_obj.h

#include <ulib/json/value.h>

class Request {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UHashMap<UString> table;
   UString radius, location;
   UVector<UString> fbPermissions;

   Request()
      {
      U_TRACE_REGISTER_OBJECT(5, Request, "")
      }

   ~Request()
      {
      U_TRACE_UNREGISTER_OBJECT(5, Request)
      }

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Request::clear()")


              table.clear();
             radius.clear();
           location.clear();
      fbPermissions.clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "Request::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(table,         UHashMap<UString>));
      json.toJSON(U_JSON_METHOD_HANDLER(radius,        UString));
      json.toJSON(U_JSON_METHOD_HANDLER(location,      UString));
      json.toJSON(U_JSON_METHOD_HANDLER(fbPermissions, UVector<UString>));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Request::toJSON()")

      U_JSON_TYPE_HANDLER(table,         UHashMap<UString>);
      U_JSON_TYPE_HANDLER(radius,        UString);
      U_JSON_TYPE_HANDLER(location,      UString);
      U_JSON_TYPE_HANDLER(fbPermissions, UVector<UString>);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Request::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(table,         UHashMap<UString>));
      json.fromJSON(U_JSON_METHOD_HANDLER(radius,        UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(location,      UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(fbPermissions, UVector<UString>));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Request::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(table,         UHashMap<UString>));
		fb.toFlatBuffer(FLATBUFFER(radius,        UString));
		fb.toFlatBuffer(FLATBUFFER(location,      UString));
		fb.toFlatBuffer(FLATBUFFER(fbPermissions, UVector<UString>));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Request::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(table,         UHashMap<UString>));
		fb.fromFlatBuffer(1, FLATBUFFER(radius,        UString));
		fb.fromFlatBuffer(2, FLATBUFFER(location,      UString));
		fb.fromFlatBuffer(3, FLATBUFFER(fbPermissions, UVector<UString>));
		}

#define REQUEST_JSON \
"{\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"},\"radius\":\"near\",\"location\":\"40.7831 N,73.9712 W\",\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"]}"

#define REQUEST_FLATBUFFER \
"[{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"},\"near\",\"40.7831 N,73.9712 W\",[\"public_profile\",\"user_friends\",\"email\"]]"

   void checkObject()
		{
		U_TRACE_NO_PARAM(5, "Request::checkObject()")

		U_ASSERT_EQUALS(radius,   "near")
		U_ASSERT_EQUALS(location, "40.7831 N,73.9712 W")

		const char* dump = UObject2String<UHashMap<UString> >(table);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		U_INTERNAL_ASSERT_EQUALS(UObjectIO::buffer_output_len, U_CONSTANT_SIZE("[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]"))

		dump = UObject2String<UVector<UString> >(fbPermissions);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		bool ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

		U_INTERNAL_ASSERT(ok)
		}

   void setObject(const UString& json)
		{
		U_TRACE(5, "Request::setObject(%V)", json.rep)

		bool ok = JSON_parse(json, *this);

		U_INTERNAL_ASSERT(ok)
		}

   void test(UValue& json_obj, const UString& json, UString& output)
		{
		U_TRACE(5, "Request::test(%p,%V,%p)", &json_obj, json.rep, &output)

		setObject(json);

		checkObject();

		JSON_stringify(output, json_obj, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), json.size() )
		}

   void testJSON()
		{
		U_TRACE_NO_PARAM(5, "Request::testJSON()")

		UValue json_obj;
		UString output, reqJson = U_STRING_FROM_CONSTANT(REQUEST_JSON);

		test(json_obj, reqJson, output);

		output.clear();

		JSON_OBJ_stringify(output, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), reqJson.size() )
		}

   void testFlatBuffer()
		{
		U_TRACE_NO_PARAM(5, "Request::testFlatBuffer()")

		UFlatBuffer fb;
		UValue json_obj;

		setObject(U_STRING_FROM_CONSTANT(REQUEST_JSON));

		fb.fromObject(*this);

		json_obj.fromFlatBuffer(fb);

		U_ASSERT_EQUALS( json_obj.output(), REQUEST_FLATBUFFER )

		clear();

		fb.toObject(*this);

		checkObject();
		}

#ifdef DEBUG
   const char* dump(bool breset) const
      {
      *UObjectIO::os << "table         (UHashMap " << (void*)&table         << ")\n"
                     << "radius        (UString "  << (void*)&radius        << ")\n"
                     << "location      (UString "  << (void*)&location      << ")\n"
                     << "fbPermissions (UVector "  << (void*)&fbPermissions << ')';

      if (breset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return U_NULLPTR;
      }
#endif

private:
   Request& operator=(const Request&) { return *this; }
};

class Response {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UVector<UString> fbPermissions;
   UString type, token;
   UHashMap<UString> table;
   
   Response(): type(U_STRING_FROM_CONSTANT("startup"))
      {
      U_TRACE_REGISTER_OBJECT(5, Response, "")
      }

   ~Response()
      {
      U_TRACE_UNREGISTER_OBJECT(5, Response)
      }

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Response::clear()")

      fbPermissions.clear();
      type.clear();
      token.clear();
      table.clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "Response::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(fbPermissions, UVector<UString>));
      json.toJSON(U_JSON_METHOD_HANDLER(type,          UString));
      json.toJSON(U_JSON_METHOD_HANDLER(token,         UString));
      json.toJSON(U_JSON_METHOD_HANDLER(table,         UHashMap<UString>));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Response::toJSON()")

      U_JSON_TYPE_HANDLER(fbPermissions, UVector<UString>);
      U_JSON_TYPE_HANDLER(type,          UString);
      U_JSON_TYPE_HANDLER(token,         UString);
      U_JSON_TYPE_HANDLER(table,         UHashMap<UString>);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Response::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(fbPermissions, UVector<UString>));
      json.fromJSON(U_JSON_METHOD_HANDLER(type,          UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(token,         UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(table,         UHashMap<UString>));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Response::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(fbPermissions, UVector<UString>));
		fb.toFlatBuffer(FLATBUFFER(type,				UString));
		fb.toFlatBuffer(FLATBUFFER(token,			UString));
		fb.toFlatBuffer(FLATBUFFER(table,         UHashMap<UString>));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Response::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(fbPermissions, UVector<UString>));
		fb.fromFlatBuffer(1, FLATBUFFER(type,          UString));
		fb.fromFlatBuffer(2, FLATBUFFER(token,			  UString));
		fb.fromFlatBuffer(3, FLATBUFFER(table,         UHashMap<UString>));
		}

#define RESPONSE_JSON \
"{\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"],\"type\":\"startup\",\"token\":\"\",\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"}}"

#define RESPONSE_FLATBUFFER \
"[[\"public_profile\",\"user_friends\",\"email\"],\"startup\",\"\",{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"}]"

   void checkObject()
		{
		U_TRACE_NO_PARAM(5, "Response::checkObject()")

		U_ASSERT_EQUALS(token, "")
		U_ASSERT_EQUALS(type,  "startup")

		const char* dump = UObject2String<UVector<UString> >(fbPermissions);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		bool ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

		U_INTERNAL_ASSERT(ok)

		dump = UObject2String<UHashMap<UString> >(table);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		U_INTERNAL_ASSERT_EQUALS(UObjectIO::buffer_output_len, U_CONSTANT_SIZE("[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]"))

		U_INTERNAL_ASSERT(ok)
		}

   void setObject(const UString& json)
		{
		U_TRACE(5, "Response::setObject(%V)", json.rep)

		bool ok = JSON_parse(json, *this);

		U_INTERNAL_ASSERT(ok)
		}

   void test(UValue& json_obj, const UString& json, UString& output)
		{
		U_TRACE(5, "Response::test(%p,%V,%p)", &json_obj, json.rep, &output)

		setObject(json);

		checkObject();

		JSON_stringify(output, json_obj, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), json.size() )
		}

   void testJSON()
		{
		U_TRACE_NO_PARAM(5, "Response::testJSON()")

		UValue json_obj;
		UString output, reqJson = U_STRING_FROM_CONSTANT(RESPONSE_JSON);

		test(json_obj, reqJson, output);

		output.clear();

		JSON_OBJ_stringify(output, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), reqJson.size() )
		}

   void testFlatBuffer()
		{
		U_TRACE_NO_PARAM(5, "Response::testFlatBuffer()")

		UFlatBuffer fb;
		UValue json_obj;

		setObject(U_STRING_FROM_CONSTANT(RESPONSE_JSON));

		fb.fromObject(*this);

		json_obj.fromFlatBuffer(fb);

		U_ASSERT_EQUALS( json_obj.output(), RESPONSE_FLATBUFFER )

		clear();

		fb.toObject(*this);

		checkObject();
		}

#ifdef DEBUG
   const char* dump(bool breset) const
      {
      *UObjectIO::os << "fbPermissions (UVector " << (void*)&fbPermissions << ")\n"
                     << "type          (UString " << (void*)&type          << ")\n"
                     << "token         (UString " << (void*)&token         << ")\n"
                     << "table         (UHashMap " << (void*)&table        << ')';

      if (breset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return U_NULLPTR;
      }
#endif
};

class Organization {
public:
   UString name, index;

   Organization() {}
   Organization(const UString& _name, const UString& _index) : name(_name), index(_index) {}

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Organization::clear()")

       name.clear();
      index.clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "Organization::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(name,  UString));
      json.toJSON(U_JSON_METHOD_HANDLER(index, UString));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Organization::toJSON()")

      U_JSON_TYPE_HANDLER(name,  UString);
      U_JSON_TYPE_HANDLER(index, UString);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Organization::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(name,  UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(index, UString));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Organization::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(name,	 UString));
		fb.toFlatBuffer(FLATBUFFER(index, UString));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Organization::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(name,  UString));
		fb.fromFlatBuffer(1, FLATBUFFER(index,	UString));
		}
};

class Social {
public:
   UString name, token;
   unsigned key;
   int64_t dateTime;

   Social() {}

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Social::clear()")

       name.clear();
      token.clear();

      key = 0;
      dateTime = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "Social::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(token, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(dateTime, int64_t));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Social::toJSON()")

      U_JSON_TYPE_HANDLER(name, UString);
      U_JSON_TYPE_HANDLER(token, UString);
      U_JSON_TYPE_HANDLER(key, unsigned);
      U_JSON_TYPE_HANDLER(dateTime, int64_t);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Social::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(token, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(dateTime, int64_t));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Social::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(name,	 UString));
		fb.toFlatBuffer(FLATBUFFER(token, UString));
      fb.toFlatBuffer(FLATBUFFER(key, unsigned));
      fb.toFlatBuffer(FLATBUFFER(dateTime, int64_t));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Social::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(name,  UString));
		fb.fromFlatBuffer(1, FLATBUFFER(token,	UString));
		fb.fromFlatBuffer(2, FLATBUFFER(key,  unsigned));
		fb.fromFlatBuffer(3, FLATBUFFER(dateTime,	int64_t));
		}
};

class StrangerSocial {
public:
   UString name;
   unsigned key;

   StrangerSocial() {}

   void clear()
      {
      U_TRACE_NO_PARAM(5, "StrangerSocial::clear()")

      name.clear();
      key = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "StrangerSocial::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "StrangerSocial::toJSON()")

      U_JSON_TYPE_HANDLER(name, UString);
      U_JSON_TYPE_HANDLER(key, unsigned);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "StrangerSocial::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "StrangerSocial::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(name,	 UString));
      fb.toFlatBuffer(FLATBUFFER(key, unsigned));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "StrangerSocial::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(name,  UString));
		fb.fromFlatBuffer(1, FLATBUFFER(key,  unsigned));
		}
};

class Link {
public:
   UString url, imageURL;
   
   Link() {}

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Link::clear()")

			  url.clear();
      imageURL.clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "Link::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(url, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(imageURL, UString));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Link::toJSON()")

      U_JSON_TYPE_HANDLER(url, UString);
      U_JSON_TYPE_HANDLER(imageURL, UString);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Link::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(url, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(imageURL, UString));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Link::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(url,	 UString));
      fb.toFlatBuffer(FLATBUFFER(imageURL, UString));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Link::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(url,  UString));
		fb.fromFlatBuffer(1, FLATBUFFER(imageURL,  UString));
		}
};

class LinkPreview {
public:

   UString imageURL, URL, title, domain;
   float imageWidth, imageHeight;

   LinkPreview() {}

   void toJSON(UString& json)
      {
      U_TRACE(5, "LinkPreview::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(imageURL, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(URL, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(title, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(domain, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(imageWidth, float));
      json.toJSON(U_JSON_METHOD_HANDLER(imageHeight, float));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "LinkPreview::toJSON()")

      U_JSON_TYPE_HANDLER(imageURL, UString);
      U_JSON_TYPE_HANDLER(URL, UString);
      U_JSON_TYPE_HANDLER(title, UString);
      U_JSON_TYPE_HANDLER(domain, UString);
      U_JSON_TYPE_HANDLER(imageWidth, float);
      U_JSON_TYPE_HANDLER(imageHeight, float);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "LinkPreview::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(imageURL, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(URL, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(title, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(domain, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(imageWidth, float));
      json.fromJSON(U_JSON_METHOD_HANDLER(imageHeight, float));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "LinkPreview::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(imageURL, UString));
		fb.toFlatBuffer(FLATBUFFER(URL,		 UString));
		fb.toFlatBuffer(FLATBUFFER(title,	 UString));
		fb.toFlatBuffer(FLATBUFFER(domain,   UString));
		fb.toFlatBuffer(FLATBUFFER(imageWidth,	 float));
		fb.toFlatBuffer(FLATBUFFER(imageHeight, float));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "LinkPreview::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(imageURL, UString));
		fb.fromFlatBuffer(1, FLATBUFFER(URL,      UString));
		fb.fromFlatBuffer(2, FLATBUFFER(title,		UString));
		fb.fromFlatBuffer(3, FLATBUFFER(domain,   UString));
		fb.fromFlatBuffer(4, FLATBUFFER(imageWidth,	float));
		fb.fromFlatBuffer(5, FLATBUFFER(imageHeight, float));
		}
};

class Message {
public:
   UString actor, target, content;
   UVector<LinkPreview*> linkPreviews;
   unsigned key;
   UString ping;
   bool becameActive;
   int64_t dateTime, readTime;

   Message() {}

   void toJSON(UString& json)
      {
      U_TRACE(5, "Message::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(actor, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(target, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(content, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(linkPreviews, UVector<LinkPreview*>));
      json.toJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(ping, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(becameActive, bool));
      json.toJSON(U_JSON_METHOD_HANDLER(dateTime, int64_t));
      json.toJSON(U_JSON_METHOD_HANDLER(readTime, int64_t));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Message::toJSON()")

      U_JSON_TYPE_HANDLER(actor, UString);
      U_JSON_TYPE_HANDLER(target, UString);
      U_JSON_TYPE_HANDLER(content, UString);
      U_JSON_TYPE_HANDLER(linkPreviews, UVector<LinkPreview*>);
      U_JSON_TYPE_HANDLER(key, unsigned);
      U_JSON_TYPE_HANDLER(ping, UString);
      U_JSON_TYPE_HANDLER(becameActive, bool);
      U_JSON_TYPE_HANDLER(dateTime, int64_t);
      U_JSON_TYPE_HANDLER(readTime, int64_t);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Message::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(actor, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(target, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(content, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(linkPreviews, UVector<LinkPreview*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(ping, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(becameActive, bool));
      json.fromJSON(U_JSON_METHOD_HANDLER(dateTime, int64_t));
      json.fromJSON(U_JSON_METHOD_HANDLER(readTime, int64_t));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Message::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(actor, UString));
		fb.toFlatBuffer(FLATBUFFER(target,		 UString));
		fb.toFlatBuffer(FLATBUFFER(content,	 UString));
		fb.toFlatBuffer(FLATBUFFER(linkPreviews, UVector<LinkPreview*>));
		fb.toFlatBuffer(FLATBUFFER(key, unsigned));
		fb.toFlatBuffer(FLATBUFFER(ping,   UString));
      fb.toFlatBuffer(FLATBUFFER(becameActive, bool));
      fb.toFlatBuffer(FLATBUFFER(dateTime, int64_t));
      fb.toFlatBuffer(FLATBUFFER(readTime, int64_t));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Message::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(actor, UString));
		fb.fromFlatBuffer(1, FLATBUFFER(target,      UString));
		fb.fromFlatBuffer(2, FLATBUFFER(content,		UString));
		fb.fromFlatBuffer(3, FLATBUFFER(ping,   UString));
		fb.fromFlatBuffer(4, FLATBUFFER(key, unsigned));
		fb.fromFlatBuffer(5, FLATBUFFER(linkPreviews, UVector<LinkPreview*>));
      fb.fromFlatBuffer(6, FLATBUFFER(becameActive, bool));
      fb.fromFlatBuffer(7, FLATBUFFER(dateTime, int64_t));
      fb.fromFlatBuffer(8, FLATBUFFER(readTime, int64_t));
		}
};

class User {
public:
   UString token, name, pic, applePushToken, directory;
   UVector<StrangerSocial*> socials;
   UVector<Link*> links;
   unsigned points, spotCount;
   bool visibility;
   int64_t aroundSince;
   Organization work, college;

   User() {}

   void toJSON(UString& json)
      {
      U_TRACE(5, "User::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(token, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(pic, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(applePushToken, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(directory, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(socials, UVector<StrangerSocial*>));
      json.toJSON(U_JSON_METHOD_HANDLER(links, UVector<Link*>));
      json.toJSON(U_JSON_METHOD_HANDLER(points, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(spotCount, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(visibility, bool));
      json.toJSON(U_JSON_METHOD_HANDLER(aroundSince, int64_t));
      json.toJSON(U_JSON_METHOD_HANDLER(work, Organization));
      json.toJSON(U_JSON_METHOD_HANDLER(college, Organization));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "User::toJSON()")

      U_JSON_TYPE_HANDLER(token, UString);
      U_JSON_TYPE_HANDLER(name, UString);
      U_JSON_TYPE_HANDLER(pic, UString);
      U_JSON_TYPE_HANDLER(applePushToken, UString);
      U_JSON_TYPE_HANDLER(directory, UString);
      U_JSON_TYPE_HANDLER(socials, UVector<StrangerSocial*>);
      U_JSON_TYPE_HANDLER(links, UVector<Link*>);
      U_JSON_TYPE_HANDLER(points, unsigned);
      U_JSON_TYPE_HANDLER(spotCount, unsigned);
      U_JSON_TYPE_HANDLER(visibility, bool);
      U_JSON_TYPE_HANDLER(aroundSince, int64_t);
      U_JSON_TYPE_HANDLER(work, Organization);
      U_JSON_TYPE_HANDLER(college, Organization);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "User::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(token, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(pic, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(applePushToken, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(directory, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(socials, UVector<StrangerSocial*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(links, UVector<Link*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(points, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(spotCount, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(visibility, bool));
      json.fromJSON(U_JSON_METHOD_HANDLER(aroundSince, int64_t));
      json.fromJSON(U_JSON_METHOD_HANDLER(work, Organization));
      json.fromJSON(U_JSON_METHOD_HANDLER(college, Organization));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "User::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(token, UString));
		fb.toFlatBuffer(FLATBUFFER(name,	 UString));
		fb.toFlatBuffer(FLATBUFFER(pic,	 UString));
		fb.toFlatBuffer(FLATBUFFER(applePushToken, UString));
		fb.toFlatBuffer(FLATBUFFER(directory, UString));
		fb.toFlatBuffer(FLATBUFFER(socials, UVector<StrangerSocial*>));
		fb.toFlatBuffer(FLATBUFFER(links, UVector<Link*>));
		fb.toFlatBuffer(FLATBUFFER(points, unsigned));
		fb.toFlatBuffer(FLATBUFFER(spotCount, unsigned));
      fb.toFlatBuffer(FLATBUFFER(visibility, bool));
      fb.toFlatBuffer(FLATBUFFER(aroundSince, int64_t));
      fb.toFlatBuffer(FLATBUFFER(work, Organization));
      fb.toFlatBuffer(FLATBUFFER(college, Organization));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "User::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(token, UString));
		fb.fromFlatBuffer(1, FLATBUFFER(name,      UString));
		fb.fromFlatBuffer(2, FLATBUFFER(pic,		UString));
		fb.fromFlatBuffer(3, FLATBUFFER(applePushToken,		UString));
		fb.fromFlatBuffer(4, FLATBUFFER(directory,		UString));
		fb.fromFlatBuffer(5, FLATBUFFER(socials, UVector<StrangerSocial*>));
		fb.fromFlatBuffer(6, FLATBUFFER(links, UVector<Link*>));
		fb.fromFlatBuffer(7, FLATBUFFER(points, unsigned));
		fb.fromFlatBuffer(8, FLATBUFFER(spotCount, unsigned));
      fb.fromFlatBuffer(9, FLATBUFFER(visibility, bool));
      fb.fromFlatBuffer(10, FLATBUFFER(aroundSince, int64_t));
      fb.fromFlatBuffer(11, FLATBUFFER(work, Organization));
      fb.fromFlatBuffer(12, FLATBUFFER(college, Organization));
		}
};

class Event {
public:
   UString actor, target;
   unsigned key;
   int64_t dateTime;

   Event() {}

   Event(const UString& _actor, unsigned _key, int64_t _dateTime) : actor(_actor), key(_key), dateTime(_dateTime) {}

   void toJSON(UString& json)
      {
      U_TRACE(5, "Event::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(actor, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(target, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(dateTime, int64_t));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Event::toJSON()")

      U_JSON_TYPE_HANDLER(actor, UString);
      U_JSON_TYPE_HANDLER(target, UString);
      U_JSON_TYPE_HANDLER(key, unsigned);
      U_JSON_TYPE_HANDLER(dateTime, int64_t);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Event::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(actor, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(target, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(dateTime, int64_t));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Event::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(actor, UString));
		fb.toFlatBuffer(FLATBUFFER(target, UString));
		fb.toFlatBuffer(FLATBUFFER(key, unsigned));
      fb.toFlatBuffer(FLATBUFFER(dateTime, int64_t));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Event::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(actor, UString));
		fb.fromFlatBuffer(4, FLATBUFFER(target,		UString));
		fb.fromFlatBuffer(8, FLATBUFFER(key, unsigned));
      fb.fromFlatBuffer(10, FLATBUFFER(dateTime, int64_t));
		}
};

class ResponseLogin {
public:
   unsigned spotCount;
   UString type, token, name, pic, directory;
   UVector<UString> actives, nows, freeFileNames;
   UVector<Link*> links;
   UVector<User*> users;
   UVector<Message*> messages;
   UVector<Event*> events;
   UVector<Social*> socials;
   Organization work, college;

   ResponseLogin(): type(U_STRING_FROM_CONSTANT("login")) {}

   void clear()
      {
      U_TRACE_NO_PARAM(5, "ResponseLogin::clear()")

      spotCount = 0;

      type.clear();
      token.clear();
      name.clear();
      pic.clear();
      directory.clear();
      actives.clear();
      nows.clear();
      freeFileNames.clear();
      links.clear();
      users.clear();
      messages.clear();
      events.clear();
      socials.clear();
      work.clear();
      college.clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "ResponseLogin::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(spotCount, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(type, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(token, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(pic, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(directory, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(actives, UVector<UString>));
      json.toJSON(U_JSON_METHOD_HANDLER(nows, UVector<UString>));
      json.toJSON(U_JSON_METHOD_HANDLER(freeFileNames, UVector<UString>));
      json.toJSON(U_JSON_METHOD_HANDLER(links, UVector<Link*>));
      json.toJSON(U_JSON_METHOD_HANDLER(users, UVector<User*>));
      json.toJSON(U_JSON_METHOD_HANDLER(messages, UVector<Message*>));
      json.toJSON(U_JSON_METHOD_HANDLER(events, UVector<Event*>));
      json.toJSON(U_JSON_METHOD_HANDLER(socials, UVector<Social*>));
      json.toJSON(U_JSON_METHOD_HANDLER(work, Organization));
      json.toJSON(U_JSON_METHOD_HANDLER(college, Organization));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "ResponseLogin::toJSON()")

      U_JSON_TYPE_HANDLER(spotCount, unsigned);
      U_JSON_TYPE_HANDLER(type, UString);
      U_JSON_TYPE_HANDLER(token, UString);
      U_JSON_TYPE_HANDLER(name, UString);
      U_JSON_TYPE_HANDLER(pic, UString);
      U_JSON_TYPE_HANDLER(directory, UString);
      U_JSON_TYPE_HANDLER(actives, UVector<UString>);
      U_JSON_TYPE_HANDLER(nows, UVector<UString>);
      U_JSON_TYPE_HANDLER(freeFileNames, UVector<UString>);
      U_JSON_TYPE_HANDLER(links, UVector<Link*>);
      U_JSON_TYPE_HANDLER(users, UVector<User*>);
      U_JSON_TYPE_HANDLER(messages, UVector<Message*>);
      U_JSON_TYPE_HANDLER(events, UVector<Event*>);
      U_JSON_TYPE_HANDLER(socials, UVector<Social*>);
      U_JSON_TYPE_HANDLER(work, Organization);
      U_JSON_TYPE_HANDLER(college, Organization);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "ResponseLogin::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(spotCount, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(type, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(token, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(name, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(pic, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(directory, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(actives, UVector<UString>));
      json.fromJSON(U_JSON_METHOD_HANDLER(nows, UVector<UString>));
      json.fromJSON(U_JSON_METHOD_HANDLER(freeFileNames, UVector<UString>));
      json.fromJSON(U_JSON_METHOD_HANDLER(links, UVector<Link*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(users, UVector<User*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(messages, UVector<Message*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(events, UVector<Event*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(socials, UVector<Social*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(work, Organization));
      json.fromJSON(U_JSON_METHOD_HANDLER(college, Organization));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "ResponseLogin::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(spotCount, unsigned));
		fb.toFlatBuffer(FLATBUFFER(type, UString));
		fb.toFlatBuffer(FLATBUFFER(token, UString));
		fb.toFlatBuffer(FLATBUFFER(name,	UString));
		fb.toFlatBuffer(FLATBUFFER(pic, UString));
		fb.toFlatBuffer(FLATBUFFER(directory, UString));
		fb.toFlatBuffer(FLATBUFFER(actives, UVector<UString>));
		fb.toFlatBuffer(FLATBUFFER(nows, UVector<UString>));
		fb.toFlatBuffer(FLATBUFFER(freeFileNames, UVector<UString>));
		fb.toFlatBuffer(FLATBUFFER(links, UVector<Link*>));
		fb.toFlatBuffer(FLATBUFFER(users, UVector<User*>));
		fb.toFlatBuffer(FLATBUFFER(messages, UVector<Message*>));
		fb.toFlatBuffer(FLATBUFFER(events, UVector<Event*>));
		fb.toFlatBuffer(FLATBUFFER(socials, UVector<Social*>));
      fb.toFlatBuffer(FLATBUFFER(work, Organization));
      fb.toFlatBuffer(FLATBUFFER(college, Organization));
		}

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "ResponseLogin::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(spotCount, unsigned));
		fb.fromFlatBuffer(1, FLATBUFFER(type, UString));
		fb.fromFlatBuffer(2, FLATBUFFER(token, UString));
		fb.fromFlatBuffer(3, FLATBUFFER(name, UString));
		fb.fromFlatBuffer(4, FLATBUFFER(pic, UString));
		fb.fromFlatBuffer(5, FLATBUFFER(directory, UString));
		fb.fromFlatBuffer(6, FLATBUFFER(actives, UVector<UString>));
		fb.fromFlatBuffer(7, FLATBUFFER(nows, UVector<UString>));
		fb.fromFlatBuffer(8, FLATBUFFER(freeFileNames, UVector<UString>));
		fb.fromFlatBuffer(9, FLATBUFFER(links, UVector<Link*>));
		fb.fromFlatBuffer(10, FLATBUFFER(users, UVector<User*>));
		fb.fromFlatBuffer(11, FLATBUFFER(messages, UVector<Message*>));
		fb.fromFlatBuffer(12, FLATBUFFER(events, UVector<Event*>));
		fb.fromFlatBuffer(13, FLATBUFFER(socials, UVector<Social*>));
      fb.fromFlatBuffer(14, FLATBUFFER(work, Organization));
      fb.fromFlatBuffer(15, FLATBUFFER(college, Organization));
		}

#define RESPONSELOGIN_JSON \
"{\"spotCount\":0,\"type\":\"login\",\"token\":\"HRq0Mgft49bF3YJaKXQCCYzZ4oRDXX5KF\",\"name\":\"victor stewart\",\"pic\":\"GRSDTbv6tqxf6P2kuVNykBsFvbZXIjsFR\",\"directory\":\"NZ45XLdN87rZJogran0y3dJl30lyw2OrQ\",\"actives\":[],\"nows\":[],\"freeFileNames\":[\"9cvxHmjzuQzzCaw2LMTvjmyMeRXM8mzXY\",\"vLivrQb9dqcRyiRa1LENgBnsSpEbFsOcN\",\"uELEMyIieNW86ruETPaISDBlnn5UOFTZr\",\"pSqtl0fITijC8BbGKvJTaSrqhgNBRDeJX\",\"sreDFtqY9a3aRU0y4PrnX4VLTJvNJUjh6\",\"1MKsg9wknND12SHLuM3NbXcO2hSxRhck8\",\"akItwhgG2JVXhUldOuSgBzrhQydIcBRRq\",\"rHc9fVz5HEN5e95834P6Ilo1ofMCjc6Vj\",\"SaMsYKXnlOWeTgm6DzcLV65R1xR0z9LSX\",\"6s982IQXEFaUbO72lDDK8wPLwrlqis9xJ\",\"5olzq3XV83bqz8ok46S2MbxvSqtjJqCYU\",\"sXTcsMH7GHVYyKNMu73UQ5Fc8weFCp4gR\",\"uHelSUm5NiHK0fP1dXMapquV3psOUd4fJ\",\"YuvfBVXQkRNQTpMLnOHKRiRM2tgQw3hv4\",\"6jQwoPPWePoHD50ZFpeuJ5OMfqYF7rog5\",\"ejh0xTPHref00GZii3YxR2Mnl1CDscS8R\",\"RDrqVKyNZkJWPhlddEOjPJYn6MHsSvyC6\",\"HxzQBW22dClawug9in1UfIEbo7IT7sb8m\",\"YRuzG4PxzMM5tlSwhZPqZ86ZBQIXlvRvQ\",\"0kxMSVcLfRjePzp14t45yTjcUlP0dlFWV\",\"RYF17ULTOh0l4NLazP9UGYewqLIWJcMk3\"],\"links\":[],\"users\":[],\"messages\":[],\"events\":[],\"socials\":[{\"name\":\"10209763296542423\",\"token\":\"EAAE2qPrDodIBAIRiZA9mP3pUpc8sQfZBtMxajCq7OL7wxHHWB8bmjs6SWj2vHJ5vC2qCL02cPkUlk2Y3WBr3kOE7WS6EL1dUwLZBnm3HZBQzTPlGogLzNZCsqAPaiZCJhLjGdVvQMNzoPYNpios6u4aFLdciq2DYV7c0806ki7ghJjOB8ApzKJof4R070wXfiePW4iZAARXe6YbW8kyrtOJFbcZBN08M4DoZD\",\"key\":3,\"dateTime\":149292654}],\"work\":{\"index\":\"\",\"name\":\"\"},\"college\":{\"index\":\"\",\"name\":\"\"}}"
#define RESPONSELOGIN_FLATBUFFER \
"[0,\"login\",\"HRq0Mgft49bF3YJaKXQCCYzZ4oRDXX5KF\",\"victor stewart\",\"GRSDTbv6tqxf6P2kuVNykBsFvbZXIjsFR\",\"NZ45XLdN87rZJogran0y3dJl30lyw2OrQ\",[],[],[\"9cvxHmjzuQzzCaw2LMTvjmyMeRXM8mzXY\",\"vLivrQb9dqcRyiRa1LENgBnsSpEbFsOcN\",\"uELEMyIieNW86ruETPaISDBlnn5UOFTZr\",\"pSqtl0fITijC8BbGKvJTaSrqhgNBRDeJX\",\"sreDFtqY9a3aRU0y4PrnX4VLTJvNJUjh6\",\"1MKsg9wknND12SHLuM3NbXcO2hSxRhck8\",\"akItwhgG2JVXhUldOuSgBzrhQydIcBRRq\",\"rHc9fVz5HEN5e95834P6Ilo1ofMCjc6Vj\",\"SaMsYKXnlOWeTgm6DzcLV65R1xR0z9LSX\",\"6s982IQXEFaUbO72lDDK8wPLwrlqis9xJ\",\"5olzq3XV83bqz8ok46S2MbxvSqtjJqCYU\",\"sXTcsMH7GHVYyKNMu73UQ5Fc8weFCp4gR\",\"uHelSUm5NiHK0fP1dXMapquV3psOUd4fJ\",\"YuvfBVXQkRNQTpMLnOHKRiRM2tgQw3hv4\",\"6jQwoPPWePoHD50ZFpeuJ5OMfqYF7rog5\",\"ejh0xTPHref00GZii3YxR2Mnl1CDscS8R\",\"RDrqVKyNZkJWPhlddEOjPJYn6MHsSvyC6\",\"HxzQBW22dClawug9in1UfIEbo7IT7sb8m\",\"YRuzG4PxzMM5tlSwhZPqZ86ZBQIXlvRvQ\",\"0kxMSVcLfRjePzp14t45yTjcUlP0dlFWV\",\"RYF17ULTOh0l4NLazP9UGYewqLIWJcMk3\"],[],[],[],[],[[\"10209763296542423\",\"EAAE2qPrDodIBAIRiZA9mP3pUpc8sQfZBtMxajCq7OL7wxHHWB8bmjs6SWj2vHJ5vC2qCL02cPkUlk2Y3WBr3kOE7WS6EL1dUwLZBnm3HZBQzTPlGogLzNZCsqAPaiZCJhLjGdVvQMNzoPYNpios6u4aFLdciq2DYV7c0806ki7ghJjOB8ApzKJof4R070wXfiePW4iZAARXe6YbW8kyrtOJFbcZBN08M4DoZD\",3,149292654]],[\"\",\"\"],[\"\",\"\"]]"

	void checkObject()
		{
		U_TRACE_NO_PARAM(5, "ResponseLogin::checkObject()")

		U_ASSERT(links.empty())
		U_ASSERT(users.empty())
		U_ASSERT(messages.empty())
		U_ASSERT(events.empty())
		U_ASSERT(work.name.empty())
		U_ASSERT_EQUALS(type, "login")
		U_ASSERT(college.name.empty())
		U_ASSERT_EQUALS(socials[0]->key, 3)
		}

   void setObject(const UString& json)
		{
		U_TRACE(5, "ResponseLogin::setObject(%V)", json.rep)

		bool ok = JSON_parse(json, *this);

		U_INTERNAL_ASSERT(ok)
		}

   void test(UValue& json_obj, const UString& json, UString& output)
		{
		U_TRACE(5, "ResponseLogin::test(%p,%V,%p)", &json_obj, json.rep, &output)

		setObject(json);

		checkObject();

		JSON_stringify(output, json_obj, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), json.size() )
		}

   void testJSON()
		{
		U_TRACE_NO_PARAM(5, "ResponseLogin::testJSON()")

		UValue json_obj;
		UString output, reqJson = U_STRING_FROM_CONSTANT(RESPONSELOGIN_JSON);

		test(json_obj, reqJson, output);

		output.clear();

		JSON_OBJ_stringify(output, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), reqJson.size() )
		}

   void testFlatBuffer()
		{
		U_TRACE_NO_PARAM(5, "ResponseLogin::testFlatBuffer()")

		UFlatBuffer fb;
		UValue json_obj;

		setObject(U_STRING_FROM_CONSTANT(RESPONSELOGIN_JSON));

		fb.fromObject(*this);

		json_obj.fromFlatBuffer(fb);

		UString output = json_obj.output();

		U_ASSERT_EQUALS( output, RESPONSELOGIN_FLATBUFFER )

		clear();

		fb.toObject(*this);

		checkObject();
		}
};

class ResponseSearch {
public:

   UString type;
   unsigned key;
   UVector<Organization*> organizations;

   ResponseSearch() : type(U_STRING_FROM_CONSTANT("search")) {}

   void clear()
      {
      U_TRACE_NO_PARAM(5, "ResponseSearch::clear()")

      key = 0;

      type.clear();
      organizations.clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "ResponseSearch::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(type, UString));
      json.toJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.toJSON(U_JSON_METHOD_HANDLER(organizations, UVector<Organization*>));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "ResponseSearch::toJSON()")

      U_JSON_TYPE_HANDLER(type, UString);
      U_JSON_TYPE_HANDLER(key, unsigned);
      U_JSON_TYPE_HANDLER(organizations, UVector<Organization*>);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "ResponseSearch::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(type, UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(key, unsigned));
      json.fromJSON(U_JSON_METHOD_HANDLER(organizations, UVector<Organization*>));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "ResponseSearch::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(type, UString));
		fb.toFlatBuffer(FLATBUFFER(key, unsigned));
		fb.toFlatBuffer(FLATBUFFER(organizations, UVector<Organization*>));
      }

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "ResponseSearch::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(type, UString));
		fb.fromFlatBuffer(1, FLATBUFFER(key, unsigned));
		fb.fromFlatBuffer(2, FLATBUFFER(organizations, UVector<Organization*>));
      }

#define RESPONSESEARCH_JSON \
"{\"type\":\"localesData\",\"key\":0,\"organizations\":[{\"name\":\"Temple University\",\"index\":\"S119\"},{\"name\":\"Tennessee State University\",\"index\":\"S266\"},{\"name\":\"Tennessee Technological University\",\"index\":\"S224\"},{\"name\":\"Texas A&M University--College Station\",\"index\":\"S75\"},{\"name\":\"Texas A&M University--Commerce\",\"index\":\"S267\"}]}"

#define RESPONSESEARCH_FLATBUFFER \
"[\"localesData\",0,[[\"Temple University\",\"S119\"],[\"Tennessee State University\",\"S266\"],[\"Tennessee Technological University\",\"S224\"],[\"Texas A&M University--College Station\",\"S75\"],[\"Texas A&M University--Commerce\",\"S267\"]]]"

	void checkObject()
		{
		U_TRACE_NO_PARAM(5, "ResponseSearch::checkObject()")

		U_INTERNAL_ASSERT_EQUALS(key, 0)
		U_ASSERT_EQUALS(type, "localesData")
		U_ASSERT_EQUALS(organizations[0]->name,  "Temple University")
		U_ASSERT_EQUALS(organizations[0]->index, "S119")
		U_ASSERT_EQUALS(organizations[4]->name,  "Texas A&M University--Commerce")
		U_ASSERT_EQUALS(organizations[4]->index, "S267")
		}

   void setObject(const UString& json)
		{
		U_TRACE(5, "ResponseSearch::setObject(%V)", json.rep)

		bool ok = JSON_parse(json, *this);

		U_INTERNAL_ASSERT(ok)
		}

   void test(UValue& json_obj, const UString& json, UString& output)
		{
		U_TRACE(5, "ResponseSearch::test(%p,%V,%p)", &json_obj, json.rep, &output)

		setObject(json);

		checkObject();

		JSON_stringify(output, json_obj, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), json.size() )
		}

   void testJSON()
		{
		U_TRACE_NO_PARAM(5, "ResponseSearch::testJSON()")

		UValue json_obj;
		UString output, reqJson = U_STRING_FROM_CONSTANT(RESPONSESEARCH_JSON);

		test(json_obj, reqJson, output);

		output.clear();

		JSON_OBJ_stringify(output, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), reqJson.size() )
		}

   void testFlatBuffer()
		{
		U_TRACE_NO_PARAM(5, "ResponseSearch::testFlatBuffer()")

		UFlatBuffer fb;
		UValue json_obj;

		setObject(U_STRING_FROM_CONSTANT(RESPONSESEARCH_JSON));

		fb.fromObject(*this);

		json_obj.fromFlatBuffer(fb);

		UString output = json_obj.output();

		U_ASSERT_EQUALS( output, RESPONSESEARCH_FLATBUFFER )

		clear();

		fb.toObject(*this);

		checkObject();
		}
};

class Multiple {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UVector<Organization*> organizations;
   UVector<Request*> vrequests;
   Request request;
   Response response;
   Response* presponse;

   Multiple()
      {
      U_NEW(Response, presponse, Response);
      }

   ~Multiple()
      {
      delete presponse;
      }

   void clear()
      {
      U_TRACE_NO_PARAM(5, "Multiple::clear()")

      organizations.clear();
      vrequests.clear();
      request.clear();
      response.clear();

      delete presponse;
      U_NEW(Response, presponse, Response);
      }

   void toJSON(UString& json)
      {
      U_TRACE(5, "Multiple::toJSON(%V)", json.rep)

      json.toJSON(U_JSON_METHOD_HANDLER(organizations, UVector<Organization*>));
      json.toJSON(U_JSON_METHOD_HANDLER(vrequests, UVector<Request*>));
      json.toJSON(U_JSON_METHOD_HANDLER(request,  Request));
      json.toJSON(U_JSON_METHOD_HANDLER(response, Response));
      json.toJSON(U_JSON_METHOD_HANDLER(presponse, Response));
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(5, "Multiple::toJSON()")

      U_JSON_TYPE_HANDLER(organizations, UVector<Organization*>);
      U_JSON_TYPE_HANDLER(vrequests, UVector<Request*>);
      U_JSON_TYPE_HANDLER(request,  Request);
      U_JSON_TYPE_HANDLER(response, Response);
      U_JSON_TYPE_HANDLER(presponse, Response);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Multiple::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(organizations, UVector<Organization*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(vrequests, UVector<Request*>));
      json.fromJSON(U_JSON_METHOD_HANDLER(request,   Request));
      json.fromJSON(U_JSON_METHOD_HANDLER(response,  Response));
      json.fromJSON(U_JSON_METHOD_HANDLER(presponse, Response));
      }

   void toFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Multiple::toFlatBuffer(%p)", &fb)

		fb.toFlatBuffer(FLATBUFFER(organizations, UVector<Organization*>));
		fb.toFlatBuffer(FLATBUFFER(vrequests, UVector<Request*>));
		fb.toFlatBuffer(FLATBUFFER(request, Request));
		fb.toFlatBuffer(FLATBUFFER(response, Response));
		fb.toFlatBuffer(FLATBUFFER(presponse, Response));
      }

   void fromFlatBuffer(UFlatBuffer& fb)
		{
		U_TRACE(5, "Multiple::fromFlatBuffer(%p)", &fb)

		fb.fromFlatBuffer(0, FLATBUFFER(organizations, UVector<Organization*>));
		fb.fromFlatBuffer(1, FLATBUFFER(vrequests, UVector<Request*>));
		fb.fromFlatBuffer(2, FLATBUFFER(request, Request));
		fb.fromFlatBuffer(3, FLATBUFFER(response, Response));
		fb.fromFlatBuffer(4, FLATBUFFER(presponse, Response));
      }

#define MULTIPLE_JSON \
"{" \
"\"organizations\":[{\"name\":\"Temple University\",\"index\":\"S119\"},{\"name\":\"Tennessee State University\",\"index\":\"S266\"},{\"name\":\"Tennessee Technological University\",\"index\":\"S224\"},{\"name\":\"Texas A&M University--College Station\",\"index\":\"S75\"},{\"name\":\"Texas A&M University--Commerce\",\"index\":\"S267\"}]," \
"\"vrequests\":[]," \
"\"request\":{\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"},\"radius\":\"near\",\"location\":\"40.7831 N, 73.9712 W\",\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"]}," \
"\"response\":{\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"],\"type\":\"startup\",\"token\":\"\",\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"}}," \
"\"presponse\":{\"fbPermissions\":[],\"type\":\"\",\"token\":\"\",\"table\":{}}" \
"}"

#define MULTIPLE_FLATBUFFER \
"[" \
"[[\"Temple University\",\"S119\"],[\"Tennessee State University\",\"S266\"],[\"Tennessee Technological University\",\"S224\"],[\"Texas A&M University--College Station\",\"S75\"],[\"Texas A&M University--Commerce\",\"S267\"]]," \
"[]," \
"[{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"},\"near\",\"40.7831 N, 73.9712 W\",[\"public_profile\",\"user_friends\",\"email\"]]," \
"[[\"public_profile\",\"user_friends\",\"email\"],\"startup\",\"\",{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"}]," \
"[[],\"\",\"\",{}]" \
"]"

	void checkObject()
		{
		U_TRACE_NO_PARAM(5, "Multiple::checkObject()")

		U_ASSERT_EQUALS(request.radius,   "near")
		U_ASSERT_EQUALS(request.location, "40.7831 N, 73.9712 W")

		const char* dump = UObject2String<UHashMap<UString> >(request.table);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		U_INTERNAL_ASSERT_EQUALS(UObjectIO::buffer_output_len, U_CONSTANT_SIZE("[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]"))

		dump = UObject2String<UVector<UString> >(request.fbPermissions);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		bool ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

		U_INTERNAL_ASSERT(ok)

		U_ASSERT_EQUALS(response.token, "")
		U_ASSERT_EQUALS(response.type,  "startup")

		dump = UObject2String<UVector<UString> >(response.fbPermissions);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

		U_INTERNAL_ASSERT(ok)

		dump = UObject2String<UHashMap<UString> >(response.table);

		U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

		U_INTERNAL_ASSERT_EQUALS(UObjectIO::buffer_output_len, U_CONSTANT_SIZE("[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]"))
		}

   void setObject(const UString& json)
		{
		U_TRACE(5, "Multiple::setObject(%V)", json.rep)

		bool ok = JSON_parse(json, *this);

		U_INTERNAL_ASSERT(ok)
		}

   void test(UValue& json_obj, const UString& json, UString& output)
		{
		U_TRACE(5, "Multiple::test(%p,%V,%p)", &json_obj, json.rep, &output)

		setObject(json);

		checkObject();

		JSON_stringify(output, json_obj, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), json.size() )
		}

   void testJSON()
		{
		U_TRACE_NO_PARAM(5, "Multiple::testJSON()")

		UValue json_obj;
		UString output, reqJson = U_STRING_FROM_CONSTANT(MULTIPLE_JSON);

		test(json_obj, reqJson, output);

		output.clear();

		JSON_OBJ_stringify(output, *this);

		U_INTERNAL_ASSERT_EQUALS( output.size(), reqJson.size() )
		}

   void testFlatBuffer()
		{
		U_TRACE_NO_PARAM(5, "Multiple::testFlatBuffer()")

		UFlatBuffer fb;
		UValue json_obj;

		setObject(U_STRING_FROM_CONSTANT(MULTIPLE_JSON));

		fb.fromObject(*this);

		json_obj.fromFlatBuffer(fb);

		UString output = json_obj.output();

		U_ASSERT_EQUALS( output, MULTIPLE_FLATBUFFER )

		clear();

		fb.toObject(*this);

		checkObject();
		}
};
