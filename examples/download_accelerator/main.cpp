// main.cpp

#include <ulib/base/utility.h>

#include <ulib/url.h>
#include <ulib/file.h>
#include <ulib/timer.h>
#include <ulib/process.h>
#include <ulib/notifier.h>
#include <ulib/net/client/ftp.h>
#include <ulib/net/client/http.h>
#include <ulib/utility/interrupt.h>

#undef  PACKAGE
#define PACKAGE "download_accelerator"
#undef  ARGS
#define ARGS "<url>"

#define U_OPTIONS \
"purpose \"download url file specified with arg <url>...\"\n"

#include <ulib/application.h>

#include <new>

#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#ifndef __MINGW32__
#  include <sys/uio.h>
#endif

#define U_APPEND_LITERAL(ptr,str) (void) apex_memcpy(ptr, str, U_CONSTANT_SIZE(str)); ptr += U_CONSTANT_SIZE(str)

/* currently-known information about a host */

typedef struct {
   bool done;                          /* if host testing is done */
   bool invalid;                       /* if we discard this host */

   unsigned num_out, num_in;           /* packets sent/received successfully */
   unsigned total_lag;                 /* combined lag on ALL received messages */
   int hops_less_than, hops_more_than; /* for guessing number of hops */

   int retries;                        /* transaction in progress */
   struct timeval send_time;           /* time of transmission */
   u_short seq;                        /* sequence number sent with packet */

   Url url;
   UIPAddress ip;                      /* remote address */
   UString server, path;

   int fd;
   char* ptr;
   off_t offset;
   size_t size, bytes_read;
} HostData;

static HostData* place;

/* Download progress - "Thermometer" (bar) progress
   The progress bar should look like this:

   xx% [=======>             ] nn,nnn 12.34K/s ETA 00:00

   Calculate the geometry. The idea is to assign as much room as possible to the progress bar.
   The other idea is to never let things "jitter", i.e. pad elements that vary in size so that
   their variance does not affect the placement of other elements.
   It would be especially bad for the progress bar to be resized randomly.

   "xx% " or "100%"  - percentage               - 4 chars
   "[]"              - progress bar decorations - 2 chars
   " nnn,nnn,nnn"    - downloaded bytes         - 12 chars or very rarely more
   " 1012.56K/s"     - dl rate                  - 11 chars
   " ETA xx:xx:xx"   - ETA                      - 13 chars

   "=====>..."       - progress bar             - the rest
*/

class ProgressBar : public UEventTime {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

   ProgressBar(long sec, long usec) : UEventTime(sec, usec)
      {
      U_TRACE_CTOR(5, ProgressBar, "%ld,%ld", sec, usec)
      }

   ~ProgressBar()
      {
      U_TRACE_DTOR(5, ProgressBar)
      }

   /* Set a progress gauge. INITIAL is the number of bytes the download starts from
      (zero if the download starts from scratch). TO_DOWNLOAD is the expected total number of bytes in this download.
   */

   void set(off_t initial, size_t to_download)
      {
      U_TRACE(5, "ProgressBar::set(%I,%lu)", initial, to_download)

      total_length     = initial + to_download;
      initial_length   = initial;
      size_legible_len = u__snprintf(size_legible, sizeof(size_legible), U_CONSTANT_TO_PARAM("%lu"), total_length);

      screen_width = u_getScreenWidth();

      if (screen_width == 0) screen_width = 80;

      U_INTERNAL_DUMP("screen_width = %d", screen_width)

      count         = 0;
      howmuch       = 0;
      dl_total_time = 0;
      }

