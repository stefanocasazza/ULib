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

#ifndef ULIB_HASH_MAP_H
#define ULIB_HASH_MAP_H 1

#include <ulib/container/vector.h>

typedef UVector<UString> UVectorUString;

typedef bool     (*bPFprpv)  (UStringRep*,void*);
typedef uint32_t (*uPFpcu)   (const char*,uint32_t);
typedef bool     (*bPFptpcu) (UHashMap<void*>*,const char*,uint32_t);

class UCDB;
class UHTTP;
class UHTTP2;
class WeightWord;
class UMimeHeader;
class UFileConfig;
class UNoCatPlugIn;
class UCertificate;

template <class T> class UJsonTypeHandler;
template <class T> class URDBObjectHandler;

class U_NO_EXPORT UHashMapNode {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   const void* elem;
   const UStringRep* key;
   UHashMapNode* next;
   uint32_t hash;

   UHashMapNode(const UStringRep* _key, const void* _elem, UHashMapNode* _next, uint32_t _hash) : elem(_elem), key(_key), next(_next), hash(_hash)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMapNode, "%V,%p,%p,%u", _key, _elem, _next, _hash)

      ((UStringRep*)_key)->hold(); // NB: we increases the reference string...
      }

   UHashMapNode(UHashMapNode* n, UHashMapNode* _next) : elem(n->elem), key(n->key), next(_next), hash(n->hash)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMapNode, "%p,%p", n, _next)

      ((UStringRep*)key)->hold(); // NB: we increases the reference string...
      }

   ~UHashMapNode()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMapNode)

      ((UStringRep*)key)->release(); // NB: we decreases the reference string...
      }

   UHashMapNode& operator=(const UHashMapNode& n)
      {
      U_TRACE(0, "UHashMapNode::operator=(%p)", &n)

      U_MEMORY_TEST_COPY(n)

      elem = n.elem;
      key  = n.key;
      next = n.next;
      hash = n.hash;

      return *this;
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   U_EXPORT const char* dump(bool reset) const;
#endif
};

template <class T> class UHashMap;

