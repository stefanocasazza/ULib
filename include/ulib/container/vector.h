// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    vector.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_VECTOR_H
#define ULIB_VECTOR_H 1

#include <ulib/container/construct.h>

#ifdef U_STDCPP_ENABLE
#  include <algorithm>
#endif

/**
 * A vector is a sequence of elements that are stored contiguously in memory and can change in size.
 * As a result, it has support for random-access and provides methods to add and delete elements.
 * It is typically used when an array is required, but the exact number if elements is unknown at compile-time
 *
 * Simple vector template class. Supports pushing at end and random-access deletions. Dynamically sized
 */

class UHTTP;
class UThreadPool;
class UHttpPlugIn;
class UFileConfig;
class UNoCatPlugIn;

template <class T> class UVector;
template <class T> class UOrmTypeHandler;
template <class T> class UJsonTypeHandler;

//#define U_RING_BUFFER

template <> class U_EXPORT UVector<void*> {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // allocate and deallocate methods

   void allocate(uint32_t n)
      {
      U_TRACE(0, "UVector<void*>::allocate(%u)", n)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MINOR(n, ((0xfffffff / sizeof(void*)) - sizeof(UVector<void*>)))

      vec       = (const void**) UMemoryPool::_malloc(&n, sizeof(void*));
      _capacity = n;
      }

   void deallocate()
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::deallocate()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_RANGE(1, _capacity, ((0xfffffff / sizeof(void*)) - sizeof(UVector<void*>)))

      UMemoryPool::_free(vec, _capacity, sizeof(void*));
      }

   UVector(uint32_t n = 64U) // create an empty vector with a size estimate
      {
      U_TRACE_REGISTER_OBJECT(0, UVector<void*>, "%u", n)

#  if defined(U_RING_BUFFER) && !defined(U_STATIC_ONLY)
      head = tail = 0;
#  endif

      _length = 0;

      allocate(n);
      }

   ~UVector()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UVector<void*>)

      deallocate();
      }

   // Size and Capacity

   uint32_t size() const
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::size()")

      U_RETURN(_length);
      }

   uint32_t capacity() const
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::capacity()")

      U_RETURN(_capacity);
      }

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::empty()")

      U_RETURN(_length == 0);
      }

   // Make room for a total of n element

   void reserve(uint32_t n);

   // ELEMENT ACCESS

   const void* front() { return *vec; }
   const void*  back() { return *(vec + _length -1); }

   const void*& at(uint32_t pos) __pure
      {
      U_TRACE(0, "UVector<void*>::at(%u)", pos)

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_MINOR(pos, _length)

      return vec[pos];
      }

   const void* at(uint32_t pos) const __pure
      {
      U_TRACE(0, "UVector<void*>::at(%u) const", pos)

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_MINOR(pos, _length)

      return vec[pos];
      }

   const void*& operator[](uint32_t pos)       { return at(pos); }
   const void*  operator[](uint32_t pos) const { return at(pos); }

   void replace(uint32_t pos, const void* elem)
      {
      U_TRACE(0, "UVector<void*>::replace(%u,%p)", pos, elem)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_MINOR(pos, _length)

      vec[pos] = elem;
      }

   // STACK OPERATIONS

   void push(     const void* elem);
   void push_back(const void* elem) { push(elem); } // add to end

   const void* last() // return last element
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::last()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

      return vec[_length-1];
      }

   const void* pop() // remove last element
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::pop()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

      return vec[--_length];
      }

   const void* pop_front() { return pop(); } // remove last element

   // LIST OPERATIONS

   void insert(uint32_t pos,             const void* elem); // add           elem before pos
   void insert(uint32_t pos, uint32_t n, const void* elem); // add n copy of elem before pos

   void erase(uint32_t pos)
      {
      U_TRACE(1, "UVector<void*>::erase(%u)", pos)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MINOR(pos, _length)
      U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

#  ifdef U_APEX_ENABLE
      if (--_length) (void) U_SYSCALL(apex_memmove, "%p,%p,%u", vec + pos, vec + pos +1, (_length - pos) * sizeof(void*));
#  else
      if (--_length) (void) U_SYSCALL(     memmove, "%p,%p,%u", vec + pos, vec + pos +1, (_length - pos) * sizeof(void*));
#  endif
      }

   void erase(uint32_t first, uint32_t _last) // erase [first,last[
      {
      U_TRACE(1, "UVector<void*>::erase(%u,%u)",  first, _last)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(_last <= _length)
      U_INTERNAL_ASSERT_MINOR(first, _last)
      U_INTERNAL_ASSERT_MINOR(first, _length)
      U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

      uint32_t new_length = (_length - (_last - first));

#  ifdef U_APEX_ENABLE
      if (new_length) (void) U_SYSCALL(apex_memmove, "%p,%p,%u", vec + first, vec + _last, (_length - _last) * sizeof(void*));
#  else
      if (new_length) (void) U_SYSCALL(     memmove, "%p,%p,%u", vec + first, vec + _last, (_length - _last) * sizeof(void*));
#  endif

      _length = new_length;
      }

   const void* remove(uint32_t pos)
      {
      U_TRACE(0, "UVector<void*>::remove(%u)", pos)

      const void* elem = vec[pos];

      erase(pos);

      return elem;
      }

   void swap(uint32_t from, uint32_t to)
      {
      U_TRACE(0, "UVector<void*>::swap(%u,%u)", from, to)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(to <= _length)
      U_INTERNAL_ASSERT(from <= _length)
      U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

      const void* tmp = vec[from];
      vec[from] = vec[to];
      vec[to]   = tmp;
      }

   // BINARY HEAP

   const void* bh_min() const __pure
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::bh_min()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

      // The item at the top of the binary heap has the minimum key value

      return vec[1];
      }

   // Call function for all entry

   void callForAllEntry(vPFpv function)
      {
      U_TRACE(0, "UVector<void*>::callForAllEntry(%p)", function)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("_length = %u", _length)

      for (uint32_t i = 0; i < _length; ++i) function((void*)vec[i]);
      }

   void callForAllEntry(bPFpv function)
      {
      U_TRACE(0, "UVector<void*>::callForAllEntry(%p)", function)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("_length = %u", _length)

      for (uint32_t i = 0; i < _length && function((void*)vec[i]); ++i) {}
      }

   uint32_t find(const void* elem)
      {
      U_TRACE(0, "UVector<void*>::find(%p)", elem)

      U_CHECK_MEMORY

      for (uint32_t i = 0; i < _length; ++i)
         {
         if (vec[i] == elem) U_RETURN(i);
         }

      U_RETURN(U_NOT_FOUND);
      }

   // EXTENSION

   static int qscomp(const void* p, const void* q)
      {
      U_TRACE(0, "UVector<void*>::qscomp(%p,%p)", p, q)

#  ifdef U_STDCPP_ENABLE
      /**
       * The comparison function must follow a strict-weak-ordering
       *
       * 1) For all x, it is not the case that x < x (irreflexivity)
       * 2) For all x, y, if x < y then it is not the case that y < x (asymmetry)
       * 3) For all x, y, and z, if x < y and y < z then x < z (transitivity)
       * 4) For all x, y, and z, if x is incomparable with y, and y is incomparable with z, then x is incomparable with z (transitivity of incomparability)
       */

      return (((UStringRep*)p)->comparenocase((UStringRep*)q) < 0);
#  else
      return (*(UStringRep**)p)->comparenocase(*(UStringRep**)q);
#  endif
      }

   void sort(qcompare compare_obj = qscomp)
      {
      U_TRACE(0+256, "UVector<void*>::sort(%p)", compare_obj)

      U_INTERNAL_DUMP("_length = %u", _length)

   // U_INTERNAL_ASSERT_RANGE(2,_length,_capacity)

#  ifdef U_STDCPP_ENABLE
      std::sort(vec, vec+_length, compare_obj);
#  else
      U_SYSCALL_VOID(qsort, "%p,%u,%d,%p", (void*)vec, _length, sizeof(void*), compare_obj);
#  endif
      }

   void move(UVector<void*>& source); // add to end and reset source

