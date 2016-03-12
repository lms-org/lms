#include <iostream>
#include <cstdlib>

#include "lms/internal/framework.h"

/**
 * @brief Parse command line arguments, show help and start
 * the framework.
 *
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[]) {
    lms::internal::ArgumentHandler arguments;
    arguments.parseArguments(argc, argv);

    lms::internal::Framework framework(arguments);

    return EXIT_SUCCESS;
}
