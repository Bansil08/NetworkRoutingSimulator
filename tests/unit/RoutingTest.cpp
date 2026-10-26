#include <gtest/gtest.h>
#include "BFSRouting.h"
#include "DijkstraRouting.h"
#include "Graph.h"
#include "TestNetwork.h"

using Path = std::vector<int>;

// ── Dijkstra ─────────────────────────────────────────────────────────────────

TEST(DijkstraRoutingTest, FindsMinimumCostPath) {
    Graph graph = sampleGraph();
    RouteResult r = DijkstraRouting().findRoute(graph, 1, 8);
    ASSERT_TRUE(r.pathFound);
    EXPECT_EQ(r.totalCost, 17);
    EXPECT_EQ(r.path.front(), 1);
    EXPECT_EQ(r.path.back(), 8);
}

TEST(DijkstraRoutingTest, PrefersCheaperLongerPath) {
    Graph graph;
    for (int id = 1; id <= 4; ++id) graph.addRouter(id, "R" + std::to_string(id));
    graph.addLink(1, 4, 10);  // direct but expensive
    graph.addLink(1, 2, 1);
    graph.addLink(2, 3, 1);
    graph.addLink(3, 4, 1);

    RouteResult r = DijkstraRouting().findRoute(graph, 1, 4);
    ASSERT_TRUE(r.pathFound);
    EXPECT_EQ(r.path, (Path{1, 2, 3, 4}));
    EXPECT_EQ(r.totalCost, 3);
}

TEST(DijkstraRoutingTest, ReportsUnreachableDestination) {
    Graph graph;
    graph.addRouter(1, "A");
    graph.addRouter(2, "B");
    RouteResult r = DijkstraRouting().findRoute(graph, 1, 2);
    EXPECT_FALSE(r.pathFound);
    EXPECT_TRUE(r.path.empty());
}

TEST(DijkstraRoutingTest, HandlesZeroWeightLinks) {
    Graph graph;
    graph.addRouter(1, "A");
    graph.addRouter(2, "B");
    graph.addLink(1, 2, 0);
    RouteResult r = DijkstraRouting().findRoute(graph, 1, 2);
    ASSERT_TRUE(r.pathFound);
    EXPECT_EQ(r.totalCost, 0);
}

// ── BFS ──────────────────────────────────────────────────────────────────────

TEST(BFSRoutingTest, FindsMinimumHopPath) {
    Graph graph = sampleGraph();
    RouteResult r = BFSRouting().findRoute(graph, 1, 8);
    ASSERT_TRUE(r.pathFound);
    EXPECT_EQ(r.path, (Path{1, 3, 5, 8}));
    EXPECT_EQ(r.totalCost, 20);  // actual weight sum along the hop path
}

TEST(BFSRoutingTest, PrefersFewerHopsOverCost) {
    Graph graph;
    for (int id = 1; id <= 4; ++id) graph.addRouter(id, "R" + std::to_string(id));
    graph.addLink(1, 4, 10);
    graph.addLink(1, 2, 1);
    graph.addLink(2, 3, 1);
    graph.addLink(3, 4, 1);

    RouteResult r = BFSRouting().findRoute(graph, 1, 4);
    ASSERT_TRUE(r.pathFound);
    EXPECT_EQ(r.path, (Path{1, 4}));
    EXPECT_EQ(r.totalCost, 10);
}

TEST(BFSRoutingTest, ReportsUnreachableDestination) {
    Graph graph;
    graph.addRouter(1, "A");
    graph.addRouter(2, "B");
    EXPECT_FALSE(BFSRouting().findRoute(graph, 1, 2).pathFound);
}

// ── Strategy pattern ─────────────────────────────────────────────────────────

TEST(RoutingStrategyTest, AlgorithmsDisagreeOnSampleNetwork) {
    Graph graph = sampleGraph();
    std::unique_ptr<RoutingStrategy> dijkstra = std::make_unique<DijkstraRouting>();
    std::unique_ptr<RoutingStrategy> bfs      = std::make_unique<BFSRouting>();

    RouteResult cheap = dijkstra->findRoute(graph, 1, 8);
    RouteResult short_ = bfs->findRoute(graph, 1, 8);

    EXPECT_LT(cheap.totalCost, short_.totalCost);
    EXPECT_GT(cheap.path.size(), short_.path.size());
}
