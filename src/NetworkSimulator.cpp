#include "NetworkSimulator.h"
#include "DijkstraRouting.h"
#include "DisjointSet.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <vector>

NetworkSimulator::NetworkSimulator()
    : routingStrategy(std::make_unique<DijkstraRouting>()) {}

void NetworkSimulator::setRoutingStrategy(std::unique_ptr<RoutingStrategy> strategy) {
    routingStrategy = std::move(strategy);
}

std::string NetworkSimulator::getAlgorithmName() const {
    return routingStrategy->getAlgorithmName();
}

const Graph& NetworkSimulator::getNetwork() const {
    return network;
}

Status NetworkSimulator::addRouter(int id, const std::string& name) {
    return network.addRouter(id, name);
}

Status NetworkSimulator::addLink(int src, int dst, int weight) {
    return network.addLink(src, dst, weight);
}

Status NetworkSimulator::removeLink(int src, int dst) {
    Status status = network.removeLink(src, dst);
    if (status == Status::Ok && hasLastRoute && linkIsOnRoute(lastRoute, src, dst)) {
        hasLastRoute = false;
    }
    return status;
}

RouteQuery NetworkSimulator::findRoute(int source, int destination) const {
    RouteQuery query;

    if (network.getRouterCount() == 0)  { query.status = Status::EmptyNetwork;        return query; }
    if (!network.hasRouter(source))     { query.status = Status::SourceNotFound;      return query; }
    if (!network.hasRouter(destination)){ query.status = Status::DestinationNotFound; return query; }

    if (source == destination) {
        query.route.pathFound = true;
        query.route.path      = {source};
        query.route.totalCost = 0;
        return query;
    }

    query.route = routingStrategy->findRoute(network, source, destination);
    return query;
}

PacketReport NetworkSimulator::sendPacket(int source, int destination, int priority) {
    ++totalPacketsSent;

    PacketReport report;
    report.packetId = nextPacketId++;
    report.priority = priority;

    RouteQuery query = findRoute(source, destination);
    report.status = query.status;
    report.route  = query.route;

    if (!report.delivered()) {
        ++failedPackets;
        return report;
    }

    ++successfulPackets;
    lastRoute            = report.route;
    hasLastRoute         = true;
    lastRouteSource      = source;
    lastRouteDestination = destination;
    return report;
}

LinkFailureReport NetworkSimulator::simulateLinkFailure(int src, int dst) {
    LinkFailureReport report;

    if (!network.hasLink(src, dst)) {
        report.status = Status::LinkNotFound;
        return report;
    }

    report.affectedLastRoute = hasLastRoute && linkIsOnRoute(lastRoute, src, dst);
    if (report.affectedLastRoute) {
        report.previousRoute    = lastRoute;
        report.routeSource      = lastRouteSource;
        report.routeDestination = lastRouteDestination;
    }

    network.removeLink(src, dst);

    if (!report.affectedLastRoute) return report;

    report.newRoute = routingStrategy->findRoute(network, lastRouteSource, lastRouteDestination);

    if (!report.newRoute.pathFound) {
        hasLastRoute = false;
        ++failedPackets;
    } else {
        lastRoute = report.newRoute;
        ++reroutedPackets;
    }
    return report;
}

ConnectivityReport NetworkSimulator::checkConnectivity() const {
    ConnectivityReport report;
    report.routerCount = network.getRouterCount();
    if (report.routerCount == 0) return report;

    std::vector<int> ids = network.getSortedRouterIds();
    report.startRouter = ids.front();

    std::unordered_set<int> visited;
    dfsVisit(report.startRouter, visited);
    report.reachableCount = static_cast<int>(visited.size());

    for (int id : ids) {
        if (!visited.count(id)) report.unreachable.push_back(id);
    }
    return report;
}

void NetworkSimulator::dfsVisit(int routerId, std::unordered_set<int>& visited) const {
    visited.insert(routerId);
    for (const auto& [neighbour, weight] : network.getNeighbours(routerId)) {
        if (!visited.count(neighbour)) {
            dfsVisit(neighbour, visited);
        }
    }
}

MstResult NetworkSimulator::findMinimumCostNetwork() const {
    MstResult result;
    int routerCount = network.getRouterCount();
    if (routerCount <= 1) {
        result.connected = true;
        return result;
    }

    std::vector<Link> edges = network.getAllEdges();
    std::sort(edges.begin(), edges.end(), [](const Link& a, const Link& b) {
        return a.weight < b.weight;
    });

    DisjointSet dsu;
    dsu.initialise(network.getSortedRouterIds());

    for (const Link& edge : edges) {
        if (dsu.unite(edge.source, edge.destination)) {
            result.edges.push_back(edge);
            result.totalCost += edge.weight;
            if (static_cast<int>(result.edges.size()) == routerCount - 1) break;
        }
    }

    result.connected = static_cast<int>(result.edges.size()) == routerCount - 1;
    return result;
}

Statistics NetworkSimulator::getStatistics() const {
    Statistics stats;
    stats.routerCount       = network.getRouterCount();
    stats.linkCount         = network.getLinkCount();
    stats.algorithmName     = getAlgorithmName();
    stats.totalPacketsSent  = totalPacketsSent;
    stats.successfulPackets = successfulPackets;
    stats.failedPackets     = failedPackets;
    stats.reroutedPackets   = reroutedPackets;
    return stats;
}

LoadReport NetworkSimulator::loadFromStream(std::istream& in) {
    LoadReport report;

    network.clear();
    hasLastRoute = false;

    std::string line;
    std::string section;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        if (line == "ROUTERS") { section = "ROUTERS"; continue; }
        if (line == "LINKS")   { section = "LINKS";   continue; }

        std::istringstream iss(line);
        LoadIssue issue;
        issue.line = line;

        if (section == "ROUTERS") {
            int id; std::string name;
            if (iss >> id >> std::ws && std::getline(iss, name) && !name.empty()) {
                issue.first  = id;
                issue.status = network.addRouter(id, name);
            } else {
                issue.status = Status::MalformedLine;
            }
        } else if (section == "LINKS") {
            issue.isLinkLine = true;
            int src, dst, weight;
            if (iss >> src >> dst >> weight) {
                issue.first  = src;
                issue.second = dst;
                issue.status = network.addLink(src, dst, weight);
            } else {
                issue.status = Status::MalformedLine;
            }
        }

        if (issue.status != Status::Ok) report.issues.push_back(issue);
    }
    return report;
}

LoadReport NetworkSimulator::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LoadReport report;
        report.status = Status::FileOpenFailed;
        return report;
    }
    return loadFromStream(file);
}

void NetworkSimulator::saveToStream(std::ostream& out) const {
    out << "ROUTERS\n";
    for (int id : network.getSortedRouterIds()) {
        out << id << " " << network.getRouter(id).getName() << "\n";
    }

    out << "\nLINKS\n";
    for (const Link& e : network.getAllEdges()) {
        out << e.source << " " << e.destination << " " << e.weight << "\n";
    }
}

Status NetworkSimulator::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return Status::FileOpenFailed;
    saveToStream(file);
    return Status::Ok;
}

bool NetworkSimulator::linkIsOnRoute(const RouteResult& route, int src, int dst) {
    const auto& path = route.path;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        int u = path[i], v = path[i + 1];
        if ((u == src && v == dst) || (u == dst && v == src)) return true;
    }
    return false;
}
