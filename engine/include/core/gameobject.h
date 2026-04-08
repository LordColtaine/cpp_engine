#pragma once
#include <cstddef>

class GameObject
{
public:
    bool IsPendingKill() const { return m_IsPendingKill; }

    virtual ~GameObject() = default;

    virtual void Update(double dt) = 0;
    virtual void Draw() const = 0;
    virtual size_t GetMemorySize() const = 0;
    virtual float GetX() const = 0;
    virtual float GetY() const = 0;

private:
    bool m_IsPendingKill = false;
};
