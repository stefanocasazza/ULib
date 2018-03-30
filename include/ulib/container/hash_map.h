// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    hash_map.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_HASHMAP_H
#define ULIB_HASHMAP_H 1

#include <ulib/container/vector.h>

/**
 * Modified, highly optimized Robin Hood Hashtable
 *
 * Algorithm from https://github.com/martinus/robin-hood-hashing/
 */

#define U_MAX_LOAD_FACTOR       0.90f // 0.50f
#define U_IS_BUCKET_TAKEN_MASK (1 << 7)

template <class T> class UHashMap;

typedef bool (*bPFpt)(UHashMap<void*>*);

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
template <class T> class UHashMapAnonIter;
#endif

class UHTTP2;
class WeightWord;

class U_NO_EXPORT UHashMapNode {
public:

   const void* elem;
   const UStringRep* key;
   uint32_t hash;

   void   set();
   void reset()
      {
      U_TRACE_NO_PARAM(0, "UHashMapNode::reset()")

      U_INTERNAL_DUMP("key = %V", key)

      ((UStringRep*)key)->release(); // NB: we decreases the reference string...
      }

   static uint32_t size() { return sizeof(uint32_t)+(sizeof(void*)*2); }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMapNode)
};

template <> class U_EXPORT UHashMap<void*> {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // Costruttori e distruttore

   UHashMap(uint32_t n = 64, bPFpt fset_index = setIndex)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<void*>, "%u,%p", n, fset_index)

      set_index = fset_index;

      init(n);
      }

   UHashMap(uint32_t n, bool ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<void*>, "%u,%b", n, ignore_case)

      set_index = (ignore_case ? setIndexIgnoreCase : setIndex);

      init(n);
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<void*>)

      if (_capacity) _deallocate();
      }

   // size and capacity

   void allocate(uint32_t n)
      {
      U_TRACE(0, "UHashMap<void*>::allocate(%u)", n)

      U_CHECK_MEMORY

      if (_capacity)
         {
         U_INTERNAL_ASSERT_DIFFERS(_capacity, n)

         _deallocate();
         }

      _allocate(n);
      }

   void deallocate()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::deallocate()")

      if (_capacity)
         {
         _deallocate();

         _capacity = 0;
         }
      }

   uint32_t size() const
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::size()")

      U_RETURN(_length);
      }

   uint32_t capacity() const
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::capacity()")

      U_RETURN(_capacity);
      }

   uint32_t getMask() const
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::getMask()")

      U_RETURN(mask);
      }

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::empty()")

      if (_length) U_RETURN(false);

      U_RETURN(true);
      }

   void setIgnoreCase(bool flag)
      {
      U_TRACE(0, "UHashMap<void*>::setIgnoreCase(%b)", flag)

      U_INTERNAL_ASSERT_EQUALS(_length, 0)

      set_index = (flag ? setIndexIgnoreCase : setIndex);
      }

   void setIndexFunction(bPFpt fset_index)
      {
      U_TRACE(0, "UHashMap<void*>::setIndexFunction(%p)", fset_index)

      U_INTERNAL_ASSERT_EQUALS(_length, 0)

      set_index = fset_index;
      }

   bool ignoreCase() const { return (set_index == setIndexIgnoreCase); }

   // ricerche

   bool find(const UString& k)
      {
      U_TRACE(0, "UHashMap<void*>::find(%V)", k.rep)

      lkey = k.rep;

      if (lookup()) U_RETURN(true);

      U_RETURN(false);
      }

   bool find(const char* k, uint32_t klen)
      {
      U_TRACE(0, "UHashMap<void*>::find(%.*S,%u)", klen, k, klen)

      setKey(k, klen);

      if (lookup()) U_RETURN(true);

      U_RETURN(false);
      }

   // get methods

   const void* elem() const      { return node->elem; }
   const UString getKey() const  { return UString(node->key); }
   const UStringRep* key() const { return node->key; }

   void* at(const UString& k)             { return (lkey = k.rep,    at()); }
   void* at(const UStringRep* k)          { return (lkey = k,        at()); }
   void* at(const char* k, uint32_t klen) { return (setKey(k, klen), at()); }

   void* operator[](const char* k)       { return (setKey(k, u__strlen(k, __PRETTY_FUNCTION__)), at()); }
   void* operator[](const UString& k)    { return (lkey = k.rep, at()); }
   void* operator[](const UStringRep* k) { return (lkey = k,     at()); }

   // after called find() (don't make the lookup)

   void   eraseAfterFind();
   void replaceAfterFind(const void* e) { node->elem = e; }
   void  insertAfterFind(const void* e)
      {
      U_TRACE(0, "UHashMap<void*>::insertAfterFind(%p)", e)

      lelem = e;

      ((UStringRep*)lkey)->hold(); // NB: we increases the reference string...

      insertAfterFind();
      }

   void insert(const UString& k, const void* e)
      {
      U_TRACE(0, "UHashMap<void*>::insert(%V,%p)", k.rep, e)

      lkey = k.rep;

      if (lookup()) replaceAfterFind(e);
      else           insertAfterFind(e);
      }

   void* erase(const UString&    k) { return (lkey = k.rep, erase()); }
   void* erase(const UStringRep* k) { return (lkey = k,     erase()); }

   void* erase(const char* k)
      {
      U_TRACE(0, "UHashMap<void*>::erase(%S)", k)

      setKey(k, u__strlen(k, __PRETTY_FUNCTION__));

      return erase();
      }

   void setNodePointer()                   { setNodePointer(table, index); }
   void setNodePointer(uint32_t idx) const { setNodePointer(table,   idx); }

   static void setNodePointer(char* table, uint32_t idx) { node = (UHashMapNode*)(table + (idx * UHashMapNode::size())); }

   static void nextNodePointer() { node = (UHashMapNode*)((char*)node + UHashMapNode::size()); }

   // traverse the hash table for all entry

   bool first()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::first()")

      U_INTERNAL_DUMP("_length = %u", _length)

      if (_length)
         {
         for (index = 0; index < _capacity; ++index)
            {
            if ((info[index] & U_IS_BUCKET_TAKEN_MASK) != 0)
               {
               setNodePointer();

               U_RETURN(true);
               }
            }
         }

      U_RETURN(false);
      }

   bool next()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::next()")

      U_INTERNAL_DUMP("index = %u", index)

      for (++index; index < _capacity; ++index)
         {
         if ((info[index] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            setNodePointer();

            U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   UHashMapNode* firstNode() { return (first() ? node : U_NULLPTR); }
   UHashMapNode*  nextNode() { return ( next() ? node : U_NULLPTR); }

   // call function for all entry

   void callForAllEntry(vPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::callForAllEntry(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      for (uint32_t idx = 0; idx < _capacity; ++idx)
         {
         if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            setNodePointer(idx);

            function((UStringRep*)node->key, (void*)node->elem);
            }
         }
      }

   void callForAllEntry(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::callForAllEntry(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      for (uint32_t idx = 0; idx < _capacity; ++idx)
         {
         if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            setNodePointer(idx);

            if (function((UStringRep*)node->key, (void*)node->elem) == false) return;
            }
         }
      }

   void getKeys(UVector<UString>& vec)
      {
      U_TRACE(0, "UHashMap<void*>::getKeys(%p)", &vec)

      pvec = &vec;

      callForAllEntry(geyKey);
      }

   void callForAllEntrySorted(vPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::callForAllEntrySorted(%p)", function)

      if (_length < 2) callForAllEntry(function);
      else            _callForAllEntrySorted(function);
      }

   void callForAllEntrySorted(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::callForAllEntrySorted(%p)", function)

      if (_length < 2) callForAllEntry(function);
      else            _callForAllEntrySorted(function);
      }

   void callWithDeleteForAllEntry(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::callWithDeleteForAllEntry(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      U_INTERNAL_ASSERT_MAJOR(_length, 0)

      for (uint32_t idx = 0; idx < _capacity; ++idx)
         {
         if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            setNodePointer(idx);

            if (function((UStringRep*)node->key, (void*)node->elem))
               {
               index = idx;

               eraseAfterFind();
               }
            }
         }

      U_INTERNAL_DUMP("_length = %u", _length)
      }

#ifdef DEBUG
# ifdef U_STDCPP_ENABLE
   const char* dump(bool reset) const;
# endif
   bool invariant();
   bool checkAt(const UStringRep* k, const void* e);
#endif

   // STREAMS

   static UHashMapNode* node;
   static bool istream_loading;
   static uint32_t index, lhash;
   static const UStringRep* lkey;

protected:
   char* table;
   uint8_t* info;
   bPFpt set_index;
   uint32_t _capacity, _length, mask, max_num_num_elements_allowed;

   static uint8_t linfo;
   static const void* lelem;
   static UVector<UString>* pvec;

   void increase_size();

   // allocate and deallocate methods

   void _allocate(uint32_t n);

   void _deallocate()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::_deallocate()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(_length, 0)
      U_INTERNAL_ASSERT_MAJOR(_capacity, 1)

      U_SYSCALL_VOID(free, "%p", info); // UMemoryPool::_free(info, _capacity, 1+UHashMapNode::size());
      }

   void init(uint32_t n)
      {
      U_TRACE(0, "UHashMap<void*>::init(%u)", n)

      U_INTERNAL_DUMP("this = %p", this)

      _length = 0;

      _allocate(n);
      }

   static void setKey(const char* k, uint32_t klen)
      {
      U_TRACE(0, "UHashMap<void*>::setKey(%.*S,%u)", klen, k, klen)

      U_INTERNAL_ASSERT_POINTER(k)
      U_INTERNAL_ASSERT_MAJOR(klen, 0)
      U_INTERNAL_ASSERT_POINTER(UString::pkey)

      lkey = UString::pkey;

      ((UStringRep*)lkey)->str     = k;
      ((UStringRep*)lkey)->_length = klen;
      }

   void putNode()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::putNode()")

      U_INTERNAL_DUMP("linfo = %u info[%u] = %u", linfo, index, info[index])

      info[index] = linfo;

      setNodePointer();

      node->set();

      U_ASSERT(checkAt(lkey, lelem))
      }

   void swapInfo()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::swapInfo()")

      U_INTERNAL_ASSERT_MAJOR(linfo, info[index])

      uint8_t tmp = linfo;
      linfo       = info[index];
      info[index] = tmp;

      U_INTERNAL_DUMP("linfo = %u info[%u] = %u", linfo, index, info[index])
      }

   void swapNode();
   void insertAfterFind();
   void swapNodeInResize();

   // Find a elem in the array with key

   bool lookup();
   bool lookup(const UString& k)    { return (lkey = k.rep, lookup()); }
   bool lookup(const UStringRep* k) { return (lkey = k,     lookup()); }

   void* at()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::at()")

      if (lookup()) U_RETURN((void*)node->elem);

      U_RETURN((void*)U_NULLPTR);
      }

   void* erase()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::erase()")

      if (lookup())
         {
         lelem = node->elem;

         eraseAfterFind();

         U_RETURN((void*)lelem);
         }

      U_RETURN((void*)U_NULLPTR);
      }

   static void geyKey(UStringRep* k, void* value)
      {
      U_TRACE(0, "UHashMap<void*>::geyKey(%V,%p)", k, value)

      pvec->UVector<UStringRep*>::push(node->key);
      }

   void getKeysSort(UVector<UString>& vec)
      {
      U_TRACE(0, "UHashMap<void*>::getKeysSort(%p)", &vec)

      U_INTERNAL_ASSERT_MAJOR(_length, 1)

      getKeys(vec);

      U_ASSERT_EQUALS(_length, vec.size())

      vec.sort(ignoreCase());
      }

   void _callForAllEntrySorted(vPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::_callForAllEntrySorted(%p)", function)

      UVector<UString> vkey(_length);

      getKeysSort(vkey);

      for (uint32_t i = 0, n = _length; i < n; ++i)
         {
         UStringRep* r = vkey.UVector<UStringRep*>::at(i);

         (void) lookup(r);

         function(r, (void*)node->elem);
         }
      }

   void _callForAllEntrySorted(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::_callForAllEntrySorted(%p)", function)

      UVector<UString> vkey(_length);

      getKeysSort(vkey);

      for (uint32_t i = 0, n = _length; i < n; ++i)
         {
         UStringRep* r = vkey.UVector<UStringRep*>::at(i);

         (void) lookup(r);

         if (function(r, (void*)node->elem) == false) return;
         }
      }

   static void setIdx(UHashMap<void*>* pthis)
      {
      U_TRACE(0, "UHashMap<void*>::setIdx(%p)", pthis)

      U_INTERNAL_DUMP("lhash = %u", lhash)

      U_INTERNAL_ASSERT_MAJOR(lhash, 0)
      U_INTERNAL_ASSERT_EQUALS(pthis->_capacity & pthis->mask, 0) // Must be a power of 2

      index = lhash & pthis->mask;

      U_INTERNAL_ASSERT_EQUALS(index, lhash % pthis->_capacity)
      }

   static bool setIndex(UHashMap<void*>* pthis)
      {
      U_TRACE(0, "UHashMap<void*>::setIndex(%p)", pthis)

      lhash = u_hash((unsigned char*)U_STRING_TO_PARAM(*lkey));

      setIdx(pthis);

      U_RETURN(false);
      }

   static bool setIndexIgnoreCase(UHashMap<void*>* pthis)
      {
      U_TRACE(0, "UHashMap<void*>::setIndexIgnoreCase(%p)", pthis)

      lhash = u_hash_ignore_case((unsigned char*)U_STRING_TO_PARAM(*lkey));

      setIdx(pthis);

      U_RETURN(true);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<void*>)

   friend class UHTTP2;
   friend class WeightWord;
   friend class UHashMapNode;
};

