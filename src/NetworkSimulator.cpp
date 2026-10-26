#include "NetworkSimulator.h"
#include "DijkstraRouting.h"
#include "BFSRouting.h"
#include "DisjointSet.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <limits>

NetworkSimulator::NetworkSimulator() {
    routingStrategy      = std::make_unique<DijkstraRouting>();
    currentAlgorithmName = routingStrategy->getAlgorithmName();
}

void NetworkSimulator::setRoutingStrategy(std::unique_ptr<RoutingStrategy> strategy) {
    routingStrategy      = std::move(strategy);
    currentAlgorithmName = routingStrategy->getAlgorithmName();
    std::cout << "  [OK] Routing algorithm changed to: " << currentAlgorithmName << "\n";
}

const std::string& NetworkSimulator::getCurrentAlgorithmName() const {
    return currentAlgorithmName;
}

void NetworkSimulator::addRouter(int id, const std::string& name) {
    if (name.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "  [Error] Router name cannot be empty.\n";
        return;
    }
    if (network.addRouter(id, name)) {
        std::cout << "  [OK] Router [" << id << "] '" << name << "' added.\n";
    }
}

void NetworkSimulator::removeRouter(int id) {
    if (!network.hasRouter(id)) {
        std::cout << "  [Error] Router " << id << " not found.\n";
        return;
    }
    std::cout << "  [Info] Router removal is a future improvement. "
              << "Use 'Remove Link' to isolate a router instead.\n";
}

void NetworkSimulator::addLink(int src, int dst, int weight) {
    if (network.addLink(src, dst, weight)) {
        std::cout << "  [OK] Link added: Router " << src << " <──["
                  << weight << "]──> Router " << dst << "\n";
    }
}

void NetworkSimulator::removeLink(int src, int dst) {
    if (network.removeLink(src, dst)) {
        std::cout << "  [OK] Link removed: Router " << src << " <──> Router " << dst << "\n";
        if (hasLastRoute && linkIsOnRoute(lastRoute, src, dst)) {
            hasLastRoute = false;
        }
    }
}

RouteResult NetworkSimulator::findAndDisplayRoute(int source, int destination) const {
    RouteResult result;

    if (network.getRouterCount() == 0) {
        std::cout << "  [Error] Network is empty.\n";
        return result;
    }
    if (!network.hasRouter(source)) {
        std::cout << "  [Error] Source router " << source << " not found.\n";
        return result;
    }
    if (!network.hasRouter(destination)) {
        std::cout << "  [Error] Destination router " << destination << " not found.\n";
        return result;
    }
    if (source == destination) {
        std::cout << "  [Info] Source and destination are the same router.\n";
        result.pathFound = true;
        result.path      = {source};
        result.totalCost = 0;
        return result;
    }

    result = routingStrategy->findRoute(network, source, destination);

    std::cout << "\n  Algorithm : " << routingStrategy->getAlgorithmName() << "\n";
    std::cout << "  From      : [" << source      << "] "
              << network.getRouter(source).getName()      << "\n";
    std::cout << "  To        : [" << destination << "] "
              << network.getRouter(destination).getName() << "\n\n";

    if (!result.pathFound) {
        std::cout << "  ✗  No route found between routers "
                  << source << " and " << destination << ".\n\n";
    } else {
        std::cout << "  Route     : ";
        printPath(result.path);
        std::cout << "  Total Cost: " << result.totalCost
                  << (routingStrategy->getAlgorithmName().find("BFS") != std::string::npos
                      ? "  (actual link-weight sum along BFS hop-path)" : "") << "\n\n";
    }

    return result;
}

void NetworkSimulator::sendPacket(int packetId, int source, int destination, int priority) {
    ++totalPacketsSent;

    std::cout << "\n  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │         PACKET TRANSMISSION              │\n";
    std::cout << "  └─────────────────────────────────────────┘\n";
    std::cout << "  Packet ID : #" << packetId << "\n";
    std::cout << "  Priority  : " << priority  << "\n";

    RouteResult result = findAndDisplayRoute(source, destination);

    if (!result.pathFound) {
        ++failedPackets;
        std::cout << "  ✗  Packet #" << packetId << " could not be delivered.\n\n";
        return;
    }

    ++successfulPackets;

    std::cout << "  Traversal : ";
    for (size_t i = 0; i < result.path.size(); ++i) {
        if (i > 0) std::cout << " → ";
        std::cout << network.getRouter(result.path[i]).getName();
    }
    std::cout << "\n  ✓  Packet #" << packetId << " delivered successfully.\n\n";

    lastRoute            = result;
    hasLastRoute         = true;
    lastRouteSource      = source;
    lastRouteDestination = destination;
}

