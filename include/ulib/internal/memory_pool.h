// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    memory_pool.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_MEMORY_POOL_H
#define ULIB_MEMORY_POOL_H 1

#include <ulib/internal/common.h>

#ifdef USE_LIBMIMALLOC
#  include <mimalloc.h> // mimalloc-new-delete.h
#  define malloc(x) mi_malloc(x)
#  define   free(x) mi_free(x)
#endif

// ---------------------------------------------------------------------------------------------------------------
// U_STACK_TYPE_[0-9] 'type' stack for which the request is serviced with preallocation

#define U_MAX_SIZE_PREALLOCATE 4096U // (U_STACK_TYPE_9) max request size serviced with preallocation otherwise malloc()

#ifdef HAVE_ARCH64
/**
 * =========================
 * NO DEBUG (64 bit)
 * -------------------------
 *    1 sizeof(UMagic)
 *    1 sizeof(UNotifier)
 *    1 sizeof(UPlugIn<void*>)
 *    8 sizeof(UCrl)
 *    8 sizeof(ULock)
 *    8 sizeof(UPKCS10)
 *    8 sizeof(UString) <==
 *    8 sizeof(UCertificate)
 * -------------------------
 * U_STACK_TYPE_0
 * 
 *   12 sizeof(UProcess)
 *   16 sizeof(UTimer)
 *   16 sizeof(UPKCS7)
 *   16 sizeof(UTimeVal)
 *   16 sizeof(UTimeDate)
 *   16 sizeof(UXMLParser)
 *   16 sizeof(USemaphore)
 *   16 sizeof(URDBServer)
 *   16 sizeof(UVector<UString>)
 *   16 sizeof(UServer<UTCPSocket>)
 *   24 sizeof(UStringRep) <==
 *   24 sizeof(USOAPObject)
 * -------------------------
 * U_STACK_TYPE_1
 * 
 *   32 sizeof(ULDAPEntry)
 *   32 sizeof(UQueryNode)
 *   32 sizeof(USOAPFault)
 *   32 sizeof(UTokenizer)
 *   32 sizeof(UXMLAttribute)
 *   32 sizeof(UTree<UString>)
 * -------------------------
 * U_STACK_TYPE_2
 * 
 *   40 sizeof(Url)
 *   40 sizeof(ULDAP)
 *   40 sizeof(UCache)
 *   40 sizeof(UHashMap<UString>)
 *   48 sizeof(UCURL)
 *   48 sizeof(UDialog)
 *   48 sizeof(UMimeHeader)
 *   48 sizeof(UMimeEntity)
 *   48 sizeof(UXMLElement)
 *   48 sizeof(UQueryParser)
 *   48 sizeof(USOAPEncoder)
 *   48 sizeof(USOAPGenericMethod)
 *   56 sizeof(UOptions)
 *   56 sizeof(UIPAddress)
 *   56 sizeof(UHTTP::UFileCacheData) <==
 * -------------------------
 * U_STACK_TYPE_3
 * 
 *   64 sizeof(UPCRE)
 *   64 sizeof(UCommand)
 *   64 sizeof(UApplication)
 *   72 sizeof(UMimePKCS7)
 *   80 sizeof(UZIP)
 *   88 sizeof(UMimeMultipartMsg)
 *   96 sizeof(UMimeMessage)
 *  104 sizeof(UClientImage<UTCPSocket>)
 *  128 sizeof(USOAPParser)
 *  128 sizeof(UMimeMultipart)
 * -------------------------
 * U_STACK_TYPE_4
 * 
 *  144 sizeof(USocket)
 *  144 sizeof(UTCPSocket)
 *  144 sizeof(UUDPSocket)
 *  176 sizeof(USSLSocket)
 *  184 sizeof(UFile)
 *  216 sizeof(UBison)
 *  216 sizeof(UFlexer)
 *  232 sizeof(ULog)
 *  232 sizeof(USmtpClient)
 *  256 sizeof(UFileConfig)
 *  256 sizeof(URDBClient<UTCPSocket>)
 * -------------------------
 * U_STACK_TYPE_5
 * 
 *  264 sizeof(USOAPClient<UTCPSocket>)
 *  312 sizeof(UHttpClient<UTCPSocket>)
 *  336 sizeof(UCDB)
 *  360 sizeof(UFtpClient)
 *  512
 * -------------------------
 * U_STACK_TYPE_6
 * 
 *  568 sizeof(URDB)
 * =========================
 * DEBUG (64 bit)
 * -------------------------
 *    1 sizeof(UMagic)
 *    1 sizeof(UNotifier)
 *    1 sizeof(UPlugIn<void*>)
 *    8 sizeof(UMagic)
 *    8 sizeof(UString) <==
 * -------------------------
 * U_STACK_TYPE_0
 * 
 *   16 sizeof(UCrl)
 *   16 sizeof(ULock)
 *   16 sizeof(UPKCS10)
 *   16 sizeof(UCertificate)
 *   24 sizeof(UTimer)
 *   24 sizeof(UPKCS7)
 *   24 sizeof(UProcess)
 *   24 sizeof(UTimeVal)
 *   24 sizeof(UTimeDate)
 *   24 sizeof(URDBServer)
 *   24 sizeof(USemaphore)
 *   24 sizeof(UVector<UString>)
 *   24 sizeof(UServer<UTCPSocket>)
 *   40 sizeof(UQueryNode)
 *   40 sizeof(USOAPFault)
 *   40 sizeof(UStringRep) <==
 *   40 sizeof(UTokenizer)
 *   40 sizeof(USOAPObject)
 *   40 sizeof(UTree<UString>)
 * -------------------------
 * U_STACK_TYPE_1
 * 
 *   48 sizeof(Url)
 *   48 sizeof(UHashMap<UString>) <==
 * -------------------------
 * U_STACK_TYPE_2
 * 
 *   56 sizeof(UDialog)
 *   56 sizeof(UMimeEntity)
 *   56 sizeof(USOAPGenericMethod)
 *   64 sizeof(UCache)
 *   64 sizeof(UOptions)
 *   64 sizeof(UMimeHeader)
 *   64 sizeof(UQueryParser)
 *   64 sizeof(USOAPEncoder)
 *   64 sizeof(UHTTP::UFileCacheData) <==
 * -------------------------
 * U_STACK_TYPE_3
 * 
 *   72 sizeof(UPCRE)
 *   72 sizeof(UCommand)
 *   80 sizeof(UApplication)
 *   88 sizeof(UZIP)
 *   88 sizeof(UMimePKCS7)
 *   96 sizeof(UIPAddress)
 *  104 sizeof(UMimeMultipartMsg)
 *  112 sizeof(UMimeMessage)
 *  112 sizeof(UClientImage<UTCPSocket>)
 *  128
 * -------------------------
 * U_STACK_TYPE_4
 * 
 *  144 sizeof(UMimeMultipart)
 *  168 sizeof(USOAPParser)
 *  192 sizeof(UFile)
 *  216 sizeof(UBison)
 *  216 sizeof(UFlexer)
 *  232 sizeof(USocket)
 *  232 sizeof(UTCPSocket)
 *  232 sizeof(UUDPSocket)
 *  256 sizeof(ULog)
 * -------------------------
 * U_STACK_TYPE_5
 * 
 *  264 sizeof(USSLSocket)
 *  272 sizeof(UFileConfig)
 *  272 sizeof(URDBClient<UTCPSocket>)
 *  280 sizeof(USOAPClient<UTCPSocket>)
 *  320 sizeof(USmtpClient)
 *  328 sizeof(UHttpClient<UTCPSocket>)
 *  344 sizeof(UCDB)
 *  512
 * -------------------------
 * U_STACK_TYPE_6
 *
 *  536 sizeof(UFtpClient)
 *  592 sizeof(URDB)
 * =========================
 */
