// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    zip.h - interface to the zip compression format
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_ZIP_H
#define U_ZIP_H 1

#include <ulib/base/base.h>
#include <ulib/base/zip/ziptool.h>

#include <ulib/file.h>
#include <ulib/container/vector.h>

#define U_ZIP_ARCHIVE "PK\003\004"

class U_EXPORT UZIP {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UZIP();
   UZIP(const UString& content);

   /**
    * Deletes this object
    */

   ~UZIP()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UZIP)

      clear();
      }

   /**
    * Returns bool value to indicate the correctness of the zip data
    */

   bool isValid() const
      {
      U_TRACE_NO_PARAM(0, "UZIP::isValid()")

      U_RETURN(valid);
      }

   bool extract(                     const UString* tmpdir = 0, bool bdir = true);
   bool extract(const UString& data, const UString* tmpdir = 0, bool bdir = true);

   void   clear();
   bool   readContent();
   char** getFilenames() const { return filenames; }

   uint32_t getFilesCount() const          { return zippartname->size(); }
   UString  getFilenameAt(int index) const { return zippartname->at(index); }
   UString  getFileContentAt(int index);

   // STORE

   UString archive(const char** add_to_filenames);

   static bool archive(const char* zipfile, const char** _filenames)
      {
      U_TRACE(1, "UZIP::archive(%S,%p)", zipfile, _filenames)

      bool result = (U_SYSCALL(zip_archive, "%S,%p", zipfile, _filenames) == 0);

      U_RETURN(result);
      }

   // OPERATOR

   UString operator[](uint32_t pos) const { return getFilenameAt(pos); }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UFile* file;
   char** filenames;
   char** filecontents;
   uint32_t* filenames_len;
   uint32_t* filecontents_len;
   UVector<UString>* zippartname;
   UVector<UString>* zippartcontent;
   UString content, tmpdir;
   uint32_t npart;
   bool valid;

private:
   void assignFilenames() U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UZIP)
};

#endif
