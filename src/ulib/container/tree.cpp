// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tree.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/container/tree.h>

uint32_t UTree<void*>::size_allocate;

// NB: if we put as pure this function the test on tree fail...

void UTree<void*>::callForAllEntry(vPFpvpv function)
{
   U_TRACE(0, "UTree<void*>::callForAllEntry(%p)", function)

   U_CHECK_MEMORY

   UTree<void*>* p;

   if (_elem) function((void*)_elem, this);

   if (UVector<void*>::empty() == false)
      {
      for (const void** ptr = vec; ptr < (vec + _length); ++ptr)
         {
         p = (UTree<void*>*)(*ptr);

         p->callForAllEntry(function);
         }
      }
}

UString UTree<UString>::back() { return ((UTree<UString>*)UVector<void*>::back())->elem(); }

void UTree<UString>::insert(uint32_t pos, const UString& str) // add elem before pos
{
   U_TRACE(0, "UTree<UString>::insert(%u,%V)", pos, str.rep)

   UTree<UStringRep*>::insert(pos, str.rep);
}

// EXTENSION

__pure uint32_t UTree<UString>::find(const UString& str)
{
   U_TRACE(0, "UTree<UString>::find(%V)", str.rep)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

   for (uint32_t i = 0; i < _length; ++i)
      {
      if (str.equal(UTree<UStringRep*>::at(i))) U_RETURN(i);
      }

   U_RETURN(U_NOT_FOUND);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UTree<UString>& t)
{
   U_TRACE(0+256,"UTree<UString>::operator>>(%p,%p)", &is, &t)

   U_INTERNAL_ASSERT_EQUALS(is.peek(),'[')

   int c = EOF;

   if (is.good())
      {
      int level = 1;
      UTree<UString>* p = &t;
      UString root(U_CAPACITY);
      streambuf* sb = is.rdbuf();

      c = sb->sbumpc(); // skip '['

      while (c != EOF)
         {
         do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

      // U_INTERNAL_DUMP("c = %C", c)

         if (c == ']' || c == EOF) break;

         if (c == '#')
            {
            do { c = sb->sbumpc(); } while (c != '\n' && c != EOF); // skip line comment

            continue;
            }

         U_INTERNAL_ASSERT_EQUALS(u__isspace(c),false)

         sb->sputbackc(c);

         root.get(is);

         t.setRoot(root);

         do {
            do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

         // U_INTERNAL_DUMP("c = %C", c)

            if (c == EOF) break;

            if (c == '#')
               {
               do { c = sb->sbumpc(); } while (c != '\n' && c != EOF); // skip line comment

               continue;
               }

                 if (c == '[') ++level;
            else if (c == ']')
               {
               --level;

               p = p->parent();
               }
            else
               {
               U_INTERNAL_ASSERT_EQUALS(u__isspace(c),false)

               sb->sputbackc(c);

               UString str(U_CAPACITY);

               str.get(is);

               p = p->push(str);
               }
            }
         while (level > 0 && c != EOF);
         }
      }

   if (c == EOF)       is.setstate(ios::eofbit);
   if (t._length == 0) is.setstate(ios::failbit);

   return is;
}

// DEBUG

#  ifdef DEBUG
const char* UTree<void*>::dump(bool reset) const
{
   *UObjectIO::os << "vec       " << (void*)vec << '\n'
                  << "_elem     " << _elem      << '\n'
                  << "_parent   " << _parent    << '\n'
                  << "_length   " << _length    << '\n'
                  << "_capacity " << _capacity;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif

/*
void UTree<void*>::copy(void* e, void* pnode)
{
   U_TRACE(0, " UTree<void*>::copy(%p,%p)", e, pnode)

   uint32_t d = ((UTree<void*>*)pnode)->depth();

   if (d != depth)
      {
      if (d > depth)
         {
         ++depth;

         ptree = tmp;

         U_INTERNAL_ASSERT_EQUALS(d,depth)
         }
      else
         {
         U_INTERNAL_ASSERT_MINOR(d,depth)

         do {
            ptree = ptree->parent();
            }
         while (d < --depth);

         U_INTERNAL_ASSERT_EQUALS(d,depth)
         }
      }

   tmp = ptree->push_back((Element*)e);
}
*/
