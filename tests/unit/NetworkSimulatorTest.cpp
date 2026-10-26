#include <gtest/gtest.h>
#include <sstream>
#include "BFSRouting.h"
#include "NetworkSimulator.h"
#include "TestNetwork.h"

class NetworkSimulatorTest : public ::testing::Test {
protected:
    NetworkSimulator sim;

    void SetUp() override { loadSample(sim); }

    void isolateHotel() {
        sim.removeLink(5, 8);
        sim.removeLink(6, 8);
        sim.removeLink(7, 8);
    }
};

// ── Routing ──────────────────────────────────────────────────────────────────

TEST_F(NetworkSimulatorTest, DefaultsToDijkstra) {
    EXPECT_EQ(sim.getAlgorithmName(), "Dijkstra (Minimum Cost)");
    EXPECT_EQ(sim.findRoute(1, 8).route.totalCost, 17);
}

TEST_F(NetworkSimulatorTest, StrategyCanBeSwappedAtRuntime) {
    sim.setRoutingStrategy(std::make_unique<BFSRouting>());
    EXPECT_EQ(sim.getAlgorithmName(), "BFS (Minimum Hops)");
    EXPECT_EQ(sim.findRoute(1, 8).route.totalCost, 20);
}

TEST_F(NetworkSimulatorTest, FindRouteValidatesRouters) {
    EXPECT_EQ(sim.findRoute(99, 1).status, Status::SourceNotFound);
    EXPECT_EQ(sim.findRoute(1, 99).status, Status::DestinationNotFound);
}

TEST(NetworkSimulatorEmptyTest, FindRouteOnEmptyNetwork) {
    NetworkSimulator empty;
    EXPECT_EQ(empty.findRoute(1, 2).status, Status::EmptyNetwork);
}

TEST_F(NetworkSimulatorTest, RouteToSelfIsTrivial) {
    RouteQuery q = sim.findRoute(3, 3);
    ASSERT_EQ(q.status, Status::Ok);
    EXPECT_TRUE(q.route.pathFound);
    EXPECT_EQ(q.route.path, (std::vector<int>{3}));
    EXPECT_EQ(q.route.totalCost, 0);
}

// ── Packets & statistics ─────────────────────────────────────────────────────

TEST_F(NetworkSimulatorTest, SendPacketUpdatesStatistics) {
    PacketReport first  = sim.sendPacket(1, 8);
    PacketReport second = sim.sendPacket(1, 99);

    EXPECT_TRUE(first.delivered());
    EXPECT_FALSE(second.delivered());
    EXPECT_EQ(first.packetId, 1);
    EXPECT_EQ(second.packetId, 2);

    Statistics s = sim.getStatistics();
    EXPECT_EQ(s.totalPacketsSent, 2);
    EXPECT_EQ(s.successfulPackets, 1);
    EXPECT_EQ(s.failedPackets, 1);
}

TEST_F(NetworkSimulatorTest, PacketToIsolatedRouterFails) {
    isolateHotel();
    PacketReport r = sim.sendPacket(1, 8);
    EXPECT_EQ(r.status, Status::Ok);
    EXPECT_FALSE(r.route.pathFound);
    EXPECT_FALSE(r.delivered());
}

// ── Link failure ─────────────────────────────────────────────────────────────

TEST_F(NetworkSimulatorTest, LinkFailureOnRouteTriggersReroute) {
    sim.sendPacket(1, 8);  // 1→3→5→7→8, cost 17
    LinkFailureReport r = sim.simulateLinkFailure(5, 7);

    ASSERT_EQ(r.status, Status::Ok);
    EXPECT_TRUE(r.affectedLastRoute);
    EXPECT_EQ(r.previousRoute.totalCost, 17);
    ASSERT_TRUE(r.newRoute.pathFound);
    EXPECT_EQ(r.newRoute.totalCost, 19);
    EXPECT_FALSE(sim.getNetwork().hasLink(5, 7));
    EXPECT_EQ(sim.getStatistics().reroutedPackets, 1);
}

TEST_F(NetworkSimulatorTest, LinkFailureOffRouteNeedsNoReroute) {
    sim.sendPacket(1, 8);
    LinkFailureReport r = sim.simulateLinkFailure(1, 4);
    EXPECT_EQ(r.status, Status::Ok);
    EXPECT_FALSE(r.affectedLastRoute);
    EXPECT_FALSE(sim.getNetwork().hasLink(1, 4));
    EXPECT_EQ(sim.getStatistics().reroutedPackets, 0);
}

TEST_F(NetworkSimulatorTest, LinkFailureWithNoAlternativeIsUnreachable) {
    sim.removeLink(5, 8);
    sim.removeLink(6, 8);
    sim.sendPacket(1, 8);  // only remaining path goes through 7-8
    LinkFailureReport r = sim.simulateLinkFailure(7, 8);
    EXPECT_TRUE(r.affectedLastRoute);
    EXPECT_FALSE(r.newRoute.pathFound);
}

