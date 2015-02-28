#include <core/framework.h>
#include <iostream>
#include <stdlib.h>

/**
 * @brief framework
 * @return
 * you can have a custom framework if you extend it and set it here
 */
int main (int argc, char *argv[]) {
    lms::ArgumentHandler arguments;
    arguments.parseArguments(argc, argv);

    if(arguments.showHelp()) {
        std::cout
            << "LMS - Lightweight Module System\n"
            << "Usage: core/lms [-h] [-c config]\n"
            << "  -h         Show help\n"
            << "  -c config  Load configuration (defaults to 'default')\n"
            << std::endl;
    } else {
        lms::Framework framework(arguments);
    }

    return EXIT_SUCCESS;
}

