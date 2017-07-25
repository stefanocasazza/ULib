// test_notifier.cpp

#include <ulib/file.h>
#include <ulib/timer.h>
#include <ulib/notifier.h>

#include <fcntl.h>

struct node {
   node* next;
   int data;
   static node* first;
};

node* node::first;

static void list_add(int i)
{
   node* n = (node*)malloc(sizeof(node));

   n->data = i;
   n->next = node::first;

   node::first = n;
}

static void list_print()
{
   printf("print list\n");

   for (node* n = node::first; n; n = n->next)
      {
      printf("list: %p %p %d\n", n, n->next, n->data);
      }

   fflush(stdout);
}

static void list_destroy()
{
   printf("destroy list\n");

   node* prev = 0;

   for (node* n = node::first; n; prev = n, n = n->next)
      {
      if (prev) free(prev);
      }

   if (prev) free(prev);

   node::first = 0;
}

static void list_creat()
{
   list_add(0); /* list: 0 */
   list_add(1); /* list: 1 0 */
   list_add(2); /* list: 2 1 0 */
   list_add(3); /* list: 3 2 1 0 */
   list_add(4); /* list: 4 3 2 1 0 */
}

static void list_remove(node** ptr)
{
   node* item = *ptr;

   *ptr = item->next;

   printf("remove %d\n", item->data);

   free(item);
}

static void list_test()
{
   list_creat();
   list_print();

   // this is WRONG...

   printf("*****WRONG************\n");

   node* item;
   node** ptr;

   for (ptr = &node::first; (item = *ptr); ptr = &(*ptr)->next)
      {
      printf("item: %p %p %d\n", item, item->next, item->data);

      if (item->data == 2) list_remove(ptr);
      }

   list_print();

   printf("**********************\n");

   // this is WRONG...

   list_destroy();
   list_creat();
   list_print();

   printf("*****WRONG************\n");

   item =  node::first;
   ptr  = &node::first;

   do {
      printf("item: %p %p %d\n", item, item->next, item->data);

      if (item->data == 2) list_remove(ptr);

      ptr = &(*ptr)->next;
      }
   while ((item = *ptr));

   list_print();

   printf("**********************\n");

   // this is WRONG...

   list_destroy();
   list_creat();
   list_print();

   printf("*****WRONG************\n");

   ptr = &node::first;

   while ((item = *ptr))
      {
      printf("item: %p %p %d\n", item, item->next, item->data);

      if (item->data == 3) list_remove(ptr);

      ptr = &(*ptr)->next;
      }

   list_print();

   printf("**********************\n");

   // this is OK...

   list_destroy();
   list_creat();
   list_print();

   printf("*****OK***************\n");

   ptr = &node::first;

   while ((item = *ptr))
      {
      printf("item: %p %p %d\n", item, item->next, item->data);

      if (item->data == 3)
         {
         list_remove(ptr);

         continue;
         }

      ptr = &(*ptr)->next;
      }

   list_print();

   printf("**********************\n");

   // this is OK...

   list_destroy();
   list_creat();
   list_print();

   printf("*****OK***************\n");

   item =  node::first;
   ptr  = &node::first;

   do {
      printf("item: %p %p %d\n", item, item->next, item->data);

      if (item->data == 2)
         {
         list_remove(ptr);

         continue;
         }

      ptr = &(*ptr)->next;
      }
   while ((item = *ptr));

   list_print();

   printf("**********************\n");

   exit(0);
}

class MyAlarm1 : public UEventTime {
public:

   // COSTRUTTORI

   MyAlarm1(long sec, long usec) : UEventTime(sec, usec)
      {
      U_TRACE_REGISTER_OBJECT(0, MyAlarm1, "%ld,%ld", sec, usec)
      }

   ~MyAlarm1()
      {
      U_TRACE_UNREGISTER_OBJECT(0, MyAlarm1)
      }

   virtual int handlerTime()
      {
      U_TRACE(0, "MyAlarm1::handlerTime()")

      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(-1);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif
};

class MyAlarm2 : public MyAlarm1 {
public:

   // COSTRUTTORI

   MyAlarm2(long sec, long usec) : MyAlarm1(sec, usec)
      {
      U_TRACE_REGISTER_OBJECT(0, MyAlarm2, "%ld,%ld", sec, usec)
      }