#if defined(U_RING_BUFFER) && !defined(U_STATIC_ONLY)
   bool isEmptyRingBuffer()
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::isEmptyRingBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("head = %u tail = %u", head, tail)

      U_INTERNAL_ASSERT_MINOR(head, _capacity)
      U_INTERNAL_ASSERT_MINOR(tail, _capacity)

      if (head == tail) U_RETURN(true);

      U_RETURN(false);
      }

   uint32_t sizeRingBuffer()
      {
      U_TRACE_NO_PARAM(0, "UVector<void*>::sizeRingBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("head = %u tail = %u", head, tail)

      U_INTERNAL_ASSERT_MINOR(head, _capacity)
      U_INTERNAL_ASSERT_MINOR(tail, _capacity)

      uint32_t sz = 0, i = head;

      while (i != tail)
         {
         ++sz;

         i = ((i+1) % _capacity);
         }

      U_RETURN(sz);
      }

   void callForAllEntryRingBuffer(bPFpv function)
      {
      U_TRACE(0, "UVector<void*>::callForAllEntryRingBuffer(%p)", function)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("head = %u tail = %u", head, tail)

      U_INTERNAL_ASSERT_MINOR(head, _capacity)
      U_INTERNAL_ASSERT_MINOR(tail, _capacity)

      for (uint32_t i = head; i != tail && function((void*)vec[i]); i = ((i+1) % _capacity)) {}
      }
#endif

#ifdef DEBUG
   bool check_memory(); // check all element
# ifdef U_STDCPP_ENABLE
   const char* dump(bool reset) const;
# endif
#endif

   // STREAMS

   static bool istream_loading;

protected:
   const void** vec;
   uint32_t _length, _capacity;
#if defined(U_RING_BUFFER) && !defined(U_STATIC_ONLY)
   volatile uint32_t tail; //  input index
   volatile uint32_t head; // output index
#endif

private:
   U_DISALLOW_ASSIGN(UVector<void*>)

   friend class UThreadPool;

   template <class T> friend class UOrmTypeHandler;
   template <class T> friend class UJsonTypeHandler;
};

