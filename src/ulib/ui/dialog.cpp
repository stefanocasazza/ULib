// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dialog.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/process.h>
#include <ulib/command.h>
#include <ulib/ui/dialog.h>
#include <ulib/container/vector.h>
#include <ulib/utility/services.h>

#if defined(__NetBSD__) || defined(__UNIKERNEL__) || defined(__OSX__)
extern char** environ;
#endif

bool  UDialog::xdialog;
char* UDialog::path_dialog;

void UDialog::initialize()
{
   U_TRACE_NO_PARAM(0, "UDialog::initialize()")

   U_INTERNAL_ASSERT_EQUALS(path_dialog, 0)

   char path[U_PATH_MAX];

   if ((xdialog = true,  u_pathfind(path, 0, 0, "Xdialog", R_OK | X_OK)) ||
       (xdialog = false, u_pathfind(path, 0, 0,  "dialog", R_OK | X_OK)))
      {
      path_dialog = strdup(path);

      U_INTERNAL_ASSERT_MINOR(u__strlen(path_dialog, __PRETTY_FUNCTION__), U_PATH_MAX)
      }
}

bool UDialog::run(const char* title, const char* format, ...)
{
   U_TRACE(1, "UDialog::run(%S,%S)", title, format)

   U_INTERNAL_ASSERT_EQUALS(isatty(STDOUT_FILENO),1)

   if (command.empty())
      {
                     command.snprintf(U_CONSTANT_TO_PARAM("%s "), path_dialog);
      if (backtitle) command.snprintf_add(U_CONSTANT_TO_PARAM("--backtitle \"%s\" "), backtitle);
      }

   UString tmp(U_CAPACITY);

   if (xdialog)
      {
      if (title) tmp.snprintf(U_CONSTANT_TO_PARAM("%v --stdout --title \"%s\" "), options.rep, title);
      else       tmp.snprintf(U_CONSTANT_TO_PARAM("%v --stdout "),                options.rep);
      }
   else
      {
      UProcess::pipe(STDOUT_FILENO);

      if (title) tmp.snprintf(U_CONSTANT_TO_PARAM("%v --output-fd %d --title \"%s\" "), options.rep, UProcess::filedes[3], title);
      else       tmp.snprintf(U_CONSTANT_TO_PARAM("%v --output-fd %d "),                options.rep, UProcess::filedes[3]);
      }

   UString cmd = command + tmp;

   va_list argp;
   va_start(argp, format);

   tmp.vsnprintf(format, u__strlen(format, __PRETTY_FUNCTION__), argp);

   va_end(argp);

                 cmd += tmp;
   if (argument) cmd += argument;

   argument.clear();
     output.setEmpty();

   UCommand::outputCommandWithDialog(cmd, environ, &output, -1, -1, (xdialog == false));

   if (UCommand::exit_value)
      {
      UCommand::printMsgError();

      U_RETURN(false);
      }

   if (xdialog                     &&
       output.empty()     == false &&
       output.last_char() == '\n')
      {
      output.size_adjust(output.size() - 1);
      }

   U_RETURN(true);
}

