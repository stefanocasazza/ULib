// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    zip.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/zip/zip.h>

UZIP::UZIP() : tmpdir(U_CAPACITY)
{
   U_TRACE_REGISTER_OBJECT(0, UZIP, "")

   npart         = 0;
   file          = 0;
   valid         = false;
   filenames     = filecontents = 0;
   zippartname   = zippartcontent = 0;
   filenames_len = filecontents_len = 0;
}

UZIP::UZIP(const UString& _content) : content(_content), tmpdir(U_CAPACITY)
{
   U_TRACE_REGISTER_OBJECT(0, UZIP, "%V", _content.rep)

   npart         = 0;
   file          = 0;
   valid         = (strncmp(_content.data(), U_CONSTANT_TO_PARAM(U_ZIP_ARCHIVE)) == 0);
   filenames     = filecontents = 0;
   zippartname   = zippartcontent = 0;
   filenames_len = filecontents_len = 0;
}

void UZIP::clear()
{
   U_TRACE_NO_PARAM(0, "UZIP::clear()")

   if (file)
      {
      U_INTERNAL_ASSERT(tmpdir)

      delete file;
             file = 0;

      (void) UFile::rmdir(tmpdir, true);
      }

   if (zippartname)
      {
      delete zippartname;
             zippartname = 0;

      U_INTERNAL_ASSERT_MAJOR(npart,0)

      for (uint32_t i = 0; i < npart; ++i)
         {
         U_SYSCALL_VOID(free, "%p", filenames[i]);
         }

      U_SYSCALL_VOID(free, "%p", filenames);
      U_SYSCALL_VOID(free, "%p", filenames_len);

      filenames     = 0;
      filenames_len = 0;
      }

   if (zippartcontent)
      {
      delete zippartcontent;
             zippartcontent = 0;

      U_INTERNAL_ASSERT_MAJOR(npart,0)

      for (uint32_t i = 0; i < npart; ++i)
         {
         U_SYSCALL_VOID(free, "%p", filecontents[i]);
         }

      U_SYSCALL_VOID(free, "%p", filecontents);
      U_SYSCALL_VOID(free, "%p", filecontents_len);

      filecontents     = 0;
      filecontents_len = 0;
      }
}

U_NO_EXPORT void UZIP::assignFilenames()
{
   U_TRACE_NO_PARAM(0, "UZIP::assignFilenames()")

   U_INTERNAL_ASSERT_MAJOR(npart, 0)
   U_INTERNAL_ASSERT_POINTER(filenames)
   U_INTERNAL_ASSERT_EQUALS(zippartname, 0)
   U_INTERNAL_ASSERT_POINTER(filenames_len)

   U_NEW(UVector<UString>, zippartname, UVector<UString>(npart));

   for (uint32_t i = 0; i < npart; ++i)
      {
      UString name(filenames[i], filenames_len[i]);

      zippartname->push_back(name);
      }
}

bool UZIP::extract(const UString* _tmpdir, bool bdir)
{
   U_TRACE(1, "UZIP::extract(%p,%b)", _tmpdir, bdir)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(content)
   U_INTERNAL_ASSERT_EQUALS((bool)tmpdir, false)

   if (_tmpdir) tmpdir = *_tmpdir;
   else
      {
      static uint32_t index;

      tmpdir.snprintf(U_CONSTANT_TO_PARAM("/tmp/UZIP_TMP_%P_%u"), index++);
      }

   const char* dir = tmpdir.c_str();

   if (UFile::mkdirs(dir) &&
       UFile::chdir(dir, true))
      {
      if (file == 0) U_NEW(UFile, file, UFile);

      file->setPath(U_STRING_FROM_CONSTANT("tmp.zip"));

      if (file->creat() &&
          file->write(content))
         {
         file->fsync();
         file->close();

         npart = U_SYSCALL(zip_extract, "%S,%p,%p,%p", "tmp.zip", 0, &filenames, &filenames_len);
         }

      if (bdir) (void) UFile::chdir(0, true);
      }

   if (npart > 0)
      {
      assignFilenames();

      U_RETURN(true);
      }

   clear();

   U_RETURN(false);
}

