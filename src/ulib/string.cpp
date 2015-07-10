// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    string.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/utility/escape.h>
#include <ulib/internal/chttp.h>
#include <ulib/container/hash_map.h>

struct ustring    { ustringrep* rep; };
union uustring    { ustring*    p1; UString*    p2; };
union uustringrep { ustringrep* p1; UStringRep* p2; };

static ustring     empty_string_storage = { &u_empty_string_rep_storage };
static uustringrep uustringrepnull      = { &u_empty_string_rep_storage };
static uustring    uustringnull         = {   &empty_string_storage };

UString*    UString::string_null        = uustringnull.p2;
UStringRep* UStringRep::string_rep_null = uustringrepnull.p2;

// OPTMIZE APPEND (BUFFERED)
char* UString::appbuf;
char* UString::ptrbuf;

const UString* UString::str_host;
const UString* UString::str_cookie;
const UString* UString::str_connection;
const UString* UString::str_user_agent;
const UString* UString::str_authorization;
const UString* UString::str_content_type;
const UString* UString::str_content_length;
const UString* UString::str_accept;
const UString* UString::str_accept_encoding;
const UString* UString::str_referer;
const UString* UString::str_X_Real_IP;
const UString* UString::str_Transfer_Encoding;
const UString* UString::str_X_Progress_ID;
const UString* UString::str_chunked;
const UString* UString::str_without_mac;
const UString* UString::str_encoding;
const UString* UString::str_user;
const UString* UString::str_name;
const UString* UString::str_localhost;
const UString* UString::str_http;
const UString* UString::str_filename;
const UString* UString::str_msg_rfc;
const UString* UString::str_txt_plain;
const UString* UString::str_address;
const UString* UString::str_ns;
const UString* UString::str_METHOD_NAME;
const UString* UString::str_RESPONSE_TYPE;
const UString* UString::str_xmlns;
const UString* UString::str_fault;
const UString* UString::str_boolean;
const UString* UString::str_byte;
const UString* UString::str_unsignedByte;
const UString* UString::str_short;
const UString* UString::str_unsignedShort;
const UString* UString::str_int;
const UString* UString::str_unsignedInt;
const UString* UString::str_long;
const UString* UString::str_unsignedLong;
const UString* UString::str_float;
const UString* UString::str_double;
const UString* UString::str_string;
const UString* UString::str_base64Binary;
const UString* UString::str_PORT;
const UString* UString::str_USER;
const UString* UString::str_SERVER;
const UString* UString::str_CA_FILE;
const UString* UString::str_CA_PATH;
const UString* UString::str_PASSWORD;
const UString* UString::str_KEY_FILE;
const UString* UString::str_PID_FILE;
const UString* UString::str_LOG_FILE;
const UString* UString::str_CERT_FILE;
const UString* UString::str_LOG_FILE_SZ;
const UString* UString::str_VERIFY_MODE;
const UString* UString::str_SOCKET_NAME;
const UString* UString::str_ENVIRONMENT;
const UString* UString::str_CLIENT_QUEUE_DIR;
const UString* UString::str_point;
const UString* UString::str_true;
const UString* UString::str_false;

