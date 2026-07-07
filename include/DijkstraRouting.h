#pragma once

#include "RoutingStrategy.h"

class DijkstraRouting : public RoutingStrategy {
public:
    RouteResult findRoute(
        const Graph& graph,
        int source,
        int destination
    ) const override;

    std::string getAlgorithmName() const override;
};
