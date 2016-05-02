// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    objectIO.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/string.h>

char*       UObjectIO::buffer_output;
uint32_t    UObjectIO::buffer_output_sz;
uint32_t    UObjectIO::buffer_output_len;
ostrstream* UObjectIO::os;
istrstream* UObjectIO::is;

// manage representation object => string

void UObjectIO::init(char* t, uint32_t sz)
{
   U_INTERNAL_TRACE("UObjectIO::init(%p,%u)", t, sz)

   buffer_output    = t;
   buffer_output_sz = sz;

#ifdef HAVE_OLD_IOSTREAM
   os = new ostrstream(buffer_output, buffer_output_sz);
#else
   static char place[sizeof(ostrstream)];

   os = (ostrstream*) new(place) ostrstream(buffer_output, buffer_output_sz);
#endif
}

void UObjectIO::input(const char* t, uint32_t tlen)
{
   U_INTERNAL_TRACE("UObjectIO::input(%.*s,%u)", tlen, t, tlen)

#ifdef HAVE_OLD_IOSTREAM
   is = new istrstream(t, tlen);
#else
   static char place[sizeof(istrstream)];

   is = new(place) istrstream(t, tlen);
#endif
}

void UObjectIO::output()
{
   U_INTERNAL_TRACE("UObjectIO::output()")

   U_INTERNAL_ASSERT_POINTER(os)

   buffer_output_len = os->pcount();

   U_INTERNAL_PRINT("os->pcount() = %d", buffer_output_len)

   U_INTERNAL_ASSERT_MINOR(buffer_output_len, buffer_output_sz)

   buffer_output[buffer_output_len] = '\0';

   U_INTERNAL_PRINT("buffer_output = %.*s", U_min(buffer_output_len,128), buffer_output)

#ifdef DEBUG_DEBUG
   off_t pos = os->rdbuf()->pubseekpos(0, U_openmode);
#else
        (void) os->rdbuf()->pubseekpos(0, U_openmode);
#endif

   U_INTERNAL_PRINT("pos = %ld, os->pcount() = %d", pos, os->pcount())
}

UStringRep* UObjectIO::create(bool bcopy)
{
   U_TRACE(0, "UObjectIO::create(%b)", bcopy)

   UObjectIO::output();

   UStringRep* rep;

   if (bcopy) rep = UStringRep::create(buffer_output_len, buffer_output_len, (const char*)buffer_output);
   else       U_NEW(UStringRep, rep, UStringRep(buffer_output, buffer_output_len));

   U_INTERNAL_PRINT("rep = %V", rep)

   U_RETURN_POINTER(rep, UStringRep);
}
