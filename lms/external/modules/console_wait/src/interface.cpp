#include <console_wait.h>

extern "C" {
void* getInstance () {
	return new Console_wait();
}
const char* getName() {
	return "Console_wait"; 
}
}
