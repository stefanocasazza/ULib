// common2.cpp

   // init log

   UProcess proc;
   UString name(U_CAPACITY), tmp(U_CAPACITY);

   tmp.snprintf(U_CONSTANT_TO_PARAM("%v/%s.log"), directory.rep, log_name);

   ULog::fmt = "%P|%4D|%s  %N\n";

   ULog log(tmp, 1024 * 1024);

   if (log.isOpen())
      {
      log.setShared(0, 0);

      log.init("%P|%4D|");
      }

   time_t tm_session = u_now->tv_sec;

   bool esito;
   USemaphore sem;
   UString request;
   const char* keypub;
   const char* keypriv;
   char response_buffer[4096 * 4];
   unsigned request_size, response_size;

   bool bIPv6 = false;
   USSHSocket sock(bIPv6);

   sock.setVerbosity(); // no msg on stderr...
   sock.setUser(username.c_str());