template <> class U_EXPORT UHashMap<void*> {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // Costruttori e distruttore

   UHashMap(uint32_t n, bool ignore_case);

   UHashMap(uint32_t n = 53, bPFptpcu _set_index = setIndex);

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<void*>)

      U_INTERNAL_ASSERT_EQUALS(_length, 0)

      if (_capacity) _deallocate();
      }

   // allocate and deallocate methods

   void deallocate()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::deallocate()")

      if (_capacity)
         {
         _deallocate();

         _capacity = 0;
         }
      }

   void allocate(uint32_t n);

   // size and capacity

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

   void setIndexFunction(bPFptpcu _set_index)
      {
      U_TRACE(0, "UHashMap<void*>::setIndexFunction(%p)", _set_index)

      U_INTERNAL_ASSERT_EQUALS(_length, 0)

      set_index = _set_index;
      }

   bool ignoreCase() const { return (set_index == setIndexIgnoreCase); }

   // ricerche

   bool find(const UString& _key)
      {
      U_TRACE(0, "UHashMap<void*>::find(%V)", _key.rep)

      lookup(_key);

      if (node) U_RETURN(true);

      U_RETURN(false);
      }

   bool find(const char* key, uint32_t keylen);

   // set/get methods

   void* operator[](const char* _key)
      {
      U_TRACE(0, "UHashMap<void*>::operator[](%S)", _key)

      U_INTERNAL_ASSERT_POINTER(pkey)

      pkey->str     =           _key;
      pkey->_length = u__strlen(_key, __PRETTY_FUNCTION__);

      return at(pkey);
      }

   void* operator[](const UString&    _key) { return at(_key.rep); }
   void* operator[](const UStringRep* _key) { return at(_key); }

   const void* elem() const      { return node->elem; }
   const UString getKey() const  { return UString(node->key); }
   const UStringRep* key() const { return node->key; }

   template <typename T> T* get(const UString& _key)
      {
      U_TRACE(0, "UHashMap<void*>::get(%V)", _key.rep)

      return (T*) operator[](_key);
      }

   // sets a field, overwriting any existing value

   void insert(const UString& _key, const void* _elem)
      {
      U_TRACE(0, "UHashMap<void*>::insert(%V,%p)", _key.rep, _elem)

      lookup(_key);

      insertAfterFind(_key.rep, _elem);
      }

   // after called find() (don't make the lookup)

   void insertAfterFind(const UString&    _key, const void* _elem) { insertAfterFind(_key.rep, _elem); }
   void insertAfterFind(const UStringRep* _key, const void* _elem);

   void   eraseAfterFind();
   void replaceAfterFind(const void* _elem)
      {
      U_TRACE(0, "UHashMap<void*>::replaceAfterFind(%p)", _elem)

      node->elem = _elem;
      }

   void replaceKey(const UString& key);

   void* erase(const char*       _key);
   void* erase(const UString&    _key) { return erase(_key.rep); }
   void* erase(const UStringRep* _key);

   // make room for a total of n element

   void reserve(uint32_t n);

   // Traverse the hash table for all entry

   UHashMapNode* first();

   // We need to pass the pointer because we can lost the internal pointer between the call...

   bool          next();
   UHashMapNode* next(UHashMapNode* node);

   // call function for all entry

   void callForAllEntry(bPFprpv function);
   void callForAllEntrySorted(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<void*>::callForAllEntrySorted(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      if (_length < 2)
         {
         callForAllEntry(function);

         return;
         }

      _callForAllEntrySorted(function);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   // STREAMS

   static bool istream_loading;

protected:
   bPFptpcu set_index;
   UHashMapNode* node;
   UHashMapNode** table;
   uint32_t _capacity, _length, hash;
public:
   uint32_t index;
protected:
   static UStringRep* pkey;

#ifdef DEBUG
   bool check_memory() const; // check all element
#endif

   void _allocate(uint32_t n)
      {
      U_TRACE(0, "UHashMap<void*>::_allocate(%u)", n)

      U_CHECK_MEMORY

      table     = (UHashMapNode**) UMemoryPool::_malloc(&n, sizeof(UHashMapNode*), true);
      _capacity = n;
      }

   void _deallocate()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<void*>::_deallocate()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 1)

      UMemoryPool::_free(table, _capacity, sizeof(UHashMapNode*));
      }

   void init(uint32_t n)
      {
      U_TRACE(0, "UHashMap<void*>::init(%u)", n)

      U_INTERNAL_DUMP("this = %p", this)

      node = 0;

      _allocate(n);

      _length =
      hash    =
      index   = 0;
      }

   // Find a elem in the array with <key>

   void* at(const UStringRep* _key)
      {
      U_TRACE(0, "UHashMap<void*>::at(%V)", _key)

      lookup(_key);

      if (node) U_RETURN((void*)node->elem);

      U_RETURN((void*)0);
      }

   void* at(const char* _key, uint32_t keylen)
      {
      U_TRACE(0, "UHashMap<void*>::at(%.*S,%u)", keylen, _key, keylen)

      U_INTERNAL_ASSERT_POINTER(pkey)
      U_INTERNAL_ASSERT_POINTER(_key)
      U_INTERNAL_ASSERT_MAJOR(keylen, 0)

      pkey->str     = _key;
      pkey->_length = keylen;

      return at(pkey);
      }

   void getKeys(UVector<UString>& vec);

   void lookup(const UString&    keyr) { return lookup(keyr.rep); }
   void lookup(const UStringRep* keyr);

   void _eraseAfterFind();
   void _callForAllEntrySorted(bPFprpv function);

   static bool setIndex(UHashMap<void*>* pthis, const char* _key, uint32_t keylen)
      {
      U_TRACE(0, "UHashMap<void*>::setIndex(%p,%.*S,%u)", pthis, keylen, _key, keylen)

      pthis->index = (pthis->hash = u_hash((unsigned char*)_key, keylen)) % pthis->_capacity;

      U_INTERNAL_ASSERT_MAJOR(pthis->hash, 0)

      U_RETURN(false);
      }

   static bool setIndexIgnoreCase(UHashMap<void*>* pthis, const char* _key, uint32_t keylen)
      {
      U_TRACE(0, "UHashMap<void*>::setIndexIgnoreCase(%p,%.*S,%u)", pthis, keylen, _key, keylen)

      pthis->index = (pthis->hash = u_hash_ignore_case((unsigned char*)_key, keylen)) % pthis->_capacity;

      U_INTERNAL_ASSERT_MAJOR(pthis->hash, 0)

      U_RETURN(true);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<void*>)

   friend class ULib;
   friend class UCDB;
   friend class UHTTP;
   friend class UHTTP2;
   friend class UValue;
   friend class UString;
   friend class WeightWord;
   friend class UFileConfig;
   friend class UCertificate;

   template <class T> friend class UJsonTypeHandler;
};

