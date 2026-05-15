#pragma once
#include <cstdint>
#include <enet/enet.h>
#include <functional>
#include <vector>

enum class ServerEventType
{
    ClientConnected,
    ClientDisconnected,
    DataReceived
};

struct ServerEvent
{
    ServerEventType type;
    ENetPeer* peer;
    std::vector<uint8_t> data;
};

class Server
{
public:
    Server();
    ~Server();

    bool Start(uint16_t port, size_t maxClients);
    void Stop();
    void Poll(std::function<void(const ServerEvent&)> onEvent);
    void Broadcast(const void* data, size_t length);

    void SendToPeer(ENetPeer* peer, const void* data, size_t length);

private:
    ENetHost* m_Host = nullptr;
    static bool s_ENetInitialized;
};
