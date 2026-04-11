#pragma once
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

enum class LogLevel
{
    INFO,
    WARN,
    ERROR
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

    std::thread m_WorkerThread;
    std::queue<std::string> m_LogQueue;
    std::mutex m_QueueMutex;
    std::condition_variable m_ConditionVariable;

    std::ofstream m_File;
    bool m_IsRunning = false;
};

#define LOG_INFO(msg) Logger::Get().Log(LogLevel::INFO, msg)
#define LOG_WARN(msg) Logger::Get().Log(LogLevel::WARN, msg)
#define LOG_ERROR(msg) Logger::Get().Log(LogLevel::ERROR, msg)
