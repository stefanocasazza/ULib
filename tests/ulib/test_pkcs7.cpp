// test_pkcs7.cpp

#include <ulib/file.h>
#include <ulib/ssl/certificate.h>
#include <ulib/utility/services.h>
#include <ulib/ssl/mime/mime_pkcs7.h>

static void check_cert(UCertificate* cert)
{
   U_TRACE(5, "check_cert(%p)", cert)

   cout << cert->getSubject()  <<
           cert->getIssuer()   <<
           cert->getFileName() <<
           cert->verify()      << "\n";
}

static void check(const UString& dati)
{
   U_TRACE(5, "check(%.*S)", U_STRING_TO_TRACE(dati))

   UPKCS7* item;
   UMimePKCS7* item0 = 0;

   if (dati.find(U_STRING_FROM_CONSTANT("MIME-Version")) == U_NOT_FOUND) item = new UPKCS7(dati);
   else
      {
      item0 = new UMimePKCS7(dati);

      item  = &(item0->getPKCS7());
      }

   cout << item->isDetached()                  <<
           item->isMessageEncrypted()          <<
           item->isMessageSignedAndEncrypted() <<
           item->verify(0)                     << "\n";

   UVector<UCertificate*> vec;

   unsigned n = item->getSignerCertificates(vec);

   for (unsigned i = 0; i < n; ++i) check_cert(vec[i]);

   if (item0) delete item0;
   else       delete item;
}

