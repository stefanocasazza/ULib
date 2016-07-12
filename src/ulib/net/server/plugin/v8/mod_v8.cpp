// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_v8.cpp - this is a wrapper of Google V8 JavaScript Engine
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/string.h>

#include <v8.h>
#include <string.h>

using namespace v8;

// http://code.google.com/apis/v8/get_started.html#intro
// ------------------------------------------------------------------------------
// compiles and executes javascript and returns the script return value as string
// ------------------------------------------------------------------------------

extern "C" {
extern U_EXPORT void runv8(UString& x);
       U_EXPORT void runv8(UString& x)
{
   U_TRACE(0, "::runv8(%V)", x.rep)

   // Create a new context
   static Persistent<Context> context = Context::New();

   // Create a stack-allocated handle scope
   HandleScope handle_scope;

   // Enter the created context for compiling and running the script
   Context::Scope context_scope(context);

   // Create a string containing the JavaScript source code
   Handle<String> source = String::New(U_STRING_TO_PARAM(x));

   // Compile the source code
   TryCatch tryCatch;

   Handle<Script> script = Script::Compile(source);

   if (script.IsEmpty())
      {
      String::Utf8Value error(tryCatch.Exception());

      (void) x.replace(*error, error.length());

      return;
      }

   // Run the script
   Handle<Value> result = script->Run();

   if (result.IsEmpty())
      {
      String::Utf8Value error(tryCatch.Exception());

      (void) x.replace(*error, error.length());

      return;
      }

   // Dispose the persistent context
   // context.Dispose();

   // return result as string, must be deallocated in cgo wrapper
   String::AsciiValue ascii(result);

   (void) x.replace(*ascii, ascii.length());
}
}