   ~MyAlarm2()
      {
      U_TRACE_UNREGISTER_OBJECT(0, MyAlarm2)
      }

   virtual int handlerTime()
      {
      U_TRACE(0, "MyAlarm2::handlerTime()")

      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(0);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return MyAlarm1::dump(_reset); }
#endif
};

static char message[4096];
static int _i, fd_input, fd_output;

class handlerOutput : public UEventFd {
public:

   handlerOutput()
      {
      fd      = fd_output;
      op_mask = EPOLLOUT;
      }

   ~handlerOutput()
      {
      }

   int handlerWrite()
      {
      U_TRACE(0, "handlerOutput::handlerWrite()")

#  ifdef U_STDCPP_ENABLE
      cout << "receive message: " << message << endl;
#  endif

      U_RETURN(0);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const
      {
      *UObjectIO::os << "fd      " << fd << "\n"
                     << "op_mask " << op_mask;

      if (_reset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return 0;
      }
#endif
};

class handlerInput : public UEventFd {
public:

   handlerInput()
      {
      fd = fd_input;
      }

   ~handlerInput()
      {
      }

   int handlerRead()
      {
      U_TRACE(0, "handlerInput::handlerRead()")

      int bytes_read = U_SYSCALL(read, "%d,%p,%u", fd, message, 13);

      if (bytes_read > 0)
         {
         if (_i & 1)
            {
            if (UNotifier::waitForWrite(fd_output) >= 1)
               {
               static handlerOutput* handler_output;

               if (handler_output == 0)
                  {
                  U_NEW(handlerOutput, handler_output, handlerOutput);

                  UNotifier::insert(handler_output);
                  }
               }
            }
         }

      U_RETURN(0);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const
      {
      *UObjectIO::os << "fd      " << fd << "\n"
                     << "op_mask " << op_mask;

      if (_reset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return 0;
      }
#endif
};

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   list_test();

   int fds[2], n = (argc > 1 ? u_atoi(argv[1]) : 5);

   pipe(fds);

// fd_input  = U_SYSCALL(open, "%S,%d",    "inp/notifier.input",  O_RDONLY);                       // STDIN_FILENO
// fd_output = U_SYSCALL(open, "%S,%d,%d", "tmp/notifier.output", O_WRONLY | O_CREAT, PERM_FILE);  // STDOUT_FILENO

   // fds[0] is for READING, fds[1] is for WRITING

   fd_input  = fds[0];
   fd_output = fds[1];

   handlerInput* c;
   handlerInput* d;

   U_NEW(handlerInput, c, handlerInput);
   U_NEW(handlerInput, d, handlerInput);

   UNotifier::init();
   UNotifier::insert(c);
   UNotifier::handlerDelete(c);
   UNotifier::insert(d);

#ifdef __unix__
   U_ASSERT(UNotifier::waitForRead( fds[0], 500) <= 0)
#endif
   U_ASSERT(UNotifier::waitForWrite(fds[1], 500) >= 1)

   MyAlarm1* a;
   MyAlarm2* b;

   U_NEW(MyAlarm1, a, MyAlarm1(1L, 0L));
   U_NEW(MyAlarm2, b, MyAlarm2(1L, 0L));

   UTimer::init(UTimer::ASYNC);

   UTimer::insert(a);
   UTimer::insert(b);

   UTimer::setTimer();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif

   UEventTime timeout;

   for (_i = 0; _i < n; ++_i)
      {
      timeout.setMicroSecond(100L * 1000L);

      (void) U_SYSCALL(write, "%d,%p,%u", fd_output, U_CONSTANT_TO_PARAM("hello, world"));

             UNotifier::waitForEvent(&timeout);
      (void) UNotifier::waitForRead(fds[0], 500);

      U_NEW(MyAlarm1, a, MyAlarm1(1L, 0L));

      UTimer::insert(a);

      UTimer::setTimer();
      
      timeout.nanosleep();

#  if defined(U_STDCPP_ENABLE) && defined(DEBUG)
      if (argc > 2) UTimer::printInfo(cout);
#  endif
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif

   UTimer::clear();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif

   UNotifier::clear();

#ifdef __unix__
   U_ASSERT(UNotifier::waitForRead( fd_input,  1 * 1000) <= 0)
#endif
   U_ASSERT(UNotifier::waitForWrite(fd_output, 1 * 1000) >= 1)
}
