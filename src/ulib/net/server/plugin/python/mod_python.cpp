// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_python.cpp - this is a wrapper to embed the PYTHON interpreter
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/uhttp.h>

#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT

#ifdef HAVE_SSIZE_T
#undef HAVE_SSIZE_T
#endif
#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif
#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#include <Python.h>

static PyObject* py_module;
static PyObject* py_userver_on_request_func;
static PyThreadState* py_main_thread_state;

#define U_PROGRAM_NAME "../python/userver.py"

extern "C" {

static inline void dict_set(PyObject* dict, const char* key, PyObject* val)
{
   U_TRACE(0, "dict_set(%p,%S,%p)", dict, key, val)

   U_SYSCALL_VOID(PyDict_SetItemString, "%p,%S,%p", dict, key, val);

   Py_XDECREF(val);
}

static void UPYTHON_set_environment(void* py_environ, char* name, char* value)
{
   U_TRACE(0, "UPYTHON_set_environment(%p,%S,%S)", py_environ, name, value)

   dict_set((PyObject*)py_environ, name, PyString_FromString(value));
}

extern U_EXPORT bool initPYTHON();
       U_EXPORT bool initPYTHON()
{
   U_TRACE(0, "::initPYTHON()")

   bool esito = true;

   U_SET_MODULE_NAME(python);

   U_INTERNAL_ASSERT_POINTER(UHTTP::py_project_app)

   U_SYSCALL_VOID(Py_SetProgramName, "%S", (char*)U_PROGRAM_NAME);

   // initialize thread support

   U_SYSCALL_VOID_NO_PARAM(PyEval_InitThreads);
   U_SYSCALL_VOID_NO_PARAM(Py_Initialize);

   char* sargv[] = { (char*)U_PROGRAM_NAME, (UHTTP::py_project_root    ? UHTTP::py_project_root->data()    : (char*)"."), UHTTP::py_project_app->data(),
                                            (UHTTP::py_virtualenv_path ? UHTTP::py_virtualenv_path->data() : (char*)"") };

   U_SYSCALL_VOID(PySys_SetArgv, "%d,%p", 4, sargv);

   PyObject* py_module_name = PyString_FromString("userver");

   U_INTERNAL_ASSERT_POINTER(py_module_name)

   // save a pointer to the main PyThreadState object

   py_main_thread_state = (PyThreadState*) U_SYSCALL_NO_PARAM(PyThreadState_Get);
   py_module            =      (PyObject*) U_SYSCALL(PyImport_Import, "%p", py_module_name);

   if (py_module == 0 ||
       PyModule_Check(py_module) == false)
      {
      esito = false;

      U_WARNING("I can't load python module userver; check parse errors...");

      PyErr_Print();

      goto end;
      }

   Py_DECREF(py_module_name);

   py_userver_on_request_func = (PyObject*) U_SYSCALL(PyObject_GetAttrString, "%p,%S", py_module, "_userver_on_request");

   U_INTERNAL_ASSERT_POINTER(py_userver_on_request_func)

   if (PyCallable_Check(py_userver_on_request_func) == false)
      {
      esito = false;

      U_WARNING("I can't call _userver_on_request() inside python module userver; check parse errors...");

      PyErr_Print();
      }

   // release the lock

   PyEval_ReleaseLock();

end:
   U_RESET_MODULE_NAME;

   U_RETURN(esito);
}

extern U_EXPORT bool runPYTHON();
       U_EXPORT bool runPYTHON()
{
   U_TRACE(0, "::runPYTHON()")

   bool esito = true;
   PyObject* py_result;

   U_SET_MODULE_NAME(python);

   PyGILState_STATE gstate = (PyGILState_STATE) U_SYSCALL_NO_PARAM(PyGILState_Ensure);
   PyObject* py_func_args  = (PyObject*) U_SYSCALL(PyTuple_New, "%d", 1);
   PyObject* py_environ    = (PyObject*) U_SYSCALL_NO_PARAM(PyDict_New);

   U_INTERNAL_ASSERT_POINTER(py_environ)

   if (UHTTP::setEnvironmentForLanguageProcessing(U_WSCGI, py_environ, UPYTHON_set_environment) == false)
      {
      esito = false;

      goto end;
      }

   dict_set(py_environ, "wsgi.run_once",     PyBool_FromLong(0));
   dict_set(py_environ, "wsgi.multithread",  PyBool_FromLong(0));
   dict_set(py_environ, "wsgi.multiprocess", PyBool_FromLong(UServer_Base::isPreForked()));

   if (*UClientImage_Base::body) dict_set(py_environ, "userver.req.content", PyByteArray_FromStringAndSize(U_STRING_TO_PARAM(*UClientImage_Base::body)));

   // call python

   U_SYSCALL_VOID(PyTuple_SetItem, "%p,%d,%p", py_func_args, 0, py_environ);

   py_result = (PyObject*) U_SYSCALL(PyObject_CallObject, "%p,%p", py_userver_on_request_func, py_func_args);

   Py_DECREF(py_func_args);

   esito = false;

   if (py_result)
      {
      if (PyString_Check(py_result))
         {
         esito = true;

         (void) UClientImage_Base::wbuffer->clear();

         U_http_info.nResponseCode = HTTP_INTERNAL_ERROR;

         U_SRV_LOG("python call failed: %S", PyString_AS_STRING(py_result));
         }
      else if (PyTuple_Check(py_result) &&
               PyTuple_Size( py_result) == 3)
         {
         esito = true;

         PyObject* py_status  = PyTuple_GET_ITEM(py_result, 0);
         PyObject* py_headers = PyTuple_GET_ITEM(py_result, 1);
         PyObject* py_body    = PyTuple_GET_ITEM(py_result, 2);

         if (py_status &&
             PyString_Check(py_status))
            {
            // get the status code

            U_http_info.nResponseCode = strtol(PyString_AS_STRING(py_status), 0, 10);

            U_DUMP("HTTP status = (%d %S)", U_http_info.nResponseCode, UHTTP::getStatusDescription())
            }

         if (py_headers &&
             PyList_Check(py_headers))
            {
            // get the headers

            for (int i = 0, size = PyList_Size(py_headers); i < size; ++i)
               {
               PyObject* py_header_tuple = PyList_GET_ITEM(py_headers, i);

               if (py_header_tuple                &&
                   PyTuple_Check(py_header_tuple) &&
                   PyTuple_Size(py_header_tuple) == 2)
                  {
                  PyObject* py_name  = PyTuple_GET_ITEM(py_header_tuple, 0);
                  PyObject* py_value = PyTuple_GET_ITEM(py_header_tuple, 1);

                  if (py_name                 &&
                      py_value                &&
                      PyString_Check(py_name) &&
                      PyString_Check(py_value))
                     {
                     (void) UClientImage_Base::wbuffer->append(PyString_AS_STRING(py_name));
                     (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(": "));

                     (void) UClientImage_Base::wbuffer->append(PyString_AS_STRING(py_value));
                     (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));
                     }
                  }
               }

            (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));

            U_http_info.endHeader = UClientImage_Base::wbuffer->size();
            }

         ssize_t rsize  = 0;
         char* rcontent = 0;

         if (PyByteArray_Check(py_body))
            {
            rsize    = PyByteArray_Size(py_body);
            rcontent = PyByteArray_AS_STRING(py_body);
            }
         else if (PyString_Check(py_body)) 
            {
            rsize    = PyString_Size(py_body);
            rcontent = PyString_AS_STRING(py_body);
            }

         if (rcontent &&
             rsize > 0)
            {
            // get the body

            (void) UClientImage_Base::wbuffer->append(rcontent, rsize);
            }
         }
      }

   if (esito == false)
      {
      U_WARNING("python call failed");

      PyErr_Print();
      }

   Py_XDECREF(py_result);

   PyGILState_Release(gstate); // Release the thread. No Python API allowed beyond this point

   UClientImage_Base::environment->setEmpty();

end:
   U_RESET_MODULE_NAME;

   U_RETURN(esito);
}

extern U_EXPORT void endPYTHON();
       U_EXPORT void endPYTHON()
{
   U_TRACE_NO_PARAM(0, "endPYTHON()")

   if (py_module)
      {
      // shut down the interpreter

      U_SYSCALL_VOID_NO_PARAM(PyEval_AcquireLock);

      U_SYSCALL_VOID(PyThreadState_Swap, "%p", py_main_thread_state);

      Py_XDECREF(py_userver_on_request_func);
      Py_XDECREF(py_module);

      U_SYSCALL_VOID_NO_PARAM(Py_Finalize);
      }
}
}
