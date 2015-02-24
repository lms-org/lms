#include <scheduler.h>

extern "C" {
void* getInstance () {
	return new Scheduler();
}
const char* getName() {
	return "Scheduler"; 
}
}
