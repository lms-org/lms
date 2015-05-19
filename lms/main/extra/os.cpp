#include <stdlib.h>

#include "lms/extra/os.h"

namespace lms {
namespace extra {

std::string username() {

#ifdef _WIN32
    const char* ENV_KEY = "USERNAME";
#elif __APPLE__
    const char* ENV_KEY = "USER";
#else
    const char* ENV_KEY = "LOGNAME";
#endif
    // Don't use getlogin_r on linux here because getenv is more flexible.
    char* logname = getenv(ENV_KEY);
    return logname != nullptr ? logname : "";
}

}  // namespace extra
}  // namespace lms
