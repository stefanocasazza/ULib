// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    gen_hash_map.h - general purpose templated hash table class
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_GENERIC_HASH_MAP_H
#define ULIB_GENERIC_HASH_MAP_H 1

#include <ulib/internal/common.h>

/**
 * Functor used by UGenericHashMap class to generate a hashcode for an object of type T. It must be specialized for your own class
 */

template <typename T> struct UHashCodeFunctor;

/**
 * Functor used by UGenericHashMap class to compare for equality two objects of type T
 * It can be specialized for your own class, by default it simply uses operator==()
 */

template <typename T> struct UEqualsFunctor { bool operator()(const T& a, const T& b) const { return (a == b); } };

/**
 * UGenericHashMap is a general purpose templated hash table class
 */

template <typename K, typename I,
          typename H = UHashCodeFunctor<K>, typename E = UEqualsFunctor<K> > class UGenericHashMap {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   struct UGenericHashMapNode // structure for keeping a linked-list of elements
      {
      K key;
      I item;
      uint32_t hash;
      UGenericHashMapNode* next;

      UGenericHashMapNode(const K& _key, const I& _item, UGenericHashMapNode* _next, uint32_t _hash)
         {
         key  = _key;
         item = _item;
         hash = _hash;
         next = _next;
         }
      };

   E equals;
   H hashcode;
   UGenericHashMapNode* node;
   UGenericHashMapNode** table;
   uint32_t _length, _capacity, index, hash;

protected:
   // Find a elem in the array with <key>

   template <typename X> void lookup(const X& _key)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::lookup(%p)", &_key)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

      hash  = hashcode(_key);
      index = hash % _capacity;

      U_INTERNAL_DUMP("index = %u", index)

      U_INTERNAL_ASSERT_MINOR(index,_capacity)

      for (node = table[index]; node; node = node->next)
         {
         if (node->key == _key) break;
         }

      U_INTERNAL_DUMP("node = %p", node)
      }

public:
   UGenericHashMap()
      {
      U_TRACE_REGISTER_OBJECT(0, UGenericHashMap, "", 0)

      node  = 0;
      table = 0;

      _length = _capacity = index = hash = 0;
      }

   ~UGenericHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UGenericHashMap)
      }

   // Allocate and deallocate methods

   void allocate(uint32_t n = 53)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::allocate(%u)", n)

      U_CHECK_MEMORY

      table     = (UGenericHashMapNode**) UMemoryPool::_malloc(&n, sizeof(UGenericHashMapNode*), true);
      _capacity = n;
      }

   void deallocate()
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::deallocate()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity,0)

      UMemoryPool::_free(table, _capacity, sizeof(UGenericHashMapNode*));

      _capacity = 0;
      }

   // Size and capacity

   uint32_t size() const
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::size()")

      U_RETURN(_length);
      }

   uint32_t capacity() const
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::capacity()")

      U_RETURN(_capacity);
      }

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::empty()")

      if (_length) U_RETURN(false);

      U_RETURN(true);
      }

   // Find

   template <typename X> bool find(const X& _key)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::find(%p)", &_key)

      lookup(_key);

      if (node != 0) U_RETURN(true);

      U_RETURN(false);
      }

   // Set/get methods

   I& elem() { return node->item; }

   template <typename X> I& operator[](const X& _key)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::operator[](%p)", &_key)

      lookup(_key);

      U_INTERNAL_ASSERT_POINTER(node)

      return node->item;
      }

   // Explicit method to access the key portion of the current element. The key cannot be modified

   const K& key() const { return node->key; }

   void eraseAfterFind()
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::eraseAfterFind()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("node = %p", node)

      UGenericHashMapNode* prev = 0;

      for (UGenericHashMapNode* pnode = table[index]; pnode; pnode = pnode->next)
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

      delete node;

      --_length;

      U_INTERNAL_DUMP("_length = %u", _length)
      }

   template <typename X, typename Y> void insertAfterFind(const X& _key, const Y& _elem)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::insertAfterFind(%p,%p)", &_key, &_elem)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(node, 0)

      /**
       * list self-organizing (move-to-front), we place before
       * the element at the beginning of the list of collisions
       */

      U_NEW(UGenericHashMapNode, table[index], UGenericHashMapNode(_key, _elem, table[index], hash));

      node = table[index];

      ++_length;

      U_INTERNAL_DUMP("_length = %u", _length)
      }

   template <typename Y> void replaceAfterFind(const Y& _elem)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::replaceAfterFind(%p)", &_elem)

      node->item = _elem;
      }

   template <typename X, typename Y> void insert(const X& _key, const Y& _item)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::insert(%p,%p)", &_key, &_item)

      lookup(_key);

      insertAfterFind(_key, _item);
      }

   template <typename X> bool erase(const X& _key)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::erase(%p)", &_key)

      lookup(_key);

      if (node)
         {
         eraseAfterFind();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // Make room for a total of n element

   void reserve(uint32_t n)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::reserve(%u)", n)

      U_INTERNAL_ASSERT_EQUALS(_capacity,0)

      uint32_t new_capacity = U_GET_NEXT_PRIME_NUMBER(n);

      if (new_capacity == _capacity) return;

      UGenericHashMapNode** old_table = table;
      uint32_t           old_capacity = _capacity, i;

      allocate(new_capacity);

      // we insert the old elements

      UGenericHashMapNode* _next;

      for (i = 0; i < old_capacity; ++i)
         {
         if (old_table[i])
            {
            node = old_table[i];

            do {
               _next = node->next;
               index = node->hash % _capacity;

               U_INTERNAL_DUMP("i = %u index = %u hash = %u", i, index, node->hash)

               /**
                * list self-organizing (move-to-front), we place before
                * the element at the beginning of the list of collisions
                */

               node->next   = table[index];
               table[index] = node;
               }
            while ((node = _next));
            }
         }

      UMemoryPool::_free(old_table, old_capacity, sizeof(UGenericHashMapNode*));
      }

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::clear()")

      U_INTERNAL_DUMP("_length = %u", _length)

