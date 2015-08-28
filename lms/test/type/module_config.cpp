#include <sstream>
#include <string>
#include <vector>

#include "lms/type/module_config.h"
#include "gtest/gtest.h"

class ModuleConfigTest : public ::testing::Test {
protected:
    virtual void SetUp() {
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

    lms::type::ModuleConfig config;
    lms::type::ModuleConfig emptyConfig;
};

TEST_F(ModuleConfigTest, empty) {
    ASSERT_FALSE(config.empty());
    ASSERT_TRUE(emptyConfig.empty());
}

TEST_F(ModuleConfigTest, hasKey) {
    ASSERT_TRUE(config.hasKey("text"));
    ASSERT_TRUE(config.hasKey("ids"));

    ASSERT_FALSE(emptyConfig.hasKey("text"));
    ASSERT_FALSE(emptyConfig.hasKey("ids"));
}

TEST_F(ModuleConfigTest, getInt) {
    ASSERT_EQ(300, config.get<int>("width"));

    // default value
    ASSERT_EQ(10, emptyConfig.get<int>("width", 10));
}

TEST_F(ModuleConfigTest, getString) {
    ASSERT_EQ(std::string("YES"), config.get<std::string>("text"));

    // default value
    ASSERT_EQ(std::string("NO"), emptyConfig.get<std::string>("text", "NO"));
}

TEST_F(ModuleConfigTest, getBool) {
    ASSERT_FALSE(config.get<bool>("flagF"));
    ASSERT_TRUE(config.get<bool>("flagT"));

    // default value
    ASSERT_FALSE(emptyConfig.get<bool>("flagF", false));
    ASSERT_TRUE(emptyConfig.get<bool>("flagT", true));
}

TEST_F(ModuleConfigTest, getFloat) {
    ASSERT_FLOAT_EQ(7.52, config.get<float>("speed"));

    // default value
    ASSERT_FLOAT_EQ(3.2, emptyConfig.get<float>("speed", 3.2));
}

TEST_F(ModuleConfigTest, getArrayInt) {
    typedef std::vector<int> V;
    ASSERT_EQ(V({1, 2, 3, 4}), config.getArray<int>("ids"));

    // default value
    ASSERT_EQ(V(), emptyConfig.getArray<int>("ids"));
}

TEST_F(ModuleConfigTest, getArrayString) {
    typedef std::vector<std::string> V;
    ASSERT_EQ(V({"Ben", "Charles", "David"}),
        config.getArray<std::string>("users"));

    // default value
    ASSERT_EQ(V(), emptyConfig.getArray<std::string>("users"));
}