#else
/**
 * =========================
 * NO DEBUG (32 bit) **
 * -------------------------
 *    1 sizeof(UMagic)
 *    1 sizeof(UNotifier)
 *    1 sizeof(UPlugIn<void*>)
 *    4 sizeof(UCrl)
 *    4 sizeof(UPKCS10)
 *    4 sizeof(UString) <==
 *    4 sizeof(UCertificate)
 * -------------------------
 * U_STACK_TYPE_0
 * 
 *    8 sizeof(ULock)
 *    8 sizeof(UTimer)
 *    8 sizeof(UPKCS7)
 *    8 sizeof(UTimeVal)
 *    8 sizeof(USemaphore)
 *   12 sizeof(UProcess)
 *   12 sizeof(URDBServer)
 *   12 sizeof(UVector<UString>)
 *   12 sizeof(UServer<UTCPSocket>)
 *   16 sizeof(UTimeDate)
 *   16 sizeof(UXMLParser)
 *   16 sizeof(UQueryNode)
 *   16 sizeof(USOAPFault)
 *   16 sizeof(UTokenizer)
 *   16 sizeof(UStringRep) <==
 *   16 sizeof(USOAPObject)
 * -------------------------
 * U_STACK_TYPE_1
 * 
 *   20 sizeof(UCache)
 *   20 sizeof(UTree<UString>)
 *   24 sizeof(UQueryParser)
 *   24 sizeof(USOAPGenericMethod)
 *   28 sizeof(UCURL)
 *   28 sizeof(UDialog)
 *   28 sizeof(UMimeEntity)
 *   28 sizeof(USOAPEncoder)
 *   28 sizeof(UHashMap<UString>)
 *   32 sizeof(Url)
 *   32 sizeof(UOptions)
 *   32 sizeof(UMimeHeader)
 *   36 sizeof(UApplication)
 *   40 sizeof(UPCRE)
 *   40 sizeof(UCommand)
 *   40 sizeof(UMimePKCS7)
 *   40 sizeof(UHTTP::UFileCacheData) <==
 * -------------------------
 * U_STACK_TYPE_2
 * 
 *   44 sizeof(UZIP)
 *   56 sizeof(UMimeMessage)
 *   64
 * -------------------------
 * U_STACK_TYPE_3
 * 
 *   68 sizeof(USOAPParser)
 *   76 sizeof(UClientImage<UTCPSocket>)
 *   80 sizeof(UIPAddress)
 *   80 sizeof(UMimeMultipart)
 *   80 sizeof(UMimeMultipartMsg)
 *  120 sizeof(UFile)
 *  128
 * -------------------------
 * U_STACK_TYPE_4
 * 
 *  148 sizeof(URDBClient<UTCPSocket>)
 *  152 sizeof(ULog)
 *  152 sizeof(USOAPClient<UTCPSocket>)
 *  168 sizeof(UFileConfig)
 *  180 sizeof(UHttpClient<UTCPSocket>)
 *  188 sizeof(USocket)
 *  188 sizeof(UTCPSocket)
 *  188 sizeof(UUDPSocket)
 *  208 sizeof(USSLSocket)
 *  220 sizeof(UCDB)
 *  240 sizeof(USmtpClient)
 *  256
 * -------------------------
 * U_STACK_TYPE_5
 * 
 *  368 sizeof(URDB)
 *  428 sizeof(UFtpClient)
 *  512
 * -------------------------
 * U_STACK_TYPE_6
 * =========================
 * DEBUG (32 bit) **
 * -------------------------
 *    1 sizeof(UNotifier)
 *    1 sizeof(UPlugIn<void*>)
 *    4 sizeof(UMagic)
 *    4 sizeof(UString) <==
 * -------------------------
 * U_STACK_TYPE_0
 * 
 *    8 sizeof(UCrl)
 *    8 sizeof(UPKCS10)
 *    8 sizeof(UCertificate)
 *   12 sizeof(ULock)
 *   12 sizeof(UTimer)
 *   12 sizeof(UPKCS7)
 *   12 sizeof(UTimeVal)
 *   12 sizeof(USemaphore)
 *   16 sizeof(UProcess)
 *   16 sizeof(URDBServer)
 *   16 sizeof(UVector<UString>)
 *   16 sizeof(UServer<UTCPSocket>)
 *   20 sizeof(UTimeDate)
 *   20 sizeof(UQueryNode)
 *   20 sizeof(USOAPFault)
 *   20 sizeof(UTokenizer)
 *   24 sizeof(USOAPObject)
 *   24 sizeof(UTree<UString>)
 *   28 sizeof(UStringRep) <==
 *   28 sizeof(USOAPGenericMethod)
 * -------------------------
 * U_STACK_TYPE_1
 * 
 *   32 sizeof(UCache)
 *   32 sizeof(UDialog)
 *   32 sizeof(UMimeEntity)
 *   32 sizeof(UQueryParser)
 *   32 sizeof(UHashMap<UString>)
 *   36 sizeof(UOptions)
 *   36 sizeof(USOAPEncoder)
 *   40 sizeof(Url)
 *   40 sizeof(UMimeHeader)
 *   44 sizeof(UPCRE)
 *   44 sizeof(UCommand)
 *   44 sizeof(UApplication)
 *   44 sizeof(UHTTP::UFileCacheData) <==
 * -------------------------
 * U_STACK_TYPE_2
 * 
 *   48 sizeof(UZIP)
 *   48 sizeof(UMimePKCS7)
 *   64 sizeof(UMimeMessage)
 * -------------------------
 * U_STACK_TYPE_3
 * 
 *   80 sizeof(UClientImage<UTCPSocket>)
 *   84 sizeof(UIPAddress)
 *   88 sizeof(USOAPParser)
 *   88 sizeof(UMimeMultipart)
 *   88 sizeof(UMimeMultipartMsg)
 *  124 sizeof(UFile)
 *  128
 * -------------------------
 * U_STACK_TYPE_4
 * 
 *  160 sizeof(URDBClient<UTCPSocket>)
 *  164 sizeof(ULog)
 *  164 sizeof(USOAPClient<UTCPSocket>)
 *  176 sizeof(UFileConfig)
 *  192 sizeof(UHttpClient<UTCPSocket>)
 *  200 sizeof(USocket)
 *  200 sizeof(UTCPSocket)
 *  200 sizeof(UUDPSocket)
 *  220 sizeof(USSLSocket)
 *  224 sizeof(UCDB)
 *  252 sizeof(USmtpClient)
 *  256
 * -------------------------
 * U_STACK_TYPE_5
 * 
 *  380 sizeof(URDB)
 *  452 sizeof(UFtpClient)
 *  512
 * -------------------------
 * U_STACK_TYPE_6
 * =========================
 */
