#include "core/world.h"

#include "core/spatialgrid.h"
#include "logger/profiler.h"

World::World()
{
    unsigned int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
    {
        numThreads = 4;
    }

    m_ThreadPool.Start(numThreads);
}

World::~World()
{
    m_ThreadPool.Stop();
    delete m_SpatialGrid;
}

void World::Init()
{
    m_SpatialGrid = new SpatialGrid(5000.0f, 5000.0f, 50.0f);
    m_MemoryManager.Init();
    m_Objects.reserve(200000);
}

void World::Update(const double dt)
{
    PROFILE_FUNCTION();

    m_SpatialGrid->Clear();
    for (GameObject* obj : m_Objects)
    {
        if (obj->HasSpatialCollision())
        {
            m_SpatialGrid->Insert(obj, obj->GetX(), obj->GetY(), obj->GetLayer());
        }
    }

    for (GameObject* obj : m_Objects)
    {
        obj->Update(dt);
    }

    for (int i = static_cast<int>(m_Objects.size()) - 1; i >= 0; --i)
    {
        if (m_Objects[i]->IsPendingKill())
        {
            GameObject* obj = m_Objects[i];
            const size_t size = obj->GetMemorySize();
            obj->Uninitialize();
            obj->~GameObject();
            m_MemoryManager.Free(obj, size);
            m_Objects[i] = m_Objects.back();
            m_Objects.pop_back();
        }
    }
}

void World::Draw() const
{
    for (const GameObject* obj : m_Objects)
    {
        obj->Draw();
    }
}
