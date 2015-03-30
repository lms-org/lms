#include "lms/extra/backtrace_formatter.h"
#include <iostream>
#include <Windows.h>

namespace lms {
namespace extra {

void printStacktrace() {
    // http://stackoverflow.com/questions/5693192/win32-backtrace-from-c-code
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx

    unsigned int   i;
    void         * stack[ 100 ];
    unsigned short frames;
    SYMBOL_INFO  * symbol;
    HANDLE         process;

    process = GetCurrentProcess();

    SymInitialize( process, NULL, TRUE );

    frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
    symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    printf("\nStacktrace\n");

    for( i = 0; i < frames; i++ ) {
        SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

        printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
    }

    free( symbol );
}

}  // namespace extra
}  // namespace lms

