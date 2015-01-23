// test_options.cpp

#include <ulib/base/utility.h>

#include <ulib/options.h>

#define OPTIONS \
"package " PACKAGE_NAME "\n" \
"version " ULIB_VERSION "\n" \
"purpose 'sample test for UOption class'\n" \
"report_bugs '\nReport bugs to <stefano.casazza@unirel.com>'\n" \
"option a option_a               0 'A option without arg' ''\n" \
"option b option_b               1 'A option with arg' ''\n" \
"option c option_c               2 'A option with optional arg' Hello\n" \
"option - option_with_no_short_1 0 'A option without short' ''\n" \
"option - option_with_no_short_2 1 'A option with default' Hello"

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UOptions opt(1);

   UString x = U_STRING_FROM_CONSTANT(OPTIONS);

   opt.load(x);

   char buffer[1024];
   const char* largv[128] = { "programma" };

   strcpy(buffer, "-a -b pippo -c --option_with_no_short_1 --option_with_no_short_2 \"Bucaiolo a te\" argument_1 argument_2");

   unsigned num_args = opt.getopt(u_split(buffer, strlen(buffer), (char**)largv+1, 0)+1, (char**)largv, &optind);

   U_ASSERT( num_args == 2 )

   U_DUMP_ATTRS(largv)

   U_ASSERT( opt[(uint32_t)0U] == "1" )
   U_ASSERT( opt[(uint32_t)1U] == "pippo" )
   U_ASSERT( opt[(uint32_t)2U] == "Hello" )
   U_ASSERT( opt[(uint32_t)3U] == "1" )
   U_ASSERT( opt[(uint32_t)4U] == "Bucaiolo a te" )

   U_ASSERT( opt['a'] == "1" )
   U_ASSERT( opt['b'] == "pippo" )
   U_ASSERT( opt['c'] == "Hello" )

   U_ASSERT( opt[U_STRING_FROM_CONSTANT("option_with_no_short_1")] == "1" )
   U_ASSERT( opt[U_STRING_FROM_CONSTANT("option_with_no_short_2")] == "Bucaiolo a te" )

   U_ASSERT( strcmp(largv[optind], "argument_1") == 0 )
   ++optind;
   U_ASSERT( strcmp(largv[optind], "argument_2") == 0 )

   opt.getopt(argc, argv, &optind);
}
