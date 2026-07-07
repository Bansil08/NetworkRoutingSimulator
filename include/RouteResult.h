#pragma once

#include <vector>
#include <string>

struct RouteResult {
    bool pathFound;
    std::vector<int> path;
    long long totalCost;

    RouteResult() : pathFound(false), totalCost(0) {}
};
