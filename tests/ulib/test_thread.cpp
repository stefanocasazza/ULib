// test_thread.cpp

#include <ulib/thread.h>

#include <iostream>

#undef  OK
#define OK    {printf("ok\n");}
#undef  ERROR
#define ERROR {printf("ko\n");return 1;}

#define TEST_CHANGE(b) {if(!TestChange(b))return 1;}

static bool status;
static volatile int n;
static int time_to_sleep = 5;

static bool WaitNValue(int value)
{
   U_TRACE(5+256, "::WaitNValue(%d)", value)

   U_INTERNAL_DUMP("n = %d", n)

   for (int i = 0; i < 100; ++i)
      {
      if (n == value) U_RETURN(true);

      UThread::nanosleep(10);
      }

   U_RETURN(false);
}

static bool WaitChangeNValue(int value)
{
   U_TRACE(5+256, "::WaitChangeNValue(%d)", value)

   U_INTERNAL_DUMP("n = %d", n)

   for (int i = 0; i < 100; ++i)
      {
      if (n != value) U_RETURN(true);

      UThread::nanosleep(10);
      }

   U_RETURN(false);
}

static bool TestChange(bool shouldChange)
{
   U_TRACE(5, "::TestChange(%b)", shouldChange)

   if (shouldChange) printf("- thread should change n...");
   else              printf("- thread should not change n...");

   fflush(0);

   if (WaitChangeNValue(n) == shouldChange)
      {
      printf("ok\n");

      U_RETURN(true);
      }

   printf("ko\n");

   fflush(0);

   U_RETURN(false);
}

class ThreadTest : public UThread {
public:

   ThreadTest() : UThread(PTHREAD_CREATE_JOINABLE) {}

   virtual void run()
      {
      U_TRACE(5+256, "ThreadTest::run()")

      n = 1;

      if (WaitNValue(2)) // wait for main thread
         {
#     ifdef DEBUG
         status = UTrace::suspend();
#     endif

         while (true)
            {
            yield();

            ++n; // increment infinitely

            sleep(time_to_sleep);
            }
         }
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return UThread::dump(reset); }
#endif
};

class Child : public UThread {
public:

   Child() : UThread(PTHREAD_CREATE_JOINABLE) {}

   virtual void run()
      {
      U_TRACE(5, "Child::run()")

      U_INTERNAL_DUMP("CHILD START")

      cout << "child start" << endl;

      sleep(50);

      U_INTERNAL_DUMP("CHILD END")

      cout << "child end" << endl;
      }
};

class Father : public UThread {
public:

   Father() : UThread(PTHREAD_CREATE_JOINABLE) {}

   virtual void run()
      {
      U_TRACE(5, "Father::run()")

      U_INTERNAL_DUMP("STARTING CHILD THREAD")

      cout << "starting child thread" << endl;

      Child* ch = U_NEW(Child);

      ch->start();

      ch->sleep(100);

      U_INTERNAL_DUMP("DELETING CHILD THREAD = %p", ch)

      delete ch;

      U_INTERNAL_DUMP("FATHER END")

      cout << "father end" << endl;
      }
};

class myObject {
public:
    myObject() {}
   ~myObject() {}
};

class myThread : public UThread {
public:

    myThread() : UThread(PTHREAD_CREATE_JOINABLE) {}
   ~myThread()                                    {}

   void run()
      {
      U_TRACE(5, "myThread::run()")

      myObject obj;

      setCancel(cancelImmediate);

      sleep(100);
      }
};

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   // This is a little regression test
   UThread* th;
   ThreadTest test;

   // test only thread, without sincronization
   printf("***********************************************\n");
   printf("* Testing class Thread without syncronization *\n");
   printf("***********************************************\n");

   printf("Testing thread creation\n\n");
   n = 0;
   test.start();

   // wait for n == 1
   printf("- thread should set n to 1...");
   if (WaitNValue(1)) OK
   else               ERROR;
   printf("\nTesting thread is working\n\n");

   // increment number in thread
   n = 2;
   TEST_CHANGE(true);
   TEST_CHANGE(true);

   // suspend thread, variable should not change
   printf("\nTesting suspend & resume\n\n");
   test.suspend();
   TEST_CHANGE(false);
   TEST_CHANGE(false);

   // resume, variable should change
   test.resume();
   TEST_CHANGE(true);
   TEST_CHANGE(true);

   printf("\nTesting recursive suspend & resume\n\n");
   test.suspend();
   test.suspend();
   TEST_CHANGE(false);
   TEST_CHANGE(false);

   test.resume();
   TEST_CHANGE(false);
   TEST_CHANGE(false);
   test.resume();
   TEST_CHANGE(true);
   TEST_CHANGE(true);

   printf("\nTesting no suspend on resume\n\n");
   test.resume();
   TEST_CHANGE(true);
   TEST_CHANGE(true);

   // suspend thread, variable should not change
   printf("\nTesting resuspend\n\n");
   test.suspend();
   TEST_CHANGE(false);
   TEST_CHANGE(false);
   test.resume();

   time_to_sleep = 5000;

#ifdef DEBUG
   UTrace::resume(status);
#endif

   // Test child thread destroying before father

   cout << "\nstarting father thread" << endl;

   th = U_NEW(Father);

   th->start();

   UThread::nanosleep(200);

   U_INTERNAL_DUMP("FATHER DELETE = %p", th)

   delete th;

   // Test if cancellation unwinds stack frame

   cout << "\nstarting thread" << endl;

   th = U_NEW(myThread);

   th->start();

   UThread::nanosleep(100); // 150 millisecond

   delete th; // delete to join

   printf("\nNow program should finish... :)\n");

   return 0;
}
