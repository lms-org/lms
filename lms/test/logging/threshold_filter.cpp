#include "gtest/gtest.h"
#include "lms/logging/threshold_filter.h"

TEST(ThresholdFilter, decide) {
    using lms::logging::Level;

    lms::logging::ThresholdFilter filter(Level::INFO);
    filter.addPrefix("clock", Level::ERROR);
    filter.addPrefix("profiler", Level::DEBUG);

    // default threshold
    EXPECT_TRUE(filter.decide(Level::INFO, "loader"));
    EXPECT_FALSE(filter.decide(Level::DEBUG, "datamanager"));

    // prefix with high threshold
    EXPECT_TRUE(filter.decide(Level::ERROR, "clock.tooSlow"));
    EXPECT_FALSE(filter.decide(Level::WARN, "clock.cycleTime"));

    // prefix with low threshold
    EXPECT_TRUE(filter.decide(Level::DEBUG, "profiler.moduleTime"));
}
