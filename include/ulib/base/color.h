/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    color.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_COLOR_H
#define ULIB_COLOR_H 1

#define U_RESET_STR          "\033[0;0m"
#define U_BLACK_STR          "\033[0;30m"
#define U_RED_STR            "\033[0;31m"                
#define U_GREEN_STR          "\033[0;32m"
#define U_YELLOW_STR         "\033[0;33m"
#define U_BLUE_STR           "\033[0;34m"
#define U_MAGENTA_STR        "\033[0;35m"
#define U_CYAN_STR           "\033[0;36m"
#define U_WHITE_STR          "\033[0;37m"
#define U_BRIGHTBLACK_STR    "\033[1;30m"
#define U_BRIGHTRED_STR      "\033[1;31m"
#define U_BRIGHTGREEN_STR    "\033[1;32m"
#define U_BRIGHTYELLOW_STR   "\033[1;33m"
#define U_BRIGHTBLUE_STR     "\033[1;34m"
#define U_BRIGHTMAGENTA_STR  "\033[1;35m"
#define U_BRIGHTCYAN_STR     "\033[1;36m"
#define U_BRIGHTWHITE_STR    "\033[1;37m"

enum colors { RESET, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
              BRIGHTBLACK, BRIGHTRED, BRIGHTGREEN, BRIGHTYELLOW, BRIGHTBLUE,
              BRIGHTMAGENTA, BRIGHTCYAN, BRIGHTWHITE };

#endif