void UString::str_allocate()
{
   U_TRACE(0+256, "UString::str_allocate()")

   U_INTERNAL_ASSERT_EQUALS(str_host, 0)
   U_INTERNAL_ASSERT_EQUALS(str_cookie, 0)
   U_INTERNAL_ASSERT_EQUALS(str_connection, 0)
   U_INTERNAL_ASSERT_EQUALS(str_user_agent, 0)
   U_INTERNAL_ASSERT_EQUALS(str_authorization, 0)
   U_INTERNAL_ASSERT_EQUALS(str_content_type, 0)
   U_INTERNAL_ASSERT_EQUALS(str_content_length, 0)
   U_INTERNAL_ASSERT_EQUALS(str_accept, 0)
   U_INTERNAL_ASSERT_EQUALS(str_accept_encoding, 0)
   U_INTERNAL_ASSERT_EQUALS(str_referer, 0)
   U_INTERNAL_ASSERT_EQUALS(str_X_Real_IP, 0)
   U_INTERNAL_ASSERT_EQUALS(str_Transfer_Encoding, 0)
   U_INTERNAL_ASSERT_EQUALS(str_X_Progress_ID, 0)
   U_INTERNAL_ASSERT_EQUALS(str_chunked, 0)
   U_INTERNAL_ASSERT_EQUALS(str_without_mac, 0)
   U_INTERNAL_ASSERT_EQUALS(str_encoding, 0)
   U_INTERNAL_ASSERT_EQUALS(str_user, 0)
   U_INTERNAL_ASSERT_EQUALS(str_name, 0)
   U_INTERNAL_ASSERT_EQUALS(str_localhost, 0)
   U_INTERNAL_ASSERT_EQUALS(str_http, 0)
   U_INTERNAL_ASSERT_EQUALS(str_filename, 0)
   U_INTERNAL_ASSERT_EQUALS(str_msg_rfc, 0)
   U_INTERNAL_ASSERT_EQUALS(str_txt_plain, 0)
   U_INTERNAL_ASSERT_EQUALS(str_address, 0)
   U_INTERNAL_ASSERT_EQUALS(str_ns, 0)
   U_INTERNAL_ASSERT_EQUALS(str_METHOD_NAME, 0)
   U_INTERNAL_ASSERT_EQUALS(str_RESPONSE_TYPE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_xmlns, 0)
   U_INTERNAL_ASSERT_EQUALS(str_fault, 0)
   U_INTERNAL_ASSERT_EQUALS(str_boolean, 0)
   U_INTERNAL_ASSERT_EQUALS(str_byte, 0)
   U_INTERNAL_ASSERT_EQUALS(str_unsignedByte, 0)
   U_INTERNAL_ASSERT_EQUALS(str_short, 0)
   U_INTERNAL_ASSERT_EQUALS(str_unsignedShort, 0)
   U_INTERNAL_ASSERT_EQUALS(str_int, 0)
   U_INTERNAL_ASSERT_EQUALS(str_unsignedInt, 0)
   U_INTERNAL_ASSERT_EQUALS(str_long, 0)
   U_INTERNAL_ASSERT_EQUALS(str_unsignedLong, 0)
   U_INTERNAL_ASSERT_EQUALS(str_float, 0)
   U_INTERNAL_ASSERT_EQUALS(str_double, 0)
   U_INTERNAL_ASSERT_EQUALS(str_string, 0)
   U_INTERNAL_ASSERT_EQUALS(str_base64Binary, 0)
   U_INTERNAL_ASSERT_EQUALS(str_PORT, 0)
   U_INTERNAL_ASSERT_EQUALS(str_USER, 0)
   U_INTERNAL_ASSERT_EQUALS(str_SERVER, 0)
   U_INTERNAL_ASSERT_EQUALS(str_CA_FILE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_CA_PATH, 0)
   U_INTERNAL_ASSERT_EQUALS(str_PASSWORD, 0)
   U_INTERNAL_ASSERT_EQUALS(str_KEY_FILE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_PID_FILE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_LOG_FILE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_CERT_FILE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_LOG_FILE_SZ, 0)
   U_INTERNAL_ASSERT_EQUALS(str_VERIFY_MODE, 0)
   U_INTERNAL_ASSERT_EQUALS(str_SOCKET_NAME, 0)
   U_INTERNAL_ASSERT_EQUALS(str_ENVIRONMENT, 0)
   U_INTERNAL_ASSERT_EQUALS(str_CLIENT_QUEUE_DIR, 0)
   U_INTERNAL_ASSERT_EQUALS(str_point, 0)
   U_INTERNAL_ASSERT_EQUALS(str_true, 0)
   U_INTERNAL_ASSERT_EQUALS(str_false, 0)
   U_INTERNAL_ASSERT_EQUALS(UHashMap<void*>::pkey, 0)

   static ustringrep stringrep_storage[] = {
      { U_STRINGREP_FROM_CONSTANT("Host") },
      { U_STRINGREP_FROM_CONSTANT("Cookie") },
      { U_STRINGREP_FROM_CONSTANT("Connection") },
      { U_STRINGREP_FROM_CONSTANT("User-Agent") },
      { U_STRINGREP_FROM_CONSTANT("Authorization") },
      { U_STRINGREP_FROM_CONSTANT("Content-Type") },
      { U_STRINGREP_FROM_CONSTANT("Content-Length") },
      { U_STRINGREP_FROM_CONSTANT("Accept") },
      { U_STRINGREP_FROM_CONSTANT("Accept-Encoding") },
      { U_STRINGREP_FROM_CONSTANT("Referer") },
      { U_STRINGREP_FROM_CONSTANT("X-Real-IP") },
      { U_STRINGREP_FROM_CONSTANT("Transfer-Encoding") },
      { U_STRINGREP_FROM_CONSTANT("X-Progress-ID") },
      { U_STRINGREP_FROM_CONSTANT("chunked") },
      { U_STRINGREP_FROM_CONSTANT("00:00:00:00:00:00") },
      { U_STRINGREP_FROM_CONSTANT("encoding") },
      { U_STRINGREP_FROM_CONSTANT("user") },
      { U_STRINGREP_FROM_CONSTANT("name") },
      { U_STRINGREP_FROM_CONSTANT("localhost") },
      { U_STRINGREP_FROM_CONSTANT("http") },
      { U_STRINGREP_FROM_CONSTANT("filename") },
      { U_STRINGREP_FROM_CONSTANT("message/rfc822") },
      { U_STRINGREP_FROM_CONSTANT("text/plain") },
      { U_STRINGREP_FROM_CONSTANT("stefano.casazza@gmail.com") },
      { U_STRINGREP_FROM_CONSTANT("ns") },
      { U_STRINGREP_FROM_CONSTANT("METHOD_NAME") },
      { U_STRINGREP_FROM_CONSTANT("RESPONSE_TYPE") },
      { U_STRINGREP_FROM_CONSTANT("xmlns") },
      { U_STRINGREP_FROM_CONSTANT("Fault") },
      { U_STRINGREP_FROM_CONSTANT("boolean") },
      { U_STRINGREP_FROM_CONSTANT("byte") },
      { U_STRINGREP_FROM_CONSTANT("unsignedByte") },
      { U_STRINGREP_FROM_CONSTANT("short") },
      { U_STRINGREP_FROM_CONSTANT("unsignedShort") },
      { U_STRINGREP_FROM_CONSTANT("int") },
      { U_STRINGREP_FROM_CONSTANT("unsignedInt") },
      { U_STRINGREP_FROM_CONSTANT("long") },
      { U_STRINGREP_FROM_CONSTANT("unsignedLong") },
      { U_STRINGREP_FROM_CONSTANT("float") },
      { U_STRINGREP_FROM_CONSTANT("double") },
      { U_STRINGREP_FROM_CONSTANT("string") },
      { U_STRINGREP_FROM_CONSTANT("base64Binary") },
      { U_STRINGREP_FROM_CONSTANT("PORT") },
      { U_STRINGREP_FROM_CONSTANT("USER") },
      { U_STRINGREP_FROM_CONSTANT("SERVER") },
      { U_STRINGREP_FROM_CONSTANT("CA_FILE") },
      { U_STRINGREP_FROM_CONSTANT("CA_PATH") },
      { U_STRINGREP_FROM_CONSTANT("PASSWORD") },
      { U_STRINGREP_FROM_CONSTANT("KEY_FILE") },
      { U_STRINGREP_FROM_CONSTANT("PID_FILE") },
      { U_STRINGREP_FROM_CONSTANT("LOG_FILE") },
      { U_STRINGREP_FROM_CONSTANT("CERT_FILE") },
      { U_STRINGREP_FROM_CONSTANT("LOG_FILE_SZ") },
      { U_STRINGREP_FROM_CONSTANT("VERIFY_MODE") },
      { U_STRINGREP_FROM_CONSTANT("SOCKET_NAME") },
      { U_STRINGREP_FROM_CONSTANT("ENVIRONMENT") },
      { U_STRINGREP_FROM_CONSTANT("/tmp/uclient") },
      { U_STRINGREP_FROM_CONSTANT(".") },
      { U_STRINGREP_FROM_CONSTANT("true") },
      { U_STRINGREP_FROM_CONSTANT("false") },
      { U_STRINGREP_FROM_CONSTANT("") }
   };

   U_NEW_ULIB_OBJECT(str_host,                  U_STRING_FROM_STRINGREP_STORAGE(0));
   U_NEW_ULIB_OBJECT(str_cookie,                U_STRING_FROM_STRINGREP_STORAGE(1));
   U_NEW_ULIB_OBJECT(str_connection,            U_STRING_FROM_STRINGREP_STORAGE(2));
   U_NEW_ULIB_OBJECT(str_user_agent,            U_STRING_FROM_STRINGREP_STORAGE(3));
   U_NEW_ULIB_OBJECT(str_authorization,         U_STRING_FROM_STRINGREP_STORAGE(4));
   U_NEW_ULIB_OBJECT(str_content_type,          U_STRING_FROM_STRINGREP_STORAGE(5));
   U_NEW_ULIB_OBJECT(str_content_length,        U_STRING_FROM_STRINGREP_STORAGE(6));
   U_NEW_ULIB_OBJECT(str_accept,                U_STRING_FROM_STRINGREP_STORAGE(7));
   U_NEW_ULIB_OBJECT(str_accept_encoding,       U_STRING_FROM_STRINGREP_STORAGE(8));
   U_NEW_ULIB_OBJECT(str_referer,               U_STRING_FROM_STRINGREP_STORAGE(9));
   U_NEW_ULIB_OBJECT(str_X_Real_IP,             U_STRING_FROM_STRINGREP_STORAGE(10));
   U_NEW_ULIB_OBJECT(str_Transfer_Encoding,     U_STRING_FROM_STRINGREP_STORAGE(11));
   U_NEW_ULIB_OBJECT(str_X_Progress_ID,         U_STRING_FROM_STRINGREP_STORAGE(12));
   U_NEW_ULIB_OBJECT(str_chunked,               U_STRING_FROM_STRINGREP_STORAGE(13));
   U_NEW_ULIB_OBJECT(str_without_mac,           U_STRING_FROM_STRINGREP_STORAGE(14));
   U_NEW_ULIB_OBJECT(str_encoding,              U_STRING_FROM_STRINGREP_STORAGE(15));
   U_NEW_ULIB_OBJECT(str_user,                  U_STRING_FROM_STRINGREP_STORAGE(16));
   U_NEW_ULIB_OBJECT(str_name,                  U_STRING_FROM_STRINGREP_STORAGE(17));
   U_NEW_ULIB_OBJECT(str_localhost,             U_STRING_FROM_STRINGREP_STORAGE(18));
   U_NEW_ULIB_OBJECT(str_http,                  U_STRING_FROM_STRINGREP_STORAGE(19));
   U_NEW_ULIB_OBJECT(str_filename,              U_STRING_FROM_STRINGREP_STORAGE(20));
   U_NEW_ULIB_OBJECT(str_msg_rfc,               U_STRING_FROM_STRINGREP_STORAGE(21));
   U_NEW_ULIB_OBJECT(str_txt_plain,             U_STRING_FROM_STRINGREP_STORAGE(22));
   U_NEW_ULIB_OBJECT(str_address,               U_STRING_FROM_STRINGREP_STORAGE(23));
   U_NEW_ULIB_OBJECT(str_ns,                    U_STRING_FROM_STRINGREP_STORAGE(24));
   U_NEW_ULIB_OBJECT(str_METHOD_NAME,           U_STRING_FROM_STRINGREP_STORAGE(25));
   U_NEW_ULIB_OBJECT(str_RESPONSE_TYPE,         U_STRING_FROM_STRINGREP_STORAGE(26));
   U_NEW_ULIB_OBJECT(str_xmlns,                 U_STRING_FROM_STRINGREP_STORAGE(27));
   U_NEW_ULIB_OBJECT(str_fault,                 U_STRING_FROM_STRINGREP_STORAGE(28));
   U_NEW_ULIB_OBJECT(str_boolean,               U_STRING_FROM_STRINGREP_STORAGE(29));
   U_NEW_ULIB_OBJECT(str_byte,                  U_STRING_FROM_STRINGREP_STORAGE(30));
   U_NEW_ULIB_OBJECT(str_unsignedByte,          U_STRING_FROM_STRINGREP_STORAGE(31));
   U_NEW_ULIB_OBJECT(str_short,                 U_STRING_FROM_STRINGREP_STORAGE(32));
   U_NEW_ULIB_OBJECT(str_unsignedShort,         U_STRING_FROM_STRINGREP_STORAGE(33));
   U_NEW_ULIB_OBJECT(str_int,                   U_STRING_FROM_STRINGREP_STORAGE(34));
   U_NEW_ULIB_OBJECT(str_unsignedInt,           U_STRING_FROM_STRINGREP_STORAGE(35));
   U_NEW_ULIB_OBJECT(str_long,                  U_STRING_FROM_STRINGREP_STORAGE(36));
   U_NEW_ULIB_OBJECT(str_unsignedLong,          U_STRING_FROM_STRINGREP_STORAGE(37));
   U_NEW_ULIB_OBJECT(str_float,                 U_STRING_FROM_STRINGREP_STORAGE(38));
   U_NEW_ULIB_OBJECT(str_double,                U_STRING_FROM_STRINGREP_STORAGE(39));
   U_NEW_ULIB_OBJECT(str_string,                U_STRING_FROM_STRINGREP_STORAGE(40));
   U_NEW_ULIB_OBJECT(str_base64Binary,          U_STRING_FROM_STRINGREP_STORAGE(41));
   U_NEW_ULIB_OBJECT(str_PORT,                  U_STRING_FROM_STRINGREP_STORAGE(42));
   U_NEW_ULIB_OBJECT(str_USER,                  U_STRING_FROM_STRINGREP_STORAGE(43));
   U_NEW_ULIB_OBJECT(str_SERVER,                U_STRING_FROM_STRINGREP_STORAGE(44));
   U_NEW_ULIB_OBJECT(str_CA_FILE,               U_STRING_FROM_STRINGREP_STORAGE(45));
   U_NEW_ULIB_OBJECT(str_CA_PATH,               U_STRING_FROM_STRINGREP_STORAGE(46));
   U_NEW_ULIB_OBJECT(str_PASSWORD,              U_STRING_FROM_STRINGREP_STORAGE(47));
   U_NEW_ULIB_OBJECT(str_KEY_FILE,              U_STRING_FROM_STRINGREP_STORAGE(48));
   U_NEW_ULIB_OBJECT(str_PID_FILE,              U_STRING_FROM_STRINGREP_STORAGE(49));
   U_NEW_ULIB_OBJECT(str_LOG_FILE,              U_STRING_FROM_STRINGREP_STORAGE(50));
   U_NEW_ULIB_OBJECT(str_CERT_FILE,             U_STRING_FROM_STRINGREP_STORAGE(51));
   U_NEW_ULIB_OBJECT(str_LOG_FILE_SZ,           U_STRING_FROM_STRINGREP_STORAGE(52));
   U_NEW_ULIB_OBJECT(str_VERIFY_MODE,           U_STRING_FROM_STRINGREP_STORAGE(53));
   U_NEW_ULIB_OBJECT(str_SOCKET_NAME,           U_STRING_FROM_STRINGREP_STORAGE(54));
   U_NEW_ULIB_OBJECT(str_ENVIRONMENT,           U_STRING_FROM_STRINGREP_STORAGE(55));
   U_NEW_ULIB_OBJECT(str_CLIENT_QUEUE_DIR,      U_STRING_FROM_STRINGREP_STORAGE(56));
   U_NEW_ULIB_OBJECT(str_point,                 U_STRING_FROM_STRINGREP_STORAGE(57));
   U_NEW_ULIB_OBJECT(str_true,                  U_STRING_FROM_STRINGREP_STORAGE(58));
   U_NEW_ULIB_OBJECT(str_false,                 U_STRING_FROM_STRINGREP_STORAGE(59));

   U_INTERNAL_ASSERT_EQUALS(*str_without_mac,      "00:00:00:00:00:00")
   U_INTERNAL_ASSERT_EQUALS(*str_CLIENT_QUEUE_DIR, "/tmp/uclient")
   U_INTERNAL_ASSERT_EQUALS(U_NUM_ELEMENTS(stringrep_storage), 61)

   uustringrep key1 = { stringrep_storage+60 };

   UHashMap<void*>::pkey = key1.p2;

   U_INTERNAL_ASSERT(UHashMap<void*>::pkey->invariant())
}

