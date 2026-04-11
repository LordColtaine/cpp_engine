#pragma once
#include "core/gameobject.h"
#include "raylib.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class Food : public GameObject
{
public:
    Food(float x, float y, float radius) : m_X(x), m_Y(y), m_Radius(radius)
    {
        m_FoodAmount = M_PI * (m_Radius * m_Radius);
    }

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

    float Harvest(float requestedAmount)
    {
        if (m_FoodAmount <= 0.0f)
            return 0.0f;

        const float actualBite = std::min(requestedAmount, m_FoodAmount);

        m_FoodAmount -= actualBite;

        if (m_FoodAmount > 0.0f)
        {
            m_Radius = std::sqrt(m_FoodAmount / M_PI);
        }
        else
        {
            m_Radius = 0.0f;
            m_FoodAmount = 0.0f;
            MarkForKill();
        }

        return actualBite;
    }

private:
    float m_X, m_Y;
    float m_Radius;
    float m_FoodAmount;
};
