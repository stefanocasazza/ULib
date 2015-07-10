// utility.cpp

#include "utility.h"

void UXAdESUtility::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(5, "UXAdESUtility::handlerConfig(%p)", &cfg)

   UString tmp;

   MSname = cfg[U_STRING_FROM_CONSTANT(    "MS-WORD.SignatureContent")];
   OOname = cfg[U_STRING_FROM_CONSTANT("OPEN-OFFICE.SignatureContent")];

   MSname.duplicate();
   OOname.duplicate();

   tmp = cfg[U_STRING_FROM_CONSTANT("OPEN-OFFICE.ToBeSigned")];

   (void) OOToBeSigned.split(U_STRING_TO_PARAM(tmp));

   tmp = cfg[U_STRING_FROM_CONSTANT("OPEN-OFFICE.ZipStructure")];

   (void) OOZipStructure.split(U_STRING_TO_PARAM(tmp));

   tmp = cfg[U_STRING_FROM_CONSTANT("MS-WORD.ToBeSigned")];

   (void) MSToBeSigned.split(U_STRING_TO_PARAM(tmp));

   tmp = cfg[U_STRING_FROM_CONSTANT("MS-WORD.ZipStructure")];

   (void) MSZipStructure.split(U_STRING_TO_PARAM(tmp));

   tmp = cfg[U_STRING_FROM_CONSTANT("MS-WORD.SignatureStructure")];

   (void) MSSignatureStructure.split(U_STRING_TO_PARAM(tmp));

   cfg.destroy();
}

// ---------------------------------------------------------------------------------------------------------------
// check for OOffice or MS-Word document...
// ---------------------------------------------------------------------------------------------------------------

bool UXAdESUtility::checkDocument(const UString& document, const char* pathname, bool adjust)
{
   U_TRACE(5, "UXAdESUtility::checkDocument(%.*S,%S,%b)", U_STRING_TO_TRACE(document), pathname, adjust)

   msword = ooffice = false;

   (void) tmpdir.reserve(100U);

   tmpdir.snprintf("%s/%s", u_tmpdir, u_basename(pathname));

   if (zip.extract(document, &tmpdir, false))
      {
      UString namefile;
      uint32_t i, n, index;

      for (i = 0, n = zip.getFilesCount(); i < n; ++i)
         {
         namefile = zip.getFilenameAt(i);

         ZipStructure.push(namefile);
           ZipContent.push(zip.getFileContentAt(i));

         U_INTERNAL_DUMP("Part %d: Filename=%.*S", i+1, U_STRING_TO_TRACE(namefile));
         }

      U_INTERNAL_DUMP("ZIP: %d parts", n)

      msword = true;

      for (i = 0, n = MSZipStructure.size(); i < n; ++i)
         {
         namefile = MSZipStructure[i];

         if (ZipStructure.isContained(namefile)) continue;

         msword  = false;
         ooffice = true;

         for (i = 0, n = OOZipStructure.size(); i < n; ++i)
            {
            namefile = OOZipStructure[i];

            if (ZipStructure.isContained(namefile)) continue;

            ooffice = false;

            break;
            }

         break;
         }

      (void) docout.reserve(100U);

      if (msword)
         {
         UString content,
                 content1_name = U_STRING_FROM_CONSTANT("[Content_Types].xml"), content1,
                 content2_name = U_STRING_FROM_CONSTANT("_rels/.rels"),         content2;

         if (adjust)
            {
            index = ZipStructure.contains(content1_name);

            if (index != U_NOT_FOUND)
               {
               content1 = ZipContent[index];

               content1.duplicate();

               (void) content1.erase(U_STRING_RFIND(content1, "</Types>"));

               (void) content1.append(U_CONSTANT_TO_PARAM("<Default Extension=\"sigs\" "
                           "ContentType=\"application/vnd.openxmlformats-package.digital-signature-origin\" />"
                           "<Override PartName=\"/_xmlsignatures/sig1.xml\" "
                           "ContentType=\"application/vnd.openxmlformats-package.digital-signature-xmlsignature+xml\" />"
                           "</Types>"));

               ZipContent.replace(index, content1);

               docout.snprintf("%.*s/[Content_Types].xml", U_STRING_TO_TRACE(tmpdir));

               (void) UFile::writeTo(docout, content1);
               }

            index = ZipStructure.contains(content2_name);

            if (index != U_NOT_FOUND)
               {
               content2 = ZipContent[index];

               content2.duplicate();

               (void) content2.erase(U_STRING_RFIND(content2, "</Relationships>"));

               (void) content2.append(U_CONSTANT_TO_PARAM("<Relationship Id=\"rId4\" "
                           "Type=\"http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin\" "
                           "Target=\"_xmlsignatures/origin.sigs\" />"
                           "</Relationships>"));

               ZipContent.replace(index, content2);

               docout.snprintf("%.*s/_rels/.rels", U_STRING_TO_TRACE(tmpdir));

               (void) UFile::writeTo(docout, content2);
               }
            }

         for (i = 0, n = MSToBeSigned.size(); i < n; ++i)
            {
            namefile = MSToBeSigned[i];
            index    = ZipStructure.contains(namefile);

            if (index == U_NOT_FOUND) continue;

            content = ZipContent[index];

            if (adjust)
               {
                    if (namefile == content1_name) content = content1;
               else if (namefile == content2_name) content = content2;
               }

                 vuri.push(namefile);
            vdocument.push(content);
            }

         docout.snprintf("%.*s/%.*s", U_STRING_TO_TRACE(tmpdir), U_STRING_TO_TRACE(MSname));

         U_RETURN(true);
         }

      if (ooffice)
         {
         UString content(U_CAPACITY),
                 content1_name = U_STRING_FROM_CONSTANT("META-INF/manifest.xml"), content1;

         if (adjust)
            {
            index = ZipStructure.contains(content1_name);

            if (index != U_NOT_FOUND)
               {
               content1 = ZipContent[index];

               content1.duplicate();

               (void) content1.erase(U_STRING_RFIND(content1, "</manifest:manifest>"));

               content.snprintf("<manifest:file-entry manifest:media-type=\"\" manifest:full-path=\"META-INF/\"/>"
                                "<manifest:file-entry manifest:media-type=\"\" manifest:full-path=\"%.*s\"/>"
                                "</manifest:manifest>", U_STRING_TO_TRACE(OOname));

               (void) content1.append(content);

               ZipContent.replace(index, content1);

               docout.snprintf("%.*s/META-INF/manifest.xml", U_STRING_TO_TRACE(tmpdir));

               (void) UFile::writeTo(docout, content1);
               }
            }

         for (i = 0, n = OOToBeSigned.size(); i < n; ++i)
            {
            namefile = OOToBeSigned[i];
            index    = ZipStructure.contains(namefile);

            if (index == U_NOT_FOUND) continue;

            content = ZipContent[index];

            if (adjust)
               {
               if (namefile == content1_name) content = content1;
               }

                 vuri.push(namefile);
            vdocument.push(content);
            }

         docout.snprintf("%.*s/%.*s", U_STRING_TO_TRACE(tmpdir), U_STRING_TO_TRACE(OOname));

         U_RETURN(true);
         }
      }

   vdocument.push(document);
        vuri.push(UString(pathname));

   U_RETURN(false);
}

