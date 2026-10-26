#pragma once

#include <string>
#include <queue>
#include "Packet.h"

class Router {
public:
    Router() = default;
    Router(int id, const std::string& name);

    int getId() const;
    const std::string& getName() const;

    void enqueuePacket(const Packet& packet);
    bool hasPackets() const;
    Packet dequeuePacket();

    void display() const;

private:
    int id = 0;
    std::string name;
    std::queue<Packet> packetQueue;
};
