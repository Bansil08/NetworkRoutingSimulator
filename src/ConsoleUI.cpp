#include "ConsoleUI.h"
#include "DijkstraRouting.h"
#include "BFSRouting.h"

#include <iostream>
#include <iomanip>

ConsoleUI::ConsoleUI(NetworkSimulator& simulator) : simulator(simulator) {}

// ── Messages ─────────────────────────────────────────────────────────────────

void ConsoleUI::printError(Status status, int first, int second) {
    std::cout << "  [Error] ";
    switch (status) {
        case Status::EmptyRouterName:     std::cout << "Router name cannot be empty."; break;
        case Status::DuplicateRouter:     std::cout << "Router with ID " << first << " already exists."; break;
        case Status::SourceNotFound:      std::cout << "Source router " << first << " not found."; break;
        case Status::DestinationNotFound: std::cout << "Destination router " << second << " not found."; break;
        case Status::SelfLoop:            std::cout << "Self-loops are not allowed (router " << first << ")."; break;
        case Status::NegativeWeight:      std::cout << "Negative link weights are not supported."; break;
        case Status::DuplicateLink:       std::cout << "Link between routers " << first << " and " << second << " already exists."; break;
        case Status::LinkNotFound:        std::cout << "No link between routers " << first << " and " << second << "."; break;
        case Status::EmptyNetwork:        std::cout << "Network is empty."; break;
        case Status::FileOpenFailed:      std::cout << "File operation failed."; break;
        case Status::MalformedLine:       std::cout << "Malformed line."; break;
        case Status::Ok:                  break;
    }
    std::cout << "\n";
}

void ConsoleUI::printPath(const std::vector<int>& path) const {
    const Graph& network = simulator.getNetwork();
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) std::cout << " → ";
        int id = path[i];
        if (network.hasRouter(id)) {
            std::cout << network.getRouter(id).getName() << "[" << id << "]";
        } else {
            std::cout << "[" << id << "]";
        }
    }
    std::cout << "\n";
}

// Prints a route query result. Returns true if a usable path was found.
bool ConsoleUI::printRoute(int source, int destination, const RouteQuery& query) const {
    if (query.status != Status::Ok) {
        printError(query.status, source, destination);
        return false;
    }
    if (source == destination) {
        std::cout << "  [Info] Source and destination are the same router.\n";
        return true;
    }

    const Graph& network = simulator.getNetwork();
    std::string algorithm = simulator.getAlgorithmName();

    std::cout << "\n  Algorithm : " << algorithm << "\n";
    std::cout << "  From      : [" << source      << "] "
              << network.getRouter(source).getName()      << "\n";
    std::cout << "  To        : [" << destination << "] "
              << network.getRouter(destination).getName() << "\n\n";

    if (!query.route.pathFound) {
        std::cout << "  ✗  No route found between routers "
                  << source << " and " << destination << ".\n\n";
        return false;
    }

    std::cout << "  Route     : ";
    printPath(query.route.path);
    std::cout << "  Total Cost: " << query.route.totalCost
              << (algorithm.find("BFS") != std::string::npos
                  ? "  (actual link-weight sum along BFS hop-path)" : "") << "\n\n";
    return true;
}

// ── Menu actions ─────────────────────────────────────────────────────────────

void ConsoleUI::addRouter() {
    int id           = promptInt("  Router ID   : ");
    std::string name = promptLine("  Router Name : ");
    Status status = simulator.addRouter(id, name);
    if (status != Status::Ok) { printError(status, id); return; }
    std::cout << "  [OK] Router [" << id << "] '" << name << "' added.\n";
}

void ConsoleUI::addLink() {
    int src    = promptInt("  Source Router ID      : ");
    int dst    = promptInt("  Destination Router ID : ");
    int weight = promptInt("  Link Weight           : ");
    Status status = simulator.addLink(src, dst, weight);
    if (status != Status::Ok) { printError(status, src, dst); return; }
    std::cout << "  [OK] Link added: Router " << src << " <──["
              << weight << "]──> Router " << dst << "\n";
}

void ConsoleUI::removeLink() {
    int src = promptInt("  Source Router ID      : ");
    int dst = promptInt("  Destination Router ID : ");
    Status status = simulator.removeLink(src, dst);
    if (status != Status::Ok) { printError(status, src, dst); return; }
    std::cout << "  [OK] Link removed: Router " << src << " <──> Router " << dst << "\n";
}

