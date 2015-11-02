#include "gtest/gtest.h"
#include "lms/buffered_datamanager.h"

TEST(BufferedDataManager, pushPop) {
    lms::BufferedDataManager dm;

    dm.push("test", 12);
    dm.push("test", 4);

    dm.push("strings", std::string("A"));

    std::vector<int> results;
    dm.popAll("test", results);

    ASSERT_EQ(2, results.size());
    EXPECT_EQ(12, results[0]);
    EXPECT_EQ(4, results[1]);

    dm.push("test", 1);
    dm.popAll("test", results);

    ASSERT_EQ(1, results.size());
    EXPECT_EQ(1, results[0]);

    std::vector<std::string> resultStrings;
    dm.popAll("strings", resultStrings);

    ASSERT_EQ(1, resultStrings.size());
    EXPECT_EQ(std::string("A"), resultStrings[0]);
}
