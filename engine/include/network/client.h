#pragma once
#include <cstdint>
#include <enet/enet.h>
#include <functional>
#include <string>
#include <vector>

enum class ClientEventType
{
    Connected,
    Disconnected,
    DataReceived
};

struct ClientEvent
{
    ClientEventType type;
    std::vector<uint8_t> data;
};

class Client
{
public:
    Client();
    ~Client();

    bool Connect(const std::string& hostAddress, uint16_t port);
    void Disconnect();

    void Poll(std::function<void(const ClientEvent&)> onEvent);
    void Send(const void* data, size_t length);

private:
    ENetHost* m_Host = nullptr;
    ENetPeer* m_Peer = nullptr;
    static bool s_ENetInitialized;
};
