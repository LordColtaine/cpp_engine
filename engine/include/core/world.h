#pragma once
#include "core/gameobject.h"
#include "core/threadpool.h"
#include "memory/binnedallocator.h"
#include <vector>

class SpatialGrid;

class World
{
public:
    World();
    ~World();
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    void Init();

    template <typename T, typename... Args> T* NewGameObject(Args&&... args)
    {
        void* memory = m_MemoryManager.Allocate(sizeof(T));
        if (memory == nullptr)
        {
            return nullptr;
        }

        T* object = new (memory) T(std::forward<Args>(args)...);
        object->SetWorld(this);
        m_Objects.push_back(object);
        return object;
    }

    void Update(const double dt);
    void Draw() const;

    ThreadPool& GetThreadPool() { return m_ThreadPool; }

    SpatialGrid* GetSpatialGrid() const { return m_SpatialGrid; }

private:
    BinnedAllocator m_MemoryManager;
    std::vector<GameObject*> m_Objects;
    ThreadPool m_ThreadPool;
    SpatialGrid* m_SpatialGrid = nullptr;
};