#endif
/**
 * 1024
 *-------------------------
 * U_STACK_TYPE_7
 *
 * 2048
 *-------------------------
 * U_STACK_TYPE_8
 *
 * 4096
 *-------------------------
 * U_STACK_TYPE_9
 */

#ifdef HAVE_ARCH64
#  define U_STACK_TYPE_0  8U
#  ifndef DEBUG
#  define U_STACK_TYPE_1 24U
#  define U_STACK_TYPE_2 32U
#  define U_STACK_TYPE_3 56U
#  else
#  define U_STACK_TYPE_1 40U
#  define U_STACK_TYPE_2 48U
#  define U_STACK_TYPE_3 64U
#  endif
#else
#  define U_STACK_TYPE_0  4U
#  ifndef DEBUG
#  define U_STACK_TYPE_1 16U
#  define U_STACK_TYPE_2 40U
#  else
#  define U_STACK_TYPE_1 28U
#  define U_STACK_TYPE_2 44U
#  endif
#  define U_STACK_TYPE_3 64U
#endif

// NB: with U_NUM_ENTRY_MEM_BLOCK == 32 and 32bit arch are nedeed type stack multiple of 2 from 128 for pointers block...

#define U_STACK_TYPE_4  128U
#define U_STACK_TYPE_5  256U
#define U_STACK_TYPE_6  512U
#define U_STACK_TYPE_7 1024U
#define U_STACK_TYPE_8 2048U
#define U_STACK_TYPE_9 U_MAX_SIZE_PREALLOCATE