U_NO_EXPORT void UStringRep::set(uint32_t __length, uint32_t __capacity, const char* ptr)
{
// U_TRACE(0, "UStringRep::set(%u,%u,%p)", __length, __capacity, ptr) // problem with sanitize address

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(ptr)

#if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   parent = 0;
#  ifdef DEBUG
   child  = 0;
#  endif
#endif

   _length    = __length;
   _capacity  = __capacity; // [0 const | -1 mmap | -2 to free]...
   references = 0;
   str        = ptr;
}

UStringRep::UStringRep(const char* t, uint32_t tlen)
{
   U_TRACE_REGISTER_OBJECT(0, UStringRep, "%.*S,%u", tlen, t, tlen) // problem with sanitize address

   U_INTERNAL_ASSERT_POINTER(t)
   U_INTERNAL_ASSERT_MAJOR(tlen, 0)

   set(tlen, 0U, t);
}

// NB: ctor is private...

UStringRep::~UStringRep()
{
   U_TRACE(0, "UStringRep::~UStringRep()")

   // NB: we don't use delete (dtor) because it add a deallocation to the destroy process...

   U_ERROR("I can't use UStringRep on stack");
}

UStringRep* UStringRep::create(uint32_t length, uint32_t need, const char* ptr)
{
   U_TRACE(1, "UStringRep::create(%u,%u,%p)", length, need, ptr)

   U_INTERNAL_ASSERT_MAJOR(need, 0)

   char* _ptr;
   UStringRep* r;

   // NB: we don't use new (ctor) because we want an allocation with more space for string data...

#ifndef ENABLE_MEMPOOL
      r = (UStringRep*) U_SYSCALL(malloc, "%u", need+(1+sizeof(UStringRep)));
   _ptr = (char*)(r + 1);
#else
   if (need > U_CAPACITY)
      {
      _ptr = UFile::mmap(&need, -1, PROT_READ | PROT_WRITE, U_MAP_ANON, 0);

      if (_ptr == MAP_FAILED) U_RETURN_POINTER(string_rep_null, UStringRep);

      r = U_MALLOC_TYPE(UStringRep);
      }
   else
      {
#  ifdef DEBUG
      UMemoryPool::obj_class = "UStringRep";
      UMemoryPool::func_call = __PRETTY_FUNCTION__;
#  endif

      // -------------------------------------------------------------------------------------------------------------------------------
      // see: http://www.codeproject.com/Articles/702065/C-Struct-Hack
      //
      // NB: we need an array of char[_capacity], plus a terminating null char element, plus enough for the UStringRep data structure...
      // -------------------------------------------------------------------------------------------------------------------------------

      if (need > (U_STACK_TYPE_8-(1+sizeof(UStringRep))))
         {
         need = U_STACK_TYPE_9-(1+sizeof(UStringRep));

         r = (UStringRep*) UMemoryPool::pop(9);
         }
      else
         {
         int stack_index;
         uint32_t sz = need + (1+sizeof(UStringRep));

         if (sz <= U_STACK_TYPE_4) // 128
            {
            need        = U_STACK_TYPE_4-(1+sizeof(UStringRep));
            stack_index = 4;
            }
         else if (sz <= U_STACK_TYPE_5) // 256
            {
            need        = U_STACK_TYPE_5-(1+sizeof(UStringRep));
            stack_index = 5;
            }
         else if (sz <= U_STACK_TYPE_6) // 512
            {
            need        = U_STACK_TYPE_6-(1+sizeof(UStringRep));
            stack_index = 6;
            }
         else if (sz <= U_STACK_TYPE_7) // 1024
            {
            need        = U_STACK_TYPE_7-(1+sizeof(UStringRep));
            stack_index = 7;
            }
         else
            {
            U_INTERNAL_ASSERT(need <= U_STACK_TYPE_8) // 2048

            need        = U_STACK_TYPE_8-(1+sizeof(UStringRep));
            stack_index = 8;
            }

         U_INTERNAL_DUMP("sz = %u need = %u stack_index = %u", sz, need, stack_index)

         r = (UStringRep*) UMemoryPool::pop(stack_index);
         }

      _ptr = (char*)(r + 1);

#  ifdef DEBUG
      UMemoryPool::obj_class = UMemoryPool::func_call = 0;
#  endif
      }
#endif

#ifdef DEBUG
   U_SET_LOCATION_INFO;
   U_REGISTER_OBJECT_PTR(0,UStringRep,r,&(r->memory._this))
   r->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#endif

   r->set(length, need, _ptr);

   if (length &&
       ptr)
      {
      U_MEMCPY((void*)_ptr, ptr, length);

      _ptr[length] = '\0';
      }

   U_INTERNAL_ASSERT(r->invariant())

   U_RETURN_POINTER(r, UStringRep);
}

bool UString::shrink()
{
   U_TRACE(0, "UString::shrink()")

#ifdef ENABLE_MEMPOOL
   uint32_t _length = rep->_length, sz = _length+(1+sizeof(UStringRep)); // NB: we need an array of char[_length], plus a terminating null char, plus the UStringRep data structure...

   U_INTERNAL_DUMP("rep->_capacity = %u _length = %u sz = %u", rep->_capacity, _length, sz)

   U_INTERNAL_ASSERT_MAJOR(rep->_capacity, 0) // mode: 0 -> const

   if (sz <= U_STACK_TYPE_8) // 2048
      {
      int stack_index;
      uint32_t _capacity;

      if (sz <= U_STACK_TYPE_4) // 128
         {
         _capacity   = U_STACK_TYPE_4-(1+sizeof(UStringRep));
         stack_index = 4;
         }
      else if (sz <= U_STACK_TYPE_5) // 256
         {
         _capacity   = U_STACK_TYPE_5-(1+sizeof(UStringRep));
         stack_index = 5;
         }
      else if (sz <= U_STACK_TYPE_6) // 512
         {
         _capacity   = U_STACK_TYPE_6-(1+sizeof(UStringRep));
         stack_index = 6;
         }
      else if (sz <= U_STACK_TYPE_7) // 1024
         {
         _capacity   = U_STACK_TYPE_7-(1+sizeof(UStringRep));
         stack_index = 7;
         }
      else // 2048
         {
         _capacity   = U_STACK_TYPE_8-(1+sizeof(UStringRep));
         stack_index = 8;
         }

      U_INTERNAL_DUMP("_capacity = %u stack_index = %u", _capacity, stack_index)

      if (_capacity < rep->_capacity)
         {
         UStringRep* r = (UStringRep*) UMemoryPool::pop(stack_index);
         char* ptr     = (char*)(r + 1);

#     ifdef DEBUG
         U_SET_LOCATION_INFO;
         U_REGISTER_OBJECT_PTR(0,UStringRep,r,&(r->memory._this))
         r->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#     endif

         r->set(_length, _capacity, ptr);

         U_MEMCPY((void*)ptr, rep->str, _length);

         ptr[_length] = '\0';

         U_INTERNAL_ASSERT(r->invariant())

         _set(r);

         U_INTERNAL_ASSERT(invariant())

         U_RETURN(true);
         }
      }
#endif

   U_RETURN(false);
}

