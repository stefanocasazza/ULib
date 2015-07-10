// main.cpp

   X509* cert;
   X509_CRL* crl;
   bool ok, is_revoked;
   UDialog x(0, 23, 50); // height of 23 characters and width of 50 characters
   UVector<UString> vec;
   STACK_OF(X509)* certs;
   unsigned j, num_revoked = 0;
   long lserial, revoked[4096];
   int op, i, n, days = 0, compress = 0;
   UString ca, result, list, cnf, policy, serial, pkcs, tmp(300U), subject;

   UApplication::exit_value = 1;

   if (client->connect() == false) goto end;

   if (method) op = atoi(method);
   else
      {
   // if (UDialog::isXdialog() == false) U_ERROR("num_method not specified and I don't find Xdialog");

      static const char* items[] = { "CA creation",
                                     "CA list",
                                     "Emit certificate (with PKCS10 request)",
                                     "Emit certificate (with SPKAC  request)",
                                     "CA certificates list",
                                     "Remove certificate",
                                     "Physically remove all CA certificates",
                                     "Emit crl",
                                     "Get crl",
                                     "Get CA certificate",
                                     "Revoke certificate", 0 };

      static const char* tags[]  = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", 0 };

      op = x.menu("Choose the operation you want:", tags, items, 0, "MENU BOX");

      if (op == -1) goto end;
      if (op !=  0) list = client->listCA(); // get CA list
      if (op >=  1)
         {
         if (op == 1) // CA list
            {
            result = list;

            goto end;
            }

         if (vec.split(list, '\n') == 0) U_ERROR("first you need to create some CA");

         x.setSize(20, 50); // height of 10 characters and width of 50 characters

         ok = x.menu("CA list", vec, ca, 0); 

         // NB: we need this reset...

          vec.clear();
         list.clear();

         client->clearData();

         if (ok == false) goto end; 

         U_INTERNAL_DUMP("ca = %V", ca.rep)
         }

      ++op;
      }

   U_INTERNAL_DUMP("op = %d", op)

   switch (op)
      {
      case 1: // CA creation
         {
         // parameters: <CA name> <days> [openssl.cnf]
         // --------------------------------------------------------------------------------
         // int ns__CSP_CA(const char* dir, unsigned days, const char* cnf, char** response);

         if (method)
            {
            if (argv[  optind])   ca = UString(argv[optind]);
            if (argv[++optind]) days =    atoi(argv[optind]);
            if (argv[++optind])  cnf = UString(argv[optind]);
            }
         else
            {
            x.setSize(10, 70); // height of 10 characters and width of 70 characters

            static const char* labels[] = { "name CA:", "certificate validity period (number of days):", 0 };

            vec.push(ca);
            vec.push(U_STRING_FROM_CONSTANT("365"));

            if (x.inputsbox2("CA creation", labels, vec, 0) == false) goto end;

            ca   = vec[0];
            days = vec[1].strtol();

            x.setSize(10, 70);

            if (x.yesno("Do you want to select a openssl template configuration file ?"))
               {
               x.setSize(60, 80);

               (void) x.fselect(cnf, "Please choose a configuration file");
               }
            }

         if (days <= 0) U_ERROR("certificate validity period not valid");

         if (cnf) cnf = UFile::contentOf(cnf);

         if (client->creatCA(ca, days, cnf)) UApplication::exit_value = 0;

      // std::cout << "CSP_CA() = " << (client->creatCA(ca, days, cnf) ? 1 : 0) << "\n";
         }
      break;

      case 2: // CA list
         {
         // no parameters
         // --------------------------------------------------------------------------------
         // int ns__CSP_LIST_CA(char** response);

         result = client->listCA();

      // std::cout << "CSP_LIST_CA() = " << client->listCA() << "\n";
         }
      break;

      case 3: // Emit certificate (with PKCS10 request)
      case 4: // Emit certificate (with  SPKAC request)
         {
         // parameters: <CA name> <PKCS10-request>.pem [policy]
         // parameters: <CA name>  <SPKAC-request>.pem [policy]
         // --------------------------------------------------------------------------------
         // int ns__CSP_SIGN_P10(  const char* ca, const char* pkcs10, const char* policy, char** response);
         // int ns__CSP_SIGN_SPACK(const char* ca, const char*  spack, const char* policy, char** response);

         if (method)
            {
            if (argv[  optind])     ca = UString(argv[optind]);
            if (argv[++optind])   pkcs = UString(argv[optind]);
            if (argv[++optind]) policy = UString(argv[optind]);
            }
         else
            {
            x.setSize(60, 80);

            if (x.fselect(pkcs, "Please choose a certificate request file") == false) goto end;

            x.setSize(10, 70);

            if (x.yesno("Do you want to select a policy ?"))
               {
               x.setSize(10, 80);

               (void) x.inputbox("Please enter the name of the policy:", policy);
               }
            }

         pkcs = UFile::contentOf(pkcs);

         if (pkcs.empty()) U_ERROR("missing certificate request");

         result = (op == 3 ? client->signP10(  ca, pkcs, policy)
                           : client->signSPKAC(ca, pkcs, policy));

      // std::cout << "CSP_SIGN_P10()   = " << client->signP10(  ca, pkcs10, policy) << "\n";
      // std::cout << "CSP_SIGN_SPACK() = " << client->signSPKAC(ca,  spack, policy) << "\n";
         }
      break;

      case 5: // CA certificates list
         {
         // parameter: <CA name> [compress]
         // --------------------------------------------------------------------------------
         // int ns__CSP_LIST_CERTS(const char* ca, unsigned compress, char** response);

         if (method)
            {
            if (argv[  optind])       ca = UString(argv[optind]);
            if (argv[++optind]) compress =    atoi(argv[optind]);
            }
         else
            {
            x.setSize(10, 70);

            compress = x.yesno("Do you want the CA certificates list be compressed ?");
            }

         if (ca.empty()) U_ERROR("missing CA name");

         result = client->listCerts(ca, compress);

      // std::cout << "CSP_LIST_CERTS() = " << client->listCerts(ca, compress) << "\n";
         }
      break;

      case  6: // Remove certificate
      case 11: // Revoke certificate
         {
         // parameters: <CA name> <cert-serial (number value)>
         // --------------------------------------------------------------------------------
         // int ns__CSP_REMOVE_CERT(const char* ca, const char* serial, char** response);

         if (method)
            {
            if (argv[  optind])    ca  = UString(argv[optind]);
            if (argv[++optind]) serial = UCertificate::checkForSerialNumber(argv[optind]);
            }
         else
            {
            list  = client->listCerts(ca, 0);
            certs = UCertificate::loadCerts(list);
            n     = U_SYSCALL(sk_X509_num, "%p", certs);

            if (n == 0) U_ERROR("empty CA certificates list");

            // NB: we need this reset...

            list.clear();

            if (op == 11 && // Revoke certificate
                (client->clearData(), client->emitCRL(ca)))
               {
               client->clearData();

               crl         = UCrl::readCRL(client->getCRL(ca), "PEM");
               num_revoked = UCrl::getRevokedSerials(crl, revoked, 4096);
               }

            for (i = 0; i < n; ++i)
               {
               cert       = sk_X509_value(certs, i);
               lserial    = UCertificate::getSerialNumber(cert);
               is_revoked = false;

               for (j = 0; j < num_revoked; ++j)
                  {
                  if (revoked[j] == lserial)
                     {
                     is_revoked = true;

                     break;
                     }
                  }

               if (is_revoked == false)
                  {
                  subject = UCertificate::getSubject(cert);

                  tmp.snprintf("0x%04X %v", lserial, subject.rep);

                  vec.push(tmp.copy());
                  }
               }

            U_SYSCALL_VOID(sk_X509_free, "%p", certs);

            x.setSize(22, 120); // height of 22 characters and width of 120 characters

            if (vec.size() > 1) vec.sort();

            ok = x.menu("CA certificates list", vec, serial, 0);

            // NB: we need this reset...

            client->clearData();

            if (ok == false) goto end;

            serial = UCertificate::checkForSerialNumber(serial.data());

            U_INTERNAL_DUMP("serial = %V", serial.rep)
            }

         if (op == 6 ? client->removeCert(ca, serial)
                     : client->revokeCert(ca, serial)) UApplication::exit_value = 0;

      // std::cout << "CSP_REMOVE_CERT() = " << (client->removeCert(ca, serial) ? 1 : 0) << "\n";
      // std::cout << "CSP_REVOKE_CERT() = " << (client->revokeCert(ca, serial) ? 1 : 0) << "\n";
         }
      break;

      case 7: // Physically remove all CA certificates
         {
         // parameter: <CA name>
         // --------------------------------------------------------------------------------
         // int ns__CSP_ZERO_CERTS(const char* ca, char** response);

         if (method)
            {
            if (argv[optind]) ca = UString(argv[optind]);
            }

         if (ca.empty()) U_ERROR("missing CA name");

         if (client->zeroCerts(ca)) UApplication::exit_value = 0;

      // std::cout << "CSP_ZERO_CERTS() = " << (client->zeroCerts(ca) ? 1 : 0) << "\n";
         }
      break;

      case 8: // Emit crl
         {
         // parameter: <CA name>
         // --------------------------------------------------------------------------------

         if (method)
            {
            if (argv[optind]) ca = UString(argv[optind]);
            }

         if (ca.empty()) U_ERROR("missing CA name");

         if (client->emitCRL(ca)) UApplication::exit_value = 0;

      // std::cout << "CSP_EMIT_CRL() = " << (client->emitCRL(ca) ? 1 : 0) << "\n";
         }
      break;

      case 9: // Get crl
         {
         // parameter: <CA name>
         // --------------------------------------------------------------------------------

         if (method)
            {
            if (argv[optind]) ca = UString(argv[optind]);
            }

         if (ca.empty()) U_ERROR("missing CA name");

         result = client->getCRL(ca);

      // std::cout << "CSP_GET_CRL() = " << client->getCRL(ca) << "\n";
         }
      break;

      case 10: // Get CA certificate
         {
         // parameter: <CA name>
         // --------------------------------------------------------------------------------

         if (method)
            {
            if (argv[optind]) ca = UString(argv[optind]);
            }

         if (ca.empty()) U_ERROR("missing CA name");

         result = client->getCA(ca);

      // std::cout << "CSP_GET_CA() = " << client->getCA(ca) << "\n";
         }
      break;

      default:
         U_ERROR("num_method not valid");
      break;
      }

end:
   if (result)
      {
      (void) write(1, U_STRING_TO_PARAM(result));
      (void) write(1, U_CONSTANT_TO_PARAM("\n"));

      UApplication::exit_value = 0;
      }

   if (UApplication::exit_value == 1)
      {
      result = client->getResponse();

      if (result) U_WARNING("%v", result.rep);
      }

   client->closeLog();
