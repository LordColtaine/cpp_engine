#include "network/client.h"
#include "logger/logger.h"

bool Client::s_ENetInitialized = false;

Client::Client()
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

Client::~Client() { Disconnect(); }

bool Client::Connect(const std::string& hostAddress, uint16_t port)
{
    m_Host = enet_host_create(nullptr, 1, 2, 0, 0);
    if (m_Host == nullptr)
    {
        LOG_ERROR("An error occurred while trying to create an ENet client host.");
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, hostAddress.c_str());
    address.port = port;

    m_Peer = enet_host_connect(m_Host, &address, 2, 0);
    if (m_Peer == nullptr)
    {
        LOG_ERROR("No available peers for initiating an ENet connection.");
        return false;
    }

    LOG_INFO("Connecting to " + hostAddress + ":" + std::to_string(port) + "...");
    return true;
}

void Client::Disconnect()
{
    if (m_Peer)
    {
        enet_peer_disconnect(m_Peer, 0);

        // Wait up to 3 seconds for the disconnect to succeed
        ENetEvent event;
        while (enet_host_service(m_Host, &event, 3000) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_RECEIVE)
            {
                enet_packet_destroy(event.packet);
            }
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                LOG_INFO("Disconnection successful.");
                m_Peer = nullptr;
                break;
            }
        }
    }

    if (m_Host)
    {
        enet_host_destroy(m_Host);
        m_Host = nullptr;
    }
}

void Client::Poll(std::function<void(const ClientEvent&)> onEvent)
{
    if (!m_Host)
        return;

    ENetEvent event;
    while (enet_host_service(m_Host, &event, 0) > 0)
    {
        ClientEvent cEvent;

        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            cEvent.type = ClientEventType::Connected;
            onEvent(cEvent);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            cEvent.type = ClientEventType::DataReceived;
            cEvent.data.assign(event.packet->data, event.packet->data + event.packet->dataLength);
            onEvent(cEvent);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            cEvent.type = ClientEventType::Disconnected;
            m_Peer = nullptr; // Clear the peer on disconnect
            onEvent(cEvent);
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

void Client::Send(const void* data, size_t length)
{
    if (!m_Peer)
        return;

    ENetPacket* packet = enet_packet_create(data, length, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(m_Peer, 0, packet);
}
