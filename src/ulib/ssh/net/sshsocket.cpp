// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    sshsocket.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/ssh/net/sshsocket.h>

void USSHSocket::close()
{
   U_TRACE_NO_PARAM(1, "USSHSocket::close()")

   if (channel)
      {
      U_SYSCALL_VOID(channel_free, "%p", channel); // closes and destroy the channel

      channel = 0;
      }

   if (session)
      {
      U_SYSCALL_VOID(ssh_disconnect, "%p", session); // sends a polite disconnect message, and does clean the session

      session = 0;
      }

   USocket::iSockDesc = -1;
   USocket::iState    = CLOSE;

   U_socket_LocalSet(this) = false;
}

U_NO_EXPORT void USSHSocket::setStatus()
{
   U_TRACE_NO_PARAM(1, "USSHSocket::setStatus()")

   int val = 0;
   const char* descr = "SSH_NO_ERROR";
   const char* errstr = "ok";

   if (ret != SSH_NO_ERROR)
      {
      errstr = U_SYSCALL(ssh_get_error,      "%p", session);
      val    = U_SYSCALL(ssh_get_error_code, "%p", session);

      switch (val)
         {
         case SSH_REQUEST_DENIED:   descr  = "SSH_REQUEST_DENIED";   break;
         case SSH_FATAL:            descr  = "SSH_FATAL";            break;
         case SSH_EINTR:            descr  = "SSH_EINTR";            break;
      // case SSH_INVALID_REQUEST:  descr  = "SSH_INVALID_REQUEST";  break;
      // case SSH_CONNECTION_LOST:  descr  = "SSH_CONNECTION_LOST";  break;
      // case SSH_INVALID_DATA:     descr  = "SSH_INVALID_DATA";     break;
         default:                   descr  = "???";                  break;
         }
      }
   else if (auth != SSH_AUTH_SUCCESS) // some serious error happened during authentication
      {
      switch ((val = auth))
         {
         case SSH_AUTH_DENIED:
            {
            descr  = "SSH_AUTH_DENIED";
            errstr = "no key matched";
            }
         break;

         case SSH_AUTH_PARTIAL:
            {
            descr  = "SSH_AUTH_PARTIAL";
            errstr = "some key matched but you still have to give an other mean of authentication (like password)";
            }
         break;

         case SSH_AUTH_INFO:
            {
            descr  = "SSH_AUTH_INFO";
            errstr = "the server has sent a few questions to ask your user";
            }
         break;
         }
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("(%d, %s) - %s"), val, descr, errstr);
}

bool USSHSocket::SSHConnection(int fd)
{
   U_TRACE(1, "USSHSocket::SSHConnection(%d)", fd)

   ret = auth = 0;

   if (fd != -1) (void) U_SYSCALL(ssh_options_set, "%p,%d,%p", session, SSH_OPTIONS_FD, &fd); // get SSH to use our socket

   // get SSH to handshake with server

   if (U_SYSCALL(ssh_connect, "%p", session) == SSH_NO_ERROR)
      {
      // Checks the user's known host file to look for a previous connection to the specified server

      int state = U_SYSCALL(ssh_is_server_known, "%p", session);

      /*
      #define SSH_SERVER_ERROR        -1
      #define SSH_SERVER_NOT_KNOWN     0
      #define SSH_SERVER_KNOWN_OK      1
      #define SSH_SERVER_KNOWN_CHANGED 2
      #define SSH_SERVER_FOUND_OTHER   3
      */

      U_INTERNAL_DUMP("state = %d", state)

      switch (state)
         {
         case SSH_SERVER_KNOWN_OK: break; // ok

         case SSH_SERVER_NOT_KNOWN:
            {
            // The server is unknown. you trust the host key
            // This new key will be written on disk for further usage

            ret = U_SYSCALL(ssh_write_knownhost, "%p", session);

            if (ret == SSH_NO_ERROR) break; // ok
            }

         case SSH_SERVER_ERROR:
         case SSH_SERVER_KNOWN_CHANGED: // Host key for server changed: for security reason, connection will be stopped
         case SSH_SERVER_FOUND_OTHER:   // The host key for this server was not found but an other type of key exists
                                        // An attacker might change the default server key to confuse your client into
                                        // thinking the key does not exist
         goto end;
         }

      // Authenticating to server

      if (public_key == 0)
         {
         auth = U_SYSCALL(ssh_userauth_autopubkey, "%p,%S", session, 0);

         if (auth != SSH_AUTH_SUCCESS) goto end;
         }
      else
         {
         U_INTERNAL_ASSERT_POINTER(public_key)
         U_INTERNAL_ASSERT_POINTER(private_key)

         if (public_key[1] == '%')
            {
            static char buf1[U_PATH_MAX];

            (void) u__snprintf(buf1, sizeof(buf1), public_key, strlen(public_key), 0);

            public_key = buf1;
            }

         int type;
         ssh_string pubkey = (ssh_string) U_SYSCALL(publickey_from_file, "%p,%S,%p", session, (char*)public_key, &type);

         if (pubkey == 0) goto end;

         auth = U_SYSCALL(ssh_userauth_offer_pubkey, "%p.%S.%d.%p", session, 0, type, pubkey);

         if (auth != SSH_AUTH_SUCCESS) goto end;

         if (private_key[1] == '%')
            {
            static char buf2[U_PATH_MAX];

            (void) u__snprintf(buf2, sizeof(buf2), private_key, strlen(private_key), 0);

            private_key = buf2;
            }

         ssh_private_key privkey = (ssh_private_key) U_SYSCALL(privatekey_from_file, "%p,%S,%d,%S", session, (char*)private_key, type, 0);

         if (privkey == 0) goto end;

         auth = U_SYSCALL(ssh_userauth_pubkey, "%p.%S.%d.%p", session, 0, pubkey, privkey);

         if (auth != SSH_AUTH_SUCCESS) goto end;
         }

      channel = (ssh_channel) U_SYSCALL(channel_new, "%p", session);

      if (U_SYSCALL(channel_open_session, "%p", channel) == SSH_NO_ERROR)
         {
         ret = U_SYSCALL(channel_request_shell, "%p", channel);

         if (ret == SSH_NO_ERROR)
            {
            USocket::iState    = CONNECT;
            USocket::iSockDesc = U_SYSCALL(ssh_get_fd, "%p", session);

            U_RETURN(true);
            }
         }
      }

end:
#ifdef DEBUG
   setStatus();

   U_INTERNAL_DUMP("status = %.*S", u_buffer_len, u_buffer)

   u_buffer_len = 0;
#endif

   U_RETURN(false);
}