#define U_NUM_STACK_TYPE 10

// Implements a simple stack allocator

#define U_SIZE_TO_STACK_INDEX(sz) ((sz) <= U_STACK_TYPE_0 ? 0 : \
                                   (sz) <= U_STACK_TYPE_1 ? 1 : \
                                   (sz) <= U_STACK_TYPE_2 ? 2 : \
                                   (sz) <= U_STACK_TYPE_3 ? 3 : \
                                   (sz) <= U_STACK_TYPE_4 ? 4 : \
                                   (sz) <= U_STACK_TYPE_5 ? 5 : \
                                   (sz) <= U_STACK_TYPE_6 ? 6 : \
                                   (sz) <= U_STACK_TYPE_7 ? 7 : \
                                   (sz) <= U_STACK_TYPE_8 ? 8 : 9)

class UStringRep;
class UServer_Base;
class UNoCatPlugIn;
class UStackMemoryPool;

template <class T> class UVector;

// This avoids memory fragmentation and the overhead of heap traversal that malloc incurs

class U_EXPORT UMemoryPool {
public:
   static const uint32_t U_STACK_INDEX_TO_SIZE[U_NUM_STACK_TYPE]; // 10
   
#ifdef ENABLE_MEMPOOL
   static void* pop(            int stack_index);
   static void  push(void* ptr, int stack_index);
   static void _free(void* ptr, uint32_t num, uint32_t type_size = 1);

