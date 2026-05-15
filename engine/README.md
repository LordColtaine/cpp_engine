# Core Systems Engine

The foundational C++17 framework powering the simulations in this repository. It is designed to be completely decoupled from game logic, focusing strictly on high-performance memory management, lock-free concurrency, and networking.

## Core Architectures

### Custom Memory Allocation
Bypassing the OS-level `malloc` overhead is critical for high-entity simulations.
* **`MemoryPool`:** A continuous chunk-based memory allocator that grows dynamically. Guarantees cache locality for frequently spawned entities.
* **`BinnedAllocator`:** A custom allocator that categorizes dynamic allocations into fixed-size bins (32, 64, 128, 256 bytes) backed by specific memory pools. Falls back to standard heap allocation for massive objects.

### Concurrency & Threading
* **Lock-Free Asynchronous Logger:** File I/O is notoriously slow. The logger uses a dedicated background thread reading from a custom `std::atomic`-driven Ring Buffer. The main thread pushes log strings into the buffer in nanoseconds without ever locking a mutex.
* **`ThreadPool`:** A reusable worker pool utilizing `std::condition_variable` and `std::mutex` to queue and execute arbitrary `std::function` tasks concurrently across hardware threads.

### High-Performance Queries
* **`SpatialGrid`:** A 2D spatial partitioning grid used to accelerate neighborhood proximity queries (essential for Boids flocking and collision detection). Reduces $O(N^2)$ entity comparisons to $O(N)$ by only checking adjacent grid buckets.

### Networking Wrapper (ENet)
* Abstracted `Client` and `Server` classes wrapping the raw C-based `ENet` library (Reliable UDP).
* Manages proper memory zeroing, connection handshaking, reliable/unreliable packet broadcasting, and lambda-based event polling (`Poll()`) to easily bridge network traffic into the game loop.

### AI Foundations
* **Behavior Trees:** A memory-safe, composite Node system (`Selector`, `Sequence`) built dynamically using the `BinnedAllocator` to drive complex AI decision-making.
