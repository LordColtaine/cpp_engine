#include "core/world.h"
#include "logger/logger.h"
#include "logger/profiler.h"
#include "network/client.h"
#include "raylib.h"
#include "unitmanager.h"
#include <cstring>
#include <unordered_map>

int main()
{
    Logger::Get().Init("rts_log.txt");

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Swarm Commander: RTS Edition");

    const int monitor = GetCurrentMonitor();
    const int screenWidth = GetMonitorWidth(monitor);
    const int screenHeight = GetMonitorHeight(monitor);

    SetWindowSize(screenWidth, screenHeight);
    ToggleFullscreen();

    SetTargetFPS(144);

    Camera2D camera = {0};
    camera.target = {UnitManager::MAP_WIDTH / 2.0f, UnitManager::MAP_HEIGHT / 2.0f};
    camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    World* world = new World();
    world->Init(UnitManager::MAP_WIDTH, UnitManager::MAP_HEIGHT, UnitManager::GRID_CELL_SIZE);

    UnitManager unitManager(world);
    unitManager.SetRandomSeed(1337);
    Client networkClient;
    networkClient.Connect("127.0.0.1", 55555);

    const float centerX = UnitManager::MAP_WIDTH / 2.0f;
    const float centerY = UnitManager::MAP_HEIGHT / 2.0f;

    // player bases.
    unitManager.SpawnBase(500, centerY, 100.0f, 0, 100.0f);
    unitManager.SpawnBase(UnitManager::MAP_WIDTH - 500, centerY, 100.0f, 1, 100.0f);

    // Neutral bases.
    unitManager.SpawnBase(centerX, centerY, 80.0f, -1, 50.0f);
    unitManager.SpawnBase(centerX, centerY - 800, 60.0f, -1, 25.0f);
    unitManager.SpawnBase(centerX, centerY + 800, 60.0f, -1, 25.0f);

    // Draw some obstacles
    unitManager.SpawnObstacle(centerX - 400, centerY - 400, 150.0f);
    unitManager.SpawnObstacle(centerX + 400, centerY - 400, 150.0f);
    unitManager.SpawnObstacle(centerX - 400, centerY + 400, 150.0f);
    unitManager.SpawnObstacle(centerX + 400, centerY + 400, 150.0f);

    unitManager.SpawnObstacle(1000, centerY, 200.0f);
    unitManager.SpawnObstacle(UnitManager::MAP_WIDTH - 1000, centerY, 200.0f);

    // Start with a small initial swarm
    for (int i = 0; i < 50; ++i)
    {
        unitManager.SpawnUnit(650, centerY + unitManager.GetRandomInt(-50, 50), 0);
        unitManager.SpawnUnit(UnitManager::MAP_WIDTH - 650, centerY + unitManager.GetRandomInt(-50, 50), 1);
    }

    Vector2 dragStartWorld = {0, 0};
    bool isDragging = false;

    const float FIXED_DT = 1.0f / 60.0f;
    double accumulator = 0.0;

    bool isGameActive = false;
    uint32_t currentSimulatedTurn = 1;
    uint32_t latestReceivedTurn = 0;
    int physicsFramesThisTurn = 0;
    std::unordered_map<uint32_t, std::vector<Command>> turnBuffer;

    int myPlayerID = -1;

    while (!WindowShouldClose())
    {
        double frameTime = GetFrameTime();
        if (frameTime > 0.25)
            frameTime = 0.25;
        accumulator += frameTime;

        networkClient.Poll(
            [&](const ClientEvent& event)
            {
                if (event.type == ClientEventType::Connected)
                {
                    LOG_INFO("Connected to Server! Sending Ready signal...");
                    Command readyCmd = {};
                    readyCmd.type = CommandType::CLIENT_READY;
                    networkClient.Send(&readyCmd, sizeof(Command));
                }
                else if (event.type == ClientEventType::DataReceived)
                {
                    if (event.data.size() % sizeof(Command) == 0)
                    {
                        const size_t numCommands = event.data.size() / sizeof(Command);
                        const Command* cmds = reinterpret_cast<const Command*>(event.data.data());

                        for (size_t i = 0; i < numCommands; ++i)
                        {
                            Command receivedCmd = cmds[i];

                            if (receivedCmd.type == CommandType::START_GAME)
                            {
                                myPlayerID = receivedCmd.playerID;
                                LOG_INFO("Start Gun! We are Player " + std::to_string(myPlayerID));
                                isGameActive = true;
                            }
                            else
                            {
                                if (receivedCmd.turnNumber > latestReceivedTurn)
                                {
                                    latestReceivedTurn = receivedCmd.turnNumber;
                                }
                                if (receivedCmd.playerID != -1)
                                {
                                    turnBuffer[receivedCmd.turnNumber].push_back(receivedCmd);
                                }
                            }
                        }
                    }
                }
            });

        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }

        const float panSpeed = 1000.0f / camera.zoom;
        if (IsKeyDown(KEY_W))
            camera.target.y -= panSpeed * frameTime;
        if (IsKeyDown(KEY_S))
            camera.target.y += panSpeed * frameTime;
        if (IsKeyDown(KEY_A))
            camera.target.x -= panSpeed * frameTime;
        if (IsKeyDown(KEY_D))
            camera.target.x += panSpeed * frameTime;

        camera.zoom += GetMouseWheelMove() * 0.1f;
        if (camera.zoom < 0.1f)
            camera.zoom = 0.1f;
        if (camera.zoom > 3.0f)
            camera.zoom = 3.0f;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isDragging = true;
            dragStartWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            isDragging = false;
            Vector2 dragEndWorld = GetScreenToWorld2D(GetMousePosition(), camera);

            const float minX = std::min(dragStartWorld.x, dragEndWorld.x);
            const float minY = std::min(dragStartWorld.y, dragEndWorld.y);
            const float width = std::abs(dragEndWorld.x - dragStartWorld.x);
            const float height = std::abs(dragEndWorld.y - dragStartWorld.y);

            if (myPlayerID != -1)
            {
                Command cmd = {};
                cmd.type = CommandType::SELECT_BOX;
                cmd.playerID = myPlayerID;
                cmd.x = minX;
                cmd.y = minY;
                cmd.width = width;
                cmd.height = height;
                networkClient.Send(&cmd, sizeof(Command));
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && myPlayerID != -1)
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            Command cmd = {};
            cmd.type = CommandType::SET_RALLY;
            cmd.playerID = myPlayerID;
            cmd.x = mouseWorldPos.x;
            cmd.y = mouseWorldPos.y;
            networkClient.Send(&cmd, sizeof(Command));
        }

        while (accumulator >= FIXED_DT || (isGameActive && currentSimulatedTurn < latestReceivedTurn))
        {
            if (isGameActive)
            {
                if (currentSimulatedTurn > latestReceivedTurn)
                    break;

                if (physicsFramesThisTurn == 0)
                {
                    auto it = turnBuffer.find(currentSimulatedTurn);
                    if (it != turnBuffer.end())
                    {
                        for (const auto& cmd : it->second)
                        {
                            unitManager.QueueCommand(cmd);
                        }
                        turnBuffer.erase(it);
                    }
                }

                unitManager.Update(FIXED_DT);
                world->Update(FIXED_DT);

                physicsFramesThisTurn++;
                if (physicsFramesThisTurn >= 6)
                {
                    currentSimulatedTurn++;
                    physicsFramesThisTurn = 0;
                }
            }

            // This prevents negative time desyncs when the network is fast-forwarding.
            if (accumulator >= FIXED_DT)
            {
                accumulator -= FIXED_DT;
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);
        Rectangle mapBounds = {0, 0, UnitManager::MAP_WIDTH, UnitManager::MAP_HEIGHT};
        DrawRectangleLinesEx(mapBounds, 20.0f, RED);

        unitManager.DrawEnvironment();
        world->Draw();

        // Draw box
        if (isDragging)
        {
            Vector2 currentMouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
            Rectangle visualBox = {
                std::min(dragStartWorld.x, currentMouseWorld.x), std::min(dragStartWorld.y, currentMouseWorld.y),
                std::abs(currentMouseWorld.x - dragStartWorld.x), std::abs(currentMouseWorld.y - dragStartWorld.y)};
            DrawRectangleRec(visualBox, Fade(GREEN, 0.2f));
            DrawRectangleLinesEx(visualBox, 2.0f, GREEN);
        }

        // Draw Rally Point
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            DrawCircleLines(mouseWorldPos.x, mouseWorldPos.y, 25.0f, ORANGE);
            DrawCircle(mouseWorldPos.x, mouseWorldPos.y, 5.0f, RED);
        }

        EndMode2D();

        DrawFPS(10, 10);
        DrawText("WASD: Pan | Scroll: Zoom | F11: Toggle Fullscreen", 10, 40, 20, LIGHTGRAY);

        EndDrawing();
    }

    delete world;
    CloseWindow();
    Logger::Get().Shutdown();

    return 0;
}