void UStringRep::release()
{
   U_TRACE(0, "UStringRep::release()") // problem with sanitize address

   U_INTERNAL_DUMP("this = %p parent = %p references = %u child = %d", this, parent, references, child)

#ifdef DEBUG
   bool ok = memory.invariant();

   if (ok == false)
      {
      U_ERROR("UStringRep::release() %s - this = %p parent = %p references = %u child = %d _capacity = %u str(%u) = %.*S",
               memory.getErrorType(this),
               this, parent, references, child, _capacity, _length, _length, str);
      }
#endif

   if (references)
      {
    --references;

      return;
      }

   // NB: we don't use delete (dtor) because add a deallocation to the destroy process...

   U_INTERNAL_DUMP("_capacity = %u str(%u) = %.*S", _capacity, _length, _length, str)

   U_INTERNAL_ASSERT_EQUALS(references, 0)
   U_INTERNAL_ASSERT_DIFFERS(this, string_rep_null)

#if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   if (parent)
#  ifdef U_SUBSTR_INC_REF
      parent->release(); // NB: solo la morte della substring de-referenzia la source...
#  else
      {
      U_INTERNAL_ASSERT_EQUALS(child, 0)

      U_INTERNAL_DUMP("parent->child = %d", parent->child)

   // U_INTERNAL_ASSERT_RANGE(1, parent->child, max_child)

      if (parent->child >= 1 &&
          parent->child <= max_child)
         {
         parent->child--;

         U_INTERNAL_DUMP("this = %p parent = %p parent->references = %u parent->child = %d", this, parent, parent->references, parent->child)
         }
      else
         {
         U_WARNING("parent->child has value(%d) out of range (1-%d)", parent->child, max_child);
         }
      }
   else // source...
      {
      if (child)
         {
         if (UObjectDB::fd > 0)
            {
            parent_destroy = this;

            U_DUMP_OBJECT("DEAD OF SOURCE STRING WITH CHILD ALIVE - child of this", checkIfChild)
            }
         else
            {
            char buffer[4096];

            (void) u__snprintf(buffer, sizeof(buffer), "DEAD OF SOURCE STRING WITH CHILD ALIVE: child(%u) source(%u) = %.*S", child, _length, _length, str);

            if (check_dead_of_source_string_with_child_alive)
               {
               U_INTERNAL_ASSERT_MSG(false, buffer)
               }
            else
               {
               U_WARNING("%s", buffer);
               }
            }
         }
      }
#  endif
#  ifdef DEBUG
   U_UNREGISTER_OBJECT(0, this)
#  endif
#endif

#ifndef ENABLE_MEMPOOL
   U_SYSCALL_VOID(free, "%p", (void*)this);
#else
   if (_capacity <= U_CAPACITY)
      {
      if (_capacity == 0) UMemoryPool::push(this, U_SIZE_TO_STACK_INDEX(sizeof(UStringRep))); // NB: no room for data, which mean constant string...
      else
         {
         // NB: we need an array of char[_capacity], plus a terminating null char element, plus enough for the UStringRep data structure...

         uint32_t sz = _capacity + (1 + sizeof(UStringRep));

         /**
          * -----------
          * power of 2:
          * -----------
          * 2^7   128
          * 2^8   256
          * 2^9   512
          * 2^10  1024
          * 2^11  2048
          * 2^12  4096
          * -----------
          */

         U_INTERNAL_ASSERT_EQUALS(sz & (sz-1), 0) // must be a power of 2

         UMemoryPool::push(this, MultiplyDeBruijnBitPosition2[(sz * 0x077CB531U) >> 27] - 3);
         }
      }
   else
      {
      if (_capacity != U_NOT_FOUND)
         {
         U_INTERNAL_DUMP("_capacity = %d", (int32_t)_capacity)

#     ifdef USE_LIBTDB
         if (_capacity == U_TO_FREE)
            {
            U_SYSCALL_VOID(free, "%p", (void*)str);
            }
         else
#     endif
         {
         U_INTERNAL_ASSERT_EQUALS(_capacity & U_PAGEMASK, 0)

         UMemoryPool::deallocate((void*)str, _capacity);
         }
         }
      else
         {
         ptrdiff_t resto = (ptrdiff_t)str % PAGESIZE;

         U_INTERNAL_DUMP("resto = %u _length = %u", resto, _length)

         if (resto)
            {
                str -= resto;
            _length += resto;
            }

         (void) U_SYSCALL(munmap, "%p,%lu", (void*)str, _length);
         }

      U_FREE_TYPE(this, UStringRep); // NB: in debug mode the memory area is zeroed...
      }
#endif
}

void UStringRep::fromValue(UStringRep* r)
{
   U_TRACE(0, "UStringRep::fromValue(%V)", r)

   U_INTERNAL_DUMP("r = %p r->parent = %p r->references = %d r->child = %d - %V", r, r->parent, r->references, r->child, r)

   U_INTERNAL_ASSERT(r->_capacity)
   U_INTERNAL_ASSERT_EQUALS(memcmp(this, string_rep_null, sizeof(UStringRep)), 0)

   u__memcpy(this, r, sizeof(UStringRep), __PRETTY_FUNCTION__);

   r->_capacity = 0; // NB: no room for data, constant string...

   U_INTERNAL_ASSERT(invariant())
}

#ifdef DEBUG
// substring capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...
int32_t     UStringRep::max_child;
UStringRep* UStringRep::parent_destroy;
UStringRep* UStringRep::string_rep_share;
bool        UStringRep::check_dead_of_source_string_with_child_alive = true;

bool UStringRep::checkIfReferences(const char* name_class, const void* ptr_object)
{
   U_TRACE(0, "UStringRep::checkIfReferences(%S,%p)", name_class, ptr_object)

   if (strncmp(name_class, U_CONSTANT_TO_PARAM("UString")) == 0)
      {
      U_INTERNAL_DUMP("references = %u", ((UString*)ptr_object)->rep->references)

      if (((UString*)ptr_object)->rep == string_rep_share) U_RETURN(true);
      }

   U_RETURN(false);
}

bool UStringRep::checkIfChild(const char* name_class, const void* ptr_object)
{
   U_TRACE(0, "UStringRep::checkIfChild(%S,%p)", name_class, ptr_object)

   if (strncmp(name_class, U_CONSTANT_TO_PARAM("UStringRep")) == 0)
      {
      U_INTERNAL_DUMP("parent = %p", ((UStringRep*)ptr_object)->parent)

      if (((UStringRep*)ptr_object)->parent == parent_destroy) U_RETURN(true);
      }

   U_RETURN(false);
}
#endif

UStringRep* UStringRep::substr(const char* t, uint32_t tlen)
{
   U_TRACE(0+256, "UStringRep::substr(%.*S,%u)", tlen, t, tlen)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(tlen <= _length)

   UStringRep* r;

   if (tlen == 0)
      {
      r = string_rep_null;

      r->references++;
      }
   else
      {
      U_INTERNAL_ASSERT_RANGE(str, t, end())

      r = U_NEW(UStringRep(t, tlen));

#  if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
      UStringRep* p = this;

      while (p->parent)
         {
         p = p->parent;

         U_INTERNAL_ASSERT(p->invariant())
         }

      r->parent = p;

#     ifdef U_SUBSTR_INC_REF
      p->references++; // substring increment reference of source string
#     else
      p->child++;      // substring capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...

      max_child = U_max(max_child, p->child);
#     endif

      U_INTERNAL_DUMP("r->parent = %p max_child = %d", r->parent, max_child)
#  endif
      }

   U_RETURN_POINTER(r, UStringRep);
}

