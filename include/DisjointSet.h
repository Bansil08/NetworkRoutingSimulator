#pragma once

#include <unordered_map>
#include <vector>

class DisjointSet {
public:
    void initialise(const std::vector<int>& ids);
    int find(int x);
    bool unite(int x, int y);

private:
    std::unordered_map<int, int> parent;
    std::unordered_map<int, int> rank;
};
