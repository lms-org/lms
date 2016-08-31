#include "backtrace_formatter.h"
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#include <DbgHelp.h>
#else
#include <execinfo.h>
#include <cxxabi.h>
#include "lms/type.h"
#endif

namespace lms {
namespace internal {

void printStacktrace() {
#ifdef _WIN32

    // http://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx

    unsigned int i;
    void *stack[100];
    unsigned short frames;
    SYMBOL_INFO *symbol;
    HANDLE process;

    process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    frames = CaptureStackBackTrace(0, 100, stack, NULL);
    symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    std::cerr << "\nBacktrace\n";

    for (i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

        std::cerr << "[bt]: (" << (frames - i - 1) << ") " << symbol->Name
                  << " " << symbol->Address << "\n";
    }

    std::cerr << std::endl;

    free(symbol);

#else

    // http://linux.die.net/man/3/backtrace

    const size_t BUFFER_SIZE = 256;
    void *buffer[BUFFER_SIZE];

    // Get the backtrace.
    std::cerr << "\n\033[31mBacktrace:\033[0m" << std::endl;
    int size = backtrace(buffer, BUFFER_SIZE);

    /* Now generate nicely formatted output.  */
    char **messages = backtrace_symbols(buffer, size);

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

            std::string realName(lms::demangle(mangled_name));

            std::cerr << "[bt]: (" << i << ") " << messages[i] << " : "
                      << realName << "+" << offset_begin << offset_end
                      << std::endl;
        } else {
            // otherwise, print the whole line
            std::cerr << "[bt]: (" << i << ") " << messages[i] << std::endl;
        }
    }

    std::cerr << std::endl;

    free(messages);

#endif
}

} // namespace internal
} // namespace lms
