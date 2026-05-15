#include "network/server.h"
#include "logger/logger.h"

bool Server::s_ENetInitialized = false;

Server::Server()
{
    if (!s_ENetInitialized)
    {
        if (enet_initialize() != 0)
        {
            LOG_ERROR("An error occurred while initializing ENet.");
        }

        s_ENetInitialized = true;
    }
}

Server::~Server() { Stop(); }

bool Server::Start(uint16_t port, size_t maxClients)
{

    ENetAddress address = {0};
    address.host = ENET_HOST_ANY;
    address.port = port;

    m_Host = enet_host_create(&address, maxClients, 2, 0, 0);
    if (m_Host == nullptr)
    {
        LOG_ERROR("An error occurred while trying to create an ENet server host.");
        return false;
    }

    LOG_INFO("Server started on port " + std::to_string(port));
    return true;
}

void Server::Stop()
{
    if (m_Host)
    {
        enet_host_destroy(m_Host);
        m_Host = nullptr;
        LOG_INFO("Server stopped.");
    }
}

void Server::Poll(std::function<void(const ServerEvent&)> onEvent)
{
    if (!m_Host)
        return;

    ENetEvent event;
    while (enet_host_service(m_Host, &event, 0) > 0)
    {
        ServerEvent sEvent;
        sEvent.peer = event.peer;

        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            sEvent.type = ServerEventType::ClientConnected;
            onEvent(sEvent);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            sEvent.type = ServerEventType::DataReceived;
            sEvent.data.assign(event.packet->data, event.packet->data + event.packet->dataLength);
            onEvent(sEvent);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            sEvent.type = ServerEventType::ClientDisconnected;
            onEvent(sEvent);
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

void Server::Broadcast(const void* data, size_t length)
{
    if (!m_Host)
        return;

    ENetPacket* packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(m_Host, 0, packet);
}

void Server::SendToPeer(ENetPeer* peer, const void* data, size_t length)
{
    if (!peer)
        return;

    ENetPacket* packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}
