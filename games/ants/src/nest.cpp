#include "nest.h"
#include "ant.h"
#include "core/world.h"
#include "logger/logger.h"

Nest::Nest(float x, float y, float radius, PheromoneGrid* grid)
    : m_X(x), m_Y(y), m_Radius(radius), m_StoredFood(0.0f), m_Grid(grid)
{
}

void Nest::Update(double dt)
{
    while (m_StoredFood >= m_FoodCostPerAnt)
    {
        m_StoredFood -= m_FoodCostPerAnt;

        if (GetWorld() != nullptr)
        {
            GetWorld()->NewGameObject<WorkerAnt>(m_X, m_Y, m_Grid, this);

            LOG_INFO("The Queen birthed a new Worker! Vault remaining: " + std::to_string(m_StoredFood));
        }
    }
}

void Nest::Draw() const { DrawCircle(static_cast<int>(m_X), static_cast<int>(m_Y), m_Radius, {101, 67, 33, 255}); }

void Nest::DepositFood(float amount)
{
    if (amount > 0.0f)
    {
        m_StoredFood += amount;
    }
}

float Nest::ConsumeFood(float requestedAmount)
{
    if (m_StoredFood <= 0.0f)
        return 0.0f;

    // Don't let them eat more than what is actually in the vault
    const float actualEaten = std::min(m_StoredFood, requestedAmount);
    m_StoredFood -= actualEaten;

    return actualEaten;
}

bool Nest::CheckCollision(float antX, float antY) const
{
    const float dx = antX - m_X;
    const float dy = antY - m_Y;
    return (dx * dx + dy * dy) <= (m_Radius * m_Radius);
}
