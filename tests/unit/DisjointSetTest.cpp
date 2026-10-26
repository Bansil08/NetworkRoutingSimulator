#include <gtest/gtest.h>
#include "DisjointSet.h"

TEST(DisjointSetTest, EachElementStartsInItsOwnSet) {
    DisjointSet dsu;
    dsu.initialise({1, 2, 3});
    EXPECT_EQ(dsu.find(1), 1);
    EXPECT_EQ(dsu.find(2), 2);
    EXPECT_EQ(dsu.find(3), 3);
}

TEST(DisjointSetTest, UniteMergesSets) {
    DisjointSet dsu;
    dsu.initialise({1, 2, 3});
    EXPECT_TRUE(dsu.unite(1, 2));
    EXPECT_EQ(dsu.find(1), dsu.find(2));
    EXPECT_NE(dsu.find(1), dsu.find(3));
}

TEST(DisjointSetTest, UniteDetectsCycle) {
    DisjointSet dsu;
    dsu.initialise({1, 2, 3});
    dsu.unite(1, 2);
    dsu.unite(2, 3);
    EXPECT_FALSE(dsu.unite(1, 3));  // already connected → would form a cycle
}

TEST(DisjointSetTest, WorksWithNonContiguousIds) {
    DisjointSet dsu;
    dsu.initialise({100, -5, 42});
    EXPECT_TRUE(dsu.unite(100, -5));
    EXPECT_TRUE(dsu.unite(-5, 42));
    EXPECT_EQ(dsu.find(100), dsu.find(42));
}

TEST(DisjointSetTest, InitialiseResetsPreviousState) {
    DisjointSet dsu;
    dsu.initialise({1, 2});
    dsu.unite(1, 2);
    dsu.initialise({1, 2});
    EXPECT_NE(dsu.find(1), dsu.find(2));
}
