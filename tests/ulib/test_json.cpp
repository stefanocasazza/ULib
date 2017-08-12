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
      U_JSON_TYPE_HANDLER(type,         UString);
      U_JSON_TYPE_HANDLER(token,        UString);
      U_JSON_TYPE_HANDLER(table,        UHashMap<UString>);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(5, "Response::fromJSON(%p)", &json)

      json.fromJSON(U_JSON_METHOD_HANDLER(fbPermissions, UVector<UString>));
      json.fromJSON(U_JSON_METHOD_HANDLER(type,          UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(token,         UString));
      json.fromJSON(U_JSON_METHOD_HANDLER(table,         UHashMap<UString>));
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
};

class ResponseSearch {
public:

   UString type;
   unsigned key;
   UVector<Organization*> organizations;

   ResponseSearch() : type(U_STRING_FROM_CONSTANT("search")) {}

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

      presponse = U_NULLPTR;
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
};

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

   result.clear();

   JSON_OBJ_stringify(result, x);

   U_ASSERT_EQUALS( result.size(), mapJson.size() )
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

   result.clear();

   JSON_OBJ_stringify(result, y);

   U_ASSERT_EQUALS( result, vecJson )
}

static void testRequest()
{
   U_TRACE_NO_PARAM(5, "testRequest()")

   UValue json_obj;
   Request request;
   const char* dump;
   UString result, reqJson = U_STRING_FROM_CONSTANT("{\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"},\"radius\":\"near\",\"location\":\"40.7831 N, 73.9712 W\",\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"]}");

   bool ok = JSON_parse(reqJson, request);

   U_INTERNAL_ASSERT(ok)

   U_INTERNAL_ASSERT_EQUALS(request.radius,   "near")
   U_INTERNAL_ASSERT_EQUALS(request.location, "40.7831 N, 73.9712 W")

   dump = UObject2String<UHashMap<UString> >(request.table);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]");

   U_INTERNAL_ASSERT(ok)

   dump = UObject2String<UVector<UString> >(request.fbPermissions);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

   U_INTERNAL_ASSERT(ok)

   JSON_stringify(result, json_obj, request);

   U_ASSERT_EQUALS( result, reqJson )

   result.clear();

   JSON_OBJ_stringify(result, request);

   U_ASSERT_EQUALS( result, reqJson )
}

static void testResponse()
{
   U_TRACE_NO_PARAM(5, "testResponse()")

   UValue json_obj;
   const char* dump;
   Response response;
   UString result, reqJson = U_STRING_FROM_CONSTANT("{\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"],\"type\":\"startup\",\"token\":\"\",\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"}}");

   bool ok = JSON_parse(reqJson, response);

   U_INTERNAL_ASSERT(ok)

   U_INTERNAL_ASSERT_EQUALS(response.token, "")
   U_INTERNAL_ASSERT_EQUALS(response.type,  "startup")

   dump = UObject2String<UVector<UString> >(response.fbPermissions);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

   U_INTERNAL_ASSERT(ok)

   dump = UObject2String<UHashMap<UString> >(response.table);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]");

   U_INTERNAL_ASSERT(ok)

   JSON_stringify(result, json_obj, response);

   U_ASSERT_EQUALS( result, reqJson )

   result.clear();

   JSON_OBJ_stringify(result, response);

   U_ASSERT_EQUALS( result, reqJson )
}