   static void allocateMemoryBlocks(const char* list);
   static void* pmalloc(uint32_t* pnum, uint32_t type_size = sizeof(char), bool bzero = false);
#else
   static void* pop(int stack_index)
      {
      U_TRACE(1, "UMemoryPool::pop(%d)", stack_index)

      U_INTERNAL_ASSERT_MINOR(stack_index, U_NUM_STACK_TYPE) // 10

      return U_SYSCALL(malloc, "%u", U_STACK_INDEX_TO_SIZE[stack_index]);
      }

   static void push(void* ptr, int stack_index)
      {
      U_TRACE(1, "UMemoryPool::push(%p,%d)", ptr, stack_index)

      U_INTERNAL_ASSERT_MINOR(stack_index, U_NUM_STACK_TYPE) // 10

      U_SYSCALL_VOID(free, "%p", ptr);
      }

   static void _free(void* ptr, uint32_t num, uint32_t type_size = sizeof(char))
      {
      U_TRACE(1, "UMemoryPool::_free(%p,%u,%u)", ptr, num, type_size)

      U_SYSCALL_VOID(free, "%p", ptr);
      }

   static void allocateMemoryBlocks(const char* ptr)
      {
      U_TRACE(0, "UMemoryPool::allocateMemoryBlocks(%S)", ptr)

      U_INTERNAL_ASSERT_POINTER(ptr)
      }

   static void* UMemoryPool::pmalloc(uint32_t* pnum, uint32_t type_size = sizeof(char), bool bzero = false)
      {
      U_TRACE(1, "UMemoryPool::pmalloc(%p,%u,%b)", pnum, type_size, bzero)

      U_INTERNAL_ASSERT_POINTER(pnum)
      U_INTERNAL_ASSERT_MAJOR(type_size, 0)

      uint32_t length = (*pnum * type_size); // in bytes

      U_INTERNAL_DUMP("length = %u", length)

#  ifndef HAVE_ARCH64
      U_INTERNAL_ASSERT_MINOR(length, 1U * 1024U * 1024U * 1024U) // NB: over 1G is very suspect on 32bit...
#  endif

      if (length == 0) length = type_size;

      ptr = U_SYSCALL(malloc, "%u", length);

      U_INTERNAL_ASSERT_POINTER_MSG(ptr, "cannot allocate memory, exiting...")

      if (bzero) (void) U_SYSCALL(memset, "%p,%d,%u", ptr, 0, length);

      U_RETURN(ptr);
      }

#endif

