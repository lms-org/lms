#include <ratio>

#include "gtest/gtest.h"
#include "lms/extra/time.h"

TEST(PrecisionTime, fromMicros) {
    using lms::extra::PrecisionTime;

    EXPECT_EQ(0, PrecisionTime::fromMicros(0).micros());
    EXPECT_EQ(1, PrecisionTime::fromMicros(1).micros());
    EXPECT_EQ(-1, PrecisionTime::fromMicros(-1).micros());
}

TEST(PrecisionTime, fromMillis) {
    using lms::extra::PrecisionTime;

    EXPECT_EQ(0, PrecisionTime::fromMillis(0).micros());
    EXPECT_EQ(1000, PrecisionTime::fromMillis(1).micros());
    EXPECT_EQ(-1000, PrecisionTime::fromMillis(-1).micros());
}

TEST(PrecisionTime, from) {
    using lms::extra::PrecisionTime;

    EXPECT_EQ(0, (PrecisionTime::from<std::ratio<1,1>>(0).micros()));

    EXPECT_EQ(1L, (PrecisionTime::from<std::micro>(1).micros()));
    EXPECT_EQ(2000L, (PrecisionTime::from<std::micro>(2000L).micros()));
    EXPECT_EQ(3000000L, (PrecisionTime::from<std::micro>(3000000L).micros()));
    EXPECT_EQ(4000L, (PrecisionTime::from<std::milli>(4).micros()));
    EXPECT_EQ(5000000L, (PrecisionTime::from<std::milli>(5000).micros()));
    EXPECT_EQ(6000000L, (PrecisionTime::from<std::ratio<1,1>>(6).micros()));

    EXPECT_EQ(-7L, (PrecisionTime::from<std::micro>(-7).micros()));
    EXPECT_EQ(-8000L, (PrecisionTime::from<std::milli>(-8).micros()));
    EXPECT_EQ(-9000000L, (PrecisionTime::from<std::ratio<1,1>>(-9).micros()));
}

TEST(PrecisionTime, toFloat) {
    using lms::extra::PrecisionTime;

    EXPECT_FLOAT_EQ(0, PrecisionTime().toFloat());

    EXPECT_FLOAT_EQ(1, PrecisionTime::fromMicros(1).toFloat<std::micro>());
    EXPECT_FLOAT_EQ(0.001f, PrecisionTime::fromMicros(1).toFloat<std::milli>());
    EXPECT_FLOAT_EQ(0.000001f, (PrecisionTime::fromMicros(1).toFloat<std::ratio<1,1>>()));
}

TEST(PrecisionTime, writable) {
    using lms::extra::PrecisionTime;

    std::ostringstream oss;
    oss << PrecisionTime();

    ASSERT_FALSE(oss.str().empty());
}

// ===================
// This is ridiculous!

typedef std::pair<lms::extra::PrecisionTime::TimeType,
        lms::extra::PrecisionTime::TimeType> TimePair;

class PrecisionTimeTest : public ::testing::TestWithParam<TimePair> {
};

INSTANTIATE_TEST_CASE_P(PrecisionTime2, PrecisionTimeTest,
                        ::testing::Values(TimePair(0,0), TimePair(0,1), TimePair(0,-1), TimePair(1,0),
                                          TimePair(-1,0), TimePair(-1,1), TimePair(1,-1), TimePair(1,1)));

#define PRECISION_TIME_ARITHMETIC_TEST(NAME, OPERATOR) \
TEST_P(PrecisionTimeTest, NAME) { \
    using lms::extra::PrecisionTime; \
    ASSERT_EQ(PrecisionTime::fromMicros(GetParam().first OPERATOR GetParam().second), \
              PrecisionTime::fromMicros(GetParam().first) OPERATOR \
              PrecisionTime::fromMicros(GetParam().second)); \
} \
TEST_P(PrecisionTimeTest, NAME ## Eq) { \
    using lms::extra::PrecisionTime; \
    PrecisionTime a = PrecisionTime::fromMicros(GetParam().first); \
    PrecisionTime b = PrecisionTime::fromMicros(GetParam().second); \
    PrecisionTime result = PrecisionTime::fromMicros(GetParam().first OPERATOR GetParam().second); \
    ASSERT_EQ(result, a OPERATOR ## = b); \
    ASSERT_EQ(result, a); \
}

PRECISION_TIME_ARITHMETIC_TEST(add, +)
PRECISION_TIME_ARITHMETIC_TEST(sub, -)

#undef PRECISION_TIME_ARITHMETIC_TEST

#define PRECISION_TIME_SCALAR_TEST(NAME, OPERATOR, PREVENT_ZERO) \
TEST_P(PrecisionTimeTest, NAME) { \
    using lms::extra::PrecisionTime; \
    if(PREVENT_ZERO && GetParam().second == 0) SUCCEED(); else \
    ASSERT_EQ(PrecisionTime::fromMicros(GetParam().first OPERATOR GetParam().second), \
              PrecisionTime::fromMicros(GetParam().first) OPERATOR GetParam().second); \
} \
TEST_P(PrecisionTimeTest, NAME ## Eq) { \
    using lms::extra::PrecisionTime; \
    if(PREVENT_ZERO && GetParam().second == 0) SUCCEED(); else {\
        PrecisionTime a = PrecisionTime::fromMicros(GetParam().first); \
        PrecisionTime::TimeType b = GetParam().second; \
        PrecisionTime result = PrecisionTime::fromMicros(GetParam().first OPERATOR GetParam().second); \
        ASSERT_EQ(result, a OPERATOR ## = b); \
        ASSERT_EQ(result, a); \
    } \
}

PRECISION_TIME_SCALAR_TEST(mul, *, 0)
PRECISION_TIME_SCALAR_TEST(div, /, 1)

#undef PRECISION_TIME_SCALAR_TEST

#define PRECISION_TIME_COMP_OP_TEST(NAME, OPERATOR) \
TEST_P(PrecisionTimeTest, NAME) { \
    using lms::extra::PrecisionTime; \
    ASSERT_EQ(GetParam().first OPERATOR GetParam().second, \
              PrecisionTime::fromMicros(GetParam().first) OPERATOR PrecisionTime::fromMicros(GetParam().second)); \
}

PRECISION_TIME_COMP_OP_TEST(gt, >)
PRECISION_TIME_COMP_OP_TEST(ge, >=)
PRECISION_TIME_COMP_OP_TEST(lt, <)
PRECISION_TIME_COMP_OP_TEST(le, <=)
PRECISION_TIME_COMP_OP_TEST(eq, ==)
PRECISION_TIME_COMP_OP_TEST(ne, !=)

#undef PRECISION_TIME_COMP_OP_TEST