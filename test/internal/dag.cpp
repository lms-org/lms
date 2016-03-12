#include "gtest/gtest.h"
#include "lms/internal/dag.h"

TEST(DAG, edge) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.edge(2, 3);

    EXPECT_TRUE(g.hasEdge(1, 2));
    EXPECT_TRUE(g.hasEdge(2, 3));

    // No transitive edges
    EXPECT_FALSE(g.hasEdge(1, 3));

    // No reverse edges
    EXPECT_FALSE(g.hasEdge(2, 1));
    EXPECT_FALSE(g.hasEdge(3, 2));
}

TEST(DAG, removeEdge) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.edge(2, 3);
    g.removeEdge(1, 2);

    ASSERT_FALSE(g.hasEdge(1, 2));
    ASSERT_TRUE(g.hasEdge(2, 3));
}

TEST(DAG, getFree) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.edge(1, 3);

    int result;
    ASSERT_TRUE(g.getFree(result));
    EXPECT_EQ(1, result);
}

TEST(DAG, getFreePredicate) {
    lms::internal::DAG<int> g;

    g.edge(1, 3);
    g.edge(2, 3);

    int result;
    ASSERT_TRUE(g.getFree(result, [] (int i) {
        return i % 2 == 0;
    }));
    EXPECT_EQ(2, result);

    ASSERT_FALSE(g.getFree(result, [] (int i) {
        return i > 3;
    }));
}

TEST(DAG, hasFreePredicate) {
    lms::internal::DAG<int> g;

    ASSERT_FALSE(g.hasFree([] (int x) {
        (void)x;
        return true;
    }));

    g.edge(1, 3);
    g.edge(1, 2);

    ASSERT_TRUE(g.hasFree([] (int x) {
        return x < 2;
    }));

    ASSERT_FALSE(g.hasFree([] (int x) {
        return x >= 2;
    }));

    g.edge(3, 1);

    ASSERT_FALSE(g.hasFree([] (int x) {
        (void)x;
        return true;
    }));
}

TEST(DAG, removeEdgesFrom) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.edge(1, 3);
    g.edge(2, 3);

    g.removeEdgesFrom(1);

    ASSERT_FALSE(g.hasEdge(1, 2));
    ASSERT_FALSE(g.hasEdge(1, 3));
    ASSERT_TRUE(g.hasEdge(2, 3));
}

TEST(DAG, hasCycle) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.edge(2, 3);
    ASSERT_FALSE(g.hasCycle());

    g.edge(3, 1);
    ASSERT_TRUE(g.hasCycle());
}

TEST(DAG, empty) {
    lms::internal::DAG<int> g;
    ASSERT_TRUE(g.empty());

    g.edge(1, 2);
    ASSERT_FALSE(g.empty());

    g.clear();
    ASSERT_TRUE(g.empty());

    g.node(1);
    ASSERT_FALSE(g.empty());
}

TEST(DAG, clear) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.clear();

    ASSERT_TRUE(g.empty());
}

TEST(DAG, topoSort) {
    lms::internal::DAG<int> g;
    g.edge(1, 2);
    g.edge(2, 3);
    g.edge(3, 0);

    std::vector<int> result;
    ASSERT_TRUE(g.topoSort(result));

    EXPECT_EQ(std::vector<int>({1, 2, 3, 0}), result);

    g.edge(0, 1);
    ASSERT_FALSE(g.topoSort(result));
}

TEST(DAG, countNodes) {
    lms::internal::DAG<int> g;
    ASSERT_EQ(0u, g.countNodes());

    g.edge(1, 2);
    ASSERT_EQ(2u, g.countNodes());

    g.edge(2, 3);
    ASSERT_EQ(3u, g.countNodes());
}

TEST(DAG, hasPath) {
    lms::internal::DAG<int> g;

    g.edge(1, 2);
    g.edge(2, 3);
    g.edge(2, 4);

    // loop
    ASSERT_TRUE(g.hasPath(1, 1));

    // single edge
    ASSERT_TRUE(g.hasPath(1, 2));

    // multiple edges
    ASSERT_TRUE(g.hasPath(1, 4));

    // wrong direction
    ASSERT_FALSE(g.hasPath(4, 1));

    // nodes does not exist
    ASSERT_FALSE(g.hasPath(1, 5));
}

TEST(DAG, removeTransitiveEdges) {
    lms::internal::DAG<int> g;

    g.edge(1, 2);
    g.edge(2, 3);
    g.edge(1, 3);

    g.removeTransitiveEdges();

    ASSERT_FALSE(g.hasEdge(1, 3));
    ASSERT_TRUE(g.hasEdge(1, 2));
    ASSERT_TRUE(g.hasEdge(2, 3));
}