template <class T> class U_EXPORT UVector<T*> : public UVector<void*> {
public:

   void clear() // erase all element
      {
      U_TRACE_NO_PARAM(0+256, "UVector<T*>::clear()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("_length = %u", _length)

      U_INTERNAL_ASSERT(_length <= _capacity)

      if (_length)
         {
         u_destroy<T>((const T**)vec, _length);
                                      _length = 0;
         }
      }

   UVector(uint32_t n = 64U) : UVector<void*>(n)
      {
      U_TRACE_REGISTER_OBJECT(0, UVector<T*>, "%u", n)
      }

   ~UVector()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UVector<T*>)

      clear();
      }

   // ELEMENT ACCESS

   T* back()  { return (T*) UVector<void*>::back(); }
   T* front() { return (T*) UVector<void*>::front(); }

   T*& at(uint32_t pos)       __pure { return (T*&) UVector<void*>::at(pos); }
   T*  at(uint32_t pos) const __pure { return (T*)  UVector<void*>::at(pos); }

   __pure T*& operator[](uint32_t pos)       { return at(pos); }
          T*  operator[](uint32_t pos) const { return at(pos); }

   uint32_t find(void* elem) { return UVector<void*>::find(elem); }

   void replace(uint32_t pos, const T* elem)
      {
      U_TRACE(0, "UVector<T*>::replace(%u,%p)", pos, elem)

      u_construct<T>(&elem, false);

      u_destroy<T>((const T*)vec[pos]);

      UVector<void*>::replace(pos, elem);
      }

   // STACK OPERATIONS

   void push(const T* elem) // add to end
      {
      U_TRACE(0, "UVector<T*>::push(%p)", elem)

      u_construct<T>(&elem, istream_loading);

      UVector<void*>::push(elem);
      }

   void push_back(const T* elem) { push(elem); } 

   T* last() // return last element
      {
      U_TRACE_NO_PARAM(0, "UVector<T*>::last()")

      T* elem = (T*) UVector<void*>::last();

      U_RETURN_POINTER(elem, T);
      }

   T* pop() // remove last element
      {
      U_TRACE_NO_PARAM(0, "UVector<T*>::pop()")

      T* elem = (T*) UVector<void*>::pop();

      U_RETURN_POINTER(elem, T);
      }

   T* pop_front() { return pop(); } // remove last element

   // LIST OPERATIONS

   void insert(uint32_t pos, const T* elem) // add elem before pos
      {
      U_TRACE(0, "UVector<T*>::insert(%u,%p)", pos, elem)

      u_construct<T>(&elem, false);

      UVector<void*>::insert(pos, elem);
      }

   void insert(uint32_t pos, uint32_t n, const T* elem) // add n copy of elem before pos
      {
      U_TRACE(0, "UVector<T*>::insert(%u,%u,%p)", pos, n, elem)

      u_construct<T>(elem, n);

      UVector<void*>::insert(pos, n, elem);
      }

   void erase(uint32_t pos)
      {
      U_TRACE(0, "UVector<T*>::erase(%u)", pos)

      u_destroy<T>((const T*)vec[pos]);

      UVector<void*>::erase(pos);
      }

   void erase(uint32_t first, uint32_t _last) // erase [first,last[
      {
      U_TRACE(0, "UVector<T*>::erase(%u,%u)",  first, _last)

      u_destroy<T>((const T**)(vec+first), _last - first);

      UVector<void*>::erase(first, _last);
      }

   T* remove(uint32_t pos)
      {
      U_TRACE(0, "UVector<T*>::remove(%u)", pos)

      return (T*) UVector<void*>::remove(pos);
      }

   // ASSIGNMENTS

   void assign(uint32_t n, const T* elem)
      {
      U_TRACE(0, "UVector<T*>::assign(%u,%p)", n, elem)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(n, 0)
      U_INTERNAL_ASSERT(_length <= _capacity)

      u_construct<T>(elem, n);

      u_destroy<T>((const T**)vec, U_min(n, _length));

      if (n > _capacity)
         {
         UVector<void*>::deallocate();
         UVector<void*>::allocate(n);
         }

      for (uint32_t i = 0; i < n; ++i) vec[i] = elem;

      _length = n;
      }

