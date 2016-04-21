#include <gtest/gtest.h>
#include <lms/internal/module_channel_graph.h>

TEST(ModuleChannelGraph, readChannel) {
    using MCG = lms::internal::ModuleChannelGraph<int>;
    MCG mcg;

    mcg.readChannel("INPUT", 1, 0);

    ASSERT_TRUE(mcg.isReaderOrWriter("INPUT", 1));
    ASSERT_FALSE(mcg.isReaderOrWriter("INPUT", 2));
    ASSERT_FALSE(mcg.isReaderOrWriter("OUTPUT", 1));
}

TEST(ModuleChannelGraph, writeChannel) {
    lms::internal::ModuleChannelGraph<int> mcg;

    mcg.writeChannel("OUTPUT", 2, 0);

    ASSERT_TRUE(mcg.isReaderOrWriter("OUTPUT", 2));
    ASSERT_FALSE(mcg.isReaderOrWriter("OUTPUT", 1));
    ASSERT_FALSE(mcg.isReaderOrWriter("INPUT", 2));
}

TEST(ModuleChannelGraph, generateDAG) {
    lms::internal::ModuleChannelGraph<int> mcg;

    mcg.writeChannel("A", 1);
    mcg.readChannel("A", 2);
    mcg.writeChannel("B", 2);
    mcg.readChannel("A", 3);
    mcg.writeChannel("C", 3);
    mcg.readChannel("B", 4);
    mcg.readChannel("C", 4);

    using V = std::vector<int>;
    V sortedList;
    mcg.generateDAG().topoSort(sortedList);

    ASSERT_TRUE(V({1,2,3,4}) == sortedList || V({1,3,2,4}) == sortedList);
}

TEST(ModuleChannelGraph, generateDAG_writePriorities) {
    lms::internal::ModuleChannelGraph<int> mcg;

    mcg.writeChannel("A", 1, 1);
    mcg.writeChannel("A", 2, 2);
    mcg.writeChannel("A", 3, 0);
    mcg.readChannel("A", 4);

    using V = std::vector<int>;
    V sortedList;
    mcg.generateDAG().topoSort(sortedList);

    ASSERT_EQ(V({2,1,3,4}), sortedList);
}

TEST(ModuleChannelGraph, generateDAG_readPriorities) {
    lms::internal::ModuleChannelGraph<int> mcg;

    mcg.writeChannel("A", 1);
    mcg.writeChannel("A", 2, 1);
    mcg.readChannel("A", 3, 1);
    mcg.readChannel("A", 4);

    using V = std::vector<int>;
    V sortedList;
    mcg.generateDAG().topoSort(sortedList);

    ASSERT_EQ(V({2, 3, 1, 4}), sortedList);
}
