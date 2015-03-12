// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    hash_map.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/container/vector.h>
#include <ulib/container/hash_map.h>

bool        UHashMap<void*>::istream_loading;
uPFpcu      UHashMap<void*>::gperf;
UStringRep* UHashMap<void*>::pkey;

UHashMap<void*>::UHashMap(uint32_t n, bool _ignore_case)
{
   U_TRACE_REGISTER_OBJECT(0, UHashMap<void*>, "%u,%b", n, _ignore_case)

   node        = 0;
   _length     = _space = index = hash = 0;
   ignore_case = _ignore_case;

   _allocate(n);
}

void UHashMap<void*>::allocate(uint32_t n)
{
   U_TRACE(0, "UHashMap<void*>::allocate(%u)", n)

   U_CHECK_MEMORY

   if (_capacity) _deallocate();

   table     = (UHashMapNode**) UMemoryPool::_malloc(&n, sizeof(UHashMapNode*), true);
   _capacity = n;
}

void UHashMap<void*>::lookup(const UStringRep* keyr)
{
   U_TRACE(0, "UHashMap<void*>::lookup(%.*S)", U_STRING_TO_TRACE(*keyr))

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

   if (gperf) index = gperf(U_STRING_TO_PARAM(*keyr));
   else
      {
      hash  = keyr->hash(ignore_case);
      index = hash % _capacity;
      }

   U_INTERNAL_DUMP("index = %u", index)

   U_INTERNAL_ASSERT_MINOR(index,_capacity)

   for (node = table[index]; node; node = node->next)
      {
      if (node->key->equal(keyr, ignore_case)) break;
      }

   U_INTERNAL_DUMP("node = %p", node)
}

void* UHashMap<void*>::erase(const UStringRep* _key)
{
   U_TRACE(0, "UHashMap<void*>::erase(%.*S)", U_STRING_TO_TRACE(*_key))

   lookup(_key);

   if (node)
      {
      const void* _elem = node->elem;

      eraseAfterFind();

      U_RETURN((void*)_elem);
      }

   U_RETURN((void*)0);
}

// OPERATOR []

void* UHashMap<void*>::at(const UStringRep* _key)
{
   U_TRACE(0, "UHashMap<void*>::at(%.*S)", U_STRING_TO_TRACE(*_key))

   lookup(_key);

   if (node) U_RETURN((void*)node->elem);

   U_RETURN((void*)0);
}

void* UHashMap<void*>::at(const char* _key, uint32_t keylen)
{
   U_TRACE(0, "UHashMap<void*>::at(%.*S,%u)", keylen, _key, keylen) // problem with sanitize address

   U_INTERNAL_ASSERT_POINTER(pkey)
   U_INTERNAL_ASSERT_POINTER(_key)
   U_INTERNAL_ASSERT_MAJOR(keylen, 0)

   pkey->str     = _key;
   pkey->_length = keylen;

   return at(pkey);
}

void UHashMap<void*>::insertAfterFind(const UStringRep* _key, const void* _elem)
{
   U_TRACE(0, "UHashMap<void*>::insertAfterFind(%.*S,%p)", U_STRING_TO_TRACE(*_key), _elem)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(node,0)

   /**
    * list self-organizing (move-to-front), we place before
    * the element at the beginning of the list of collisions
    */

   node = table[index] = U_NEW(UHashMapNode(_key, _elem, table[index], hash));

   ++_length;

   U_INTERNAL_DUMP("_length = %u", _length)
}

