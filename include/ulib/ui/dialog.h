// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dialog.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DIALOG_H
#define ULIB_DIALOG_H 1

#include <ulib/string.h>

/**
 * @class UDialog
 *
 * @brief UDialog is a simple interface to the Unix dialog program by Savio Lam and Stuart Herbert.
 *        I think dialog is an awesome way of building simple user interfaces
 */

template <class T> class UVector;

class U_EXPORT UDialog {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // Takes one parameter defining the backtitle of the application (will be on the top of screen for all Dialog windows)

   UString output;

   UDialog(const char* _backtitle = 0, int _height = 0, int _width = 0)
         : output(U_CAPACITY), backtitle(_backtitle), command(U_CAPACITY), argument(U_CAPACITY)
      {
      U_TRACE_REGISTER_OBJECT(0, UDialog, "%S,%d,%d", _backtitle, _height, _width)

      width  = _width;
      height = _height;

      if (path_dialog == 0) initialize();
      }

   ~UDialog()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UDialog)
      }

   // SERVICES: calendar, checklist, fselect, gauge, infobox, inputbox, inputmenu, menu, msgbox (message),
   //           password, radiolist, tailbox, tailboxbg, textbox, timebox, and yesno (yes/no)...

   static bool isXdialog()
      {
      U_TRACE_NO_PARAM(0, "UDialog::isXdialog()")

      U_RETURN(xdialog);
      }

   void setSize(int _height = 0, int _width = 0)
      {
      U_TRACE(0, "UDialog::setSize(%d,%d)", _height, _width)

      width  = _width;
      height = _height;
      }

   void setOptions(const UString& opt)
      {
      U_TRACE(0, "UDialog::setOptions(%V)", opt.rep)

      options = opt;
      }

   void setArgument(const UString& arg)
      {
      U_TRACE(0, "UDialog::setArgument(%V)", arg.rep)

      argument = arg;
      }

   bool run(const char* title, const char* format, ...);

   // A password box is similar to an input box, except that the text the user enters is not displayed

   bool passwordbox(const char* text, UString& init, const char* title = 0);

   // Display a list of items as a series of checkboxes.
   // The array selected defines default values (any objects which appear in the list are checked)

   void checklist(const char* text, const char* tags[], const char* items[], bool selected[], int list_height = 0, const char* title = 0);

   // Display a list of items as a series of radioboxes. The param select defines default values.
   // Will return the numerical index of item selected

   int radiolist(const char* text, const char* tags[],
                 const char* items[], int select = 0, int list_height = 0, const char* title = 0);

   // Display a yesno box prompting user with a question. Returns true if the user selects yes or false otherwise

   bool yesno(const char* text, const char* title = 0) { return run(title, "--yesno \"%s\" %d %d", text, height, width); }

   // Display a simple message box on the screen

   void msgbox(const char* text, const char* title = 0) { (void) run(title, "--msgbox \"%s\" %d %d", text, height, width); }

   // An info box is basically a message box. However, in this  case, dialog will exit immediately after displaying
   // the message to the user. The screen is not cleared when dialog exits, so that the message will remain on the screen

   void infobox(const char* text, const char* title = 0) { (void) run(title, "--infobox \"%s\" %d %d", text, height, width); }

   // Display a given file in a textbox

   void textbox(const char* file, const char* title = 0) { (void) run((title ? title : file), "--textbox \"%s\" %d %d", file, height, width); }

   // file selector dialog

   bool fselect(UString& filepath, const char* title = 0);

   // Display text from a file in a dialog box, as in a "tail -f" command

   void tailbox(const char* file, const char* title = 0) { (void) run(title, "--tailbox \"%s\" %d %d", file, height, width); }

   // Display a menu from a list of items. Will return the numerical index of item selected

   int  menu(const char* text, const char* tags[], const char* items[], int list_height = 0, const char* title = 0);
   bool menu(const char* text, UVector<UString>& list, UString& choice, int list_height = 0, const char* title = 0);

   // Display a input box with a text prompt given an initial string

   bool inputbox(const char* text, UString& init, const char* title = 0);

   // ---------------------------------------------------------------------------------------------
   // NB: specific for Xdialog
   // ---------------------------------------------------------------------------------------------

   // The --2inputsbox and the --3inputsbox widgets allow for two or three entry fields into the same box. A <label> is setup above each field
   // (see the --center, --left, --right and --fill common options to learn how the labels justification and alignement can be affected); as for
   // the <text> parameter, the <label>s may hold "\n" sequences IOT force text splitting into several lines. The <init> strings cannot be omitted
   // but may perfectly be NULL (empty) strings

   bool inputsbox2(const char* text, const char* labels[], UVector<UString>& init, const char* title = 0) { return inputsbox(2,text,labels,init,title); }
   bool inputsbox3(const char* text, const char* labels[], UVector<UString>& init, const char* title = 0) { return inputsbox(3,text,labels,init,title); }

   // The combobox displays a <text> together with an entry field to which a pull-down list of <items> is attached,
   // returns the entry field contents once the OK button is pressed

   bool combobox(const char* text, UVector<UString>& list, UString& choice, const char* title = 0);

   // Display a tree box on the screen

   int tree(const char* text, const char* items[], uint32_t items_depth[], const char* item_tags[] = 0,
            const char* items_status[] = 0, const char* items_help[] = 0, int list_height = 0, const char* title = 0);

   // ---------------------------------------------------------------------------------------------

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   const char* backtitle;
   UString command, options, argument;
   int height, width;

   static bool xdialog;
   static char* path_dialog;

   static void initialize(); // does a quick search to find location of dialog program

   bool inputsbox(int n, const char* text, const char* labels[], UVector<UString>& init, const char* title);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UDialog)
};

#endif