   void display()
      {
      U_TRACE(5, "ProgressBar::display()")

      size_t size = initial_length + count;

      U_INTERNAL_DUMP("size = %lu  count = %lu", size, count)

      U_INTERNAL_ASSERT(size <= total_length)

      size_legible_len = u__snprintf(size_legible, sizeof(size_legible), U_CONSTANT_TO_PARAM("%lu"), size);

      unsigned dlbytes_size  = 1 + U_max(size_legible_len, 13),
               progress_size = screen_width - (4 + 2 + dlbytes_size + 11 + 13);

      U_INTERNAL_DUMP("size_legible_len = %u dlbytes_size = %u progress_size = %u",
                        size_legible_len, dlbytes_size, progress_size)

      U_INTERNAL_ASSERT(progress_size > 5)

      char* p = buffer;

      /* "xx% " */

      unsigned i, percentage = (unsigned)(100.0 * size / total_length);

      U_INTERNAL_ASSERT(percentage <= 100)

      if (percentage < 100) sprintf(p, "%2u%% ", percentage);
      else                  U_MEMCPY(p, "100%", U_CONSTANT_SIZE("100%"));

      p += 4;

      /* The progress bar: "[====>      ]" or "[++==>      ]" */

      unsigned insz = (unsigned)((double)initial_length / (double)total_length * progress_size), // Size initial portion
               dlsz = (unsigned)((double)          size / (double)total_length * progress_size); // Size downloaded portion

      U_INTERNAL_ASSERT(insz <= dlsz)
      U_INTERNAL_ASSERT(dlsz <= progress_size)

      *p++ = '[';

      char* begin = p;

      for (i = 0; i < insz; i++) *p++ = '+'; // Print the initial portion of the download with '+' chars,
                                             // the rest with '=' and one '>'

      dlsz -= insz;

      if (dlsz > 0)
         {
         for (i = 0; i < dlsz - 1; i++) *p++ = '=';

         *p++ = '>';
         }

      while ((p - begin) < (int)progress_size) *p++ = ' ';

      *p++ = ']';

   // U_INTERNAL_DUMP("p - buffer = %u", p - buffer)

      /* "2,234,567,890" */

      (void) sprintf(p, " %-13s", size_legible);

      p += u__strlen(p, __PRETTY_FUNCTION__);

   // U_INTERNAL_DUMP("p - buffer = %u", p - buffer)

      /* " 1012.45K/s" */

      U_INTERNAL_DUMP("howmuch = %u", howmuch)

      if (howmuch)
         {
         double dlspeed = u_calcRate(howmuch, dltime, &units);

         (void) sprintf(p, " %7.2f%s/s", dlspeed, u_short_units[units]);

         p += u__strlen(p, __PRETTY_FUNCTION__);

      // U_INTERNAL_DUMP("p - buffer = %u", p - buffer)

         /* Calculate ETA using the average download speed to predict the future speed */

         if (howmuch == total_length) goto no_eta;

         unsigned eta_hrs, eta_min, eta_sec;

         double time_sofar      = (double)dl_total_time / 1000.;
         size_t bytes_remaining = total_length - size;
         unsigned eta           = (unsigned)((double) time_sofar * bytes_remaining / (double) count);

         eta_hrs = eta / 3600, eta %= 3600;
         eta_min = eta / 60,   eta %= 60;
         eta_sec = eta;

         U_INTERNAL_DUMP("eta_hrs = %u eta_min = %u eta_sec = %u", eta_hrs, eta_min, eta_sec)

         if (eta_hrs > 99) goto no_eta;

         if (eta_hrs == 0)
            {
            /* Hours not printed: pad with three spaces */

            U_APPEND_LITERAL("   ");

            (void) sprintf(p, " ETA %02u:%02u", eta_min, eta_sec);
            }
         else
            {
            /* Hours printed with one digit: pad with one space */

            if (eta_hrs < 10) *p++ = ' ';

            (void) sprintf(p, " ETA %u:%02u:%02u", eta_hrs, eta_min, eta_sec);
            }

         p += u__strlen(p, __PRETTY_FUNCTION__);
         }
      else
         {
         U_APPEND_LITERAL("   --.--K/s");
no_eta:
         U_APPEND_LITERAL("             ");
         }

   // U_INTERNAL_DUMP("p - buffer = %u", p - buffer)

      while (p < (buffer + screen_width)) *p++ = ' ';

      *p = '\0';

      struct iovec iov[2] = { { (caddr_t)"\r", 1 },
                              { (caddr_t)buffer, screen_width } };

      (void) UFile::writev(STDOUT_FILENO, iov, 2); 

      U_INTERNAL_ASSERT((p - buffer) <= screen_width)
      }

   void start()
      {
      U_TRACE(5, "ProgressBar::start()")

      struct iovec iov[3] = { { (caddr_t)U_CONSTANT_TO_PARAM(" \nStart download file (") },
                              { (caddr_t)size_legible, size_legible_len },
                              { (caddr_t)U_CONSTANT_TO_PARAM(" bytes)\n") } };

      (void) UFile::writev(STDOUT_FILENO, iov, 3); 

      UTimer::init(UTimer::SYNC);

      UTimer::insert(this);

      UTimer::setTimer();

      time = *u_now;

      display();
      }

