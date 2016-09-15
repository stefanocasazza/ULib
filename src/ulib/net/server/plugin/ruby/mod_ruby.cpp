// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_ruby.cpp - this is a wrapper to embed the RUBY interpreter
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/server/server.h>

#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT

#include <ruby.h>

#if RUBY_VERSION < 19
#  include <st.h>
#  define rb_errinfo() ruby_errinfo
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(x) RARRAY(x)->len
#endif

#ifndef RARRAY_PTR
#define RARRAY_PTR(x) RARRAY(x)->ptr
#endif

#ifndef RSTRING_PTR
#define RSTRING_PTR(x) RSTRING(x)->ptr
#endif

#ifndef RSTRING_LEN
#define RSTRING_LEN(x) RSTRING(x)->len
#endif

static ID call;
static VALUE dispatcher;
static VALUE dollar_zero;
static VALUE URUBY_io_class;
static const char* post_readline_pos;
static const char* post_readline_watermark;

extern "C" {

static VALUE URUBY_io_init(int argc, VALUE* argv, VALUE self)
{
   U_TRACE(0, "URUBY_io_init(%d,%p,%llu)", argc, argv, self)

   U_RETURN(self);
}

static VALUE URUBY_io_new(VALUE _class)
{
   U_TRACE(0, "URUBY_io_new(%llu)", _class)

   VALUE self = Data_Wrap_Struct(_class, 0, 0, 0);

   U_INTERNAL_DUMP("self = %llu", self)

   U_SYSCALL_VOID(rb_obj_call_init, "%llu,%d,%p", self, 0, 0);

   U_RETURN(self);
}

static VALUE URUBY_io_rewind(VALUE obj, VALUE args)
{
   U_TRACE(0, "URUBY_io_rewind(%llu,%llu)", obj, args)

   post_readline_pos       =                     UClientImage_Base::body->data();
   post_readline_watermark = post_readline_pos + UClientImage_Base::body->size();

   U_INTERNAL_DUMP("post_readline_pos = %p post_readline_watermark = %p", post_readline_pos, post_readline_watermark)

   U_RETURN(Qnil);
}

static VALUE URUBY_io_read(VALUE obj, VALUE args)
{
   U_TRACE(0, "URUBY_io_read(%llu,%llu)", obj, args)

   // When EOF is reached, this method returns nil if length is given and not nil, or "" if length is not given or is nil

   U_INTERNAL_DUMP("post_readline_pos = %p post_readline_watermark = %p RARRAY_LEN(args) = %d RARRAY_PTR(args)[0] = %p",
                    post_readline_pos,     post_readline_watermark,     RARRAY_LEN(args),     RARRAY_PTR(args)[0])

   if (RARRAY_LEN(args) > 0 &&
       RARRAY_PTR(args)[0] != Qnil)
      {
      if (post_readline_pos >= post_readline_watermark)
         {
         post_readline_pos = post_readline_watermark = 0; // all the readline buffer has been consumed

         U_RETURN(Qnil);
         }

      U_INTERNAL_DUMP("hint = %ld", NUM2LONG(RARRAY_PTR(args)[0]))
      }

   if (post_readline_pos < post_readline_watermark)
      {
      // If buffer is given, then the read data will be placed into buffer instead of a newly created String object

      VALUE result = (RARRAY_LEN(args) > 1 ? rb_str_cat(RARRAY_PTR(args)[1], post_readline_pos, post_readline_watermark - post_readline_pos)
                                           : rb_str_new(                     post_readline_pos, post_readline_watermark - post_readline_pos));

      post_readline_pos = post_readline_watermark = 0; // all the readline buffer has been consumed

      U_RETURN(result);
      }

   U_RETURN(Qnil);
}

static VALUE URUBY_io_gets(VALUE obj, VALUE args)
{
   U_TRACE(0, "URUBY_io_gets(%llu,%llu)", obj, args)

   U_INTERNAL_DUMP("post_readline_pos = %p post_readline_watermark = %p", post_readline_pos, post_readline_watermark)

   if (post_readline_pos >= post_readline_watermark)
      {
      post_readline_pos = post_readline_watermark = 0; // all the readline buffer has been consumed

      U_RETURN(Qnil);
      }

   VALUE result;
   uint32_t rlen;

   for (const char* ptr = post_readline_pos; ptr < post_readline_watermark; ++ptr)
      {
      if (*ptr == '\n') // found a newline
         {
         rlen = (ptr - post_readline_pos) + 1;

         result = rb_str_new(post_readline_pos, rlen);

         post_readline_pos += rlen;

         U_RETURN(result);
         }
      }

   // no line found, let's return all

   result = rb_str_new(post_readline_pos, post_readline_watermark - post_readline_pos);

   post_readline_pos = post_readline_watermark = 0; // all the readline buffer has been consumed

   U_RETURN(result);
}

static VALUE URUBY_io_each(VALUE obj, VALUE args)
{
   U_TRACE(0, "URUBY_io_each(%llu,%llu)", obj, args)

   if (!rb_block_given_p()) rb_raise(rb_eArgError, "Expected block on rack.input 'each' method");

   // yield strings chunks

   for (;;)
      {
      VALUE chunk = URUBY_io_gets(obj, Qnil);

      if (chunk == Qnil) U_RETURN(Qnil);

      rb_yield(chunk);
      }

   U_RETURN(Qnil);
}

static VALUE URUBY_send_body(VALUE obj)
{
   U_TRACE(0, "URUBY_send_body(%llu)", obj)

   if (TYPE(obj) != T_STRING)
      {
      U_SRV_LOG("URUBY_send_body() - unmanaged body type: %d", TYPE(obj));
      }
   else
      {
      uint32_t len    = RSTRING_LEN(obj);
      const char* ptr = RSTRING_PTR(obj);

      U_INTERNAL_DUMP("body(%d) = %.*S", len, len, ptr)

      if (len == 0)
         {
         U_SRV_LOG("URUBY_send_body() - body empty");
         }
      else if (len != 3 ||
               memcmp(ptr, U_CONSTANT_TO_PARAM("nil")) != 0)   
         {
         (void) UClientImage_Base::wbuffer->append(ptr, len);
         }
      else
         {
         (void) UClientImage_Base::wbuffer->clear();

         U_http_info.nResponseCode = HTTP_BAD_REQUEST;

         U_SRV_LOG("URUBY_send_body() - body nil");
         }
      }

   U_RETURN(Qnil);
}

static void URUBY_add_header(const char* k, uint32_t kl, const char* v, uint32_t vl)
{
   U_TRACE(0, "URUBY_add_header(%.*S,%u,%.*S,%u)", kl, k, kl, vl, v, vl)

   if (kl > 0)
      {
      (void) UClientImage_Base::wbuffer->append(k, kl);
      (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(": "));
      }

   (void) UClientImage_Base::wbuffer->append(v, vl);
   (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));
}

