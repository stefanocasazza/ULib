// test_event.cpp

#include <ulib/file.h>
#include <ulib/libevent/event.h>

#include <map>
#include <iostream>

struct handler
{
   int i;

   handler() : i(0) {}

   void operator()(int signum, short event)
      {
      U_TRACE(5, "handler::operator()(%d,%hd)", signum, event)

      if (i < 5) cout << "keep going...\n";
      else
         {
         cout << "done!\n";

         UDispatcher::exit();
         }
      }
};

struct handler1
{
   int i;

   handler1() : i(0) {}

   void handle_event(int signum, short event)
      {
      cout << ++i << " interrupts, ";

      if (i < 5) cout << "keep going...\n";
      else
         {
         cout << "done!\n";

         UDispatcher::exit();
         }
      }
};

static void sighandler(int signum, short event, void* data)
{
   U_TRACE(5, "::sighandler(%d,%hd,%p)", signum, event, data)

   int& i = *(int*)(data);

   cout << ++i << " interrupts, ";
}

typedef UMemCb< handler1, void (handler1::*)(int, short) > cb_type;

static void check_mixed_way()
{
   U_TRACE(5, "check_mixed_way()")

   handler  h;
   handler1 h1;
   cb_type  cb(h1, &handler1::handle_event);

   USignal<handler> e(SIGINT, h);
   USignal<cb_type> e1(SIGINT, cb);
   csignal          sigev(SIGINT, sighandler, &h.i);

   UDispatcher::add(e1);    // third...
   UDispatcher::add(e);     // second...
   UDispatcher::add(sigev); // first...

   UDispatcher::dispatch();

   UDispatcher::del(&sigev);
   UDispatcher::del(&e);
   UDispatcher::del(&e1);
}

struct handler2
{
   int fds[4];
   std::map< int, UEvent_Base* > events;

   handler2()
      {
      pipe(fds);
      pipe(fds+2);
      }

   void add(UEvent_Base& e) { events[e.fd()] = &e; }

   void operator()(int fd, short event)
      {
      U_TRACE(5, "handler1::operator()(%d,%hd)", fd, event)

      char buf[7];

      read(fd, buf, 7);

      cout << "Read from fd " << fd << ": " << buf << "\n";

      UDispatcher::del(events[fd]);
      }
};

static void check_prio_test()
{
   U_TRACE(5, "check_prio_test()")

   if (UDispatcher::priority_init(2))
      {
      handler2 h;

      write(h.fds[1], "hola 1", 7);
      write(h.fds[3], "hola 2", 7);

      UEvent< handler2 > e1(h.fds[0], EV_READ, h);
      UEvent< handler2 > e2(h.fds[2], EV_READ, h);

      h.add(e1);
      h.add(e2);

      UDispatcher::add(e1, 1);
      UDispatcher::add(e2, 0); // major priority...

      UDispatcher::dispatch();

      UDispatcher::del(&e1);
      UDispatcher::del(&e2);
      }
}

static int len;
static int called;
static cevent* ev;
static int sckpair[2];
static char buf[256];
static int test_okay;
static const char* test = "test string";

static void read_cb(int fd, short event, void* arg)
{
   U_TRACE(5, "read_cb(%d,%hd,%p)", fd, event, arg)

   len = read(fd, buf, sizeof(buf));

   sprintf(buf, "%s: read %d%s\n", __func__, len, len ? "" : " - means EOF");

   cout << buf;

   if (len)
      {
      if (!called) UDispatcher::add(*ev);
      }
   else if (called == 1) test_okay = 1;

   called++;
}

static void check_test_eof()
{
   U_TRACE(5, "check_test_eof()")

   socketpair(AF_UNIX, SOCK_STREAM, 0, sckpair);

   write(sckpair[0], test, strlen(test)+1);

   shutdown(sckpair[0], SHUT_WR);

   // Initalize one event

   ev = new cevent(sckpair[1], EV_READ, read_cb, 0);

   UDispatcher::add(*ev);

   UDispatcher::dispatch();

   UDispatcher::del(ev);

   delete ev;

   cout << "check_test_eof() test_okay = " << test_okay << '\n';
}

typedef void (cb_t)(int, short);

static UEvent<cb_t>* evw;

static void write_cb(int fd, short event)
{
   U_TRACE(5, "write_cb(%d,%hd)", fd, event)

   len = write(fd, test, strlen(test) + 1);

   sprintf(buf, "%s: write %d%s\n", __func__, len, len ? "" : " - means EOF");

   cout << buf;

   if (len > 0)
      {
      if (!called) UDispatcher::add(*evw);

      close(sckpair[0]);
      }
   else if (called == 1) test_okay = 1;

   called++;
}

static void check_test_weof()
{
   U_TRACE(5, "check_test_weof()")

   called = 0;
   test_okay = 0;

   signal(SIGPIPE, SIG_IGN);

   socketpair(AF_UNIX, SOCK_STREAM, 0, sckpair);

   // Initalize one event

   evw = new UEvent<cb_t>(sckpair[1], EV_WRITE, write_cb);

   UDispatcher::add(*evw);

   UDispatcher::dispatch();

   UDispatcher::del(evw);

   delete evw;

   cout << "check_test_weof() test_okay = " << test_okay << '\n';
}

#define NEVENT 100

static ctimer* evt[NEVENT];

static void time_cb(int fd, short event, void *arg)
{
   U_TRACE(5, "time_cb(%d,%hd,%p)", fd,  event, arg)

   called++;

   if (called < 10 * NEVENT)
      {
      for (int i = 0; i < 10; i++)
         {
         UTimeVal tv(0, random() % 50000L);

         int j = random() % NEVENT;

         if (tv.tv_usec % 2) UDispatcher::add(*evt[j], tv);
         else                UDispatcher::del( evt[j]);
         }
      }
}

static void check_test_time()
{
   U_TRACE(5, "check_test_time()")

   called = 0;
   test_okay = 0;

   for (int i = 0; i < NEVENT; i++)
      {
      // Initalize one event

      evt[i] = new ctimer(time_cb, NULL);

      UDispatcher::add(*evt[i], UTimeVal(0, random() % 50000L));
      }

   UDispatcher::dispatch();

   test_okay = (called >= NEVENT);

   cout << "check_test_time() test_okay = " << test_okay << '\n';
}


int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   u_ev_base = (struct event_base*) U_SYSCALL_NO_PARAM(event_init);

   check_mixed_way();
   check_prio_test();
   check_test_eof();
   check_test_weof();
   check_test_time();
}
