#include "logger/logger.h"
#include "network/server.h"
#include "unitmanager.h"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>

std::atomic<bool> g_ServerRunning{true};

void HandleSignal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "\n[INFO] Shutdown signal received. Closing server gracefully...\n";
        g_ServerRunning = false;
    }
}

int main()
{
    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    Logger::Get().Init("server_log.txt");
    LOG_INFO("Starting Headless RTS Server...");

    Server server;
    if (!server.Start(55555, 2))
    {
        LOG_ERROR("Server failed to start. Shutting down.");
        Logger::Get().Shutdown();
        enet_deinitialize();
        return -1;
    }

    LOG_INFO("Server running. Waiting for 2 players to connect...");

    uint32_t currentTurn = 1;
    std::vector<Command> turnCommands;
    auto lastTurnTime = std::chrono::high_resolution_clock::now();
    const auto turnDuration = std::chrono::milliseconds(100);

    int readyPlayers = 0;
    bool gameStarted = false;
    std::vector<ENetPeer*> readyPeers;

    while (g_ServerRunning)
    {
        server.Poll(
            [&](const ServerEvent& event)
            {
                if (event.type == ServerEventType::ClientConnected)
                {
                    LOG_INFO("A client established connection. Waiting for READY signal...");
                }
                else if (event.type == ServerEventType::ClientDisconnected)
                {
                    LOG_INFO("A client disconnected.");
                }
                else if (event.type == ServerEventType::DataReceived)
                {
                    if (event.data.size() == sizeof(Command))
                    {
                        Command cmd;
                        std::memcpy(&cmd, event.data.data(), sizeof(Command));

                        if (!gameStarted && cmd.type == CommandType::CLIENT_READY)
                        {
                            readyPlayers++;
                            readyPeers.push_back(event.peer);
                            LOG_INFO("Player is READY! (" + std::to_string(readyPlayers) + "/2)");
                        }
                        else if (gameStarted)
                        {
                            turnCommands.push_back(cmd);
                        }
                    }
                }
            });

        if (!gameStarted && readyPlayers == 2)
        {
            LOG_INFO("All players fully loaded! Firing Start Gun...");

            Command startCmd0 = {};
            startCmd0.type = CommandType::START_GAME;
            startCmd0.playerID = 0;
            server.SendToPeer(readyPeers[0], &startCmd0, sizeof(Command));

            Command startCmd1 = {};
            startCmd1.type = CommandType::START_GAME;
            startCmd1.playerID = 1;
            server.SendToPeer(readyPeers[1], &startCmd1, sizeof(Command));

            gameStarted = true;
            lastTurnTime = std::chrono::high_resolution_clock::now();
        }

        if (gameStarted)
        {
            auto now = std::chrono::high_resolution_clock::now();
            if (now - lastTurnTime >= turnDuration)
            {
                for (auto& cmd : turnCommands)
                {
                    cmd.turnNumber = currentTurn;
                }

                if (turnCommands.empty())
                {
                    Command emptyCmd = {};
                    emptyCmd.type = CommandType::CLEAR_RALLY;
                    emptyCmd.playerID = -1;
                    emptyCmd.turnNumber = currentTurn;
                    server.Broadcast(&emptyCmd, sizeof(Command));
                }
                else
                {
                    server.Broadcast(turnCommands.data(), turnCommands.size() * sizeof(Command));
                }

                turnCommands.clear();
                currentTurn++;
                lastTurnTime = now;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    LOG_INFO("Server shut down successfully.");
    Logger::Get().Shutdown();

    enet_deinitialize();
    return 0;
}
