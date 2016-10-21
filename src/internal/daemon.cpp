#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <csignal>

#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
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

    // STEP 8
    //reopen in,out,error and throw it away.
    int in = open("/dev/null", O_RDONLY);
    int out = open("/dev/null", O_RDWR);
    int err = open("/dev/null", O_RDWR);
    dup2(in, STDIN_FILENO);
    dup2(out, STDOUT_FILENO);
    dup2(err, STDERR_FILENO);
    // I don't think that we can close them as the next fd will be pointing to 2 causing errors written to it?
    //TODO investigate
    //close(in);
    //close(out);
    //close(err);

    return true;
}
}
}
