#pragma once

//! USAGE EXAMPLE:
//! printf(COLOR_RED"this will be in red, even the number %d\n"COLOR_RESET, 123);

#ifndef NO_COLORS
    #define COLOR_RESET "\x1B[0m" /* No Color */

    // plain colors
    #define COLOR_BLACK  "\x1B[0;30m"
    #define COLOR_RED    "\x1B[0;31m"
    #define COLOR_GREEN  "\x1B[0;32m"
    #define COLOR_ORANGE "\x1B[0;33m"
    #define COLOR_BLUE   "\x1B[0;34m"
    #define COLOR_PURPLE "\x1B[0;35m"
    #define COLOR_CYAN   "\x1B[0;36m"
    #define COLOR_GRAY   "\x1B[0;37m"
    
    // BOLD Colors (brighter than plain colors)
    #define COLOR_BOLD_GRAY   "\033[1;30m"
    #define COLOR_BOLD_RED    "\033[1;31m"
    #define COLOR_BOLD_GREEN  "\033[1;32m"
    #define COLOR_BOLD_YELLOW "\033[1;33m"
    #define COLOR_BOLD_BLUE   "\033[1;34m"
    #define COLOR_BOLD_PURPLE "\033[1;35m"
    #define COLOR_BOLD_CYAN   "\033[1;36m"
    #define COLOR_BOLD_WHITE  "\033[1;37m"
#else
    #define COLOR_RESET
    #define COLOR_BLACK
    #define COLOR_RED
    #define COLOR_GREEN
    #define COLOR_ORANGE
    #define COLOR_BLUE
    #define COLOR_PURPLE
    #define COLOR_CYAN
    #define COLOR_GRAY
    #define COLOR_BOLD_GRAY
    #define COLOR_BOLD_RED
    #define COLOR_BOLD_GREEN
    #define COLOR_BOLD_YELLOW
    #define COLOR_BOLD_BLUE
    #define COLOR_BOLD_PURPLE
    #define COLOR_BOLD_CYAN
    #define COLOR_BOLD_WHITE
#endif

#define COLOR_POINTS  COLOR_ORANGE
#define COLOR_MESSAGE COLOR_CYAN
#define COLOR_COMMAND COLOR_BOLD_WHITE
#define COLOR_ERROR   COLOR_BOLD_RED
#define COLOR_SUCCESS COLOR_BOLD_GREEN
#define COLOR_WARNING COLOR_BOLD_YELLOW



