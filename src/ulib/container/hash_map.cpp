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

#include <ulib/container/hash_map.h>

bool              UHashMap<void*>::istream_loading;
uint8_t           UHashMap<void*>::linfo;
uint32_t          UHashMap<void*>::lhash;
uint32_t          UHashMap<void*>::index;
const void*       UHashMap<void*>::lelem;
UHashMapNode*     UHashMap<void*>::node;
const UStringRep* UHashMap<void*>::lkey;
UVector<UString>* UHashMap<void*>::pvec;

void UHashMap<void*>::_allocate(uint32_t n)
{
   U_TRACE(0, "UHashMap<void*>::_allocate(%u)", n)

   U_CHECK_MEMORY

   // must be a power of 2, It's done this way because bitwise-and is an inexpensive operation, whereas integer modulo (%) is quite heavy

   U_INTERNAL_ASSERT_EQUALS(n & (n-1), 0)

   info  = (uint8_t*) UMemoryPool::_malloc(n, 1+UHashMapNode::size(), false);
   table = (char*) (info + n);

   (void) U_SYSCALL(memset, "%p,%d,%u", info, 0, n);

   mask = (_capacity = n) - 1;

   U_INTERNAL_ASSERT_EQUALS(_capacity & mask, 0)

   max_num_num_elements_allowed = U_min(_capacity-2, (uint32_t)(_capacity * U_MAX_LOAD_FACTOR)); // max * (1 - 1/20) = max * 0.95

   U_INTERNAL_DUMP("_capacity = %u mask = %u max_num_num_elements_allowed = %u", _capacity, mask, max_num_num_elements_allowed)
}