static VALUE URUBY_send_header(VALUE obj, VALUE headers)
{
   U_TRACE(0, "URUBY_send_header(%llu,%llu)", obj, headers)

   VALUE hkey, hval;

   if (TYPE(obj) == T_ARRAY)
      {
      if (RARRAY_LEN(obj) < 2) U_RETURN(Qnil);

      hkey = rb_obj_as_string(RARRAY_PTR(obj)[0]);
      hval = rb_obj_as_string(RARRAY_PTR(obj)[1]);
      }
   else if (TYPE(obj) == T_STRING)
      {
      hkey = obj;

#  if RUBY_VERSION >= 19
      hval = rb_hash_lookup(headers, obj);
#  else
      hval = rb_hash_aref(headers, obj);
#  endif
      }
   else
      {
      U_RETURN(Qnil);
      }

   if (TYPE(hkey) != T_STRING ||
       TYPE(hval) != T_STRING)
      {
      U_RETURN(Qnil);
      }

   char* this_header;
   char* header_value;
   int cnt = 0, header_value_len;

   header_value_len           = RSTRING_LEN(hval);
   header_value = this_header = RSTRING_PTR(hval);

   for (int i = 0; i < header_value_len; ++i)
      {
      if (header_value[i] == '\n') // multiline header, send it
         {
         URUBY_add_header(RSTRING_PTR(hkey), RSTRING_LEN(hkey), this_header, cnt);

         this_header += cnt + 1;
                        cnt = 0;

         continue;
         }

      cnt++;
      }

   if (cnt > 0) URUBY_add_header(RSTRING_PTR(hkey), RSTRING_LEN(hkey), this_header, cnt);

   U_RETURN(Qnil);
}

