#include <sstream>
#include <vector>

#include "lms/config.h"
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
        "flagT=1\r\n"
        "\n"
        "ids = 1, 2,\\\n"
        "3, 4\n"
        "users = Ben, Charles, David\\\n"
        "\n"
        "speed = 7.52\n"
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
    ASSERT_EQ(300, config.get<int>("width"));

    // default value
    ASSERT_EQ(10, emptyConfig.get<int>("width", 10));
}

TEST_F(ConfigTest, getString) {
    ASSERT_EQ(std::string("YES"), config.get<std::string>("text"));

    // default value
    ASSERT_EQ(std::string("NO"), emptyConfig.get<std::string>("text", "NO"));
}

TEST_F(ConfigTest, getBool) {
    ASSERT_FALSE(config.get<bool>("flagF"));
    ASSERT_TRUE(config.get<bool>("flagT"));

    // default value
    ASSERT_FALSE(emptyConfig.get<bool>("flagF", false));
    ASSERT_TRUE(emptyConfig.get<bool>("flagT", true));
}

TEST_F(ConfigTest, getFloat) {
    ASSERT_FLOAT_EQ(7.52, config.get<float>("speed"));

    // default value
    ASSERT_FLOAT_EQ(3.2, emptyConfig.get<float>("speed", 3.2));
}

TEST_F(ConfigTest, getArrayInt) {
    typedef std::vector<int> V;
    ASSERT_EQ(V({1, 2, 3, 4}), config.getArray<int>("ids"));

    // default value
    ASSERT_EQ(V(), emptyConfig.getArray<int>("ids"));
}

TEST_F(ConfigTest, getArrayString) {
    typedef std::vector<std::string> V;
    ASSERT_EQ(V({"Ben", "Charles", "David"}),
        config.getArray<std::string>("users"));

    // default value
    ASSERT_EQ(V(), emptyConfig.getArray<std::string>("users"));
}
