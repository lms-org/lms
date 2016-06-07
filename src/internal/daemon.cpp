#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <csignal>

namespace lms {
namespace internal {

bool daemonize() {
    // STEP 1: First fork
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid != 0) {
        // daemonizing is successful, return to main program
        return false;
    }

    // STEP 2: Create new session
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // STEP 3: Ignore signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // STEP 4: Second fork
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid != 0) {
        exit(EXIT_SUCCESS);
    }

    // STEP 5: Change to root directory
    chdir("/");

    // STEP 6
    umask(0);

    // STEP 7
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x > 0; x--) {
        close(x);
    }

    return true;
}
}
}