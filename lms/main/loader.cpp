#include <lms/loader.h>
#include <lms/logger.h>
#include <lms/module.h>

namespace lms {

Loader::Loader(logging::Logger &rootLogger) : logger("LOADER", &rootLogger) {
}

}  // namespace lms
