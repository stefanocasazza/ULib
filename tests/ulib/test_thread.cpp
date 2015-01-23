// test_thread.cpp

#include <ulib/thread.h>

#include <iostream>

static volatile int n;

static bool WaitNValue(int value)
{
   U_TRACE(5, "::WaitNValue(%d)", value)

   U_INTERNAL_DUMP("n = %d", n)

   for (int i = 0; ; ++i)
      {
      if (n == value) break;

      if (i >= 100) U_RETURN(false);

      UThread::sleep(10);
      }

   U_RETURN(true);
}

static bool WaitChangeNValue(int value)
{
   U_TRACE(5, "::WaitChangeNValue(%d)", value)

   U_INTERNAL_DUMP("n = %d", n)

   for (int i = 0; ; ++i)
      {
      if (n != value) break;

      if (i >= 100) U_RETURN(false);

      UThread::sleep(10);
      }

   U_RETURN(true);
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

   ThreadTest() : UThread(true) {}

   virtual void run()
      {
      U_TRACE(5, "ThreadTest::run()")

      n = 1;

      // wait for main thread

      if (!WaitNValue(2)) return;

      // increment infinitely

      while (true)
         {
         yield();

         n = n+1;
         }
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return UThread::dump(reset); }
#endif
};

#undef  OK
#define OK    {printf("ok\n");}
#undef  ERROR
#define ERROR {printf("ko\n");return 1;}

#define TEST_CHANGE(b) {if(!TestChange(b))return 1;}

class Child : public UThread {
public:

   Child() {}

   virtual void run()
      {
      U_TRACE(5, "Child::run()")

      cout << "child start" << endl;

      UThread::sleep(1500);

      cout << "child end" << endl;
      }
};

class Father : public UThread {
public:

   Father() {}

   virtual void run()
      {
      U_TRACE(5, "Father::run()")

      cout << "starting child thread" << endl;

      UThread* th = new Child;

      th->start();

      UThread::sleep(1000);

      delete th;

      cout << "father end" << endl;
      }
};

class myObject {
public:
    myObject() { cout << "created auto object on stack"    << endl; }
   ~myObject() { cout << "destroyed auto object on cancel" << endl; }
};

class myThread : public UThread {
public:

    myThread() : UThread() {}
   ~myThread()             { cout << "ending thread" << endl; }

   void run()
      {
      U_TRACE(5, "myThread::run()")

      myObject obj;

      setCancel(cancelImmediate);

      UThread::sleep(2000);
      }
};

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   // This is a little regression test
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

   // Test child thread destroying before father

   cout << "\nstarting father thread" << endl;

   Father* th = new Father;

   th->start();

   UThread::sleep(2000);

   delete th;

   // Test if cancellation unwinds stack frame

   cout << "\nstarting thread" << endl;

   myThread* th1 = new myThread;

   th1->start();

   UThread::sleep(1000); // 1 second

   delete th1; // delete to join

   printf("\nNow program should finish... :)\n");

   return 0;
}