   void progress()
      {
      U_TRACE(5, "ProgressBar::progress()")

      u_gettimenow();

      dltime         = (u_now->tv_sec  -   time.tv_sec)  * 1000 +
                       (u_now->tv_usec -   time.tv_usec) / 1000;
      dl_total_time  = (u_now->tv_sec  - _start.tv_sec)  * 1000 +
                       (u_now->tv_usec - _start.tv_usec) / 1000;

      U_INTERNAL_DUMP("dltime = %ld dl_total_time = %ld", dltime, dl_total_time)
      }

   void end()
      {
      U_TRACE(5, "ProgressBar::end()")

      UTimer::erase(this);

      howmuch = count = (total_length - initial_length);

      progress();

      dltime = dl_total_time;

      display();

      (void) UFile::write(STDOUT_FILENO, U_CONSTANT_TO_PARAM("\n")); 

      delete this;
      }

   virtual int handlerTime()
      {
      U_TRACE(5, "ProgressBar::handlerTime()")

      long last_count = count;

      count = place->bytes_read;

      howmuch = count - last_count;

      progress();

      time = *u_now;

      display(); /* Inform the progress gauge of newly received bytes */

      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(0);
      }

#ifdef DEBUG
   const char* dump(bool reset) const
      {
      *UObjectIO::os << "units           " << units          << '\n'
                     << "count           " << count          << '\n'
                     << "dltime          " << dltime         << '\n'
                     << "howmuch         " << howmuch        << '\n'
                     << "time.tv_sec     " << time.tv_sec    << '\n'
                     << "time.tv_usec    " << time.tv_usec   << '\n'
                     << "screen_width    " << screen_width   << '\n'
                     << "total_length    " << total_length   << '\n'
                     << "initial_length  " << initial_length;

      if (reset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return 0;
      }
#endif

private:
   size_t count;                       /* total bytes downloaded so far */
   char* buffer[1024];                 /* buffer where the bar "image" is stored */
   long dl_total_time;                 /* time measured since the beginning of download */
   size_t total_length;                /* expected total byte count when the download finishes */
   off_t initial_length;               /* how many bytes have been downloaded previously */
   unsigned  howmuch, dltime;          /* bytes and time downloaded so far */
   int size_legible_len;
   char size_legible[32];
   int units, screen_width;
   struct timeval _start, time;        /* last screen update */
};

typedef struct {
   struct ip ip;
   struct udphdr udp;
   u_char seq;             /* sequence number of this packet */
   u_char ttl;             /*  ttl packet left with */
   struct timeval tv;      /* time packet left */
} OPacket;                 /* format of a (udp) probe packet */

static OPacket outpacket;  /* last output (udp) packet */

class Application : public UApplication {
public:

   bool sendProbe()
      {
      U_TRACE(5, "Application::sendProbe()")

      U_INTERNAL_ASSERT_POINTER(host)

      ip = &outpacket.ip;

      U_INTERNAL_ASSERT(ip->ip_v    == IPVERSION)
      U_INTERNAL_ASSERT(ip->ip_p    == IPPROTO_UDP)
      U_INTERNAL_ASSERT(ip->ip_hl   == sizeof(*ip) >> 2)
      U_INTERNAL_ASSERT(ip->ip_off  == 0)

      struct udphdr* up = &outpacket.udp;

      U_INTERNAL_ASSERT(up->len    == htons((u_short)(sizeof(OPacket) - sizeof(struct ip))))
      U_INTERNAL_ASSERT(up->check  == 0)
      U_INTERNAL_ASSERT(up->source == htons(ident))

      U_MEMCPY(&ip->ip_dst, host->ip.get_in_addr(), host->ip.getInAddrLength()); /* remote address */

      outpacket.tv  = host->send_time;
      outpacket.seq = host->seq;
      outpacket.ttl = ip->ip_ttl = chooseTTL();

   // ip->ip_off = 0;
   // ip->ip_p   = IPPROTO_UDP;
   // ip->ip_v   = IPVERSION;
   // ip->ip_hl  = sizeof(struct ip) >> 2;

      ip->ip_id  = htons(ident + host->seq);
      ip->ip_len = 0; /* kernel fills this in */

   // up->len    = htons((u_short)(sizeof(OPacket) - sizeof(struct ip)));
   // up->check  = 0;
   // up->source = htons(ident);

      int dest = port + host->seq;
      up->dest = htons(dest);

      int result = sndsock.sendTo(&outpacket, sizeof(OPacket), 0, host->ip, dest);

      if (result != sizeof(OPacket))
         {
         if (result < 0)
            {
            switch (errno)
               {
               case ENETDOWN:
               case ENETUNREACH:
               case EHOSTDOWN:
               case EHOSTUNREACH: host->invalid = true; break;

               default: U_RETURN(false);
               }
            }
         }

      U_RETURN(true);
      }

