#pragma once
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    void Start(uint32_t numThreads);
    void QueueJob(const std::function<void()>& job);
    void WaitAll();
    void Stop();

private:
    void ThreadLoop();

    bool m_ShouldTerminate = false;
    int m_ActiveJobs = 0;

    std::mutex m_QueueMutex;
    std::condition_variable m_MutexCondition;

    std::mutex m_WaitMutex;
    std::condition_variable m_WaitCondition;

    std::vector<std::thread> m_Threads;
    std::queue<std::function<void()>> m_Jobs;
};
