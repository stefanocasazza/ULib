// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tree.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_TREE_H
#define ULIB_TREE_H 1

#include <ulib/container/vector.h>

template <class T> class UTree;

template <> class U_EXPORT UTree<void*> : public UVector<void*> {
public:

   UTree(const void* __elem = 0, const void* __parent = 0, uint32_t n = 1) : UVector<void*>(n)
      {
      U_TRACE_REGISTER_OBJECT(0, UTree<void*>, "%p,%p,%u", __elem, __parent, n)

      _elem   = __elem;
      _parent = __parent;
      }

   ~UTree()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTree<void*>)
      }

   // ACCESS

   const void* elem() const { return _elem; }

   UTree<void*>*   parent() const                      { return (UTree<void*>*)_parent; }
   UVector<void*>* vector() const                      { return (UVector<void*>*)this; }
   UTree<void*>*   childAt(uint32_t pos) const  __pure { return (UTree<void*>*)vector()->at(pos); }

   // SERVICES

   bool null() const
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::null()")

      U_RETURN(_elem == 0);
      }

   bool root() const
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::root()")

      U_RETURN(_parent == 0);
      }

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::empty()")

      U_RETURN(_elem == 0 && _length == 0);
      }

   uint32_t numChild() const
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::numChild()")

      U_RETURN(_length);
      }

   // compute the depth to the root

   uint32_t depth() const __pure
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::depth()")

      uint32_t result = 0;
      const UTree<void*>* p = this;

      while (p->parent() != 0)
         {
         ++result;

         p = p->parent();
         }

      U_RETURN(result);
      }

   // OPERATIONS

   void setRoot(const void* __elem)
      {
      U_TRACE(0, "UTree<void*>::setRoot(%p)", __elem)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(_parent, 0)

      _elem = __elem;
      }

   void setParent(const void* __parent)
      {
      U_TRACE(0, "UTree<void*>::setParent(%p)", __parent)

      U_CHECK_MEMORY

      _parent = __parent;
      }

   // STACK

   UTree<void*>* push(const void* __elem) // add to end
      {
      U_TRACE(0, "UTree<void*>::push(%p)", __elem)

      UTree<void*>* p;

      U_NEW(UTree<void*>, p, UTree<void*>(__elem, this, (size_allocate ? : 64)));

      UVector<void*>::push(p);

      U_RETURN_POINTER(p,UTree<void*>);
      }

   UTree<void*>* push_back(const void* __elem)
      {
      U_TRACE(0, "UTree<void*>::push_back(%p)", __elem)

      if (_parent == 0 &&
          _elem   == 0)
         {
         _elem = __elem;

         return this;
         }

      return push(__elem);
      }

   UTree<void*>* last() // return last element
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::last()")

      return (UTree<void*>*) UVector<void*>::last();
      }

   UTree<void*>* pop() // remove last element
      {
      U_TRACE_NO_PARAM(0, "UTree<void*>::pop()")

      return (UTree<void*>*) UVector<void*>::pop();
      }

   // LIST

   UTree<void*>* insert(uint32_t pos, const void* __elem) // add elem before pos
      {
      U_TRACE(0, "UTree<void*>::insert(%u,%p)", pos, __elem)

      UTree<void*>* p;

      U_NEW(UTree<void*>, p, UTree<void*>(__elem, this));

      UVector<void*>::insert(pos, p);

      U_RETURN_POINTER(p,UTree<void*>);
      }

   void erase(uint32_t pos) // remove element at pos
      {
      U_TRACE(0, "UTree<void*>::erase(%u)", pos)

      delete (UTree<void*>*) vec[pos];

      UVector<void*>::erase(pos);
      }

   // Call function for all entry

   void callForAllEntry(vPFpvpv function);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   static uint32_t size_allocate;

protected:
   const void* _elem;
   const void* _parent;

private:
   U_DISALLOW_ASSIGN(UTree<void*>)
};

