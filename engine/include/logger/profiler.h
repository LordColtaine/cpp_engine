#pragma once
#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>

// ==========================================
// TOGGLE THIS MACRO TO TURN PROFILING ON/OFF
// ==========================================
#define ENABLE_PROFILING 0

struct ProfileResult
{
    const char* Name;
    long long Start, End;
    uint32_t ThreadID;
};

class Profiler
{
public:
    static Profiler& Get()
    {
        static Profiler instance;
        return instance;
    }

#if ENABLE_PROFILING
    void BeginSession() { m_ProfileCount.store(0, std::memory_order_relaxed); }

    void WriteProfile(const ProfileResult& result)
    {
        size_t index = m_ProfileCount.fetch_add(1, std::memory_order_relaxed);
        if (index < MAX_PROFILES)
        {
            m_Buffer[index] = result;
        }
    }

    void EndSession()
    {
        std::ofstream file("profile.json");
        file << "{\"otherData\": {},\"traceEvents\":[";

        size_t count = std::min(static_cast<size_t>(m_ProfileCount.load()), MAX_PROFILES);
        for (size_t i = 0; i < count; ++i)
        {
            const auto& result = m_Buffer[i];
            if (i > 0)
                file << ",";

            // Chrome Tracing JSON Format
            file << "{";
            file << "\"cat\":\"function\",";
            file << "\"dur\":" << (result.End - result.Start) << ",";
            file << "\"name\":\"" << result.Name << "\",";
            file << "\"ph\":\"X\","; // Phase X means "Complete event"
            file << "\"pid\":0,";
            file << "\"tid\":" << result.ThreadID << ",";
            file << "\"ts\":" << result.Start;
            file << "}";
        }
        file << "]}";
        file.close();
    }
#else
    void BeginSession() {}
    void WriteProfile(const ProfileResult& result) {}
    void EndSession() {}
#endif
private:
    Profiler() = default;

#if ENABLE_PROFILING
    static constexpr size_t MAX_PROFILES = 1048576;
    std::array<ProfileResult, MAX_PROFILES> m_Buffer;
    alignas(64) std::atomic<size_t> m_ProfileCount{0};
#endif
};

class InstrumentationTimer
{
public:
    InstrumentationTimer(const char* name) : m_Name(name)
    {
        m_StartTimepoint =
            std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now());
    }

    ~InstrumentationTimer()
    {
        auto endTimepoint =
            std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now());

        long long start = m_StartTimepoint.time_since_epoch().count();
        long long end = endTimepoint.time_since_epoch().count();
        uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());

        Profiler::Get().WriteProfile({m_Name, start, end, threadID});
    }

private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::microseconds> m_StartTimepoint;
};

// --- THE MACROS ---
#if ENABLE_PROFILING
// __FUNCSIG__ gives the full function name, or fallback to __FUNCTION__
#if defined(_MSC_VER)
#define PROFILE_FUNCTION() InstrumentationTimer timer##__LINE__(__FUNCSIG__)
#else
#define PROFILE_FUNCTION() InstrumentationTimer timer##__LINE__(__PRETTY_FUNCTION__)
#endif

#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#else
#define PROFILE_FUNCTION()
#define PROFILE_SCOPE(name)
#endif