TEST_F(NetworkSimulatorTest, LinkFailureOnMissingLink) {
    EXPECT_EQ(sim.simulateLinkFailure(1, 8).status, Status::LinkNotFound);
}

// ── Connectivity & MST ───────────────────────────────────────────────────────

TEST_F(NetworkSimulatorTest, SampleNetworkIsConnected) {
    ConnectivityReport r = sim.checkConnectivity();
    EXPECT_TRUE(r.isConnected());
    EXPECT_TRUE(r.unreachable.empty());
}

TEST_F(NetworkSimulatorTest, DetectsDisconnectedRouter) {
    isolateHotel();
    ConnectivityReport r = sim.checkConnectivity();
    EXPECT_FALSE(r.isConnected());
    EXPECT_EQ(r.reachableCount, 7);
    EXPECT_EQ(r.unreachable, (std::vector<int>{8}));
}

TEST(NetworkSimulatorScaleTest, ConnectivityOnLongChainDoesNotOverflowStack) {
    const int routers = 300000;  // deep enough to crash a recursive DFS
    std::stringstream in;
    in << "ROUTERS\n";
    for (int i = 0; i < routers; ++i) in << i << " R\n";
    in << "LINKS\n";
    for (int i = 0; i + 1 < routers; ++i) in << i << " " << i + 1 << " 1\n";

    NetworkSimulator sim;
    sim.loadFromStream(in);
    ConnectivityReport r = sim.checkConnectivity();
    EXPECT_TRUE(r.isConnected());
    EXPECT_EQ(r.reachableCount, routers);
}

TEST_F(NetworkSimulatorTest, MinimumSpanningTreeOfSampleNetwork) {
    MstResult mst = sim.findMinimumCostNetwork();
    ASSERT_TRUE(mst.connected);
    EXPECT_EQ(mst.edges.size(), 7u);
    EXPECT_EQ(mst.totalCost, 18);
}

TEST_F(NetworkSimulatorTest, MinimumSpanningTreeFailsWhenDisconnected) {
    isolateHotel();
    MstResult mst = sim.findMinimumCostNetwork();
    EXPECT_FALSE(mst.connected);
    EXPECT_EQ(mst.edges.size(), 6u);
}

// ── File I/O ─────────────────────────────────────────────────────────────────

TEST_F(NetworkSimulatorTest, LoadsSampleTopology) {
    EXPECT_EQ(sim.getNetwork().getRouterCount(), 8);
    EXPECT_EQ(sim.getNetwork().getLinkCount(), 13);
}

TEST(NetworkSimulatorFileTest, LoadsShippedSampleFile) {
    NetworkSimulator sim;
    LoadReport r = sim.loadFromFile(SAMPLE_NETWORK_PATH);
    EXPECT_EQ(r.status, Status::Ok);
    EXPECT_TRUE(r.issues.empty());
    EXPECT_EQ(sim.getNetwork().getRouterCount(), 8);
}

TEST(NetworkSimulatorFileTest, MissingFileReportsError) {
    NetworkSimulator sim;
    EXPECT_EQ(sim.loadFromFile("does/not/exist.txt").status, Status::FileOpenFailed);
}

TEST_F(NetworkSimulatorTest, SaveAndReloadRoundTrip) {
    sim.addRouter(9, "New York");
    sim.addLink(8, 9, 7);

    std::stringstream buffer;
    sim.saveToStream(buffer);

    NetworkSimulator reloaded;
    LoadReport r = reloaded.loadFromStream(buffer);
    EXPECT_TRUE(r.issues.empty());
    EXPECT_EQ(reloaded.getNetwork().getRouterCount(), 9);
    EXPECT_EQ(reloaded.getNetwork().getLinkCount(), 14);
    EXPECT_EQ(reloaded.getNetwork().getRouter(9).getName(), "New York");
}

TEST(NetworkSimulatorFileTest, ReportsMalformedAndInvalidLines) {
    std::istringstream in(
        "ROUTERS\n"
        "1 Alpha\n"
        "oops\n"
        "1 Duplicate\n"
        "2 Bravo\n"
        "LINKS\n"
        "1 2\n"
        "1 1 5\n"
        "1 2 3\n");

    NetworkSimulator sim;
    LoadReport r = sim.loadFromStream(in);

    ASSERT_EQ(r.issues.size(), 4u);
    EXPECT_EQ(r.issues[0].status, Status::MalformedLine);
    EXPECT_FALSE(r.issues[0].isLinkLine);
    EXPECT_EQ(r.issues[1].status, Status::DuplicateRouter);
    EXPECT_EQ(r.issues[2].status, Status::MalformedLine);
    EXPECT_TRUE(r.issues[2].isLinkLine);
    EXPECT_EQ(r.issues[3].status, Status::SelfLoop);
    EXPECT_EQ(sim.getNetwork().getLinkCount(), 1);
}
