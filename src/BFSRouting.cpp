#include "BFSRouting.h"
#include "Graph.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

std::string BFSRouting::getAlgorithmName() const {
    return "BFS (Minimum Hops)";
}

RouteResult BFSRouting::findRoute(
    const Graph& graph,
    int source,
    int destination
) const {
    RouteResult result;

    std::queue<int>             bfsQueue;
    std::unordered_set<int>     visited;
    std::unordered_map<int,int> parent;

    bfsQueue.push(source);
    visited.insert(source);

    bool found = false;

    while (!bfsQueue.empty() && !found) {
        int u = bfsQueue.front();
        bfsQueue.pop();

        for (const auto& [v, weight] : graph.getNeighbours(u)) {
            if (visited.count(v)) continue;

            visited.insert(v);
            parent[v] = u;

            if (v == destination) {
                found = true;
                break;
            }
            bfsQueue.push(v);
        }
    }

    if (!found) {
        result.pathFound = false;
        return result;
    }

    result.pathFound = true;

    int current = destination;
    while (current != source) {
        result.path.push_back(current);
        current = parent[current];
    }
    result.path.push_back(source);
    std::reverse(result.path.begin(), result.path.end());

    result.totalCost = 0;
    for (size_t i = 0; i + 1 < result.path.size(); ++i) {
        int from = result.path[i];
        int to   = result.path[i + 1];
        for (const auto& [neighbour, w] : graph.getNeighbours(from)) {
            if (neighbour == to) { result.totalCost += w; break; }
        }
    }

    return result;
}
