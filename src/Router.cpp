#include "Router.h"
#include <iostream>

Router::Router(int id, const std::string& name) : id(id), name(name) {}

int Router::getId() const { return id; }
const std::string& Router::getName() const { return name; }

void Router::enqueuePacket(const Packet& packet) {
    packetQueue.push(packet);
}

bool Router::hasPackets() const {
    return !packetQueue.empty();
}

Packet Router::dequeuePacket() {
    Packet p = packetQueue.front();
    packetQueue.pop();
    return p;
}

void Router::display() const {
    std::cout << "  Router [" << id << "] " << name;
    if (!packetQueue.empty()) {
        std::cout << "  (queued packets: " << packetQueue.size() << ")";
    }
    std::cout << "\n";
}