static void testResponseLogin()
{
   U_TRACE_NO_PARAM(5, "testResponseLogin()")

   UValue json_obj;
   ResponseLogin response;
   UString result, reqJson = U_STRING_FROM_CONSTANT("{\"spotCount\":0,\"type\":\"login\",\"token\":\"HRq0Mgft49bF3YJaKXQCCYzZ4oRDXX5KF\",\"name\":\"victor stewart\",\"pic\":\"GRSDTbv6tqxf6P2kuVNykBsFvbZXIjsFR\",\"directory\":\"NZ45XLdN87rZJogran0y3dJl30lyw2OrQ\",\"actives\":[],\"nows\":[],\"freeFileNames\":[\"9cvxHmjzuQzzCaw2LMTvjmyMeRXM8mzXY\",\"vLivrQb9dqcRyiRa1LENgBnsSpEbFsOcN\",\"uELEMyIieNW86ruETPaISDBlnn5UOFTZr\",\"pSqtl0fITijC8BbGKvJTaSrqhgNBRDeJX\",\"sreDFtqY9a3aRU0y4PrnX4VLTJvNJUjh6\",\"1MKsg9wknND12SHLuM3NbXcO2hSxRhck8\",\"akItwhgG2JVXhUldOuSgBzrhQydIcBRRq\",\"rHc9fVz5HEN5e95834P6Ilo1ofMCjc6Vj\",\"SaMsYKXnlOWeTgm6DzcLV65R1xR0z9LSX\",\"6s982IQXEFaUbO72lDDK8wPLwrlqis9xJ\",\"5olzq3XV83bqz8ok46S2MbxvSqtjJqCYU\",\"sXTcsMH7GHVYyKNMu73UQ5Fc8weFCp4gR\",\"uHelSUm5NiHK0fP1dXMapquV3psOUd4fJ\",\"YuvfBVXQkRNQTpMLnOHKRiRM2tgQw3hv4\",\"6jQwoPPWePoHD50ZFpeuJ5OMfqYF7rog5\",\"ejh0xTPHref00GZii3YxR2Mnl1CDscS8R\",\"RDrqVKyNZkJWPhlddEOjPJYn6MHsSvyC6\",\"HxzQBW22dClawug9in1UfIEbo7IT7sb8m\",\"YRuzG4PxzMM5tlSwhZPqZ86ZBQIXlvRvQ\",\"0kxMSVcLfRjePzp14t45yTjcUlP0dlFWV\",\"RYF17ULTOh0l4NLazP9UGYewqLIWJcMk3\"],\"links\":[],\"users\":[],\"messages\":[],\"events\":[],\"socials\":[{\"name\":\"10209763296542423\",\"token\":\"EAAE2qPrDodIBAIRiZA9mP3pUpc8sQfZBtMxajCq7OL7wxHHWB8bmjs6SWj2vHJ5vC2qCL02cPkUlk2Y3WBr3kOE7WS6EL1dUwLZBnm3HZBQzTPlGogLzNZCsqAPaiZCJhLjGdVvQMNzoPYNpios6u4aFLdciq2DYV7c0806ki7ghJjOB8ApzKJof4R070wXfiePW4iZAARXe6YbW8kyrtOJFbcZBN08M4DoZD\",\"key\":3,\"dateTime\":149292654}],\"work\":{\"index\":\"\",\"name\":\"\"},\"college\":{\"index\":\"\",\"name\":\"\"}}");

   bool ok = JSON_parse(reqJson, response);

   U_INTERNAL_ASSERT(ok)

   U_ASSERT(response.links.empty())
   U_ASSERT(response.users.empty())
   U_ASSERT(response.messages.empty())
   U_ASSERT(response.events.empty())
   U_ASSERT(response.work.name.empty())
   U_ASSERT_EQUALS(response.type, "login")
   U_ASSERT(response.college.name.empty())
   U_ASSERT_EQUALS(response.socials[0]->key, 3)

   JSON_stringify(result, json_obj, response);

// (void) UFile::writeToTmp(U_STRING_TO_PARAM(result), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("test_json.%P"), 0);

   U_ASSERT_EQUALS( result.size(), reqJson.size() )

   result.clear();

   JSON_OBJ_stringify(result, response);

   U_ASSERT_EQUALS( result.size(), reqJson.size() )
}

static void testResponseSearch()
{
   U_TRACE_NO_PARAM(5, "testResponseSearch()")

   UValue json_obj;
   ResponseSearch response;
   UString result, reqJson = U_STRING_FROM_CONSTANT("{\"type\":\"localesData\",\"key\":0,\"organizations\":[{\"name\":\"Temple University\",\"index\":\"S119\"},{\"name\":\"Tennessee State University\",\"index\":\"S266\"},{\"name\":\"Tennessee Technological University\",\"index\":\"S224\"},{\"name\":\"Texas A&M University--College Station\",\"index\":\"S75\"},{\"name\":\"Texas A&M University--Commerce\",\"index\":\"S267\"}]}");

   bool ok = JSON_parse(reqJson, response);

   U_INTERNAL_ASSERT(ok)

   U_INTERNAL_ASSERT_EQUALS(response.key, 0)
   U_ASSERT_EQUALS(response.type, "localesData")
   U_ASSERT_EQUALS(response.organizations[0]->name,  "Temple University")
   U_ASSERT_EQUALS(response.organizations[0]->index, "S119")
   U_ASSERT_EQUALS(response.organizations[4]->name,  "Texas A&M University--Commerce")
   U_ASSERT_EQUALS(response.organizations[4]->index, "S267")

   JSON_stringify(result, json_obj, response);

// (void) UFile::writeToTmp(U_STRING_TO_PARAM(result), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("test_json.%P"), 0);

   U_ASSERT_EQUALS( result, reqJson )

   result.clear();

   JSON_OBJ_stringify(result, response);

   U_ASSERT_EQUALS( result, reqJson )
}

