#include <core/framework.h>
#include <iostream>
#include <cstdlib>

/**
 * @brief framework
 * @return
 * you can have a custom framework if you extend it and set it here
 */
int main (int argc, char *argv[]) {
    lms::ArgumentHandler arguments;
    arguments.parseArguments(argc, argv);

    if(arguments.argHelp()) {
        std::cout
            << "LMS - Lightweight Modular System\n"
            << "Usage: core/lms [-h] [-c config]\n"
            << "  -h, --help          Show help\n"
            << "  -c config           Load configuration (defaults to 'default')\n"
            << "  --logging-min-level Filter minimum logging level, e.g. ERROR\n"
            << "  --logging-prefix    Prefix of logging tags to filter\n"
            << std::endl;
    } else {
        lms::Framework framework(arguments);
    }

    return EXIT_SUCCESS;
}

