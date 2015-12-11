#include <ratio>

#include "gtest/gtest.h"
#include "lms/time.h"

TEST(Time, fromMicros) {
    using lms::Time;

    EXPECT_EQ(0, Time::fromMicros(0).micros());
    EXPECT_EQ(1, Time::fromMicros(1).micros());
    EXPECT_EQ(-1, Time::fromMicros(-1).micros());
}

TEST(Time, fromMillis) {
    using lms::Time;

    EXPECT_EQ(0, Time::fromMillis(0).micros());
    EXPECT_EQ(1000, Time::fromMillis(1).micros());
    EXPECT_EQ(-1000, Time::fromMillis(-1).micros());
}

TEST(Time, from) {
    using lms::Time;

    EXPECT_EQ(0, (Time::from<std::ratio<1,1>>(0).micros()));

    EXPECT_EQ(1L, (Time::from<std::micro>(1).micros()));
    EXPECT_EQ(2000L, (Time::from<std::micro>(2000L).micros()));
    EXPECT_EQ(3000000L, (Time::from<std::micro>(3000000L).micros()));
    EXPECT_EQ(4000L, (Time::from<std::milli>(4).micros()));
    EXPECT_EQ(5000000L, (Time::from<std::milli>(5000).micros()));
    EXPECT_EQ(6000000L, (Time::from<std::ratio<1,1>>(6).micros()));

    EXPECT_EQ(-7L, (Time::from<std::micro>(-7).micros()));
    EXPECT_EQ(-8000L, (Time::from<std::milli>(-8).micros()));
    EXPECT_EQ(-9000000L, (Time::from<std::ratio<1,1>>(-9).micros()));
}

TEST(Time, toFloat) {
    using lms::Time;

    EXPECT_FLOAT_EQ(0, Time().toFloat());

    EXPECT_FLOAT_EQ(1, Time::fromMicros(1).toFloat<std::micro>());
    EXPECT_FLOAT_EQ(0.001f, Time::fromMicros(1).toFloat<std::milli>());
    EXPECT_FLOAT_EQ(0.000001f, (Time::fromMicros(1).toFloat<std::ratio<1,1>>()));
}

TEST(Time, writable) {
    using lms::Time;

    std::ostringstream oss;
    oss << Time();

    ASSERT_FALSE(oss.str().empty());
}

// ===================
// This is ridiculous!

typedef std::pair<lms::Time::TimeType, lms::Time::TimeType> TimePair;

class TimeTest : public ::testing::TestWithParam<TimePair> {
};

INSTANTIATE_TEST_CASE_P(Time2, TimeTest,
                        ::testing::Values(TimePair(0,0), TimePair(0,1), TimePair(0,-1), TimePair(1,0),
                                          TimePair(-1,0), TimePair(-1,1), TimePair(1,-1), TimePair(1,1)));

#define PRECISION_TIME_ARITHMETIC_TEST(NAME, OPERATOR) \
TEST_P(TimeTest, NAME) { \
    using lms::Time; \
    ASSERT_EQ(Time::fromMicros(GetParam().first OPERATOR GetParam().second), \
              Time::fromMicros(GetParam().first) OPERATOR \
              Time::fromMicros(GetParam().second)); \
} \
TEST_P(TimeTest, NAME ## Eq) { \
    using lms::Time; \
    Time a = Time::fromMicros(GetParam().first); \
    Time b = Time::fromMicros(GetParam().second); \
    Time result = Time::fromMicros(GetParam().first OPERATOR GetParam().second); \
    ASSERT_EQ(result, a OPERATOR ## = b); \
    ASSERT_EQ(result, a); \
}

PRECISION_TIME_ARITHMETIC_TEST(add, +)
PRECISION_TIME_ARITHMETIC_TEST(sub, -)

#undef PRECISION_TIME_ARITHMETIC_TEST

#define PRECISION_TIME_SCALAR_TEST(NAME, OPERATOR, PREVENT_ZERO) \
TEST_P(TimeTest, NAME) { \
    using lms::Time; \
    if(PREVENT_ZERO && GetParam().second == 0) SUCCEED(); else \
    ASSERT_EQ(Time::fromMicros(GetParam().first OPERATOR GetParam().second), \
              Time::fromMicros(GetParam().first) OPERATOR GetParam().second); \
} \
TEST_P(TimeTest, NAME ## Eq) { \
    using lms::Time; \
    if(PREVENT_ZERO && GetParam().second == 0) SUCCEED(); else {\
        Time a = Time::fromMicros(GetParam().first); \
        Time::TimeType b = GetParam().second; \
        Time result = Time::fromMicros(GetParam().first OPERATOR GetParam().second); \
        ASSERT_EQ(result, a OPERATOR ## = b); \
        ASSERT_EQ(result, a); \
    } \
}

PRECISION_TIME_SCALAR_TEST(mul, *, 0)
PRECISION_TIME_SCALAR_TEST(div, /, 1)

#undef PRECISION_TIME_SCALAR_TEST

#define PRECISION_TIME_COMP_OP_TEST(NAME, OPERATOR) \
TEST_P(TimeTest, NAME) { \
    using lms::Time; \
    ASSERT_EQ(GetParam().first OPERATOR GetParam().second, \
              Time::fromMicros(GetParam().first) OPERATOR Time::fromMicros(GetParam().second)); \
}

PRECISION_TIME_COMP_OP_TEST(gt, >)
PRECISION_TIME_COMP_OP_TEST(ge, >=)
PRECISION_TIME_COMP_OP_TEST(lt, <)
PRECISION_TIME_COMP_OP_TEST(le, <=)
PRECISION_TIME_COMP_OP_TEST(eq, ==)
PRECISION_TIME_COMP_OP_TEST(ne, !=)

#undef PRECISION_TIME_COMP_OP_TEST
