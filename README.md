<div align="center">

# 🌐 Network Routing & Failure Recovery Simulator

**A C++17 command-line simulator that models a computer network as a weighted undirected graph.**

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue?style=for-the-badge&logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![Build](https://img.shields.io/badge/Build-CMake-red?style=for-the-badge&logo=cmake)](https://cmake.org/)
[![OOP](https://img.shields.io/badge/Paradigm-OOP-green?style=for-the-badge)]()
[![Algorithms](https://img.shields.io/badge/Algorithms-Dijkstra%20%7C%20BFS%20%7C%20Kruskal-orange?style=for-the-badge)]()

</div>

---

## 📌 What Is This Project?

This is a **fully interactive CLI application** that simulates how packets are routed across a computer network.

The network is modelled as a **weighted, undirected graph** — routers are vertices and network links are weighted edges. The simulator lets you:

- Build a network by adding routers and links
- Route packets using **Dijkstra** (minimum cost) or **BFS** (minimum hops), switchable at runtime
- Simulate **link failures** and watch the system automatically find an alternative route
- Check if the network is **fully connected**
- Compute the **minimum-cost backbone** (MST) using Kruskal's algorithm
- Save and load network topologies from files

> **Purpose:** Demonstrate mastery of C++17, OOP, STL containers, graph algorithms, and the Strategy design pattern — in clean, interview-ready code.

---

## ✨ Features

| Feature | Details |
|---|---|
| 🔀 **Dual Routing Algorithms** | Dijkstra (min cost) and BFS (min hops) — switchable at runtime via Strategy Pattern |
| ⚡ **Link Failure Simulation** | Removes a link, checks if the last packet's route is affected, reroutes automatically |
| 🔗 **Connectivity Check** | DFS-based traversal; lists all unreachable routers on a disconnected network |
| 🌲 **Kruskal's MST** | Finds the cheapest set of links to connect all routers using DSU |
| 📦 **Packet Transmission** | Send packets, trace the route hop-by-hop, track delivery statistics |
| 💾 **File I/O** | Load and save network topology in a simple human-readable format |
| 🛡️ **Input Validation** | Handles duplicate IDs, self-loops, negative weights, missing routers, and more |
| 📊 **Live Statistics** | Tracks routers, links, packets sent/delivered/failed/rerouted |

---

## 🏗️ Project Architecture

### Class Diagram

```
                    ┌─────────────────────────────┐
                    │       NetworkSimulator       │  ← Main controller
                    │─────────────────────────────│
                    │ - network: Graph             │
                    │ - routingStrategy: unique_ptr│
                    │ - statistics counters        │
                    │ - lastRoute: RouteResult     │
                    └──────┬──────────────┬────────┘
                           │ owns         │ owns
               ┌───────────▼──┐     ┌─────▼──────────────────┐
               │    Graph     │     │   <<abstract>>          │
               │──────────────│     │   RoutingStrategy       │
               │ - routers    │     │─────────────────────────│
               │ - adjList    │     │ + findRoute()  = 0      │
               │ - linkCount  │     │ + getAlgorithmName() = 0│
               └──────┬───────┘     └────────┬────────────────┘
                      │ stores               │ inherited by
          ┌───────────┼──────────┐    ┌──────┴──────────────┐
     ┌────▼───┐  ┌────▼──┐ ┌────▼──┐ │                     │
     │ Router │  │ Link  │ │Route  │ │                     │
     │────────│  │───────│ │Result │ ▼                     ▼
     │-id     │  │-src   │ │───────│ DijkstraRouting    BFSRouting
     │-name   │  │-dst   │ │-found │ (min cost)         (min hops)
     │-queue  │  │-weight│ │-path  │ O((V+E)logV)       O(V+E)
     └────────┘  └───────┘ │-cost  │
          │                └───────┘
     ┌────▼───┐
     │ Packet │              ┌──────────────┐
     │────────│              │  DisjointSet │ ← Used only in Kruskal's
     │-id     │              │──────────────│
     │-src    │              │ - parent[]   │
     │-dst    │              │ - rank[]     │
     │-prio   │              │ + find()     │  ← Path compression
     └────────┘              │ + unite()    │  ← Union by rank
                             └──────────────┘
```

### Strategy Pattern (Core Design)

```
NetworkSimulator holds:
    unique_ptr<RoutingStrategy> routingStrategy

At runtime:
    simulator.setRoutingStrategy(make_unique<DijkstraRouting>());
    // or
    simulator.setRoutingStrategy(make_unique<BFSRouting>());

When routing:
    routingStrategy->findRoute(graph, src, dst);
    // ↑ C++ vtable picks the right algorithm — simulator doesn't care which one
```

---

## 📂 Folder Structure

```
NetworkRoutingSimulator/
│
├── CMakeLists.txt            ← Build config (C++17, -Wall -Wextra)
├── README.md
│
├── include/                  ← All class declarations (.h files)
│   ├── Packet.h              ← Data unit (packetId, src, dst, priority)
│   ├── Link.h                ← Weighted edge (src, dst, weight)
│   ├── RouteResult.h         ← Common return type for all routing algorithms
│   ├── Router.h              ← Network node (id, name, packet queue)
│   ├── Graph.h               ← Adjacency-list graph (core data structure)
│   ├── RoutingStrategy.h     ← Abstract base class (Strategy Pattern)
│   ├── DijkstraRouting.h     ← Minimum-cost routing (inherits RoutingStrategy)
│   ├── BFSRouting.h          ← Minimum-hop routing  (inherits RoutingStrategy)
│   ├── DisjointSet.h         ← Union-Find for Kruskal's MST
│   └── NetworkSimulator.h    ← Main controller class
│
├── src/                      ← All implementations (.cpp files)
│   ├── main.cpp
│   ├── Router.cpp
│   ├── Graph.cpp
│   ├── DijkstraRouting.cpp
│   ├── BFSRouting.cpp
│   ├── DisjointSet.cpp
│   └── NetworkSimulator.cpp
│
├── data/
│   └── sample_network.txt    ← 8 routers, 13 links — ready to demo everything
│
└── tests/
    └── test_scenarios.txt    ← Step-by-step test guide
```

---

## 🧱 OOP Concepts — Where They Appear

| Concept | How It's Used |
|---|---|
| **Abstraction** | `RoutingStrategy` defines a pure-virtual interface. The simulator calls `findRoute()` without knowing which algorithm runs underneath |
| **Encapsulation** | `Graph` keeps `routers` and `adjacencyList` private; only exposes safe accessors like `getNeighbours()` |
| **Inheritance** | `DijkstraRouting` and `BFSRouting` both inherit from `RoutingStrategy` and override `findRoute()` |
| **Runtime Polymorphism** | `NetworkSimulator` holds `unique_ptr<RoutingStrategy>`; C++ vtable dispatches `findRoute()` to the correct subclass at runtime |
| **Composition** | `NetworkSimulator` *owns* a `Graph` and a `RoutingStrategy` — it composes them rather than inheriting |

---

## 🗃️ Data Structures — What & Why

| Data Structure | Used In | Why |
|---|---|---|
| `unordered_map<int, Router>` | `Graph::routers` | O(1) router lookup by ID |
| `unordered_map<int, vector<pair<int,int>>>` | `Graph::adjacencyList` | Adjacency list — O(V+E) space vs O(V²) for a matrix |
| `priority_queue` (min-heap) | `DijkstraRouting` | Always expands the cheapest router first |
| `queue<int>` | `BFSRouting` | FIFO — explores hops level-by-level |
| `queue<Packet>` | `Router::packetQueue` | Packets processed in arrival order |
| `unordered_set<int>` | BFS visited set, connectivity DFS | O(1) membership check — "already visited?" |
| `unordered_map<int,int>` | `dist[]` and `parent[]` in both algorithms | O(1) distance update and path reconstruction |
| `vector<int>` | `RouteResult::path` | Ordered sequence of router IDs on the path |
| `vector<Link>` | Kruskal's edge list | Sortable collection of all edges |
| **Disjoint Set Union** | `DisjointSet`, Kruskal's | Near-O(1) cycle detection using path compression + union by rank |

---

## ⚙️ Algorithms

### 1. Dijkstra's Shortest Path — `DijkstraRouting.cpp`

> **Finds the route with minimum total link-weight cost.**

```
Input:  Graph, source ID, destination ID
Output: RouteResult { pathFound, path[], totalCost }

Steps:
  1. dist[source] = 0,  dist[all others] = ∞
  2. Push (0, source) onto min-heap
  3. Pop cheapest router u
     - Skip if stale (cost > dist[u])
     - Stop if u == destination
     - For each neighbour v: if dist[u] + weight < dist[v] → update + push
  4. Reconstruct path via parent[] map (walk backwards, then reverse)

Complexity:  Time O((V+E) log V)   Space O(V+E)
```

### 2. BFS Routing — `BFSRouting.cpp`

> **Finds the route with the fewest hops (links), ignoring weights.**

```
Input:  Graph, source ID, destination ID
Output: RouteResult { pathFound, path[], totalCost (actual weight sum) }

Steps:
  1. Enqueue source; mark visited
  2. Dequeue u → explore all unvisited neighbours v
     - Mark v visited, set parent[v] = u
     - If v == destination → stop
     - Otherwise enqueue v
  3. Reconstruct path via parent[]

Complexity:  Time O(V+E)   Space O(V)
```

> **Why Dijkstra ≠ BFS on the same network:**
> BFS finds the 3-hop path `Alpha→Charlie→Echo→Hotel` (cost 20).
> Dijkstra finds the 4-hop path `Alpha→Charlie→Echo→Golf→Hotel` (cost 17).
> Fewer hops ≠ cheaper cost when link weights differ.

### 3. DFS Connectivity Check — `NetworkSimulator.cpp`

> **Determines if every router can reach every other router.**

```
Start DFS from any router → recursively visit all reachable neighbours.
If visited count == total routers → CONNECTED
Otherwise → DISCONNECTED (lists unreachable routers)

Complexity:  Time O(V+E)   Space O(V)
```

### 4. Kruskal's MST — `NetworkSimulator.cpp` + `DisjointSet.cpp`

> **Finds the cheapest set of links that connects all routers.**

```
Steps:
  1. Collect all edges → sort by weight ascending
  2. Initialise DSU with all router IDs
  3. For each edge (u, v, w):
       if unite(u, v) succeeds → add to MST  (different components → no cycle)
       else → skip  (same component → would create a cycle)
  4. If MST has < V-1 edges → network is disconnected → MST impossible

Complexity:  Time O(E log E)   Space O(V+E)
```

### 5. Disjoint Set Union (Union-Find) — `DisjointSet.cpp`

```cpp
find(x)    → walks up parent[] to root, compresses path on the way back
unite(x,y) → attaches shorter tree under taller tree (union by rank)
             returns false if already in same set (cycle detected)

Complexity per operation:  O(α(V)) ≈ O(1) for all practical inputs
```

---

## 🔨 Build & Run

### Prerequisites
- **C++17** compiler — Apple Clang / GCC 7+ / Clang 5+
- **CMake 3.14+**

### Build

```bash
git clone <your-repo-url>
cd NetworkRoutingSimulator

mkdir build
cd build
cmake ..
cmake --build .
```

### Run

```bash
# With sample network pre-loaded (recommended)
./NetworkRoutingSimulator ../data/sample_network.txt

# Empty network (add routers/links manually)
./NetworkRoutingSimulator
```

---

## 🖥️ Sample CLI Interaction

```
  ╔══════════════════════════════════════════════╗
  ║   NETWORK ROUTING & FAILURE RECOVERY         ║
  ║              SIMULATOR  v1.0                 ║
  ║   C++17 · OOP · Graph Algorithms · STL       ║
  ╚══════════════════════════════════════════════╝

  Loading network from: data/sample_network.txt
  [OK] Network loaded — Routers: 8   Links: 13

  ╠══════════════════════════════════════════════╣
  ║  Active Algorithm: Dijkstra (Minimum Cost)   ║
  ╠══════════════════════════════════════════════╣
  ║  1.  Add Router                              ║
  ║  5.  Find Shortest Route                     ║
  ║  6.  Change Routing Algorithm                ║
  ║  7.  Send Packet                             ║
  ║  8.  Simulate Link Failure        ← KEY      ║
  ║  10. Find Minimum Cost Network (MST)         ║
  ╚══════════════════════════════════════════════╝

  Choice: 5
  Source: 1   Destination: 8

  Algorithm : Dijkstra (Minimum Cost)
  Route     : Alpha[1] → Charlie[3] → Echo[5] → Golf[7] → Hotel[8]
  Total Cost: 17

  Choice: 6 → Select BFS

  Choice: 5
  Source: 1   Destination: 8

  Algorithm : BFS (Minimum Hops)
  Route     : Alpha[1] → Charlie[3] → Echo[5] → Hotel[8]
  Total Cost: 20   ← 3 hops, but costs more
```

---

## ⚡ Link Failure & Rerouting Demo

```
Step 1 — Send packet (Dijkstra active)
  Route: Alpha → Charlie → Echo → Golf → Hotel   cost=17 ✓

Step 2 — Simulate link failure: Echo(5) ↔ Golf(7)
  Choice: 8 → Source: 5, Destination: 7

  ╔══════════════════════════════════════╗
  ║       LINK FAILURE SIMULATION        ║
  ╚══════════════════════════════════════╝

  ⚡ Failed Link: Router 5 <──> Router 7

  Previous Route: Alpha[1] → Charlie[3] → Echo[5] → Golf[7] → Hotel[8]
  Previous Cost : 17

  Link removed from network.
  Recomputing route...

  ✓  New Route Found!
     New Route : Alpha[1] → Charlie[3] → Bravo[2] → Delta[4] → Foxtrot[6] → Golf[7] → Hotel[8]
     New Cost  : 19

Step 3 — Statistics
  Total Packets Sent   : 1
  Successful Deliveries: 1
  Rerouted Packets     : 1
```

---

## 🗺️ Sample Network Topology

```
data/sample_network.txt — 8 routers, 13 links

         4          1
  Alpha ─────── Bravo ─────── Charlie
   │  \          │               │
   2   15        5               8
   │    \        │               │
Charlie  Delta ──┘            Echo
            │  \                │  \
            2   6               3   10
            │    \              │    \
          Echo  Foxtrot ─── Golf ─── Hotel
                    │    1    │   4
                    9         │
                    └──────── Hotel
                           9

Dijkstra 1→8:  Alpha→Charlie→Echo→Golf→Hotel       cost = 17  (4 hops)
BFS      1→8:  Alpha→Charlie→Echo→Hotel             cost = 20  (3 hops) ← different!
Kruskal  MST:  7 edges connecting all 8 routers     cost = 18
```

---

## 🔁 Execution Flow

```
main()
  └── NetworkSimulator simulator         (constructor sets Dijkstra as default)
  └── simulator.loadFromFile(...)        (optional: load topology from file)
  └── simulator.run()                    (interactive loop)
        └── displayMenu()
        └── read user choice
        └── switch(choice):
              case 7 (Send Packet):
                sendPacket(id, src, dst, priority)
                  └── findAndDisplayRoute(src, dst)
                        └── routingStrategy->findRoute(graph, src, dst)
                              └── [Dijkstra or BFS depending on current strategy]
                        └── display path + cost
                  └── update statistics, save lastRoute
              case 8 (Link Failure):
                simulateLinkFailure(src, dst)
                  └── check if link exists
                  └── check if lastRoute uses this link
                  └── graph.removeLink(src, dst)
                  └── routingStrategy->findRoute(...)  [reroute]
                  └── display new route or "unreachable"
```

---

## 🧠 Interview Q&A

**Q1: Why adjacency list instead of adjacency matrix?**
> Adjacency matrix needs O(V²) space. A 1000-router network with 3000 links would waste 997,000 cells. Adjacency list uses only O(V+E) space — exactly what's needed for sparse graphs.

**Q2: How does Dijkstra guarantee the optimal path?**
> The min-heap always processes the cheapest unvisited router. With non-negative weights, once a router is popped, its distance is final — no future path can possibly be cheaper (greedy choice property).

**Q3: When does BFS give a different result than Dijkstra?**
> BFS ignores weights — it counts every link as 1 hop. If a 3-hop path costs 20 and a 4-hop path costs 17, BFS picks 3 hops, Dijkstra picks cost 17. They diverge whenever weights are unequal.

**Q4: What does the Strategy Pattern give you here?**
> You can swap routing algorithms at runtime without modifying `NetworkSimulator`. Adding a new algorithm (e.g., Bellman-Ford) only requires creating a new subclass — no changes to existing code (Open/Closed Principle).

**Q5: Why does DSU need path compression and union by rank?**
> Without them, `find()` degrades to O(n) in the worst case (a long chain of parents). Path compression flattens the tree on each `find()`. Union by rank prevents trees from growing tall. Together they give O(α(n)) ≈ O(1) per operation.

---

## 📄 Resume Description

> Designed and implemented a **C++17 network routing simulator** using a weighted undirected graph (adjacency list), demonstrating OOP principles — inheritance, runtime polymorphism, encapsulation, and composition — across 10 modular classes with Dijkstra O((V+E)logV), BFS O(V+E), and Kruskal's MST O(ElogE) with Disjoint Set Union; applied the **Strategy design pattern** for runtime-switchable algorithms and simulated autonomous link-failure detection with automatic rerouting, compiled with zero warnings.

---

## 🚀 Future Improvements

- [ ] Router removal (requires adjacency list cleanup)
- [ ] Multiple simultaneous link failures (cascade simulation)
- [ ] Bellman-Ford as a third routing strategy
- [ ] ASCII network graph visualization
- [ ] Priority-based packet queuing at each router
- [ ] Unit tests with GoogleTest or Catch2

---

<div align="center">
  <b>Built with C++17 · STL · CMake · Graph Algorithms · OOP · Strategy Pattern</b>
</div>
