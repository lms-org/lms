#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

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

   if(argc >= 2 && strcmp("master", argv[1]) == 0) {
        std::cout << "Starting Master Server with PID: " << getpid() << std::endl;
        lms::internal::MasterServer server;
        server.useUnix("/tmp/lms.sock");
        server.start();
        exit(0);
    }

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

    /*int fv[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fv);

    lms::Request req;
    req.set_type(lms::Request_MessageType_RUN);

    lms::internal::ProtobufWrapper writer(fv[0]);
    writer.writeMessage(req);

    lms::internal::ProtobufWrapper reader(fv[1]);
    lms::Request res;
    reader.readMessage(res);

    printf("%d\n", res.type());*/

    return EXIT_SUCCESS;
}