   void deltaT()
      {
      U_TRACE(5, "Application::deltaT()")

      U_INTERNAL_ASSERT_POINTER(host)

      delta_time = ((host->send_time.tv_sec  != 0 ||
                     host->send_time.tv_usec != 0)
                        ? (u_now->tv_sec  - host->send_time.tv_sec)  * 1000 +
                          (u_now->tv_usec - host->send_time.tv_usec) / 1000
                        : 0);

      U_INTERNAL_DUMP("delta_time = %ld", delta_time)
      }

   int checkPacket(u_char* buf, int cc)
      {
      U_TRACE(5, "Application::checkPacket(%p,%d)", buf, cc)

      U_INTERNAL_ASSERT_POINTER(host)

      ip = (struct ip*) buf;

      int hlen = ip->ip_hl << 2;

      if (cc >= hlen + ICMP_MINLEN)
         {
         struct icmp* icp = (struct icmp*)(buf + hlen);

         u_char type = icp->icmp_type,
                code = icp->icmp_code;

         if ((type == ICMP_TIMXCEED &&
              code == ICMP_TIMXCEED_INTRANS) ||
              type == ICMP_UNREACH)
            {
            cc -= hlen;

            ip   = &icp->icmp_ip;
            hlen = ip->ip_hl << 2;

            struct udphdr* up = (struct udphdr*) ((u_char*)ip + hlen);

            for (i = 0, host = place; i < numhosts; ++i, ++host)
               {
               if (host->invalid) continue;

               if (hlen + 12 <= cc            &&
                   ip->ip_p   == IPPROTO_UDP  &&
                   up->source == htons(ident) &&
                   up->dest   == htons(port + host->seq))
                  {
                  U_RETURN(type == ICMP_TIMXCEED ? -1 : code + 1); /* ICMP error code returned */
                  }
               }
            }
         }

      U_RETURN(255);
      }

   int waitForReply() /* ICMP error code returned */
      {
      U_TRACE(5, "Application::waitForReply()")

      UIPAddress cSourceIP;
      u_char inpacket[512];
      int code = 0, iSourcePortNumber, cc = rcvsock.recvFrom((void*)inpacket, 512, 0, cSourceIP, iSourcePortNumber);

      if (cc > 0) code = checkPacket(inpacket, cc);

      U_RETURN(code);
      }

   int chooseTTL()
      {
      U_TRACE(5, "Application::chooseTTL()")

      U_INTERNAL_ASSERT_POINTER(host)
      U_INTERNAL_ASSERT(host->invalid == false)

      /* converge upwards to hops_less_than -- manages rounding errors */

      int result = host->hops_less_than - (host->hops_less_than - host->hops_more_than) / 2;

      U_RETURN(result);
      }

   int hostScore()
      {
      U_TRACE(5, "Application::hostScore()")

      U_INTERNAL_ASSERT_POINTER(host)

      if (host->num_in)
         {
         int score = host->total_lag * host->num_out / host->num_in / host->num_in;
             score = score + (score * host->hops_less_than / 10);

         U_RETURN(score);
         }

      U_RETURN(99999); // rotten score
      }

