// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    vector.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/string_ext.h>

#if defined(ENABLE_MEMPOOL) && defined(U_LINUX)
#  include <ulib/file.h>
#endif

bool UVector<void*>::istream_loading;

void UVector<void*>::push(const void* elem) // add to end
{
   U_TRACE(0, "UVector<void*>::push(%p)", elem)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u _capacity = %u", _length, _capacity)

   U_INTERNAL_ASSERT_MAJOR(_capacity, 0)
   U_INTERNAL_ASSERT(_length <= _capacity)

   if (_length == _capacity)
      {
      const void** old_vec  = vec;
      uint32_t old_capacity = _capacity;

      _capacity <<= 1; // x 2...

      vec = (const void**) UMemoryPool::_malloc(&_capacity, sizeof(void*));

      if (_length) U_MEMCPY(vec, old_vec, _length * sizeof(void*));

      UMemoryPool::_free(old_vec, old_capacity, sizeof(void*));
      }

   vec[_length++] = elem;
}

void UVector<void*>::move(UVector<void*>& source) // add to end and reset source
{
   U_TRACE(0, "UVector<void*>::move(%p)", &source)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MAJOR(source._length, 0)

   if ((_length + source._length) >= _capacity)
      {
      const void** old_vec  = vec;
      uint32_t old_capacity = _capacity;

      _capacity <<= 1; // x 2...

      vec = (const void**) UMemoryPool::_malloc(&_capacity, sizeof(void*));

      if (_length) U_MEMCPY(vec, old_vec, _length * sizeof(void*));

      UMemoryPool::_free(old_vec, old_capacity, sizeof(void*));
      }

   U_MEMCPY(vec+_length, source.vec, source._length * sizeof(void*));

   _length += source._length;
              source._length = 0;
}

// LIST OPERATIONS

void UVector<void*>::insert(uint32_t pos, const void* elem) // add elem before pos
{
   U_TRACE(0, "UVector<void*>::insert(%u,%p)", pos, elem)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(pos <= _length)
   U_INTERNAL_ASSERT(_length <= _capacity)

   if (_length == _capacity)
      {
      const void** old_vec  = vec;
      uint32_t old_capacity = _capacity;

      _capacity <<= 1; // x 2...

      vec = (const void**) UMemoryPool::_malloc(&_capacity, sizeof(void*));

      U_MEMCPY(vec,           old_vec,                  pos  * sizeof(void*));
      U_MEMCPY(vec + pos + 1, old_vec + pos, (_length - pos) * sizeof(void*));

      UMemoryPool::_free(old_vec, old_capacity, sizeof(void*));
      }
   else
      {
#  ifdef U_APEX_ENABLE
      (void) U_SYSCALL(apex_memmove, "%p,%p,%u", vec + pos + 1, vec + pos, (_length - pos) * sizeof(void*));
#  else
      (void) U_SYSCALL(     memmove, "%p,%p,%u", vec + pos + 1, vec + pos, (_length - pos) * sizeof(void*));
#  endif
      }

   vec[pos] = elem;

   ++_length;
}

void UVector<void*>::insert(uint32_t pos, uint32_t n, const void* elem) // add n copy of elem before pos
{
   U_TRACE(0, "UVector<void*>::insert(%u,%u,%p)", pos, n, elem)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MAJOR(n,0)
   U_INTERNAL_ASSERT(pos <= _length)
   U_INTERNAL_ASSERT(_length <= _capacity)

   uint32_t new_length = _length + n;

   if (new_length > _capacity)
      {
      const void** old_vec  = vec;
      uint32_t old_capacity = _capacity;

      _capacity = new_length << 1; // x 2...

      vec = (const void**) UMemoryPool::_malloc(&_capacity, sizeof(void*));

      U_MEMCPY(vec,           old_vec,                  pos  * sizeof(void*));
      U_MEMCPY(vec + pos + n, old_vec + pos, (_length - pos) * sizeof(void*));

      UMemoryPool::_free(old_vec, old_capacity, sizeof(void*));
      }
   else
      {
#  ifdef U_APEX_ENABLE
      (void) U_SYSCALL(apex_memmove, "%p,%p,%u", vec + pos + n, vec + pos, (_length - pos) * sizeof(void*));
#  else
      (void) U_SYSCALL(     memmove, "%p,%p,%u", vec + pos + n, vec + pos, (_length - pos) * sizeof(void*));
#  endif
      }

   for (uint32_t i = 0; i < n; ++i) vec[pos++] = elem;

   _length = new_length;
}

