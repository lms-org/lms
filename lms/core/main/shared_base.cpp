#include <core/shared_base.h>
#include <core/shared_interface.h>

namespace lms{
void Shared_Base::base_initialize(DataManager* d) {
	dm = d;
	initialize();
}
}
