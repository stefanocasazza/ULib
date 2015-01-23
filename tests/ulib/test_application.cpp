// test_application.cpp

#include <ulib/utility/interrupt.h>

#define U_OPTIONS \
"purpose \"sample test for UApplication class\"\n" \
"option a option_a               0 \"A option without arg\"       \"\"\n" \
"option b option_b               1 \"A option with arg\"          \"\"\n" \
"option c option_c               2 \"A option with optional arg\" Hello\n" \
"option - option_with_no_short_1 0 \"A option without short\"     \"\"\n" \
"option - option_with_no_short_2 1 \"A option with default\"      Hello"

#include <ulib/application.h>

class Application : public UApplication {
public:

   static void reset()
      {
      U_TRACE(5, "Application::reset()")

      ::fflush(stdout);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      U_ASSERT( num_args == 2 )

      U_ASSERT( opt[(uint32_t)0U] == U_STRING_FROM_CONSTANT("1") )
      U_ASSERT( opt[(uint32_t)1U] == U_STRING_FROM_CONSTANT("pippo") )
      U_ASSERT( opt[(uint32_t)2U] == U_STRING_FROM_CONSTANT("Hello") )
      U_ASSERT( opt[(uint32_t)3U] == U_STRING_FROM_CONSTANT("1") )
      U_ASSERT( opt[(uint32_t)4U] == U_STRING_FROM_CONSTANT("Bucaiolo_a_te") )

      U_ASSERT( opt['a'] == U_STRING_FROM_CONSTANT("1") )
      U_ASSERT( opt['b'] == U_STRING_FROM_CONSTANT("pippo") )
      U_ASSERT( opt['c'] == U_STRING_FROM_CONSTANT("Hello") )

      U_ASSERT( opt[U_STRING_FROM_CONSTANT("option_with_no_short_1")] == U_STRING_FROM_CONSTANT("1") )
      U_ASSERT( opt[U_STRING_FROM_CONSTANT("option_with_no_short_2")] == U_STRING_FROM_CONSTANT("Bucaiolo_a_te") )

      U_ASSERT( strcmp(argv[optind], "argument_1") == 0 )
      ++optind;
      U_ASSERT( strcmp(argv[optind], "argument_2") == 0 )

      u_atexit(reset);

      (void) write(STDOUT_FILENO, U_CONSTANT_TO_PARAM("AutoSending SIGTERM...\n"));
      
#  ifndef DEBUG
      UInterrupt::act.sa_flags   = 0;
      UInterrupt::act.sa_handler = UInterrupt::handlerInterrupt;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGTERM, &UInterrupt::act, 0); // 15
#  endif

      UInterrupt::sendSignal(SIGTERM, u_pid);
      }
};

U_MAIN