void NetworkSimulator::simulateLinkFailure(int src, int dst) {
    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║       LINK FAILURE SIMULATION        ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";

    if (!network.hasLink(src, dst)) {
        std::cout << "  [Error] No link between routers " << src << " and " << dst << ".\n\n";
        return;
    }

    std::cout << "  ⚡ Failed Link: Router " << src << " <──> Router " << dst << "\n\n";

    bool affectsLastRoute = hasLastRoute && linkIsOnRoute(lastRoute, src, dst);

    if (affectsLastRoute) {
        std::cout << "  Previous Route: ";
        printPath(lastRoute.path);
        std::cout << "  Previous Cost : " << lastRoute.totalCost << "\n\n";
    }

    network.removeLink(src, dst);
    std::cout << "  Link removed from network.\n\n";

    if (!affectsLastRoute) {
        std::cout << "  [Info] The failed link was NOT on the last known route.\n"
                  << "         No rerouting is required.\n\n";
        return;
    }

    std::cout << "  Recomputing route...\n\n";
    RouteResult newRoute = routingStrategy->findRoute(network, lastRouteSource, lastRouteDestination);

    if (!newRoute.pathFound) {
        std::cout << "  ✗  DESTINATION UNREACHABLE AFTER LINK FAILURE\n";
        std::cout << "     No alternative path from Router " << lastRouteSource
                  << " to Router " << lastRouteDestination << ".\n\n";
        hasLastRoute = false;
        ++failedPackets;
    } else {
        std::cout << "  ✓  New Route Found!\n";
        std::cout << "     New Route : ";
        printPath(newRoute.path);
        std::cout << "     New Cost  : " << newRoute.totalCost << "\n\n";

        lastRoute = newRoute;
        ++reroutedPackets;
    }
}

void NetworkSimulator::checkConnectivity() const {
    std::cout << "\n  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │         CONNECTIVITY CHECK               │\n";
    std::cout << "  └─────────────────────────────────────────┘\n\n";

    if (network.getRouterCount() == 0) {
        std::cout << "  [Info] Network is empty.\n\n";
        return;
    }

    const auto& allRouters = network.getRouters();
    int startId = allRouters.begin()->first;

    std::unordered_set<int> visited;
    dfsVisit(startId, visited);

    if (static_cast<int>(visited.size()) == network.getRouterCount()) {
        std::cout << "  ✓  Network is FULLY CONNECTED.\n";
        std::cout << "     All " << network.getRouterCount() << " routers can communicate.\n\n";
    } else {
        std::cout << "  ✗  Network is DISCONNECTED.\n";
        std::cout << "     Reachable from Router " << startId << ": "
                  << visited.size() << " / " << network.getRouterCount() << " routers.\n\n";

        std::cout << "  Unreachable Routers:\n";
        for (const auto& [id, router] : allRouters) {
            if (!visited.count(id)) {
                std::cout << "     ✗  Router [" << id << "] " << router.getName() << "\n";
            }
        }
        std::cout << "\n";
    }
}

void NetworkSimulator::dfsVisit(int routerId, std::unordered_set<int>& visited) const {
    visited.insert(routerId);
    for (const auto& [neighbour, weight] : network.getNeighbours(routerId)) {
        if (!visited.count(neighbour)) {
            dfsVisit(neighbour, visited);
        }
    }
}

