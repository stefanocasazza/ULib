// portable.h

#ifndef PORTABLE_H
#define PORTABLE_H 1

/**
 * #define NO_ULIB
 * #define U_STD_STRING
 */

#ifdef NO_ULIB
#  ifdef    DEBUG
#     undef DEBUG
#  endif

#  define U_ULIB_INIT(expr)

#  define U_INTERNAL_ASSERT(expr)
#  define U_INTERNAL_ASSERT_MINOR(a,b)
#  define U_INTERNAL_ASSERT_MAJOR(a,b)
#  define U_INTERNAL_ASSERT_EQUALS(a,b)
#  define U_INTERNAL_ASSERT_DIFFERS(a,b)
#  define U_INTERNAL_ASSERT_POINTER(ptr)
#  define U_INTERNAL_ASSERT_RANGE(a,x,b)

#  define U_INTERNAL_ASSERT_MSG(expr,info)
#  define U_INTERNAL_ASSERT_MINOR_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_MAJOR_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_EQUALS_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_DIFFERS_MSG(a,b,info)
#  define U_INTERNAL_ASSERT_POINTER_MSG(ptr,info)
#  define U_INTERNAL_ASSERT_RANGE_MSG(a,x,b,info)

#  define U_ASSERT(expr)
#  define U_ASSERT_MINOR(a,b)
#  define U_ASSERT_MAJOR(a,b)
#  define U_ASSERT_EQUALS(a,b)
#  define U_ASSERT_DIFFERS(a,b)
#  define U_ASSERT_POINTER(ptr)
#  define U_ASSERT_RANGE(a,x,b)

#  define U_ASSERT_MSG(expr,info)
#  define U_ASSERT_MINOR_MSG(a,b,info)
#  define U_ASSERT_MAJOR_MSG(a,b,info)
#  define U_ASSERT_EQUALS_MSG(a,b,info)
#  define U_ASSERT_DIFFERS_MSG(a,b,info)
#  define U_ASSERT_POINTER_MSG(ptr,info)
#  define U_ASSERT_RANGE_MSG(a,x,b,info)

#  define U_MEMORY_TEST
#  define U_MEMORY_ALLOCATOR
#  define U_MEMORY_DEALLOCATOR
#  define U_MEMORY_TEST_COPY(o)
#  define U_CHECK_MEMORY
#  define U_CHECK_MEMORY

#  define U_TRACE(level,args...)
#  define U_TRACE_NO_PARAM(level,name)
#  define U_DUMP(args...)
#  define U_INTERNAL_DUMP(args...)
#  define U_SYSCALL_NO_PARAM(name)              ::name()
#  define U_SYSCALL_VOID_NO_PARAM(name)         ::name()
#  define U_SYSCALL(name,format,args...)        ::name(args)
#  define U_SYSCALL_VOID(name,format,args...)   ::name(args)

#  define U_RETURN(r)           return (r)
#  define U_RETURN_STRING(r)    return (r)
#  define U_RETURN_OBJECT(obj)  return (obj)
#  define U_RETURN_POINTER(r,t) return (r)

#  define U_UNREGISTER_OBJECT(level,pointer)
#  define U_TRACE_UNREGISTER_OBJECT(level,CLASS)
#  define U_TRACE_REGISTER_OBJECT(level,CLASS,format,args...)

#  define U_EXIT(exit_value) ::exit(exit_value)

#  define U_CONSTANT_SIZE(str)         (sizeof(str)-1)
#  define U_CONSTANT_TO_PARAM(str)     (const char*)str,(size_t)U_CONSTANT_SIZE(str)
#  define U_STRING_FROM_CONSTANT(str)  string(str,U_CONSTANT_SIZE(str))

using namespace std;
#endif

#ifdef U_STD_STRING
#  include <string>
#  include <vector>
#  include <iostream.h>

#  define UVector vector
#  define UString string

#  define U_STR_RESERVE(str,len)           (str).resize(len)
#  define U_STR_SIZE_ADJUST(str,len)       (str).resize(len)
#  define U_STR_SIZE_ADJUST_FORCE(str,len) (str).resize(len)
#  define U_STR_ASSIGN_MMAP(str,map,size)  (str)->reserve(size); (str)->assign(map,size)

unsigned split(UVector<UString>& vec, const UString& buffer, const char* delim);

#  define U_VEC_SPLIT(vec,str,delim)      split(vec,str,delim)
#  define U_VEC_ERASE1(vec,i)             vec.erase(vec.begin()+i)
#  define U_VEC_INSERT(vec,i,str)         vec.insert(vec.begin()+i,str)
#  define U_VEC_ERASE2(vec,first,last)    vec.erase(vec.begin()+first, vec.begin()+last)
#else
#  include <ulib/string.h>
#  include <ulib/container/vector.h>

#  define U_STR_RESERVE(str,len)             (str).reserve(len)
#  define U_STR_SIZE_ADJUST(str,len)         (str).size_adjust(len)
#  define U_STR_SIZE_ADJUST_FORCE(str,len)   (str).size_adjust_force(len)
#  define U_STR_ASSIGN_MMAP(str,map,size)    (str)->mmap(map,size)

#  define U_VEC_INSERT(vec,i,str)            vec.insert(i,str)
#  define U_VEC_SPLIT(vec,str,delim)         vec.split(str,delim)
#  define U_VEC_ERASE1(vec,i)                vec.erase(i)
#  define U_VEC_ERASE2(vec,first,last)       vec.erase(first, last)
#endif

#endif
