#include <iostream>
#include <string>
#include "NetworkSimulator.h"
#include "ConsoleUI.h"

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║   NETWORK ROUTING & FAILURE RECOVERY         ║\n";
    std::cout << "  ║              SIMULATOR  v1.0                 ║\n";
    std::cout << "  ║   C++17 · OOP · Graph Algorithms · STL       ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n\n";

    NetworkSimulator simulator;
    ConsoleUI ui(simulator);

    if (argc == 2) {
        std::string filename(argv[1]);
        std::cout << "  Loading network from: " << filename << "\n";
        if (!ui.loadFromFile(filename)) {
            std::cout << "  [Warning] Could not load file. Starting with an empty network.\n";
        }
    } else {
        std::cout << "  TIP: Run as  ./NetworkRoutingSimulator data/sample_network.txt\n";
        std::cout << "       to auto-load the sample topology.\n";
    }

    ui.run();
    return 0;
}
