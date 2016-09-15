// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    pop3.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/client/pop3.h>
#include <ulib/utility/socket_ext.h>

#define U_POP3_OK    "+OK"
#define U_POP3_ERR   "-ERR"
#define U_POP3_EOD   ".\r\n"
#define U_POP3_EODML "\r\n.\r\n"

/*
#define TEST
*/

#ifdef TEST
#  include <ulib/file.h>
#endif

U_NO_EXPORT void UPop3Client::setStatus()
{
   U_TRACE_NO_PARAM(0, "UPop3Client::setStatus()")

   const char* descr1;
   const char* descr2;

   switch (state)
      {
      case INIT:                 descr1 = "INIT";           break;
      case AUTHORIZATION:        descr1 = "AUTHORIZATION";  break;
      case TRANSACTION:          descr1 = "TRANSACTION";    break;
      case UPDATE:               descr1 = "UPDATE";         break;
      default:                   descr1 = "???";            break;
      }

   switch (response)
      {
      case OK:                   descr2 = "OK";                      break;
      case BAD_STATE:            descr2 = "bad state";               break;
      case UNAUTHORIZED:         descr2 = "not authenticated";       break;
      case CANT_LIST:            descr2 = "LIST error";              break;
      case NO_SUCH_MESSAGE:      descr2 = "no such message";         break;
      case CAPA_NOT_SUPPORTED:   descr2 = "CAPA not supported";      break;
      case STLS_NOT_SUPPORTED:   descr2 = "STARTTLS not supported";  break;
      case UIDL_NOT_SUPPORTED:   descr2 = "UIDL not supported";      break;
      default:                   descr2 = "???";                     break;
      }

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%s - (%d, %s)"), descr1, response, descr2);
}

bool UPop3Client::_connectServer(const UString& server, unsigned int port, int timeoutMS)
{
   U_TRACE(0, "UPop3Client::_connectServer(%V,%u,%d)", server.rep, port, timeoutMS)

   if (Socket::connectServer(server, port, timeoutMS) &&
       (USocketExt::readLineReply(this, buffer), memcmp(buffer.data(), U_CONSTANT_TO_PARAM(U_POP3_OK)) == 0))
      {
      state    = AUTHORIZATION;
      response = OK;

#  ifdef DEBUG
      setStatus();

      U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

      u_buffer_len = 0;
#  endif

      /*
      if (timeoutMS &&
          USocket::isBlocking())
         {
         USocket::setNonBlocking(); // setting socket to nonblocking
         }
      */

      U_RETURN(true);
      }

   U_RETURN(false);
}

// Send a command to the POP3 server and wait for a response eventually with eod or size fixed...

U_NO_EXPORT bool UPop3Client::syncCommand(int eod, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UPop3Client::syncCommand(%d,%.*S,%u)", eod, fmt_size, format, fmt_size)

#ifdef DEBUG
   setStatus();

   U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

   u_buffer_len = 0;

   U_ASSERT(buffer.uniq())
#endif

   va_list argp;
   va_start(argp, fmt_size);

   buffer.rep->_length = end = USocketExt::vsyncCommand(this, buffer.data(), buffer.capacity(), format, fmt_size, argp);

   va_end(argp);

   if (memcmp(buffer.data(), U_CONSTANT_TO_PARAM(U_POP3_OK)) != 0) U_RETURN(false);

   if (eod != -1)
      {
      pos = buffer.find('\n') + 1;

      if (eod)
         {
         eod += (U_CONSTANT_SIZE(U_POP3_EOD) - (buffer.size() - pos)); // adjust how many bytes read...  

         if (eod > 0 &&
             USocketExt::read(this, buffer, eod) == false)
            {
            U_RETURN(false);
            }

         end = buffer.size() - U_CONSTANT_SIZE(U_POP3_EOD);
         }
      else
         {
         end = U_STRING_FIND(buffer, pos, U_POP3_EOD);

         if (end == (int)U_NOT_FOUND)
            {
            end = USocketExt::readWhileNotToken(this, buffer, U_CONSTANT_TO_PARAM(U_POP3_EOD));

            if (end == (int)U_NOT_FOUND) U_RETURN(false);
            }
         }

      U_INTERNAL_DUMP("pos = %d end = %d", pos, end)
      }

   response = OK;

   U_RETURN(true);
}