bool UHashMap<void*>::lookup()
{
   U_TRACE(0, "UHashMap<void*>::lookup()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(set_index)
   U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

   bool ignore_case = set_index(this);

   U_INTERNAL_DUMP("lhash = %u index = %u ignore_case = %b", lhash, index, ignore_case)

   U_INTERNAL_ASSERT_MINOR(index, _capacity)

   /**
    * Robin Hood Hashing uses the hash value to calculate the position to place it, than does linear probing until it finds an empty spot to place it.
    * While doing so it swaps out entries that have a lesser distance to its original bucket. This minimizes the maximum time a lookup takes. For a lookup,
    * it is only necessary to lineary probe until the distance to the original bucket is larger than the current element’s distance.
    *
    * In this “Infobyte” implementation variant, instead of storing the full 64 bit hash value I directly store the distance to the original bucket in a byte.
    * One bit of this byte is used to mark the bucket as taken or empty. The bit layout is defined like this:
    *
    * bits: | 7 | 6   5   4   3   2   1   0|
    *       | ^ |         offset           |
    *         |
    *       full?
    */

   linfo = U_IS_BUCKET_TAKEN_MASK;

   while (linfo < info[index])
      {
      U_INTERNAL_DUMP("linfo = %u info[%u] = %u", linfo, index, info[index])

      index = (index+1) & mask;

      ++linfo;
      }

   // check while info matches with the source index

   while (linfo == info[index])
      {
      setNodePointer();

      if (UStringRep::equal_lookup(lkey, node->key, ignore_case)) U_RETURN(true);

      index = (index+1) & mask;

      ++linfo;
      }

   U_INTERNAL_DUMP("info[%u] = %u linfo = %u", index, info[index], linfo)

   // nothing found!

   U_RETURN(false);
}

#define U_HASHMAP_SAVE_CONTEXT \
 uint8_t           _tmp1 = linfo; \
 uint32_t          _tmp2 = index; \
 uint32_t          _tmp3 = lhash; \
 const void*       _tmp4 = lelem; \
 const UStringRep* _tmp5 = lkey

#define U_HASHMAP_RESTORE_CONTEXT \
 linfo = _tmp1; \
 index = _tmp2; \
 lhash = _tmp3; \
 lelem = _tmp4; \
 lkey  = _tmp5

#ifdef DEBUG
bool UHashMap<void*>::checkAt(const UStringRep* k, const void* e)
{
   U_TRACE(0, "UHashMap<void*>::checkAt(%V,%p)", k, e)

   U_HASHMAP_SAVE_CONTEXT;

   bool result = (at(k) == e);

   U_HASHMAP_RESTORE_CONTEXT;

   U_RETURN(result);
}

bool UHashMap<void*>::invariant()
{
   U_TRACE_NO_PARAM(0, "UHashMap<void*>::invariant()")

   for (uint32_t idx = 0; idx < _capacity; ++idx)
      {
      if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
         {
         setNodePointer(idx);

         if (checkAt(node->key, node->elem) == false) U_RETURN(false);
         }
      }

   U_RETURN(true);
}
#endif

#define U_HASHMAP_SWAP_NODE(e,k,h) \
 const void* _tmp1 = node->elem; \
 node->elem = e; \
 e = _tmp1; \
 const UStringRep* _tmp2 = node->key; \
 node->key = k; \
 k = _tmp2; \
 uint32_t _tmp3 = node->hash; \
 node->hash = h; \
 h = _tmp3

void UHashMap<void*>::swapNodeInResize()
{
   U_TRACE_NO_PARAM(0, "UHashMap<void*>::swapNodeInResize()")

   swapInfo();

   UHashMapNode* pnode = node;

   U_INTERNAL_ASSERT_POINTER(pnode)

   setNodePointer();

   U_HASHMAP_SWAP_NODE(pnode->elem, pnode->key, pnode->hash);

   U_ASSERT(checkAt(node->key, node->elem))

   node = pnode;
}

void UHashMap<void*>::increase_size()
{
   U_TRACE(0, "UHashMap<void*>::increase_size()")

   char*    old_table    = table;
   uint8_t* old_info     = info;
   uint32_t old_capacity = _capacity;

   U_INTERNAL_ASSERT_MAJOR(_capacity, 1)

   _allocate(_capacity << 1); // x 2...

   // we insert the old elements

   for (uint32_t idx = 0; idx < old_capacity; ++idx)
      {
      if ((old_info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0) // inserts a keyval that is guaranteed to be new, e.g. when the hashmap is resized
         {
         setNodePointer(old_table, idx);

         index = node->hash & mask;

         linfo = U_IS_BUCKET_TAKEN_MASK;

         while (linfo <= info[index]) // skip forward. Use <= because we know the element is not there
            {
            U_INTERNAL_DUMP("linfo = %u info[%u] = %u", linfo, index, info[index])

            index = (index+1) & mask;

            ++linfo;
            }

         // loop while we have not found an empty spot

         while ((info[index] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            U_INTERNAL_ASSERT_DIFFERS(linfo, 0) // Overflow! This is bad, shouldn't happen

            if (linfo > info[index]) swapNodeInResize();

            index = (index+1) & mask;

            ++linfo;
            }

         // bucket is empty! put it there

         lelem = node->elem;
         lkey  = node->key;
         lhash = node->hash;

         putNode();
         }
      }

   UMemoryPool::_free(old_info, old_capacity, 1+UHashMapNode::size());
}

void UHashMap<void*>::swapNode()
{
   U_TRACE_NO_PARAM(0, "UHashMap<void*>::swapNode()")

   swapInfo();

   setNodePointer();

   U_INTERNAL_DUMP("lkey = %V lelem = %p lhash = %u node->key = %V node->elem = %p node->hash = %u", lkey, lelem, lhash, node->key, node->elem, node->hash)

   U_HASHMAP_SWAP_NODE(lelem, lkey, lhash);

   U_INTERNAL_DUMP("lkey = %V lelem = %p lhash = %u node->key = %V node->elem = %p node->hash = %u", lkey, lelem, lhash, node->key, node->elem, node->hash)

   U_ASSERT(checkAt(node->key, node->elem))
}

void UHashMap<void*>::insertAfterFind()
{
   U_TRACE_NO_PARAM(0, "UHashMap<void*>::insertAfterFind()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u max_num_num_elements_allowed = %u", _length, max_num_num_elements_allowed)

   U_INTERNAL_ASSERT_MINOR(_length, max_num_num_elements_allowed)

   U_INTERNAL_DUMP("info[%u] = %u linfo = %u", index, info[index], linfo)

   /**
    * Robin Hood Hashing uses the hash value to calculate the position to place it, than does linear probing until it finds an empty spot to place it.
    * While doing so it swaps out entries that have a lesser distance to its original bucket. This minimizes the maximum time a lookup takes. For a lookup,
    * it is only necessary to lineary probe until the distance to the original bucket is larger than the current element’s distance
    */

loop:
   while (info[index] >= U_IS_BUCKET_TAKEN_MASK && linfo) // loop while we have not found an empty spot, and while no info overflow
      {
      U_INTERNAL_DUMP("linfo = %u info[%u] = %u", linfo, index, info[index])

      if (linfo > info[index]) swapNode();

      index = (index+1) & mask;

      ++linfo;
      }

   if (linfo) putNode(); // bucket is empty! put it there
   else
      {
      // Overflow: resize and try again

      const void*       tmp1 = lelem;
      const UStringRep* tmp2 = lkey;

      increase_size();

      lelem = tmp1;
      lkey  = tmp2;

      (void) lookup();

      goto loop;
      }

   if (++_length == max_num_num_elements_allowed) increase_size();
}

void UHashMap<void*>::eraseAfterFind()
{
   U_TRACE_NO_PARAM(0, "UHashMap<void*>::eraseAfterFind()")

   U_CHECK_MEMORY

   node->reset();

   // perform backward shift deletion: shift elements to the left until we find one that is either empty or has zero offset
   //
   // NB: no need to check for last element, this acts as a sentinel

   uint32_t nextIdx = (index+1) & mask;

   U_INTERNAL_DUMP("index = %u nextIdx = %u", index, nextIdx)

   while (info[nextIdx] > U_IS_BUCKET_TAKEN_MASK)
      {
      U_INTERNAL_DUMP("info[%u] = %u", nextIdx, info[nextIdx])

      info[index] = info[nextIdx]-1;

      U_INTERNAL_DUMP("index = %u nextIdx = %u", index, nextIdx)

      U_INTERNAL_ASSERT_DIFFERS(index, nextIdx)

      setNodePointer(nextIdx);

      U_INTERNAL_DUMP("node->key = %V node->elem = %p node->hash = %u", node->key, node->elem, node->hash)

      lelem = node->elem;
      lkey  = node->key;
      lhash = node->hash;

      setNodePointer();

      U_INTERNAL_DUMP("node->key = %V node->elem = %p node->hash = %u", node->key, node->elem, node->hash)

      node->elem = lelem;
      node->key  = lkey;
      node->hash = lhash;

        index = nextIdx;
      nextIdx = (index+1) & mask;
      }

   if (info[index]) info[index] = 0;

   --_length;

   U_INTERNAL_DUMP("_length = %u", _length)
}

__pure bool UHashMap<UString>::operator==(const UHashMap<UString>& t)
{
   U_TRACE(0, "UHashMap<UString>::operator==(%p)", &t)

   U_CHECK_MEMORY

   if (_length == t._length)
      {
      U_INTERNAL_DUMP("_length = %u", _length)

      for (uint32_t idx = 0; idx < t._capacity; ++idx)
         {
         if ((t.info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            t.setNodePointer(idx);

            if (UHashMap<void*>::lookup((UStringRep*)t.node->key) == false ||
                ((UStringRep*)t.node->elem)->equal(elem()) == false)
               {
               U_RETURN(false);
               }
            }
         }

      U_RETURN(true);
      }

   U_RETURN(false);
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

      if (c == '"')
         {
         // NB: check if we have a string null...

         if (*ptr != '"') _key.setFromData(&ptr, _end - ptr, '"');
         else
            {
            ++ptr;

            _key.clear();
            }
         }
      else
         {
         --ptr;

         _key.setFromData(&ptr, _end - ptr, terminator);
         }

      U_INTERNAL_ASSERT(_key)
      U_INTERNAL_ASSERT(_key.isNullTerminated())

      do { c = *ptr++; } while (u__isspace(c) && ptr < _end); // skip white-space

   // U_INTERNAL_DUMP("c = %C", c)

      if (ptr >= _end) break;

      U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

      UString str(U_CAPACITY);

   // U_INTERNAL_DUMP("c = %C", c)

      if (c == '"')
         {
         // NB: check if we have a string null...

         if (*ptr != '"') str.setFromData(&ptr, _end - ptr, '"');
         else
            {
            ++ptr;

            str.clear();
            }
         }
      else
         {
         --ptr;

         str.setFromData(&ptr, _end - ptr, terminator);
         }

      if (str)
         {
         U_INTERNAL_ASSERT(str.isNullTerminated())

         insert(_key, str);

         continue;
         }

      U_WARNING("UHashMap<UString>::loadFromData() has found a key(%u) = %V without value", _key.size(), _key.rep);
      }

   U_INTERNAL_ASSERT((uint32_t)(ptr-_start) <= sz)

   sz = ptr-_start;

   U_INTERNAL_DUMP("ptr-_start = %u", sz)

   U_RETURN(sz);
}

__pure uint32_t UHashMap<UString>::getSpaceToDump() const
{
   U_TRACE_NO_PARAM(0+256, "UHashMap<UString>::getSpaceToDump()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   uint32_t space = U_CONSTANT_SIZE("[\n]");

   if (_length)
      {
      uint32_t sz;

      for (uint32_t idx = 0; idx < _capacity; ++idx)
         {
         if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) == 0) continue;

         setNodePointer(idx);

         sz = node->key->size();

         U_INTERNAL_DUMP("node->key(%u) = %p %V", sz, node->key, node->key)

         U_INTERNAL_ASSERT_MAJOR(sz, 0)

         space += sz + 1 + ((UStringRep*)node->elem)->getSpaceToDump() + 1;
         }
      }

   U_RETURN(space);
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
const char* UHashMap<void*>::dump(bool reset) const
{
   *UObjectIO::os << "mask                         " << mask          << '\n'
                  << "info                         " << (void*)info   << '\n'
                  << "lhash                        " << lhash         << '\n'
                  << "linfo                        " << linfo         << '\n'
                  << "table                        " << (void*)table  << '\n'
                  << "index                        " << index         << '\n'
                  << "_length                      " << _length       << "\n"
                  << "_capacity                    " << _capacity     << '\n'
                  << "max_num_num_elements_allowed " << max_num_num_elements_allowed;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#  endif
#endif
