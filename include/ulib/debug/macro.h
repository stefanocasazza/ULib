// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    macro.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_MACRO_H
#define ULIB_MACRO_H 1

// Design by contract - if (expr == false) then stop

#ifdef DEBUG                             // NB: we need to save the status on the stack because of thread (ex. time resolution optimizatio) 
#  define U_ASSERT(expr)                 { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT(expr);                 u_trace_suspend = _s; }
#  define U_ASSERT_MINOR(a,b)            { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_MINOR(a,b);            u_trace_suspend = _s; }
#  define U_ASSERT_MAJOR(a,b)            { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_MAJOR(a,b);            u_trace_suspend = _s; }
#  define U_ASSERT_EQUALS(a,b)           { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_EQUALS(a,b);           u_trace_suspend = _s; }
#  define U_ASSERT_DIFFERS(a,b)          { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_DIFFERS(a,b);          u_trace_suspend = _s; }
#  define U_ASSERT_POINTER(ptr)          { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_POINTER(ptr);          u_trace_suspend = _s; }
#  define U_ASSERT_RANGE(a,x,b)          { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_RANGE(a,x,b);          u_trace_suspend = _s; }

#  define U_ASSERT_MSG(expr,info)        { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_MSG(expr,info);        u_trace_suspend = _s; }
#  define U_ASSERT_MINOR_MSG(a,b,info)   { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_MINOR_MSG(a,b,info);   u_trace_suspend = _s; }
#  define U_ASSERT_MAJOR_MSG(a,b,info)   { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_MAJOR_MSG(a,b,info);   u_trace_suspend = _s; }
#  define U_ASSERT_EQUALS_MSG(a,b,info)  { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_EQUALS_MSG(a,b,info);  u_trace_suspend = _s; }
#  define U_ASSERT_DIFFERS_MSG(a,b,info) { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_DIFFERS_MSG(a,b,info); u_trace_suspend = _s; }
#  define U_ASSERT_POINTER_MSG(ptr,info) { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_POINTER_MSG(ptr,info); u_trace_suspend = _s; }
#  define U_ASSERT_RANGE_MSG(a,x,b,info) { int _s = u_trace_suspend; u_trace_suspend = 1; U_INTERNAL_ASSERT_RANGE_MSG(a,x,b,info); u_trace_suspend = _s; }
#elif defined(U_TEST)
#  define U_ASSERT(expr)                  U_INTERNAL_ASSERT(expr)
#  define U_ASSERT_MINOR(a,b)             U_INTERNAL_ASSERT_MINOR(a,b)
#  define U_ASSERT_MAJOR(a,b)             U_INTERNAL_ASSERT_MAJOR(a,b)
#  define U_ASSERT_EQUALS(a,b)            U_INTERNAL_ASSERT_EQUALS(a,b)
#  define U_ASSERT_DIFFERS(a,b)           U_INTERNAL_ASSERT_DIFFERS(a,b)
#  define U_ASSERT_POINTER(ptr)           U_INTERNAL_ASSERT_POINTER(ptr)
#  define U_ASSERT_RANGE(a,x,b)           U_INTERNAL_ASSERT_RANGE(a,x,b)

#  define U_ASSERT_MSG(expr,info)         U_INTERNAL_ASSERT_MSG(expr,info)
#  define U_ASSERT_MINOR_MSG(a,b,info)    U_INTERNAL_ASSERT_MINOR_MSG(a,b,info)
#  define U_ASSERT_MAJOR_MSG(a,b,info)    U_INTERNAL_ASSERT_MAJOR_MSG(a,b,info)
#  define U_ASSERT_EQUALS_MSG(a,b,info)   U_INTERNAL_ASSERT_EQUALS_MSG(a,b,info)
#  define U_ASSERT_DIFFERS_MSG(a,b,info)  U_INTERNAL_ASSERT_DIFFERS_MSG(a,b,info)
#  define U_ASSERT_POINTER_MSG(ptr,info)  U_INTERNAL_ASSERT_POINTER_MSG(ptr,info)
#  define U_ASSERT_RANGE_MSG(a,x,b,info)  U_INTERNAL_ASSERT_RANGE_MSG(a,x,b,info)
#else
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
#endif