   static void* u_malloc(uint32_t num, uint32_t type_size = sizeof(char), bool bzero = false);

#ifdef DEBUG
   static const char* obj_class;
   static const char* func_call;

# ifdef ENABLE_MEMPOOL
   static bool check(void* ptr);
# endif
# ifdef U_STDCPP_ENABLE
#  ifdef __clang__
   static void printInfo(     ostream& os);
#  else
   static void printInfo(std::ostream& os);
#  endif
   static void writeInfoTo(const char* format, uint32_t fmt_size, ...);
# endif
#endif

private:
#ifdef ENABLE_MEMPOOL
   static void deallocate(void* ptr, uint32_t length);
#else
   static void deallocate(void* ptr, uint32_t length)
      {
      U_TRACE(1, "UMemoryPool::deallocate(%p,%u)", ptr, length)

      U_SYSCALL_VOID(free, "%p", ptr);
      }
#endif

#ifdef ENABLE_MEMPOOL
   static void allocateMemoryBlocks(int stack_index, uint32_t n);
#else
   static void allocateMemoryBlocks(int stack_index, uint32_t n)
      {
      U_TRACE(0+256, "UMemoryPool::allocateMemoryBlocks(%d,%u)", stack_index, n)

      U_INTERNAL_ASSERT_MAJOR(stack_index, 0)
      U_INTERNAL_ASSERT_MINOR(stack_index, U_NUM_STACK_TYPE) // 10
      }
#endif

   U_DISALLOW_COPY_AND_ASSIGN(UMemoryPool)

   friend class UStringRep;
   friend class UServer_Base;
   friend class UNoCatPlugIn;
   friend class UStackMemoryPool;

   friend void check_mmap(uint32_t);

   template <class T> friend class UVector;
};

#ifdef DEBUG
template <class T> bool u_check_memory_vector(T* _vec, uint32_t n)
{
   U_TRACE(0+256, "u_check_memory_vector<T>(%p,%u)", _vec, n)

   for (uint32_t i = 0; i < n; ++i) if (_vec[i].check_memory() == false) U_RETURN(false);

   U_RETURN(true);
}
#endif

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
#  define U_WRITE_MEM_POOL_INFO_TO(fmt,args...) UMemoryPool::writeInfoTo(U_CONSTANT_TO_PARAM(fmt) , ##args)
#else
#  define U_WRITE_MEM_POOL_INFO_TO(fmt,args...)
#endif

#ifdef ENABLE_MEMPOOL
#  define U_MALLOC_TYPE(  type) (type*)UMemoryPool::pop(     U_SIZE_TO_STACK_INDEX(sizeof(type)));U_INTERNAL_ASSERT(sizeof(type)<=U_MAX_SIZE_PREALLOCATE)
#  define U_FREE_TYPE(ptr,type)      { UMemoryPool::push(ptr,U_SIZE_TO_STACK_INDEX(sizeof(type)));U_INTERNAL_ASSERT(sizeof(type)<=U_MAX_SIZE_PREALLOCATE) } 

#  define U_MEMORY_ALLOCATOR \
void* operator new(  size_t sz)          { U_INTERNAL_ASSERT(sz <= U_MAX_SIZE_PREALLOCATE); return UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(sz)); } \
void* operator new[](size_t sz)          {                                                  return UMemoryPool::u_malloc(sz); }
#     define U_MEMORY_DEALLOCATOR \
void  operator delete(  void* _ptr, size_t sz) { U_INTERNAL_ASSERT(sz <= U_MAX_SIZE_PREALLOCATE); UMemoryPool::push( _ptr, U_SIZE_TO_STACK_INDEX(sz)); } \
void  operator delete[](void* _ptr, size_t sz) {                                                  UMemoryPool::_free(_ptr, sz); }
#else
#  define U_MALLOC_TYPE(  type) (type*)malloc(sizeof(type))
#  define U_FREE_TYPE(ptr,type) { free(ptr); }

#  define U_MEMORY_ALLOCATOR
#  define U_MEMORY_DEALLOCATOR
#endif

#endif