inline void UHashMapNode::set()
{
   U_TRACE_NO_PARAM(0, "UHashMapNode::set()")

   elem = UHashMap<void*>::lelem;
    key = UHashMap<void*>::lkey;
   hash = UHashMap<void*>::lhash;
}

template <class T> class U_EXPORT UHashMap<T*> : public UHashMap<void*> {
public:

   UHashMap(uint32_t n, bool ignore_case) : UHashMap<void*>(n, ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<T*>, "%u,%b", n, ignore_case)
      }

   UHashMap(uint32_t n = 64, bPFpt fset_index = setIndex) : UHashMap<void*>(n, fset_index)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<T*>, "%u,%p", n, fset_index)
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<T*>)

      clear();
      }

   T* elem() const { return (T*) UHashMap<void*>::elem(); }

   T* operator[](const char*       k) { return (T*) UHashMap<void*>::operator[](k); }
   T* operator[](const UString&    k) { return (T*) UHashMap<void*>::operator[](k); }
   T* operator[](const UStringRep* k) { return (T*) UHashMap<void*>::operator[](k); }

   T* erase(const char*       k) { return (T*) UHashMap<void*>::erase(k); }
   T* erase(const UString&    k) { return (T*) UHashMap<void*>::erase(k.rep); }
   T* erase(const UStringRep* k) { return (T*) UHashMap<void*>::erase(k); }

   void eraseAfterFind()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<T*>::eraseAfterFind()")

      u_destroy<T>((T*)node->elem);

      UHashMap<void*>::eraseAfterFind();
      }

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
   UHashMapAnonIter<T> begin() { return UHashMapAnonIter<T>(this, 0); }
   UHashMapAnonIter<T>   end() { return UHashMapAnonIter<T>(this, _length); }

   UHashMapAnonIter<T> erase(UHashMapAnonIter<T> it)
      {
      U_TRACE(0, "UHashMap<T*>::erase(%p)", &it)

      node = *it;

      eraseAfterFind();

      return ++it;
      }
