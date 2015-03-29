#include "backtrace_formatter.h"
#include <iostream>
#include <execinfo.h>
#include <iostream>
#include <cxxabi.h>
#include <sys/wait.h>
#include <unistd.h>
#include "lms/extra/type.h"

namespace lms {

void BacktraceFormatter::print() {
    // http://linux.die.net/man/3/backtrace

    const size_t BUFFER_SIZE = 256;
    void* buffer[BUFFER_SIZE];

    // Get the backtrace.
    std::cerr << "\n\033[31mBacktrace:\033[0m" << std::endl;
    int size = backtrace (buffer, BUFFER_SIZE);

    /*
     * Now try to locate the PC from signal context in the backtrace.
     * Normally it will be found at arr[2], but it might appear later
     * if there were some signal handler wrappers.  Allow a few bytes
     * difference to cope with as many arches as possible.
     * */
    uintptr_t pc = (uintptr_t) buffer[3]; //GET_PC (ctx);
    int i;
    for (i = 0; i < size; ++i)
        if ((uintptr_t) buffer[i] >= pc - 16 && (uintptr_t) buffer[i] <= pc + 16)
            break;

    /* If we haven't found it, better dump full backtrace even including
    the signal handler frames instead of not dumping anything.  */
    if (i == size)
        i = 0;
    /* Now generate nicely formatted output.  */

    char** messages = backtrace_symbols (buffer + i, size - i);

    for (int i = 1; i < size && messages != NULL; ++i)
    {
        char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

        // find parantheses and +address offset surrounding mangled name
        for (char *p = messages[i]; *p; ++p)
        {
            if (*p == '(')
            {
                mangled_name = p;
            }
            else if (*p == '+')
            {
                offset_begin = p;
            }
            else if (*p == ')')
            {
                offset_end = p;
                break;
            }
        }

        // if the line could be processed, attempt to demangle the symbol
        if (mangled_name && offset_begin && offset_end &&
            mangled_name < offset_begin)
        {
            *mangled_name++ = '\0';
            *offset_begin++ = '\0';
            *offset_end++ = '\0';

            std::string realName(lms::extra::demangle(mangled_name));

            std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                      << realName << "+" << offset_begin << offset_end
                      << std::endl;
        }
        // otherwise, print the whole line
        else
        {
            std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
        }

    }
    std::cerr << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
    free (messages);
    }
}