void ConsoleUI::displayTopology() const {
    const Graph& network = simulator.getNetwork();
    if (network.getRouterCount() == 0) {
        std::cout << "  [Info] Network is empty. Add routers and links first.\n";
        return;
    }

    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         NETWORK TOPOLOGY             ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";

    for (int id : network.getSortedRouterIds()) {
        std::cout << "  Router [" << std::setw(2) << id << "] "
                  << network.getRouter(id).getName() << "\n";

        const auto& neighbours = network.getNeighbours(id);
        if (neighbours.empty()) {
            std::cout << "      (no links)\n";
        } else {
            for (const auto& [neighbour, weight] : neighbours) {
                std::cout << "      ── [weight " << std::setw(3) << weight << "] ──> "
                          << "[" << neighbour << "] "
                          << network.getRouter(neighbour).getName() << "\n";
            }
        }
        std::cout << "\n";
    }

    std::cout << "  Routers: " << network.getRouterCount()
              << "   Links: " << network.getLinkCount() << "\n\n";
}

void ConsoleUI::findRoute() {
    int src = promptInt("  Source Router ID      : ");
    int dst = promptInt("  Destination Router ID : ");
    printRoute(src, dst, simulator.findRoute(src, dst));
}

void ConsoleUI::changeAlgorithm() {
    std::cout << "  Select Algorithm:\n";
    std::cout << "    1. Dijkstra (Minimum Cost)\n";
    std::cout << "    2. BFS (Minimum Hops)\n";
    int alg = promptInt("  Choice: ");
    if (alg == 1) {
        simulator.setRoutingStrategy(std::make_unique<DijkstraRouting>());
    } else if (alg == 2) {
        simulator.setRoutingStrategy(std::make_unique<BFSRouting>());
    } else {
        std::cout << "  [Error] Invalid choice.\n";
        return;
    }
    std::cout << "  [OK] Routing algorithm changed to: " << simulator.getAlgorithmName() << "\n";
}

void ConsoleUI::sendPacket() {
    int src      = promptInt("  Source Router ID      : ");
    int dst      = promptInt("  Destination Router ID : ");
    int priority = promptInt("  Packet Priority (1-5) : ");

    PacketReport report = simulator.sendPacket(src, dst, priority);

    std::cout << "\n  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │         PACKET TRANSMISSION              │\n";
    std::cout << "  └─────────────────────────────────────────┘\n";
    std::cout << "  Packet ID : #" << report.packetId << "\n";
    std::cout << "  Priority  : " << report.priority  << "\n";

    printRoute(src, dst, RouteQuery{report.status, report.route});

    if (!report.delivered()) {
        std::cout << "  ✗  Packet #" << report.packetId << " could not be delivered.\n\n";
        return;
    }

    const Graph& network = simulator.getNetwork();
    std::cout << "  Traversal : ";
    for (size_t i = 0; i < report.route.path.size(); ++i) {
        if (i > 0) std::cout << " → ";
        std::cout << network.getRouter(report.route.path[i]).getName();
    }
    std::cout << "\n  ✓  Packet #" << report.packetId << " delivered successfully.\n\n";
}

void ConsoleUI::simulateLinkFailure() {
    int src = promptInt("  Failed Link — Source Router ID      : ");
    int dst = promptInt("  Failed Link — Destination Router ID : ");

    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║       LINK FAILURE SIMULATION        ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";

    LinkFailureReport report = simulator.simulateLinkFailure(src, dst);

    if (report.status != Status::Ok) {
        printError(report.status, src, dst);
        std::cout << "\n";
        return;
    }

    std::cout << "  ⚡ Failed Link: Router " << src << " <──> Router " << dst << "\n\n";

    if (report.affectedLastRoute) {
        std::cout << "  Previous Route: ";
        printPath(report.previousRoute.path);
        std::cout << "  Previous Cost : " << report.previousRoute.totalCost << "\n\n";
    }

    std::cout << "  Link removed from network.\n\n";

    if (!report.affectedLastRoute) {
        std::cout << "  [Info] The failed link was NOT on the last known route.\n"
                  << "         No rerouting is required.\n\n";
        return;
    }

    std::cout << "  Recomputing route...\n\n";

    if (!report.newRoute.pathFound) {
        std::cout << "  ✗  DESTINATION UNREACHABLE AFTER LINK FAILURE\n";
        std::cout << "     No alternative path from Router " << report.routeSource
                  << " to Router " << report.routeDestination << ".\n\n";
    } else {
        std::cout << "  ✓  New Route Found!\n";
        std::cout << "     New Route : ";
        printPath(report.newRoute.path);
        std::cout << "     New Cost  : " << report.newRoute.totalCost << "\n\n";
    }
}

