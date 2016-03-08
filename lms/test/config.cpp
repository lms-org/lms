#include <sstream>
#include <vector>
#include <cmath>

#include "lms/config.h"
#include "lms/time.h"
#include "gtest/gtest.h"

class ConfigTest : public ::testing::Test {
protected:
    virtual void SetUp() override {
        std::istringstream iss(
        "# This is a comment\n"
        "width = 300\n"
        "text = YES\n"
        "flagF = false\n"
        "#AnotherComment\n"
        "flagT=true\r\n"
        "\n"
        "ids = 1, 2,\\\n"
        "3, 4\n"
        "users = Ben, Charles, David\\\n"
        "\n"
        "speed = 7.52\n"
        "duration=12s 10ms\n"
        "dist=12cm -3mm\n"
        "angle=180deg"
        );

        config.load(iss);
    }

    lms::Config config;
    lms::Config emptyConfig;
};

TEST_F(ConfigTest, empty) {
    ASSERT_FALSE(config.empty());
    ASSERT_TRUE(emptyConfig.empty());
}

TEST_F(ConfigTest, hasKey) {
    ASSERT_TRUE(config.hasKey("text"));
    ASSERT_TRUE(config.hasKey("ids"));

    ASSERT_FALSE(emptyConfig.hasKey("text"));
    ASSERT_FALSE(emptyConfig.hasKey("ids"));
}

TEST_F(ConfigTest, getInt) {
    ASSERT_EQ(300, config.get<int>("width", 0));

    // default value
    ASSERT_EQ(10, emptyConfig.get<int>("width", 10));
}

TEST_F(ConfigTest, getString) {
    ASSERT_EQ(std::string("YES"), config.get<std::string>("text", ""));

    // default value
    ASSERT_EQ(std::string("NO"), emptyConfig.get<std::string>("text", "NO"));
}

TEST_F(ConfigTest, getBool) {
    ASSERT_FALSE(config.get<bool>("flagF", false));
    ASSERT_TRUE(config.get<bool>("flagT", false));

    // default value
    ASSERT_FALSE(emptyConfig.get<bool>("flagF", false));
    ASSERT_TRUE(emptyConfig.get<bool>("flagT", true));
}

TEST_F(ConfigTest, getFloat) {
    ASSERT_FLOAT_EQ(7.52, config.get<float>("speed", 0));

    // default value
    ASSERT_FLOAT_EQ(3.2, emptyConfig.get<float>("speed", 3.2));
}

TEST_F(ConfigTest, getArrayInt) {
    typedef std::vector<int> V;
    ASSERT_EQ(V({1, 2, 3, 4}), config.getArray<V::value_type>("ids", {}));

    // default value
    ASSERT_EQ(V(), emptyConfig.getArray<V::value_type>("ids", {}));

    // default value
    ASSERT_EQ(V({1,2}), emptyConfig.getArray<V::value_type>("ids", V({1,2})));
}

TEST_F(ConfigTest, getArrayString) {
    typedef std::vector<std::string> V;
    ASSERT_EQ(V({"Ben", "Charles", "David"}),
              config.getArray<V::value_type>("users", {}));

    // default value
    ASSERT_EQ(V(), emptyConfig.getArray<V::value_type>("users", {}));
}

/* TODO uncomment when supported again

TEST_F(ConfigTest, getTime) {
    ASSERT_EQ(lms::Time::fromMillis(12010), config.get<lms::Time>("duration"));
}

TEST_F(ConfigTest, getDistance) {
    ASSERT_FLOAT_EQ(0.117f, config.get<lms::Distance>("dist").toSI());
}

TEST_F(ConfigTest, getAngle) {
    ASSERT_FLOAT_EQ(180, config.get<lms::Angle>("angle").toDeg());
    ASSERT_FLOAT_EQ(M_PI, config.get<lms::Angle>("angle").toRad());
}
*/
