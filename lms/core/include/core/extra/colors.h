#ifndef LMS_EXTRA_COLORS_H

#define COLOR_RED 	 "\033[031m"
#define COLOR_YELLOW "\033[033m"
#define COLOR_GREEN	 "\033[032m"
#define COLOR_WHITE  "\033[0m"
#define COLOR_PINK	 "\033[035m"
#define COLOR_BLUE "\033[034m"

#define RED(a)		COLOR_RED a COLOR_WHITE
#define GREEN(a)	COLOR_GREEN a COLOR_WHITE
#define YELLOW(a)	COLOR_YELLOW a COLOR_WHITE
#define PINK(a)		COLOR_PINK a COLOR_WHITE

#define EMPHASIZE(a) "\033[103m" a COLOR_WHITE


#define ERROR_COLORED(msg) RED("\nERROR: ") msg 

#endif