void UVector<void*>::reserve(uint32_t n)
{
   U_TRACE(0, "UVector<void*>::reserve(%u)", n)

   if (n != _capacity)
      {
      if (n == 0) n = 64; // NB: the check n == 0 is specific for class UTree...

      const void** old_vec  = vec;
      uint32_t old_capacity = _capacity;

      allocate(n);

      if (_length) U_MEMCPY(vec, old_vec, _length * sizeof(void*));

      if (old_capacity) UMemoryPool::_free(old_vec, old_capacity, sizeof(void*));
      }
}

#ifdef DEBUG
bool UVector<void*>::check_memory() // check all element
{
   U_TRACE_NO_PARAM(0+256, "UVector<void*>::check_memory()")

   U_CHECK_MEMORY

   const void* pelem;

   for (uint32_t i = 0; i < _length; ++i)
      {
      pelem = at(i);

      U_INTERNAL_ASSERT_EQUALS(((const UMemoryError*)pelem)->_this, (void*)U_CHECK_MEMORY_SENTINEL)
      }

   U_RETURN(true);
}
#endif

UVector<UString>::UVector(const UString& str, char delim) : UVector<UStringRep*>(64)
{
   U_TRACE_REGISTER_OBJECT(0, UVector<UString>, "%V,%C", str.rep, delim)

   uint32_t n = str.size() / 128;

   if (n > 64)
      {
#  if defined(ENABLE_MEMPOOL) && defined(U_LINUX)
      UMemoryPool::allocateMemoryBlocks(U_SIZE_TO_STACK_INDEX(sizeof(UStringRep)), UFile::getSizeAligned(n * sizeof(UStringRep)) / sizeof(UStringRep));
#  endif

      UMemoryPool::_free(vec, _capacity, sizeof(void*));

      allocate(n);
      }

   (void) split(str, delim);

   if (_length) reserve(_length);
}

UVector<UString>::UVector(const UString& x, const char* delim) : UVector<UStringRep*>(64)
{
   U_TRACE_REGISTER_OBJECT(0, UVector<UString>, "%V,%S", x.rep, delim)

   const char* s = x.data();
         char  c = *s;

   if (c == '[' ||
       c == '(')
      {
      istream_loading = true; // NB: we need this flag for distinguish this operation in UString::setFromData()...

      (void) loadFromData(s, x.size());

      istream_loading = false;
      }
   else
      {
      (void) split(x, delim); // NB: use substr(), so dependency from x...
      }

   if (_length) reserve(_length);
}

