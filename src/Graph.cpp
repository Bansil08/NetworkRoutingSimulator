#include "Graph.h"
#include <algorithm>

Status Graph::addRouter(int id, const std::string& name) {
    if (name.find_first_not_of(" \t") == std::string::npos) return Status::EmptyRouterName;
    if (routers.count(id)) return Status::DuplicateRouter;

    routers.emplace(id, Router(id, name));
    adjacencyList[id];
    return Status::Ok;
}

Status Graph::addLink(int src, int dst, int weight) {
    if (!hasRouter(src))    return Status::SourceNotFound;
    if (!hasRouter(dst))    return Status::DestinationNotFound;
    if (src == dst)         return Status::SelfLoop;
    if (weight < 0)         return Status::NegativeWeight;
    if (hasLink(src, dst))  return Status::DuplicateLink;

    adjacencyList[src].emplace_back(dst, weight);
    adjacencyList[dst].emplace_back(src, weight);
    ++linkCount;
    return Status::Ok;
}

Status Graph::removeLink(int src, int dst) {
    if (!hasLink(src, dst)) return Status::LinkNotFound;

    auto removeEdge = [](std::vector<std::pair<int,int>>& list, int target) {
        list.erase(
            std::remove_if(list.begin(), list.end(),
                [target](const std::pair<int,int>& p) { return p.first == target; }),
            list.end());
    };

    removeEdge(adjacencyList[src], dst);
    removeEdge(adjacencyList[dst], src);
    --linkCount;
    return Status::Ok;
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

std::vector<int> Graph::getSortedRouterIds() const {
    std::vector<int> ids;
    ids.reserve(routers.size());
    for (const auto& [id, _] : routers) ids.push_back(id);
    std::sort(ids.begin(), ids.end());
    return ids;
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

void Graph::clear() {
    routers.clear();
    adjacencyList.clear();
    linkCount = 0;
}