static void URUBY_rack_hack_dollar_zero(VALUE name, ID id)
{
   U_TRACE(0, "URUBY_rack_hack_dollar_zero(%llu,%llu)", name, id)

   dollar_zero = rb_obj_as_string(name);

   rb_obj_taint(dollar_zero);
}

static void URUBY_set_environment(void* env, char* name, char* value)
{
   U_TRACE(0, "URUBY_set_environment(%p,%S,%S)", env, name, value)

   (void) rb_hash_aset((VALUE)env, rb_str_new2(name), rb_str_new2(value));
}

extern U_EXPORT bool initRUBY();
       U_EXPORT bool initRUBY()
{
   U_TRACE(0, "::initRUBY()")

   bool esito = true;

   U_SET_MODULE_NAME(ruby);

#if RUBY_VERSION >= 19
   int argc      = 2;
   char* sargv[] = { (char*) "userver", (char*) "-e0" };
   char** argv   = sargv;

   U_SYSCALL_VOID(ruby_sysinit, "%p,%p", &argc, &argv);

   RUBY_INIT_STACK;
#endif

   U_SYSCALL_VOID_NO_PARAM(ruby_init); // Sets up and initializes the interpreter. This function should be called before any other Ruby-related functions

#if RUBY_VERSION >= 19
   if (UHTTP::ruby_libdir)
      {
      U_SYSCALL_VOID(ruby_incpush, "%S", UHTTP::ruby_libdir->data());

      U_SRV_LOG("directory %rV added to the ruby libdir search path", UHTTP::ruby_libdir->rep);
      }

   (void) U_SYSCALL(ruby_options, "%d,%p", argc, argv);
#elif RUBY_VERSION < 19
   VALUE dummy;

   Init_stack(&dummy);

   U_SYSCALL_VOID_NO_PARAM(ruby_init_loadpath);
#endif

   // U_SYSCALL_VOID_NO_PARAM(ruby_show_version);

   // Sets the name of the Ruby script (and $0) to name

   U_SYSCALL_VOID(ruby_script, "%S", "userver");

   dollar_zero = rb_str_new2("userver");

   rb_define_hooked_variable("$0",            &dollar_zero, 0, (void(*)(ANYARGS))URUBY_rack_hack_dollar_zero);
   rb_define_hooked_variable("$PROGRAM_NAME", &dollar_zero, 0, (void(*)(ANYARGS))URUBY_rack_hack_dollar_zero);

   if (UHTTP::ruby_on_rails)
      {
#  if RUBY_VERSION < 19
      (void) U_SYSCALL(rb_require, "%S", "rubygems");
#  endif
      (void) rb_funcall(rb_cObject, rb_intern("require"), 1, rb_str_new2("rack"));

      VALUE rack = (VALUE) U_SYSCALL(rb_const_get, "%llu,%llu", rb_cObject, rb_intern("Rack"));

#  if RUBY_VERSION == 19
      ID BodyProxy = rb_intern("BodyProxy");

      VALUE result = rb_funcall(rack, rb_intern("const_defined?"), 1, ID2SYM(BodyProxy));

      if (result == Qtrue)
         {
         VALUE bodyproxy = (VALUE) U_SYSCALL(rb_const_get, "%llu,%llu", rack, BodyProxy);

         // get the list of available instance_methods

         VALUE arg          = Qfalse;
         VALUE methods_list = rb_class_instance_methods(1, &arg, bodyproxy);

         U_INTERNAL_DUMP("methods_list = %S", RSTRING_PTR(rb_inspect(methods_list)))

         if (rb_ary_includes(methods_list, ID2SYM(rb_intern("each"))) == Qfalse &&
             rb_eval_string("module Rack;class BodyProxy;def each(&block);@body.each(&block);end;end;end"))
            {
            U_SRV_LOG("Rack::BodyProxy successfully patched");
            }
         }
#  endif

      VALUE builder = (VALUE) U_SYSCALL(rb_const_get, "%llu,%llu", rack, rb_intern("Builder"));

      U_INTERNAL_DUMP("builder = %llu", builder)

      VALUE rackup = rb_funcall(builder, rb_intern("parse_file"), 1, rb_str_new2("config.ru"));

      call = rb_intern("call");

      U_INTERNAL_DUMP("rackup = %llu call = %llu", rackup, call)

      if (call == 0               ||
          TYPE(rackup) != T_ARRAY ||
          RARRAY_LEN(rackup) < 1)
         {
         esito = false;

         U_WARNING("Unable to find RACK entry point in Ruby on Rails");

         goto end;
         }

      dispatcher = RARRAY_PTR(rackup)[0];

      URUBY_io_class = (VALUE) U_SYSCALL(rb_define_class, "%S,%llu", "URUBY_IO", rb_cObject);

      U_SYSCALL_VOID(rb_gc_register_address, "%p", &call);
      U_SYSCALL_VOID(rb_gc_register_address, "%p", &dispatcher);
      U_SYSCALL_VOID(rb_gc_register_address, "%p", &URUBY_io_class);

      rb_define_singleton_method(URUBY_io_class, "new",        (VALUE(*)(ANYARGS))URUBY_io_new,    1);
      rb_define_method(          URUBY_io_class, "initialize", (VALUE(*)(ANYARGS))URUBY_io_init,  -1);
      rb_define_method(          URUBY_io_class, "gets",       (VALUE(*)(ANYARGS))URUBY_io_gets,   0);
      rb_define_method(          URUBY_io_class, "each",       (VALUE(*)(ANYARGS))URUBY_io_each,   0);
      rb_define_method(          URUBY_io_class, "read",       (VALUE(*)(ANYARGS))URUBY_io_read,  -2);
      rb_define_method(          URUBY_io_class, "rewind",     (VALUE(*)(ANYARGS))URUBY_io_rewind, 0);

      U_SRV_LOG("Ruby(%d) on Rails application successfully loaded", RUBY_VERSION);
      }

end:
   U_RESET_MODULE_NAME;

   U_RETURN(esito);
}