__pure uint32_t UVector<UString>::find(const char* s, uint32_t n)
{
   U_TRACE(0, "UVector<UString>::find(%.*S,%u)", n, s, n)

   U_CHECK_MEMORY

   UStringRep* r;

   for (uint32_t i = 0; i < _length; ++i)
      {
      r = UVector<UStringRep*>::at(i);

      if (r->equal(s, n)) U_RETURN(i);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UVector<UString>::findRange(const char* s, uint32_t n, uint32_t start, uint32_t _end)
{
   U_TRACE(0+256, "UVector<UString>::findRange(%.*S,%u,%u,%u)", n, s, n, start, _end)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(_end <= _length)
   U_INTERNAL_ASSERT_MINOR(start, _end)

   UStringRep* r;

   for (uint32_t i = start; i < _end; ++i)
      {
      r = UVector<UStringRep*>::at(i);

      if (r->equal(s, n)) U_RETURN(i);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UVector<UString>::find(const UString& str, bool ignore_case)
{
   U_TRACE(0, "UVector<UString>::find(%V,%b)", str.rep, ignore_case)

   U_CHECK_MEMORY

   uint32_t i;
   UStringRep* r;

   if (ignore_case)
      {
      for (i = 0; i < _length; ++i)
         {
         r = UVector<UStringRep*>::at(i);

         if (str.equalnocase(r)) U_RETURN(i);
         }
      }
   else
      {
      for (i = 0; i < _length; ++i)
         {
         r = UVector<UStringRep*>::at(i);

         if (str.equal(r)) U_RETURN(i);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

// Check equality with string at pos

__pure bool UVector<UString>::isEqual(uint32_t pos, const UString& str, bool ignore_case)
{
   U_TRACE(0, "UVector<UString>::isEqual(%u,%V,%b)", pos, str.rep, ignore_case)

   U_CHECK_MEMORY

   if (_length)
      {
      UStringRep* rep = UVector<UStringRep*>::at(pos);

      if (UStringRep::equal_lookup(rep, U_STRING_TO_PARAM(*rep), str.rep, str.size(), ignore_case)) U_RETURN(true);
      }

   U_RETURN(false);
}

__pure uint32_t UVector<UString>::findSorted(const UString& str, bool ignore_case, bool bcouple)
{
   U_TRACE(0, "UVector<UString>::findSorted(%V,%b,%b)", str.rep, ignore_case, bcouple)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT_RANGE(1,_length,_capacity)

   UStringRep* key;
   UStringRep* target = str.rep;
   int old_probe = -1, low = -1, high = _length;
   uint32_t mask = (bcouple ? 0xFFFFFFFE : 0xFFFFFFFF);

   while ((high - low) > 1)
      {
      int probe = (((low + high) & 0xFFFFFFFF) >> 1) & mask;

      U_INTERNAL_DUMP("low = %d high = %d probe = %d old_probe = %d", low, high, probe, old_probe)

      if (probe == old_probe) U_RETURN(U_NOT_FOUND);

      key = UVector<UStringRep*>::at(probe);
      int cmp = (ignore_case ? key->comparenocase(target)
                             : key->compare(      target));

      U_INTERNAL_DUMP("cmp = %d", cmp)

           if (cmp  > 0)   high = probe;
      else if (cmp == 0) U_RETURN(probe);
      else                  low = probe;

      old_probe = probe;
      }

   if (low == -1 || (key = UVector<UStringRep*>::at(low),
                     (ignore_case ? key->comparenocase(target)
                                  : key->compare(target))) != 0)
      {
      U_RETURN(U_NOT_FOUND);
      }

   U_RETURN(low);
}

uint32_t UVector<UString>::contains(const UString& str, bool ignore_case)
{
   U_TRACE(0, "UVector<UString>::contains(%V,%b)", str.rep, ignore_case)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   UString elem;
   uint32_t i, n;

   if (ignore_case)
      {
      for (i = 0; i < _length; ++i)
         {
         elem = at(i);
         n    = elem.findnocase(str);

         if (n != U_NOT_FOUND) U_RETURN(i);
         }
      }
   else
      {
      for (i = 0; i < _length; ++i)
         {
         elem = at(i);
         n    = elem.find(str);

         if (n != U_NOT_FOUND) U_RETURN(i);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

bool UVector<UString>::contains(UVector<UString>& _vec, bool ignore_case)
{
   U_TRACE(0, "UVector<UString>::contains(%p,%b)", &_vec, ignore_case)

   U_CHECK_MEMORY

   UString elem;

   for (uint32_t i = 0, n = _vec.size(); i < n; ++i)
      {
      elem = _vec.at(i);

      if (contains(elem, ignore_case) != U_NOT_FOUND) U_RETURN(true);
      }

   U_RETURN(false);
}

// Check equality with an existing vector object

bool UVector<UString>::_isEqual(UVector<UString>& _vec, bool ignore_case)
{
   U_TRACE(0, "UVector<UString>::_isEqual(%p,%b)", &_vec, ignore_case)

   U_INTERNAL_ASSERT(_length <= _capacity)

   if (_length)
      {
      UString elem;

      for (uint32_t i = 0; i < _length; ++i)
         {
         elem = at(i);

         if (_vec.find(elem, ignore_case) == U_NOT_FOUND) U_RETURN(false);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString UVector<UString>::join(char c)
{
   U_TRACE(0, "UVector<UString>::join(%C)", c)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   if (_length == 0) return UString::getStringNull();

   uint32_t i   = 0,
            len = 0;

   for (; i < _length; ++i) len += ((UStringRep*)vec[i])->size();

   len += (_length - 1);

   UString str(len < U_CAPACITY ? U_CAPACITY : len);

   str.size_adjust(len);

   i = 0;
   char* ptr = str.data();

   while (true)
      {
      UStringRep* rep = (UStringRep*)vec[i];

      uint32_t sz = rep->size();

      if (sz) U_MEMCPY(ptr, rep->data(), sz);

      if (++i >= _length) break;

      ptr += sz;

      *ptr++ = c;
      }

   (void) str.shrink();

   U_RETURN_STRING(str);
}

UString UVector<UString>::join(const char* t, uint32_t tlen)
{
   U_TRACE(0, "UVector<UString>::join(%.*S,%u)", tlen, t, tlen)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   if (_length == 0) return UString::getStringNull();

   uint32_t i   = 0,
            len = 0;

   for (; i < _length; ++i) len += ((UStringRep*)vec[i])->size();

   len += (_length - 1) * tlen;

   UString str(len < U_CAPACITY ? U_CAPACITY : len);

   str.size_adjust(len);

   i = 0;
   char* ptr = str.data();

   while (true)
      {
      UStringRep* rep = (UStringRep*)vec[i];

      uint32_t sz = rep->size();

      if (sz) U_MEMCPY(ptr, rep->data(), sz);

      if (++i >= _length) break;

      ptr += sz;

      if (tlen)
         {
         U_MEMCPY(ptr, t, tlen);

         ptr += tlen;
         }
      }

   (void) str.shrink();

   U_RETURN_STRING(str);
}

uint32_t UVector<UString>::split(const UString& str, const char* delim)
{
   U_TRACE(0, "UVector<UString>::split(%V,%S)", str.rep, delim)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   const char* p;
   UStringRep* r;

   uint32_t len, n  = _length;
   const char* s    = str.data();
   const char* _end = s + str.size();

   if (str.isQuoted())
      {
      ++s;
      --_end;
      }

   while (s < _end)
      {
      s = u_delimit_token(s, &p, _end, delim, '#');

      U_INTERNAL_DUMP("s = %p end = %p", s, _end)

      if (s <= _end)
         {
         len = s++ - p;
         r   = str.rep->substr(p, len);

         U_INTERNAL_DUMP("r = %V", r)

         UVector<void*>::push(r);
         }
      }

   U_RETURN(_length - n);
}

uint32_t UVector<UString>::split(const char* s, uint32_t len, const char* delim)
{
   U_TRACE(0, "UVector<UString>::split(%.*S,%u,%S)", len, s, len, delim)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   const char* p;
   UStringRep* r;

   uint32_t n       = _length;
   const char* _end = s + len;

   if (*s        == '"' &&
       *(_end-1) == '"')
      {
      ++s;
      --_end;
      }

   while (s < _end)
      {
      s = u_delimit_token(s, &p, _end, delim, '#');

      U_INTERNAL_DUMP("s = %p end = %p", s, _end)

      if (s <= _end)
         {
         len = s++ - p;
         r   = UStringRep::create(len, len, p);

         U_INTERNAL_DUMP("r = %V", r)

         UVector<void*>::push(r);
         }
      }

   U_RETURN(_length - n);
}

uint32_t UVector<UString>::split(const UString& str, char delim)
{
   U_TRACE(0, "UVector<UString>::split(%V,%C)", str.rep, delim)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   const char* p;
   UStringRep* r;

   uint32_t n       = _length;
   const char* s    = str.data();
   const char* _end = s + str.size();

   if (delim != ';' && // usp translator (printfor)
       str.isQuoted())
      {
      ++s;
      --_end;
      }

   while (s < _end)
      {
      // skip char delimiter

      if (*s == delim)
         {
         ++s;

         continue;
         }

      // delimit token with char delimiter

      p = s;
      s = (const char*) memchr(s, delim, _end - s);

      if (s == 0) s = _end;

      r = str.rep->substr(p, s - p);

      UVector<void*>::push(r);

      ++s;
      }

   U_RETURN(_length - n);
}

uint32_t UVector<UString>::split(const char* s, uint32_t len, char delim)
{
   U_TRACE(0, "UVector<UString>::split(%.*S,%u,%C)", len, s, len, delim)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   const char* p;
   UStringRep* r;

   uint32_t n       = _length;
   const char* _end = s + len;

   if (*s        == '"' &&
       *(_end-1) == '"')
      {
      ++s;
      --_end;
      }

   while (s < _end)
      {
      // skip char delimiter

      if (*s == delim)
         {
         ++s;

         continue;
         }

      // delimit token with char delimiter

      p = s;
      s = (const char*) memchr(s, delim, _end - s);

      if (s == 0) s = _end;

      len = s++ - p;
      r   = UStringRep::create(len, len, p);

      UVector<void*>::push(r);
      }

   U_RETURN(_length - n);
}

uint32_t UVector<UString>::intersection(UVector<UString>& set1, UVector<UString>& set2)
{
   U_TRACE(0, "UVector<UString>::intersection(%p,%p)", &set1, &set2)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("_length = %u", _length)

   U_INTERNAL_ASSERT(_length <= _capacity)

   if (set1.empty() ||
       set2.empty())
      {
      U_RETURN(0);
      }

   UString elem;
   uint32_t i, n = _length;

   for (i = 0; i < set1._length; ++i)
      {
      elem = set1.at(i);

      if (set2.find(elem) != U_NOT_FOUND) push(elem);
      }

   U_RETURN(_length - n);
}

__pure bool UVector<UString>::operator==(const UVector<UString>& v) const
{
   U_TRACE(0, "UVector<UString>::operator==(%p)", &v)

   U_CHECK_MEMORY

   if (_length == v._length)
      {
      for (uint32_t i = 0; i < _length; ++i)
         {
         UStringRep* r1 =   UVector<UStringRep*>::at(i);
         UStringRep* r2 = v.UVector<UStringRep*>::at(i);

         if (r1->equal(r2) == false) U_RETURN(false);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

// ------------------------------------------------------
// THREE-WAY RADIX QUICKSORT
// ------------------------------------------------------
// Multikey Quicksort - Dr. Dobb's Journal, November 1998
//
// by Jon Bentley and Robert Sedgewick
// ------------------------------------------------------

static inline int chfunc(UStringRep* a[], int i, int depth)
{
   U_TRACE(0, "chfunc(%p,%d,%d)", a, i, depth)

   UStringRep* t = a[i];

   U_INTERNAL_DUMP("t = %V", t)

   int result = (t->data())[depth];

   U_RETURN(result);
}

static inline void vecswap2(UStringRep** a, UStringRep** b, int n)
{
   U_TRACE(0, "vecswap2(%p,%p,%d)", a, b, n)

   UStringRep* t;

   while (n-- > 0)
      {
      t    = *a;
      *a++ = *b;
      *b++ = t;
      }
}

#define ch(a) chfunc(a, 0, depth)

#define med3(a, b, c) med3func(a, b, c, depth)

#define swap2(a,b) { t = *(a); *(a) = *(b); *(b) = t; }

static inline UStringRep** med3func(UStringRep** a, UStringRep** b, UStringRep** c, int depth)
{
   U_TRACE(0, "med3func(%p,%p,%p,%d)", a, b, c, depth)

   int va, vb, vc;
   UStringRep** result;

   if ((va = ch(a)) == (vb = ch(b)))   U_RETURN_POINTER(a, UStringRep*);
   if ((vc = ch(c)) == va || vc == vb) U_RETURN_POINTER(c, UStringRep*);

   result = (va < vb ? (vb < vc ? b : (va < vc ? c : a))
                     : (vb > vc ? b : (va < vc ? a : c)));

   U_RETURN_POINTER(result, UStringRep*);
}

static inline void inssort(UStringRep** a, int n, int depth)
{
   U_TRACE(0, "inssort(%p,%d,%d)", a, n, depth)

   UStringRep** pi, **pj, *t;

   for (pi = a + 1; --n > 0; ++pi)
      {
      for (pj = pi; pj > a; --pj)
         {
         // Inline strcmp: break if *(pj-1) <= *pj

         if ((*(pj-1))->compare(*pj, depth) <= 0) break;

         swap2(pj, pj-1);
         }
      }
}

void UVector<UString>::mksort(UStringRep** a, int n, int depth)
{
   U_TRACE(0+256, "UVector<UString>::mksort(%p,%d,%d)", a, n, depth)

   int r, partval;
   UStringRep** pa, **pb, **pc, **pd, **pl, **pm, **pn, *t;

   if (n <= 10) // insertion sort
      {
      inssort(a, n, depth);

      return;
      }

   pl = a;
   pm = a + (n / 2);
   pn = a + (n - 1);

   if (n > 50)
      {
      // On big arrays, pseudomedian of 9

      int d = (n / 8);

      pl = med3(pl,         pl + d, pl + 2 * d);
      pm = med3(pm - d,     pm,     pm + d);
      pn = med3(pn - 2 * d, pn - d, pn);
      }

   pm = med3(pl, pm, pn);

   swap2(a, pm);

   partval = ch(a);

   pa = pb = a + 1;
   pc = pd = a + n - 1;

   for (;;)
      {
      while (pb <= pc &&
             (r = ch(pb) - partval) <= 0)
         {
         if (r == 0)
            {
            swap2(pa, pb);

            ++pa;
            }

         ++pb;
         }

      while (pb <= pc &&
             (r = ch(pc) - partval) >= 0)
         {
         if (r == 0)
            {
            swap2(pc, pd);

            --pd;
            }

         --pc;
         }

      if (pb > pc) break;

      swap2(pb, pc);

      ++pb;
      --pc;
      }

   pn = a + n;
   r  = U_min(pa - a,  pb - pa);     vecswap2(a,  pb - r, r);
   r  = U_min(pd - pc, pn - pd - 1); vecswap2(pb, pn - r, r);

   if ((r = pb - pa)  > 1) mksort(        a,                    r, depth);
   if (ch(a + r)     != 0) mksort(a     + r, pa - a + pn - pd - 1, depth + 1);
   if ((r = pd - pc)  > 1) mksort(a + n - r,                    r, depth);
}

uint32_t UVector<UString>::loadFromData(const char* ptr, uint32_t sz)
{
   U_TRACE(0+256, "UVector<UString>::loadFromData(%.*S,%u)", sz, ptr, sz)

   U_INTERNAL_ASSERT_MAJOR(sz, 0)
   U_INTERNAL_ASSERT_MAJOR(_capacity, 0)

   const char* _end   = ptr + sz;
   const char* _start = ptr;

   char terminator = 0, c = *ptr;

   if (c == '(' ||
       c == '[')
      {
      ++ptr; // skip '(' or '['

      terminator = (c == '(' ? ')' : ']');
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

      push(str);
      }

   U_INTERNAL_DUMP("ptr - _start = %lu", ptr - _start)

   U_INTERNAL_ASSERT((ptr - _start) <= sz)

   sz = ptr - _start;

   U_RETURN(sz);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UVector<UString>& v)
{
   U_TRACE(0+256, "UVector<UString>::operator>>(%p,%p)", &is, &v)

   U_INTERNAL_ASSERT_MAJOR(v._capacity,0)
   U_INTERNAL_ASSERT(is.peek() == '[' || is.peek() == '(')

   if (is.good())
      {
      streambuf* sb = is.rdbuf();

      int c = sb->sbumpc(); // skip '[' or '('

      if (c == EOF) is.setstate(ios::eofbit);
      else
         {
         int terminator = (c == '[' ? ']' : ')');

         while (true)
            {
            do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

         // U_INTERNAL_DUMP("c = %C", c)

            if (c == terminator ||
                c == EOF)
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

            UString str(U_CAPACITY);

            str.get(is);

            v.push(str);
            }

         if (c == EOF) is.setstate(ios::eofbit);
         }
      }

// -------------------------------------------------
// NB: we can load an empty vector (ex. mod_http)...
// -------------------------------------------------
// if (v._length == 0) is.setstate(ios::failbit);
// -------------------------------------------------

   return is;
}

// DEBUG

#  ifdef DEBUG
const char* UVector<void*>::dump(bool reset) const
{
   *UObjectIO::os << "vec       " << (void*)vec << '\n'
                  << "_length   " << _length    << '\n'
                  << "_capacity " << _capacity;

                  /*
                  << "tail      " << tail       << '\n'
                  << "head      " << head       << '\n'
                  */

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
