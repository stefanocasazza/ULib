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

#ifndef ULIB_INTERNAL_MACRO_H
#define ULIB_INTERNAL_MACRO_H 1

#define U_SINGLE_READ     0U
#define U_TIMEOUT_MS     (30L * 1000L) // 30 second connection/read timeout
#define U_SSL_TIMEOUT_MS (10L * 1000L) // 10 second handshake       timeout

// NB: to avoid mis-aligned we use 4 bytes...
#define U_MINIZ_COMPRESS "\x89MNZ" // "\211MNZ" "\x89\x4d\x4e\x5a"

#define U_PATH_MAX (1024U - (1 + sizeof(ustringrep)))
// -------------------------------------------------------------------------------------------------------------------
// NB: the value must be a stack type boundary, see UStringRep::checkIfMReserve()...
// -------------------------------------------------------------------------------------------------------------------
#define U_CAPACITY (U_MAX_SIZE_PREALLOCATE - (1 + sizeof(ustringrep))) // UStringRep::max_size(U_MAX_SIZE_PREALLOCATE)
// -------------------------------------------------------------------------------------------------------------------
#define U_STRING_MAX_SIZE (((U_NOT_FOUND-sizeof(ustringrep))/sizeof(char))-4096)

// default move assignment operator
#if defined(U_COMPILER_RVALUE_REFS) && \
  (defined(U_COVERITY_FALSE_POSITIVE) || !defined(__GNUC__) || GCC_VERSION_NUM > 50300) // GCC has problems dealing with move constructor, so turn the feature on for 5.3.1 and above, only
#  define U_MOVE_ASSIGNMENT(TypeName) \
      TypeName(TypeName &&) = default; \
      TypeName& operator=(TypeName &&) = default;
#else
#  define U_MOVE_ASSIGNMENT(TypeName)
#endif

// Put this in the declarations for a class to be unassignable
#ifdef U_COMPILER_DELETE_MEMBERS
#  define U_DISALLOW_ASSIGN(TypeName) \
      void operator=(const TypeName&) = delete;
#else
#  define U_DISALLOW_ASSIGN(TypeName) \
      void operator=(const TypeName&) {}
#endif

// A macro to disallow the copy constructor and operator= functions.
// This should be used in the private: declarations for a class
#ifdef U_COMPILER_DELETE_MEMBERS
#  define U_DISALLOW_COPY_AND_ASSIGN(TypeName) \
      TypeName(const TypeName&) = delete; \
      void operator=(const TypeName&) = delete;
#else
#  define U_DISALLOW_COPY_AND_ASSIGN(TypeName) \
      TypeName(const TypeName&) {} \
      void operator=(const TypeName&) {}
#endif

#endif
