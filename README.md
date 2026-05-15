# Custom C++17 Game Engine & Simulation Sandbox

A high-performance, modular C++17 monorepo containing a custom game engine built from scratch, alongside multiple independent simulations (Deterministic Lockstep RTS, Emergent Ant Colony AI). 

This project was built to explore low-level systems engineering, Data-Oriented Design (DOD), custom memory allocators, lock-free concurrency, and advanced networking architectures.

## Repository Structure

* **[`engine/`](engine/README.md):** The core reusable C++ engine. Contains custom memory pools, lock-free asynchronous logging, a multithreading pool, ENet networking wrappers, and spatial partitioning systems.
* **[`games/rts/`](games/rts/README.md):** A multiplayer RTS tech demo featuring Deterministic Lockstep Networking and DOD-based Boids flocking for thousands of units.
* **[`games/ants/`](games/ants/README.md):** An artificial life simulation utilizing Behavior Trees and pheromone diffusion grids to simulate emergent colony AI.

## Build Instructions

This project uses modern `CMake` and `FetchContent` to guarantee a completely portable build system. All external dependencies (Raylib, ENet) are automatically downloaded and statically linked during the CMake configuration phase.

### Requirements
* C++17 compatible compiler (GCC, Clang, or MSVC)
* CMake 3.10+

### Building the Project
You can build the project using the provided shell script or standard CMake commands. The build system is configured to compile one active game at a time.

```bash
# Clone the repository
git clone [https://github.com/YOUR_USERNAME/YOUR_REPO.git](https://github.com/YOUR_USERNAME/YOUR_REPO.git)
cd YOUR_REPO

# Generate and build (defaults to RTS)
mkdir build && cd build
cmake .. -DACTIVE_GAME="rts" 
make -j$(nproc)

```
##Formatting
The project includes a custom FormatCode target. Running make FormatCode will automatically format all engine and game source files using clang-format based on the .clang-format style configuration.