#ifdef DEBUG
// Manage class test for memory corruption
#  define U_MEMORY_TEST         UMemoryError memory;
#  define U_MEMORY_TEST_COPY(o)   memory = o.memory;

#  define U_CHECK_MEMORY_OBJECT(p) U_ASSERT_MACRO((p)->memory.invariant(), "ERROR ON MEMORY", (p)->memory.getErrorType(p))
#  define U_CHECK_MEMORY           U_CHECK_MEMORY_OBJECT(this)

// Manage info on execution of program
#  define U_TRACE(level,fmt,args...)   UTrace utr(level, U_CONSTANT_TO_PARAM(fmt) , ##args);
#  define U_TRACE_NO_PARAM(level,name) UTrace utr(level, U_CONSTANT_TO_TRACE(name));

#  define U_TRACE_CTOR(level,CLASS,fmt,args...) UTrace utr(level, U_CONSTANT_TO_PARAM(#CLASS"::"#CLASS"(" fmt ")") , ##args);
#  define U_TRACE_DTOR(level,CLASS)             UTrace utr(level, U_CONSTANT_TO_TRACE(#CLASS"::~"#CLASS"()"));

// NB: U_DUMP, U_SYSCALL() and U_RETURN() depend on presence of U_TRACE()

#  define U_INTERNAL_DUMP(fmt,args...) { if (utr.active[0])                  u_trace_dump(fmt,U_CONSTANT_SIZE(fmt) , ##args); }
#  define          U_DUMP(fmt,args...) { if (utr.active[0]) { utr.suspend(); u_trace_dump(fmt,U_CONSTANT_SIZE(fmt) , ##args); utr.resume(); } }

#  define U_SYSCALL_NO_PARAM(name) (utr.trace_syscall(U_CONSTANT_TO_PARAM("::"#name"()"),0), \
                                    utr.trace_sysreturn_type(::name()))

#  define U_SYSCALL_VOID_NO_PARAM(name) { utr.trace_syscall(U_CONSTANT_TO_PARAM("::"#name"()"),0); \
                                          name(); utr.trace_sysreturn(false,U_NULLPTR,0); }

#  define U_SYSCALL(name,format,args...) (utr.suspend(), utr.trace_syscall(U_CONSTANT_TO_PARAM("::"#name"(" format ")") , ##args), \
                                          utr.resume(),  utr.trace_sysreturn_type(::name(args)))

#  define U_SYSCALL_VOID(name,format,args...) { utr.suspend();               utr.trace_syscall(U_CONSTANT_TO_PARAM("::"#name"(" format ")") , ##args); \
                                                utr.resume();  ::name(args); utr.trace_sysreturn(false,U_NULLPTR,0); }

#  define U_RETURN(r)                                                 return (utr.trace_return_type((r)))
#  define U_RETURN_STRING(str) {U_INTERNAL_ASSERT((str).invariant()); return (utr.trace_return(U_CONSTANT_TO_PARAM("%V"),(str).rep),(str));}
#  define U_RETURN_OBJECT(obj)                                        return (utr.trace_return(U_CONSTANT_TO_PARAM("%O"),U_OBJECT_TO_TRACE((obj))),(obj))
#  define U_RETURN_POINTER(ptr,type)                                  return ((type*)utr.trace_return_type((void*)(ptr)))

#  define U_MEMCPY(a,b,n) U_SYSCALL_VOID(u__memcpy,"%p,%p,%u,%S",(void*)(a),(const void*)(b),(n),__PRETTY_FUNCTION__)

// Dump argument for exec()

#  define U_DUMP_EXEC(argv, envp) { uint32_t _i; \
for (_i = 0; argv[_i]; ++_i) \
   { \
   U_INTERNAL_DUMP("argv[%2u] = %p %S", _i, argv[_i], argv[_i]) \
   } \
U_INTERNAL_DUMP("argv[%2u] = %p %S", _i, argv[_i], argv[_i]) \
if (envp) \
   { \
   for (_i = 0; envp[_i]; ++_i) \
      { \
      U_INTERNAL_DUMP("envp[%2u] = %p %S", _i, envp[_i], envp[_i]) \
      } \
   U_INTERNAL_DUMP("envp[%2u] = %p %S", _i, envp[_i], envp[_i]) \
   } }

// Dump attributes

