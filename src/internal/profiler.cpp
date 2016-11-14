#include "profiler.h"

namespace lms {
namespace internal {

void Profiler::addBegin(const std::string &tag, lms::Time timestamp) {
    // memorize new begin timestamp
    beginTimes[tag] = timestamp;
}

void Profiler::addEnd(const std::string &tag, lms::Time timestamp) {
    auto it = beginTimes.find(tag);
    if(it == beginTimes.end()) {
        // could not find begin time
    } else {
        lms::Time begin = it->second;
        auto &trace = measurements[tag];
        trace.update((timestamp - begin).micros());
        beginTimes.erase(it);
    }
}

void Profiler::getOverview(std::map<std::string, logging::Trace<float>> &result) const {
    // TODO running since

    for(const auto &pair : measurements) {
        result[pair.first] = pair.second;
    }
    // add all beginTimes that are not already in measurements
    for(const auto &pair : beginTimes) {
        auto it = measurements.find(pair.first);
        if(it == measurements.end()) {
            logging::Trace<float> trace;
            result[pair.first] = trace;
        }
    }
}

void Profiler::reset() {
    beginTimes.clear();
    measurements.clear();
}

}
}
