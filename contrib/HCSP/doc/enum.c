/* Listing System and Physical Stores */

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN /* Exclude rarely-used stuff from Windows headers */
#  include <windows.h>
#  ifdef _WIN32
#     include <wincrypt.h>
#  else
#     define PSYSTEMTIME void*
#     include "./WinCrypt.h"
#  endif
#  include <winnls.h>
#else
#  include "./CSP.h"
#endif

#ifdef __MINGW32__
#  include "./CSP.fld" /* MinGW does not yet include all the needed definitions for CryptoAPI, so define here whatever extra is needed */
#endif

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

typedef struct _CERT_SYSTEM_STORE_RELOCATE_PARA {
    union {
        HKEY                hKeyBase;
        void                *pvBase;
    } DUMMYUNIONNAME;
    union {
        void                *pvSystemStore;
        LPCSTR              pszSystemStore;
        LPCWSTR             pwszSystemStore;
    } DUMMYUNIONNAME2;
} CERT_SYSTEM_STORE_RELOCATE_PARA, *PCERT_SYSTEM_STORE_RELOCATE_PARA;

static void MyHandleError(char *s);

typedef struct _ENUM_ARG {
    BOOL        fAll;
    BOOL        fVerbose;
    DWORD       dwFlags;
    const void  *pvStoreLocationPara;
    HKEY        hKeyBase;
} ENUM_ARG, *PENUM_ARG;

//-------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Declare callback functions. 
// Definitions of these functions follow main.

static BOOL WINAPI EnumPhyCallback(
       const void *pvSystemStore,
       DWORD dwFlags, 
       LPCWSTR pwszStoreName, 
       PCERT_PHYSICAL_STORE_INFO pStoreInfo,
       void *pvReserved, 
       void *pvArg);

static BOOL WINAPI EnumSysCallback(
       const void *pvSystemStore,
       DWORD dwFlags,
       PCERT_SYSTEM_STORE_INFO pStoreInfo,
       void *pvReserved,
       void *pvArg);

static BOOL WINAPI EnumLocCallback(
       LPCWSTR pwszStoreLocation,
       DWORD dwFlags,
       void *pvReserved,
       void *pvArg);

//-------------------------------------------------------------------
// Begin main.

void main(void) 
{
#ifdef __MINGW32__
   mingw_load_crypto_func();
#endif

//-------------------------------------------------------------------
// Declare and initialize variables.

DWORD dwExpectedError = 0;
DWORD dwLocationID = CERT_SYSTEM_STORE_CURRENT_USER_ID;
DWORD dwFlags = 0;
CERT_PHYSICAL_STORE_INFO PhyStoreInfo;
ENUM_ARG EnumArg;
LPSTR pszStoreParameters = NULL;          
LPWSTR pwszStoreParameters = NULL;
LPWSTR pwszSystemName = NULL;
LPWSTR pwszPhysicalName = NULL;
LPWSTR pwszStoreLocationPara = NULL;
void *pvSystemName;                   
void *pvStoreLocationPara;              
DWORD dwNameCnt = 0;
LPCSTR pszTestName;
HKEY hKeyRelocate = HKEY_CURRENT_USER;
LPSTR pszRelocate = NULL;               
HKEY hKeyBase = NULL;

//-------------------------------------------------------------------
//  Initialize data structure variables.

memset(&PhyStoreInfo, 0, sizeof(PhyStoreInfo));
PhyStoreInfo.cbSize = sizeof(PhyStoreInfo);
PhyStoreInfo.pszOpenStoreProvider = sz_CERT_STORE_PROV_SYSTEM_W;
pszTestName = "Enum";  
pvSystemName = pwszSystemName;
pvStoreLocationPara = pwszStoreLocationPara;

memset(&EnumArg, 0, sizeof(EnumArg));
EnumArg.dwFlags = dwFlags;
EnumArg.hKeyBase = hKeyBase;

EnumArg.pvStoreLocationPara = pvStoreLocationPara;
EnumArg.fAll = TRUE;
dwFlags &= ~CERT_SYSTEM_STORE_LOCATION_MASK;
dwFlags |= (dwLocationID << CERT_SYSTEM_STORE_LOCATION_SHIFT) &
    CERT_SYSTEM_STORE_LOCATION_MASK;

printf("Begin enumeration of store locations. \n");
if(CertEnumSystemStoreLocation(
    dwFlags,
    &EnumArg,
    EnumLocCallback
    ))
{
    printf("\nFinished enumerating locations. \n");
}
else
{
    MyHandleError("Enumeration of locations failed.");
}
printf("\nBegin enumeration of system stores. \n");

if(CertEnumSystemStore(
    dwFlags,
    pvStoreLocationPara,
    &EnumArg,
    EnumSysCallback
    ))
{
    printf("\nFinished enumerating system stores. \n");
}
else
{
    MyHandleError("Enumeration of system stores failed.");
}

printf("\n\nEnumerate the physical stores "
    "for the MY system store. \n");
if(CertEnumPhysicalStore(
    L"MY",
    dwFlags,
    &EnumArg,
    EnumPhyCallback
    ))
{
    printf("Finished enumeration of the physical stores. \n");
}
else
{
    MyHandleError("Enumeration of physical stores failed.");
}
}    //   End of main

//-------------------------------------------------------------------
//   Define function GetSystemName.

