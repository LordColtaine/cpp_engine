# Swarm Commander: RTS Edition

A multiplayer Real-Time Strategy engine designed to handle thousands of active units simultaneously over a network.

## Architecture: Deterministic Lockstep
Synchronizing 10,000 units over the internet using standard State Replication (sending X/Y coordinates) is impossible due to bandwidth limitations. This engine implements **Deterministic Lockstep**, guaranteeing that all clients perfectly mirror each other by executing the exact same mathematical simulation at the exact same time.

* **Server Metronome:** A headless server acts as a strictly timed metronome (100ms ticks). It collects client inputs, packages them into a "Turn", and broadcasts them simultaneously.
* **The Handshake & Start Gun:** To prevent start-time race conditions, the server waits for a strict `CLIENT_READY` packet from all peers before assigning Player IDs and firing a synchronized `START_GAME` packet.
* **Turn Buffers & Fast-Forwarding:** Clients decouple network traffic from the physics engine using a local `std::unordered_map` Turn Buffer. If a client lags behind the server, it instantly triggers a physics fast-forward (`accumulator` loop) to silently catch up to the current turn without skipping frames.
* **Networked Commands:** All user inputs (Selection Boxes, Rally Points) are serialized into C-structs, routed through the server, and executed simultaneously on all machines.

## Architecture: Data-Oriented Design (DOD)
Traditional Object-Oriented Programming (OOP) causes massive CPU Cache misses when iterating over thousands of units. 

This engine separates visual logic from physics logic:
* **`UnitPhysics`:** A lightweight, tightly packed C-struct containing only the data necessary for math (position, velocity, acceleration).
* **Pre-allocated Pools:** Physics structs are stored contiguously in a `std::vector` (the `m_PhysicsPool`). When computing Flocking behavior, the CPU pre-fetches the entire array into the L1 cache, drastically increasing execution speed.

## Flocking Algorithm (Boids)
Units maneuver using Craig Reynolds' Boids algorithm, integrated with the `SpatialGrid` for rapid neighbor detection:
1. **Separation:** Steer to avoid crowding local flockmates.
2. **Alignment:** Steer towards the average heading of local flockmates.
3. **Cohesion:** Steer to move toward the average position of local flockmates.
4. **Avoidance:** Soft-dodge steering forces to navigate around spherical obstacles.

## How to Run the Multiplayer Test
1. Compile the project (Ensure `ACTIVE_GAME="rts"` in CMake).
2. Start the headless server: `./server`
3. Start Client 1: `./game` (Will connect and wait)
4. Start Client 2: `./game` (Will trigger the start gun and unlock both screens)
5. Use `Left-Click Drag` to select units, and `Right-Click` to assign network-replicated Rally points.