/* tarchive_cert.pem */
#define SIGNERFILE \
"-----BEGIN CERTIFICATE-----\n" \
"MIIHsTCCBZmgAwIBAgIBFjANBgkqhkiG9w0BAQUFADCBljELMAkGA1UEBhMCSVQx\n" \
"DTALBgNVBAgTBFJvbWExDTALBgNVBAcTBFJvbWExGjAYBgNVBAoTEVQtQnVzaW5l\n" \
"c3MgUy5SLkwuMSEwHwYDVQQLExhULUJ1c2luZXNzIC0gUHlDQSBUZXN0Q0ExKjAo\n" \
"BgNVBAMTIVQtQnVzaW5lc3MgLSBTZWN1cmUgRU1haWwgVGVzdCBDQTAeFw0wNDAx\n" \
"MTYxNjExMjFaFw0wNDA4MDMxNjExMjFaMIGGMQswCQYDVQQGEwJJVDEaMBgGA1UE\n" \
"ChMRVC1CdXNpbmVzcyBzLnIubC4xGjAYBgNVBAsTEVQtQXJjaGl2ZSBwcm9qZWN0\n" \
"MRkwFwYDVQQDExBULUFyY2hpdmUgc2VydmVyMSQwIgYJKoZIhvcNAQkBFhV0YXJj\n" \
"aGl2ZUB0LWJpemNvbS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIB\n" \
"AQC+LG3wm8KEmcJ+UKrUeMFodjTyCxtV1rbn9QYW2Xi/upWrjyB91kbTXycNIUy2\n" \
"eUUx2KFLboWU83+gXE7CzBBoQNQioHEx7GXxk9mXLHFYxn91JhwFoulURtKtC4kJ\n" \
"71fjndsyrBnPGJX4KaB7ukphx+eRXcH/OVUSdSERW4oSWMlSvUStZz799quauN6G\n" \
"0PZPHM1XUOwdEkoV7kC+7yVOJYI+sYfqgIr+PGZld9KdvRKqHFqX3Efem/rFkiMS\n" \
"WMoWVZQCjaNqHruaEEcWxvkEtO9CQ38fTGtqIfzDhJQ2KBtZ+/D4wLeZNw6lIBDR\n" \
"Ksr56NnuXKXzwF7CHRbeww2JAgMBAAGjggMWMIIDEjAdBgNVHQ4EFgQUs1+gOBrX\n" \
"VXlxURpaHDak0JhOI+MwgboGA1UdIwSBsjCBr4AU5g9lqcJ/yUXAd+GCOBpnABRg\n" \
"CpWhgZOkgZAwgY0xCzAJBgNVBAYTAklUMQ0wCwYDVQQIEwRSb21hMQ0wCwYDVQQH\n" \
"EwRSb21hMRowGAYDVQQKExFULUJ1c2luZXNzIFMuUi5MLjEhMB8GA1UECxMYVC1C\n" \
"dXNpbmVzcyAtIFB5Q0EgVGVzdENBMSEwHwYDVQQDExhULUJ1c2luZXNzIC0gUm9v\n" \
"dCBUZXN0Q0GCAQEwCwYDVR0PBAQDAgXgMBMGA1UdJQQMMAoGCCsGAQUFBwMEMEUG\n" \
"A1UdEgQ+MDyGOmh0dHBzOi8vY2EudC1iaXpjb20uY29tL3B5Y2EvZ2V0LWNlcnQu\n" \
"cHkvRW1haWxDZXJ0cy9jYS5jcnQwSwYDVR0fBEQwQjBAoD6gPIY6aHR0cDovL2Nh\n" \
"LnQtYml6Y29tLmNvbS9weWNhL2dldC1jZXJ0LnB5L0VtYWlsQ2VydHMvY3JsLmNy\n" \
"bDAgBgNVHREEGTAXgRV0YXJjaGl2ZUB0LWJpemNvbS5jb20wWQYJYIZIAYb4QgEN\n" \
"BEwWSlRoaXMgY2VydGlmaWNhdGUgaXMgdXNlZCBmb3IgZS1tYWlsIGFuZCBpcyB2\n" \
"YWxpZCBvbmx5IGZvciB0ZXN0aW5nIHB1cnBvc2UuMCcGCWCGSAGG+EIBAgQaFhho\n" \
"dHRwczovL2NhLnQtYml6Y29tLmNvbS8wMgYJYIZIAYb4QgEEBCUWI3B5Y2EvZ2V0\n" \
"LWNlcnQucHkvRW1haWxDZXJ0cy9jcmwuY3JsMC8GCWCGSAGG+EIBAwQiFiBweWNh\n" \
"L25zLWNoZWNrLXJldi5weS9FbWFpbENlcnRzPzAtBglghkgBhvhCAQcEIBYecHlj\n" \
"YS9ucy1yZW5ld2FsLnB5L0VtYWlsQ2VydHM/MDEGCWCGSAGG+EIBCAQkFiJweWNh\n" \
"L3BvbGljeS9FbWFpbENlcnRzLXBvbGljeS5odG1sMBEGCWCGSAGG+EIBAQQEAwIF\n" \
"IDANBgkqhkiG9w0BAQUFAAOCAgEAiqJD89UxdANtgyx3i2Ah22jdLw2lTFoOwpvi\n" \
"sjXYS5U3r6P/HOoqUozZ82LJKpkJB+4koxPvxQHaJGM/QQJBxnQgQ6wCd68qAC+l\n" \
"067ZA7FRF3MxYn8Sfv5V08qBHhbZQgypTLzL2jlRbzpQ8ZJdv2qi7r9yXb+0LwaK\n" \
"7AreLNh52QQ/nx0tRCr3dr4D/kLJoiuVAyf//mOPFtYK+JuHfcFv/3G3VlqRCC6n\n" \
"0jbLt10VqfB5YQ789LAQA1dXhd2aykWSdGtezJWjQA2t1SDRTj7KSXHHcsw0e2gn\n" \
"BnpLI8FiNdqQdeTK5JB1M7uYDrFbzk6ckK4lg4i2cGPacu8KTG/CQr/tDx/gtBPL\n" \
"5mQzapD4WGNFrd6V41NZqJikuPV9FoCIP5/I45LR2MJQJuK46KeyyGB368Rf06UZ\n" \
"/3EmeU5cmY+0KLkN8M70LBpHSrtuVfivy/54N+6A3Or9v1ta4Xi4ScustBh857O0\n" \
"5QewB7K9zD0u/KXbJ37bEm2zPLQCMDT2hII5xiKMghaBYZU/ZyuXMEbSvU0ilSPc\n" \
"8ZHfL7DLooBA2w1ttMEwD1ps6o+qTmPwE6rHyfUa94EDs5ISSRyoucpLgOaqKzZA\n" \
"O6CREiKd2NZwo51N/otRlmC47erwrcHVksw5FI1RiPTvdQlcK9QNdl3DIfG5tygF\n" \
"WDz9OSs=\n" \
"-----END CERTIFICATE-----"

