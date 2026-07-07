#include "DisjointSet.h"
#include <vector>

void DisjointSet::initialise(const std::vector<int>& ids) {
    parent.clear();
    rank.clear();
    for (int id : ids) {
        parent[id] = id;
        rank[id]   = 0;
    }
}

int DisjointSet::find(int x) {
    if (parent[x] != x) {
        parent[x] = find(parent[x]);
    }
    return parent[x];
}

bool DisjointSet::unite(int x, int y) {
    int rootX = find(x);
    int rootY = find(y);

    if (rootX == rootY) return false;

    if (rank[rootX] < rank[rootY]) {
        parent[rootX] = rootY;
    } else if (rank[rootX] > rank[rootY]) {
        parent[rootY] = rootX;
    } else {
        parent[rootY] = rootX;
        ++rank[rootX];
    }
    return true;
}
