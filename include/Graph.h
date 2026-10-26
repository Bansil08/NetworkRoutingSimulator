#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include "Router.h"
#include "Link.h"
#include "Status.h"

class Graph {
public:
    Status addRouter(int id, const std::string& name);
    Status addLink(int src, int dst, int weight);
    Status removeLink(int src, int dst);

    bool hasRouter(int id) const;
    bool hasLink(int src, int dst) const;
    const Router& getRouter(int id) const;
    const std::unordered_map<int, Router>& getRouters() const;
    std::vector<int> getSortedRouterIds() const;
    const std::vector<std::pair<int,int>>& getNeighbours(int routerId) const;
    std::vector<Link> getAllEdges() const;

    int getRouterCount() const;
    int getLinkCount() const;

    void clear();

private:
    std::unordered_map<int, Router> routers;
    std::unordered_map<int, std::vector<std::pair<int,int>>> adjacencyList;
    int linkCount = 0;
};
