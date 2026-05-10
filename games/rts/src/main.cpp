#include "core/world.h"
#include "logger/logger.h"
#include "logger/profiler.h"
#include "raylib.h"

int main()
{
    // 1. Engine Initialization [cite: 250, 447-448]
    Logger::Get().Init("rts_log.txt");

    // Config for RTS-style window [cite: 251]
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Swarm Commander: RTS Edition");
    SetTargetFPS(144);

    World* world = new World();
    world->Init(); // Sets up SpatialGrid and Memory [cite: 418-419]

    // 2. Main Loop [cite: 264]
    while (!WindowShouldClose())
    {
        // --- Update Phase ---
        double dt = GetFrameTime();
        world->Update(dt); // [cite: 420-423]

        // --- Render Phase ---
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawFPS(10, 10);
        DrawText("RTS ENGINE ACTIVE", 10, 40, 20, LIME);

        EndDrawing();
    }

    // 3. Cleanup [cite: 298]
    delete world;
    CloseWindow();
    Logger::Get().Shutdown();

    return 0;
}
