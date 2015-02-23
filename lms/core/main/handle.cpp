#include <core/handle.h>
#include <core/datamanager.h>

int GenericHandle::getSize() {
    return datamanager()->get_channel_info(this).length;
}

void GenericHandle::get(int handle, void** data) {
	datamanager()->get_channel(handle, data);
}