#endif

   void insertAfterFind(const T* e)
      {
      U_TRACE(0, "UHashMap<T*>::insertAfterFind(%p)", e)

      u_construct<T>(&e, istream_loading);

      UHashMap<void*>::insertAfterFind(e);
      }

   void replaceAfterFind(const T* e)
      {
      U_TRACE(0, "UHashMap<T*>::replaceAfterFind(%p)", e)

      u_construct<T>(&e, false);

      u_destroy<T>((T*)node->elem);

      UHashMap<void*>::replaceAfterFind(e);
      }

   void insert(const UStringRep* k, const T* e)
      {
      U_TRACE(0, "UHashMap<T*>::insert(%V,%p)", k, e)

      if (UHashMap<void*>::lookup(k)) replaceAfterFind(e);
      else                               insertAfterFind(e);
      }

   void insert(const UString& k, const T* e) { return insert(k.rep, e); }

   // find a elem in the array with key

   T* at(const UString& k)             { return (T*) UHashMap<void*>::at(k.rep); }
   T* at(const UStringRep* k)          { return (T*) UHashMap<void*>::at(k); }
   T* at(const char* k, uint32_t klen) { return (T*) UHashMap<void*>::at(k, klen); }

   void clear() // erase all element
      {
      U_TRACE_NO_PARAM(0+256, "UHashMap<T*>::clear()")

      if (_length)
         {
         U_INTERNAL_DUMP("_length = %u", _length)

         for (uint32_t idx = 0; idx < _capacity; ++idx)
            {
            if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
               {
               setNodePointer(idx);

               node->reset();

               u_destroy<T>((T*)node->elem);
               }
            }

         _length = 0;

         (void) U_SYSCALL(memset, "%p,%d,%u", info, 0, _capacity);
         }
      }

   void callWithDeleteForAllEntry(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<T*>::callWithDeleteForAllEntry(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      U_INTERNAL_ASSERT_MAJOR(_length, 0)

      for (uint32_t idx = 0; idx < _capacity; ++idx)
         {
         if ((info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            setNodePointer(idx);

            if (function((UStringRep*)node->key, (void*)node->elem))
               {
               index = idx;

               eraseAfterFind();
               }
            }
         }

      U_INTERNAL_DUMP("_length = %u", _length)
      }

   void assign(UHashMap<T*>& t)
      {
      U_TRACE(0, "UHashMap<T*>::assign(%p)", &t)

      U_INTERNAL_ASSERT_DIFFERS(this, &t)

      clear();

      if (t.first())
         {
         uint32_t idx;

         if (_capacity != t._capacity) allocate(t._capacity);

         do {
            idx = index;

            insert(t.key(), t.elem());
            }
         while (index = idx, t.next());
         }

      U_INTERNAL_DUMP("_length = %u", _length)

      U_INTERNAL_ASSERT_EQUALS(_length, t._length)
      }

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend istream& operator>>(istream& is, UHashMap<T*>& t)
      {
      U_TRACE(0+256, "UHashMap<T*>::operator>>(%p,%p)", &is, &t)

      U_INTERNAL_ASSERT_MAJOR(t._capacity, 1)

      int c = EOF;

      if (is.good())
         {
         istream_loading = true; // NB: we need this flag for distinguish this operation in type's ctor...

         T* elem;
         
         U_NEW(T, elem, T);

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

            if (terminator == c) break;

            if (terminator == EOF &&
                (c == '}' || c == ']'))
               {
               break;
               }

            if (c == EOF) break;

            if (c == '#')
               {
               do { c = sb->sbumpc(); } while (c != EOF && c != '\n'); // skip line comment

               continue;
               }

            U_INTERNAL_ASSERT_EQUALS(u__isspace(c),false)

            sb->sputbackc(c);

            UString key(U_CAPACITY);

            key.get(is);

            U_INTERNAL_ASSERT(key)
            U_INTERNAL_ASSERT(key.isNullTerminated())

            do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

         // U_INTERNAL_DUMP("c = %C", c)

            if (c == EOF) break;

            U_INTERNAL_ASSERT_EQUALS(u__isspace(c),false)

            sb->sputbackc(c);

            is >> *elem;

            if (is.bad()) is.clear();
            else          t.insert(key.rep, elem);
            }
         while (c != EOF);

         u_destroy<T>(elem);

         istream_loading = false;
         }

      if (c == EOF) is.setstate(ios::eofbit);

   // -------------------------------------------------
   // NB: we can load an empty table
   // -------------------------------------------------
   // if (t._length == 0) is.setstate(ios::failbit);
   // -------------------------------------------------

      return is;
      }

   friend ostream& operator<<(ostream& _os, const UHashMap<T*>& t)
      {
      U_TRACE(0+256, "UHashMap<T*>::operator<<(%p,%p)", &_os, &t)

      U_INTERNAL_DUMP("t._length = %u", t._length)

      _os.put('[');
      _os.put('\n');

      for (uint32_t idx = 0; idx < t._capacity; ++idx)
         {
         if ((t.info[idx] & U_IS_BUCKET_TAKEN_MASK) != 0)
            {
            t.setNodePointer(idx);

            t.node->key->write(_os);

            _os.put('\t');

            _os << *(T*)(t.node->elem);

            _os.put('\n');
            }
         }

      _os.put(']');

      return _os;
      }

# ifdef DEBUG
   const char* dump(bool reset) const { return UHashMap<void*>::dump(reset); }
# endif
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<T*>)
};

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
template <class T> class UHashMapAnonIter {
public:
   explicit UHashMapAnonIter(UHashMap<T*>* m, uint32_t l) : map(m), length(l) {}

   bool operator!=(const UHashMapAnonIter& other) const { return (length != other.length); }

   UHashMapNode* operator*() const { return node; }

   UHashMapAnonIter& operator++()
      {
      node = (length++ ? map->nextNode() : map->firstNode());

      return *this;
      }

protected:
   UHashMap<T*>* map;
   UHashMapNode* node;
   uint32_t length;
};
#endif

