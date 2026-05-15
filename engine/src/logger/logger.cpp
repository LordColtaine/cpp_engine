#include "logger/logger.h"
#include <cstring>
#include <iostream>

void Logger::Init(const std::string& filepath)
{
    if (m_IsRunning.load(std::memory_order_relaxed))
        return;

    m_File.open(filepath, std::ios::out | std::ios::trunc);
    if (!m_File.is_open())
    {
        std::cerr << "Failed to open log file: " << filepath << "\n";
        return;
    }

    m_IsRunning.store(true, std::memory_order_relaxed);
    m_WorkerThread = std::thread(&Logger::ProcessQueue, this);

    LOG_INFO("--- ENGINE STARTED (LOCK-FREE I/O) ---");
}

void Logger::Shutdown()
{
    if (!m_IsRunning.load(std::memory_order_relaxed))
        return;

    LOG_INFO("--- ENGINE SHUTTING DOWN ---");

    m_IsRunning.store(false, std::memory_order_relaxed);

    if (m_WorkerThread.joinable())
    {
        m_WorkerThread.join();
    }

    if (m_File.is_open())
    {
        m_File.close();
    }
}

void Logger::Log(LogLevel level, const std::string& message)
{
    if (!m_IsRunning.load(std::memory_order_relaxed))
        return;

    std::string prefix;
    switch (level)
    {
    case LogLevel::INFO:
        prefix = "[INFO] ";
        break;
    case LogLevel::WARN:
        prefix = "[WARN] ";
        break;
    case LogLevel::ERROR:
        prefix = "[ERROR] ";
        break;
    }

    const size_t currentWrite = m_WriteIndex.fetch_add(1, std::memory_order_relaxed);
    const size_t index = currentWrite & (RING_SIZE - 1);

    snprintf(m_RingBuffer[index].message, sizeof(LogEntry::message), "%s%s\n", prefix.c_str(), message.c_str());
    m_RingBuffer[index].isReady.store(true, std::memory_order_release);
}

void Logger::ProcessQueue()
{
    while (m_IsRunning.load(std::memory_order_relaxed) ||
           m_ReadIndex.load(std::memory_order_relaxed) < m_WriteIndex.load(std::memory_order_relaxed))
    {
        const size_t index = m_ReadIndex.load(std::memory_order_relaxed) & (RING_SIZE - 1);

        if (m_RingBuffer[index].isReady.load(std::memory_order_acquire))
        {
            m_File << m_RingBuffer[index].message;
            std::cout << m_RingBuffer[index].message;

            m_RingBuffer[index].isReady.store(false, std::memory_order_relaxed);
            m_ReadIndex.fetch_add(1, std::memory_order_relaxed);
        }
        else
        {
            std::this_thread::yield();
        }
    }

    m_File.flush();
}
