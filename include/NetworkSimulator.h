#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "Graph.h"
#include "RoutingStrategy.h"
#include "RouteResult.h"
#include "Status.h"

// ── Result types returned by the simulator (no I/O happens in the core) ──────

struct RouteQuery {
    Status      status = Status::Ok;
    RouteResult route;
};

struct PacketReport {
    int         packetId = 0;
    int         priority = 0;
    Status      status   = Status::Ok;
    RouteResult route;

    bool delivered() const { return status == Status::Ok && route.pathFound; }
};

struct LinkFailureReport {
    Status      status            = Status::Ok;
    bool        affectedLastRoute = false;
    RouteResult previousRoute;
    RouteResult newRoute;
    int         routeSource       = -1;
    int         routeDestination  = -1;
};

struct ConnectivityReport {
    int              routerCount    = 0;
    int              startRouter    = -1;
    int              reachableCount = 0;
    std::vector<int> unreachable;      // sorted router IDs

    bool isConnected() const { return reachableCount == routerCount; }
};

struct MstResult {
    bool              connected = false;
    std::vector<Link> edges;
    long long         totalCost = 0;
};

struct Statistics {
    int         routerCount       = 0;
    int         linkCount         = 0;
    std::string algorithmName;
    int         totalPacketsSent  = 0;
    int         successfulPackets = 0;
    int         failedPackets     = 0;
    int         reroutedPackets   = 0;
};

struct LoadIssue {
    std::string line;
    Status      status = Status::Ok;
    bool        isLinkLine = false;
    int         first  = 0;   // router ID, or link source
    int         second = 0;   // link destination
};

struct LoadReport {
    Status                 status = Status::Ok;
    std::vector<LoadIssue> issues;
};

// ── Core simulator: owns the network and routing strategy ────────────────────

class NetworkSimulator {
public:
    NetworkSimulator();

    void setRoutingStrategy(std::unique_ptr<RoutingStrategy> strategy);
    std::string getAlgorithmName() const;

    const Graph& getNetwork() const;

    Status addRouter(int id, const std::string& name);
    Status addLink(int src, int dst, int weight);
    Status removeLink(int src, int dst);

    RouteQuery        findRoute(int source, int destination) const;
    PacketReport      sendPacket(int source, int destination, int priority = 1);
    LinkFailureReport simulateLinkFailure(int src, int dst);

    ConnectivityReport checkConnectivity() const;
    MstResult          findMinimumCostNetwork() const;
    Statistics         getStatistics() const;

    LoadReport loadFromStream(std::istream& in);
    LoadReport loadFromFile(const std::string& filename);
    void       saveToStream(std::ostream& out) const;
    Status     saveToFile(const std::string& filename) const;

private:
    Graph                            network;
    std::unique_ptr<RoutingStrategy> routingStrategy;

    int nextPacketId = 1;

    RouteResult lastRoute;
    bool        hasLastRoute         = false;
    int         lastRouteSource      = -1;
    int         lastRouteDestination = -1;

    int totalPacketsSent  = 0;
    int successfulPackets = 0;
    int failedPackets     = 0;
    int reroutedPackets   = 0;

    static bool linkIsOnRoute(const RouteResult& route, int src, int dst);
    void dfsVisit(int routerId, std::unordered_set<int>& visited) const;
};
