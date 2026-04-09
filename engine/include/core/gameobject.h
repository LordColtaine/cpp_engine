#pragma once
#include "core/typeid.h"
#include <cstddef>

class World;
class GameObject
{
public:
    bool IsPendingKill() const { return m_IsPendingKill; }
    void MarkForKill() { m_IsPendingKill = true; }
    void SetWorld(World* world) { m_World = world; }
    World* GetWorld() const { return m_World; }

    virtual ~GameObject() = default;

    template <typename T> bool IsA() const { return GetInstanceTypeID() == GetTypeID<T>(); }

    virtual size_t GetInstanceTypeID() const = 0;
    virtual void Update(double dt) = 0;
    virtual void Draw() const = 0;
    virtual size_t GetMemorySize() const = 0;
    virtual float GetX() const = 0;
    virtual float GetY() const = 0;

private:
    bool m_IsPendingKill = false;
    World* m_World = nullptr;
};
