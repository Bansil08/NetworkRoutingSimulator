#include "DijkstraRouting.h"
#include "Graph.h"
#include <queue>
#include <unordered_map>
#include <vector>
#include <limits>
#include <algorithm>

std::string DijkstraRouting::getAlgorithmName() const {
    return "Dijkstra (Minimum Cost)";
}

RouteResult DijkstraRouting::findRoute(
    const Graph& graph,
    int source,
    int destination
) const {
    RouteResult result;

    if (!graph.hasRouter(source) || !graph.hasRouter(destination)) {
        return result;
    }

    using Entry = std::pair<long long, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> minHeap;

    const long long INF = std::numeric_limits<long long>::max();

    std::unordered_map<int, long long> dist;
    std::unordered_map<int, int>       parent;

    for (const auto& [id, _] : graph.getRouters()) {
        dist[id] = INF;
    }

    dist[source] = 0;
    minHeap.push({0, source});

    while (!minHeap.empty()) {
        auto [currentCost, u] = minHeap.top();
        minHeap.pop();

        if (currentCost > dist[u]) continue;

        if (u == destination) break;

        for (const auto& [v, weight] : graph.getNeighbours(u)) {
            long long newCost = dist[u] + weight;
            if (newCost < dist[v]) {
                dist[v] = newCost;
                parent[v] = u;
                minHeap.push({newCost, v});
            }
        }
    }

    if (dist[destination] == INF) {
        result.pathFound = false;
        return result;
    }

    result.pathFound  = true;
    result.totalCost  = dist[destination];

    int current = destination;
    while (current != source) {
        result.path.push_back(current);
        current = parent[current];
    }
    result.path.push_back(source);
    std::reverse(result.path.begin(), result.path.end());

    return result;
}