#if defined(U_RING_BUFFER) && !defined(U_STATIC_ONLY)
   bool put(const T* elem) // queue an element at the end
      {
      U_TRACE(0, "UVector<T*>::put(%p)", elem)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("head = %u tail = %u", head, tail)

      U_INTERNAL_ASSERT_MINOR(head, _capacity)
      U_INTERNAL_ASSERT_MINOR(tail, _capacity)

      // Producer only: updates tail index after writing

      uint32_t nextTail = (tail + 1) % _capacity;

      // changes only the tail, but verifies that queue is not full (check of head)

      U_INTERNAL_DUMP("nextTail = %u head = %u", nextTail, head)

      if (nextTail != head)
         {
         u_construct<T>(&elem, false);

         vec[tail] = elem;

         tail = nextTail;

         U_RETURN(true);
         }

      // queue was full

      U_RETURN(false);
      }

   bool get(const T*& elem) // dequeue the element off the front
      {
      U_TRACE(0, "UVector<T*>::get(%p)", &elem)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("head = %u tail = %u", head, tail)

      U_INTERNAL_ASSERT_MINOR(head, _capacity)
      U_INTERNAL_ASSERT_MINOR(tail, _capacity)

      // changes only the head but verifies that the queue is not empty (check of tail)

      if (head == tail) U_RETURN(false); // empty queue

      // Consumer only: updates head index after reading

      uint32_t nextHead = (head + 1) % _capacity;

      U_INTERNAL_DUMP("nextHead = %u", nextHead)

      elem = (const T*) vec[head];

      head = nextHead;

      U_RETURN(true);
      }
