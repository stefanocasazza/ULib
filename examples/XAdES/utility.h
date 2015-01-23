// utility.h

#ifndef ULIB_XAdES_UTILITY
#define ULIB_XAdES_UTILITY 1

#include <ulib/zip/zip.h>
#include <ulib/file_config.h>

struct U_EXPORT UXAdESUtility {

   UZIP zip;
   bool ooffice, msword;
   UString MSname, OOname, tmpdir, docout;
   UVector<UString> vuri, vdocument, OOZipStructure, MSZipStructure, ZipStructure, ZipContent, OOToBeSigned, MSToBeSigned, MSSignatureStructure;

   void clean();
   void handlerConfig(UFileConfig& cfg);

   // ---------------------------------------------------------------------------------------------------------------
   // check for OOffice or MS-Word document...
   // ---------------------------------------------------------------------------------------------------------------

   bool checkDocument(const UString& document, const char* data_uri, bool adjust);

   UString getSigned();
   void    outputDocument(const UString& firma);
};

#endif