#  define U_DUMP_ATTRS(attrs)   { uint32_t _i; for (_i = 0; (attrs)[_i]; ++_i) { U_INTERNAL_DUMP("attrs[%2u] = %S", _i, (attrs)[_i]) } }
#  define U_DUMP_IOVEC(iov,cnt) {      int _i; for (_i = 0;    _i < cnt; ++_i) { U_INTERNAL_DUMP("iov[%2u](%u) = %.*S", _i, (iov)[_i].iov_len, (iov)[_i].iov_len, (iov)[_i].iov_base) } }

#ifdef _MSWINDOWS_
#  define U_FORK()  -1
#  define U_VFORK() -1
#else
#  define U_FORK()  u_debug_fork(  ::fork(), utr.active[0])
#  define U_VFORK() u_debug_vfork(::vfork(), utr.active[0])
#endif

#  define U_EXIT(exit_value) { if (utr.active[0]) u_debug_exit(exit_value); ::exit(exit_value); }
#  define U_EXEC(pathname, argv, envp) u_debug_exec(pathname, argv, envp, utr.active[0])

// Dump fd_set

#ifndef __FDS_BITS
#  ifdef _MSWINDOWS_
#     define __FDS_BITS(fd_set) ((fd_set)->fd_array)
#  else
#     define __FDS_BITS(fd_set) ((fd_set)->fds_bits)
#  endif
#endif

// A mechanism that allow all objects to be registered with a central in-memory "database" that can dump the state of all live objects

# ifdef U_STDCPP_ENABLE
#  define U_REGISTER_OBJECT_PTR(level,CLASS,p,pmemory) \
           { if (UObjectDB::fd > 0 && \
                (level) >= UObjectDB::level_active) { \
                  UObjectDB::registerObject(new UObjectDumpable_Adapter<CLASS>(level,#CLASS,(const CLASS*)p,(void**)pmemory)); } }  

#  define U_UNREGISTER_OBJECT_PTR(level,ptr) \
            if (UObjectDB::fd > 0 && \
                (level) >= UObjectDB::level_active) { \
                UObjectDB::unregisterObject((void*)ptr); }

#  define U_REGISTER_OBJECT(CLASS) U_REGISTER_OBJECT_PTR(0,CLASS,this,&(this->memory._this))
#  define U_UNREGISTER_OBJECT(level) U_UNREGISTER_OBJECT_PTR(level,this)

/*
#  define U_TRACE_REGISTER_OBJECT(level,CLASS,format,args...) U_REGISTER_OBJECT_PTR(level,CLASS,this,&(this->memory._this)) \
                                                              UTrace utr(level, U_CONSTANT_TO_PARAM(#CLASS"::"#CLASS"(" format ")") , ##args);

#  define U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(level,CLASS,format,args...) U_REGISTER_OBJECT_PTR(level,CLASS,this,U_NULLPTR) \
                                                                                   UTrace utr(level, U_CONSTANT_TO_PARAM(#CLASS"::"#CLASS"(" format ")") , ##args);

#  define U_TRACE_UNREGISTER_OBJECT(level,CLASS) U_UNREGISTER_OBJECT_PTR(level,this); UTrace utr(level,U_CONSTANT_TO_TRACE(#CLASS"::~"#CLASS"()"));
*/

#  define U_DELETE(obj) { U_UNREGISTER_OBJECT_PTR(0,obj) delete obj; }

// Manage location info for object allocation

# ifdef ENABLE_MEMPOOL
#  define U_NEW(CLASS,obj,args...) { UMemoryPool::obj_class = #CLASS; \
                                     UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                     U_SET_LOCATION_INFO; (obj) = new args; \
                                     UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; \
                                     U_REGISTER_OBJECT_PTR(0,CLASS,(obj),&((obj)->memory._this)); }

#  define U_NEW_WITHOUT_CHECK_MEMORY(CLASS,obj,args...) { UMemoryPool::obj_class = #CLASS; \
                                                          UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                                          U_SET_LOCATION_INFO; (obj) = new args; \
                                                          UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; \
                                                          U_REGISTER_OBJECT_PTR(0,CLASS,(obj),U_NULLPTR); }

#  define U_NEW_STRING(obj,args...) { UMemoryPool::obj_class = "UString"; \
                                      UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                      U_SET_LOCATION_INFO; (obj) = new args; \
                                      UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; \
                                      U_REGISTER_OBJECT_PTR(0,UString,(obj),U_NULLPTR); }

#  define U_NEW_ULIB_STRING(obj,args...) { UMemoryPool::obj_class = "UString"; \
                                           UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                           U_SET_LOCATION_INFO; (obj) = new args; \
                                           UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; \
                                           U_REGISTER_OBJECT_PTR(-1,UString,(obj),U_NULLPTR); }