/* tarchive_key.pem */
#define KEYFILE \
"Bag Attributes\n" \
"    friendlyName: T-Archive server's T-Business S.R.L. ID\n" \
"    localKeyID: DD 77 B3 E3 F7 52 12 D8 52 12 F5 66 83 9C 8F BE AB EB 94 A5\n" \
"Key Attributes: <No Attributes>\n" \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"Proc-Type: 4,ENCRYPTED\n" \
"DEK-Info: DES-EDE3-CBC,04CFE5031CD4A8AB\n" \
"\n" \
"McxHRsUPVmQzAZTfFS7BF54EPyXpAfLd9OT1ft3otr8TbbBPvFyhdHVm1v+UkgES\n" \
"Oy0UFx88UiQg0UF/tNlapE+cDfvdzW9cd2YaESkAaahjKiLWIwlo3hxXQ9ifobvn\n" \
"qo/EMHT76krFs9u7KnW8QOo4mG4p9zouzMbLI9tWT6nzI5N4FPSV1b0IdawB6SHa\n" \
"kgvBuH7XqlCXipqlan6/QqHJldwPftEIbosmN01r5qJJo0MyxyV2BaGne+VH8wq8\n" \
"YHSh1fcsMPGqPpSicnr+equlADgkZr6wsf23c3+8HeOYIRFMb9VvITSGLvzwObMC\n" \
"u9MnjZeosjah4cwSWqQV24XZZY/br+9cz1FrYD/x2qB0e9QkV7MF3riij9FwJ6MX\n" \
"PCSPm5AeGAEKxM6Esyn/LvI5DPbuPEmI9s//KxSpWIAbUy3h8YX6mw+30Bs5ERhj\n" \
"idf2nUWqal1JGQF8pj470qcrbJ27esOh0YBNMxSi9ePRRIlbU11JhhX1Il+u+Vu3\n" \
"h4Nk+kq4m8JsAHa5uHn80dEO93n5KW81Qjsu33wgZuwALxjmsEJVYtNtvzeqHtUj\n" \
"ncieRzUWMZhL6GfplDMmv/39AQV7W4pUFooE48kGzphWk6P5t+zlp9BQSCPtBPKp\n" \
"ZSxTLiUjvV8qIgJrHu1Wqvav9kSnX3N5M15SlbKRQ9R0bIMGADiHhtgkoa50mHHu\n" \
"tBdaSeYT9ial4WcJl24dRb4AELvqGosdjps853KJChV2DsyAZS8W+Fr1rbPGxXlj\n" \
"OqQaZ/wdCTiAOCucqrHVJ5rne3erquzxf29QbMdK/AXB28o7eLMu+MHEW4GsCsLr\n" \
"r9GjP5x6zep7vT01sXnrbEuQm5i9d6V/2zmLvVDI9sKIBjNosF/SxmujtvAn3kxK\n" \
"X/o0phQ+cdSgbjHA792O+4kkT3A9Ai7O/l6LRzIFHfgFSaMwFkeESJU/aDQOSF8A\n" \
"PHCxhc8Fr4GHpBeys5x2JOb602jVQyteAWotWIMBO3ifz43Nu01kXjeliwVl73zl\n" \
"IldLKEwEvo7n9pAfOmPz1dHw83OvcwSW6npt+Jp/8vnMR0aQvgSnlphrx4klFyUQ\n" \
"uZ00CVxLYzbestwMr7stkfP4MmydJWgIOMu4RD5KbNSduSwZSMECyoGYwimDmJqe\n" \
"vnTrSbjmCVx7iZ+BUY5sBwEEMDyuihMAFrPKscQOmpNDUse+/vVeXss0rkVHBsDs\n" \
"QdfwrM8zy/SPaNQidVR7E9VyRoVgXYnoT09V+7UT/XTKokPlfk0q3JAiryL91bIH\n" \
"ejlP+1+xjLPKzNaEuitHvkrT84lw+1w7C4ZgaP1n+05ONGwtvxPtrpSFCNYYw5VC\n" \
"58vvhV2CJAXYjyLKKINyXz5Jwal9QYhb9EcPQoYrzoGKAi3+AeHvR2Q/nPIsEqC/\n" \
"5YqcFxKfym/omx6/NNp6BwYyNFxaCmr4TuPu4qKTgXyYZRda51a2zrDWpknN4pcq\n" \
"1jnGOwYpgwQB4yIxXE4N/o5sqN7+Pxc03t7db43xTU5rkAXB8nDEYY+qnVoAxjj1\n" \
"ZJ3XpSoLrjA595I06BrTK8oyTxduWh9d0ElwXftAHJl06pb8h5oqHxlrNxegszgr\n" \
"\n-----END RSA PRIVATE KEY-----"

