#pragma once
#include "core/gameobject.h"
#include "raylib.h"

class Food : public GameObject
{
public:
    Food(float x, float y, float radius) : m_X(x), m_Y(y), m_Radius(radius) {}

    void Update(double dt) override
    {
        if (m_Radius <= 0.5f)
        {
            MarkForKill();
        }
    }

    void Draw() const override { DrawCircle(static_cast<int>(m_X), static_cast<int>(m_Y), m_Radius, GREEN); }

    size_t GetMemorySize() const override { return sizeof(*this); }
    float GetX() const override { return m_X; }
    float GetY() const override { return m_Y; }
    float GetRadius() const { return m_Radius; }
    size_t GetInstanceTypeID() const override { return GetTypeID<Food>(); }

    // Ants call this to take bites out of the food
    bool Harvest(float amount)
    {
        if (m_Radius > 0.0f)
        {
            m_Radius -= amount;
            return true;
        }
        return false;
    }

private:
    float m_X, m_Y;
    float m_Radius;
};
