#include "logger/logger.h"
#include <iostream>

void Logger::Init(const std::string& filepath)
{
    if (m_IsRunning)
        return;

    m_File.open(filepath, std::ios::out | std::ios::app);
    if (!m_File.is_open())
    {
        std::cerr << "Failed to open log file: " << filepath << "\n";
        return;
    }

    m_IsRunning = true;
    m_WorkerThread = std::thread(&Logger::ProcessQueue, this);

    LOG_INFO("--- ENGINE STARTED ---");
}

void Logger::Shutdown()
{
    if (!m_IsRunning)
        return;

    LOG_INFO("--- ENGINE SHUTTING DOWN ---");

    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_IsRunning = false;
    }

    m_ConditionVariable.notify_one(); // Wake the thread up one last time

    if (m_WorkerThread.joinable())
    {
        m_WorkerThread.join(); // Wait for it to finish writing remaining logs
    }

    if (m_File.is_open())
    {
        m_File.close();
    }
}

void Logger::Log(LogLevel level, const std::string& message)
{
    if (!m_IsRunning)
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

    const std::string formattedMessage = prefix + message + "\n";

    // Lock the queue, push the message, unlock, and tell the background thread to wake up
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_LogQueue.push(formattedMessage);
    }
    m_ConditionVariable.notify_one();
}

void Logger::ProcessQueue()
{
    while (true)
    {
        std::string currentMessage;

        {
            // The thread goes to sleep here until notified.
            // It uses practically 0% CPU while sleeping.
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_ConditionVariable.wait(lock, [this] { return !m_LogQueue.empty() || !m_IsRunning; });

            if (!m_IsRunning && m_LogQueue.empty())
            {
                break; // Exit the thread completely
            }

            currentMessage = m_LogQueue.front();
            m_LogQueue.pop();
        } // The mutex unlocks here so the game loop can keep pushing

        // Write to the physical disk AFTER dropping the lock
        if (m_File.is_open())
        {
            m_File << currentMessage;
            // Note: In a production engine, you wouldn't flush() every single line
            // as it's slow, but for debugging crashes, it ensures the last line is saved.
            m_File.flush();
        }
    }
}
