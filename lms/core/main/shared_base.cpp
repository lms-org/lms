#include <shared_base.h>
#include <shared_interface.h>

void Shared_Base::base_initialize(DataManager* d) {
	dm = d;
	initialize();
}

