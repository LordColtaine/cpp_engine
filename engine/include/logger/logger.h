#pragma once
#include <array>
#include <atomic>
#include <fstream>
#include <string>
#include <thread>

enum class LogLevel
{
    INFO,
    WARN,
    ERROR
};

struct LogEntry
{
    std::atomic<bool> isReady{false};
    char message[256];
};

class Logger
{
public:
    static Logger& Get()
    {
        static Logger instance;
        return instance;
    }

    void Init(const std::string& filepath);
    void Shutdown();
    void Log(LogLevel level, const std::string& message);

private:
    Logger() = default;
    ~Logger() { Shutdown(); }

    void ProcessQueue();

    static constexpr size_t RING_SIZE = 1024; // power of 2 for replacing the % operator.
    std::array<LogEntry, RING_SIZE> m_RingBuffer;

    // Atomic indices for lock-free tracking
    alignas(64) std::atomic<size_t> m_WriteIndex{0};
    alignas(64) std::atomic<size_t> m_ReadIndex{0};

    std::thread m_WorkerThread;
    std::ofstream m_File;
    std::atomic<bool> m_IsRunning{false};
};

#define LOG_INFO(msg) Logger::Get().Log(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::Get().Log(LogLevel::WARN, msg)
#define LOG_ERROR(msg) Logger::Get().Log(LogLevel::ERROR, msg)