U_NO_EXPORT bool UPop3Client::syncCommandML(const UString& req, int* vpos, int* vend)
{
   U_TRACE(1, "UPop3Client::syncCommandML(%V,%p,%p)", req.rep, vpos, vend)

#ifdef DEBUG
   setStatus();

   U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

   u_buffer_len = 0;
#endif

   uint32_t sz = req.size();

   end = Socket::send(req.data(), sz);

   if (USocket::checkIO(end) == false) U_RETURN(false);

   int i = 0;

   pos = end = 0;
   buffer.setEmpty();

   if (vpos)
      {
      (void) U_SYSCALL(memset, "%p,%d,%u", vpos, 0xff, num_msg * sizeof(int));
      (void) U_SYSCALL(memset, "%p,%d,%u", vend, 0xff, num_msg * sizeof(int));
      }

   do {
      if ( USocketExt::read(this, buffer) == false) U_RETURN(false);

      U_INTERNAL_DUMP("buffer.size() = %u", buffer.size())

      do {
         if (buffer.size() < (uint32_t)(end + U_CONSTANT_SIZE(U_POP3_OK))) break;

      // end = U_STRING_FIND(buffer, end, U_POP3_OK);

         if (vpos)
            {
            if (vpos[i] == (int)U_NOT_FOUND)
               {
               vpos[i] = buffer.find('\n', end);

               if (vpos[i] == (int)U_NOT_FOUND) break;

               vpos[i] += 1;
               }

            vend[i] = U_STRING_FIND(buffer, end, U_POP3_EODML);

            U_INTERNAL_DUMP("vpos[%i] = %d vend[%i] = %d", i, vpos[i], i, vend[i])

            if (vend[i] == (int)U_NOT_FOUND) break;

            vend[i] += U_CONSTANT_SIZE(U_CRLF);

            end = vend[i] + U_CONSTANT_SIZE(U_POP3_EOD);
            }
         else
            {
            pos = buffer.find('\n', end);

            if (pos == (int)U_NOT_FOUND) break;

            end = ++pos;
            }

         if (++i == num_msg)
            {
#        ifdef TEST
            UFile::writeTo("pop3.data", buffer);
#        endif

            U_INTERNAL_ASSERT_EQUALS((uint32_t)end, buffer.size())

            goto end;
            }
         }
      while (true);
      }
   while (true);

end:
   response = OK;

   U_RETURN(true);
}

bool UPop3Client::startTLS()
{
   U_TRACE_NO_PARAM(0, "UPop3Client::startTLS()")

#ifdef USE_LIBSSL
   U_ASSERT(Socket::isSSL())

        if (state != AUTHORIZATION) response = BAD_STATE;
   else if (syncCommand(-1, U_CONSTANT_TO_PARAM("STLS")))
      {
      if (((USSLSocket*)this)->secureConnection()) U_RETURN(true);

      if ((USocketExt::readLineReply(this, buffer), memcmp(buffer.data(), U_CONSTANT_TO_PARAM(U_POP3_ERR)) == 0))
         {
         response = STLS_NOT_SUPPORTED;

#     ifdef DEBUG
         setStatus();

         U_INTERNAL_DUMP("status() = %.*S", u_buffer_len, u_buffer)

         u_buffer_len = 0;
#     endif
         }
      }
#endif

   U_RETURN(false);
}

bool UPop3Client::login(const char* user, const char* passwd)
{
   U_TRACE(0, "UPop3Client::login(%S,%S)", user, passwd)

   if (state != AUTHORIZATION) response = BAD_STATE;
   else
      {
      if (syncCommand(-1, U_CONSTANT_TO_PARAM("USER %s"), user) &&
          syncCommand(-1, U_CONSTANT_TO_PARAM("PASS %s"), passwd))
         {
         state = TRANSACTION;

         U_RETURN(true);
         }

      response = UNAUTHORIZED;
      }

   U_RETURN(false);
}