#endif

   /**
    * BINARY HEAP
    *
    * A binary heap can be used to find the C (where C <= n) smallest numbers out of n input numbers without sorting the entire input.
    *
    * The binary heap is a heap ordered binary tree. A binary tree allows each node in the tree to have two children. Each node has a
    * value associated with it, called its key. The term `heap ordered' means that no child in the tree has a key greater than the key
    * of its parent. By maintaining heap order in the tree, the root node has the smallest key. Because the heap has simple access to
    * the minimum node, the find_min() operation to takes O(1) time. Because of the binary heaps simplicity, it is possible to maintain
    * it using one dimensional arrays. The root node is located at position 1 in the array. The first child of the root is located at
    * position 2 and the second child at position 3. In general, the children of the node at position i are located at 2*i and 2*i + 1.
    * So the children of the node at position 3 in the array are located at positions 6 and 7. Similarly, the parent of the node at
    * position i is located at i div 2.
    *
    * Note that array entry 0 is unused
    */

   T* bh_min() const __pure { return (T*) UVector<void*>::bh_min(); }

   void bh_put(const T* elem)
      {
      U_TRACE(0, "UVector<T*>::bh_put(%p)", elem)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT(_length <= _capacity)

      u_construct<T>(&elem, false);

      if (++_length == _capacity) reserve(_capacity * 2);

      // i - insertion point
      // j - parent of i
      // y - parent's entry in the heap

      T* y;
      uint32_t j;

      // i initially indexes the new entry at the bottom of the heap

      uint32_t i = _length;

      // Stop if the insertion point reaches the top of the heap

      while (i >= 2)
         {
         // j indexes the parent of i. y is the parent's entry

         j = i / 2;
         y = (T*) vec[j];

         // We have the correct insertion point when the item is >= parent
         // Otherwise we move the parent down and insertion point up

         if (*((T*)elem) >= *y) break;

         vec[i] = y;

         i = j;
         }

      // Insert the new item at the insertion point found

      vec[i] = elem;
      }

   T* bh_get()
      {
      U_TRACE_NO_PARAM(0, "UVector<T*>::bh_get()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
      U_INTERNAL_ASSERT(_length <= _capacity)

      if (_length)
         {
         T* elem = bh_min();

         // y - the heap entry of the root
         // j - the current insertion point for the root
         // k - the child of the insertion point
         // z - heap entry of the child of the insertion point

         T* z;

         // Get the value of the root and initialise the insertion point and child

         T* y       = (T*)vec[_length--];
         uint32_t j = 1;
         uint32_t k = 2 * 1;

         // sift-up only if there is a child of the insertion point

         while (k <= _length)
            {
            // Choose the minimum child unless there is only one

            z = (T*) vec[k];

            if (k < _length)
               {
               if (*z > *((T*)vec[k+1])) z = (T*) vec[++k];
               }

            // We stop if the insertion point for the root is in the correct place.
            // Otherwise the child goes up and the root goes down. (i.e. swap)

            if (*y <= *z) break;

            vec[j] = z;

            j = k;
            k = 2 * j;
            }

         // Insert the root in the correct place in the heap

         vec[j] = y;

         U_RETURN_POINTER(elem, T);
         }

      U_RETURN_POINTER(0, T);
      }

   // EXTENSION

   uint32_t find(bPFpv function)
      {
      U_TRACE(0, "UVector<T*>::find(%p)", function)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("_length = %u", _length)

      T* elem;

      for (uint32_t i = 0; i < _length; ++i)
         {
         elem = at(i);

         if (function(elem)) U_RETURN(i);
         }

      U_RETURN(U_NOT_FOUND);
      }

   void move(UVector<T*>& source) { UVector<void*>::move(source); } // add to end and reset source

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend istream& operator>>(istream& is, UVector<T*>& v)
      {
      U_TRACE(0+256,"UVector<T*>::operator>>(%p,%p)", &is, &v)

      U_INTERNAL_ASSERT_MAJOR(v._capacity,0)
      U_INTERNAL_ASSERT(is.peek() == '[' || is.peek() == '(')

      int c = EOF;

      if (is.good())
         {
         istream_loading = true; // NB: we need this flag for distinguish this operation in type's ctor...

         T* _elem;

         U_NEW(T, _elem, T);

         streambuf* sb = is.rdbuf();

         c = sb->sbumpc(); // skip '[' or '('

         while (c != EOF)
            {
            do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

         // U_INTERNAL_DUMP("c = %C", c)

            if (c == ')' ||
                c == ']' ||
                c == EOF)
               {
               break;
               }

            if (c == '#')
               {
               do { c = sb->sbumpc(); } while (c != '\n' && c != EOF); // skip line comment

               continue;
               }

            U_INTERNAL_ASSERT_EQUALS(u__isspace(c),false)

            sb->sputbackc(c);

            is >> *_elem;

            if (is.bad()) is.clear();
            else          v.push(_elem);
            }

         u_destroy<T>((const T*)_elem);

         istream_loading = false;
         }

      if (c == EOF) is.setstate(ios::eofbit);

   // -------------------------------------------------
   // NB: we can load an empty vector
   // -------------------------------------------------
   // if (v._length == 0) is.setstate(ios::failbit);
   // -------------------------------------------------

      return is;
      }

   friend ostream& operator<<(ostream& _os, const UVector<T*>& v)
      {
      U_TRACE(0+256, "UVector<T*>::operator<<(%p,%p)", &_os, &v)

      _os.put('(');
      _os.put(' ');

      for (const void** ptr = v.vec; ptr < (v.vec + v._length); ++ptr)
         {
         _os << *((T*)(*ptr));

         _os.put(' ');
         }

      _os.put(')');

      return _os;
      }

# ifdef DEBUG
   const char* dump(bool reset) const { return UVector<void*>::dump(reset); }
# endif
#endif

private:
   U_DISALLOW_ASSIGN(UVector<T*>)

   friend class UThreadPool;
};

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
class UVectorStringIter { // this class is to make work Range-based for loop: for ( UString x : UVector<UString> ) loop_statement      
public:
   explicit UVectorStringIter(const UVector<UString>* p_vec, uint32_t pos) : _pos(pos), _p_vec(p_vec) {}

