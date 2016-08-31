#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include "internal/framework.h"
#include "internal/master.h"
#include "internal/daemon.h"

/**
 * @brief Parse command line arguments, show help and start
 * the framework.
 *
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[]) {
    /*lms::internal::ArgumentHandler arguments;
    arguments.parseArguments(argc, argv);

    lms::internal::Framework framework(arguments);*/

    try {
        lms::internal::connectToMaster(argc, argv);
    } catch (lms::LmsException &ex) {
        std::cout << "Start LMS Master Server ..." << std::endl;
        if (lms::internal::daemonize()) {
            // This is inside the daemon
            lms::internal::MasterServer server;
            server.useUnix("/tmp/lms.sock");
            server.start();
        } else {
            // This is outside the daemon
            sleep(1);
            lms::internal::connectToMaster(argc, argv);
        }
    }

    return EXIT_SUCCESS;
}