#  define U_NEW_ULIB_OBJECT(CLASS,obj,args...) { UMemoryPool::obj_class = #CLASS; \
                                                 UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                                 U_SET_LOCATION_INFO; (obj) = new args; \
                                                 UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; \
                                                 U_REGISTER_OBJECT_PTR(-1,CLASS,(obj),&((obj)->memory._this)); }
# else
#  define U_NEW(CLASS,obj,args...) { U_SET_LOCATION_INFO; (obj) = new args; \
                                     U_REGISTER_OBJECT_PTR(0,CLASS,(obj),U_NULLPTR); }

#  define U_NEW_WITHOUT_CHECK_MEMORY(CLASS,obj,args...) U_NEW(CLASS,obj,args)

#  define U_NEW_STRING(obj,args...) { U_SET_LOCATION_INFO; (obj) = new args; \
                                      U_REGISTER_OBJECT_PTR(0,UString,(obj),U_NULLPTR); }

#  define U_NEW_ULIB_STRING(obj,args...) { U_SET_LOCATION_INFO; (obj) = new args; \
                                           U_REGISTER_OBJECT_PTR(-1,UString,(obj),U_NULLPTR); }

#  define U_NEW_ULIB_OBJECT(CLASS,obj,args...) { U_SET_LOCATION_INFO; (obj) = new args; \
                                                 U_REGISTER_OBJECT_PTR(-1,CLASS,(obj),U_NULLPTR); }
# endif

#  define U_DUMP_OBJECT(obj)    { if (utr.active[0])                                                    u_trace_dump(U_CONSTANT_TO_PARAM(#obj" = %S"), U_OBJECT_TO_TRACE((obj))); }
#  define U_DUMP_CONTAINER(obj) { if (utr.active[0] && (obj).getSpaceToDump() < U_MAX_SIZE_PREALLOCATE) u_trace_dump(U_CONSTANT_TO_PARAM(#obj" = %O"), U_OBJECT_TO_TRACE((obj))); }