/* tarchive_chain.pem */
#define CERTFILE \
"-----BEGIN CERTIFICATE-----\n" \
"MIIIdjCCBl6gAwIBAgIBATANBgkqhkiG9w0BAQQFADCBjTELMAkGA1UEBhMCSVQx\n" \
"DTALBgNVBAgTBFJvbWExDTALBgNVBAcTBFJvbWExGjAYBgNVBAoTEVQtQnVzaW5l\n" \
"c3MgUy5SLkwuMSEwHwYDVQQLExhULUJ1c2luZXNzIC0gUHlDQSBUZXN0Q0ExITAf\n" \
"BgNVBAMTGFQtQnVzaW5lc3MgLSBSb290IFRlc3RDQTAeFw0wMzEwMDMxNjM0Mjha\n" \
"Fw0wNzEwMDIxNjM0MjhaMIGWMQswCQYDVQQGEwJJVDENMAsGA1UECBMEUm9tYTEN\n" \
"MAsGA1UEBxMEUm9tYTEaMBgGA1UEChMRVC1CdXNpbmVzcyBTLlIuTC4xITAfBgNV\n" \
"BAsTGFQtQnVzaW5lc3MgLSBQeUNBIFRlc3RDQTEqMCgGA1UEAxMhVC1CdXNpbmVz\n" \
"cyAtIFNlY3VyZSBFTWFpbCBUZXN0IENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8A\n" \
"MIICCgKCAgEApCX5rWHn8x+wqET5PaGy8FDOiPu6+TkYPLVbj1jzibE5GmluFfWm\n" \
"fDqzqVBmq0zHJIJezKnpl8qW7UsSe8N/bDk0AExhGzeDwaHKaCAAR/mMxNpSeRcy\n" \
"oRkNMYXbg7zRnb6wCSlGg4iH5rJ2J/jKoIfR3CszDJXPmIs/ZdfOYgWG8Sn9f7Eq\n" \
"YGapMaAT+orfqEZoJw3hwitg36fcdjMVW46t1WaTglUZlOppwJl/uMGVtA9r4NRH\n" \
"V+PaQTLQe1nAfXtzXdX6vVnPIZNjuPS7dvptrTbDkMNGAejC3dAjsObTqUiykcjJ\n" \
"7ZN9+ZUSTJ8rhT82kj2VIK1Dx3ChS26xtd2yQ74cdPpulOLBJ2+iVV4H1/PcDMNk\n" \
"mG0Q/uT2A7vr6D5hBgYlgzzLByCpVQOtzU6vk6V9Fvg6wV2tmJfox6Rxu/IRoiTu\n" \
"hCxu1/xxpy3gp60z/qBBDh8iidSO2hnH8jaLrazV7YubDT5bix0RDAGJIR+w0nZx\n" \
"VmEbvt8yY1GUxWVXDMd0HNhNOCkkBCJ4XStinPnIERMe9jxX3wXzJJLGadIyuGLs\n" \
"TbDGfzfmXYD6Eo80MvTzlbciefG7LdcJWv3qWhxYnAGSvrpSBBgmWHRlfj0B7t9X\n" \
"gCQbDeK/HTYXMWSYEjImOkZ7itQspjqzjwRXrZoEMju2KjsDymBhatUCAwEAAaOC\n" \
"AtQwggLQMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFOYPZanCf8lFwHfhgjga\n" \
"ZwAUYAqVMIG6BgNVHSMEgbIwga+AFClzQLGh1PpwvsRcRYiw//8xQQeMoYGTpIGQ\n" \
"MIGNMQswCQYDVQQGEwJJVDENMAsGA1UECBMEUm9tYTENMAsGA1UEBxMEUm9tYTEa\n" \
"MBgGA1UEChMRVC1CdXNpbmVzcyBTLlIuTC4xITAfBgNVBAsTGFQtQnVzaW5lc3Mg\n" \
"LSBQeUNBIFRlc3RDQTEhMB8GA1UEAxMYVC1CdXNpbmVzcyAtIFJvb3QgVGVzdENB\n" \
"ggEAMA4GA1UdDwEB/wQEAwIB5jATBgNVHSUEDDAKBggrBgEFBQcDBDBEBgNVHREE\n" \
"PTA7hjlodHRwOi8vY2EudC1iaXpjb20uY29tL3B5Y2EvZ2V0LWNlcnQucHkvRW1h\n" \
"aWxDZXJ0cy9jYS5jcnQwPgYDVR0SBDcwNYYzaHR0cDovL2NhLnQtYml6Y29tLmNv\n" \
"bS9weWNhL2dldC1jZXJ0LnB5L1Jvb3QvY2EuY3J0MEUGA1UdHwQ+MDwwOqA4oDaG\n" \
"NGh0dHA6Ly9jYS50LWJpemNvbS5jb20vcHljYS9nZXQtY2VydC5weS9Sb290L2Ny\n" \
"bC5jcmwwWAYJYIZIAYb4QgENBEsWSVRoaXMgQ0EgaXNzdWVzIGUtbWFpbCBjZXJ0\n" \
"aWZpY2F0ZXMgYW5kIGlzIHZhbGlkIG9ubHkgZm9yIHRlc3RpbmcgcHVycG9zZS4w\n" \
"PQYJYIZIAYb4QgEIBDAWLmh0dHBzOi8vY2EudC1iaXpjb20uY29tL0VtYWlsQ2Vy\n" \
"dHMvcG9saWN5Lmh0bWwwEQYJYIZIAYb4QgEBBAQDAgECMEMGCWCGSAGG+EIBBAQ2\n" \
"FjRodHRwOi8vY2EudC1iaXpjb20uY29tL3B5Y2EvZ2V0LWNlcnQucHkvUm9vdC9j\n" \
"cmwuY3JsMA0GCSqGSIb3DQEBBAUAA4ICAQBT4wHqv8Q4GepyDx/f4rMo4GGParXq\n" \
"Vx6t3w6N1RA4nugPruTzpZW/pqZTL8aILvYABj3sruzTnntZvYsSarJudr5eKbPb\n" \
"5misgry8pGLOzW3oM1u4bEkl/pF8Z3/4L24Al/IbeqgpUGX6F6P/XRIq/Et6iVZb\n" \
"cRrAr0fJ+mwl1JICP/SP29vKYwWpTk+gzaVvkNZdv44PZ/s8a7Rsm7K8nAdfqVAi\n" \
"X65Wka5v0TpJ7ZKBZizf0Psp7om9EXsHAl5M4gMAFFGpjws3rLBtQwq5JaBTFwg6\n" \
"hFSiufqdYOw8g9QNw7KGKxTcT+/Fz7XwQg9Yht3NnbseUPDHQ4GaVO+jZQGDLuDU\n" \
"oKGDZ0qS6L+uYO/HxjSSlEkds+4byqeddixU/qwv8daM7H9PTz3rBFKWhp4nxycy\n" \
"unhMBiJWNp7rngDplnhet/loQkm81Sp2w7KPH27ZyoSXPOUsb2ywTRRslRqfGzOj\n" \
"MgZALiHdZ6FQ1w3Pthn8djf85E8wVAml7s/ycw6jovuZdB3+QIwXAmthybgvA5Fb\n" \
"LNPPuS5R9ntzV/U1xOiwEOh4iM45GhycQ32HGYel//HUjYI3DlpvlTHaMu99Y5xj\n" \
"2LiX7beTZuMXIxqlSKvYiXNWnddorvNGMxNM3LaC6VLMjedd16nJnWhvfUgY2BmB\n" \
"+Y60Lq2cquqMjg==\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIII7TCCBtWgAwIBAgIBADANBgkqhkiG9w0BAQQFADCBjTELMAkGA1UEBhMCSVQx\n" \
"DTALBgNVBAgTBFJvbWExDTALBgNVBAcTBFJvbWExGjAYBgNVBAoTEVQtQnVzaW5l\n" \
"c3MgUy5SLkwuMSEwHwYDVQQLExhULUJ1c2luZXNzIC0gUHlDQSBUZXN0Q0ExITAf\n" \
"BgNVBAMTGFQtQnVzaW5lc3MgLSBSb290IFRlc3RDQTAeFw0wMzEwMDMxNjMyMTFa\n" \
"Fw0xMTEwMDIxNjMyMTFaMIGNMQswCQYDVQQGEwJJVDENMAsGA1UECBMEUm9tYTEN\n" \
"MAsGA1UEBxMEUm9tYTEaMBgGA1UEChMRVC1CdXNpbmVzcyBTLlIuTC4xITAfBgNV\n" \
"BAsTGFQtQnVzaW5lc3MgLSBQeUNBIFRlc3RDQTEhMB8GA1UEAxMYVC1CdXNpbmVz\n" \
"cyAtIFJvb3QgVGVzdENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA\n" \
"26igcJ4NWyz4OoClQTuh7NbRgdXIckyXpihjMCcroxRGTWvlRY/gmB0RLr65kJWg\n" \
"VMZ2R1//FOeXdQQ0APBFgMAPEg1n/c1YkcFN2X7GSlfKJojM2yvyLxxmq9D8RY4U\n" \
"x9kzjkNcUxLr8Y6dkMHOGRWPN8JoyDs2vWhGO0HW7QDrdHpgXHHzphzZVn5/CWp5\n" \
"7jQQUFxi07/ERhllvdnz+t2vv96RBIp3rkloBq5y/QreVGAAnwF1g17OlnliWcZ9\n" \
"vOVf6Pr6ohOEKOrNEOLYP6HvUrmkI+dka/tlt5w3pwTl66LSKkPYmvNomZsTcRLP\n" \
"bdgb0pf7lEjubvvPay3PweD3B/T+lHhWNA2+wWeXM2qjCM3AelOV4U/Mv+pmOHes\n" \
"ZZLncMFoYZy0h8dxMIm+mRZiI9fc8x7teTq4CpWmvsClzj+FAtMpxftH0EDn9GZk\n" \
"ox8P4a3mOnGtFs9++sSBpuP2xJwJ1GvbeaFWnYGqHGF70KpTF4wWhaVZq7pUmyut\n" \
"e2SxROFcsNHErD15SqdQ3WKh5OXss0GG5uy9IeHUz0s8M4djGUk2Ax9FoEedzbZN\n" \
"o9jLk7glyVWT2yllTr8nZjzmL2LAAiNG3nvdIxiHdczYw++eTYnI1dxBLQAcyI2Y\n" \
"9kwHT067HsYOzalanXslivfkM5x5nyDuBwbmH7DfrfcCAwEAAaOCA1QwggNQMBEG\n" \
"CWCGSAGG+EIBAQQEAwIABzCBkwYJYIZIAYb4QgENBIGFFoGCVGhpcyBSb290IENB\n" \
"IGlzc3VlcyBzdWItQ0EgY2VydHMgb2YgZGlmZmVyZW50IHBvbGljaWVzIGFuZCBo\n" \
"YXMgbm8gY29udGFjdCB3aXRoIGVuZC1lbnRpdGllcyBhbmQgaXMgdmFsaWQgb25s\n" \
"eSBmb3IgdGVzdGluZyBwdXJwb3NlLjA3BglghkgBhvhCAQgEKhYoaHR0cHM6Ly9j\n" \
"YS50LWJpemNvbS5jb20vUm9vdC9wb2xpY3kuaHRtbDBDBglghkgBhvhCAQQENhY0\n" \
"aHR0cDovL2NhLnQtYml6Y29tLmNvbS9weWNhL2dldC1jZXJ0LnB5L1Jvb3QvY3Js\n" \
"LmNybDAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBQpc0CxodT6cL7EXEWIsP//\n" \
"MUEHjDCBugYDVR0jBIGyMIGvgBQpc0CxodT6cL7EXEWIsP//MUEHjKGBk6SBkDCB\n" \
"jTELMAkGA1UEBhMCSVQxDTALBgNVBAgTBFJvbWExDTALBgNVBAcTBFJvbWExGjAY\n" \
"BgNVBAoTEVQtQnVzaW5lc3MgUy5SLkwuMSEwHwYDVQQLExhULUJ1c2luZXNzIC0g\n" \
"UHlDQSBUZXN0Q0ExITAfBgNVBAMTGFQtQnVzaW5lc3MgLSBSb290IFRlc3RDQYIB\n" \
"ADCBogYDVR0lBIGaMIGXBggrBgEFBQcDAQYIKwYBBQUHAwIGCCsGAQUFBwMDBggr\n" \
"BgEFBQcDBAYIKwYBBQUHAwgGCisGAQQBgjcKAwEGCCsGAQUFBwMFBggrBgEFBQcD\n" \
"BgYIKwYBBQUHAwcGCisGAQQBgjcKAwQGCisGAQQBgjcCARUGCisGAQQBgjcCARYG\n" \
"CisGAQQBgjcKAwMGCWCGSAGG+EIEATAOBgNVHQ8BAf8EBAMCAfYwPgYDVR0RBDcw\n" \
"NYYzaHR0cDovL2NhLnQtYml6Y29tLmNvbS9weWNhL2dldC1jZXJ0LnB5L1Jvb3Qv\n" \
"Y2EuY3J0MEUGA1UdHwQ+MDwwOqA4oDaGNGh0dHA6Ly9jYS50LWJpemNvbS5jb20v\n" \
"cHljYS9nZXQtY2VydC5weS9Sb290L2NybC5jcmwwDQYJKoZIhvcNAQEEBQADggIB\n" \
"AMtZglWCSvO9mRFIRKSKH+GEs2YltFryyerOGyTfGP19AEyuImSR9GNMwwmo6Cjc\n" \
"E9No0/eKUFPwvrmKl7VyYWsVTSpzKPc5ttBswfHeBYkXz4vbp1HySLf9YNPKEpWH\n" \
"B6gkFHmj3IgZ9lq3ZHhXFFeyl0dkSQCpvuLVheggBX7i5wY+fB9+ZvOr979ELrhl\n" \
"L1NYmoHBdkwJEkHxZHhq80uTrcr3/FtxeYrLx6T+U/AKeKQEt2jlhJCHPVCxOMpG\n" \
"eesbf9LCa2frMqPUtT6gGvCeYrQzakvBjYvWjHYaHUJvCm6t2CI9bYex9KpGOlIB\n" \
"MaBuekUQicJdk6pDwpLQltR6D4VcJl4xXnYHrGqDaRhAVrqIuo2kYGcXyuPm/ruc\n" \
"C+EwOu7lRPU4P3aohlv/paLXjHhgZz4VkjVLg7/78ZkXYa6bQqSA87r4XyZRA7ye\n" \
"DSyLKDMHxBzF81tr4nnPo6gl00KpsMEz5oGVsn2pUU4DbSFHcSFFsbKIErQyaVo+\n" \
"oPzRIesm2NA+sEzLtJ9B9WXPPZ99ClpdBcm67XDEayBtkVfaSfO1/iIawjvCn/64\n" \
"8G0JtrSqXtJRYwP/gQanILXQD9qfFPJNQHlxb8B+9Je/c9HT9Jgpxd/97+XnvtRY\n" \
"/N9P0DRQytn+P31V3J9qTmK8Pz/FUMdmgPMER7SESM/A\n" \
"-----END CERTIFICATE-----"

