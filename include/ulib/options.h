// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    options.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_OPTIONS_H
#define ULIB_OPTIONS_H 1

#include <ulib/string.h>

#ifndef ARGS
#define ARGS "[ARGS]" 
#endif

#ifndef REPORT_BUGS
#define REPORT_BUGS \
"\nMaintained by Stefano Casazza <stefano.casazza@gmail.com>" \
"\nReport bugs to <stefano.casazza@gmail.com>"
#endif

class U_EXPORT UOptions {
public:

   typedef struct option_item {
      UStringRep* desc;
      UStringRep* value;
      UStringRep* long_opt;
      int has_arg;
      char short_opt;
   } option_item;

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString package,
           version,
           purpose,
           args,
           report_bugs;

    UOptions(uint32_t n = 0);
   ~UOptions();

   // SERVICES

   void add(const UString& desc,
            const UString& long_opt,
            const UString& default_value,
            int has_arg = 1, char short_opt = '\0');

   // -------------------------------------------------------------------------------------
   // [package <PACKNAME>]
   // [version <VERSION>]
   // [purpose <PURPOSE>]
   // [report_bugs <REPORT_BUGS>]
   // option <SHORT> <LONG> <HAS_ARG> <DESC> <DEFAULT>
   // option <SHORT> <LONG> <HAS_ARG> <DESC> <DEFAULT>
   // ....
   // -------------------------------------------------------------------------------------
   // purpose "sample test for UOption class"
   // option a option_a               0 "A option without arg"       ""
   // option b option_b               1 "A option with arg"          ""
   // option c option_c               2 "A option with optional arg" Hello
   // option - option_with_no_short_1 0 "A option without short"     ""
   // option - option_with_no_short_2 1 "A option with default"      Hello
   // -------------------------------------------------------------------------------------
   // Example: program -h|--help
   // -------------------------------------------------------------------------------------
   // ulib: 0.1.0
   // Purpose: sample test for UOption class
   // Usage: lt-test_options [OPTIONS] [ARGS]...
   //    -h  --help                          Print help and exit
   //    -V  --version                       Print version and exit
   //    -a  --option_a                      A option without arg
   //    -b  --option_b=VALUE                A option with arg (default=pippo)
   //    -c  --option_c[=VALUE]              A option with optional arg (default=Hello)
   //        --option_with_no_short_1        A option without short
   //        --option_with_no_short_2=VALUE  A option with default (default=Bucaiolo a te)
   //
   // Maintained by Stefano Casazza <stefano@unirel.it>
   // Report bugs to <stefano@unirel.it>
   // -------------------------------------------------------------------------------------

   void load(const UString& str);

   // -----------------------------------------------------------------------------------------------------
   // PARSING
   // -----------------------------------------------------------------------------------------------------
   // Scan elements of <argv> (whose length is <argc>) for option characters given in <optstring>.
   // If an element of <argv> starts with '-', and is not exactly "-" or "--", then it is an option element.
   // The characters of this element (aside from the initial '-') are option characters.
   // -----------------------------------------------------------------------------------------------------

   uint32_t getopt(int argc, char** argv, int* poptind); // return how many arguments

   // VALUE OF OPTION

   UString operator[](char c);
   UString operator[](uint32_t i);
   UString operator[](const UString& long_opt);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const; // dump the state of the object
#endif

protected:
   option_item* item;
   uint32_t length, capacity;

   static struct option long_options[128];

   void printHelp(vPF func) __attribute__ ((noreturn));

private:
   U_DISALLOW_COPY_AND_ASSIGN(UOptions)

   friend class Application;
   friend class UApplication;
   friend class UXApplicazione;
};

#endif
