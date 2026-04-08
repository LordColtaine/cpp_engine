#include "ant.h"
#include "core/world.h"
#include "pheromonegrid.h"
#include "raylib.h"
#include <algorithm>
#include <iostream>

// ==========================================
// ENGINE CONFIGURATION
// ==========================================
namespace
{
    // Timing & Framerate
    constexpr int TARGET_FPS = 144;
    constexpr double TICK_RATE = 60.0;
    constexpr double FIXED_DT = 1.0 / TICK_RATE;
    constexpr double MAX_FRAME_TIME = 0.25;

    // World Dimensions
    constexpr float WORLD_WIDTH = 5000.0f;
    constexpr float WORLD_HEIGHT = 5000.0f;
    constexpr int GRID_CELL_SIZE = 4;

    // Spawning & Entities
    constexpr int TOTAL_ANTS_TO_SPAWN = 10000;
    constexpr int ANTS_SPAWN_PER_FRAME = 500;
    constexpr int SOLDIER_SPAWN_CHANCE = 10; // 10% chance
    constexpr float NEST_RADIUS = 30.0f;

    // Player Interaction
    constexpr float CAMERA_ZOOM_SPEED = 0.05f;
    constexpr float CAMERA_MIN_ZOOM = 0.1f;
    constexpr float FOOD_SPAWN_RADIUS = 20.0f;
    constexpr float OBSTACLE_SPAWN_RADIUS = 15.0f;
    constexpr float RALLY_SPAWN_RADIUS = 5.0f;
    constexpr float RALLY_INTENSITY = 5000.0f;

    // Multithreading
    constexpr int THREAD_JOB_COUNT = 4;

    // UI Loading Screen
    constexpr int UI_BAR_WIDTH = 400;
    constexpr int UI_BAR_HEIGHT = 20;
    constexpr int UI_TITLE_OFFSET_Y = 60;
    constexpr int UI_TITLE_FONT_SIZE = 20;
} // namespace

enum class GameState
{
    Loading,
    Playing
};