#  ifdef DEBUG
      int sum = 0, max = 0, min = 1024, width;
#  endif

      UGenericHashMapNode* _next;

      for (index = 0; index < _capacity; ++index)
         {
         if (table[index])
            {
            node = table[index];

#        ifdef DEBUG
            ++sum;
            width = -1;
#        endif

            do {
#           ifdef DEBUG
               ++width;
#           endif

               _next = node->next;

               delete node;
               }
            while ((node = _next));

#        ifdef DEBUG
            if (max < width) max = width;
            if (min > width) min = width;
#        endif

            table[index] = 0;
            }
         }

      U_INTERNAL_DUMP("collision(min,max) = (%d,%d) - distribution = %f", min, max, (sum ? (double)_length / (double)sum : 0))

      _length = 0;
      }

   // Traverse the hash table for all entry

   UGenericHashMapNode* first()
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::first()")

      U_INTERNAL_DUMP("_length = %u", _length)

      for (index = 0; index < _capacity; ++index)
         {
         if (table[index])
            {
            node = table[index];

            U_RETURN_POINTER(node, UGenericHashMapNode);
            }
         }

      U_RETURN_POINTER(0, UGenericHashMapNode);
      }

   bool next()
      {
      U_TRACE_NO_PARAM(0, "UGenericHashMap<K,I>::next()")

      U_INTERNAL_DUMP("index = %u node = %p next = %p", index, node, node->next)

      if ((node = node->next)) U_RETURN_POINTER(node, UGenericHashMapNode);

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

   // We need to pass the pointer because we can lost the internal pointer between the call...

   UGenericHashMapNode* next(UGenericHashMapNode* _node)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::next(%p)", _node)

      U_INTERNAL_DUMP("index = %u", index)

      if ((node = _node->next)) U_RETURN_POINTER(node, UGenericHashMapNode);

      for (++index; index < _capacity; ++index)
         {
         if (table[index])
            {
            node = table[index];

            U_RETURN_POINTER(node, UGenericHashMapNode);
            }
         }

      U_RETURN_POINTER(0, UGenericHashMapNode);
      }

   // Call function for all entry

   void callForAllEntry(bPFpvpv function)
      {
      U_TRACE(0, "UGenericHashMap<K,I>::callForAllEntry(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      UGenericHashMapNode* n;
      UGenericHashMapNode* _next;

      for (uint32_t i = 0; i < _capacity; ++i)
         {
         if (table[i])
            {
            n = table[i];

            do {
               _next = n->next; // NB: function can delete the node...

               if (function(&(n->key), &(n->elem)) == false) return;
               }
            while ((n = _next));
            }
         }
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const
      {
      *UObjectIO::os << "hash      " << hash         << '\n'
                     << "node      " << (void*)node  << '\n'
                     << "index     " << index        << '\n'
                     << "table     " << (void*)table << '\n'
                     << "_length   " << _length      << "\n"
                     << "_capacity " << _capacity;

      if (reset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return 0;
      }
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
                                                            UGenericHashMap(const UGenericHashMap&) = delete;
                                                            UGenericHashMap& operator=(const UGenericHashMap&) = delete;
   template<typename A, typename B, typename C, typename D> UGenericHashMap(const UGenericHashMap<A,B,C,D>&) = delete;
   template<typename A, typename B, typename C, typename D> UGenericHashMap& operator=(const UGenericHashMap<A,B,C,D>&) = delete;
#else
                                                            UGenericHashMap(const UGenericHashMap&)                     {}
                                                            UGenericHashMap& operator=(const UGenericHashMap&)          { return *this; }
   template<typename A, typename B, typename C, typename D> UGenericHashMap(const UGenericHashMap<A,B,C,D>&)            {}
   template<typename A, typename B, typename C, typename D> UGenericHashMap& operator=(const UGenericHashMap<A,B,C,D>&) { return *this; }
#endif
};

// Functor used by UGenericHashMap class to generate a hashcode for an object of type <int>

template <> struct UHashCodeFunctor<int> {
   uint32_t operator()(const int& value) const
      {
      // http://www.concentric.net/~Ttwang/tech/inthash.htm

      uint32_t key = value, c2 = 0x27d4eb2d; // a prime or an odd constant

      key = (key ^ 61) ^ (key >> 16);
      key += key << 3;
      key ^= key >> 4;
      key *= c2;
      key ^= key >> 15;

      return key;
      }
};

#endif