template <class T> class U_EXPORT UTree<T*> : public UTree<void*> {
public:

   void clear() // erase all element
      {
      U_TRACE_NO_PARAM(0, "UTree<T*>::clear()")

      if (_elem)
         {
         u_destroy<T>((const T*)_elem);

         _elem = 0;
         }

      if (UVector<void*>::empty() == false)
         {
         const void** _end = vec + _length;

         for (const void** ptr = vec; ptr < _end; ++ptr) delete (UTree<T*>*)(*ptr);

         _length = 0;
         }
      }

   UTree(const T* __elem = 0, const T* __parent = 0, uint32_t n = 1) : UTree<void*>(__elem, __parent, n)
      {
      U_TRACE_REGISTER_OBJECT(0, UTree<T*>, "%p,%p,%u", __elem, __parent, n)
      }

   ~UTree()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTree<T*>)

      clear();
      }

   // ACCESS

   T* elem() const { return (T*) _elem; }

   UTree<T*>*   parent() const                      { return (UTree<T*>*)_parent; }
   UVector<T*>* vector() const                      { return (UVector<T*>*)this; }
   UTree<T*>*   childAt(uint32_t pos) const  __pure { return (UTree<T*>*)((UVector<void*>*)this)->at(pos); }

   T* front() { return ((UTree<T*>*)UVector<void*>::front())->elem(); }
   T*  back() { return ((UTree<T*>*)UVector<void*>::back())->elem(); }

   T* at(uint32_t pos) const { return ((UTree<T*>*)UVector<void*>::at(pos))->elem(); }

   T* operator[](uint32_t pos) const { return at(pos); }

   // OPERATIONS

   void setRoot(const T* __elem)
      {
      U_TRACE(0, "UTree<T*>::setRoot(%p)", __elem)

      u_construct<T>(&__elem, false);

      UTree<void*>::setRoot(__elem);
      }

   // STACK

   UTree<T*>* push(const T* __elem) // add to end
      {
      U_TRACE(0, "UTree<T*>::push(%p)", __elem)

      u_construct<T>(&__elem, false);

      return (UTree<T*>*) UTree<void*>::push(__elem);
      }

   UTree<T*>* push_back(const T* __elem)
      {
      U_TRACE(0, "UTree<T*>::push_back(%p)", __elem)

      if (_parent == 0 &&
          _elem   == 0)
         {
         setRoot(__elem);

         return this;
         }

      return push(__elem);
      }

   UTree<T*>* last() // return last element
      {
      U_TRACE_NO_PARAM(0, "UTree<T*>::last()")

      return (UTree<T*>*) UVector<void*>::last();
      }

   UTree<T*>* pop() // remove last element
      {
      U_TRACE_NO_PARAM(0, "UTree<T*>::pop()")

      return (UTree<T*>*) UVector<void*>::pop();
      }

   // LIST

   UTree<T*>* insert(uint32_t pos, const T* __elem) // add elem before pos
      {
      U_TRACE(0, "UTree<T*>::insert(%u,%p)", pos, __elem)

      u_construct<T>(&__elem, false);

      return (UTree<T*>*) UTree<void*>::insert(pos, __elem);
      }

   void erase(uint32_t pos) // remove element at pos
      {
      U_TRACE(0, "UTree<T*>::erase(%u)", pos)

      delete (UTree<T*>*) vec[pos];

      UVector<void*>::erase(pos);
      }

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& os, const UTree<T*>& t)
      {
      U_TRACE(0+256, "UTree<T*>::operator<<(%p,%p)", &os, &t)

      for (uint32_t i = 0; i < t.depth(); ++i)
         {
         os.put('\t');
         }

      os.put('[');

      if (t.null() == false)
         {
         os.put(' ');

         os << *(t.elem());
         }

      if (t.UVector<void*>::empty() == false)
         {
         UTree<T*>* p;

         for (const void** ptr = t.vec; ptr < (t.vec + t._length); ++ptr)
            {
            p = (UTree<T*>*)(*ptr);

            os.put('\n');

            os << *p;
            }
         }

      os.put(' ');
      os.put(']');

      return os;
      }

# ifdef DEBUG
   const char* dump(bool reset) const { return UTree<void*>::dump(reset); }
# endif
#endif

private:
   U_DISALLOW_ASSIGN(UTree<T*>)
};

template <> class U_EXPORT UTree<UString> : public UTree<UStringRep*> {
public:

   UTree(uint32_t n = 64) : UTree<UStringRep*>(0, 0, n)
      {
      U_TRACE_REGISTER_OBJECT(0, UTree<UString>, "%u", n)
      }

   ~UTree()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTree<UString>)
      }

   // ACCESS

   UString elem() const
      {
      U_TRACE_NO_PARAM(0, "UTree<UString>::elem()")

      if (_elem)
         {
         UString str((UStringRep*)_elem);

         U_RETURN_STRING(str);
         }

      return UString::getStringNull();
      }

   UTree<UString>*   parent() const                       { return (UTree<UString>*)_parent; }
   UVector<UString>* vector() const                       { return (UVector<UString>*)this; }
   UTree<UString>*   childAt(uint32_t pos) const   __pure { return (UTree<UString>*)((UVector<void*>*)this)->at(pos); }

   UString  back();
   UString front() { return ((UTree<UString>*)UVector<void*>::front())->elem(); }

   UString at(uint32_t pos) const { return ((UTree<UString>*)UVector<void*>::at(pos))->elem(); }

   UString operator[](uint32_t pos) const { return at(pos); }

   // OPERATIONS

   void setRoot(const UString& str)
      {
      U_TRACE(0, "UTree<UString>::setRoot(%V)", str.rep)

      UTree<UStringRep*>::setRoot(str.rep);
      }

   // STACK

   UTree<UString>* push(const UString& str)
      {
      U_TRACE(0, "UTree<UString>::push(%V)", str.rep)

      return (UTree<UString>*) UTree<UStringRep*>::push(str.rep);
      }

   UTree<UString>* push_back(const UString& str)
      {
      U_TRACE(0, "UTree<UString>::push_back(%V)", str.rep)

      return (UTree<UString>*) UTree<UStringRep*>::push_back(str.rep);
      }

   UTree<UString>* last()
      {
      U_TRACE_NO_PARAM(0, "UTree<UString>::last()")

      return (UTree<UString>*) UTree<UStringRep*>::last();
      }

   UTree<UString>* pop()
      {
      U_TRACE_NO_PARAM(0, "UTree<UString>::pop()")

      return (UTree<UString>*) UTree<UStringRep*>::pop();
      }

   // LIST

   void insert(uint32_t pos, const UString& str); // add elem before pos

   // EXTENSION

   uint32_t find(const UString& str) __pure;

   // STREAMS

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UTree<UString>& t);
   friend          ostream& operator<<(ostream& os, const UTree<UString>& t) { return operator<<(os, (const UTree<UStringRep*>&)t); }
#endif

private:
   U_DISALLOW_ASSIGN(UTree<UString>)
};

#endif