void UHashMap<void*>::_eraseAfterFind()
{
   U_TRACE(0, "UHashMap<void*>::_eraseAfterFind()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("node = %p", node)

   UHashMapNode* prev = 0;

   for (UHashMapNode* pnode = table[index]; pnode; pnode = pnode->next)
      {
      if (pnode == node)
         {
         /**
          * list self-organizing (move-to-front), we place before
          * the element at the beginning of the list of collisions
          */

         if (prev)
            {
            prev->next   = pnode->next;
            pnode->next  = table[index];
            table[index] = pnode;
            }

         U_INTERNAL_ASSERT_EQUALS(node,table[index])

         break;
         }

      prev = pnode;
      }

   U_INTERNAL_DUMP("prev = %p", prev)

   /**
    * list self-organizing (move-to-front), we requires the
    * item to be deleted at the beginning of the list of collisions
    */

   U_INTERNAL_ASSERT_EQUALS(node, table[index])

   table[index] = node->next;
}

void UHashMap<void*>::eraseAfterFind()
{
   U_TRACE(0, "UHashMap<void*>::eraseAfterFind()")

   _eraseAfterFind();

   delete node;

   --_length;

   U_INTERNAL_DUMP("_length = %u", _length)
}

void UHashMap<void*>::replaceKey(const UString& _key)
{
   U_TRACE(0, "UHashMap<void*>::replaceKey(%.*S)", U_STRING_TO_TRACE(_key))

   UHashMapNode* pnode = node;

   _eraseAfterFind();

   lookup(_key);

   U_INTERNAL_ASSERT_EQUALS(node,0)

   pnode->hash = hash;
   pnode->next = table[index];

   ((UStringRep*)pnode->key)->release(); // NB: we decreases the reference string...

   pnode->key = _key.rep;

   ((UStringRep*)pnode->key)->hold();    // NB: we increases the reference string...

   /**
    * list self-organizing (move-to-front), we place before
    * the element at the beginning of the list of collisions
    */

   node = table[index] = pnode;
}

void UHashMap<void*>::reserve(uint32_t n)
{
   U_TRACE(0, "UHashMap<void*>::reserve(%u)", n)

   U_INTERNAL_ASSERT_EQUALS(gperf,0)
   U_INTERNAL_ASSERT_MAJOR(_capacity, 1)

   uint32_t new_capacity = U_GET_NEXT_PRIME_NUMBER(n);

   if (new_capacity == _capacity) return;

   UHashMapNode** old_table    = table;
   uint32_t       old_capacity = _capacity, i;

   _allocate(new_capacity);

#ifdef DEBUG
   int sum = 0, max = 0, min = 1024, width;
#endif

   // inserisco i vecchi elementi

   UHashMapNode* _next;

   for (i = 0; i < old_capacity; ++i)
      {
      if (old_table[i])
         {
         node = old_table[i];

#     ifdef DEBUG
         ++sum;
         width = -1;
#     endif

         do {
#        ifdef DEBUG
            ++width;
#        endif

            _next  = node->next;
            index  = node->hash % _capacity;

            U_INTERNAL_DUMP("i = %u index = %u hash = %u", i, index, node->hash)

            /**
             * list self-organizing (move-to-front), we place before
             * the element at the beginning of the list of collisions
             */

            node->next   = table[index];
            table[index] = node;
            }
         while ((node = _next));

#     ifdef DEBUG
         if (max < width) max = width;
         if (min > width) min = width;
#     endif
         }
      }

   UMemoryPool::_free(old_table, old_capacity, sizeof(UHashMapNode*));

   U_INTERNAL_DUMP("OLD: collision(min,max) = (%3d,%3d) - distribution = %3f", min, max, (sum ? (double)_length / (double)sum : 0))

#ifdef DEBUG
   sum = 0, max = 0, min = 1024;

   UHashMapNode* _n;

   for (i = 0; i < _capacity; ++i)
      {
      if (table[i])
         {
         _n = table[i];

         ++sum;
         width = -1;

         do {
            ++width;

            _next = _n->next;
            }
         while ((_n = _next));

         if (max < width) max = width;
         if (min > width) min = width;
         }
      }
#endif

   U_INTERNAL_DUMP("NEW: collision(min,max) = (%3d,%3d) - distribution = %3f", min, max, (sum ? (double)_length / (double)sum : 0))
}

bool UHashMap<void*>::first()
{
   U_TRACE(0, "UHashMap<void*>::first()")

   U_INTERNAL_DUMP("_length = %u", _length)

   for (index = 0; index < _capacity; ++index)
      {
      if (table[index])
         {
         node = table[index];

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

bool UHashMap<void*>::next()
{
   U_TRACE(0, "UHashMap<void*>::next()")

   U_INTERNAL_DUMP("index = %u node = %p next = %p", index, node, node->next)

   if ((node = node->next)) U_RETURN(true);

   for (++index; index < _capacity; ++index)
      {
      if (table[index])
         {
         node = table[index];

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

void UHashMap<void*>::callForAllEntry(bPFprpv function)
{
   U_TRACE(0, "UHashMap<void*>::callForAllEntry(%p)", function)

#ifdef DEBUG
   int sum = 0, max = 0, min = 1024, width;
#endif

   U_INTERNAL_DUMP("_length = %u", _length)

   UHashMapNode* _node;
   UHashMapNode* _next;
   UHashMapNode** ptr;
   UHashMapNode** end;

   for (end = (ptr = table) + _capacity; ptr < end; ++ptr)
      {
      if (*ptr)
         {
         _node = *ptr;

#     ifdef DEBUG
         ++sum;
         width = -1;
#     endif

         do {
#        ifdef DEBUG
            ++width;
#        endif

            _next = _node->next;

            if (function((UStringRep*)_node->key, (void*)_node->elem) == false) return;
            }
         while ((_node = _next));

#     ifdef DEBUG
         if (max < width) max = width;
         if (min > width) min = width;
#     endif
         }
      }

   U_INTERNAL_DUMP("collision(min,max) = (%3d,%3d) - distribution = %3f", min, max, (sum ? (double)_length / (double)sum : 0))
}

void UHashMap<void*>::getKeys(UVector<UString>& vec)
{
   U_TRACE(0, "UHashMap<void*>::getKeys(%p)", &vec)

   UHashMapNode* _node;
   UHashMapNode* _next;
   UHashMapNode** ptr;
   UHashMapNode** end;

   for (end = (ptr = table) + _capacity; ptr < end; ++ptr)
      {
      if (*ptr)
         {
         _node = *ptr;

         do {
            vec.UVector<UStringRep*>::push(_node->key);

            _next = _node->next;
            }
         while ((_node = _next));
         }
      }
}

void UHashMap<void*>::_callForAllEntrySorted(bPFprpv function)
{
   U_TRACE(0, "UHashMap<void*>::_callForAllEntrySorted(%p)", function)

   U_INTERNAL_ASSERT_MAJOR(_length, 1)

   UVector<UString> vkey(_length);

   getKeys(vkey);

   U_ASSERT_EQUALS(_length, vkey.size())

   vkey.sort(ignore_case);

   for (uint32_t i = 0, n = _length; i < n; ++i)
      {
      UStringRep* r = vkey.UVector<UStringRep*>::at(i);

      lookup(r);

      U_INTERNAL_ASSERT_POINTER(node)

      if (function(r, (void*)node->elem) == false) return;
      }
}

// specializzazione stringa

void UHashMap<UString>::insertAfterFind(const UString& _key, const UString& str)
{
   U_TRACE(0, "UHashMap<UString>::insertAfterFind(%.*S,%.*S)", U_STRING_TO_TRACE(_key), U_STRING_TO_TRACE(str))

   UHashMap<UStringRep*>::insertAfterFind(_key, str.rep);
}

void UHashMap<UString>::insertAfterFind(const char* _key, uint32_t keylen, const UString& str)
{
   U_TRACE(0, "UHashMap<UString>::insertAfterFind(%.*S,%u,%.*S)", keylen, _key, keylen, U_STRING_TO_TRACE(str))

   U_INTERNAL_ASSERT_POINTER(pkey)

   pkey->str     = _key;
   pkey->_length = keylen;

   UHashMap<UStringRep*>::insertAfterFind(pkey, str.rep);
}

UString UHashMap<UString>::erase(const UString& _key)
{
   U_TRACE(0, "UHashMap<UString>::erase(%.*S)", U_STRING_TO_TRACE(_key))

   UHashMap<void*>::lookup(_key);

   if (node)
      {
      UString str(elem());

      U_INTERNAL_DUMP("str.reference() = %u", str.reference())

      U_INTERNAL_ASSERT_MAJOR(str.reference(), 0)

      eraseAfterFind();

      U_INTERNAL_DUMP("str.reference() = %u", str.reference())

      U_RETURN_STRING(str);
      }

   U_RETURN_STRING(UString::getStringNull());
}

// OPERATOR []

UString UHashMap<UString>::at(const UStringRep* _key)
{
   U_TRACE(0, "UHashMap<UString>::at(%.*S)", U_STRING_TO_TRACE(*_key))

   UHashMap<void*>::lookup(_key);

   if (node)
      {
      UString str(elem());

      U_RETURN_STRING(str);
      }

   U_RETURN_STRING(UString::getStringNull());
}

void* UHashMap<void*>::operator[](const char* _key)
{
   U_TRACE(0, "UHashMap<void*>::operator[](%S)", _key)

   U_INTERNAL_ASSERT_POINTER(pkey)

   pkey->str     =           _key;
   pkey->_length = u__strlen(_key, __PRETTY_FUNCTION__);

   return at(pkey);
}

UString UHashMap<UString>::operator[](const char* _key)
{
   U_TRACE(0, "UHashMap<UString>::operator[](%S)", _key)

   U_INTERNAL_ASSERT_POINTER(pkey)

   pkey->str     =           _key;
   pkey->_length = u__strlen(_key, __PRETTY_FUNCTION__);

   return at(pkey);
}

void* UHashMap<void*>::erase(const char* _key)
{
   U_TRACE(0, "UHashMap<void*>::erase(%S)", _key)

   U_INTERNAL_ASSERT_POINTER(pkey)

   pkey->str     =           _key;
   pkey->_length = u__strlen(_key, __PRETTY_FUNCTION__);

   return erase(pkey);
}

UString UHashMap<UString>::at(const char* _key, uint32_t keylen)
{
   U_TRACE(0, "UHashMap<UString>::at(%.*S,%u)", keylen, _key, keylen)

   U_INTERNAL_ASSERT_POINTER(pkey)

   pkey->str     = _key;
   pkey->_length =  keylen;

   return at(pkey);
}

bool UHashMap<void*>::find(const char* _key, uint32_t keylen)
{
   U_TRACE(0, "UHashMap<void*>::find(%.*S,%u)", keylen, _key, keylen)

   U_INTERNAL_ASSERT_POINTER(pkey)

   pkey->str     = _key;
   pkey->_length =  keylen;

   lookup(pkey);

   U_RETURN(node != 0);
}

uint32_t UHashMap<UString>::loadFromData(const char* ptr, uint32_t sz)
{
   U_TRACE(0+256, "UHashMap<UString>::loadFromData(%.*S,%u)", sz, ptr, sz)

   U_INTERNAL_ASSERT_MAJOR(sz, 0)
   U_INTERNAL_ASSERT_MAJOR(_capacity, 1)

   const char* _end   = ptr + sz;
   const char* _start = ptr;

   // NB: we need this way for plugin...

   char terminator = 0, c = *ptr;

   if (c == '{' ||
       c == '[')
      {
      ++ptr; // skip '{' or '['

      terminator = (c == '{' ? '}' : ']');
      }

   U_INTERNAL_DUMP("terminator = %C", terminator)

   while (ptr < _end)
      {
   // U_INTERNAL_DUMP("ptr = %.*S", 20, ptr)

      c = *ptr++;

      if (u__isspace(c)) continue; // skip white-space

   // U_INTERNAL_DUMP("c = %C", c)

      if ( terminator == c ||
          (terminator == 0 &&
           (c == '}' || c == ']')))
         {
         break;
         }

      if (c == '#')
         {
         do { c = *ptr++; } while (c != '\n' && ptr < _end); // skip line comment

         continue;
         }

      U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

      UString _key(U_CAPACITY);

   // U_INTERNAL_DUMP("c = %C", c)

      if (c == '"') _key.setFromData(&ptr, sz, '"');
      else
         {
         --ptr;

         _key.setFromData(&ptr, sz);
         }

      U_INTERNAL_ASSERT(_key)
      U_INTERNAL_ASSERT(_key.isNullTerminated())

      do { c = *ptr++; } while (u__isspace(c) && ptr < _end); // skip white-space

   // U_INTERNAL_DUMP("c = %C", c)

      if (ptr >= _end) break;

      U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

      UString str(U_CAPACITY);

   // U_INTERNAL_DUMP("c = %C", c)

      if (c == '"') str.setFromData(&ptr, sz, '"');
      else
         {
         --ptr;

         str.setFromData(&ptr, sz);
         }

      U_INTERNAL_ASSERT(str)
      U_INTERNAL_ASSERT(str.isNullTerminated())

      insert(_key, str);
      }

   U_INTERNAL_DUMP("ptr-_start = %lu", ptr-_start)

   U_INTERNAL_ASSERT((ptr-_start) <= sz)

   sz = ptr - _start;

   U_RETURN(sz);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UHashMap<UString>& t)
{
   U_TRACE(0+256, "UHashMap<UString>::operator>>(%p,%p)", &is, &t) // problem with sanitize address

   U_INTERNAL_ASSERT_MAJOR(t._capacity, 1)

   int c = EOF;

   if (is.good())
      {
      streambuf* sb = is.rdbuf();

      // NB: we need this way for plugin...

      int terminator = EOF;

      if (is.peek() == '{' ||
          is.peek() == '[')
         {
         c = sb->sbumpc(); // skip '{' or '['

         terminator = (c == '{' ? '}' : ']');
         }

      do {
         do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

      // U_INTERNAL_DUMP("c = %C", c)

         if ( EOF        == c   ||
              terminator == c   ||
             (terminator == EOF &&
              (c == '}' || c == ']')))
            {
            break;
            }

         if (c == '#')
            {
            do { c = sb->sbumpc(); } while (c != '\n' && c != EOF); // skip line comment

            continue;
            }

         U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

         sb->sputbackc(c);

         UString key(U_CAPACITY);

         key.get(is);

         U_INTERNAL_ASSERT(key)
         U_INTERNAL_ASSERT(key.isNullTerminated())

         do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

      // U_INTERNAL_DUMP("c = %C", c)

         if (c == EOF) break;

         U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

         sb->sputbackc(c);

         UString str(U_CAPACITY);

         str.get(is);

         U_INTERNAL_ASSERT(str)
         U_INTERNAL_ASSERT(str.isNullTerminated())

         t.insert(key, str);
         }
      while (c != EOF);
      }

   if (c == EOF)       is.setstate(ios::eofbit);
   // if (t._length == 0) is.setstate(ios::failbit);

   return is;
}

// DEBUG

#  ifdef DEBUG
const char* UHashMapNode::dump(bool reset) const
{
   *UObjectIO::os << "elem               " << elem        << '\n'
                  << "hash               " << hash        << '\n'
                  << "key  (UStringRep   " << (void*)key  << ")\n"
                  << "next (UHashMapNode " << (void*)next << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UHashMap<void*>::dump(bool reset) const
{
   *UObjectIO::os << "hash               " << hash         << '\n'
                  << "index              " << index        << '\n'
                  << "table              " << (void*)table << '\n'
                  << "_length            " << _length      << "\n"
                  << "_capacity          " << _capacity    << '\n'
                  << "ignore_case        " << ignore_case  << '\n'
                  << "node (UHashMapNode " << (void*)node  << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
