#include "lms/clock.h"

namespace lms {

Clock::Clock(logging::Logger &rootLogger, extra::PrecisionTime loopTime)
    : logger("Clock", &rootLogger), loopTime(loopTime),
    bufferInsertIndex(0), bufferSize(0) {
}

void Clock::beforeLoopIteration() {
    beforeLoopTimestamp = extra::PrecisionTime::now();
}

void Clock::afterLoopIteration() {
    extra::PrecisionTime averageLoopTime;
    if(bufferSize == 0) {
        averageLoopTime = loopTime; // TODO not really clever
    } else {
        averageLoopTime = lastIterationsSum / bufferSize;
    }

    extra::PrecisionTime sleepTime = loopTime - averageLoopTime;

    if(sleepTime.micros() > 0) {
        logger.info("afterLoopIteration") << "Sleep for " << sleepTime;
        sleepTime.sleep();
    } else {
        // TODO warn only if threshold is hit
        logger.warn("afterLoopIteration") << "Too slow: " << averageLoopTime;
    }

    // save loop iteration time
    // TODO do this in beforeLoopIteration
    extra::PrecisionTime delta = extra::PrecisionTime::now() - beforeLoopTimestamp;

    // to avoid implementing a loop to sum all data in lastIterations
    // we optimize this here a bit:
    // subtract the value that will be overwritten and add the new value
    if(bufferSize == lastIterations.size()) {
        lastIterationsSum += delta - lastIterations[bufferInsertIndex];
    } else {
        lastIterationsSum += delta;
    }
    lastIterations[bufferInsertIndex] = delta;
    bufferInsertIndex ++;
    bufferInsertIndex %= lastIterations.size();
    if(bufferSize < lastIterations.size()) {
        bufferSize++;
    }
}

}  // namespace lms
