#pragma once

#include <string>
#include "RouteResult.h"

class Graph;

class RoutingStrategy {
public:
    virtual RouteResult findRoute(
        const Graph& graph,
        int source,
        int destination
    ) const = 0;

    virtual std::string getAlgorithmName() const = 0;

    virtual ~RoutingStrategy() = default;
};
