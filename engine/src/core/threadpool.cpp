#include "core/threadpool.h"

ThreadPool::ThreadPool() {}

ThreadPool::~ThreadPool() { Stop(); }

void ThreadPool::Start(uint32_t numThreads)
{
    m_Threads.reserve(numThreads);
    for (uint32_t i = 0; i < numThreads; ++i)
    {
        m_Threads.emplace_back(&ThreadPool::ThreadLoop, this);
    }
}

void ThreadPool::ThreadLoop()
{
    while (true)
    {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_MutexCondition.wait(lock, [this] { return !m_Jobs.empty() || m_ShouldTerminate; });

            if (m_ShouldTerminate && m_Jobs.empty())
            {
                return;
            }

            job = m_Jobs.front();
            m_Jobs.pop();
        }

        job();

        {
            std::unique_lock<std::mutex> lock(m_WaitMutex);
            m_ActiveJobs--;

            if (m_ActiveJobs == 0 && m_Jobs.empty())
            {
                m_WaitCondition.notify_all();
            }
        }
    }
}

void ThreadPool::QueueJob(const std::function<void()>& job)
{
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_Jobs.push(job);
    }

    {
        std::unique_lock<std::mutex> lock(m_WaitMutex);
        m_ActiveJobs++;
    }

    m_MutexCondition.notify_one();
}

void ThreadPool::WaitAll()
{
    std::unique_lock<std::mutex> lock(m_WaitMutex);

    m_WaitCondition.wait(lock, [this] { return m_ActiveJobs == 0 && m_Jobs.empty(); });
}

void ThreadPool::Stop()
{
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_ShouldTerminate = true;
    }

    m_MutexCondition.notify_all();

    for (std::thread& active_thread : m_Threads)
    {
        if (active_thread.joinable())
        {
            active_thread.join();
        }
    }

    m_Threads.clear();
}