   // these three methods form the basis of an iterator for use with a range-based for loop
   bool operator!=(const UVectorStringIter& other) const { return (_pos != other._pos); }

   // this method must be defined after the definition of UVector<UString> since it needs to use it
   inline UString operator*() const;

   const UVectorStringIter& operator++()
      {
      ++_pos;

      // although not strictly necessary for a range-based for loop
      // following the normal convention of returning a value from
      // operator++ is a good idea

      return *this;
      }

private:
   uint32_t _pos;
   const UVector<UString>* _p_vec;
};
#endif

template <> class U_EXPORT UVector<UString> : public UVector<UStringRep*> {
public:

   explicit UVector(uint32_t n = 64) : UVector<UStringRep*>(n)
      {
      U_TRACE_REGISTER_OBJECT(0, UVector<UString>, "%u", n)
      }

   explicit UVector(const UString& str,       char  delim);
   explicit UVector(const UString& str, const char* delim = 0);

   explicit UVector(UVector<UString>& source, uint32_t n) : UVector<UStringRep*>(n)
      {
      U_TRACE_REGISTER_OBJECT(0, UVector<UString>, "%p,%u", &source, n)

      UVector<void*>::move(source); // add to end and reset source
      }

   ~UVector()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UVector<UString>)

      U_ASSERT(check_memory())
      }

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
    UVectorStringIter begin() const { return UVectorStringIter(this, 0); }
    UVectorStringIter   end() const { return UVectorStringIter(this, _length); }

# ifdef U_COMPILER_RANGE_FOR
   explicit UVector(const std::initializer_list<UString>& l) : UVector<UStringRep*>(l.size())
      {
      U_TRACE(0, "UVector<UString>::UVector<UString>(%p)", &l)

      for (UString item : l) push_back(item);
      }