template <class T> class U_EXPORT UHashMap<T*> : public UHashMap<void*> {
public:

   UHashMap(uint32_t n, bool ignore_case) : UHashMap<void*>(n, ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<T*>, "%u,%b", n, ignore_case)
      }

   UHashMap(uint32_t n = 53, bPFptpcu _set_index = setIndex) : UHashMap<void*>(n, _set_index)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<T*>, "%u,%p", n, _set_index)
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<T*>)

      clear();
      }

   T* erase(const char*       _key) { return (T*) UHashMap<void*>::erase(_key); }
   T* erase(const UString&    _key) { return (T*) UHashMap<void*>::erase(_key.rep); }
   T* erase(const UStringRep* _key) { return (T*) UHashMap<void*>::erase(_key); }

   T* elem() const { return (T*) UHashMap<void*>::elem(); }

   T* operator[](const char*       _key) { return (T*) UHashMap<void*>::operator[](_key); }
   T* operator[](const UString&    _key) { return (T*) UHashMap<void*>::operator[](_key); }
   T* operator[](const UStringRep* _key) { return (T*) UHashMap<void*>::operator[](_key); }

   void eraseAfterFind()
      {
      U_TRACE_NO_PARAM(0, "UHashMap<T*>::eraseAfterFind()")

      U_INTERNAL_ASSERT_POINTER(node)

      u_destroy<T>((const T*)node->elem);

      UHashMap<void*>::eraseAfterFind();
      }

   void insertAfterFind(const UStringRep* _key, const T* _elem)
      {
      U_TRACE(0, "UHashMap<T*>::insertAfterFind(%V,%p)", _key, _elem)

      u_construct<T>(&_elem, istream_loading);

      if (node == 0) UHashMap<void*>::insertAfterFind(_key, _elem);
      else
         {
         u_destroy<T>((const T*)node->elem);

         node->elem = _elem;
         }
      }

   void insertAfterFind(const UString& _key, const T* _elem) { insertAfterFind(_key.rep, _elem); }

   void replaceAfterFind(const T* _elem)
      {
      U_TRACE(0, "UHashMap<T*>::replaceAfterFind(%p)", _elem)

      U_INTERNAL_ASSERT_POINTER(node)

      u_construct<T>(&_elem, false);

      u_destroy<T>((const T*)node->elem);

      UHashMap<void*>::replaceAfterFind(_elem);
      }

   // sets a field, overwriting any existing value

   void insert(const UStringRep* _key, const T* _elem)
      {
      U_TRACE(0, "UHashMap<T*>::insert(%V,%p)", _key, _elem)

      UHashMap<void*>::lookup(_key);

      insertAfterFind(_key, _elem);
      }

   void insert(const UString& _key, const T* _elem)
      {
      U_TRACE(0, "UHashMap<T*>::insert(%V,%p)", _key.rep, _elem)

      UHashMap<void*>::lookup(_key);

      insertAfterFind(_key.rep, _elem);
      }

   // find a elem in the array with <key>

   T* at(const UString& _key)               { return (T*) UHashMap<void*>::at(_key.rep); }
   T* at(const UStringRep* keyr)            { return (T*) UHashMap<void*>::at(keyr); }
   T* at(const char* _key, uint32_t keylen) { return (T*) UHashMap<void*>::at(_key, keylen); }

   void clear() // erase all element
      {
      U_TRACE_NO_PARAM(0+256, "UHashMap<T*>::clear()")

      U_INTERNAL_ASSERT(check_memory())

      U_INTERNAL_DUMP("_length = %u", _length)

      if (_length)
         {
         T* _elem;
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
                  _next  =    _node->next;
                  _elem = (T*)_node->elem;

                  if (_elem) u_destroy<T>(_elem);

                  delete _node;
                  }
               while ((_node = _next));

               *ptr = 0;
               }
            }

         _length = 0;
         }
      }

   void callWithDeleteForAllEntry(bPFprpv function)
      {
      U_TRACE(0, "UHashMap<T*>::callWithDeleteForAllEntry(%p)", function)

      U_INTERNAL_DUMP("_length = %u", _length)

      const T* _elem;
      UHashMapNode** ptr;
      UHashMapNode** end;
      UHashMapNode* _node;
      UHashMapNode** pnode;

      for (end = (ptr = table) + _capacity; ptr < end; ++ptr)
         {
         if (*ptr)
            {
            _node = *(pnode = ptr);

            do {
               _elem = (const T*)_node->elem;

               if (function((UStringRep*)_node->key, (void*)_elem))
                  {
                  *pnode = _node->next; // we remove it from the list collisions

                  u_destroy<T>(_elem);

                  delete _node;

                  --_length;

                  continue;
                  }

               pnode = &(*pnode)->next;
               }
            while ((_node = *pnode));
            }
         }

      U_INTERNAL_DUMP("_length = %u", _length)
      }

   void assign(UHashMap<T*>& t)
      {
      U_TRACE(0, "UHashMap<T*>::assign(%p)", &t)

      U_INTERNAL_ASSERT_DIFFERS(this, &t)

      clear();

      if (t._length)
         {
         const T* _elem;
         UHashMapNode* _node;
         UHashMapNode* _next;
         UHashMapNode** ptr1;
         UHashMapNode** end1;

         allocate(t._capacity);

         UHashMapNode** ptr = table;

         for (end1 = (ptr1 = t.table) + t._capacity; ptr1 < end1; ++ptr1, ++ptr)
            {
            if (*ptr1)
               {
               _node = *ptr1;

               U_INTERNAL_ASSERT_EQUALS(*ptr, 0)

               do {
                  U_NEW(UHashMapNode, *ptr, UHashMapNode(_node, *ptr)); // we place it in the list collisions

                  _elem = (const T*) (*ptr)->elem;

                  U_INTERNAL_DUMP("_elem = %p", _elem)

                  U_ASSERT_EQUALS(_elem, t[_node->key])

                  u_construct<T>(&_elem, false);

                  _next = _node->next;
                  }
               while ((_node = _next));
               }
            }

         _length = t._length;
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

         const T* _elem;
         
         U_NEW(T, _elem, T);

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

            is >> *(T*)_elem;

            if (is.bad()) is.clear();
            else          t.insert(key.rep, _elem);
            }
         while (c != EOF);

         u_destroy<T>(_elem);

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

      U_INTERNAL_ASSERT(t.check_memory())

      U_INTERNAL_DUMP("t._length = %u", t._length)

      UHashMapNode* _node;
      UHashMapNode* _next;
      UHashMapNode** ptr;
      UHashMapNode** end;

      _os.put('[');
      _os.put('\n');

      for (end = (ptr = t.table) + t._capacity; ptr < end; ++ptr)
         {
         if (*ptr)
            {
            _node = *ptr;

            do {
               _node->key->write(_os);

               _os.put('\t');

               _os << *((T*)_node->elem);

               _os.put('\n');

               _next = _node->next;
               }
            while ((_node = _next));
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

   friend class UHTTP;
   friend class UValue;
   friend class WeightWord;
   friend class UNoCatPlugIn;
};

template <> class U_EXPORT UHashMap<UString> : public UHashMap<UStringRep*> {
public:

   explicit UHashMap(uint32_t n, bool ignore_case) : UHashMap<UStringRep*>(n, ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UString>, "%u,%b", n, ignore_case)
      }

   explicit UHashMap(uint32_t n = 53, bPFptpcu _set_index = setIndex) : UHashMap<UStringRep*>(n, _set_index)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UString>, "%u,%p", n, _set_index)
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<UString>)
      }

   void replaceAfterFind(const UString& str)
      {
      U_TRACE(0, "UHashMap<T*>::replaceAfterFind(%V)", str.rep)

      UHashMap<UStringRep*>::replaceAfterFind(str.rep);
      }

   void insert(const UStringRep* _key, const UStringRep* _elem)
      {
      U_TRACE(0, "UHashMap<UString>::insert(%V,%V)", _key, _elem)

      UHashMap<UStringRep*>::insert(_key, _elem);
      }

   void insert(const UString& _key, const UString& str)
      {
      U_TRACE(0, "UHashMap<UString>::insert(%V,%V)", _key.rep, str.rep)

      UHashMap<UStringRep*>::insert(_key.rep, str.rep);
      }

   UString erase(const UString& key);

   void insertAfterFind(const UString& _key, const UString& str)
      {
      U_TRACE(0, "UHashMap<UString>::insertAfterFind(%V,%V)", _key.rep, str.rep)

      UHashMap<UStringRep*>::insertAfterFind(_key, str.rep);
      }

   // OPERATOR

   bool operator==(const UHashMap<UString>& v) __pure;
   bool operator!=(const UHashMap<UString>& v) { return ! operator==(v); }

   // OPERATOR []

   UString operator[](const char*       _key);
   UString operator[](const UString&    _key) { return at(_key.rep); }
   UString operator[](const UStringRep* _key) { return at(_key);     }

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UHashMap<UString>& t);
   friend U_EXPORT ostream& operator<<(ostream& os, const UHashMap<UString>& t) { return operator<<(os, (const UHashMap<UStringRep*>&)t); }