int UPop3Client::getCapabilities(UVector<UString>& vec)
{
   U_TRACE(0, "UPop3Client::getCapabilities(%p)", &vec)

   if (syncCommand(0, U_CONSTANT_TO_PARAM("CAPA")))
      {
      (void) capa.replace(0, capa.size(), buffer, pos, end - pos);

      uint32_t n = vec.split(capa, U_CRLF);

      U_RETURN(n);
      }

   response = CAPA_NOT_SUPPORTED;

   U_RETURN(-1);
}

int UPop3Client::getUIDL(UVector<UString>& vec)
{
   U_TRACE(0, "UPop3Client::getUIDL(%p)", &vec)

   if (state != TRANSACTION)
      {
      response = BAD_STATE;
      }
   else if (syncCommand(0, U_CONSTANT_TO_PARAM("UIDL")))
      {
      UString r;
      const char* p;
      uint32_t  n      = vec.size();
      const char* s    = buffer.c_pointer(pos);
      const char* _end = buffer.c_pointer(end);

      while (s < _end)
         {
         // skip white space

         if (u__isspace(*s))
            {
            ++s;

            continue;
            }

         s = u_delimit_token(s, &p, _end, 0, 0); // n-esimo
         s = u_delimit_token(s, &p, _end, 0, 0); // uidl

         r = UString((void*)p, s - p);

         vec.push(r);

         ++s;
         }

      n = vec.size() - n;

      U_RETURN(n);
      }
   else
      {
      response = UIDL_NOT_SUPPORTED;
      }

   U_RETURN(-1);
}

int UPop3Client::getSizeMessage(uint32_t n)
{
   U_TRACE(0, "UPop3Client::getSizeMessage(%u)", n)

   if (state == TRANSACTION)
      {
      if ((n ? syncCommand(-1, U_CONSTANT_TO_PARAM("LIST %u"), n) :
               syncCommand(-1, U_CONSTANT_TO_PARAM("STAT"))))
         {
         char* ptr = buffer.c_pointer(sizeof(U_POP3_OK));

         num_msg      = strtol(ptr, (char**)&ptr, 10);
         int size_msg = atoi(ptr);

         U_INTERNAL_DUMP("num_msg = %d size_msg = %d", num_msg, size_msg)

         U_RETURN(size_msg);
         }

      response = CANT_LIST;
      }
   else
      {
      response = BAD_STATE;
      }

   U_RETURN(-1);
}
 
// Execute an pop3 session

UString UPop3Client::getHeader(uint32_t n)
{
   U_TRACE(0, "UPop3Client::getHeader(%u)", n)

   if (state == TRANSACTION)
      {
      if (syncCommand(0, U_CONSTANT_TO_PARAM("TOP %u 0"), n))
         {
         UString result((void*)buffer.c_pointer(pos), end - pos);

         U_RETURN_STRING(result);
         }

      response = NO_SUCH_MESSAGE;
      }
   else
      {
      response = BAD_STATE;
      }

   return UString::getStringNull();
}

UString UPop3Client::getMessage(uint32_t n)
{
   U_TRACE(0, "UPop3Client::getMessage(%u)", n)

   int size_msg = getSizeMessage(n);

   if (size_msg > 0)
      {
      (void) buffer.reserve(size_msg);

      if (syncCommand(size_msg, U_CONSTANT_TO_PARAM("RETR %u"), n))
         {
         UString result((void*)buffer.c_pointer(pos), size_msg);

         U_RETURN_STRING(result);
         }

      response = NO_SUCH_MESSAGE;
      }

   return UString::getStringNull();
}

bool UPop3Client::deleteMessage(uint32_t n)
{
   U_TRACE(0, "UPop3Client::deleteMessage(%u)", n)

   if (state == TRANSACTION)
      {
      if (syncCommand(-1, U_CONSTANT_TO_PARAM("DELE %u"), n)) U_RETURN(true);

      response = NO_SUCH_MESSAGE;
      }
   else
      {
      response = BAD_STATE;
      }

   U_RETURN(false);
}

// PIPELINING

