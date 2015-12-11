#include <lms/loader.h>
#include <lms/module.h>

namespace lms {

std::string Loader::getModulePath(const std::string &libname) {
    return libname + ".dll";
}

bool Loader::checkSharedLibrary(const std::string &libpath) {
	logger.error("checkSharedLibrary") << "Not implemented on Win32";
	return false;
}

Module* Loader::load(module_entry& entry) {
	logger.error("load") << "Not implemented on Win32";
	return nullptr;
}

}  // namespace lms
