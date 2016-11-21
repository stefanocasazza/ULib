// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    data_session.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/uhttp.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/data_session.h>

uint32_t UDataStorage::buffer_len;

// method VIRTUAL to define

char* UDataStorage::toBuffer()
{
   U_TRACE_NO_PARAM(0, "UDataStorage::toBuffer()")

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

#ifdef U_STDCPP_ENABLE
   ostrstream os(u_buffer, U_BUFFER_SIZE);

   toStream(os);

   if (u_buffer_len == 0) u_buffer_len = os.pcount();
#endif

   U_INTERNAL_ASSERT_MINOR(u_buffer_len, U_BUFFER_SIZE)

   buffer_len = u_buffer_len;

   U_RETURN(u_buffer);
}

void UDataStorage::fromData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UDataStorage::fromData(%.*S,%u)", len, ptr, len)

   U_INTERNAL_ASSERT_POINTER(ptr)

#ifdef U_STDCPP_ENABLE
   istrstream is(ptr, len);

   fromStream(is);
#endif
}

UString UDataSession::setKeyIdDataSession(uint32_t counter)
{
   U_TRACE(0, "UDataSession::setKeyIdDataSession(%u)", counter)

   keyid.setBuffer(100U);

   keyid.snprintf(U_CONSTANT_TO_PARAM("%.*s_%u_%P_%u"), U_CLIENT_ADDRESS_TO_TRACE, UHTTP::getUserAgent(), counter);

   U_RETURN_STRING(keyid);
}

UString UDataSession::setKeyIdDataSession(uint32_t counter, const UString& data)
{
   U_TRACE(0, "UDataSession::setKeyIdDataSession(%u,%V)", counter, data.rep)

   keyid.setBuffer(100U + data.size());

   keyid.snprintf(U_CONSTANT_TO_PARAM("%.*s_%u_%P_%u:%v"), U_CLIENT_ADDRESS_TO_TRACE, UHTTP::getUserAgent(), counter, data.rep);

   U_RETURN_STRING(keyid);
}

#ifdef U_STDCPP_ENABLE
void UDataSession::toStream(ostream& os)
{
   U_TRACE(0, "UDataSession::toStream(%p)", &os)

   os.put('{');
   os.put(' ');
   os << creation;
   os.put(' ');
   os << *vec_var;
   os.put(' ');
   os.put('}');
}

void UDataSession::fromStream(istream& is)
{
   U_TRACE(0, "UDataSession::fromStream(%p)", &is)

   U_INTERNAL_ASSERT_EQUALS(is.peek(), '{')

   is.get(); // skip '{'

   is >> creation;

   is.get(); // skip ' '

   U_INTERNAL_ASSERT(is.peek() == '(')

   is >> *vec_var;

   is.get(); // skip ' '
   is.get(); // skip '}'

   last_access = u_now->tv_sec;
}

#  ifdef DEBUG
const char* UDataStorage::dump(bool reset) const
{
   *UObjectIO::os << "keyid   (UString           " << (void*)&keyid << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UDataSession::dump(bool reset) const
{
   UDataStorage::dump(false);

   *UObjectIO::os << '\n'
                  << "creation                   " << creation       << '\n'
                  << "last_access                " << last_access    << '\n'
                  << "vec_var (UVector<UString*> " << (void*)vec_var << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