#endif

   void loadFromData(const UString& str) { (void) loadFromData(U_STRING_TO_PARAM(str)); }

protected:
   UString at(const UStringRep* keyr);
   UString at(const char* _key, uint32_t keylen);

   uint32_t loadFromData(const char* start, uint32_t size);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<UString>)

   friend class UHTTP;
   friend class UHTTP2;
   friend class UFileConfig;
   friend class UMimeHeader;
   friend class UNoCatPlugIn;

   template <class T> friend class URDBObjectHandler;
};

template <> class U_EXPORT UHashMap<UVectorUString> : public UHashMap<UVectorUString*> {
public:

   explicit UHashMap(uint32_t n, bool ignore_case) : UHashMap<UVectorUString*>(n, ignore_case)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UVectorUString>, "%u,%b", n, ignore_case)
      }

   explicit UHashMap(uint32_t n = 53, bPFptpcu _set_index = setIndex) : UHashMap<UVectorUString*>(n, _set_index)
      {
      U_TRACE_REGISTER_OBJECT(0, UHashMap<UVectorUString>, "%u,%p", n, _set_index)
      }

   ~UHashMap()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHashMap<UVectorUString>)
      }

   bool empty();

   void erase(const UString& _key, uint32_t pos); // remove element at pos

   void push(const UString& _key, const UString& str);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHashMap<UVectorUString>)
};

#endif
