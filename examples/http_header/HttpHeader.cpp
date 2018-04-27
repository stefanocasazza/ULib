// HttpHeader.cpp

#include <HttpHeader.h>

__pure unsigned HttpHeader::count(const UString& name)
{
   U_TRACE(5, "HttpHeader::count(%.*S)", U_STRING_TO_TRACE(name))

   unsigned i, j;

   for (i = j = 0; i < header.size(); ++i)
      {
      if (name == header[i]->name)
         {
         ++j;
         }
      }

   U_RETURN(j);
}

HttpField* HttpHeader::add(HttpField* field, unsigned index)
{
   U_TRACE(5, "HttpHeader::add(%p,%d)", field, index)

   U_INTERNAL_ASSERT_POINTER(field)

   if (index > 0)
      {
      if (index < header.size())
         {
         HttpField* old = header[index];
         header[index]  = field;

         U_RETURN_POINTER(old, HttpField);
         }
      else
         {
         U_RETURN_POINTER(U_NULLPTR, HttpField);
         }
      }
   else
      {
      header.push_back(field);
      }

   U_RETURN_POINTER(field, HttpField);
}

__pure HttpField* HttpHeader::find(const UString& name, unsigned index)
{
   U_TRACE(5, "HttpHeader::find(%.*S,%d)", U_STRING_TO_TRACE(name), index)

   unsigned i, j;

   for (i = j = 0; i < header.size(); ++i)
      {
      if (name == header[i]->name)
         {
         if (index == j)
            {
            U_RETURN_POINTER(header[i], HttpField);
            }

         ++j;
         }
      }

   U_RETURN_POINTER(U_NULLPTR, HttpField);
}

HttpField* HttpHeader::del(const UString& name, unsigned index)
{
   U_TRACE(5, "HttpHeader::del(%.*S,%d)", U_STRING_TO_TRACE(name), index)

   unsigned i, j;

   for (i = j = 0; i < header.size(); ++i)
      {
      if (name == header[i]->name)
         {
         if (index == j)
            {
            HttpField* field = header[i];

            U_VEC_ERASE1(header, i);

            U_RETURN_POINTER(field, HttpField);
            }

         ++j;
         }
      }

   U_RETURN_POINTER(U_NULLPTR, HttpField);
}

HttpField* HttpHeader::del(HttpField* field)
{
   U_TRACE(5, "HttpHeader::del(%p)", field)

   unsigned i;

   for (i = 0; i < header.size(); ++i)
      {
      if (field == header[i])
         {
         U_VEC_ERASE1(header, i);

         U_RETURN_POINTER(field, HttpField);
         }
      }

   U_RETURN_POINTER(U_NULLPTR, HttpField);
}

void HttpHeader::stringify(UString& field)
{
   U_TRACE(256+5, "HttpHeader::stringify(%.*S)", U_STRING_TO_TRACE(field))

   HttpField* f;

   for (unsigned i = 0; i < header.size(); ++i)
      {
      f = header[i];

      U_INTERNAL_DUMP("f = %p", f)

      f->stringify(field);
      }

   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

void HttpHeader::clear()
{
   U_TRACE(5, "HttpHeader::clear()")

   HttpField* f;

   for (unsigned i = 0; i < header.size(); ++i)
      {
      f = header[i];

      U_INTERNAL_DUMP("f = %p", f)

      U_DELETE(f)
      }
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpHeader::dump(bool reset) const
{
   *UObjectIO::os << "header    (UVector " << (void*)&header << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