void ConsoleUI::checkConnectivity() const {
    std::cout << "\n  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │         CONNECTIVITY CHECK               │\n";
    std::cout << "  └─────────────────────────────────────────┘\n\n";

    ConnectivityReport report = simulator.checkConnectivity();

    if (report.routerCount == 0) {
        std::cout << "  [Info] Network is empty.\n\n";
        return;
    }

    if (report.isConnected()) {
        std::cout << "  ✓  Network is FULLY CONNECTED.\n";
        std::cout << "     All " << report.routerCount << " routers can communicate.\n\n";
        return;
    }

    std::cout << "  ✗  Network is DISCONNECTED.\n";
    std::cout << "     Reachable from Router " << report.startRouter << ": "
              << report.reachableCount << " / " << report.routerCount << " routers.\n\n";

    const Graph& network = simulator.getNetwork();
    std::cout << "  Unreachable Routers:\n";
    for (int id : report.unreachable) {
        std::cout << "     ✗  Router [" << id << "] " << network.getRouter(id).getName() << "\n";
    }
    std::cout << "\n";
}

void ConsoleUI::findMinimumCostNetwork() const {
    std::cout << "\n  ┌─────────────────────────────────────────┐\n";
    std::cout << "  │     MINIMUM COST NETWORK (Kruskal MST)  │\n";
    std::cout << "  └─────────────────────────────────────────┘\n\n";

    const Graph& network = simulator.getNetwork();
    int routerCount = network.getRouterCount();
    if (routerCount == 0) {
        std::cout << "  [Info] Network is empty.\n\n";
        return;
    }
    if (routerCount == 1) {
        std::cout << "  [Info] Only one router. No links needed.\n\n";
        return;
    }
    if (network.getLinkCount() == 0) {
        std::cout << "  [Info] No links in the network. Cannot build MST.\n\n";
        return;
    }

    MstResult mst = simulator.findMinimumCostNetwork();

    if (!mst.connected) {
        std::cout << "  ✗  Cannot build MST — Network is DISCONNECTED.\n";
        std::cout << "     Found only " << mst.edges.size() << " MST edge(s), need "
                  << routerCount - 1 << ".\n\n";
        return;
    }

    std::cout << "  ✓  Minimum Cost Network found!\n\n";
    std::cout << "  MST Edges:\n";
    for (const Link& e : mst.edges) {
        const std::string& sName = network.getRouter(e.source).getName();
        const std::string& dName = network.getRouter(e.destination).getName();
        std::cout << "     " << sName << " [" << e.source << "] <──["
                  << e.weight << "]──> [" << e.destination << "] " << dName << "\n";
    }
    std::cout << "\n  Total MST Cost: " << mst.totalCost << "\n\n";
}

void ConsoleUI::displayStatistics() const {
    Statistics stats = simulator.getStatistics();

    std::cout << "\n  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║          NETWORK STATISTICS          ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";
    std::cout << "  Routers              : " << stats.routerCount       << "\n";
    std::cout << "  Links                : " << stats.linkCount         << "\n";
    std::cout << "  Active Algorithm     : " << stats.algorithmName     << "\n\n";
    std::cout << "  Total Packets Sent   : " << stats.totalPacketsSent  << "\n";
    std::cout << "  Successful Deliveries: " << stats.successfulPackets << "\n";
    std::cout << "  Failed Deliveries    : " << stats.failedPackets     << "\n";
    std::cout << "  Rerouted Packets     : " << stats.reroutedPackets   << "\n\n";
}

