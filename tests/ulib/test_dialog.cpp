// test_dialog.cpp

#include <ulib/ui/dialog.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString filepath;
   UDialog x(" My first dialog", 10, 41);

   x.msgbox("Hi, this is a simple message box.\n"
            "You can use this to display any message you like.\n"
            "The box will remain until you press the ENTER key.");

   x.setSize(0, 0);

   x.textbox("/etc/passwd");

   (void) x.yesno("Hello, this is my first dialog program");

   x.setSize(16, 51);

   UString init = U_STRING_FROM_CONSTANT("bucaiolo");

   (void) x.inputbox("Hi, this is an input dialog box. You can use\n"
                       "this to ask questions that require the user\n"
                       "to input a string as the answer. You can\n"
                       "input strings of length longer than the\n"
                       "width of the input box, in that case, the\n"
                       "input field will be automatically scrolled.\n"
                       "You can use BACKSPACE to correct errors.\n\n"
                       "Try entering your name below:", init, "INPUT BOX");

   (void) x.passwordbox("Hi, this is an password dialog box. You can use\n"
                       "this to ask questions that require the user\n"
                       "to input a string as the answer. You can\n"
                       "input strings of length longer than the\n"
                       "width of the input box, in that case, the\n"
                       "input field will be automatically scrolled.\n"
                       "You can use BACKSPACE to correct errors.\n\n"
                       "Try entering your name below:", init, "PASSWORD BOX");

   x.setSize(34, 70);

   filepath = U_STRING_FROM_CONSTANT("/var/log/dmesg");

   x.tailbox(filepath.c_str(), "TAIL BOX");

   const char* tags1[] = { "Apple",
                           "Dog",
                           "Orange",
                           "Chicken",
                           "Cat",
                           "Fish",
                           "Lemon", 0 };

   const char* items1[] = { "It's an apple",
                           "No, that's not my dog",
                           "Yeah, that's juicy",
                           "Chicken Normally not a pet",
                           "No, never put a dog and a cat together!",
                           "Cats like fish",
                           "You know how it tastes", 0 };

   bool selected[]        = { false, true, false, false, true, true, true };
// bool selected_result[] = { false, true, false, false, true, true, true };

   x.setSize(20, 61);

   x.checklist("Hi, this is a checklist box. You can use this to\n"
               "present a list of choices which can be turned on or\n"
               "off. If there are more items than can fit on the\n"
               "screen, the list will be scrolled. You can use the\n"
               "UP/DOWN arrow keys, the first letter of the choice as a\n"
               "hot key, or the number keys 1-9 to choose an option.\n"
               "Press SPACE to toggle an option on/off.\n\n"
               "Which of the following are fruits?", tags1, items1, selected, 0, "CHECKLIST BOX");

// U_ASSERT( memcmp(selected, selected_result, sizeof(selected[7])) == 0 )

   (void) x.radiolist("Hi, this is a radiolist box. You can use this to\n"
                        "present a list of choices which can be turned on or\n"
                        "off. If there are more items than can fit on the\n"
                        "screen, the list will be scrolled. You can use the\n"
                        "UP/DOWN arrow keys, the first letter of the choice as a\n"
                        "hot key, or the number keys 1-9 to choose an option.\n"
                        "Press SPACE to toggle an option on/off.\n\n"
                        "Which of the following are fruits?", tags1, items1, 4, 0, "RADIOLIST BOX");

   const char* tags2[] = { "Linux",
                           "NetBSD",
                           "OS/2",
                           "WIN NT",
                           "PCDOS",
                           "MSDOS", 0 };

   const char* items2[] = { "The Great Unix Clone for 386/486",
                            "Another free Unix Clone for 386/486",
                            "IBM OS/2",
                            "Microsoft Windows NT",
                            "IBM PC DOS",
                            "Microsoft DOS", 0 };

   x.setSize(20, 51);

   (void) x.menu("Hi, this is a menu box. You can use this to\n"
                  "present a list of choices for the user to \n"
                  "choose. If there are more items than can fit\n"
                  "on the screen, the menu will be scrolled.\n"
                  "You can use the UP/DOWN arrow keys, the first\n"
                  "letter of the choice as a hot key, or the\n"
                  "number keys 1-9 to choose an option.\n"
                  "Try it now!\n\n"
                  "Choose the OS you like:", tags2, items2, 0, "MENU BOX");

   x.setSize(14, 48);

   (void) x.fselect(filepath, "Please choose a file");

   if (UDialog::isXdialog())
      {
      x.setSize(24, 80);

      const char* items3[] = { "one",
                               "two",
                               "three",
                               "four",
                               "five",
                               "six",
                               "seven",
                               "eight",
                               "nine",
                               "ten", 0 };

      uint32_t items_depth[] = {
                               0,
                               1,
                               2,
                               1,
                               2,
                               3,
                               3,
                               4,
                               3,
                               1, 0 };

      const char* item_tags[] = {
                              "tag000",
                              "tag001",
                              "tag002",
                              "tag003",
                              "tag004",
                              "tag005",
                              "tag006",
                              "tag007",
                              "tag008",
                              "tag009", 0 };

      const char* items_status[] = {
                               "off",
                               "off",
                               "on",
                               "off",
                               "off",
                               "off",
                               "off",
                               "off",
                               "off",
                               "off", 0 };

      const char* items_help[] = {
                               "one help",
                               "two help",
                               "three help",
                               "four help",
                               "five help",
                               "six help",
                               "seven help",
                               "eight help",
                               "nine help",
                               "ten help", 0 };

      x.setOptions(U_STRING_FROM_CONSTANT("--item-help"));

      (void) x.tree("treeview box demo", items3, items_depth, item_tags, items_status, items_help, 0, "TREE VIEW BOX");
      }
}
