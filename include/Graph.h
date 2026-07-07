#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include "Router.h"
#include "Link.h"

class Graph {
public:
    bool addRouter(int id, const std::string& name);
    bool addLink(int src, int dst, int weight);
    bool removeLink(int src, int dst);

    bool hasRouter(int id) const;
    bool hasLink(int src, int dst) const;
    const Router& getRouter(int id) const;
    const std::unordered_map<int, Router>& getRouters() const;
    const std::vector<std::pair<int,int>>& getNeighbours(int routerId) const;
    std::vector<Link> getAllEdges() const;

    int getRouterCount() const;
    int getLinkCount() const;

    void displayTopology() const;
    void clear();

private:
    std::unordered_map<int, Router> routers;
    std::unordered_map<int, std::vector<std::pair<int,int>>> adjacencyList;
    int linkCount = 0;
};
