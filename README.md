# Swarm Engine: High-Performance 2D Agent Simulation

A high-performance, multithreaded 2D engine built from scratch in C++17 to simulate decentralized swarm intelligence, cellular automata, and emergent AI behaviors. This project serves as a technical showcase for low-level systems programming, demonstrating how to maintain a stable 144 FPS while managing 10,000+ autonomous agents.

![Ant Swarm Demo](assets/ants.gif)

## 🚀 Key Technical Features

### 1. Data-Oriented Memory Architecture
To eliminate heap fragmentation and memory leaks, the engine bypasses standard `malloc`/`new` in high-frequency loops:
* **Binned Allocator:** Objects are served from specialized memory bins (32, 64, 128, and 256 bytes) to minimize allocation overhead.
* **Page Pools:** High-density entities are managed in contiguous memory pages, ensuring cache-friendly iteration and $O(1)$ allocation/deallocation.

### 2. Multi-threaded Job System
The engine utilizes a custom **ThreadPool** and a **Fixed-Timestep Accumulator** to maximize CPU utilization across all available cores:
* **Asynchronous Updates:** Heavy cellular automata (pheromone diffusion/blurring) are processed in parallel "chunks," freeing the main thread for sequential entity logic.
* **Spiral of Death Protection:** An accumulator clamp prevents the simulation from "choking" during heavy processing spikes or low-performance intervals.

### 3. Layered Spatial Hash Grid
To bypass $O(N^2)$ bottlenecks, the engine employs a cell-based spatial partitioning system:
* **$O(1)$ Neighbor Querying:** Highly optimized collision detection and neighbor lookups within a fixed radius.
* **Collision Layers:** Entities are partitioned by type (Agents, Resources, Static Obstacles), allowing predators to hunt 10,000 ants without scanning thousands of irrelevant food items.

### 4. GPU Texture Streaming
Pheromone dynamics are simulated on a grid and rendered via real-time GPU texture updates:
* **1D Array Flattening:** Scent data is stored in a cache-friendly 1D array and mapped directly to pixel data.
* **Visual Performance:** Real-time rendering of complex scent gradients via Raylib texture streaming for seamless visual feedback.

### 5. Modular Behavior Tree AI
Agent logic is driven by a custom **Behavior Tree (BT)** implementation featuring `Selector`, `Sequence`, and `BTNode` composites:
* **BTAgent Integration:** Seamlessly integrates complex AI states (Patrolling, Hunting, Fleeing) into the game object lifecycle.

### 6. Engineering Tooling
Lock-Free Logger: A high-performance, ring-buffer based asynchronous logger designed for minimal frame-time impact during heavy I/O.
Chrome Profiler: Integrated instrumentation that exports performance data to Chrome Tracing JSON format, allowing for deep analysis of thread execution and bottlenecks.
Automatic Formatting: Integrated clang-format target to maintain strict coding standards across the engine core and game modules.

---

## 🐜 Showcase: The Ant Swarm

The current simulation demonstrates a complex emergent ecosystem:
* **Specialized Castes:** Worker ants gather food while Soldier ants defend the nest with a dedicated "Swarm" response to intruders.
* **Pheromone Dynamics:** Multi-threaded simulation of four distinct scents (Home, Food, Rally, and Fear) with realistic diffusion and evaporation.
* **Predator/Prey Interaction:** Predators hunt workers to maintain energy levels, emitting a "Fear" aura that triggers a global panic state in the swarm.

---

## 🛠️ Build & Run

The project uses **CMake** and **Raylib** (fetched automatically via `FetchContent`).

### Prerequisites
* C++17 Compiler (GCC/Clang/MSVC)
* CMake 3.10+
* Git

### Quick Start
Use the provided automation script to build and launch the project:

```bash
# Build and Run the current default project (RTS)
./build.sh

# Build and Run the Ant Swarm specifically
./build.sh ants
```
### Advanced Usage

The engine supports multiple projects through a centralized CMakeLists.txt. You can manually switch targets by editing the ACTIVE_GAME variable or passing it via the command line:
# To compile the RTS project manually:
cmake .. -DACTIVE_GAME=rts
make

# To compile the Ants project manually:
cmake .. -DACTIVE_GAME=ants
make