# endif
#endif

   // ELEMENT ACCESS

   UString front() { return UString(UVector<UStringRep*>::front()); }
   UString  back() { return UString(UVector<UStringRep*>::back()); }

   UString at(uint32_t pos) const __pure
      {
      U_TRACE(0, "UVector<UString>::at(%u)", pos)

      UString result(UVector<UStringRep*>::at(pos));

      U_RETURN_STRING(result);
      }

   UString operator[](uint32_t pos) const
      {
      U_TRACE(0, "UVector<UString>::operator[](%u)", pos)

#  ifdef DEBUG
      if (pos >= _length)
         {
         U_ERROR("Array access out of bounds - UVector<UString>::at(pos:%u >= _length:%u) _capacity = %u elem(0) = %V elem(%u) = %V",
                  pos, _length, _capacity, vec[0], _length-1, (_length ? vec[_length-1] : UStringRep::string_rep_null));
         }
#  endif

      return at(pos);
      }

   char* c_pointer(uint32_t pos)
      {
      U_TRACE(0, "UVector<UString>::c_pointer(%u)", pos)

      U_CHECK_MEMORY

      if (empty()) return 0;

      UStringRep* rep = UVector<UStringRep*>::at(pos);

      return rep->data();
      }

   void replace(uint32_t pos, const UString& str)
      {
      U_TRACE(0, "UVector<UString>::replace(%u,%V)", pos, str.rep)

      UVector<UStringRep*>::replace(pos, str.rep);
      }

   // STACK OPERATIONS

   void push(const UString& str) // add to end
      {
      U_TRACE(0, "UVector<UString>::push(%V)", str.rep)

      UVector<UStringRep*>::push(str.rep);

      U_INTERNAL_DUMP("str.rep = %p at(%u) = %p", str.rep, _length-1, UVector<UStringRep*>::at(_length-1))

      U_ASSERT_EQUALS(str.rep, UVector<UStringRep*>::at(_length-1))
      }

   void push_back(const UString& str) { push(str); } // add to end

   void push(     const UStringRep* rep) { UVector<UStringRep*>::push(rep); }
   void push_back(const UStringRep* rep) { UVector<UStringRep*>::push(rep); }

   void push(     const char* t, uint32_t tlen) { UString str(t, tlen); push(str); }
   void push_back(const char* t, uint32_t tlen) { push(t, tlen); }

   UString last() // return last element
      {
      U_TRACE_NO_PARAM(0, "UVector<UString>::last()")

      UStringRep* rep = UVector<UStringRep*>::last();

      U_INTERNAL_ASSERT_POINTER(rep)

      UString str(rep);

      U_RETURN_STRING(str);
      }

   UString pop() // remove last element
      {
      U_TRACE_NO_PARAM(0, "UVector<UString>::pop()")

      UStringRep* rep = UVector<UStringRep*>::pop();

      U_INTERNAL_ASSERT_POINTER(rep)

      UString str(rep);

      rep->release();

      U_RETURN_STRING(str);
      }

   UString pop_front() { return pop(); } // remove last element

   // LIST OPERATIONS

   void insert(uint32_t pos, const UString& str) // add elem before pos
      {
      U_TRACE(0, "UVector<UString>::insert(%u,%V)", pos, str.rep)

      UVector<UStringRep*>::insert(pos, str.rep);
      }

   void insert(uint32_t pos, uint32_t n, const UString& str) // add n copy of elem before pos
      {
      U_TRACE(0, "UVector<UString>::insert(%u,%u,%V)", pos, n, str.rep)

      UVector<UStringRep*>::insert(pos, n, str.rep);
      }

   void erase(uint32_t pos) // remove element at pos
      {
      U_TRACE(0, "UVector<UString>::erase(%u)", pos)

      UVector<UStringRep*>::erase(pos);
      }

   void erase(uint32_t first, uint32_t _last) // erase [first,last[
      {
      U_TRACE(0, "UVector<UString>::erase(%u,%u)",  first, _last)

      UVector<UStringRep*>::erase(first, _last);
      }

   // ASSIGNMENTS

   void assign(uint32_t n, const UString& str)
      {
      U_TRACE(0, "UVector<UString>::assign(%u,%V)", n, str.rep)

      UVector<UStringRep*>::assign(n, str.rep);
      }

#if defined(U_RING_BUFFER) && !defined(U_STATIC_ONLY)
   bool put(const UString& str) // queue an element at the end
      {
      U_TRACE(0, "UVector<UString>::put(%V)", str.rep)

      return UVector<UStringRep*>::put(str.rep);
      }

   bool get(UString& str) // dequeue the element off the front
      {
      U_TRACE(0, "UVector<UString>::get(%p)", &str)

      const UStringRep* rep;

      if (UVector<UStringRep*>::get(rep))
         {
         str._set((UStringRep*)rep);

         U_RETURN(true);
         }

      U_RETURN(false);
      }