__pure bool UStringRep::isSubStringOf(UStringRep* rep) const
{
   U_TRACE(0, "UStringRep::isSubStringOf(%V)", rep)

   U_CHECK_MEMORY

   if (this != rep             &&
       _capacity == 0          && // mode: 0 -> const
       begin() >= rep->begin() &&
         end() <= rep->end())
      {
#  if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
      U_INTERNAL_ASSERT_EQUALS(parent, rep)
      U_INTERNAL_ASSERT_MAJOR(rep->child, 0)
#  endif

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UStringRep::replace(const char* s, uint32_t n)
{
   U_TRACE(0, "UStringRep::replace(%S,%u)", s, n)

   U_INTERNAL_ASSERT_MAJOR(n, 0)
   U_INTERNAL_ASSERT(_capacity >= n)

   U_MEMCPY((char*)str, s, n);

   ((char*)str)[(_length = n)] = '\0';
}

void UStringRep::copy(char* s, uint32_t n, uint32_t pos) const
{
   U_TRACE(0, "UStringRep::copy(%p,%u,%u)", s, n, pos)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(pos <= _length)

   if (n > (_length - pos)) n = (_length - pos);

   U_INTERNAL_ASSERT_MAJOR(n, 0)

   U_MEMCPY(s, str + pos, n);

   s[n] = '\0';
}

void UStringRep::trim()
{
   U_TRACE(0, "UStringRep::trim()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(_capacity, 0)

   // skip white space from start

   while (_length && u__isspace(*str))
      {
      ++str;
      --_length;
      }

   U_INTERNAL_DUMP("_length = %u", _length)

   // skip white space from end

   while (_length && u__isspace(str[_length-1])) --_length;
}

__pure int UStringRep::compare(const UStringRep* rep, uint32_t depth) const
{
   U_TRACE(0, "UStringRep::compare(%p,%u)", rep, depth)

   U_CHECK_MEMORY

   int r;
   uint32_t min = U_min(_length, rep->_length);

   U_INTERNAL_DUMP("min = %u", min)

   if (depth > min) goto next;

   r = memcmp(str + depth, rep->str + depth, min - depth);

   U_INTERNAL_DUMP("str[%u] = %.*S", depth, min - depth, str + depth)

   if (r == 0)
next:
      r = (_length - rep->_length);

   U_RETURN(r);
}

__pure uint32_t UStringRep::findWhiteSpace(uint32_t pos) const
{
   U_TRACE(0, "UStringRep::findWhiteSpace(%u)", pos)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(pos <= _length)

   for (; pos < _length; ++pos)
      {
      if (u__isspace(str[pos])) U_RETURN(pos);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure bool UStringRep::isEndHeader(uint32_t pos) const
{
   U_TRACE(0, "UStringRep::isEndHeader(%u)", pos)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MINOR(pos, _length)

   const char* ptr  = str + pos;
   uint32_t _remain = (_length - pos);

   if (_remain >= 4 &&
       u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'))
      {
   // U_line_terminator_len = 2;

      U_INTERNAL_ASSERT(u__islterm(*ptr))

      U_RETURN(true);
      }

   if (_remain >= 2 &&
       u_get_unalignedp16(ptr) == U_MULTICHAR_CONSTANT16('\n','\n'))
      {
   // U_line_terminator_len = 1;

      U_INTERNAL_ASSERT(u__islterm(*ptr))

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString::UString(const char* t)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%S", t)

   uint32_t len = (t ? u__strlen(t, __PRETTY_FUNCTION__) : 0);

   if (len) rep = U_NEW(UStringRep(t, len));
   else     _copy(UStringRep::string_rep_null);

   U_INTERNAL_ASSERT(invariant())
}

UString::UString(const char* t, uint32_t len)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%.*S,%u", len, t, len)

   if (len) rep = U_NEW(UStringRep(t, len));
   else     _copy(UStringRep::string_rep_null);

   U_INTERNAL_ASSERT(invariant())
}

UString::UString(const UString& str, uint32_t pos, uint32_t n)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%p,%u,%u", &str, pos, n)

   U_INTERNAL_ASSERT(pos <= str.size())

   uint32_t sz = str.rep->fold(pos, n);

   if (sz) rep = UStringRep::create(sz, sz, str.rep->str + pos);
   else    _copy(UStringRep::string_rep_null);

   U_INTERNAL_ASSERT(invariant())
}

UString::UString(ustringrep* r)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%p", r)

#ifdef DEBUG
   r->_this = (void*)U_CHECK_MEMORY_SENTINEL;
#endif

   uustringrep u = { r };

   _copy(u.p2);

   U_INTERNAL_ASSERT(invariant())
}

UString UString::copy() const
{
   U_TRACE(0, "UString::copy()")

   if (rep->_length)
      {
      uint32_t sz = rep->_length;

      UString copia((void*)rep->str, sz);

      U_RETURN_STRING(copia);
      }

   return getStringNull();
}

// SERVICES

UString::UString(uint32_t n, unsigned char c)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%u,%C", n, c)

   rep = UStringRep::create(n, n, 0);

   (void) memset((void*)rep->str, c, n);

   U_INTERNAL_ASSERT(invariant())
}

UString::UString(uint32_t len, uint32_t sz, char* ptr) // NB: for UStringExt::deflate()...
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%u,%u,%p", len, sz, ptr)

   U_INTERNAL_ASSERT_MAJOR(sz, U_CAPACITY)
   U_INTERNAL_ASSERT_EQUALS(sz & U_PAGEMASK, 0)

   rep = U_MALLOC_TYPE(UStringRep);

#ifdef DEBUG
   U_SET_LOCATION_INFO;
   U_REGISTER_OBJECT_PTR(0,UStringRep,rep,&(rep->memory._this))
   rep->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#endif

   rep->set(len, sz, ptr);

   U_INTERNAL_ASSERT(invariant())
}

UString& UString::assign(const char* s, uint32_t n)
{
   U_TRACE(0, "UString::assign(%S,%u)", s, n)

   if (rep->references ||
       rep->_capacity < n)
      {
      if (n) _set(U_NEW(UStringRep(s, n)));
      else _assign(UStringRep::string_rep_null);
      }
   else
      {
      char* ptr = (char*)rep->str;

      U_INTERNAL_ASSERT_MAJOR(n, 0)
      U_INTERNAL_ASSERT_DIFFERS(ptr, s)

      U_MEMCPY(ptr, s, n);

      U_ASSERT(rep->uniq())

      ptr[(rep->_length = n)] = '\0';
      }

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

void UString::setBuffer(uint32_t n)
{
   U_TRACE(0, "UString::setBuffer(%u)", n)

   U_INTERNAL_ASSERT_RANGE(1, n, max_size())

   U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_capacity = %u",
                    rep,     rep->parent,     rep->references,     rep->child,     rep->_capacity)

   if (rep->references == 0 &&
       n <= rep->_capacity)
      {
      ((char*)rep->str)[(rep->_length = 0)] = '\0';
      }
   else
      {
      if (n < U_CAPACITY) n = U_CAPACITY;

      _set(UStringRep::create(0U, n, 0));
      }

   U_INTERNAL_ASSERT(invariant())
}

void UString::moveToBeginDataInBuffer(uint32_t n)
{
   U_TRACE(1, "UString::moveToBeginDataInBuffer(%u)", n)

   U_INTERNAL_ASSERT_MAJOR(rep->_length, n)
   U_INTERNAL_ASSERT_RANGE(1, n, max_size())
   U_INTERNAL_ASSERT_MAJOR(rep->_capacity, n)

#if defined(DEBUG) && !defined(U_SUBSTR_INC_REF)
   U_INTERNAL_ASSERT(rep->references == 0)
#endif

   rep->_length -= n;

   (void) U_SYSCALL(memmove, "%p,%p,%u", (void*)rep->str, rep->str + n, rep->_length);

   U_INTERNAL_ASSERT(invariant())
}

void UString::_reserve(UString& buffer, uint32_t n)
{
   U_TRACE(0, "UString::_reserve(%V,%u)", buffer.rep, n)

   UStringRep* rep = buffer.rep;

   U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_length = %u rep->_capacity = %u",
                    rep,     rep->parent,     rep->references,     rep->child,     rep->_length,     rep->_capacity)

   U_ASSERT(rep->space() < n)
   U_INTERNAL_ASSERT(n <= max_size())

   uint32_t need = rep->_length + n;

        if (need < U_CAPACITY) need = U_CAPACITY;
   else if (need > U_CAPACITY)
      {
      if (need < 2*1024*1024) need  = (need * 2) + (PAGESIZE * 2);
                              need += PAGESIZE; // NB: to avoid duplication on realloc...
      }

   buffer._set(UStringRep::create(rep->_length, need, rep->str));

   U_INTERNAL_ASSERT(buffer.invariant())
   U_INTERNAL_ASSERT(buffer.space() >= n)
}

// manage UString as memory mapped area...

void UString::mmap(const char* map, uint32_t len)
{
   U_TRACE(0, "UString::mmap(%.*S,%u)", len, map, len)

   U_INTERNAL_ASSERT_DIFFERS(map, MAP_FAILED)

   if (isMmap())
      {
      U_ASSERT(uniq())

      rep->str     = map;
      rep->_length = len;
      }
   else
      {
      _set(U_NEW(UStringRep(map, len)));

      rep->_capacity = U_NOT_FOUND;

#  if defined(MADV_SEQUENTIAL)
      if (len > (64 * PAGESIZE)) (void) U_SYSCALL(madvise, "%p,%u,%d", (void*)map, len, MADV_SEQUENTIAL);
#  endif
      }

   U_INTERNAL_ASSERT(invariant())
}

U_NO_EXPORT char* UString::__replace(uint32_t pos, uint32_t n1, uint32_t n2)
{
   U_TRACE(0, "UString::__replace(%u,%u,%u)", pos, n1, n2)

   U_INTERNAL_ASSERT_DIFFERS(n2, U_NOT_FOUND)

   uint32_t sz = size();

   U_INTERNAL_ASSERT(pos <= sz)

   uint32_t sz1 = rep->fold(pos, n1),
            n   = sz + n2  - sz1;

   U_INTERNAL_DUMP("sz1 = %u, n = %u", sz1, n)

   if (n == 0)
      {
      _assign(UStringRep::string_rep_null);

      return 0;
      }

   int32_t how_much = sz - pos - sz1;

   U_INTERNAL_DUMP("how_much = %d", how_much)

   U_INTERNAL_ASSERT(how_much >= 0)

         char* str = (char*)rep->str;
   const char* src = str + pos + sz1;

   uint32_t __capacity = rep->_capacity;

   if (__capacity == U_NOT_FOUND) __capacity = 0;

   if (rep->references ||
       n > __capacity)
      {
      U_INTERNAL_DUMP("__capacity = %u, n = %u", __capacity, n)

      if (__capacity < n) __capacity = n;

      UStringRep* r = UStringRep::create(n, __capacity, 0);

      if (pos)      U_MEMCPY((void*)r->str,            str, pos);
      if (how_much) U_MEMCPY((char*)r->str + pos + n2, src, how_much);

      _set(r);

      str = (char*)r->str;
      }
   else if (how_much > 0 &&
            n1 != n2)
      {
      (void) U_SYSCALL(memmove, "%p,%p,%u", str + pos + n2, src, how_much);
      }

   U_ASSERT(uniq())

   str[(rep->_length = n)] = '\0';

   return str + pos;
}

UString& UString::replace(uint32_t pos, uint32_t n1, const char* s, uint32_t n2)
{
   U_TRACE(0, "UString::replace(%u,%u,%S,%u)", pos, n1, s, n2)

   char* ptr = __replace(pos, n1, n2);

   if (ptr && n2) U_MEMCPY(ptr, s, n2);

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

UString& UString::replace(uint32_t pos, uint32_t n1, uint32_t n2, char c)
{
   U_TRACE(0, "UString::replace(%u,%u,%u,%C)", pos, n1, n2, c)

   char* ptr = __replace(pos, n1, n2);

   if (ptr && n2) (void) U_SYSCALL(memset, "%p,%d,%u", ptr, c, n2);

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

void UString::unQuote()
{
   U_TRACE(0, "UString::unQuote()")

   U_ASSERT(uniq())

   uint32_t len = rep->_length;

        if (len            <= 2) clear();
   else if (rep->_capacity == 0) rep->unQuote();
   else
      {
      len -= 2;

      char* ptr = (char*) rep->str;

      (void) U_SYSCALL(memmove, "%p,%p,%u", ptr, ptr + 1, len);

      ptr[(rep->_length = len)] = '\0';
      }
}

U_NO_EXPORT char* UString::__append(uint32_t n)
{
   U_TRACE(0, "UString::__append(%u)", n)

   UStringRep* r;
   char* str = (char*)rep->str;
   uint32_t sz = rep->_length, need = sz + n;

   U_INTERNAL_DUMP("need = %u", need)

   U_INTERNAL_ASSERT_MAJOR(need, 0)

   if (rep->references ||
       need > rep->_capacity)
      {
      r = UStringRep::create(sz, (need < U_CAPACITY ? U_CAPACITY : (need * 2) + (PAGESIZE * 2)), str);

      _set(r);

      str = (char*)r->str;
      }

   U_ASSERT(uniq())

   str[(rep->_length = need)] = '\0';

   return str + sz;
}

UString& UString::append(const char* s, uint32_t n)
{
   U_TRACE(0, "UString::append(%.*S,%u)", n, s, n) // problem with sanitize address

   if (n)
      {
      char* ptr = __append(n);

      U_MEMCPY(ptr, s, n);
      }

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

UString& UString::append(uint32_t n, char c)
{
   U_TRACE(0, "UString::append(%u,%C)", n, c)

   if (n)
      {
      char* ptr    = __append(n);
            ptr[0] = c;

      if (--n) (void) U_SYSCALL(memset, "%p,%d,%u", ptr+1, c, n);
      }

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

void UString::duplicate() const
{
   U_TRACE(0, "UString::duplicate()")

   uint32_t sz = size();

   if (sz) ((UString*)this)->_set(UStringRep::create(sz, sz, rep->str));
   else
      {
        ((UString*)this)->_set(UStringRep::create(0, 100U, 0));

      *(((UString*)this)->UString::rep->begin()) = '\0';
      }

   U_INTERNAL_ASSERT(invariant())
   U_INTERNAL_ASSERT(isNullTerminated())
}

void UString::setNullTerminated() const
{
   U_TRACE(0, "UString::setNullTerminated()")

   // A file is mapped in multiples of the page size. For a file that is not a multiple of the page size,
   // the remaining memory is zeroed when mapped, and writes to that region are not written out to the file

   if (writeable() == false ||
       (isMmap() && (rep->_length % PAGESIZE) == 0))
      {
      duplicate();
      }
   else
      {
      rep->setNullTerminated();
      }

   U_ASSERT_EQUALS(u__strlen(rep->str, __PRETTY_FUNCTION__), rep->_length)
}

void UString::resize(uint32_t n, unsigned char c)
{
   U_TRACE(0, "UString::resize(%u,%C)", n, c)

   U_INTERNAL_ASSERT(n <= max_size())

   uint32_t sz = size();

   if      (n > sz) (void) append(n - sz, c);
   else if (n < sz) erase(n);
   else             size_adjust(n);

   U_INTERNAL_ASSERT(invariant())
}

// The `find' function searches string for a specified string (possibly a single character) and returns
// its starting position. You can supply the parameter pos to specify the position where search must begin

__pure uint32_t UString::find(const char* s, uint32_t pos, uint32_t s_len, uint32_t how_much) const
{
   U_TRACE(0, "UString::find(%S,%u,%u,%u)", s, pos, s_len, how_much)

   U_INTERNAL_ASSERT_MAJOR(s_len, 0)

// An empty string consists of no characters, therefore it should be found at every point in a UString, except beyond the end...
// if (s_len == 0) U_RETURN(pos <= size() ? pos : U_NOT_FOUND);

   uint32_t n = rep->fold(pos, how_much);

   U_INTERNAL_DUMP("rep->_length = %u", rep->_length)

   U_INTERNAL_ASSERT(n <= rep->_length)

   const char* str = rep->str;
   const char* ptr = (const char*) u_find(str + pos, n, s, s_len);

   n = (ptr ? ptr - str : U_NOT_FOUND);

   U_RETURN(n);
}

__pure uint32_t UString::findnocase(const char* s, uint32_t pos, uint32_t s_len, uint32_t how_much) const
{
   U_TRACE(0, "UString::findnocase(%S,%u,%u,%u)", s, pos, s_len, how_much)

   U_INTERNAL_ASSERT_MAJOR(s_len, 1)

   uint32_t n     = rep->fold(pos, how_much);
    int32_t __end = n - s_len + 1;

   if (__end > 0)
      {
      const char* str = rep->str + pos;

      for (int32_t xpos = 0; xpos < __end; ++xpos)
         {
         if (u__strncasecmp(str + xpos, s, s_len) == 0) U_RETURN(pos+xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::find(%C,%u)", c, pos)

   uint32_t sz  = size(),
            ret = U_NOT_FOUND;

   const char* str = rep->str;

   if (pos < sz)
      {
      uint32_t how_much = (sz - pos);

   // U_INTERNAL_DUMP("how_much = %u", how_much)

      void* p = (void*) memchr(str + pos, c, how_much);

      if (p) ret = (const char*)p - str;
      }

   U_RETURN(ret);
}

// rfind() instead of starting at the beginning of the string and searching for the text's first occurence, starts its search at the end and returns the last occurence

__pure uint32_t UString::rfind(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::rfind(%C,%u)", c, pos)

   uint32_t sz = size();

   if (sz)
      {
      uint32_t xpos = sz - 1;

      if (xpos > pos) xpos = pos;

      const char* str = rep->str;

      for (++xpos; xpos-- > 0; )
         {
         if (str[xpos] == c) U_RETURN(xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::rfind(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::rfind(%S,%u,%u)", s, pos, n)

   uint32_t sz = size();

   if (n <= sz)
      {
      pos = U_min(sz - n, pos);

      const char* str = rep->str;

      do {
         if (memcmp(str + pos, s, n) == 0) U_RETURN(pos);
         }
      while (pos-- > 0);
      }

   U_RETURN(U_NOT_FOUND);
}

// Instead of searching for the entire string, find_first_of() returns as soon as a single common element is found between the strings being compared.
// And yes, this means that the find_first_of() that take a single char are exactly the same as the find() functions with the same parameters...

__pure uint32_t UString::find_first_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_first_of(%S,%u,%u)", s, pos, n)

   if (n)
      {
      uint32_t sz = size();

      const char* str = rep->str;

      for (; pos < sz; ++pos)
         {
         if (memchr(s, str[pos], n)) U_RETURN(pos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_last_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_last_of(%S,%u,%u)", s, pos, n)

   uint32_t sz = size();

   if (sz && n)
      {
      if (--sz > pos) sz = pos;

      const char* str = rep->str;

      do {
         if (memchr(s, str[sz], n)) U_RETURN(sz);
         }
      while (sz-- != 0);
      }

   U_RETURN(U_NOT_FOUND);
}

// Now these functions, instead of returning an index to the first common element, returns an index to the first non-common element...

__pure uint32_t UString::find_first_not_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_first_not_of(%S,%u,%u)", s, pos, n)

   if (n)
      {
      uint32_t sz   = size(),
               xpos = pos;

      const char* str = rep->str;

      for (; xpos < sz; ++xpos)
         {
         if (memchr(s, str[xpos], n) == 0) U_RETURN(xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_first_not_of(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::find_first_not_of(%C,%u)", c, pos)

   uint32_t sz   = size(),
            xpos = pos;

   const char* str = rep->str;

   for (; xpos < sz; ++xpos)
      {
      if (str[xpos] != c) U_RETURN(xpos);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_last_not_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_last_not_of(%S,%u,%u)", s, pos, n)

   uint32_t sz = size();

   if (sz && n)
      {
      if (--sz > pos) sz = pos;

      const char* str = rep->str;

      do {
         if (memchr(s, str[sz], n) == 0) U_RETURN(sz);
         }
      while (sz-- != 0);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_last_not_of(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::find_last_not_of(%C,%u)", c, pos)

   uint32_t sz = size();

   if (sz)
      {
      uint32_t xpos = sz - 1;

      if (xpos > pos) xpos = pos;

      const char* str = rep->str;

      for (++xpos; xpos-- > 0; )
         {
         if (str[xpos] != c) U_RETURN(xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

// EXTENSION

void UString::snprintf(const char* format, ...)
{
   U_TRACE(0, "UString::snprintf(%S)", format)

   U_INTERNAL_ASSERT_POINTER(format)

   va_list argp;
   va_start(argp, format);

   UString::vsnprintf(format, argp); 

   va_end(argp);
}

void UString::snprintf_add(const char* format, ...)
{
   U_TRACE(0, "UString::snprintf_add(%S)", format)

   U_INTERNAL_ASSERT_POINTER(format)

   va_list argp;
   va_start(argp, format);

   UString::vsnprintf_add(format, argp); 

   va_end(argp);
}

__pure bool UStringRep::strtob() const
{
   U_TRACE(0, "UStringRep::strtob()")

   if (_length)
      {
      enum {
         VAL_yes  = U_MULTICHAR_CONSTANT32('y','e','s',0),
         VAL_Yes  = U_MULTICHAR_CONSTANT32('Y','e','s',0),
         VAL_YES  = U_MULTICHAR_CONSTANT32('Y','E','S',0),
         VAL_true = U_MULTICHAR_CONSTANT32('t','r','u','e'),
         VAL_True = U_MULTICHAR_CONSTANT32('T','r','u','e'),
         VAL_TRUE = U_MULTICHAR_CONSTANT32('T','R','U','E')
      };

      switch (u_get_unalignedp32(str))
         {
         case VAL_yes:
         case VAL_Yes:
         case VAL_YES:
         case VAL_true:
         case VAL_True:
         case VAL_TRUE: U_RETURN(true); 
         }

      switch (u_get_unalignedp16(str))
         {
         case U_MULTICHAR_CONSTANT16('1',0):
         case U_MULTICHAR_CONSTANT16('o','n'):
         case U_MULTICHAR_CONSTANT16('O','n'):
         case U_MULTICHAR_CONSTANT16('O','N'): U_RETURN(true);
         }
      }

   U_RETURN(false);
}

long UStringRep::strtol(int base) const
{
   U_TRACE(0, "UStringRep::strtol(%d)", base)

   if (_length)
      {
      char* eos = (char*)str + _length;

      if (isNullTerminated() == false && writeable()) *eos = '\0';

      errno = 0;

      char* endptr;
      long  result = (long) strtoul(str, &endptr, base);

      U_INTERNAL_DUMP("errno = %d", errno)

      if (endptr &&
          endptr < eos)
         {
         U_NUMBER_SUFFIX(result, *endptr);
         }

      U_RETURN(result);
      }

   U_RETURN(0L);
}

#ifdef HAVE_STRTOULL
int64_t UStringRep::strtoll(int base) const
{
   U_TRACE(0, "UStringRep::strtoll(%d)", base)

   if (_length)
      {
      char* eos = (char*)str + _length;

      if (isNullTerminated() == false && writeable()) *eos = '\0';

      char* endptr;
      int64_t result = (int64_t) strtoull(str, &endptr, base);

      U_INTERNAL_DUMP("errno = %d", errno)

      if (endptr &&
          endptr < eos)
         {
         U_NUMBER_SUFFIX(result, *endptr);
         }

      U_RETURN(result);
      }

   U_RETURN(0LL);
}
#endif

#ifdef HAVE_STRTOF
// extern "C" { float strtof(const char* nptr, char** endptr); }
float UStringRep::strtof() const
{
   U_TRACE(0, "UStringRep::strtof()")

   if (_length)
      {
      if (isNullTerminated() == false && writeable()) setNullTerminated();

      float result = ::strtof(str, 0);

      U_INTERNAL_DUMP("errno = %d", errno)

      U_RETURN(result);
      }

   U_RETURN(0);
}
#endif

double UStringRep::strtod() const
{
   U_TRACE(0, "UStringRep::strtod()")

   if (_length)
      {
      if (isNullTerminated() == false && writeable()) setNullTerminated();

      double result = ::strtod(str, 0);

      U_INTERNAL_DUMP("errno = %d", errno)

      U_RETURN(result);
      }

   U_RETURN(0);
}

#ifdef HAVE_STRTOLD
// extern "C" { long double strtold(const char* nptr, char** endptr); }
long double UStringRep::strtold() const
{
   U_TRACE(0, "UStringRep::strtold()")

   if (_length)
      {
      if (isNullTerminated() == false && writeable()) setNullTerminated();

      long double result = ::strtold(str, 0);

      U_INTERNAL_DUMP("errno = %d", errno)

      U_RETURN(result);
      }

   U_RETURN(0);
}
#endif

// UTF8 <--> ISO Latin 1

UStringRep* UStringRep::fromUTF8(const unsigned char* s, uint32_t n)
{
   U_TRACE(0, "UStringRep::fromUTF8(%.*S,%u)", n, s, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n, 0)

   int c, c1, c2;
   UStringRep* r = UStringRep::create(n, n, 0);

   char* p                   = (char*)r->str;
   const unsigned char* _end = s + n;

   while (s < _end)
      {
      if (  s < (_end - 1)        &&
          (*s     & 0xE0) == 0xC0 &&
          (*(s+1) & 0xC0) == 0x80)
         {
         c1 = *s++ & 0x1F;
         c2 = *s++ & 0x3F;
         c  = (c1 << 6) + c2;

         U_INTERNAL_DUMP("c = %d %C", c, (char)c)
         }
      else
         {
         c = *s++;
         }

      *p++ = (unsigned char)c;
      }

   r->_length = p - r->str;

   U_INTERNAL_ASSERT(r->invariant())

   U_RETURN_POINTER(r, UStringRep);
}

UStringRep* UStringRep::toUTF8(const unsigned char* s, uint32_t n)
{
   U_TRACE(0, "UStringRep::toUTF8(%.*S,%u)", n, s, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n, 0)

   UStringRep* r = UStringRep::create(n, n * 2, 0);

   char* p                   = (char*)r->str;
   const unsigned char* _end = s + n;

   while (s < _end)
      {
      int c = *s++;

      if (c >= 0x80)
         {
         *p++ = ((c >> 6) & 0x1F) | 0xC0;
         *p++ = ( c       & 0x3F) | 0x80;

         continue;
         }

      *p++ = (unsigned char)c;
      }

   r->_length = p - r->str;

   U_INTERNAL_ASSERT(r->invariant())

   U_RETURN_POINTER(r, UStringRep);
}

void UString::printKeyValue(const char* key, uint32_t keylen, const char* _data, uint32_t datalen)
{
   U_TRACE(0, "UString::printKeyValue(%.*S,%u,%.*S,%u,%d)", keylen, key, keylen, datalen, _data, datalen)

   uint32_t n = 5 + 18 + keylen + datalen; 

   if (rep->space() < n) _reserve(*this, n);

   char* ptr = (char*)rep->str + rep->_length;

   ptr += u__snprintf(ptr, 40, "+%u,%u:", keylen, datalen);

   U_MEMCPY(ptr, key, keylen);
            ptr +=    keylen;

   U_MEMCPY(ptr, "->", U_CONSTANT_SIZE("->"));
            ptr +=     U_CONSTANT_SIZE("->");

   U_MEMCPY(ptr, _data, datalen);
            ptr +=      datalen;

   u_put_unalignedp16(ptr, U_MULTICHAR_CONSTANT16('\n','\0'));

   rep->_length = (ptr - rep->str) + 1;

   U_INTERNAL_ASSERT(invariant())
}

void UString::setFromData(const char** p, uint32_t sz)
{
   U_TRACE(0, "UString::setFromData(%.*S,%u)", sz, *p, sz)

   U_ASSERT(empty())
   U_INTERNAL_ASSERT_MAJOR(sz, 0)

   const char* ptr  = *p;
   unsigned char c  = *ptr;
   const char* pend =  ptr + sz;

   U_INTERNAL_DUMP("c = %C", c)

   U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

   if (LIKELY(c != '@'))
      {
      do {
         _append(c);

         if  (++ptr >= pend) break;

         c = *ptr;

         if (u__isspace(c)) break;
         }
      while (true);

      _append();
      }
   else
      {
      // get content pointed by string 'meta' (that start with '@')

      if (u_get_unalignedp32(ptr+1) == U_MULTICHAR_CONSTANT32('F','I','L','E'))
         {
         UFile file;
         char pathname[U_PATH_MAX];

         ptr += U_CONSTANT_SIZE("@FILE:");

         if (*ptr == '"') ++ptr; // check if string is quoted...

         U_INTERNAL_ASSERT_EQUALS(u__isspace(*ptr), false)

         for (char* path = pathname; ptr < pend; ++path, ++ptr)
            {
            c = *ptr;

            if (c == '"' ||
                u__isspace(c))
               {
               if (c == '"') ++ptr;

               *path = '\0';

               break;
               }

            *path = c;
            }

         *p = ptr;

         if (file.open(pathname)) *this = file.getContent();
         else
            {
            U_WARNING("open file %S specified in configuration failed", pathname);
            }

         return;
         }

      U_INTERNAL_ASSERT_EQUALS(memcmp(ptr, U_CONSTANT_TO_PARAM("@STRING:")), 0)

      ptr += U_CONSTANT_SIZE("@STRING:");

      const char* start;

      if (*ptr == '"') // check if string is quoted...
         {
         ptr = u_find_char((start = (ptr+1)), pend, '"'); // find char '"' not quoted

         if (ptr == pend)
            {
            (void) append(ptr, pend-ptr);

            *p = pend;

            return;
            }

         U_INTERNAL_ASSERT_EQUALS(*ptr, '"')

         sz = ptr++ - start;
         }
      else
         {
         for (start = ptr; ptr < pend; ++ptr)
            {
            c = *ptr;

            if (u__isspace(c)) break;
            }

         sz = ptr - start;
         }

      setBuffer(sz * 4);

      UEscape::decode(start, sz, *this);
      }

   *p = ptr;

   if (empty()  == false &&
       shrink() == false)
      {
      setNullTerminated();
      }

   U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

   U_INTERNAL_ASSERT(invariant())
}

void UString::setFromData(const char** p, uint32_t sz, unsigned char delim)
{
   U_TRACE(0, "UString::setFromData(%.*S,%u,%C)", sz, *p, sz, delim)

   U_ASSERT(empty())

   const char* ptr = *p;

   U_INTERNAL_ASSERT_EQUALS(u__isspace(*ptr), false)

   for (const char* pend = ptr + sz; ptr < pend; ++ptr)
      {
      unsigned char c = *ptr;

      U_INTERNAL_DUMP("c = %C", c)

      if (c == delim)
         {
         ++ptr;

         break;
         }

      if (c == '\\')
         {
         c = *++ptr;

         U_INTERNAL_DUMP("c (after '\\') = %C", c)

         if (c != delim)
            {
            if (c == '\n')
               {
               // compress multiple white-space in a single new-line...

               U_INTERNAL_DUMP("ptr+1 = %.*S", 20, ptr+1)

               while (ptr < pend)
                  {
                  if (u__isspace(ptr[1]) == false) break;

                  ++ptr;
                  }

               U_INTERNAL_DUMP("ptr+1 = %.*S", 20, ptr+1)
               }
            else if (strchr("nrtbfvae", c))
               {
               _append('\\');
               }
            }
         }

      _append(c);
      }

   *p = ptr;

   _append();

   if (empty()  == false &&
       shrink() == false)
      {
      setNullTerminated();
      }

   U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

   U_INTERNAL_ASSERT(invariant())
}

// STREAM

#ifdef U_STDCPP_ENABLE
void UString::get(istream& is)
{
   U_TRACE(0, "UString::get(%p)", &is) // problem with sanitize address

   if (is.peek() != '"') is >> *this;
   else
      {
      (void) is.get(); // skip '"'

      (void) getline(is, '"');
      }
}

void UStringRep::write(ostream& os) const
{
   U_TRACE(0, "UStringRep::write(%p)", &os)

   bool need_quote = (_length == 0);

   if (need_quote == false)
      {
      for (const unsigned char* s = (const unsigned char*)str, *_end = s + _length; s < _end; ++s)
         {
         unsigned char c = *s;

         if (c == '"'  ||
             c == '\\' ||
             u__isspace(c))
            {
            need_quote = true;

            break;
            }
         }
      }

   if (need_quote == false) os.write(str, _length);
   else
      {
      os.put('"');

      char* p;
      char* s    = (char*)str;
      char* _end = s + _length;

      while (s < _end)
         {
         p = (char*) memchr(s, '"', _end - s);

         if (p == 0)
            {
            os.write(s, _end - s);

            break;
            }

         os.write(s, p - s);

         if (*(p-1) == '\\') os.put('\\');
                             os.put('\\');
                             os.put('"');

         s = p + 1;
         }

      os.put('"');
      }
}

U_EXPORT istream& operator>>(istream& in, UString& str)
{
   U_TRACE(0+256, "UString::operator>>(%p,%p)", &in, &str)

   uint32_t extracted = 0;

   if (in.good())
      {
      streambuf* sb = in.rdbuf();

      int c = sb->sbumpc();

      if (in.flags() & ios::skipws)
         {
         while ((c != EOF) &&
                u__isspace(c))
            {
            c = sb->sbumpc();
            }
         }

      if (c != EOF)
         {
         if (str) str.rep->size_adjust(0U);

         streamsize w = in.width();

         uint32_t n = (w > 0 ? (uint32_t)w
                             : str.max_size());

         while (extracted < n &&
                u__isspace(c) == false)
            {
            str._append(c);

            ++extracted;

            c = sb->sbumpc();

         // U_INTERNAL_DUMP("c = %C, EOF = %C", c, EOF)

            if (c == EOF) break;
            }

         str._append();
         }

      if (c == EOF) in.setstate(ios::eofbit);
      else          sb->sputbackc(c);

      in.width(0);

      U_INTERNAL_DUMP("size = %u, str = %V", str.size(), str.rep)
      }

        if (extracted == 0) in.setstate(ios::failbit);
   else if (str.shrink() == false) str.setNullTerminated();

   U_INTERNAL_ASSERT(str.invariant())

   return in;
}

istream& UString::getline(istream& in, unsigned char delim)
{
   U_TRACE(0+256, "UString::getline(%p,%C)", &in, delim)

   int c          = EOF;
   bool extracted = false;

   if (in.good())
      {
      if (empty() == false) rep->size_adjust(0U);

      streambuf* sb = in.rdbuf();

      while (true)
         {
         c = sb->sbumpc();

         U_INTERNAL_DUMP("c = %C", c)

         if (c == '\\')
            {
            c = sb->sbumpc();

            U_INTERNAL_DUMP("c = %C", c)

            if (c == delim)
               {
               _append(delim);

               continue;
               }

            if (strchr("nrt\nbfvae", c))
               {
               if (c != '\n') _append('\\');
               else
                  {
                  // compress multiple white-space in a single new-line...

                  do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c));

                  if (c != EOF)
                     {
                     sb->sputbackc(c);

                     c = '\n';
                     }
                  }
               }
            }

         if (c == EOF)
            {
            in.setstate(ios::eofbit);

            break;
            }

         if (c == delim) break;

         _append(c);
         }

      _append();

      U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

      extracted = (empty() == false);

      if (extracted &&
          shrink() == false)
         {
         setNullTerminated();
         }
      }

   if (c         != delim &&
       extracted == false)
      {
      in.setstate(ios::failbit);
      }

   U_INTERNAL_ASSERT(invariant())

   return in;
}

U_EXPORT ostream& operator<<(ostream& out, const UString& str)
{
   U_TRACE(0, "UString::operator<<(%p,%p)", &out, &str)

   if (out.good())
      {
      const char* s = str.data();

      streamsize res,
                 w   = out.width(),
                 len = (streamsize)str.size();

      if (w <= len) res = out.rdbuf()->sputn(s, len);
      else
         {
         int plen = (int)(w - len);

         ios::fmtflags fmt = (out.flags() & ios::adjustfield);

         if (fmt == ios::left)
            {
            res = out.rdbuf()->sputn(s, len);

            // Padding last

            for (int i = 0; i < plen; ++i) (void) out.rdbuf()->sputc(' ');
            }
         else
            {
            // Padding first

            for (int i = 0; i < plen; ++i) (void) out.rdbuf()->sputc(' ');

            res = out.rdbuf()->sputn(s, len);
            }
         }

      U_INTERNAL_DUMP("len = %u, res = %u, w = %u", len, res, w)

      out.width(0);

      if (res != len) out.setstate(ios::failbit);
      }

   return out;
}
#endif

// operator +

U_EXPORT UString operator+(const char* lhs, const UString& rhs)
{
   uint32_t len = u__strlen(lhs, __PRETTY_FUNCTION__);
   UString str(len + rhs.size());

   (void) str.append(lhs, len);
   (void) str.append(rhs);

   return str;
}

U_EXPORT UString operator+(char lhs, const UString& rhs)
{
   UString str(1U + rhs.size());

   (void) str.append(1U, lhs);
   (void) str.append(rhs);

   return str;
}

#ifdef DEBUG
const char* UStringRep::dump(bool reset) const
{
#ifdef U_STDCPP_ENABLE
   *UObjectIO::os << "length     " << _length       << '\n'
                  << "capacity   " << _capacity     << '\n'
                  << "references " << references    << '\n'
                  << "parent     " << (void*)parent << '\n'
                  << "child      " << child         << '\n'
                  << "str        " << (void*)str    << ' ';

   char buffer[1024];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), "%V", this));

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }
#endif

   return 0;
}

bool UStringRep::invariant() const
{
   if (_capacity &&
       _capacity < _length)
      {
      U_WARNING("error on rep string: (overflow)\n"
                "--------------------------------------------------\n%s", UStringRep::dump(true));

      return false;
      }

   if ((int32_t)references < 0)
      {
      U_WARNING("error on rep string: (leak reference)\n"
                "--------------------------------------------------\n%s", UStringRep::dump(true));

      return false;
      }

   if (this == string_rep_null)
      {
      U_CHECK_MEMORY

      if (_length)
         {
         U_WARNING("error on string_rep_null: (not empty)\n"
                   "--------------------------------------------------\n%s", UStringRep::dump(true));

         return false;
         }

      return true;
      }

   return string_rep_null->invariant();
}

bool UString::invariant() const
{
   if (rep == 0)
      {
      U_WARNING("error on string: (rep = null pointer)");

      return false;
      }

   return rep->invariant();
}

void UString::vsnprintf_check(const char* format) const
{
   bool ok_writeable  = writeable(),
        ok_isNull     = (isNull() == false),
        ok_references = (rep->references == 0),
        ok_format     = (rep->_capacity > u__strlen(format, __PRETTY_FUNCTION__));

   if (ok_writeable == false ||
       ok_isNull    == false ||
       ok_format    == false)
      {
      // -----------------------------------------------------------------------------------------------------------------------------------------
      // Ex: userver_tcp: ERROR: UString::vsnprintf_check() this = 0xa79bbd18 parent = (nil) references = 2126 child = 0 _capacity = 0 str(0) = ""
      //                  format = "%v:" - ok_writeable = false ok_isNull = false ok_references = false ok_format = false
      // -----------------------------------------------------------------------------------------------------------------------------------------

      U_ERROR("UString::vsnprintf_check() this = %p parent = %p references = %u child = %d _capacity = %u str(%u) = %V format = %S - "
              "ok_writeable = %b ok_isNull = %b ok_references = %b ok_format = %b",
               this, rep->parent, rep->references, rep->child, rep->_capacity, rep->_length, rep->_length, rep, format,
               ok_writeable, ok_isNull, ok_references, ok_format);
      }
   else if (ok_references == false)
      {
      U_WARNING("UString::vsnprintf_check() this = %p parent = %p references = %u child = %d _capacity = %u str(%u) = %V format = %S - "
                "ok_writeable = %b ok_isNull = %b ok_references = %b ok_format = %b",
                this, rep->parent, rep->references, rep->child, rep->_capacity, rep->_length, rep->_length, rep, format,
                ok_writeable, ok_isNull, ok_references, ok_format);
      }
}
#endif

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UString::dump(bool reset) const
{
   U_CHECK_MEMORY_OBJECT(rep)

   *UObjectIO::os << "rep (UStringRep " << (void*)rep << ")";

   if (rep == rep->string_rep_null) UObjectIO::os->write(U_CONSTANT_TO_PARAM(" == UStringRep::string_rep_null"));

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