static BOOL GetSystemName( 
    const void *pvSystemStore,
    DWORD dwFlags, 
    PENUM_ARG pEnumArg, 
    LPCWSTR *ppwszSystemName )
{
//-------------------------------------------------------------------
// Declare local variables.

*ppwszSystemName = NULL;

if (pEnumArg->hKeyBase && 0 == (dwFlags & 
    CERT_SYSTEM_STORE_RELOCATE_FLAG)) 
{
  printf("Failed => RELOCATE_FLAG not set in callback. \n");
  return FALSE;
} 
else 
{
  if (dwFlags & CERT_SYSTEM_STORE_RELOCATE_FLAG) 
  {
     PCERT_SYSTEM_STORE_RELOCATE_PARA pRelocatePara;
     if (!pEnumArg->hKeyBase) 
     {
        MyHandleError("Failed => RELOCATE_FLAG is set in callback");
     }
     pRelocatePara = (PCERT_SYSTEM_STORE_RELOCATE_PARA) 
         pvSystemStore;
     if (pRelocatePara->hKeyBase != pEnumArg->hKeyBase) 
     {
         MyHandleError("Wrong hKeyBase passed to callback");
     }
     *ppwszSystemName = pRelocatePara->pwszSystemStore;
  } 
  else
  {
    *ppwszSystemName = (LPCWSTR) pvSystemStore;
  }
}
return TRUE;
}

//-------------------------------------------------------------------
// Define the callback functions.

static BOOL WINAPI EnumPhyCallback(
      const void *pvSystemStore,
      DWORD dwFlags, 
      LPCWSTR pwszStoreName, 
      PCERT_PHYSICAL_STORE_INFO pStoreInfo,
      void *pvReserved, 
      void *pvArg )
{
//-------------------------------------------------------------------
//  Declare and initialize local variables.
PENUM_ARG pEnumArg = (PENUM_ARG) pvArg;
LPCWSTR pwszSystemStore;

//-------------------------------------------------------------------
//  Begin callback process.

if (GetSystemName(
       pvSystemStore, 
       dwFlags, 
       pEnumArg, 
       &pwszSystemStore))
{
printf("    %S", pwszStoreName);
}
else
{
   MyHandleError("GetSystemName failed.");
}
if (pEnumArg->fVerbose &&
      (dwFlags & CERT_PHYSICAL_STORE_PREDEFINED_ENUM_FLAG))
      printf(" (implicitly created)");
printf("\n"); 
return TRUE;
}

static BOOL WINAPI EnumSysCallback(
    const void *pvSystemStore,
    DWORD dwFlags,
    PCERT_SYSTEM_STORE_INFO pStoreInfo,
    void *pvReserved,
    void *pvArg)
//-------------------------------------------------------------------
//  Begin callback process.
{
//-------------------------------------------------------------------
//  Declare and initialize local variables.

PENUM_ARG pEnumArg = (PENUM_ARG) pvArg;
LPCWSTR pwszSystemStore;
static int line_counter=0;
char x;

//-------------------------------------------------------------------
//  Begin processing.

//-------------------------------------------------------------------
//  Prepare and display the next detail line.

if (GetSystemName(pvSystemStore, dwFlags, pEnumArg, &pwszSystemStore))
{
     printf("  %S\n", pwszSystemStore);
}
else
{
     MyHandleError("GetSystemName failed.");
}
if (pEnumArg->fAll || pEnumArg->fVerbose) 
{
    dwFlags &= CERT_SYSTEM_STORE_MASK;
    dwFlags |= pEnumArg->dwFlags & ~CERT_SYSTEM_STORE_MASK;
    if (!CertEnumPhysicalStore(
       pvSystemStore,
       dwFlags,
       pEnumArg,
       EnumPhyCallback
       )) 
    {
        DWORD dwErr = GetLastError();
        if (!(ERROR_FILE_NOT_FOUND == dwErr ||
            ERROR_NOT_SUPPORTED == dwErr))
        {
               printf("    CertEnumPhysicalStore");
        }
    }
}
return TRUE;
}

static BOOL WINAPI EnumLocCallback(
    LPCWSTR pwszStoreLocation,
    DWORD dwFlags,
    void *pvReserved,
    void *pvArg)

{
//-------------------------------------------------------------------
//  Declare and initialize local variables.

PENUM_ARG pEnumArg = (PENUM_ARG) pvArg;
DWORD dwLocationID = (dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK) >>
   CERT_SYSTEM_STORE_LOCATION_SHIFT;
static int linecount=0;
char x;

//-------------------------------------------------------------------
//  Begin processing.

//-------------------------------------------------------------------
//  Prepare and display the next detail line.

printf("======   %S   ======\n", pwszStoreLocation);
if (pEnumArg->fAll) 
{
    dwFlags &= CERT_SYSTEM_STORE_MASK;
    dwFlags |= pEnumArg->dwFlags & ~CERT_SYSTEM_STORE_LOCATION_MASK;
    CertEnumSystemStore(
         dwFlags,
         (void *) pEnumArg->pvStoreLocationPara,
         pEnumArg,
         EnumSysCallback ); 
}
return TRUE;
}

//-------------------------------------------------------------------
//  This example uses the function MyHandleError, a simple error
//  handling function, to print an error message to  
//  the standard error (stderr) file and exit the program. 
//  For most applications, replace this function with one 
//  that does more extensive error reporting.

void MyHandleError(char *s)
{
    fprintf(stderr,"An error occurred in running the program. \n");
    fprintf(stderr,"%s\n",s);
    fprintf(stderr, "Error number %x.\n", GetLastError());
    fprintf(stderr, "Program terminating. \n");
    exit(1);
} // End of MyHandleError