#endif

   // BINARY HEAP

   UString bh_min() const __pure { return UString(UVector<UStringRep*>::bh_min()); }

   void bh_put(const UString& str)
      {
      U_TRACE(0, "UVector<UString>::bh_put(%V)", str.rep)

      UVector<UStringRep*>::bh_put(str.rep);
      }

   UString bh_get()
      {
      U_TRACE_NO_PARAM(0, "UVector<UString>::bh_get()")

      UStringRep* rep = UVector<UStringRep*>::bh_get();

      if (rep)
         {
         UString str(rep);

         rep->release();

         U_RETURN_STRING(str);
         }

      return UString::getStringNull();
      }

   // EXTENSION

   UString join(      char  delim);
   UString join(const char* delim, uint32_t delim_len);

   uint32_t split(const    char* str, uint32_t len,       char  delim);
   uint32_t split(const UString& str,                     char  delim);     // NB: use substr(), so dependency from str...
   uint32_t split(const    char* str, uint32_t len, const char* delim = 0);
   uint32_t split(const UString& str,               const char* delim = 0); // NB: use substr(), so dependency from str...

   uint32_t loadFromData(const UString& str) { (void) loadFromData(U_STRING_TO_PARAM(str)); return _length; }

   // NB: find() are based on UString::equal()...

   uint32_t find(      const UString& str, bool ignore_case = false) __pure;
   uint32_t findSorted(const UString& str, bool ignore_case = false, bool bcouple = false) __pure;

   uint32_t find(     const char* s, uint32_t n) __pure;
   uint32_t findRange(const char* s, uint32_t n, uint32_t start, uint32_t end) __pure;

   // Check equality with string at pos

   bool isEqual(uint32_t pos, const UString& str, bool ignore_case = false) __pure;

   // Check equality with an existing vector object

   bool isEqual(UVector<UString>& _vec, bool ignore_case = false)
      {
      U_TRACE(0, "UVector<UString>::isEqual(%p,%b)", &_vec, ignore_case)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("_length = %u", _length)

      if (_length != _vec.size()) U_RETURN(false);

      return _isEqual(_vec, ignore_case); 
      }

   void sort(bool ignore_case = false)
      {
      U_TRACE(0, "UVector<UString>::sort(%b)", ignore_case)

      U_INTERNAL_DUMP("_length = %u", _length)

   // U_INTERNAL_ASSERT_RANGE(2,_length,_capacity)

      if (ignore_case == false) mksort((UStringRep**)vec, _length, 0);
      else                      UVector<void*>::sort();
      }

   void move(UVector<UString>& source) { UVector<void*>::move(source); } // add to end and reset source

   // AS SET

   void insertAsSet(const UString& str) // no duplicate value
      {
      U_TRACE(0, "UVector<UString>::insertAsSet(%V)", str.rep)

      if (empty() || find(str) == U_NOT_FOUND) push(str);
      }

   uint32_t intersection(UVector<UString>& set1, UVector<UString>& set2);

   // NB: contains() are based on UString::find()...

   uint32_t contains(const UString& str,    bool ignore_case = false);
   bool     contains(UVector<UString>& vec, bool ignore_case = false);
   bool  isContained(const UString& str,    bool ignore_case = false) { return (contains(str, ignore_case) != U_NOT_FOUND); }

   // OPERATOR

   bool operator==(const UVector<UString>& v) const __pure;
   bool operator!=(const UVector<UString>& v) const { return ! operator==(v); }

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UVector<UString>& v);
   friend          ostream& operator<<(ostream& os, const UVector<UString>& v) { return operator<<(os, (const UVector<UStringRep*>&)v); }
#endif

protected:
   uint32_t loadFromData(const char* start, uint32_t size);

   UStringRep* getStringRep(uint32_t pos) { return UVector<UStringRep*>::at(pos); }

private:
   static void mksort(UStringRep** a, int n, int depth);
          bool _isEqual(UVector<UString>& vec, bool ignore_case);

   U_DISALLOW_ASSIGN(UVector<UString>)

   friend class UHTTP;
   friend class UHttpPlugIn;
   friend class UFileConfig;
   friend class UNoCatPlugIn;
};

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
inline UString UVectorStringIter::operator* () const { return _p_vec->at(_pos); }
#endif
#endif
