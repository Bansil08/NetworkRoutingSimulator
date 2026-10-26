#pragma once

#include <string>
#include <vector>
#include "NetworkSimulator.h"

// Interactive text front-end. All std::cin / std::cout lives here; the
// NetworkSimulator it drives is pure logic and can be tested on its own.
class ConsoleUI {
public:
    explicit ConsoleUI(NetworkSimulator& simulator);

    void run();
    bool loadFromFile(const std::string& filename);

private:
    NetworkSimulator& simulator;

    void addRouter();
    void addLink();
    void removeLink();
    void displayTopology() const;
    void findRoute();
    void changeAlgorithm();
    void sendPacket();
    void simulateLinkFailure();
    void checkConnectivity() const;
    void findMinimumCostNetwork() const;
    void displayStatistics() const;
    void saveToFile();

    void displayMenu() const;
    bool printRoute(int source, int destination, const RouteQuery& query) const;
    void printPath(const std::vector<int>& path) const;
    static void printError(Status status, int first = 0, int second = 0);

    static std::string promptLine(const std::string& prompt);
    static int         promptInt(const std::string& prompt);
};
