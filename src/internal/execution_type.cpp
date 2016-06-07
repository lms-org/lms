#include "lms/execution_type.h"

namespace lms {

std::string executionTypeName(ExecutionType type) {
    std::string result;

    switch (type) {
    case ExecutionType::NEVER_MAIN_THREAD:
        result = "NEVER_MAIN_THREAD";
        break;
    case ExecutionType::ONLY_MAIN_THREAD:
        result = "ONLY_MAIN_THREAD";
        break;
    }

    return result;
}

bool executionTypeByName(const std::string &name, ExecutionType &type) {
    if (name == "NEVER_MAIN_THREAD") {
        type = ExecutionType::NEVER_MAIN_THREAD;
    } else if (name == "ONLY_MAIN_THREAD") {
        type = ExecutionType::ONLY_MAIN_THREAD;
    } else {
        return false;
    }

    return true;
}

} // namespace lms
