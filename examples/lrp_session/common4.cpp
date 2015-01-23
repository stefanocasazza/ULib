// common4.cpp

   // get devices attribute from LDAP

   ldap.get(device_entry);

   // loop for every device

   for (k = 0; k < j; ++k)
      {
      ip_device = device_entry.getCStr(DEVICE_ATTR_IPHOST_POS, k);

      U_INTERNAL_DUMP("DEVICE = %S %S", device_entry[k], ip_device)

      keypub  = device_entry.getCStr(DEVICE_ATTR_KEYPUB_POS,  k);
      keypriv = device_entry.getCStr(DEVICE_ATTR_KEYPRIV_POS, k);

      if (proc.fork() &&
          proc.child())
         {
         // send request to device

         sock.setKey(keypriv, keypub);

         response_size      = 0;
         const char* reason = "";

         if (sock.connectServer(UString(ip_device), 22) && sock.send(U_STRING_TO_PARAM(request)))
            {
            sock.sendEOF();

            response_size = sock.recv(response_buffer, sizeof(response_buffer));
            }

         if (response_size == 0)
            {
            esito  = false;
            reason = sock.getError();
            }
         else
            {
            /*
            <RESPONSE sid="sid1" version="1">
            <STATUS code="ok">IMPORT_POLICYLABEL name: webservices-amount : Executed without error</STATUS></RESPONSE>
            <RESPONSE sid="sid2" version="1">
            <STATUS code="error">EXECUTE_POLICYLABEL name: webservices-amount command: drop : Action error</STATUS>
            </RESPONSE>
            */

            const char* match_ptr =        "<STATUS code=\"error\">";
            unsigned    match_len = sizeof("<STATUS code=\"error\">")-1;

            const char* ptr = (const char*) u_find(response_buffer, response_size, match_ptr, match_len);

            if (ptr == 0) esito = true;
            else
               {
               esito = false;
               ptr  += match_len;

               static char errbuf[256];

               int n = U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p", ptr,
                     "%*[^:]" ": " // EXECUTE_POLICYLABEL name
                     "%*[^:]" ": " // webservices-amount command
                     "%*[^:]" ": " // drop
                     "%[^<]"       // Action error
                     "</STATUS>", &errbuf[0]);

               if (n == 1) reason = errbuf;
               }
            }

         sem.lock(); // timestamp shared...

         log.log("%s|%s|%s|%s|%b|%s|\n", ip_device, policy, filtro, operation, esito, reason);

         // write response to file

         if (response_size)
            {
            name.snprintf("%s/%s_%s_%s_%#4D.res", directory.c_str(), filtro, policy, ip_device, tm_session);

            (void) UFile::writeTo(name, UString(response_buffer, response_size));
            }

         sem.unlock();

         U_EXIT(esito ? 0 : 1);
         }

      // parent
      }