/* infile.txt */
#define DATA \
"# Do not remove the following line, or various programs\n" \
"# that require network functionality will fail.\n" \
"127.0.0.1     localhost.localdomain localhost"

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UFile filex;
   UString filename;

   UServices::setupOpenSSLStore(0, argv[1]);

   while (cin >> filename &&
          filex.open(filename))
      {
      check(filex.getContent());

      filex.getPath().clear();
      }

   UString data       = U_STRING_FROM_CONSTANT(DATA),
           signerfile = U_STRING_FROM_CONSTANT(SIGNERFILE),
           keyfile    = U_STRING_FROM_CONSTANT(KEYFILE),
           passin     = U_STRING_FROM_CONSTANT("caciucco"),
           certfile   = U_STRING_FROM_CONSTANT(CERTFILE);

   PKCS7* p7 = UPKCS7::sign(data, signerfile, keyfile, passin, certfile);

   STACK_OF(X509)* certs = UCertificate::loadCerts(U_STRING_FROM_CONSTANT(CERTFILE));

   int n = U_SYSCALL(sk_X509_num, "%p", certs);

   U_INTERNAL_ASSERT_EQUALS(n, 2)

   STACK_OF(X509)* certs1 = p7->d.sign->cert;

   n = U_SYSCALL(sk_X509_num, "%p", certs1);

   U_INTERNAL_ASSERT_EQUALS(n, 3)

   UCertificate c(U_STRING_FROM_CONSTANT(SIGNERFILE));

   X509* x  = c.getX509();;
   X509* x1 = sk_X509_value(certs1, 0);

   U_ASSERT_EQUALS( X509_cmp(x, x1), 0 )

   x  = sk_X509_value(certs,  0);
   x1 = sk_X509_value(certs1, 1);

   U_ASSERT_EQUALS( X509_cmp(x, x1), 0 )

   x  = sk_X509_value(certs,  1);
   x1 = sk_X509_value(certs1, 2);

   U_ASSERT_EQUALS( X509_cmp(x, x1), 0 )

   cout << UPKCS7::writeMIME(p7);
}
