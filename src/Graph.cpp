#include "Graph.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

bool Graph::addRouter(int id, const std::string& name) {
    if (routers.count(id)) {
        std::cout << "  [Error] Router with ID " << id << " already exists.\n";
        return false;
    }
    routers.emplace(id, Router(id, name));
    adjacencyList[id];
    return true;
}

bool Graph::addLink(int src, int dst, int weight) {
    if (!hasRouter(src)) {
        std::cout << "  [Error] Source router " << src << " not found.\n";
        return false;
    }
    if (!hasRouter(dst)) {
        std::cout << "  [Error] Destination router " << dst << " not found.\n";
        return false;
    }
    if (src == dst) {
        std::cout << "  [Error] Self-loops are not allowed (router " << src << ").\n";
        return false;
    }
    if (weight < 0) {
        std::cout << "  [Error] Negative link weights are not supported.\n";
        return false;
    }
    if (hasLink(src, dst)) {
        std::cout << "  [Error] Link between routers " << src << " and " << dst << " already exists.\n";
        return false;
    }

    adjacencyList[src].emplace_back(dst, weight);
    adjacencyList[dst].emplace_back(src, weight);
    ++linkCount;
    return true;
}

bool Graph::removeLink(int src, int dst) {
    if (!hasLink(src, dst)) {
        std::cout << "  [Error] No link between routers " << src << " and " << dst << ".\n";
        return false;
    }
    auto removeEdge = [](std::vector<std::pair<int,int>>& list, int target) {
        list.erase(
            std::remove_if(list.begin(), list.end(),
                [target](const std::pair<int,int>& p) { return p.first == target; }),
            list.end());
    };

    removeEdge(adjacencyList[src], dst);
    removeEdge(adjacencyList[dst], src);
    --linkCount;
    return true;
}

bool Graph::hasRouter(int id) const {
    return routers.count(id) > 0;
}

bool Graph::hasLink(int src, int dst) const {
    if (!adjacencyList.count(src)) return false;
    const auto& neighbours = adjacencyList.at(src);
    for (const auto& [neighbour, weight] : neighbours) {
        if (neighbour == dst) return true;
    }
    return false;
}

const Router& Graph::getRouter(int id) const {
    return routers.at(id);
}

const std::unordered_map<int, Router>& Graph::getRouters() const {
    return routers;
}

const std::vector<std::pair<int,int>>& Graph::getNeighbours(int routerId) const {
    static const std::vector<std::pair<int,int>> empty;
    auto it = adjacencyList.find(routerId);
    if (it == adjacencyList.end()) return empty;
    return it->second;
}

std::vector<Link> Graph::getAllEdges() const {
    std::vector<Link> edges;
    for (const auto& [routerId, neighbours] : adjacencyList) {
        for (const auto& [neighbour, weight] : neighbours) {
            if (routerId < neighbour) {
                edges.emplace_back(routerId, neighbour, weight);
            }
        }
    }
    return edges;
}

int Graph::getRouterCount() const { return static_cast<int>(routers.size()); }
int Graph::getLinkCount()  const  { return linkCount; }

void Graph::displayTopology() const {
    if (routers.empty()) {
        std::cout << "  [Info] Network is empty. Add routers and links first.\n";
        return;
    }

    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         NETWORK TOPOLOGY             ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";

    std::vector<int> ids;
    ids.reserve(routers.size());
    for (const auto& [id, _] : routers) ids.push_back(id);
    std::sort(ids.begin(), ids.end());

    for (int id : ids) {
        const Router& r = routers.at(id);
        std::cout << "  Router [" << std::setw(2) << id << "] " << r.getName() << "\n";

        const auto& neighbours = adjacencyList.at(id);
        if (neighbours.empty()) {
            std::cout << "      (no links)\n";
        } else {
            for (const auto& [neighbour, weight] : neighbours) {
                const std::string& neighbourName = routers.at(neighbour).getName();
                std::cout << "      ── [weight " << std::setw(3) << weight << "] ──> "
                          << "[" << neighbour << "] " << neighbourName << "\n";
            }
        }
        std::cout << "\n";
    }

    std::cout << "  Routers: " << routers.size()
              << "   Links: " << linkCount << "\n\n";
}

void Graph::clear() {
    routers.clear();
    adjacencyList.clear();
    linkCount = 0;
}