   bool results()
      {
      U_TRACE(5+256, "Application::results()")

      HostData* lowest_host;
      unsigned n = numhosts, score;

      num_score = 0;

      while (n)
         {
         lowest_host  = 0;
         unsigned lowest_score = 99999;

         for (i = 0, host = place; i < numhosts; ++i, ++host)
            {
            if (host->invalid) continue;

            score = hostScore();

            if (score < lowest_score)
               {
               lowest_host  = host;
               lowest_score = score;
               }
            }

         if (lowest_host == 0) break;

         from[num_score++] = lowest_host;

         lowest_host->invalid = true; // skip this one next time

         --n;
         }

      U_RETURN(num_score > 0);
      }

   bool selectHost(unsigned min_tries)
      {
      U_TRACE(5+256, "Application::selectHost(%u)", min_tries)

      U_INTERNAL_ASSERT(numhosts > 0)

      if (u_is_tty) (void) UFile::write(STDOUT_FILENO, U_CONSTANT_TO_PARAM("Select hosts for download file "));

      int rcvsock_fd, sndsock_fd;

      if ((rcvsock_fd = USocket::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0 ||
          (sndsock_fd = USocket::socket(AF_INET, SOCK_RAW, IPPROTO_RAW))  < 0)
         {
         U_RETURN(false);
         }

      rcvsock.iSockDesc = rcvsock_fd;
      sndsock.iSockDesc = sndsock_fd;

      int code, lag;
      unsigned must_continue = numhosts;
      int seq = 0, endcount = 0, min_lag = 100;

      if (rcvsock.setTimeoutRCV(50) == false) U_RETURN(false); /* transmit time must be <= min_lag / 2 */

#  ifdef DEBUG
      const char* result = 0;
#  endif

      port  = 32768 + 666;                /* start udp dest port for probe packets */
      ident = (u_pid & 0xffff) | 0x8000;

      outpacket.ip.ip_v  = IPVERSION;
      outpacket.ip.ip_p  = IPPROTO_UDP;
      outpacket.ip.ip_hl = sizeof(struct ip) >> 2;

      outpacket.udp.len    = htons((u_short)(sizeof(OPacket) - sizeof(struct ip)));
      outpacket.udp.source = htons(ident);

      /* keep going until most of the hosts have been finished */

      u_gettimenow();

      while (must_continue &&
             must_continue >= numhosts/2)
         {
         /* start firing UDP packets with "random-guess" TTL values, much like traceroute does.
         Actually, the code for this is derived from traceroute. send out a packet; if there are
         no interesting packets to send out, make sure we only loop once through the list of hosts.
         Also make sure that next time we start the loop, we pick up at the host where we left off,
         rather than starting over; this increases fairness.
         */

         bool sent_one = false;
         int startcount = endcount;
         int hostcount = endcount;
         must_continue = 0;

         do {
            hostcount = (hostcount + 1) % numhosts;

            host = &place[hostcount];

            if (host->invalid) continue;

         // U_INTERNAL_DUMP("hostcount = %2d done = %d", hostcount, host->done)

            deltaT();

            if (host->done == false)
               {
               if (host->num_out < min_tries || delta_time < 5000) ++must_continue;
               else                                                host->done = true;
               }

            if (host->retries && delta_time < 3000) continue;

            if (host->retries < 3 &&
                host->num_out < min_tries)
               {
               if (sent_one == false)
                  {
#              ifdef DEBUG
                  if (host->retries >= 1) U_INTERNAL_DUMP("%-35s - %-12s %5u ms", host->server.data(), "TIMEOUT", delta_time)
#              endif

                  host->seq = ++seq;
                                seq %= 256;

                  host->num_out++;
                  host->retries++;

                  host->send_time = *u_now;

                  if (sendProbe() == false) U_RETURN(false);

                  U_INTERNAL_DUMP("%-35s - %-12s num_out = %2d  ttl = %d", host->server.data(),
                                                                           "SEND", host->num_out, outpacket.ttl)

                  sent_one = true;
                  endcount = hostcount;
                  }
               }
            else if (host->hops_less_than - host->hops_more_than > 2)
               {
               /* sometimes we get a TIMEOUT instead of an error if the ttl is too small; just move to the next one then */

               host->retries        = 0;
               host->hops_more_than = chooseTTL();
               }
            else
               {
               host->done = true;
               }
            }
         while (hostcount != startcount);

         code = waitForReply() - 1; /* ICMP error code returned */

         u_gettimenow();

         if (u_is_tty) (void) UFile::write(STDOUT_FILENO, U_CONSTANT_TO_PARAM(".")); 

         switch (code)
            {
            case -1:
            case 254:
               {
#           ifdef DEBUG
               if (code == 254) U_INTERNAL_DUMP("RECEIVED AN UNKNOWN PACKET")
#           endif

               continue;
               }
            break;

            case -2:
               {
               // if an "ICMP TTL Expired" message comes back, then the TTL was too low: the host is farther away than that.
               // Increase TTL next time.

#           ifdef DEBUG
               result = "HIGHER";
#           endif

               if (chooseTTL() >= host->hops_less_than) host->hops_less_than = chooseTTL() + 1;

               host->num_out--;
               host->retries = 0;
               host->hops_more_than = chooseTTL();
               }
            break;

            case ICMP_UNREACH_PORT: /* 3 bad port */
               {
               // Otherwise, a "Port Unreachable" message comes back, meaning the TTL was large enough.
               // Try a smaller one. We do this until we narrow down the TTL. (This is where the "binary search" comes in)

#           ifdef DEBUG
               result = "OK";
#           endif

               host->num_in++;
               host->retries = 0;
               host->hops_less_than = chooseTTL();

               deltaT();

               lag = delta_time;

               if (lag > 10 &&
                   lag < min_lag)
                  {
                  min_lag = lag;

                  if (rcvsock.setTimeoutRCV(min_lag / 2) == false) U_RETURN(false); // transmit time must be <= min_lag / 2
                  }

               host->total_lag += lag;
               }
            break;

         // case ICMP_UNREACH_NET:              /*  0 bad net */
         // case ICMP_UNREACH_HOST:             /*  1 bad host */
         // case ICMP_UNREACH_PROTOCOL:         /*  2 bad protocol */
         // case ICMP_UNREACH_NEEDFRAG:         /*  4 IP_DF caused drop */
         // case ICMP_UNREACH_SRCFAIL:          /*  5 src route failed */
         // case ICMP_UNREACH_NET_UNKNOWN:      /*  6 unknown net */
         // case ICMP_UNREACH_HOST_UNKNOWN:     /*  7 unknown host */
         // case ICMP_UNREACH_ISOLATED:         /*  8 src host isolated */
         // case ICMP_UNREACH_NET_PROHIB:       /*  9 net denied */
         // case ICMP_UNREACH_HOST_PROHIB:      /* 10 host denied */
         // case ICMP_UNREACH_TOSNET:           /* 11 bad tos for net */
         // case ICMP_UNREACH_TOSHOST:          /* 12 bad tos for host */
         // case ICMP_UNREACH_FILTER_PROHIB:    /* 13 admin prohib */
         // case ICMP_UNREACH_HOST_PRECEDENCE:  /* 14 host prec vio. */
         // case ICMP_UNREACH_PRECEDENCE_CUTOF: /* 15 prec cutoff */
            default:
               {
#           ifdef DEBUG
               result = "UNREACHABLE";
#           endif

               host->invalid = true;
               }
            break;
            }

#     ifdef DEBUG
         deltaT();

         U_INTERNAL_DUMP("%-35s - %-12s %5u ms - %-12s code = %2d", host->server.data(), "RECEIVED", delta_time, result, code)
#     endif
         }

   /* When all the hosts have had their TTL values narrowed down, and the minimum tries have expired, we're done.
      Close the sockets and dump the statistics.
   */

      rcvsock.close();
      sndsock.close();

#  ifdef DEBUG
      int score;

      for (i = 0, host = place; i < numhosts; ++i, ++host)
         {
         if (host->invalid) continue;

         if (host->num_in  == 0 ||
             host->num_out == 0)
            {
            U_INTERNAL_DUMP("%-35s 9999 ms %2d hops 0%%", host->server.data(), host->hops_less_than)
            }
         else
            {
            score = hostScore();

            U_INTERNAL_DUMP("%-35s %5u ms %2d hops %3d%% (%2d/%2d) [%5d]", host->server.data(), host->total_lag / host->num_in,
                              host->hops_less_than, host->num_in * 100 / host->num_out, host->num_in, host->num_out, score)
            }
         }
#  endif

      U_RETURN(true);
      }

   bool resolvNameHost()
      {
      U_TRACE(5, "Application::resolvNameHost()")

      if (numhosts > 1) mirror.sort();

      for (i = 0, host = place; i < numhosts; ++i, ++host)
         {
         U_INTERNAL_ASSERT(host->send_time.tv_sec  == 0 &&
                           host->send_time.tv_usec == 0)

         host->invalid = true;

         // check for duplicate server

         host->url.set(mirror[i]);

                       host->server = host->url.getHost();
         if (server == host->server) continue;
             server  = host->server;

         if (proc.fork() &&
             proc.child())
            {
            if (host->ip.setHostName(host->server, bIPv6))
               {
               host->invalid        = false;
               host->hops_less_than = max_ttl;
               }

            U_EXIT(0);
            }
         }

      proc.waitAll();

      int validhosts = 0;

      for (i = 0, host = place; i < numhosts; ++i, ++host)
         {
         if (host->invalid == false) ++validhosts;
         }

      U_INTERNAL_DUMP("numhosts = %u validhosts = %d", numhosts, validhosts)

      U_RETURN(validhosts > 0);
      }

   void setDataShared()
      {
      U_TRACE(5, "Application::setDataShared()")

      num = (numhosts > numproc ? numhosts : numproc);

      U_INTERNAL_DUMP("num = %u", num)

      map_size1 = num * sizeof(HostData);

      place = (HostData*) UFile::mmap(&map_size1);

      U_INTERNAL_ASSERT(place->bytes_read == 0)

      host = new((void*)place) HostData[num];

      U_INTERNAL_ASSERT(host == place)
      U_INTERNAL_ASSERT(host[num-1].server.invariant())

      map_size2 = num * sizeof(HostData*);

      from = (HostData**) UFile::mmap(&map_size2);
      }

   void download()
      {
      U_TRACE(5, "Application::download()")

      U_INTERNAL_ASSERT_POINTER(host)

      int fd       = host->fd;
      char* ptr    = host->ptr;
      size_t size  = host->size;
      off_t offset = host->offset;

      U_INTERNAL_ASSERT(fd != -1)

      U_INTERNAL_DUMP("fd = %d offset = %I size = %lu", fd, offset, size)

      ssize_t value;
      size_t bytes_read = 0;

      do {
         value = USocket::recv(fd, ptr + bytes_read, size - bytes_read);

         if (value == -1) break;

         place->bytes_read += value;
                bytes_read += value;

         U_INTERNAL_DUMP("bytes_read = %.8lu", bytes_read)
         }
      while (bytes_read < size);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage url file

      if (argv[optind] == NULL) U_ERROR("arg <url> not specified");

      Url url(argv[optind], u__strlen(argv[optind], __PRETTY_FUNCTION__));

      // create file dest

      server = url.getHost();
      path   = url.getPath();

      unsigned index = path.find_last_of('/');

      filename = (index ? path.substr(index + 1) : server);

      file.setPath(filename);

      if (file.creat() == false) U_ERROR("file creat failed");

      off_t offset = file.size();

      // ask for mirror file at filesearching.com

      bIPv6    = false;
      u_is_tty = isatty(STDOUT_FILENO);

      UHttpClient<UTCPSocket> http(0);

      if (u_is_tty) (void) UFile::write(STDOUT_FILENO, U_CONSTANT_TO_PARAM("Query to filesearching.com searcher for file to download\n"));

      numproc  = 4;
      UIPAddress ip_addr;
      numhosts = http.FTPFileSearch(filename, mirror);

      setDataShared();

      // select host for download: make sure at least 50% of the hosts get tested with many packets.
      // The more packets you use, the more accurate the results... and the longer it takes to run.
      // The default is 10, which is usually okay.

      unsigned min_tries = 10;

      max_ttl = 30; // maximum ttl. Don't accept hosts with more hops than this.

      if (numhosts              &&
          resolvNameHost()      &&
          selectHost(min_tries) &&
          results())
         {
         for (i = 0; i < num_score; ++i)
            {
            host = from[i];

            host->path    = host->url.getPath();
            host->done    = false;
            host->invalid = false;

            U_DUMP("%-35s", host->server.data())
            }

         if (num_score < numproc) numproc = num_score;
         }
      else
         {
         U_INTERNAL_ASSERT(index != 0); // check if url exist...

         if (ip_addr.setHostName(server, bIPv6) == false) U_ERROR("unknown host %.*S", U_STRING_TO_TRACE(server));

         for (i = 0, host = place; i < numproc; ++i, ++host)
            {
            host->ip      = ip_addr;
            host->path    = path;
            host->done    = false;
            host->invalid = false;

            from[i] = host;
            }

         num_score = numproc;
         }

      U_INTERNAL_DUMP("num_score = %u", num_score)

      UFtpClient ftp(bIPv6);

      // find the first usable server...

      for (i = 0; i < num_score; ++i)
         {
         host = from[i];

         host->fd = ftp.download(host->ip, host->path, offset);

         if (host->fd == -1)
            {
            U_WARNING("cannot download from host %.*S", U_STRING_TO_TRACE(host->server));

            host->invalid = true;

            if (ftp.isConnected()) ftp.close();

            continue;
            }

         host->done = true;

         break;
         }

      size_t count = ftp.getFileSize();

      U_INTERNAL_DUMP("offset = %I count = %lu", offset, count)

      U_INTERNAL_ASSERT_MAJOR(count,0)

      size_t chunk, resto = 0;

      if (offset)
         {
         resto   = offset % PAGESIZE;
         offset -= resto;

         U_INTERNAL_DUMP("resto = %lu", resto)
         }

      if (file.ftruncate(count)                          == false ||
          file.memmap(PROT_READ | PROT_WRITE, 0, offset) == false)
         {
         U_ERROR("no more space on disk for size %u", count);
         }

      char* map = file.getMap();

      if (resto)
         {
         map    += resto;
         offset += resto;
         }

      count -= offset;

      if (u_is_tty)
         {
         pinfo = new ProgressBar(1L, 0L);

         pinfo->set(offset, count);
         }

      resto = count % numproc;
      chunk = count / numproc;

      U_INTERNAL_DUMP("chunk = %lu resto = %lu", chunk, resto)

      // first precess

      host->ptr    = map;
      host->size   = chunk;
      host->offset = offset;

      // other precess

      unsigned j = 1, k = i + 1;

      while (j < numproc)
         {
         U_INTERNAL_ASSERT(k < num_score)

         host = from[k];

         if (host->done ||
             host->invalid)
            {
            if (++k >= num_score) k = i + 1;

            continue;
            }

         map    += chunk;
         offset += chunk;

         if (proc.fork() &&
             proc.child())
            {
            while (true)
               {
               ftp.close();

               host->fd = ftp.download(host->ip, host->path, offset);

               if (host->fd == -1)
                  {
                  U_WARNING("cannot download from host %.*S", U_STRING_TO_TRACE(host->server));

                  host->invalid = true;

                  // search for next server

                  while (++k < num_score)
                     {
                     host = from[k];

                     if (host->done ||
                         host->invalid)
                        {
                        continue;
                        }

                     break;
                     }

                  if (k >= num_score)
                     {
                     // next server not found, grab the next after the first process...

                     for (k = i + 1;  k < num_score; ++k)
                        {
                        host = from[k];

                        if (host->invalid) continue;

                        break;
                        }
                     }

                  U_INTERNAL_ASSERT(k < num_score)

                  continue;
                  }

               break;
               }

            host->done   = true;
            host->ptr    = map;
            host->size   = chunk + resto;
            host->offset = offset;

            download();

            U_EXIT(0);
            }

         ++j;
         ++k;
         }

      // parent

      U_INTERNAL_ASSERT(j == numproc)

      host = from[i];

      if (pinfo) pinfo->start();

      download();

      (void) proc.waitAll();

      if (pinfo) pinfo->end();

      file.munmap();

#  ifdef DEBUG
      file.close();

      UFile::munmap(place, map_size1);
      UFile::munmap(from,  map_size2);
#  endif
      }

private:
   struct ip* ip;
   HostData* host;
   HostData** from;
   ProgressBar* pinfo;
   time_t delta_time;
   uint32_t map_size1, map_size2;
   unsigned i, num, numhosts, num_score, numproc;
   int max_ttl;
   UFile file;
   UProcess proc;
   UVector<UString> mirror;
   UString filename, server, path;

   USocket rcvsock;     /* receive (icmp) socket */
   USocket sndsock;     /*    send  (udp) socket */
   u_short ident, port; /* start udp dest port for probe packets */
   bool bIPv6;
};

U_MAIN
