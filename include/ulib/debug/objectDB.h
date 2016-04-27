// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    objectDB.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_OBJECTDB_H
#define ULIB_OBJECTDB_H 1

#include <ulib/base/base.h>

/**
 * class UObjectDumpable
 *
 * Base class that defines a uniform interface for all object dumping
 *
 * A mechanism that allow all objects to be registered with a central in-memory "database" that can dump the state of all
 * live objects. The macros which allow easy registration and removal of objects to be dumped (U_REGISTER_OBJECT_PTR,
 * U_TRACE_REGISTER_OBJECT and U_UNREGISTER_OBJECT, U_TRACE_UNREGISTER_OBJECT) are turned into no-ops by compiling with
 * the DEBUG macro undefined. This allows usage to be removed in "release mode" builds without changing code. There are
 * several interesting aspects to this design:
 *
 * 1. It uses the External Polymorphism pattern to avoid having to derive all classes from a common base class that has
 * virtual methods (this is crucial to avoid unnecessary overhead). In addition, there is no additional space added
 * to objects (this is crucial to maintain binary layout compatibility)
 *
 * 2. This mechanism can be conditionally compiled in order to completely disable this feature entirely. Moreover, by
 * using macros there are relatively few changes to code
 *
 * 3. This mechanism copes with single-inheritance hierarchies of dumpable classes. In such cases we typically want only
 * one dump, corresponding to the most derived instance. Note, however, that this scheme doesn't generalize to work
 * with multiple-inheritance or virtual base classes
 */

class UObjectDB;
class UHashMapObjectDumpable;

class U_NO_EXPORT UObjectDumpable {
public:

   UObjectDumpable(int lv, const char* name, const void* pobj, uint32_t sz, const void** pmem)
               : level(lv), name_class(name), ptr_object(pobj),  sz_obj(sz),   psentinel(pmem)
      {
      cnt       = 0;
      num_line  = 0;
      name_file = name_function = 0;
      }

   virtual ~UObjectDumpable() {}

   virtual const char* dump() const = 0; // This pure virtual method must be filled in by a subclass

protected:
   int level, num_line;
   const char* name_file;
   const char* name_class;
   const char* name_function;
   const void* ptr_object; // Pointer to the object that is being stored
   uint32_t sz_obj, cnt;
   const void** psentinel; // Pointer to the memory sentinel of the object (for memory check)

   friend class UObjectDB;
   friend class UHashMapObjectDumpable;
};

/**
 * class UObjectDumpable_Adapter
 *
 * This class inherits the interface of the abstract UObjectDumpable class and is instantiated with the implementation
 * of the concrete component 'class Concrete'. This design is similar to the Adapter and Decorator patterns from the
 * 'Gang of Four' book. Note that 'class Concrete' need not inherit from a common class since UObjectDumpable
 * provides the uniform virtual interface!
 */

template <class Concrete> class U_NO_EXPORT UObjectDumpable_Adapter : public UObjectDumpable {
public:

   UObjectDumpable_Adapter(int lv, const char* name, const Concrete* pobj, const void** pmem) : UObjectDumpable(lv, name, pobj, sizeof(Concrete), pmem)
      {
      U_INTERNAL_TRACE("UObjectDumpable_Adapter::UObjectDumpable_Adapter(%d,%s,%p,%p)", lv, name, pobj, pmem)

      U_INTERNAL_PRINT("this = %p", this)
      }

   ~UObjectDumpable_Adapter()
      {
      U_INTERNAL_TRACE("UObjectDumpable_Adapter::~UObjectDumpable_Adapter()", 0)

      U_INTERNAL_PRINT("this = %p", this)
      }

   // Concrete dump method (simply delegates to the 'dump()' method of class Concrete)

   virtual const char* dump() const __pure { return ((const Concrete*)ptr_object)->dump(true); }
};

/**
 * class UObjectDB
 *
 * This is the object database (ODB) that keeps track of all live objects
 */

class U_NO_EXPORT UObjectDB {
public:

   static bool U_EXPORT flag_new_object;
   static int  U_EXPORT fd, level_active;
   static bool U_EXPORT flag_ulib_object;

   static void close();
   static void initFork();
   static void init(bool flag, bool info);

   static void U_EXPORT   registerObject(UObjectDumpable* dumper); // Add the 'dumper' to the list of registered objects
   static void U_EXPORT unregisterObject(const void* ptr_object);  // Use 'ptr_object' to locate and remove the associated 'dumper' from the list of registered objects

   // Iterates through the entire set of registered objects and dumps their state

   static void     dumpObjects();
   static void     dumpObject(const UObjectDumpable* dumper);
   static uint32_t dumpObject(char* buffer, uint32_t buffer_size, bPFpcpv check_object) U_EXPORT;
   static uint32_t dumpObject(char* buffer, uint32_t buffer_size, const void* ptr_object);

private:
   static char*    file_ptr;
   static char*    file_mem;
   static char*    file_limit;
   static uint32_t file_size;

   static char* lbuf;
   static char* lend;
   static uint32_t n;
   static iovec liov[8];
   static char buffer1[64];
   static char buffer2[256];
   static char buffer3[64];
   static bPFpcpv checkObject;
   static const char* _name_class;
   static const void* _ptr_object;
   static const UObjectDumpable** vec_obj_live;

   static void _write(const struct iovec* iov, int n) U_NO_EXPORT;
   static bool addObjLive(const UObjectDumpable* dumper) U_NO_EXPORT;
   static bool printObjLive(const UObjectDumpable* dumper) U_NO_EXPORT;
   static int  compareDumper(const void* dumper1, const void* dumper2) __pure U_NO_EXPORT;
   static bool checkIfObject(const char* name_class, const void* ptr_object) __pure U_NO_EXPORT;
};

#endif