void NetworkSimulator::findMinimumCostNetwork() const {
    std::cout << "\n  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │     MINIMUM COST NETWORK (Kruskal MST)  │\n";
    std::cout << "  └─────────────────────────────────────────┘\n\n";

    int routerCount = network.getRouterCount();
    if (routerCount == 0) {
        std::cout << "  [Info] Network is empty.\n\n";
        return;
    }
    if (routerCount == 1) {
        std::cout << "  [Info] Only one router. No links needed.\n\n";
        return;
    }

    std::vector<Link> edges = network.getAllEdges();
    if (edges.empty()) {
        std::cout << "  [Info] No links in the network. Cannot build MST.\n\n";
        return;
    }

    std::sort(edges.begin(), edges.end(), [](const Link& a, const Link& b) {
        return a.weight < b.weight;
    });

    DisjointSet dsu;
    std::vector<int> routerIds;
    for (const auto& [id, _] : network.getRouters()) {
        routerIds.push_back(id);
    }
    dsu.initialise(routerIds);

    std::vector<Link> mstEdges;
    long long totalCost = 0;

    for (const Link& edge : edges) {
        if (dsu.unite(edge.source, edge.destination)) {
            mstEdges.push_back(edge);
            totalCost += edge.weight;
            if (static_cast<int>(mstEdges.size()) == routerCount - 1) break;
        }
    }

    if (static_cast<int>(mstEdges.size()) < routerCount - 1) {
        std::cout << "  ✗  Cannot build MST — Network is DISCONNECTED.\n";
        std::cout << "     Found only " << mstEdges.size() << " MST edge(s), need "
                  << routerCount - 1 << ".\n\n";
        return;
    }

    std::cout << "  ✓  Minimum Cost Network found!\n\n";
    std::cout << "  MST Edges:\n";
    for (const Link& e : mstEdges) {
        const std::string& sName = network.getRouter(e.source).getName();
        const std::string& dName = network.getRouter(e.destination).getName();
        std::cout << "     " << sName << " [" << e.source << "] <──["
                  << e.weight << "]──> [" << e.destination << "] " << dName << "\n";
    }
    std::cout << "\n  Total MST Cost: " << totalCost << "\n\n";
}

void NetworkSimulator::displayStatistics() const {
    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║          NETWORK STATISTICS          ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";
    std::cout << "  Routers              : " << network.getRouterCount()  << "\n";
    std::cout << "  Links                : " << network.getLinkCount()    << "\n";
    std::cout << "  Active Algorithm     : " << currentAlgorithmName      << "\n\n";
    std::cout << "  Total Packets Sent   : " << totalPacketsSent          << "\n";
    std::cout << "  Successful Deliveries: " << successfulPackets         << "\n";
    std::cout << "  Failed Deliveries    : " << failedPackets             << "\n";
    std::cout << "  Rerouted Packets     : " << reroutedPackets           << "\n\n";
}

void NetworkSimulator::displayTopology() const {
    network.displayTopology();
}

bool NetworkSimulator::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "  [Error] Cannot open file: " << filename << "\n";
        return false;
    }

    network.clear();
    hasLastRoute = false;

    std::string line;
    std::string section;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        if (line == "ROUTERS") { section = "ROUTERS"; continue; }
        if (line == "LINKS")   { section = "LINKS";   continue; }

        std::istringstream iss(line);
        if (section == "ROUTERS") {
            int id; std::string name;
            if (iss >> id >> std::ws && std::getline(iss, name) && !name.empty()) {
                network.addRouter(id, name);
            } else {
                std::cout << "  [Warning] Malformed router line: " << line << "\n";
            }
        } else if (section == "LINKS") {
            int src, dst, weight;
            if (iss >> src >> dst >> weight) {
                network.addLink(src, dst, weight);
            } else {
                std::cout << "  [Warning] Malformed link line: " << line << "\n";
            }
        }
    }

    std::cout << "  [OK] Network loaded from '" << filename << "'\n";
    std::cout << "       Routers: " << network.getRouterCount()
              << "  Links: " << network.getLinkCount() << "\n";
    return true;
}

bool NetworkSimulator::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "  [Error] Cannot write to file: " << filename << "\n";
        return false;
    }

    file << "ROUTERS\n";

    std::vector<int> ids;
    for (const auto& [id, _] : network.getRouters()) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    for (int id : ids) {
        file << id << " " << network.getRouter(id).getName() << "\n";
    }

    file << "\nLINKS\n";
    for (const Link& e : network.getAllEdges()) {
        file << e.source << " " << e.destination << " " << e.weight << "\n";
    }

    std::cout << "  [OK] Network saved to '" << filename << "'\n";
    return true;
}

void NetworkSimulator::printPath(const std::vector<int>& path) const {
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) std::cout << " → ";
        int id = path[i];
        if (network.hasRouter(id)) {
            std::cout << network.getRouter(id).getName() << "[" << id << "]";
        } else {
            std::cout << "[" << id << "]";
        }
    }
    std::cout << "\n";
}

bool NetworkSimulator::linkIsOnRoute(const RouteResult& route, int src, int dst) const {
    const auto& path = route.path;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        int u = path[i], v = path[i + 1];
        if ((u == src && v == dst) || (u == dst && v == src)) return true;
    }
    return false;
}

std::string NetworkSimulator::promptLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int NetworkSimulator::promptInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) return 0;  // EOF: let run() exit
        if (line.empty()) continue;
        try {
            return std::stoi(line);
        } catch (...) {
            std::cout << "  [Error] Please enter a valid integer.\n";
        }
    }
}