static void testMultiple()
{
   U_TRACE_NO_PARAM(5, "testMultiple()")

   UValue json_obj;
   const char* dump;
   Multiple multiple;
   UString result, reqJson = U_STRING_FROM_CONSTANT("{"
   "\"organizations\":[{\"name\":\"Temple University\",\"index\":\"S119\"},{\"name\":\"Tennessee State University\",\"index\":\"S266\"},{\"name\":\"Tennessee Technological University\",\"index\":\"S224\"},{\"name\":\"Texas A&M University--College Station\",\"index\":\"S75\"},{\"name\":\"Texas A&M University--Commerce\",\"index\":\"S267\"}],"
   "\"vrequests\":[],"
   "\"request\":{\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"},\"radius\":\"near\",\"location\":\"40.7831 N, 73.9712 W\",\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"]},"
   "\"response\":{\"fbPermissions\":[\"public_profile\",\"user_friends\",\"email\"],\"type\":\"startup\",\"token\":\"\",\"table\":{\"type\":\"localesData\",\"token\":\"A619828KAIJ6D3\"}},"
   "\"presponse\":{\"fbPermissions\":[],\"type\":\"\",\"token\":\"\",\"table\":{}}"
   "}");

   bool ok = JSON_parse(reqJson, multiple);

   U_INTERNAL_ASSERT(ok)

   U_INTERNAL_ASSERT_EQUALS(multiple.request.radius,   "near")
   U_INTERNAL_ASSERT_EQUALS(multiple.request.location, "40.7831 N, 73.9712 W")

   dump = UObject2String<UHashMap<UString> >(multiple.request.table);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]");

   U_INTERNAL_ASSERT(ok)

   dump = UObject2String<UVector<UString> >(multiple.request.fbPermissions);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

   U_INTERNAL_ASSERT(ok)

   U_INTERNAL_ASSERT_EQUALS(multiple.response.token, "")
   U_INTERNAL_ASSERT_EQUALS(multiple.response.type,  "startup")

   dump = UObject2String<UVector<UString> >(multiple.response.fbPermissions);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "( public_profile user_friends email )");

   U_INTERNAL_ASSERT(ok)

   dump = UObject2String<UHashMap<UString> >(multiple.response.table);

   U_INTERNAL_DUMP("dump(%u) = %.*S)", UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, dump)

   ok = U_STREQ(dump, UObjectIO::buffer_output_len, "[\ntype\tlocalesData\ntoken\tA619828KAIJ6D3\n]");

   U_INTERNAL_ASSERT(ok)

   JSON_stringify(result, json_obj, multiple);

// (void) UFile::writeToTmp(U_STRING_TO_PARAM(result), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("test_json.%P"), 0);

   U_ASSERT_EQUALS( result, reqJson )

   result.clear();

   JSON_OBJ_stringify(result, multiple);

   U_ASSERT_EQUALS( result, reqJson )
}

// Do a query and print the results

static void testQuery(const UString& json, const char* cquery, const UString& expected)
{
   U_TRACE(5, "testQuery(%V,%S,%V)", json.rep, cquery, expected.rep)

   char buffer[4096];
   UString result, query(cquery, strlen(cquery));
   int dataType = UValue::jread(json, query, result);

   cout.write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("dataType = (%d %S) query = %V result(%u) = %V UValue::jread_elements = %d "
                                                                              "UValue::jread_error = (%d %S)\n"),
              dataType, UValue::getDataTypeDescription(dataType), query.rep, result.size(), result.rep, UValue::jread_elements, UValue::jread_error,
              UValue::getJReadErrorDescription()));

   U_INTERNAL_ASSERT_EQUALS(result, expected)
}

int
U_EXPORT main (int argc, char* argv[], char* env[])
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

   testMap();
   testVector();
   testRequest();
   testResponse();
   testResponseLogin();
   testResponseSearch();

   testMultiple();

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

      if (UValue::jreadArrayStep(array, result1) != U_OBJECT_VALUE)
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

   (void) U_JFIND(U_STRING_FROM_CONSTANT("{ \"pricePoint\" : 2.48333333333333, \"socialWeight\" : 8.75832720587083, \"gender\" : 0, \"lessThan16\" : false }"),
                  "pricePoint", pricePoint);

   U_INTERNAL_ASSERT_EQUALS(pricePoint, 2.48333333333333)

   (void) U_JFIND(U_STRING_FROM_CONSTANT("{ \"cityKey\" : 0 }"), "cityKey", city);

   U_INTERNAL_ASSERT_EQUALS(city, 0)

   (void) UValue::jread(query, U_STRING_FROM_CONSTANT("{'coordinates' [0"), workingString);

   U_INTERNAL_ASSERT_EQUALS(workingString, "-73.9888983")

   workingString.clear();

   (void) U_JFIND(query, "coordinates", workingString);

   U_INTERNAL_ASSERT_EQUALS(workingString, "[ -73.9888983, 40.7212405 ]")

   workingString.clear();
   result1.clear();

   (void) U_JFIND(U_STRING_FROM_CONSTANT("{\"saltedHash\":\"f66113b5ed33f961219c\",\"osVersion\":\"10.3.1\",\"socials\":[{\"name\":\"victor]},\"t\":\"createAccount\"}"),
                  "t", result1);

   U_INTERNAL_ASSERT_EQUALS(result1, "createAccount")

   result1.clear();

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

   result1.clear();

   JSON_OBJ_stringify(result1, v);

// (void) UFile::writeToTmp(U_STRING_TO_PARAM(result1), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("test_json.%P"), 0);

   U_ASSERT_EQUALS( result1, vecJson )

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

   while (cin >> filename)
      {
      content = UFile::contentOf(filename);

      if (json.parse(content)) cout << json.output() << '\n';

      json.clear();
      }
}
