#include "profiler.h"

namespace lms {
namespace internal {

void Profiler::addBegin(const std::string &tag, lms::Time timestamp) {
    // memorize new begin timestamp
    measurements[tag].updateBegin(timestamp);
}

void Profiler::addEnd(const std::string &tag, lms::Time timestamp) {
    measurements[tag].updateEnd(timestamp);
}

void Profiler::getOverview(std::map<std::string, logging::Trace<double>> &result) const {
    result = measurements;
}

void Profiler::reset() {
    for(auto &pair : measurements) {
        pair.second.reset();
    }
}

}
}