template <> class U_EXPORT UHashMap<UString> : public UHashMap<UStringRep*> {
public:

   explicit UHashMap(uint32_t n, bool ignore_case) : UHashMap<UStringRep*>(n, ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UString>, "%u,%b", n, ignore_case)
      }

   explicit UHashMap(uint32_t n = 64, bPFpt fset_index = setIndex) : UHashMap<UStringRep*>(n, fset_index)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UString>, "%u,%p", n, fset_index)
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<UString>)
      }

   void insertAfterFind(const UString& str)
      {
      U_TRACE(0, "UHashMap<UString>::insertAfterFind(%V)", str.rep)

      UHashMap<UStringRep*>::insertAfterFind(str.rep);
      }

   void replaceAfterFind(const UString& str)
      {
      U_TRACE(0, "UHashMap<T*>::replaceAfterFind(%V)", str.rep)

      UHashMap<UStringRep*>::replaceAfterFind(str.rep);
      }

   void insert(const UStringRep* k, const UStringRep* e)
      {
      U_TRACE(0, "UHashMap<UString>::insert(%V,%V)", k, e)

      UHashMap<UStringRep*>::insert(k, e);
      }

   void insert(const UString& k, const UString& str)
      {
      U_TRACE(0, "UHashMap<UString>::insert(%V,%V)", k.rep, str.rep)

      UHashMap<UStringRep*>::insert(k.rep, str.rep);
      }

   UString erase(const UString& k)
      {
      U_TRACE(0, "UHashMap<UString>::erase(%V)", k.rep)

      if (UHashMap<void*>::lookup(k))
         {
         UString str(elem());

         U_INTERNAL_DUMP("str.reference() = %u", str.reference())

         U_INTERNAL_ASSERT_MAJOR(str.reference(), 0)

         UHashMap<UStringRep*>::eraseAfterFind();

         U_INTERNAL_DUMP("str.reference() = %u", str.reference())

         U_RETURN_STRING(str);
         }

      return UString::getStringNull();
      }

   uint32_t getSpaceToDump() const __pure;

   // OPERATOR

   bool operator==(const UHashMap<UString>& v) __pure;
   bool operator!=(const UHashMap<UString>& v) { return ! operator==(v); }

   // OPERATOR []

   UString at(const UString& k)             { return (lkey = k.rep,    at()); }
   UString at(const UStringRep* k)          { return (lkey = k,        at()); }
   UString at(const char* k, uint32_t klen) { return (setKey(k, klen), at()); }

   UString operator[](const char* k)
      {
      U_TRACE(0, "UHashMap<UString>::operator[](%S)", k)

      setKey(k, u__strlen(k, __PRETTY_FUNCTION__));

      return at();
      }

   UString operator[](const UString& k)    { return (lkey = k.rep, at()); }
   UString operator[](const UStringRep* k) { return (lkey = k,     at()); }

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UHashMap<UString>& t);
   friend U_EXPORT ostream& operator<<(ostream& os, const UHashMap<UString>& t) { return operator<<(os, (const UHashMap<UStringRep*>&)t); }
