# C++ Multithreaded Agent-Based Swarm Simulator

A high-performance, multithreaded 2D engine built from scratch in C++ to simulate decentralized swarm intelligence and cellular automata.


![Ant Swarm Demo](assets/ants.gif)

## 🚀 Technical Features
* **Custom Engine Architecture:** Built in C++17 with strict adherence to Data-Oriented Design and cache-friendly memory layouts.
* **Asynchronous Job System:** Utilizes a custom Thread Pool with mutex-guarded work queues to process heavy cellular automata (pheromone blurring) concurrently, freeing the main thread for strictly sequential, high-speed entity logic.
* **Custom Memory Allocators:** Replaced standard `malloc`/`new` with a custom Binned Memory Allocator and Page Pool to eliminate heap fragmentation and memory leaks.
* **Spatial Hash Grid:** $O(1)$ spatial partitioning for highly optimized collision detection and neighbor querying, completely bypassing $O(N^2)$ bottlenecks.
* **GPU Texture Streaming:** Pheromone cellular automata rendered via 1D array flattening and real-time GPU texture updates via Raylib.

## 🛠️ Build Instructions
```bash
git clone [https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git](https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git)
cd YOUR_REPO_NAME
./build.sh all
