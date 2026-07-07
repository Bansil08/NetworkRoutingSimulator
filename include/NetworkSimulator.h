#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include "Graph.h"
#include "RoutingStrategy.h"
#include "RouteResult.h"
#include "Packet.h"

class NetworkSimulator {
public:
    NetworkSimulator();

    void setRoutingStrategy(std::unique_ptr<RoutingStrategy> strategy);
    const std::string& getCurrentAlgorithmName() const;

    void addRouter(int id, const std::string& name);
    void removeRouter(int id);

    void addLink(int src, int dst, int weight);
    void removeLink(int src, int dst);

    RouteResult findAndDisplayRoute(int source, int destination) const;
    void sendPacket(int packetId, int source, int destination, int priority = 1);

    void simulateLinkFailure(int src, int dst);

    void checkConnectivity() const;
    void findMinimumCostNetwork() const;

    void displayTopology() const;
    void displayStatistics() const;
    void displayMenu() const;

    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;

    void run();

private:
    Graph                            network;
    std::unique_ptr<RoutingStrategy> routingStrategy;
    std::string                      currentAlgorithmName;

    int nextPacketId = 1;

    RouteResult lastRoute;
    bool        hasLastRoute         = false;
    int         lastRouteSource      = -1;
    int         lastRouteDestination = -1;

    int totalPacketsSent  = 0;
    int successfulPackets = 0;
    int failedPackets     = 0;
    int reroutedPackets   = 0;

    void printPath(const std::vector<int>& path) const;
    bool linkIsOnRoute(const RouteResult& route, int src, int dst) const;
    void dfsVisit(int routerId, std::unordered_set<int>& visited) const;

    static std::string promptLine(const std::string& prompt);
    static int         promptInt(const std::string& prompt);
};
