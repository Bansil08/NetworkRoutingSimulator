#pragma once

#include <sstream>
#include "NetworkSimulator.h"

// Same topology as data/sample_network.txt, kept inline so unit tests
// don't depend on the working directory.
inline const char* kSampleNetwork = R"(
ROUTERS
1 Alpha
2 Bravo
3 Charlie
4 Delta
5 Echo
6 Foxtrot
7 Golf
8 Hotel

LINKS
1 2 4
1 3 2
2 3 1
2 4 5
3 5 8
4 5 2
4 6 6
5 7 3
5 8 10
6 7 1
6 8 9
7 8 4
1 4 15
)";

inline void loadSample(NetworkSimulator& sim) {
    std::istringstream in(kSampleNetwork);
    sim.loadFromStream(in);
}

inline Graph sampleGraph() {
    NetworkSimulator sim;
    loadSample(sim);
    return sim.getNetwork();
}