extern U_EXPORT bool runRUBY();
       U_EXPORT bool runRUBY()
{
   U_TRACE(0, "::runRUBY()")

   bool esito = true;

   U_SET_MODULE_NAME(ruby);

   if (UHTTP::ruby_on_rails)
      {
      VALUE env = rb_hash_new();

      if (UHTTP::setEnvironmentForLanguageProcessing(U_RAKE, (void*)env, URUBY_set_environment) == false)
         {
         esito = false;

         goto end;
         }

      VALUE rbv = rb_ary_new();

      /**
       * "rack.version"=>[1, 0],
       * "rack.run_once"=>false,
       * "rack.multithread"=>false,
       * "rack.multiprocess"=>false,
       * "rack.url_scheme"=>"http",
       * "rack.errors"=>#<IO:<STDERR>>,
       * "rack.input"=>#<StringIO:0x007fa1bce039f8>,
       */

      (void) rb_ary_store(rbv, 0, INT2NUM(1));
      (void) rb_ary_store(rbv, 1, INT2NUM(1));
      (void) rb_hash_aset(env, rb_str_new2("rack.version"),      rbv);
      (void) rb_hash_aset(env, rb_str_new2("rack.run_once"),     Qfalse);
      (void) rb_hash_aset(env, rb_str_new2("rack.multithread"),  Qfalse);
      (void) rb_hash_aset(env, rb_str_new2("rack.multiprocess"), UServer_Base::isPreForked() ? Qtrue : Qfalse);

      VALUE nw       = rb_intern("new"),
            io_class = (VALUE) U_SYSCALL(rb_const_get, "%llu,%llu", rb_cObject, rb_intern("IO"));

      (void) rb_hash_aset(env, rb_str_new2("rack.errors"), rb_funcall(      io_class, nw, 2, INT2NUM(2), rb_str_new("w", 1)));
      (void) rb_hash_aset(env, rb_str_new2("rack.input"),  rb_funcall(URUBY_io_class, nw, 1));

      URUBY_io_rewind(Qnil, Qnil);

      VALUE ret = rb_funcall(dispatcher, call, 1, env), status, headers, body, result;

      U_INTERNAL_DUMP("ret = %llu", ret)

      if (ret == Qnil          ||
          TYPE(ret) != T_ARRAY ||
          RARRAY_LEN(ret) != 3)
         {
         esito = false;

         U_WARNING("Ruby on Rails failed - invalid RACK response");

         goto end;
         }

      status  = rb_obj_as_string(RARRAY_PTR(ret)[0]);
      headers =                  RARRAY_PTR(ret)[1];
      body    =                  RARRAY_PTR(ret)[2];
      result  = Qnil;

      // get the status code

      U_http_info.nResponseCode = strtol(RSTRING_PTR(status), 0, 10);

      U_DUMP("HTTP status = (%d %S)", U_http_info.nResponseCode, UHTTP::getStatusDescription())

      // get the body

      if (rb_respond_to(body, rb_intern("to_path")))
         {
         char buffer[U_PATH_MAX];

         // If the Body responds to to_path, it must return a String identifying the location of a file whose contents are
         // identical to that produced by calling each; this may be used by the server as an alternative, possibly more
         // efficient way to transport the response

         result = rb_funcall(body, rb_intern("to_path"), 0);

         U_http_info.endHeader = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("X-Sendfile: %.*s\r\n\r\n"), RSTRING_LEN(result), RSTRING_PTR(result));

         (void) UClientImage_Base::wbuffer->append(buffer, U_http_info.endHeader);
         }
      else
         {
         // get the headers

         if (rb_respond_to(headers, rb_intern("each")))
            {
#     if RUBY_VERSION >= 19
            result = rb_block_call(headers, rb_intern("each"), 0, 0, (VALUE(*)(ANYARGS))URUBY_send_header, headers);
#     else
            result = rb_iterate(rb_each, headers, URUBY_send_header, headers);
#     endif
            }

         (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));

         U_http_info.endHeader = UClientImage_Base::wbuffer->size();

         if (rb_respond_to(body, rb_intern("each")))
            {
#        if RUBY_VERSION >= 19
            result = rb_block_call(body, rb_intern("each"), 0, 0, (VALUE(*)(ANYARGS))URUBY_send_body, 0);
#        else
            result = rb_iterate(rb_each, body, URUBY_send_body, 0);
#        endif
            }
         }

      if (rb_respond_to(body, rb_intern("close"))) result = rb_funcall(body, rb_intern("close"), 0);

      U_INTERNAL_DUMP("result = %llu", result)

      UClientImage_Base::environment->setEmpty();
      }
   else
      {
      void* node = U_SYSCALL(rb_load_file, "%S", UHTTP::file->getPathRelativ()); // Loads the given file into the interpreter

      if (node == 0)
         {
         esito = false;

         U_WARNING("Ruby script %.*S load failed", U_FILE_TO_TRACE(*UHTTP::file));

         goto end;
         }

      int result = U_SYSCALL(ruby_exec_node, "%p", node);

      if (result)
         {
      // VALUE err    = rb_errinfo();
      // VALUE eclass = rb_class_name(rb_class_of(err));

         VALUE global_bang = rb_gv_get("$!");
         VALUE e_msg       = rb_funcall(global_bang, rb_intern("message"), 0);
         char* c_e_msg     = StringValueCStr(e_msg);

         U_SRV_LOG("RUBY script %.*S return (%d, %s)", U_FILE_TO_TRACE(*UHTTP::file), result, c_e_msg);

#     ifdef DEBUG
         if (u_get_unalignedp32(c_e_msg) != U_MULTICHAR_CONSTANT32('e','x','i','t'))
            {
            char buffer[8192];

            VALUE trace     = rb_funcall(global_bang, rb_intern("backtrace"), 0),
                  trace_str = rb_funcall(trace, rb_intern("join"), 1, rb_str_new_cstr("\n"));

            (void) UFile::writeToTmp(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("Error: \"%s\"\n\n%s"), c_e_msg, StringValueCStr(trace_str)),
                                     O_RDWR | O_APPEND, "ruby_embedded.err", 0);
            }
#     endif
         }
      }

end:
   U_RESET_MODULE_NAME;

   U_RETURN(esito);
}

extern U_EXPORT void endRUBY();
       U_EXPORT void endRUBY()
{
   U_TRACE_NO_PARAM(0, "endRUBY()")

   if (dollar_zero)
      {
#  if RUBY_VERSION < 19
      U_SYSCALL_VOID_NO_PARAM(ruby_finalize);
#  else
      (void) U_SYSCALL(ruby_cleanup, "%d", 0);
#  endif
      }
}
}
