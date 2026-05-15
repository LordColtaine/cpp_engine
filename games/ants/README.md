# Emergent Ant Colony Simulation

An Artificial Life simulation demonstrating emergent behavior through simple local rules, pheromone diffusion, and Behavior Trees.

## Simulation Mechanics

### The Pheromone Grid
Ants communicate indirectly through stigmergy—modifying their environment. 
* The map is divided into a dense grid of float values.
* As ants walk, they deposit "To Home" or "To Food" pheromones based on their current state.
* The grid is processed every frame to simulate mathematical **Diffusion** (blurring values to neighboring cells) and **Evaporation** (slowly reducing values over time).
* Ants sample the grid in front of them and steer toward the highest concentration of the desired pheromone, creating organic, dynamic highways.

### Behavior Tree AI (`BTAgent`)
Individual ant logic is governed by a hierarchical Behavior Tree. The tree is evaluated every frame to switch contexts without complex finite state machine (FSM) spaghetti code.
* **Predator Evasion:** Highest priority. If a predator is detected in the spatial grid, drop food and run away.
* **Foraging State:** Wander randomly while sampling the "To Food" pheromone grid. If food is found, grab it and switch states.
* **Return State:** Steer based on the "To Home" pheromone grid to deposit food back at the colony.

### Data Orientation & Engine Usage
* The simulation utilizes the core engine's `SpatialGrid` to allow Predators to efficiently hunt thousands of ants without causing $O(N^2)$ frame-drops.
* Heavy memory allocation (spawning new ants, allocating BTNodes) utilizes the `BinnedAllocator` to prevent memory fragmentation during long-running simulations.
