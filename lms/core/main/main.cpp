#include <iostream>
#include <cstdlib>

#include "lms/framework.h"

/**
 * @brief Parse command line arguments, show help and start
 * the framework.
 *
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[]) {
    lms::ArgumentHandler arguments;
    arguments.parseArguments(argc, argv);

    if (arguments.argHelp()) {
        arguments.printHelp();
    } else {
        lms::Framework framework(arguments);
    }

    return EXIT_SUCCESS;
}
