#include <lms/extra/file_monitor.h>

namespace lms {
namespace extra {

FileMonitor::FileMonitor() {
}

FileMonitor::~FileMonitor() {
}

FileMonitor::operator bool () const {
    return false;
}

bool FileMonitor::watch(const std::string &path) {
	// TODO Fabian
    return false;
}

void FileMonitor::unwatchAll() {
	// TODO Fabian
}

bool FileMonitor::hasChangedFiles() {
	// TODO Fabian
    return false;
}

} //extra
} //lms

