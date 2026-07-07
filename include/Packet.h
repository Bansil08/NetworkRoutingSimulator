#pragma once

struct Packet {
    int packetId;
    int source;
    int destination;
    int priority;

    Packet() : packetId(0), source(0), destination(0), priority(0) {}
    Packet(int id, int src, int dst, int prio = 1)
        : packetId(id), source(src), destination(dst), priority(prio) {}
};
