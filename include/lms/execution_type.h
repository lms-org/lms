#ifndef LMS_EXECUTION_TYPE_H
#define LMS_EXECUTION_TYPE_H

#include <iostream>

namespace lms {

enum class ExecutionType { ONLY_MAIN_THREAD, NEVER_MAIN_THREAD };

std::string executionTypeName(ExecutionType type);

bool executionTypeByName(const std::string &name, ExecutionType &type);

} // namespace lms

#endif // LMS_EXECUTION_TYPE_H