UString UXAdESUtility::getSigned()
{
   U_TRACE(5, "UXAdESUtility::getSigned()")

   UString name = (msword ? MSname : OOname);

   if (name)
      {
      uint32_t index = ZipStructure.contains(name);

      if (index != U_NOT_FOUND)
         {
         UString firma = ZipContent[index];

         U_RETURN_STRING(firma);
         }
      }

   return UString::getStringNull();
}

void UXAdESUtility::outputDocument(const UString& firma)
{
   U_TRACE(5, "UXAdESUtility::outputDocument(%.*S)", U_STRING_TO_TRACE(firma))

   UString output = firma;

   if (msword ||
       ooffice)
      {
      UString x = getSigned();

      if (x.empty()) x = (msword ? MSname : OOname);

      const char* add_to_filenames[32];

      add_to_filenames[0] = (x ? x.c_str() : 0);
      add_to_filenames[1] =                  0;

      (void) UFile::writeTo(docout, firma, false, true);

      if (msword &&
          MSSignatureStructure.empty() == false)
         {
         UString namefile, pcontent, tpath(100U);

         int32_t j = 1, n = MSSignatureStructure.size();

         for (int32_t i = 0; i < n; i += 2)
            {
            namefile = MSSignatureStructure[i];

            if (ZipStructure.isContained(namefile)) continue;

            pcontent = MSSignatureStructure[i+1];

            tpath.snprintf("%.*s/%.*s", U_STRING_TO_TRACE(tmpdir), U_STRING_TO_TRACE(namefile));

            (void) UFile::writeTo(tpath, UFile::contentOf(pcontent), false, true);

            add_to_filenames[j++] = strdup(namefile.c_str());
            }

         add_to_filenames[j] = 0;
         }

      output = zip.archive(add_to_filenames);
      }

#ifdef _MSWINDOWS_
   (void) setmode(1, O_BINARY);
#endif

   std::cout.write(U_STRING_TO_PARAM(output));

   output.clear();

   clean();
}

void UXAdESUtility::clean()
{
   U_TRACE(5, "UXAdESUtility::clean()")

   vdocument.clear();
   ZipContent.clear();
   MSSignatureStructure.clear();

   vuri.clear();
   ZipStructure.clear();
   OOToBeSigned.clear();
   MSToBeSigned.clear();
   OOZipStructure.clear();
   MSZipStructure.clear();

   zip.clear();
}