bool ConsoleUI::loadFromFile(const std::string& filename) {
    LoadReport report = simulator.loadFromFile(filename);
    if (report.status == Status::FileOpenFailed) {
        std::cout << "  [Error] Cannot open file: " << filename << "\n";
        return false;
    }

    for (const LoadIssue& issue : report.issues) {
        if (issue.status == Status::MalformedLine) {
            std::cout << "  [Warning] Malformed " << (issue.isLinkLine ? "link" : "router")
                      << " line: " << issue.line << "\n";
        } else {
            printError(issue.status, issue.first, issue.second);
        }
    }

    const Graph& network = simulator.getNetwork();
    std::cout << "  [OK] Network loaded from '" << filename << "'\n";
    std::cout << "       Routers: " << network.getRouterCount()
              << "  Links: " << network.getLinkCount() << "\n";
    return true;
}

void ConsoleUI::saveToFile() {
    std::string filename = promptLine("  Filename: ");
    if (simulator.saveToFile(filename) != Status::Ok) {
        std::cout << "  [Error] Cannot write to file: " << filename << "\n";
        return;
    }
    std::cout << "  [OK] Network saved to '" << filename << "'\n";
}

// ── Input helpers ────────────────────────────────────────────────────────────

std::string ConsoleUI::promptLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int ConsoleUI::promptInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) return 0;  // EOF: let run() exit
        if (line.empty()) continue;
        try {
            return std::stoi(line);
        } catch (...) {
            std::cout << "  [Error] Please enter a valid integer.\n";
        }
    }
}

// ── Main loop ────────────────────────────────────────────────────────────────

void ConsoleUI::displayMenu() const {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════╗\n";
    std::cout << "  ║    NETWORK ROUTING & FAILURE RECOVERY        ║\n";
    std::cout << "  ║              SIMULATOR                       ║\n";
    std::cout << "  ╠══════════════════════════════════════════════╣\n";
    std::cout << "  ║  Active Algorithm: " << std::left << std::setw(26)
              << simulator.getAlgorithmName() << std::right << "║\n";
    std::cout << "  ╠══════════════════════════════════════════════╣\n";
    std::cout << "  ║  1.  Add Router                              ║\n";
    std::cout << "  ║  2.  Add Network Link                        ║\n";
    std::cout << "  ║  3.  Remove Network Link                     ║\n";
    std::cout << "  ║  4.  Display Network Topology                ║\n";
    std::cout << "  ║  5.  Find Shortest Route                     ║\n";
    std::cout << "  ║  6.  Change Routing Algorithm                ║\n";
    std::cout << "  ║  7.  Send Packet                             ║\n";
    std::cout << "  ║  8.  Simulate Link Failure                   ║\n";
    std::cout << "  ║  9.  Check Network Connectivity              ║\n";
    std::cout << "  ║  10. Find Minimum Cost Network (MST)         ║\n";
    std::cout << "  ║  11. Display Network Statistics              ║\n";
    std::cout << "  ║  12. Load Network From File                  ║\n";
    std::cout << "  ║  13. Save Network To File                    ║\n";
    std::cout << "  ║  0.  Exit                                    ║\n";
    std::cout << "  ╚══════════════════════════════════════════════╝\n";
    std::cout << "  Choice: ";
}

void ConsoleUI::run() {
    std::string choiceStr;
    while (true) {
        displayMenu();
        if (!std::getline(std::cin, choiceStr)) {
            std::cout << "\n  Input closed. Goodbye!\n\n";
            return;
        }

        int choice = -1;
        try { choice = std::stoi(choiceStr); } catch (...) {}

        std::cout << "\n";

        switch (choice) {
            case 1:  addRouter();              break;
            case 2:  addLink();                break;
            case 3:  removeLink();             break;
            case 4:  displayTopology();        break;
            case 5:  findRoute();              break;
            case 6:  changeAlgorithm();        break;
            case 7:  sendPacket();             break;
            case 8:  simulateLinkFailure();    break;
            case 9:  checkConnectivity();      break;
            case 10: findMinimumCostNetwork(); break;
            case 11: displayStatistics();      break;
            case 12: loadFromFile(promptLine("  Filename: ")); break;
            case 13: saveToFile();             break;
            case 0:
                std::cout << "  Goodbye!\n\n";
                return;
            default:
                std::cout << "  [Error] Invalid menu choice. Enter 0–13.\n";
                break;
        }
    }
}
