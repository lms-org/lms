#ifndef EXTRA_BACKTRACE_FORMATTER
#define EXTRA_BACKTRACE_FORMATTER

#include <signal.h>
#include <execinfo.h>
#include <iostream>
#include <cxxabi.h>
#include <sys/wait.h>
#include <unistd.h>

namespace lms{
class BacktraceFormatter {
public:
    static void print();
};
}
#endif /* EXTRA_BACKTRACE_FORMATTER */