#  define U_DUMP_OBJECT_TO_TMP(obj,fname) \
            { char _buffer[2 * 1024 * 1024]; \
               uint32_t _n = UObject2String((obj), _buffer, sizeof(_buffer)); \
               U_INTERNAL_ASSERT_MINOR(_n, sizeof(_buffer)) \
               (void) UFile::writeToTmp(_buffer, _n, O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM(#fname".%P"), 0); }

#  define U_DUMP_OBJECT_WITH_CHECK(msg,check_object) \
            if (UObjectDB::fd > 0) { \
               char _buffer[4096]; \
               uint32_t _n = UObjectDB::dumpObject(_buffer, sizeof(_buffer), (check_object)); \
               if (utr.active[0]) u_trace_dump(U_CONSTANT_TO_PARAM(msg " = \n%.*s\n"), U_min(_n,4000), _buffer); }

# else /* U_STDCPP_ENABLE */
#  define U_REGISTER_OBJECT(CLASS)
#  define U_REGISTER_OBJECT_PTR(level,CLASS,p,pmemory)
#  define U_UNREGISTER_OBJECT(CLASS)
#  define U_UNREGISTER_OBJECT_PTR(level,ptr)

#  define U_DELETE(obj) delete obj;

# ifdef ENABLE_MEMPOOL
#  define U_NEW(CLASS,obj,args...) { UMemoryPool::obj_class = #CLASS; \
                                     UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                     U_SET_LOCATION_INFO; (obj) = new args; \
                                     UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; }

#  define U_NEW_STRING(obj,args...) { UMemoryPool::obj_class = "UString"; \
                                      UMemoryPool::func_call = __PRETTY_FUNCTION__; \
                                      U_SET_LOCATION_INFO; (obj) = new args; \
                                      UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR; }
# else
#  define U_NEW(CLASS,obj,args...)  { U_SET_LOCATION_INFO; (obj) = new args; }
#  define U_NEW_STRING(obj,args...) { U_SET_LOCATION_INFO; (obj) = new args; }
# endif

#  define U_NEW_ULIB_STRING(obj,args...)                { U_SET_LOCATION_INFO; (obj) = new args; }
#  define U_NEW_ULIB_OBJECT(CLASS,obj,args...)          { U_SET_LOCATION_INFO; (obj) = new args; }
#  define U_NEW_WITHOUT_CHECK_MEMORY(CLASS,obj,args...) { U_SET_LOCATION_INFO; (obj) = new args; }

#  define U_DUMP_OBJECT(obj)
#  define U_DUMP_CONTAINER(obj)
#  define U_DUMP_OBJECT_TO_TMP(obj,fname)
#  define U_DUMP_OBJECT_WITH_CHECK(msg,check_object)

# endif /* U_STDCPP_ENABLE */

#else /* DEBUG */

#  define U_MEMORY_TEST
#  define U_CHECK_MEMORY
#  define U_MEMORY_TEST_COPY(o)
#  define U_CHECK_MEMORY_OBJECT(p)

#  define U_DUMP(args...)
#  define U_TRACE(level,args...)
#  define U_TRACE_NO_PARAM(level,name)
#  define U_TRACE_CTOR(level,CLASS,fmt,args...)
#  define U_TRACE_DTOR(level,CLASS)
#  define U_INTERNAL_DUMP(args...)
#  define U_SYSCALL_NO_PARAM(name)            ::name()
#  define U_SYSCALL_VOID_NO_PARAM(name)       ::name()
#  define U_SYSCALL(name,format,args...)      ::name(args)
#  define U_SYSCALL_VOID(name,format,args...) ::name(args)

# ifdef U_APEX_ENABLE
#  define U_MEMCPY(a,b,n) (void) apex_memcpy((void*)(a),(const void*)(b),(n))
# else
#  define U_MEMCPY(a,b,n)      (void) memcpy((void*)(a),(const void*)(b),(n))
# endif

#  define U_RETURN(r)                return (r)
#  define U_RETURN_STRING(r)         return (r)
#  define U_RETURN_OBJECT(obj)       return (obj)
#  define U_RETURN_POINTER(ptr,type) return ((type*)ptr)

#  define U_OBJECT_TO_TRACE(object)

#  define U_DUMP_ATTRS(attrs)
#  define U_DUMP_IOVEC(iov,cnt)
#  define U_DUMP_EXEC(argv, envp)

#ifdef _MSWINDOWS_
#  define U_FORK()  -1
#  define U_VFORK() -1
#else
#  define U_FORK()  ::fork()
#  define U_VFORK() ::vfork()
#endif

#  define U_EXIT(exit_value) ::exit(exit_value)
#  define U_EXEC(pathname, argv, envp) u_exec_failed = false; ::execve(pathname, argv, envp); u_exec_failed = true; \
                                       U_WARNING("::execve(%s,%p,%p) = -1%R", pathname, argv, envp, NULL); \
                                       ::_exit(EX_UNAVAILABLE)

#  define U_REGISTER_OBJECT(CLASS)
#  define U_UNREGISTER_OBJECT(level)
#  define U_REGISTER_OBJECT_PTR(level,CLASS,p,pmemory)
#  define U_UNREGISTER_OBJECT_PTR(level,pointer)

#  define U_DELETE(obj) delete obj;

#  define U_NEW(CLASS,obj,args...)                      (obj) = new args;
#  define U_NEW_STRING(obj,args...)                     (obj) = new args;
#  define U_NEW_ULIB_STRING(obj,args...)                (obj) = new args;
#  define U_NEW_ULIB_OBJECT(CLASS,obj,args...)          (obj) = new args;
#  define U_NEW_WITHOUT_CHECK_MEMORY(CLASS,obj,args...) (obj) = new args;

#  define U_DUMP_OBJECT(obj)
#  define U_DUMP_CONTAINER(obj)
#  define U_DUMP_OBJECT_TO_TMP(obj,fname)
#  define U_DUMP_OBJECT_WITH_CHECK(msg,check_object)

#endif /* DEBUG */

#endif