void NetworkSimulator::displayMenu() const {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║    NETWORK ROUTING & FAILURE RECOVERY        ║\n";
    std::cout << "  ║              SIMULATOR                       ║\n";
    std::cout << "  ╠══════════════════════════════════════════════╣\n";
    std::cout << "  ║  Active Algorithm: " << std::left << std::setw(26)
              << currentAlgorithmName << std::right << "║\n";
    std::cout << "  ╠══════════════════════════════════════════════╣\n";
    std::cout << "  ║  1.  Add Router                              ║\n";
    std::cout << "  ║  2.  Add Network Link                        ║\n";
    std::cout << "  ║  3.  Remove Network Link                     ║\n";
    std::cout << "  ║  4.  Display Network Topology                ║\n";
    std::cout << "  ║  5.  Find Shortest Route                     ║\n";
    std::cout << "  ║  6.  Change Routing Algorithm                ║\n";
    std::cout << "  ║  7.  Send Packet                             ║\n";
    std::cout << "  ║  8.  Simulate Link Failure                   ║\n";
    std::cout << "  ║  9.  Check Network Connectivity              ║\n";
    std::cout << "  ║  10. Find Minimum Cost Network (MST)         ║\n";
    std::cout << "  ║  11. Display Network Statistics              ║\n";
    std::cout << "  ║  12. Load Network From File                  ║\n";
    std::cout << "  ║  13. Save Network To File                    ║\n";
    std::cout << "  ║  0.  Exit                                    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n";
    std::cout << "  Choice: ";
}

void NetworkSimulator::run() {
    std::string choiceStr;
    while (true) {
        displayMenu();
        if (!std::getline(std::cin, choiceStr)) {
            std::cout << "\n  Input closed. Goodbye!\n\n";
            return;
        }

        int choice = -1;
        try { choice = std::stoi(choiceStr); } catch (...) {}

        std::cout << "\n";

        switch (choice) {
            case 1: {
                int id           = promptInt("  Router ID   : ");
                std::string name = promptLine("  Router Name : ");
                addRouter(id, name);
                break;
            }
            case 2: {
                int src    = promptInt("  Source Router ID      : ");
                int dst    = promptInt("  Destination Router ID : ");
                int weight = promptInt("  Link Weight           : ");
                addLink(src, dst, weight);
                break;
            }
            case 3: {
                int src = promptInt("  Source Router ID      : ");
                int dst = promptInt("  Destination Router ID : ");
                removeLink(src, dst);
                break;
            }
            case 4:
                displayTopology();
                break;
            case 5: {
                int src = promptInt("  Source Router ID      : ");
                int dst = promptInt("  Destination Router ID : ");
                findAndDisplayRoute(src, dst);
                break;
            }
            case 6: {
                std::cout << "  Select Algorithm:\n";
                std::cout << "    1. Dijkstra (Minimum Cost)\n";
                std::cout << "    2. BFS (Minimum Hops)\n";
                int alg = promptInt("  Choice: ");
                if (alg == 1) {
                    setRoutingStrategy(std::make_unique<DijkstraRouting>());
                } else if (alg == 2) {
                    setRoutingStrategy(std::make_unique<BFSRouting>());
                } else {
                    std::cout << "  [Error] Invalid choice.\n";
                }
                break;
            }
            case 7: {
                int src      = promptInt("  Source Router ID      : ");
                int dst      = promptInt("  Destination Router ID : ");
                int priority = promptInt("  Packet Priority (1-5) : ");
                sendPacket(nextPacketId++, src, dst, priority);
                break;
            }
            case 8: {
                int src = promptInt("  Failed Link — Source Router ID      : ");
                int dst = promptInt("  Failed Link — Destination Router ID : ");
                simulateLinkFailure(src, dst);
                break;
            }
            case 9:
                checkConnectivity();
                break;
            case 10:
                findMinimumCostNetwork();
                break;
            case 11:
                displayStatistics();
                break;
            case 12: {
                std::string filename = promptLine("  Filename: ");
                loadFromFile(filename);
                break;
            }
            case 13: {
                std::string filename = promptLine("  Filename: ");
                saveToFile(filename);
                break;
            }
            case 0:
                std::cout << "  Goodbye!\n\n";
                return;
            default:
                std::cout << "  [Error] Invalid menu choice. Enter 0–13.\n";
                break;
        }
    }
}
