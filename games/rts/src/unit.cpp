#include "unit.h"

#include "raylib.h"
#include <cmath>

Unit::Unit(int physicsIndex, UnitPhysics* pool)
    : m_PhysicsIndex(physicsIndex), m_Pool(pool), m_X(pool[physicsIndex].x), m_Y(pool[physicsIndex].y)
{
}

void Unit::Update(double dt)
{
    if (!m_Pool[m_PhysicsIndex].active)
    {
        MarkForKill();
        return;
    }

    BTAgent::Update(dt);

    m_X = m_Pool[m_PhysicsIndex].x;
    m_Y = m_Pool[m_PhysicsIndex].y;
}

void Unit::Draw() const
{
    if (!m_Pool[m_PhysicsIndex].active)
    {
        return;
    }
    const bool isSelected = m_Pool[m_PhysicsIndex].selected;
    Color unitColor = m_Pool[m_PhysicsIndex].teamID == 0 ? LIME : RED;

    const float vx = m_Pool[m_PhysicsIndex].vx;
    const float vy = m_Pool[m_PhysicsIndex].vy;
    const float angle = std::atan2(vy, vx);

    // Boid dimensions
    const float noseLength = 10.0f;
    const float wingSpread = 7.0f;

    // Calculate the 3 vertices of the triangle
    // Nose (pointing forward along the angle)
    const Vector2 p1 = {m_X + std::cos(angle) * noseLength, m_Y + std::sin(angle) * noseLength};

    // Back Left Wing (offset angle by ~140 degrees / 2.5 radians)
    const Vector2 p2 = {m_X + std::cos(angle - 2.5f) * wingSpread, m_Y + std::sin(angle - 2.5f) * wingSpread};

    // Back Right Wing
    const Vector2 p3 = {m_X + std::cos(angle + 2.5f) * wingSpread, m_Y + std::sin(angle + 2.5f) * wingSpread};

    if (isSelected)
    {
        DrawCircleLines(static_cast<int>(m_X), static_cast<int>(m_Y), 12.0f, WHITE);
    }

    // Raylib expects vertices in counter-clockwise order for DrawTriangle
    DrawTriangle(p1, p2, p3, unitColor);

    // Subtle dark outline.
    DrawTriangleLines(p1, p2, p3, ColorAlpha(BLACK, 0.5f));
}
