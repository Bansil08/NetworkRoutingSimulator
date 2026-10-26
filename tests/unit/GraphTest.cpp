#include <gtest/gtest.h>
#include "Graph.h"

class GraphTest : public ::testing::Test {
protected:
    Graph graph;

    void SetUp() override {
        graph.addRouter(1, "Alpha");
        graph.addRouter(2, "Bravo");
        graph.addRouter(3, "Charlie");
    }
};

TEST_F(GraphTest, AddRouterStoresNameAndId) {
    EXPECT_TRUE(graph.hasRouter(1));
    EXPECT_EQ(graph.getRouter(2).getName(), "Bravo");
    EXPECT_EQ(graph.getRouter(2).getId(), 2);
    EXPECT_EQ(graph.getRouterCount(), 3);
}

TEST_F(GraphTest, AddRouterAcceptsNamesWithSpaces) {
    EXPECT_EQ(graph.addRouter(9, "New York"), Status::Ok);
    EXPECT_EQ(graph.getRouter(9).getName(), "New York");
}

TEST_F(GraphTest, TrimsSurroundingWhitespaceFromNames) {
    ASSERT_EQ(graph.addRouter(9, "  Lima \t"), Status::Ok);
    EXPECT_EQ(graph.getRouter(9).getName(), "Lima");
}

TEST_F(GraphTest, RejectsDuplicateRouter) {
    EXPECT_EQ(graph.addRouter(1, "Other"), Status::DuplicateRouter);
    EXPECT_EQ(graph.getRouter(1).getName(), "Alpha");
}

TEST_F(GraphTest, RejectsEmptyRouterName) {
    EXPECT_EQ(graph.addRouter(9, ""),    Status::EmptyRouterName);
    EXPECT_EQ(graph.addRouter(9, "   "), Status::EmptyRouterName);
    EXPECT_FALSE(graph.hasRouter(9));
}

TEST_F(GraphTest, AddLinkIsUndirected) {
    ASSERT_EQ(graph.addLink(1, 2, 5), Status::Ok);
    EXPECT_TRUE(graph.hasLink(1, 2));
    EXPECT_TRUE(graph.hasLink(2, 1));
    EXPECT_EQ(graph.getLinkCount(), 1);
}

TEST_F(GraphTest, AddLinkValidatesInput) {
    EXPECT_EQ(graph.addLink(99, 1, 1), Status::SourceNotFound);
    EXPECT_EQ(graph.addLink(1, 99, 1), Status::DestinationNotFound);
    EXPECT_EQ(graph.addLink(1, 1, 1),  Status::SelfLoop);
    EXPECT_EQ(graph.addLink(1, 2, -3), Status::NegativeWeight);
    EXPECT_EQ(graph.getLinkCount(), 0);
}

TEST_F(GraphTest, RejectsDuplicateLinkInEitherDirection) {
    ASSERT_EQ(graph.addLink(1, 2, 5), Status::Ok);
    EXPECT_EQ(graph.addLink(1, 2, 7), Status::DuplicateLink);
    EXPECT_EQ(graph.addLink(2, 1, 7), Status::DuplicateLink);
    EXPECT_EQ(graph.getLinkCount(), 1);
}

TEST_F(GraphTest, RemoveLinkRemovesBothDirections) {
    graph.addLink(1, 2, 5);
    EXPECT_EQ(graph.removeLink(2, 1), Status::Ok);
    EXPECT_FALSE(graph.hasLink(1, 2));
    EXPECT_FALSE(graph.hasLink(2, 1));
    EXPECT_EQ(graph.getLinkCount(), 0);
}

TEST_F(GraphTest, RemoveMissingLinkFails) {
    EXPECT_EQ(graph.removeLink(1, 3), Status::LinkNotFound);
}

TEST_F(GraphTest, GetAllEdgesListsEachLinkOnce) {
    graph.addLink(1, 2, 5);
    graph.addLink(2, 3, 7);
    auto edges = graph.getAllEdges();
    ASSERT_EQ(edges.size(), 2u);
    for (const Link& e : edges) EXPECT_LT(e.source, e.destination);
}

TEST_F(GraphTest, SortedRouterIds) {
    graph.addRouter(0, "Zero");
    EXPECT_EQ(graph.getSortedRouterIds(), (std::vector<int>{0, 1, 2, 3}));
}

TEST_F(GraphTest, NeighboursOfUnknownRouterIsEmpty) {
    EXPECT_TRUE(graph.getNeighbours(42).empty());
}

TEST_F(GraphTest, ClearResetsEverything) {
    graph.addLink(1, 2, 5);
    graph.clear();
    EXPECT_EQ(graph.getRouterCount(), 0);
    EXPECT_EQ(graph.getLinkCount(), 0);
}
