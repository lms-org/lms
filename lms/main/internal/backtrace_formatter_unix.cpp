#include "lms/internal/backtrace_formatter.h"
#include <iostream>
#include <execinfo.h>
#include <cxxabi.h>
#include "lms/extra/type.h"

namespace lms {
namespace internal {

void printStacktrace() {
    // http://linux.die.net/man/3/backtrace

    const size_t BUFFER_SIZE = 256;
    void* buffer[BUFFER_SIZE];

    // Get the backtrace.
    std::cerr << "\n\033[31mBacktrace:\033[0m" << std::endl;
    int size = backtrace (buffer, BUFFER_SIZE);

    /* Now generate nicely formatted output.  */
    char** messages = backtrace_symbols (buffer, size);

    for (int i = 1; i < size && messages != NULL; ++i) {
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        // find parantheses and +address offset surrounding mangled name
        for (char *p = messages[i]; *p; ++p) {
            if (*p == '(') {
                mangled_name = p;
            } else if (*p == '+') {
                offset_begin = p;
            } else if (*p == ')') {
                offset_end = p;
                break;
            }
        }

        // if the line could be processed, attempt to demangle the symbol
        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin) {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            std::string realName(lms::extra::demangle(mangled_name));

            std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                      << realName << "+" << offset_begin << offset_end
                      << std::endl;
        } else {
            // otherwise, print the whole line
            std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
        }

    }

    std::cerr << std::endl;

    free (messages);
}

}  // namespace internal
}  // namespace lms