int UPop3Client::getAllHeader(UVector<UString>& vec)
{
   U_TRACE(0, "UPop3Client::getAllHeader(%p)", &vec)

   int size_msg = getSizeMessage(0);

   if (size_msg > 0)
      {
      int i = 2;
      int vpos[8192], vend[8192];
      UString req(U_max(U_CAPACITY, 11U * num_msg));

                                req.snprintf(    U_CONSTANT_TO_PARAM("TOP  1 0\r\n"));
      for (; i <= num_msg; ++i) req.snprintf_add(U_CONSTANT_TO_PARAM("TOP %d 0\r\n"), i);

      (void) req.shrink();

      (void) buffer.reserve(size_msg);

      if (syncCommandML(req, vpos, vend))
         {
         UString str;

         for (i = 0; i < num_msg; ++i)
            {
            str = UString((void*)buffer.c_pointer(vpos[i]), vend[i] - vpos[i]);

            vec.push(str);
            }

         U_RETURN(num_msg);
         }

      response = CANT_LIST;
      }

   U_RETURN(-1);
}

int UPop3Client::getAllMessage(UVector<UString>& vec)
{
   U_TRACE(0, "UPop3Client::getAllMessage(%p)", &vec)

   int size_msg = getSizeMessage(0);

   if (size_msg > 0)
      {
      int i = 2;
      int vpos[8192], vend[8192];
      UString req(U_max(U_CAPACITY, 10U * num_msg));

                                req.snprintf(    U_CONSTANT_TO_PARAM("RETR  1\r\n"));
      for (; i <= num_msg; ++i) req.snprintf_add(U_CONSTANT_TO_PARAM("RETR %d\r\n"), i);

      (void) req.shrink();

      (void) buffer.reserve(size_msg + (num_msg * (sizeof(U_POP3_OK) + sizeof("Message follows") + sizeof(U_POP3_EODML))));

      if (syncCommandML(req, vpos, vend))
         {
         UString str;

         for (i = 0; i < num_msg; ++i)
            {
            str = UString((void*)buffer.c_pointer(vpos[i]), vend[i] - vpos[i]);

            vec.push(str);
            }

         U_RETURN(num_msg);
         }

      response = CANT_LIST;
      }

   U_RETURN(-1);
}

bool UPop3Client::deleteAllMessage()
{
   U_TRACE_NO_PARAM(0, "UPop3Client::deleteAllMessage()")

   int size_msg = getSizeMessage(0);

   if (size_msg > 0)
      {
      int i = 2;
      UString req(U_max(U_CAPACITY, 10U * num_msg));
      uint32_t size = num_msg * (sizeof(U_POP3_OK) + sizeof(" message deleted"));

                                req.snprintf(    U_CONSTANT_TO_PARAM("DELE  1\r\n"));
      for (; i <= num_msg; ++i) req.snprintf_add(U_CONSTANT_TO_PARAM("DELE %d\r\n"), i);

      (void) req.shrink();

      (void) buffer.reserve(size);

      if (syncCommandML(req, 0, 0)) U_RETURN(true);
      }

   U_RETURN(false); 
}

bool UPop3Client::reset()
{
   U_TRACE_NO_PARAM(0, "UPop3Client::reset()")

   if (syncCommand(-1, U_CONSTANT_TO_PARAM("RSET")))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

// Quit an pop3 session

bool UPop3Client::quit()
{
   U_TRACE_NO_PARAM(0, "UPop3Client::quit()")

   if (syncCommand(-1, U_CONSTANT_TO_PARAM("QUIT")))
      {
      state = UPDATE;

      U_RETURN(true);
      }

   U_RETURN(false);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UPop3Client::dump(bool _reset) const
{
   Socket::dump(false);

   *UObjectIO::os << '\n'
                  << "pos                           " << pos             << '\n'
                  << "end                           " << end             << '\n'
                  << "state                         " << state           << '\n'
                  << "num_msg                       " << num_msg         << '\n'
                  << "response                      " << response        << '\n'
                  << "capa            (UString      " << (void*)&capa    << ")\n"
                  << "buffer          (UString      " << (void*)&buffer  << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
