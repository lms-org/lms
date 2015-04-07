#ifndef LMS_SCHEDULER_H
#define LMS_SCHEDULER_H

#include <array>

#include "lms/extra/time.h"
#include "lms/logger.h"

namespace lms {

class Clock {
public:
    Clock(logging::Logger &rootLogger, extra::PrecisionTime loopTime);
    void beforeLoopIteration();
    void afterLoopIteration();
private:
    logging::ChildLogger logger;

    extra::PrecisionTime loopTime;
    extra::PrecisionTime beforeLoopTimestamp;

    std::array<extra::PrecisionTime, 10> lastIterations;
    extra::PrecisionTime lastIterationsSum;
    int bufferInsertIndex;
    unsigned int bufferSize;
};

}  // namespace lms

#endif /* LMS_SCHEDULER_H */
