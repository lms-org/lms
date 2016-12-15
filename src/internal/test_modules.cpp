#include "test_modules.h"

#include <unistd.h>

namespace lms {
namespace internal {

bool NumberGenerator::init() {
    currentNumber = config().get<int>("start", 0);
    out = writeChannel<int>("OUT");
    return true;
}

bool NumberGenerator::cycle() {
    ::usleep(config().get<int>("sleep", 1000000));  // 10 ms
    *out = currentNumber;
    currentNumber += config().get<int>("step", 1);
    return true;
}

void NumberGenerator::destroy() { logger.info() << "destroy()"; }

NumberGenerator::~NumberGenerator() { logger.info() << "destructor"; }

bool Multiply::init() {
    factor1 = readChannel<int>("FACTOR_1");
    factor2 = readChannel<int>("FACTOR_2");
    product = writeChannel<int>("PRODUCT");

    return true;
}

bool Multiply::cycle() {
    ::usleep(config().get<int>("sleep", 1000000));  // 10 ms
    *product = (*factor1) * (*factor2);
    return true;
}

void Multiply::destroy() { logger.info() << "destroy()"; }

Multiply::~Multiply() { logger.info() << "destructor"; }

bool Display::init() {
    in = readChannel<int>("IN");
    return true;
}

bool Display::cycle() {
    ::usleep(config().get<int>("sleep", 1000000));  // 10 ms
    logger.info() << *in;
    return true;
}

void Display::destroy() { logger.info() << "destroy()"; }

Display::~Display() { logger.info() << "destructor"; }

bool ExitAfter::init() { return true; }

bool ExitAfter::cycle() {
    if (cycleCounter() == config().get<int>("cycles", -1)) {
        exitRuntime();
    }
    return true;
}

void ExitAfter::destroy() {}

}  // namespace internal
}  // namespace lms

LMS_EXPORT_BEGIN
LMS_EXPORT_MODULE_NS(lms::internal, Multiply)
LMS_EXPORT_MODULE_NS(lms::internal, NumberGenerator)
LMS_EXPORT_MODULE_NS(lms::internal, Display)
LMS_EXPORT_MODULE_NS(lms::internal, ExitAfter)
LMS_EXPORT_END

LMS_REGISTER_BEGIN
LMS_REGISTER(int)
LMS_REGISTER_END
