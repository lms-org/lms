#ifndef $[LAYER_C]_$[MODULE_C]_$[LAYER_C]_H
#define $[LAYER_C]_$[MODULE_C]_$[LAYER_C]_H

#include <core/$[LAYER_L].h>
#include <core/datamanager.h>

class $[MODULE_W] : public $[LAYER_W] {
	IMPLEMENT_BASE;
public:
	bool initialize();
	bool deinitialize();

	bool cycle();

protected: 
};

#endif