bool UDialog::fselect(UString& filepath, const char* title)
{
   U_TRACE(0, "UDialog::fselect(%V,%S)",  filepath.rep, title)

   if (run(title, "--fselect \"%v\" %d %d", filepath.rep, height, width))
      {
      filepath = output.copy();

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UDialog::passwordbox(const char* text, UString& init, const char* title)
{
   U_TRACE(0, "UDialog::passwordbox(%S,%V,%S)", text, init.rep, title)

   if (run(title, "--passwordbox \"%s\" %d %d", text, height, width))
      {
      init = output.copy();

      U_RETURN(true);
      }

   U_RETURN(false);
}

int UDialog::radiolist(const char* text, const char* tags[], const char* items[], int select, int list_height, const char* title)
{
   U_TRACE(0, "UDialog::radiolist(%S,%p,%p,%d,%d,%S)", text, tags, items, select, list_height, title)

   if (argument.empty())
      {
      UString item(U_CAPACITY);

      for (int i = 0; items[i]; ++i)
         {
         item.snprintf(U_CONSTANT_TO_PARAM(" \"%s\" \"%s\" %s"), tags[i], items[i], (i == select ? "on" : "off"));

         argument += item;
         }
      }

   if (run(title, "--single-quoted --radiolist \"%s\" %d %d %d", text, height, width, list_height) &&
       output)
      {
      for (int j = 0; tags[j]; ++j)
         {
         if (tags[j] == output)
            {
            U_RETURN(j);
            }
         }
      }

   U_RETURN(-1);
}

void UDialog::checklist(const char* text, const char* tags[], const char* items[], bool selected[], int list_height, const char* title)
{
   U_TRACE(0, "UDialog::checklist(%S,%p,%p,%p,%d,%S)", text, tags, items, selected, list_height, title)

   if (argument.empty())
      {
      UString item(U_CAPACITY);

      for (int i = 0; items[i]; ++i)
         {
         item.snprintf(U_CONSTANT_TO_PARAM(" \"%s\" \"%s\" %s"), tags[i], items[i], (selected[i] ? "on" : "off"));
                                                                selected[i] = false;

         argument += item;
         }
      }

   if (run(title, "--single-quoted --checklist \"%s\" %d %d %d", text, height, width, list_height) &&
       output)
      {
      UVector<UString> vec;
      uint32_t j, k, n = vec.split(output);

      for (j = 0; j < n; ++j)
         {
         for (k = j; tags[k]; ++k)
            {
            if (tags[k] == vec[j])
               {
               selected[k] = true;

               break;
               }
            }
         }
      }
}

int UDialog::menu(const char* text, const char* tags[], const char* items[], int list_height, const char* title)
{
   U_TRACE(0, "UDialog::menu(%S,%p,%p,%d,%S)", text, tags, items, list_height, title)

   if (argument.empty())
      {
      UString item(U_CAPACITY);

      for (int i = 0; items[i]; ++i)
         {
         item.snprintf(U_CONSTANT_TO_PARAM(" \"%s\" \"%s\""), tags[i], items[i]);

         argument += item;
         }
      }

   if (run(title, "--single-quoted --menu \"%s\" %d %d %d", text, height, width, list_height) &&
       output)
      {
      for (int j = 0; tags[j]; ++j)
         {
         if (tags[j] == output)
            {
            U_RETURN(j);
            }
         }
      }

   U_RETURN(-1);
}

bool UDialog::menu(const char* text, UVector<UString>& list, UString& choice, int list_height, const char* title)
{
   U_TRACE(0, "UDialog::menu(%S,%p,%p,%d,%S)", text, &list, &choice, list_height, title)

   if (argument.empty())
      {
      UString item, fmt(U_CAPACITY);

      for (uint32_t i = 0, n = list.size(); i < n; ++i)
         {
         item = list[i];

         fmt.snprintf(U_CONSTANT_TO_PARAM(" \"%u\" \"%v\""), i+1, item.rep);

         argument += fmt;
         }
      }

   if (run(title, "--single-quoted --menu \"%s\" %d %d %d", text, height, width, list_height) &&
       output)
      {
      choice = list[output.strtoul() -1].copy();

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UDialog::inputbox(const char* text, UString& init, const char* title)
{
   U_TRACE(0, "UDialog::inputbox(%S,%V,%S)", text, init.rep, title)

   if (run(title, "--inputbox \"%s\" %d %d \"%v\"", text, height, width, init.rep))
      {
      init = output.copy();

      U_RETURN(true);
      }

   U_RETURN(false);
}

// ---------------------------------------------------------------------------------------------
// NB: specific for Xdialog
// ---------------------------------------------------------------------------------------------

// The --2inputsbox and the --3inputsbox widgets allow for two or three entry fields into the same box. A <label> is setup above each field
// (see the --center, --left, --right and --fill common options to learn how the labels justification and alignement can be affected); as for
// the <text> parameter, the <label>s may hold "\n" sequences IOT force text splitting into several lines. The <init> strings cannot be omitted
// but may perfectly be NULL (empty) strings

bool UDialog::inputsbox(int n, const char* text, const char* labels[], UVector<UString>& init, const char* title)
{
   U_TRACE(0, "UDialog::inputsbox(%d,%S,%p,%p,%S)", n, text, labels, &init, title)

   int i;

   if (xdialog &&
       n <= 3)
      {
      if (argument.empty())
         {
         UString item(U_CAPACITY);

         for (i = 0; labels[i]; ++i)
            {
            item.snprintf(U_CONSTANT_TO_PARAM(" \"%s\" \"%v\""), labels[i], init[i].rep);

            argument += item;
            }
         }

      if (run(title, "--%dinputsbox \"%s\" %d %d", n, text, height, width))
         {
         UVector<UString> vec(output, '/');

         for (i = 0, n = vec.size(); i < n; ++i) init.replace(i, vec[i].copy());

         U_RETURN(true);
         }
      }
   else
      {
      UString item;

      for (i = 0, n = init.size(); i < n; ++i)
         {
         item = init[i];

         if (inputbox(labels[i], item, title) == false) U_RETURN(false);

         init.replace(i, output.copy());
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

// The combobox displays a <text> together with an entry field to which a pull-down list of <items> is attached,
// returns the entry field contents once the OK button is pressed

bool UDialog::combobox(const char* text, UVector<UString>& list, UString& choice, const char* title)
{
   U_TRACE(0, "UDialog::combobox(%S,%p,%p,%S)", text, &list, &choice, title)

   if (argument.empty())
      {
      UString item, fmt(U_CAPACITY);

      for (uint32_t i = 0, n = list.size(); i < n; ++i)
         {
         item = list[i];

         fmt.snprintf(U_CONSTANT_TO_PARAM(" \"%v\""), item.rep);

         argument += fmt;
         }
      }

   if (run(title, "--combobox \"%s\" %d %d", text, height, width))
      {
      choice = output.copy();

      U_RETURN(true);
      }

   U_RETURN(false);
}

// Display a tree box on the screen

int UDialog::tree(const char* text, const char* items[], uint32_t items_depth[], const char* item_tags[],
                  const char* items_status[], const char* items_help[], int list_height, const char* title)
{
   U_TRACE(0, "UDialog::tree(%S,%p,%p,%p,%p,%p,%d,%S)", text, items, items_depth, item_tags, items_status, items_help, list_height, title)

   if (argument.empty())
      {
      UString item(U_CAPACITY);

      for (int i = 0; items[i]; ++i)
         {
         item.snprintf(U_CONSTANT_TO_PARAM(" \"%s\" \"%s\" %s %u \"%s\""),
                           (item_tags ? item_tags[i] : ""),
                           items[i],
                           (items_status ? items_status[i] : "on"),
                           items_depth[i],
                           (items_help ? items_help[i] : ""));

         argument += item;
         }
      }

   if (run(title, "--treeview \"%s\" %d %d %d", text, height, width, list_height) &&
       output)
      {
      if (item_tags)
         {
         for (int j = 0; item_tags[j]; ++j) if (item_tags[j] == output) U_RETURN(j);
         }

      U_RETURN(0);
      }

   U_RETURN(-1);
}

// ---------------------------------------------------------------------------------------------

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UDialog::dump(bool reset) const
{
   *UObjectIO::os << "width              " << width               << '\n'
                  << "height             " << height              << '\n'
                  << "path_dialog        " << path_dialog         << '\n'
                  << "output    (UString " << (void*)&output      << ")\n"
                  << "command   (UString " << (void*)&command     << ")\n"
                  << "options   (UString " << (void*)&options     << ")\n"
                  << "argument  (UString " << (void*)&argument    << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
