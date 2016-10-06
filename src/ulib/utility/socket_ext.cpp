// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    socket_ext.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/notifier.h>
#include <ulib/utility/interrupt.h>
#include <ulib/net/server/server.h>

#ifdef HAVE_SYS_SENDFILE_H
#  ifndef HAVE_SENDFILE64
#     undef __USE_FILE_OFFSET64
#  endif
#  include <sys/sendfile.h>
#  ifndef HAVE_SENDFILE64
#     define __USE_FILE_OFFSET64
#  endif
#endif

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#elif defined(HAVE_NETPACKET_PACKET_H) && !defined(U_ALL_CPP)
#  include <net/if.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif

#ifdef USE_C_ARES
#  include <ares.h>
#endif

/**
 * Socket I/O - read while not received almost count data
 *
 * @timeoutMS  specified the timeout value, in milliseconds.
 *             A negative value indicates no timeout, i.e. an infinite wait
 * @time_limit specified the maximum execution time, in seconds. If set to zero, no time limit is imposed
 */

bool USocketExt::read(USocket* sk, UString& buffer, uint32_t count, int timeoutMS, uint32_t time_limit)
{
   U_TRACE(0, "USocketExt::read(%p,%V,%u,%d,%u)", sk, buffer.rep, count, timeoutMS, time_limit)

   U_INTERNAL_ASSERT_POINTER(sk)
   U_INTERNAL_ASSERT(sk->isConnected())

   U_DUMP("bssl = %b blocking = %b", sk->isSSLActive(), sk->isBlocking())

   char* ptr;
   long timeout = 0;
   int byte_read = 0;
   ssize_t value; // = -1;
   uint32_t start  = buffer.size(), // NB: read buffer can have previous data...
            ncount = buffer.space(),
            chunk  = count;

   if (LIKELY(chunk < U_CAPACITY)) chunk = U_CAPACITY;

   if (UNLIKELY(ncount < chunk))
      {
      if (sk == UServer_Base::csocket) UClientImage_Base::manageReadBufferResize(chunk);
      else                                             UString::_reserve(buffer, chunk);

      ncount = buffer.space();
      }

   ptr = buffer.c_pointer(start);

read:
   if (sk->isBlocking() &&
       timeoutMS != 0)
      {
#  ifndef USE_LIBSSL
      if (errno = 0, UNotifier::waitForRead(sk->iSockDesc, timeoutMS) != 1) goto error;
#  else
      /**
       * When packets in SSL arrive at a destination, they are pulled off the socket in chunks of sizes
       * controlled by the encryption protocol being used, decrypted, and placed in SSL-internal buffers.
       * The buffer content is then transferred to the application program through SSL_read(). If you've
       * read only part of the decrypted data, there will still be pending input data on the SSL connection,
       * but it won't show up on the underlying file descriptor via select(). Your code needs to call
       * SSL_pending() explicitly to see if there is any pending data to be read
       */

      U_DUMP("sk->pending() = %u", ((USSLSocket*)sk)->pending())

      /**
       * What I see by myself is that for blocking socket SSL_read() can return some data though
       * it won't show up on the underlying file descriptor via select() while SSL_pending() return 0...
       */

      if (sk->isSSLActive() == false && // NB: without this csp test fail: we have a timeout(10 seconds) when we try to read SOAP body response...
          (errno = 0, UNotifier::waitForRead(sk->iSockDesc, timeoutMS) != 1))
         {
         goto error;
         }
#  endif
      }

   value = sk->recv(ptr + byte_read, ncount);

   if (value <= 0)
      {
      if (value == -1)
         {
error:   U_INTERNAL_DUMP("errno = %d", errno)

         if (errno != EAGAIN)
            {
            if (U_ClientImage_parallelization != U_PARALLELIZATION_CHILD)
               {
               if (errno != ECONNRESET &&
                   sk == UServer_Base::csocket)
                  {
                  sk->iState = USocket::BROKEN;
                  }

               sk->abortive_close();
               }

            U_RETURN(false);
            }

         if (timeoutMS != 0)
            {
            if (UNotifier::waitForRead(sk->iSockDesc, timeoutMS) == 1) goto read;

            sk->iState |= USocket::TIMEOUT;
            }

         U_INTERNAL_DUMP("sk->state = %d %B", sk->iState, sk->iState)
         }
      else
         {
         U_INTERNAL_ASSERT_EQUALS(value, 0)

         errno = 0;

         if (byte_read == 0 ||
             sk->shutdown(SHUT_RD) == false)
            {
            U_INTERNAL_DUMP("byte_read = %d errno = %d", byte_read, errno)

            if (U_ClientImage_parallelization != U_PARALLELIZATION_CHILD) sk->abortive_close();

            U_RETURN(false);
            }

         UClientImage_Base::setCloseConnection();
         }

      goto done;
      }

   byte_read += value;

   U_INTERNAL_DUMP("byte_read = %d", byte_read)

   U_INTERNAL_ASSERT_MAJOR(byte_read, 0)

   if (byte_read < (int)count)
      {
      U_INTERNAL_ASSERT_DIFFERS(count, U_SINGLE_READ)

      if (time_limit &&
          sk->checkTime(time_limit, timeout) == false) // NB: may be we are attacked by a "slow loris"... http://lwn.net/Articles/337853/
         {
         sk->iState |= USocket::TIMEOUT;

         goto done;
         }

      ncount -= value;

      goto read;
      }

   if (value == (ssize_t)ncount)
      {
      // NB: may be there are available more bytes to read...

      buffer.rep->_length = start + byte_read;

      if (sk == UServer_Base::csocket) UClientImage_Base::manageReadBufferResize(ncount * 2);
      else                                             UString::_reserve(buffer, ncount * 2);

      ptr       = buffer.c_pointer(start);
      ncount    = buffer.space();
      timeoutMS = 0;

      goto read;
      }

#ifdef U_EPOLLET_POSTPONE_STRATEGY
   if (UNotifier::bepollet == false)
#endif
   {
#if !defined(U_LINUX) || !defined(ENABLE_THREAD) || !defined(U_LOG_DISABLE) || defined(USE_LIBZ)
   if (sk->isBlocking() == false)
      {
      /**
       * Edge trigger (EPOLLET) simply means (unless you've used EPOLLONESHOT) that you'll get 1 event when something
       * enters the (kernel) buffer. Thus, if you get 1 EPOLLIN event and do nothing about it, you'll get another
       * EPOLLIN the next time some data arrives on that descriptor - if no new data arrives, you will not get an
       * event though, even if you didn't read any data as indicated by the first event. Well, to put it succinctly,
       * EPOLLONESHOT just means that if you don't read the data you're supposed to read, they will be discarded.
       * Normally, you'd be notified with an event for the same data if you don't read them. With EPOLLONESHOT, however,
       * not reading the data is perfectly legal and they will be just ignored. Hence, no further events will be generated.
       * -------------------------------------------------------------------------------------------------------------------
       * The suggested way to use epoll as an edge-triggered (EPOLLET) interface is as follows:
       *
       * 1) with nonblocking file descriptors
       * 2) by waiting for an event only after read(2) or write(2) return EAGAIN
       * -------------------------------------------------------------------------------------------------------------------
       * Edge-triggered semantics allow a more efficient internal implementation than level-triggered semantics.
       *
       * see: https://raw.githubusercontent.com/dankamongmen/libtorque/master/doc/mteventqueues
       */

      buffer.rep->_length = start + byte_read;

      ncount    = buffer.space();
      timeoutMS = 0;

      goto read;
      }
#endif
   }

done:
   U_INTERNAL_DUMP("byte_read = %d", byte_read)

   if (byte_read)
      {
      start += byte_read;

      if (start > buffer.size()) buffer.size_adjust_force(start); // NB: we force because the string can be referenced...

      if (byte_read >= (int)count &&
          sk->iState != USocket::CLOSE)
         {
         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

/**
 * Socket I/O - read while not received token, return position of token in buffer read
 *
 * @param timeoutMS specified the timeout value, in milliseconds.
 *        A negative value indicates no timeout, i.e. an infinite wait
 */

uint32_t USocketExt::readWhileNotToken(USocket* sk, UString& buffer, const char* token, uint32_t token_len, int timeoutMS)
{
   U_TRACE(0, "USocketExt::readWhileNotToken(%p,%V,%.*S,%u,%d)", sk, buffer.rep, token_len, token, token_len, timeoutMS)

   uint32_t start = buffer.size();

   while (USocketExt::read(sk, buffer, U_SINGLE_READ, timeoutMS))
      {
      uint32_t pos_token = buffer.find(token, start, token_len);

      if (pos_token != U_NOT_FOUND) U_RETURN(pos_token);

      U_ASSERT_MAJOR(buffer.size(), token_len)

      start = buffer.size() - token_len;
      }

   U_RETURN(U_NOT_FOUND);
}

// write data

int USocketExt::write(USocket* sk, const char* ptr, uint32_t count, int timeoutMS)
{
   U_TRACE(0, "USocketExt::write(%p,%.*S,%u,%d)", sk, count, ptr, count, timeoutMS)

   U_INTERNAL_ASSERT_POINTER(sk)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT(sk->isConnected())

   U_DUMP("bssl = %b blocking = %b", sk->isSSLActive(), sk->isBlocking())

   ssize_t value;
   int byte_written = 0;

write:
   if (sk->isBlocking() &&
       timeoutMS != 0   &&
       (errno = 0, UNotifier::waitForWrite(sk->iSockDesc, timeoutMS) != 1))
      {
      goto error;
      }

   value = sk->send(ptr + byte_written, count);

   if (value <= 0)
      {
      if (value == -1)
         {
error:   U_INTERNAL_DUMP("errno = %d", errno)

              if (errno != EAGAIN) sk->abortive_close();
         else if (timeoutMS != 0)
            {
            if (UNotifier::waitForWrite(sk->iSockDesc, timeoutMS) == 1) goto write;

            sk->iState |= USocket::TIMEOUT;
            }

         U_INTERNAL_DUMP("sk->state = %d %B", sk->iState, sk->iState)
         }

      U_RETURN(byte_written);
      }

   byte_written += value;

   U_INTERNAL_DUMP("byte_written = %d", byte_written)

   U_INTERNAL_ASSERT_MAJOR(byte_written, 0)

   if (byte_written < (int)count)
      {
      count -= value;

      goto write;
      }

   U_RETURN(byte_written);
}

// sendfile() copies data between one file descriptor and another. Either or both of these file descriptors may refer to a socket.
// OUT_FD should be a descriptor opened for writing. POFFSET is a pointer to a variable holding the input file pointer position from
// which sendfile() will start reading data. When sendfile() returns, this variable will be set to the offset of the byte following
// the last byte that was read. COUNT is the number of bytes to copy between file descriptors. Because this copying is done within
// the kernel, sendfile() does not need to spend time transferring data to and from user space

int USocketExt::sendfile(USocket* sk, int in_fd, off_t* poffset, uint32_t count, int timeoutMS)
{
   U_TRACE(1, "USocketExt::sendfile(%p,%d,%p,%u,%d)", sk, in_fd, poffset, count, timeoutMS)

   U_INTERNAL_ASSERT_POINTER(sk)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT(sk->isConnected())

   U_DUMP("bssl = %b blocking = %b", sk->isSSLActive(), sk->isBlocking())

   U_INTERNAL_ASSERT_EQUALS(sk->isSSLActive(), false)

#if defined(HAVE_MACOSX_SENDFILE)
   off_t len;
#endif
   ssize_t value;
   int byte_written = 0;

loop:
   if (sk->isBlocking() &&
       timeoutMS != 0   &&
       (errno = 0, UNotifier::waitForWrite(sk->iSockDesc, timeoutMS) != 1))
      {
      goto error;
      }

#if defined(HAVE_MACOSX_SENDFILE)
   /**
    * struct sf_hdtr {
    *  struct iovec *headers;  // pointer to  header iovecs
    *  int hdr_cnt;            //  number of  header iovecs
    *  struct iovec *trailers; // pointer to trailer iovecs
    *  int trl_cnt;            //  number of trailer iovecs
    * };
    *
    * int sendfile(int fd, int s, off_t offset, off_t* len, struct sf_hdtr* hdtr, int flags);
    *
    * Since Mac OSX uses the fourth argument as a value-return parameter, success or failure, we need to put the result into value after the call
    */

   len   = count;
   value = U_SYSCALL(sendfile, "%d,%d,%p,%u", sk->getFd(), in_fd, *poffset, &len, 0, 0);

   if (value == -1) goto error;

   poffset += (value = len);
#else
   value = U_SYSCALL(sendfile, "%d,%d,%p,%u", sk->getFd(), in_fd, poffset, count);
#endif

   if (value <= 0)
      {
      if (value == -1)
         {
error:   U_INTERNAL_DUMP("errno = %d", errno)

         if (errno != EAGAIN)
            {
            if (errno == EINTR)
               {
               UInterrupt::checkForEventSignalPending();

               goto loop;
               }

            sk->abortive_close();
            }
         else if (timeoutMS != 0)
            {
            if (UNotifier::waitForWrite(sk->iSockDesc, timeoutMS) == 1) goto loop;

            sk->iState |= USocket::TIMEOUT;
            }

         U_INTERNAL_DUMP("sk->state = %d %B", sk->iState, sk->iState)
         }

      U_RETURN(byte_written);
      }

   byte_written += value;

   U_INTERNAL_DUMP("byte_written = %d", byte_written)

   U_INTERNAL_ASSERT_MAJOR(byte_written, 0)

   if (byte_written < (int)count)
      {
      count -= value;

      goto loop;
      }

   U_RETURN(byte_written);
}

// write data from multiple buffers

U_NO_EXPORT void USocketExt::iov_resize(struct iovec* iov, int iovcnt, size_t value)
{
   U_TRACE(0, "USocketExt::iov_resize(%p,%d,%d)", iov, iovcnt, value)

   int idx;

   for (idx = 0; value >= iov[idx].iov_len; ++idx)
      {
      value -= iov[idx].iov_len;
               iov[idx].iov_len = 0;
      }

   U_INTERNAL_DUMP("iov[%d].iov_len = %d", idx, iov[idx].iov_len)

   U_INTERNAL_ASSERT_MAJOR(iov[idx].iov_len, value)

   iov    += idx;
   iovcnt -= idx;

   U_INTERNAL_ASSERT_MAJOR(iovcnt, 0)

          iov[0].iov_base =
   (char*)iov[0].iov_base + value;
          iov[0].iov_len -= value;
}

int USocketExt::_writev(USocket* sk, struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS)
{
   U_TRACE(0, "USocketExt::_writev(%p,%p,%d,%u,%d)", sk, iov, iovcnt, count, timeoutMS)

   U_INTERNAL_ASSERT_POINTER(sk)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT(sk->isConnected())

   U_DUMP("bssl = %b blocking = %b", sk->isSSLActive(), sk->isBlocking())

   ssize_t value;
   int byte_written = 0;

#ifdef DEBUG
   int i;
   uint32_t sum;
   for (i = sum = 0; i < iovcnt; ++i) sum += iov[i].iov_len;
   U_INTERNAL_ASSERT_EQUALS(sum, count)
#endif

loop:
   if (sk->isBlocking() &&
       timeoutMS != 0   &&
       (errno = 0, UNotifier::waitForWrite(sk->iSockDesc, timeoutMS) != 1))
      {
      goto error;
      }

#if defined(USE_LIBSSL) && !defined(_MSWINDOWS_)
   if (sk->isSSLActive())
#endif
#if defined(USE_LIBSSL) ||  defined(_MSWINDOWS_)
   {
   U_INTERNAL_ASSERT_EQUALS(iovcnt, 1)

   value = sk->send((const char*)iov[0].iov_base, iov[0].iov_len);

   goto check;
   }
#endif
   value = U_SYSCALL(writev, "%d,%p,%d", sk->iSockDesc, iov, iovcnt);

#if defined(USE_LIBSSL) ||  defined(_MSWINDOWS_)
check:
#endif
   if (value <= 0)
      {
      if (value == -1)
         {
error:   U_INTERNAL_DUMP("errno = %d", errno)

         if (errno != EAGAIN)
            {
            if (errno == EINTR)
               {
               UInterrupt::checkForEventSignalPending();

               goto loop;
               }

            sk->abortive_close();
            }
         else if (timeoutMS != 0)
            {
            if (UNotifier::waitForWrite(sk->iSockDesc, timeoutMS) == 1) goto loop;

            sk->iState |= USocket::TIMEOUT;
            }

         U_INTERNAL_DUMP("sk->state = %d %B", sk->iState, sk->iState)
         }

      U_RETURN(byte_written);
      }

   byte_written += value;

   U_INTERNAL_DUMP("byte_written = %d", byte_written)

   U_INTERNAL_ASSERT_MAJOR(byte_written, 0)

   if (byte_written < (int)count)
      {
      iov_resize(iov, iovcnt, value);

      goto loop;
      }

   U_RETURN(byte_written);
}

int USocketExt::writev(USocket* sk, struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS)
{
   U_TRACE(0, "USocketExt::writev(%p,%p,%d,%u,%d)", sk, iov, iovcnt, count, timeoutMS)

   U_INTERNAL_ASSERT_POINTER(sk)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT(sk->isConnected())

#if defined(USE_LIBSSL) && !defined(_MSWINDOWS_)
   if (sk->isSSLActive())
#endif
#if defined(USE_LIBSSL) ||  defined(_MSWINDOWS_)
   {
   int sz, byte_written;

   if (count <= U_CAPACITY)
      {
      U_INTERNAL_ASSERT_MINOR(iovcnt, 255)

      struct iovec _iov[256];
      UString buffer(U_CAPACITY);

      for (int i = 0; i < iovcnt; ++i)
         {
         if ((sz = _iov[i].iov_len = iov[i].iov_len))
            {
            (void) buffer.append((const char*)(_iov[i].iov_base = iov[i].iov_base), sz);
            }
         }

      U_INTERNAL_ASSERT_EQUALS(count, buffer.size())

      _iov[iovcnt].iov_len  = count;
      _iov[iovcnt].iov_base = buffer.data();

      byte_written = _writev(sk, _iov+iovcnt, 1, count, timeoutMS);

      if (byte_written < (int)count)
         {
         if (byte_written) iov_resize(iov, iovcnt, byte_written);
         }
      }
   else
      {
      ssize_t value;

      byte_written = 0;

      for (int i = 0; i < iovcnt; ++i)
         {
         if ((sz = iov[i].iov_len))
            {
            value = _writev(sk, iov+i, 1, sz, timeoutMS);

            byte_written += value;

            if (value < sz) break;

            iov[i].iov_len = 0;
            }
         }
      }

   U_RETURN(byte_written);
   }
#endif

   int byte_written = _writev(sk, iov, iovcnt, count, timeoutMS);

   U_RETURN(byte_written);
}

int USocketExt::writev(USocket* sk, struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS, uint32_t cloop)
{
   U_TRACE(0, "USocketExt::writev(%p,%p,%d,%u,%d,%u)", sk, iov, iovcnt, count, timeoutMS, cloop)

   U_INTERNAL_ASSERT_POINTER(sk)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT_MAJOR(cloop, 0)
   U_INTERNAL_ASSERT_MINOR(iovcnt, 256)
   U_INTERNAL_ASSERT(sk->isConnected())

   struct iovec _iov[256];

   char* ptr   = (char*)_iov;
   uint32_t sz = sizeof(struct iovec) * iovcnt;

   U_MEMCPY(ptr, iov, sz);

#ifdef U_PIPELINE_HOMOGENEOUS_DISABLE
   U_INTERNAL_ASSERT_EQUALS(cloop, 1)
#else
   if (cloop > 1)
      {
      for (uint32_t i = 1; i < cloop; ++i)
         {
                  ptr +=    sz;
         U_MEMCPY(ptr, iov, sz);
         }

      iov     = _iov;
      iovcnt *= cloop;
      }
#endif

   U_INTERNAL_DUMP("iov[0].iov_len = %d iov[1].iov_len = %d", iov[0].iov_len, iov[1].iov_len)

#if defined(USE_LIBSSL) || defined(_MSWINDOWS_)
   int byte_written = writev(sk, iov, iovcnt, count, timeoutMS);
#else
   U_INTERNAL_ASSERT_EQUALS(sk->isSSLActive(), false)

   int byte_written = _writev(sk, iov, iovcnt, count, timeoutMS);
#endif

        if (cloop == 1) U_MEMCPY(iov, _iov, sz);
#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
   else if (cloop > 1                 &&
            byte_written < (int)count &&
            byte_written > 0)
      {
      U_INTERNAL_ASSERT(sk->isOpen())
      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      ptr = u_buffer;

      for (int i = 0; i < iovcnt; ++i)
         {
         if (iov[i].iov_len)
            {
            U_MEMCPY(ptr, iov[i].iov_base, iov[i].iov_len);
                     ptr +=                iov[i].iov_len;
            }
         }

      u_buffer_len = ptr - u_buffer;

      U_INTERNAL_ASSERT_MINOR( u_buffer_len, U_BUFFER_SIZE)
      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, count - byte_written)
      }
#endif

   U_INTERNAL_DUMP("iov[0].iov_len = %d iov[1].iov_len = %d", iov[0].iov_len, iov[1].iov_len)

   U_RETURN(byte_written);
}

// Send a command to a server and wait for a response (single line)

int USocketExt::vsyncCommand(USocket* sk, char* buffer, uint32_t buffer_size, const char* format, uint32_t fmt_size, va_list argp)
{
   U_TRACE(0, "USocketExt::vsyncCommand(%p,%p,%u,%.*S,%u)", sk, buffer, buffer_size, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT(sk->isOpen())

   uint32_t buffer_len = u__vsnprintf(buffer, buffer_size-2, format, fmt_size, argp);

   buffer[buffer_len++] = '\r';
   buffer[buffer_len++] = '\n';

   int n        =  sk->send(buffer, buffer_len),
       response = (sk->checkIO(n) ? readLineReply(sk, buffer, buffer_size) : 0);

   U_RETURN(response);
}

// Send a command to a server and wait for a response (multi line)

int USocketExt::vsyncCommandML(USocket* sk, char* buffer, uint32_t buffer_size, const char* format, uint32_t fmt_size, va_list argp)
{
   U_TRACE(0, "USocketExt::vsyncCommandML(%p,%p,%u,%.*S,%u)", sk, buffer, buffer_size, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT(sk->isOpen())

   uint32_t buffer_len = u__vsnprintf(buffer, buffer_size-2, format, fmt_size, argp);

   buffer[buffer_len++] = '\r';
   buffer[buffer_len++] = '\n';

   int n        =  sk->send(buffer, buffer_len),
       response = (sk->checkIO(n) ? readMultilineReply(sk, buffer, buffer_size) : 0);

   U_RETURN(response);
}

// Send a command to a server and wait for a response (check for token line)

int USocketExt::vsyncCommandToken(USocket* sk, UString& buffer, const char* format, uint32_t fmt_size, va_list argp)
{
   U_TRACE(1, "USocketExt::vsyncCommandToken(%p,%V,%.*S,%u)", sk, buffer.rep, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT(sk->isOpen())
   U_INTERNAL_ASSERT_EQUALS((bool)buffer, false)

   static uint32_t cmd_count;

   char token[32];
   uint32_t token_len = u__snprintf(token, sizeof(token), U_CONSTANT_TO_PARAM("U%04u "), cmd_count++);

   U_INTERNAL_DUMP("token = %.*S", token_len, token)

   char* p = buffer.data();

   U_MEMCPY(p, token, token_len);

   uint32_t buffer_len = token_len + u__vsnprintf(p+token_len, buffer.capacity(), format, fmt_size, argp);

   p[buffer_len++] = '\r';
   p[buffer_len++] = '\n';

   int n = sk->send(p, buffer_len);

   if (sk->checkIO(n))
      {
      uint32_t pos_token = USocketExt::readWhileNotToken(sk, buffer, token, token_len);

      if (pos_token != U_NOT_FOUND)
         {
                          U_ASSERT(buffer.c_char(buffer.size()-1) == '\n')
#     ifdef DEBUG
         if (pos_token) { U_ASSERT(buffer.c_char(pos_token-1)     == '\n') }
#     endif

         U_RETURN(pos_token + token_len);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

U_NO_EXPORT inline bool USocketExt::parseCommandResponse(char* buffer, int r, int response)
{
   U_TRACE(0, "USocketExt::parseCommandResponse(%p,%d,%d)", buffer, r, response)

   /**
    * Thus the format for multi-line replies is that the first line will begin with the exact required reply code,
    * followed immediately by a Hyphen, "-" (also known as Minus), followed by text. The last line will begin with
    * the same code, followed immediately by Space <SP>, optionally some text, and the Telnet end-of-line code.
    * For example:
    * 123-First line
    *    Second line
    *    234 A line beginning with numbers
    *    123 The last line
    * The user-process then simply needs to search for the second occurrence of the same reply code, followed by
    * <SP> (Space), at the beginning of a line, and ignore all intermediary lines. If an intermediary line begins
    * with a 3-digit number, the Server must pad the front to avoid confusion
    */

   int complete = 2;

   if (buffer[3] == '-')
      {
      complete = 0;

      for (int i = 0; i < r; ++i)
         {
         if (buffer[i] == '\n')
            {
            if (complete == 1)
               {
               complete = 2;

               break;
               }

            U_INTERNAL_DUMP("buffer = %S", buffer+i+1)

            if (buffer[i+4] == ' ')
               {
               int j = -1;

               (void) sscanf(buffer+i+1, "%3i", &j);

               U_INTERNAL_DUMP("j = %d response = %d", j, response)

               if (j == response) complete = 1;
               }
            }
         }
      }

   U_INTERNAL_DUMP("complete = %d", complete)

   U_RETURN(complete != 2);
}

int USocketExt::readLineReply(USocket* sk, char* buffer, uint32_t buffer_size) // response from server (single line)
{
   U_TRACE(0, "USocketExt::readLineReply(%p,%p,%u)", sk, buffer, buffer_size)

   U_INTERNAL_ASSERT(sk->isConnected())

   int i, r = 0;

   do {
      int count = buffer_size - r;

      i = sk->recv(buffer + r, count);

      if (sk->checkIO(i) == false) U_RETURN(0);

      r += i;
      }
   while (buffer[r-1] != '\n');

   buffer[r] = '\0';

   U_RETURN(r);
}

int USocketExt::readMultilineReply(USocket* sk, char* buffer, uint32_t buffer_size) // response from server (multi line)
{
   U_TRACE(0, "USocketExt::readMultilineReply(%p,%p,%u)", sk, buffer, buffer_size)

   U_INTERNAL_ASSERT(sk->isConnected())

   int r = 0, response = 0;

   do {
      r = readLineReply(sk, buffer + r, buffer_size - r);

      if (r) response = atoi(buffer);
      }
   while (parseCommandResponse(buffer, r, response));

   U_RETURN(response);
}

// SERVICES

UString USocketExt::getNetworkDevice(const char* exclude)
{
   U_TRACE(1, "USocketExt::getNetworkDevice(%S)", exclude)

   UString result(100U);

#if !defined(_MSWINDOWS_) && defined(HAVE_SYS_IOCTL_H)
   FILE* route = (FILE*) U_SYSCALL(fopen, "%S,%S", "/proc/net/route", "r");

   if (U_SYSCALL(fscanf, "%p,%S", route, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s") != EOF) // Skip the first line
      {
      char  dev[7],
           dest[9];
      char* ptr = dest;

      while (U_SYSCALL(fscanf, "%p,%S", route, "%6s %8s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n", dev, dest) != EOF)
         {
         bool found = (exclude ? (strncmp(dev, exclude, 6) != 0)                                                        // not the whatever it is
                               : (u_get_unalignedp64(ptr) == U_MULTICHAR_CONSTANT64('0','0','0','0','0','0','0','0'))); // default route

         if (found)
            {
            (void) result.assign(dev);

            break;
            }
         }
      }

   (void) U_SYSCALL(fclose, "%p", route);
#endif

   U_RETURN_STRING(result);
}

bool USocketExt::getARPCache(UString& cache, UVector<UString>& vec)
{
   U_TRACE(0+256, "USocketExt::getARPCache(%V,%p)", cache.rep, &vec)

#if !defined(_MSWINDOWS_) && defined(HAVE_SYS_IOCTL_H)
   /*
   FILE* arp = (FILE*) U_SYSCALL(fopen, "%S,%S", "/proc/net/arp", "r");

   // ------------------------------------------------------------------------------
   // Skip the first line
   // ------------------------------------------------------------------------------
   // IP address       HW type     Flags       HW address            Mask     Device
   // 192.168.253.1    0x1         0x2         00:14:a5:6e:9c:cb     *        ath0
   // 10.30.1.131      0x1         0x2         00:16:ec:fb:46:da     *        eth0
   // ------------------------------------------------------------------------------

   if (U_SYSCALL(fscanf, "%p,%S", arp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s") != EOF)
      {
      char _ip[16];

      while (U_SYSCALL(fscanf, "%p,%S", arp, "%15s %*s %*s %*s %*s %*s\n", _ip) != EOF)
         {
         UString item((void*)_ip);

         vec.push_back(item);
         }
      }

   (void) U_SYSCALL(fclose, "%p", arp);
   */

   UString content = UFile::getSysContent("/proc/net/arp");

   if (cache != content)
      {
      UString item;
      UVector<UString> vec_row(content, '\n'), vec_entry(6);

      vec.clear();

      cache = content;

      for (uint32_t i = 1, n = vec_row.size(); i < n; ++i) // Skip the first line
         {
         // ------------------------------------------------------------------------------
         // IP address       HW type     Flags       HW address            Mask     Device
         // ------------------------------------------------------------------------------
         // 192.168.253.1    0x1         0x2         00:14:a5:6e:9c:cb     *        ath0
         // 10.30.1.131      0x1         0x2         00:16:ec:fb:46:da     *        eth0
         // ------------------------------------------------------------------------------

         (void) vec_entry.split(vec_row[i]);

         item = vec_entry[0]; // ip

         U_INTERNAL_ASSERT(item)

         vec.push_back(item);

         item = vec_entry[3]; // mac

         U_INTERNAL_ASSERT(item)

         vec.push_back(item);

         item = vec_entry[5]; // dev

         U_INTERNAL_ASSERT(item)

         vec.push_back(item);

         vec_entry.clear();
         }

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

UString USocketExt::getNetworkInterfaceName(const char* ip, uint32_t ip_len)
{
   U_TRACE(0, "USocketExt::getNetworkInterfaceName(%.*S,%u)", ip_len, ip, ip_len)

   U_INTERNAL_ASSERT(u_isIPv4Addr(ip, ip_len))

   UString result(100U);

#if !defined(_MSWINDOWS_) && defined(HAVE_SYS_IOCTL_H)
   /*
   FILE* arp = (FILE*) U_SYSCALL(fopen, "%S,%S", "/proc/net/arp", "r");

   // ------------------------------------------------------------------------------
   // Skip the first line
   // ------------------------------------------------------------------------------
   // IP address       HW type     Flags       HW address            Mask     Device
   // 192.168.253.1    0x1         0x2         00:14:a5:6e:9c:cb     *        ath0
   // 10.30.1.131      0x1         0x2         00:16:ec:fb:46:da     *        eth0
   // ------------------------------------------------------------------------------

   if (U_SYSCALL(fscanf, "%p,%S", arp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s") != EOF)
      {
      char _ip[16], dev[16];

      while (U_SYSCALL(fscanf, "%p,%S", arp, "%15s %*s %*s %*s %*s %15s\n", _ip, dev) != EOF)
         {
         if (strcmp(ip, _ip) == 0)
            {
            (void) result.assign(dev);

            break;
            }
         }
      }

   (void) U_SYSCALL(fclose, "%p", arp);
   */

   UString content;
   UVector<UString> vec;

   if (getARPCache(content, vec))
      {
      for (uint32_t i = 0, n = vec.size(); i < n; i += 3)
         {
         if (vec[i].equal(ip, ip_len))
            {
            result = vec[i+2].copy();

            break;
            }
         }
      }
#endif

   U_RETURN_STRING(result);
}

UString USocketExt::getMacAddress(const char* ip, uint32_t ip_len)
{
   U_TRACE(0, "USocketExt::getMacAddress(%.*S,%u)", ip_len, ip, ip_len)

   U_INTERNAL_ASSERT(u_isIPv4Addr(ip, ip_len))

#if !defined(_MSWINDOWS_) && defined(HAVE_SYS_IOCTL_H)
   /*
   FILE* arp = (FILE*) U_SYSCALL(fopen, "%S,%S", "/proc/net/arp", "r");

   // ------------------------------------------------------------------------------
   // Skip the first line
   // ------------------------------------------------------------------------------
   // IP address       HW type     Flags       HW address            Mask     Device
   // 192.168.253.1    0x1         0x2         00:14:a5:6e:9c:cb     *        ath0
   // 10.30.1.131      0x1         0x2         00:16:ec:fb:46:da     *        eth0
   // ------------------------------------------------------------------------------

   if (U_SYSCALL(fscanf, "%p,%S", arp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s") != EOF)
      {
      char ip[16], hw[18];

      while (U_SYSCALL(fscanf, "%p,%S", arp, "%15s %*s %*s %17s %*s %*s\n", ip, hw) != EOF)
         {
         if (strncmp(device_or_ip, ip, sizeof(ip)) == 0)
            {
            (void) result.assign(hw);

            break;
            }
         }
      }

   (void) U_SYSCALL(fclose, "%p", arp);
   */

   UString content;
   UVector<UString> vec;

   if (getARPCache(content, vec))
      {
      UString result;

      for (uint32_t i = 0, n = vec.size(); i < n; i += 3)
         {
         if (vec[i].equal(ip, ip_len))
            {
            result = vec[i+1].copy();

            U_RETURN_STRING(result);
            }
         }
      }
#endif

   U_RETURN_STRING(*UString::str_without_mac);
}

UString USocketExt::getMacAddress(int fd, const char* device)
{
   U_TRACE(1, "USocketExt::getMacAddress(%d,%S)", fd, device)

   U_INTERNAL_ASSERT_POINTER(device)

   UString result(100U);

#ifdef U_LINUX
   U_INTERNAL_ASSERT(fd != -1)

   struct ifreq ifr;

   (void) u__strncpy(ifr.ifr_name, device, IFNAMSIZ-1);

   if (U_SYSCALL(ioctl, "%d,%d,%p", fd, SIOCGIFHWADDR, &ifr) == 0)
      {
      char* hwaddr = ifr.ifr_hwaddr.sa_data;

      result.snprintf(U_CONSTANT_TO_PARAM("%02x:%02x:%02x:%02x:%02x:%02x"),
                      hwaddr[0] & 0xFF,
                      hwaddr[1] & 0xFF,
                      hwaddr[2] & 0xFF,
                      hwaddr[3] & 0xFF,
                      hwaddr[4] & 0xFF,
                      hwaddr[5] & 0xFF);
      }
#endif

   U_RETURN_STRING(result);
}

UString USocketExt::getIPAddress(int fd, const char* device)
{
   U_TRACE(1, "USocketExt::getIPAddress(%d,%S)", fd, device)

   U_INTERNAL_ASSERT(fd != -1)
   U_INTERNAL_ASSERT_POINTER(device)

   UString result(100U);

#ifdef U_LINUX
   struct ifreq ifr;

   (void) u__strncpy(ifr.ifr_name, device, IFNAMSIZ-1);

   /* Get the IP address of the interface */

   if (U_SYSCALL(ioctl, "%d,%d,%p", fd, SIOCGIFADDR, &ifr) == 0)
      {
      uusockaddr addr;

      U_MEMCPY(&addr, &ifr.ifr_addr, sizeof(struct sockaddr));

      U_INTERNAL_ASSERT_EQUALS(addr.psaIP4Addr.sin_family, AF_INET)

      (void) U_SYSCALL(inet_ntop, "%d,%p,%p,%u", AF_INET, &(addr.psaIP4Addr.sin_addr), result.data(), INET_ADDRSTRLEN);

      result.size_adjust();
      }
#endif

   U_RETURN_STRING(result);
}

UString USocketExt::getNetworkAddress(int fd, const char* device)
{
   U_TRACE(1, "USocketExt::getNetworkAddress(%d,%S)", fd, device)

   U_INTERNAL_ASSERT(fd != -1)
   U_INTERNAL_ASSERT_POINTER(device)

   UString result(100U);

#ifdef U_LINUX
   struct ifreq ifaddr, ifnetmask;

   (void) u__strncpy(   ifaddr.ifr_name, device, IFNAMSIZ-1);
   (void) u__strncpy(ifnetmask.ifr_name, device, IFNAMSIZ-1);

   // retrieve the IP address and subnet mask

   if (U_SYSCALL(ioctl, "%d,%d,%p", fd, SIOCGIFADDR,    &ifaddr)    == 0 &&
       U_SYSCALL(ioctl, "%d,%d,%p", fd, SIOCGIFNETMASK, &ifnetmask) == 0)
      {
      // compute the current network value from the address and netmask

      int network;
      uusockaddr addr, netmask;

      U_MEMCPY(&addr,    &ifaddr.ifr_addr,       sizeof(struct sockaddr));
      U_MEMCPY(&netmask, &ifnetmask.ifr_netmask, sizeof(struct sockaddr));

      U_INTERNAL_ASSERT_EQUALS(addr.psaIP4Addr.sin_family,    AF_INET)
      U_INTERNAL_ASSERT_EQUALS(netmask.psaIP4Addr.sin_family, AF_INET)

      network =     addr.psaIP4Addr.sin_addr.s_addr &
                 netmask.psaIP4Addr.sin_addr.s_addr;

      /*
      result.snprintf(U_CONSTANT_TO_PARAM("%d.%d.%d.%d"),
                      (network       & 0xFF),
                      (network >>  8 & 0xFF),
                      (network >> 16 & 0xFF),
                      (network >> 24 & 0xFF));
      */

      (void) U_SYSCALL(inet_ntop, "%d,%p,%p,%u", AF_INET, &network, result.data(), INET_ADDRSTRLEN);

      result.size_adjust();
      }
#endif

   U_RETURN_STRING(result);
}

#ifdef USE_C_ARES
int   USocketExt::resolv_status;
char  USocketExt::resolv_hostname[INET6_ADDRSTRLEN];
void* USocketExt::resolv_channel;

U_NO_EXPORT void USocketExt::callbackResolv(void* arg, int status, int timeouts, struct hostent* phost)
{
   U_TRACE(0, "USocketExt::callbackResolv(%p,%d,%d,%p)", arg, status, timeouts, phost)

   U_INTERNAL_ASSERT_POINTER(resolv_channel)
   U_INTERNAL_ASSERT_EQUALS(resolv_status, ARES_ENODATA)

   resolv_status = status;

   if (phost)
      {
#  ifdef HAVE_INET_NTOP
      (void) U_SYSCALL(inet_ntop, "%d,%p,%p,%u", phost->h_addrtype, phost->h_addr_list[0], resolv_hostname, INET6_ADDRSTRLEN);
#  else
      char* result = U_SYSCALL(inet_ntoa, "%u", *((struct in_addr*)phost->h_addr_list[0]));

      if (result) u__strcpy(resolv_hostname, result);
#  endif

      U_INTERNAL_DUMP("Found address name %s (%s) - status %d timeouts %d", phost->h_name, resolv_hostname, status, timeouts)
      }
}

void USocketExt::waitResolv()
{
   U_TRACE_NO_PARAM(1, "USocketExt::waitResolv()")

   U_INTERNAL_ASSERT_POINTER(resolv_channel)

   while (resolv_status == ARES_ENODATA)
      {
      int nfds;
      struct timeval tv;
      struct timeval* tvp;
      fd_set read_fds, write_fds;

      FD_ZERO( &read_fds);
      FD_ZERO(&write_fds);

      nfds = U_SYSCALL(ares_fds, "%p,%p,%p", (ares_channel)resolv_channel, &read_fds, &write_fds);

      if (nfds <= 0) break;

      tvp = (struct timeval*) U_SYSCALL(ares_timeout, "%p,%p,%p", (ares_channel)resolv_channel, 0, &tv);

      (void) U_SYSCALL(select, "%d,%p,%p,%p,%p", nfds, &read_fds, &write_fds, 0, tvp);

      U_SYSCALL_VOID(ares_process, "%p,%p,%p", (ares_channel)resolv_channel, &read_fds, &write_fds);

      U_INTERNAL_DUMP("status %d", resolv_status)
      }
}

void USocketExt::startResolv(const char* name, int family)
{
   U_TRACE(1, "USocketExt::startResolv(%S,%d)", name, family)

   if (resolv_channel == 0)
      {
      int status = U_SYSCALL(ares_library_init, "%d", ARES_LIB_INIT_ALL);

      if (status != ARES_SUCCESS) U_ERROR("ares_library_init() failed: %s", ares_strerror(status));

      struct ares_options options;

      union uuares_channeldata {
                  void** p1;
      ares_channeldata** p2;
      };

      union uuares_channeldata p = { &resolv_channel };

      status = U_SYSCALL(ares_init_options, "%p,%p,%d", p.p2, &options, 0);

      if (status != ARES_SUCCESS) U_ERROR("ares_init_options() failed: %s", ares_strerror(status));
      }

   resolv_status = ARES_ENODATA;

   U_SYSCALL_VOID(ares_gethostbyname, "%p,%S,%d,%p,%p", (ares_channel)resolv_channel, name, family, &USocketExt::callbackResolv, 0);
}
#endif

#ifdef U_LINUX
#  include <linux/types.h>
#  include <linux/rtnetlink.h>
#endif

UString USocketExt::getGatewayAddress(const char* network, uint32_t network_len)
{
   U_TRACE(1, "USocketExt::getGatewayAddress(%.*S,%u)", network_len, network, network_len)

   UString result(100U);

   // Ex: ip route show to exact 192.168.1.0/24

#ifdef U_LINUX
   static int sock;

   if (sock == 0) sock = USocket::socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

   if (sock != -1)
      {
      char msgBuf[4096];

      (void) U_SYSCALL(memset, "%p,%d,%u", msgBuf, 0, 4096);

      /*
      struct nlmsghdr {
         __u32 nlmsg_len;    // Length of message including header
         __u16 nlmsg_type;   // Type of message content
         __u16 nlmsg_flags;  // Additional flags
         __u32 nlmsg_seq;    // Sequence number
         __u32 nlmsg_pid;    // PID of the sending process
      };
      */

      // point the header and the msg structure pointers into the buffer

      union uunlmsghdr {
         char*            p;
         struct nlmsghdr* h;
      };

      union uunlmsghdr nlMsg = { &msgBuf[0] };

      // Fill in the nlmsg header

      nlMsg.h->nlmsg_len   = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message (28)
      nlMsg.h->nlmsg_type  = RTM_GETROUTE;                       // Get the routes from kernel routing table
      nlMsg.h->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;         // The message is a request for dump
      nlMsg.h->nlmsg_seq   = 0;                                  // Sequence of the message packet
      nlMsg.h->nlmsg_pid   = u_pid;                              // PID of process sending the request

      // Send the request

      if (U_SYSCALL(send, "%d,%p,%u,%u", sock, CAST(nlMsg.h), nlMsg.h->nlmsg_len, 0) == (ssize_t)nlMsg.h->nlmsg_len)
         {
         // Read the response

         int readLen;
         uint32_t msgLen = 0;
         char* bufPtr = msgBuf;
         union uunlmsghdr nlHdr;

         do {
            // Receive response from the kernel

            readLen = U_SYSCALL(recv, "%d,%p,%u,%d", sock, CAST(bufPtr), 4096 - msgLen, 0);

            if (readLen < 0) break;

            nlHdr.p = bufPtr;

            // Check if the header is valid

            if ((NLMSG_OK(nlHdr.h, (uint32_t)readLen) == 0) || (nlHdr.h->nlmsg_type == NLMSG_ERROR)) break;

            // Check if it is the last message

            U_INTERNAL_DUMP("nlmsg_type = %u nlmsg_seq = %u nlmsg_pid = %u nlmsg_flags = %B",
                             nlHdr.h->nlmsg_type, nlHdr.h->nlmsg_seq, nlHdr.h->nlmsg_pid, nlHdr.h->nlmsg_flags)

            if (nlHdr.h->nlmsg_type == NLMSG_DONE) break;
            else
               {
               // Else move the pointer to buffer appropriately

               bufPtr += readLen;
               msgLen += readLen;
               }

            // Check if it is a multi part message

            if ((nlHdr.h->nlmsg_flags & NLM_F_MULTI) == 0) break;
            }
         while ((nlHdr.h->nlmsg_seq != 1) || (nlHdr.h->nlmsg_pid != (uint32_t)u_pid));

         U_INTERNAL_DUMP("msgLen = %u readLen = %d", msgLen, readLen)

         // Parse the response

         int rtLen;
         char* dst;
         char dstMask[32];
         struct rtattr* rtAttr;
         char ifName[IF_NAMESIZE];
         struct in_addr dstAddr, srcAddr, gateWay;

         for (; NLMSG_OK(nlMsg.h,msgLen); nlMsg.h = NLMSG_NEXT(nlMsg.h,msgLen))
            {
            struct rtmsg* rtMsg = (struct rtmsg*) NLMSG_DATA(nlMsg.h);

            U_INTERNAL_DUMP("rtMsg = %p msgLen = %u rtm_family = %u rtm_table = %u", rtMsg, msgLen, rtMsg->rtm_family, rtMsg->rtm_table)

            /*
            #define AF_INET   2 // IP protocol family
            #define AF_INET6 10 // IP version 6
            */

            if ((rtMsg->rtm_family != AF_INET)) continue; // If the route is not for AF_INET then continue

            /* Reserved table identifiers

            enum rt_class_t {
               RT_TABLE_UNSPEC=0,
               RT_TABLE_COMPAT=252,
               RT_TABLE_DEFAULT=253,
               RT_TABLE_MAIN=254,
               RT_TABLE_LOCAL=255,
               RT_TABLE_MAX=0xFFFFFFFF }; */

            if ((rtMsg->rtm_table != RT_TABLE_MAIN)) continue; // If the route does not belong to main routing table then continue

            ifName[0] = '\0';
            dstAddr.s_addr = srcAddr.s_addr = gateWay.s_addr = 0;

            // get the rtattr field

            rtAttr = (struct rtattr*) RTM_RTA(rtMsg);
            rtLen  = RTM_PAYLOAD(nlMsg.h);

            for (; RTA_OK(rtAttr,rtLen); rtAttr = RTA_NEXT(rtAttr,rtLen))
               {
               U_INTERNAL_DUMP("rtAttr = %p rtLen = %u rta_type = %u rta_len = %u", rtAttr, rtLen, rtAttr->rta_type, rtAttr->rta_len)

               /* Routing message attributes

               struct rtattr {
                  unsigned short rta_len;  // Length of option
                  unsigned short rta_type; //   Type of option
               // Data follows
               };

               enum rtattr_type_t {
                  RTA_UNSPEC,    // 0
                  RTA_DST,       // 1
                  RTA_SRC,       // 2
                  RTA_IIF,       // 3
                  RTA_OIF,       // 4
                  RTA_GATEWAY,   // 5
                  RTA_PRIORITY,  // 6
                  RTA_PREFSRC,   // 7
                  RTA_METRICS,   // 8
                  RTA_MULTIPATH, // 9
                  RTA_PROTOINFO, // no longer used
                  RTA_FLOW,      // 11
                  RTA_CACHEINFO, // 12
                  RTA_SESSION,   // no longer used
                  RTA_MP_ALGO,   // no longer used
                  RTA_TABLE,     // 15
                  RTA_MARK,      // 16
                  __RTA_MAX }; */

               switch (rtAttr->rta_type)
                  {
                  case RTA_OIF:     (void) if_indextoname(*(unsigned*)RTA_DATA(rtAttr), ifName);   break;
                  case RTA_GATEWAY: U_MEMCPY(&gateWay, RTA_DATA(rtAttr), sizeof(struct in_addr)); break;
                  case RTA_PREFSRC: U_MEMCPY(&srcAddr, RTA_DATA(rtAttr), sizeof(struct in_addr)); break;
                  case RTA_DST:     U_MEMCPY(&dstAddr, RTA_DATA(rtAttr), sizeof(struct in_addr)); break;
                  }
               }

            U_DUMP("ifName = %S dstAddr = %S rtMsg->rtm_dst_len = %u srcAddr = %S gateWay = %S", ifName,
                        UIPAddress::toString(dstAddr.s_addr).data(), rtMsg->rtm_dst_len,
                        UIPAddress::toString(srcAddr.s_addr).data(),
                        UIPAddress::toString(gateWay.s_addr).data())

            dst = U_SYSCALL(inet_ntoa, "%u", dstAddr);

            if (u__snprintf(dstMask, sizeof(dstMask), U_CONSTANT_TO_PARAM("%s/%u"), dst, rtMsg->rtm_dst_len) == network_len &&
                    strncmp(dstMask, network, network_len) == 0)
               {
               if (gateWay.s_addr)
                  {
                  (void) U_SYSCALL(inet_ntop, "%d,%p,%p,%u", AF_INET, &gateWay, result.data(), result.capacity());
                  }
               else
                  {
                  U_INTERNAL_ASSERT_MAJOR(srcAddr.s_addr, 0)

                  (void) U_SYSCALL(inet_ntop, "%d,%p,%p,%u", AF_INET, &srcAddr, result.data(), result.capacity());
                  }

               result.size_adjust();

               break;
               }
            }
         }
      }
#endif

   U_RETURN_STRING(result);
}