bool UZIP::extract(const UString& data, const UString* _tmpdir, bool bdir)
{
   U_TRACE(0, "UZIP::extract(%V,%p,%b)", data.rep, _tmpdir, bdir)

   U_INTERNAL_ASSERT_EQUALS(valid, false)

   if (strncmp(data.data(), U_CONSTANT_TO_PARAM(U_ZIP_ARCHIVE)) == 0)
      {
      content = data;
      valid   = extract(_tmpdir, bdir);
      }

   U_RETURN(valid);
}

UString UZIP::archive(const char** add_to_filenames)
{
   U_TRACE(1, "UZIP::archive(%p)", add_to_filenames)

   U_INTERNAL_ASSERT(tmpdir)
   U_INTERNAL_ASSERT(content)
   U_INTERNAL_ASSERT_MAJOR(npart, 0)
   U_INTERNAL_ASSERT_POINTER(add_to_filenames)

   UString result;
   const char* dir = tmpdir.c_str();

   if (UFile::chdir(dir, true))
      {
      uint32_t i, j;
      const char* names[1024];

      for (i = 0; i < npart; ++i)
         {
         names[i] = (const char*) filenames[i];

         U_INTERNAL_DUMP("name[%d] = %S", i, names[i])

         U_INTERNAL_ASSERT_POINTER(names[i])
         U_INTERNAL_ASSERT(names[i][0])
         }

      for (j = 0; (names[i] = add_to_filenames[j]); ++i,++j)
         {
         U_INTERNAL_DUMP("name[%d] = %S", i, names[i])

         U_INTERNAL_ASSERT_POINTER(names[i])
         U_INTERNAL_ASSERT(names[i][0])
         }

      if (U_SYSCALL(zip_archive, "%S,%p", "tmp.zip", names) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(zip_match("tmp.zip", names), 1)

         result = UFile::contentOf(U_STRING_FROM_CONSTANT("tmp.zip"));
         }

      (void) UFile::chdir(0, true);
      }

   U_RETURN_STRING(result);
}

bool UZIP::readContent()
{
   U_TRACE_NO_PARAM(1, "UZIP::readContent()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(content)
   U_INTERNAL_ASSERT_EQUALS(zippartname, 0)

   npart = U_SYSCALL(zip_get_content, "%p,%u,%p,%p,%p,%p", U_STRING_TO_PARAM(content), &filenames, &filenames_len, &filecontents, &filecontents_len);

   if (npart > 0)
      {
      assignFilenames();

      U_NEW(UVector<UString>, zippartcontent, UVector<UString>(npart));

      for (uint32_t i = 0; i < npart; ++i)
         {
         UString item(filecontents[i], filecontents_len[i]);

         zippartcontent->push_back(item);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString UZIP::getFileContentAt(int index)
{
   U_TRACE(0, "UZIP::getFileContentAt(%d)", index)

   U_CHECK_MEMORY

   UString dati;

   if (zippartcontent) dati = zippartcontent->at(index);
   else
      {
      U_INTERNAL_ASSERT_POINTER(file)

      UString filename = zippartname->at(index), buffer(U_CAPACITY);

      buffer.snprintf(U_CONSTANT_TO_PARAM("%v/%v"), tmpdir.rep, filename.rep);

      dati = (file->setPath(buffer), file->getContent(true, true));
      }

   U_RETURN_STRING(dati);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UZIP::dump(bool reset) const
{
   *UObjectIO::os << "npart                            " << npart                      << '\n'
                  << "valid                            " << valid                      << '\n'
                  << "filenames                        " << (void*)filenames           << '\n'
                  << "filecontents                     " << (void*)filecontents        << '\n'
                  << "filenames_len                    " << (void*)filenames_len       << '\n'
                  << "filecontents_len                 " << (void*)filecontents_len    << '\n'
                  << "file           (UFile            " << (void*)file                << ")\n"
                  << "tmpdir         (UString          " << (void*)&tmpdir             << ")\n"
                  << "content        (UString          " << (void*)&content            << ")\n"
                  << "zippartname    (UVector<UString> " << (void*)zippartname         << ")\n"
                  << "zippartcontent (UVector<UString> " << (void*)zippartcontent      << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
