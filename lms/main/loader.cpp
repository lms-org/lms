#include <lms/loader.h>
#include <lms/logger.h>
#include <lms/module.h>

namespace lms {

Loader::Loader(logging::Logger &rootLogger) : logger("LOADER", &rootLogger) {
}

Loader::module_entry::module_entry() : writePriority(0), enabled(false),
  moduleInstance(nullptr) {}

}  // namespace lms
