#include <$[MODULE_L].h>

extern "C" {
void* getInstance () {
	return new $[MODULE_W]();
}
const char* getName() {
	return "$[MODULE_W]"; 
}
}
