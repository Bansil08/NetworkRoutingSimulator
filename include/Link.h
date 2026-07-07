#pragma once

struct Link {
    int source;
    int destination;
    int weight;

    Link() : source(0), destination(0), weight(0) {}
    Link(int src, int dst, int w) : source(src), destination(dst), weight(w) {}
};