int main()
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "Ant Swarm Simulation");
    SetTargetFPS(TARGET_FPS);

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    const float monitorCenterX = static_cast<float>(screenWidth) / 2.0f;
    const float monitorCenterY = static_cast<float>(screenHeight) / 2.0f;

    const float worldCenterX = WORLD_WIDTH / 2.0f;
    const float worldCenterY = WORLD_HEIGHT / 2.0f;

    World* world = new World();
    world->Init();

    PheromoneGrid grid(WORLD_WIDTH, WORLD_HEIGHT, GRID_CELL_SIZE);
    grid.SetNest(worldCenterX, worldCenterY, NEST_RADIUS);

    // --- Initialize the Camera ---
    Camera2D camera = {0};
    camera.target = {worldCenterX, worldCenterY};
    camera.offset = {monitorCenterX, monitorCenterY};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    GameState currentState = GameState::Loading;
    int antsSpawned = 0;

    double previousTime = GetTime();
    double accumulator = 0.0;

    while (!WindowShouldClose())
    {
        // ==========================================
        // STATE 1: LOADING
        // ==========================================
        if (currentState == GameState::Loading)
        {
            const int batchSize = std::min(ANTS_SPAWN_PER_FRAME, TOTAL_ANTS_TO_SPAWN - antsSpawned);
            for (int i = 0; i < batchSize; i++)
            {
                if (rand() % 100 < SOLDIER_SPAWN_CHANCE)
                {
                    world->NewGameObject<SoldierAnt>(worldCenterX, worldCenterY, &grid);
                }
                else
                {
                    world->NewGameObject<WorkerAnt>(worldCenterX, worldCenterY, &grid);
                }
            }
            antsSpawned += batchSize;

            if (antsSpawned >= TOTAL_ANTS_TO_SPAWN)
            {
                currentState = GameState::Playing;
                previousTime = GetTime();
                accumulator = 0.0;
            }

            BeginDrawing();
            ClearBackground(DARKGRAY);

            const char* titleText = "SPAWNING THE SWARM...";
            DrawText(titleText, static_cast<int>(monitorCenterX) - (MeasureText(titleText, UI_TITLE_FONT_SIZE) / 2),
                     static_cast<int>(monitorCenterY) - UI_TITLE_OFFSET_Y, UI_TITLE_FONT_SIZE, RAYWHITE);

            const float progress = static_cast<float>(antsSpawned) / TOTAL_ANTS_TO_SPAWN;
            DrawRectangle(static_cast<int>(monitorCenterX) - (UI_BAR_WIDTH / 2),
                          static_cast<int>(monitorCenterY) - (UI_BAR_HEIGHT / 2), UI_BAR_WIDTH, UI_BAR_HEIGHT, BLACK);
            DrawRectangle(static_cast<int>(monitorCenterX) - (UI_BAR_WIDTH / 2),
                          static_cast<int>(monitorCenterY) - (UI_BAR_HEIGHT / 2),
                          static_cast<int>(UI_BAR_WIDTH * progress), UI_BAR_HEIGHT, LIME);
            EndDrawing();
        }

        // ==========================================
        // STATE 2: PLAYING
        // ==========================================
        else if (currentState == GameState::Playing)
        {
            const double currentTime = GetTime();
            double frameTime = currentTime - previousTime;
            previousTime = currentTime;

            // Cap the frame time to prevent the "Spiral of Death" during lag spikes
            if (frameTime > MAX_FRAME_TIME)
            {
                frameTime = MAX_FRAME_TIME;
            }
            accumulator += frameTime;

            // --- CAMERA CONTROLS ---
            camera.zoom += (static_cast<float>(GetMouseWheelMove()) * CAMERA_ZOOM_SPEED);
            if (camera.zoom < CAMERA_MIN_ZOOM)
            {
                camera.zoom = CAMERA_MIN_ZOOM;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
            {
                Vector2 delta = GetMouseDelta();
                camera.target.x -= delta.x / camera.zoom;
                camera.target.y -= delta.y / camera.zoom;
            }

            // --- PLAYER INPUT ---
            const Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                grid.SpawnFood(mouseWorldPos.x, mouseWorldPos.y, FOOD_SPAWN_RADIUS);
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                grid.SpawnObstacle(mouseWorldPos.x, mouseWorldPos.y, OBSTACLE_SPAWN_RADIUS);
            }

            if (IsKeyDown(KEY_SPACE))
            {
                grid.AddRallyPheromone(mouseWorldPos.x, mouseWorldPos.y, RALLY_SPAWN_RADIUS,
                                       RALLY_INTENSITY * static_cast<float>(FIXED_DT));
            }

            // --- FIXED TIMESTEP UPDATE ---
            while (accumulator >= FIXED_DT)
            {
                grid.PrepareUpdate();

                const int rowsPerJob = grid.GetHeight() / THREAD_JOB_COUNT;
                for (int i = 0; i < THREAD_JOB_COUNT; ++i)
                {
                    const int startY = i * rowsPerJob;
                    const int endY = (i == THREAD_JOB_COUNT - 1) ? grid.GetHeight() : startY + rowsPerJob;

                    world->GetThreadPool().QueueJob([&grid, startY, endY]()
                                                    { grid.UpdateChunk(FIXED_DT, startY, endY); });
                }

                world->GetThreadPool().WaitAll();

                world->Update(FIXED_DT);
                accumulator -= FIXED_DT;
            }

            // --- RENDERING ---
            BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(camera);
            grid.DrawDebug();
            world->Draw();
            EndMode2D();

            DrawFPS(10, 10);

            EndDrawing();
        }
    }

    // --- SHUTDOWN ---
    grid.Cleanup();
    CloseWindow();
    delete world;

    return 0;
}