#endif

   uint32_t loadFromData(const char* start, uint32_t size);

   void loadFromData(const UString& str) { (void) loadFromData(U_STRING_TO_PARAM(str)); }

protected:

   UString at()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<UString>::at()")

      if (UHashMap<void*>::lookup())
         {
         UString str(elem());

         U_RETURN_STRING(str);
         }

      return UString::getStringNull();
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<UString>)
};

template <> class U_EXPORT UHashMap<UVectorUString> : public UHashMap<UVectorUString*> {
public:

   explicit UHashMap(uint32_t n, bool ignore_case) : UHashMap<UVectorUString*>(n, ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UVectorUString>, "%u,%b", n, ignore_case)
      }

   explicit UHashMap(uint32_t n = 64, bPFpt fset_index = setIndex) : UHashMap<UVectorUString*>(n, fset_index)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UVectorUString>, "%u,%p", n, fset_index)
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<UVectorUString>)
      }

   bool empty()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<UVectorUString>::empty()")

      if (first())
         {
         do {
            pvec = elem();
            
            if (pvec->empty() == false) U_RETURN(false);
            }
         while (next());
         }

      U_RETURN(true);
      }

   void erase(const UString& k, uint32_t pos) // remove element at pos
      {
      U_TRACE(0, "UHashMap<UVectorUString>::erase(%V,%u)", k.rep, pos)

      if (UHashMap<void*>::lookup(k)) ((UVector<UString>*)node->elem)->erase(pos);
      }

   void push(const UString& k, const UString& str)
      {
      U_TRACE(0, "UHashMap<UVectorUString>::push(%V,%V)", k.rep, str.rep)

      if (UHashMap<void*>::lookup(k)) pvec = (UVector<UString>*) node->elem;
      else
         {
         U_NEW(UVector<UString>, pvec, UVector<UString>);

         UHashMap<UVectorUString*>::insertAfterFind(pvec);
         }

      pvec->push(str);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<UVectorUString>)
};
#endif