bool USSHSocket::setError()
{
   U_TRACE_NO_PARAM(0, "USSHSocket::setError()")

   if (USocket::isSysError())
      {
      errno = - USocket::iState;

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%R"), 0); // NB: the last argument (0) is necessary...

      U_RETURN(true);
      }

   if (ret  != SSH_NO_ERROR ||
       auth != SSH_AUTH_SUCCESS) // some serious error happened during authentication
      {
      setStatus();

      U_RETURN(true);
      }

   U_RETURN(false);
}

// VIRTUAL METHOD

bool USSHSocket::connectServer(const UString& server, unsigned int iServPort, int timeoutMS) // 22
{
   U_TRACE(1, "USSHSocket::connectServer(%V,%u,%d)", server.rep, iServPort, timeoutMS)

#if LIBSSH_VERSION_INT < 1280 // (0.5.0)
   if (USocket::connectServer(server, iServPort, timeoutMS))
      {
#else
   if (USocket::cRemoteAddress.setHostName(server, U_socket_IPv6(this)))
      {
      USocket::iRemotePort = iServPort;
#endif
      const char* srv = server.c_str();

      (void) U_SYSCALL(ssh_options_set, "%p,%d,%S", session, SSH_OPTIONS_HOST, srv);
      (void) U_SYSCALL(ssh_options_set, "%p,%d,%p", session, SSH_OPTIONS_PORT, &iServPort);

      if (SSHConnection(USocket::iSockDesc)) U_RETURN(true);
      }

   U_RETURN(false);
}

int USSHSocket::recv(void* pBuffer, uint32_t iBufferLen)
{
   U_TRACE(1, "USSHSocket::recv(%p,%u)", pBuffer, iBufferLen)

   U_INTERNAL_ASSERT_POINTER(channel)
   U_INTERNAL_ASSERT(USocket::isConnected())

   if (buffer == 0) buffer = (ssh_buffer) U_SYSCALL_NO_PARAM(buffer_new);

   int iBytesRead = U_SYSCALL(channel_read_buffer, "%p,%p,%d,%d", channel, buffer, iBufferLen, 0);

   if (iBytesRead > 0) U_MEMCPY(pBuffer, buffer_get(buffer), iBytesRead);

#ifdef DEBUG
        if (iBytesRead > 0) U_INTERNAL_DUMP("BytesRead(%d) = %#.*S", iBytesRead, iBytesRead, pBuffer)
   else if (iBytesRead < 0)
      {
      setStatus(iBytesRead);

      U_INTERNAL_DUMP("status = %.*S", u_buffer_len, u_buffer)

      u_buffer_len = 0;
      }
#endif

   U_RETURN(iBytesRead);
}

int USSHSocket::send(const char* pData, uint32_t iDataLen)
{
   U_TRACE(1, "USSHSocket::send(%p,%u)", pData, iDataLen)

   U_INTERNAL_ASSERT_POINTER(channel)
   U_INTERNAL_ASSERT(USocket::isConnected())

   int iBytesWrite = U_SYSCALL(channel_write, "%p,%p,%d", channel, (void*)pData, iDataLen);

#ifdef DEBUG
        if (iBytesWrite > 0) U_INTERNAL_DUMP("BytesWrite(%d) = %#.*S", iBytesWrite, iBytesWrite, pData)
   else if (iBytesWrite < 0)
      {
      setStatus(iBytesWrite);

      U_INTERNAL_DUMP("status = %.*S", u_buffer_len, u_buffer)

      u_buffer_len = 0;
      }
#endif

   U_RETURN(iBytesWrite);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USSHSocket::dump(bool reset) const
{
   USocket::dump(false);

   *UObjectIO::os << '\n'
                  << "ret                          " << ret                 << '\n'
                  << "auth                         " << auth                << '\n'
                  << "user                         " << (void*)&user        << '\n'
                  << "buffer                       " << (void*)buffer       << '\n'
                  << "session                      " << (void*)session      << '\n'
                  << "channel                      " << (void*)channel      << '\n'
                  << "public_key                   " << (void*)&public_key  << '\n'
                  << "private_key                  " << (void*)&private_key;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
