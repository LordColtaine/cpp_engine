#include "object/ant.h"
#include "raylib.h"
#include "world/world.h"
#include <algorithm> // For std::min
#include <iostream>

// --- ENGINE CONSTANTS ---
const double TICK_RATE = 60.0;
const double FIXED_DT = 1.0 / TICK_RATE;

// --- GAME STATES ---
enum class GameState { Loading, Playing };

int main() {
    // 1. Tell Raylib to launch in Borderless Fullscreen mode
    SetConfigFlags(FLAG_FULLSCREEN_MODE);

    // 2. Passing 0, 0 tells Raylib to auto-detect the monitor's native resolution
    InitWindow(0, 0, "Custom C++ RTS Engine");
    SetTargetFPS(144);

    // 3. Grab the actual dimensions that Raylib detected
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Calculate the mathematical center of your monitor
    float centerX = static_cast<float>(screenWidth) / 2.0f;
    float centerY = static_cast<float>(screenHeight) / 2.0f;

    // Initialize our World with the new dimensions
    World *world = new World();
    world->Init(screenWidth, screenHeight);

    // Set the Nest in the exact center of your screen
    world->GetGrid()->SetNest(centerX, centerY, 30.0f);

    // --- STATE & LOADING VARIABLES ---
    GameState currentState = GameState::Loading;
    const int totalAntsToSpawn = 1000; // Up to you!
    int antsSpawned = 0;
    const int spawnPerFrame = 50;

    double previousTime = GetTime();
    double accumulator = 0.0;

    while (!WindowShouldClose()) {
        if (currentState == GameState::Loading) {

            int batchSize = std::min(spawnPerFrame, totalAntsToSpawn - antsSpawned);
            for (int i = 0; i < batchSize; i++) {
                // Spawn ants at the new dynamic center!
                world->NewGameObject<Ant>(centerX, centerY, world->GetGrid());
            }
            antsSpawned += batchSize;

            // 2. Check if we are done loading
            if (antsSpawned >= totalAntsToSpawn) {
                currentState = GameState::Playing;

                // CRITICAL FIX: Reset the clock!
                // This prevents the fixed timestep from trying to "catch up"
                // on all the time spent rendering the loading screen.
                previousTime = GetTime();
                accumulator = 0.0;
            }

            // 3. Draw the Loading Screen
            BeginDrawing();
            ClearBackground(DARKGRAY);

            // Grab the exact center of the screen
            int cx = GetScreenWidth() / 2;
            int cy = GetScreenHeight() / 2;

            // --- PERFECTLY CENTERED TITLE ---
            const char *titleText = "SPAWNING THE SWARM...";
            int titleWidth = MeasureText(titleText, 20);
            DrawText(titleText, cx - (titleWidth / 2), cy - 60, 20, RAYWHITE);

            // --- CENTERED LOADING BAR ---
            int barWidth = 400;
            int barHeight = 20;
            int barX = cx - (barWidth / 2);
            int barY = cy - (barHeight / 2);

            float progress = static_cast<float>(antsSpawned) / totalAntsToSpawn;

            DrawRectangle(barX, barY, barWidth, barHeight, BLACK);
            DrawRectangle(barX, barY, static_cast<int>(barWidth * progress), barHeight, LIME);

            // --- PERFECTLY CENTERED PROGRESS NUMBERS ---
            const char *progressText = TextFormat("%d / %d", antsSpawned, totalAntsToSpawn);
            int progressWidth = MeasureText(progressText, 20);
            DrawText(progressText, cx - (progressWidth / 2), barY + 35, 20, LIGHTGRAY);

            EndDrawing();
        }
        // ==========================================
        // STATE 2: PLAYING
        // ==========================================
        else if (currentState == GameState::Playing) {

            // --- TIME MATH ---
            double currentTime = GetTime();
            double frameTime = currentTime - previousTime;
            previousTime = currentTime;

            if (frameTime > 0.25) {
                frameTime = 0.25;
            }
            accumulator += frameTime;

            // --- INPUT HANDLING ---
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                float mouseX = static_cast<float>(GetMouseX());
                float mouseY = static_cast<float>(GetMouseY());

                // Spawn a green food circle with a radius of 20 pixels
                world->GetGrid()->SpawnFood(mouseX, mouseY, 20.0f);

                std::cout << "Food dropped at: " << mouseX << ", " << mouseY << std::endl;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                float mouseX = static_cast<float>(GetMouseX());
                float mouseY = static_cast<float>(GetMouseY());

                // Draw a wall with a 15-pixel radius brush
                world->GetGrid()->SpawnObstacle(mouseX, mouseY, 15.0f);
            }

            // --- GAME LOGIC (Fixed Timestep) ---
            while (accumulator >= FIXED_DT) {
                world->Update(FIXED_DT);
                accumulator -= FIXED_DT;
            }

            // --- RENDERING ---
            BeginDrawing();
            ClearBackground(RAYWHITE);

            // Draw everything
            world->Draw();

            DrawFPS(10, 10);
            EndDrawing();
        }
    }

    // --- CLEANUP ---
    CloseWindow();
    delete world;
    return 0;
}
