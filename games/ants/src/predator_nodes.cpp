#include "predator_nodes.h"
#include "ant.h"
#include "core/spatialgrid.h"
#include "core/world.h"
#include "logger/profiler.h"
#include "predator.h"

NodeState Condition_IsHungry::Tick(double dt)
{
    if (m_Owner->GetEnergy() < (m_Owner->GetMaxEnergy() * 0.9f))
    {
        return NodeState::Success;
    }

    return NodeState::Failure;
}

NodeState Action_Patrol::Tick(double dt)
{
    if (!m_HasTarget)
    {
        m_TargetX = m_Owner->GetX() + ((std::rand() % 400) - 200.0f);
        m_TargetY = m_Owner->GetY() + ((std::rand() % 400) - 200.0f);
        m_HasTarget = true;
    }

    const float dx = m_TargetX - m_Owner->GetX();
    const float dy = m_TargetY - m_Owner->GetY();
    const float distSq = dx * dx + dy * dy;

    if (distSq < 25.0f)
    {
        m_HasTarget = false;
        return NodeState::Success;
    }

    const float dist = std::sqrt(distSq);
    m_Owner->SetPosition(m_Owner->GetX() + (dx / dist) * m_Speed * static_cast<float>(dt),
                         m_Owner->GetY() + (dy / dist) * m_Speed * static_cast<float>(dt));

    return NodeState::Running;
}

NodeState Action_HuntAnts::Tick(double dt)
{
    PROFILE_FUNCTION();
    thread_local std::vector<Ant*> nearbyAnts;
    nearbyAnts.clear();

    m_Owner->GetWorld()->GetSpatialGrid()->GetNearbyType<Ant>(nearbyAnts, m_Owner->GetX(), m_Owner->GetY(), 200.0f,
                                                              CollisionLayer::Layer1);

    if (nearbyAnts.empty())
    {
        return NodeState::Failure;
    }

    Ant* targetAnt = nullptr;
    float closestDistSq = 999999.0f;

    for (Ant* const ant : nearbyAnts)
    {
        if (ant->IsPendingKill())
            continue;

        const float dx = ant->GetX() - m_Owner->GetX();
        const float dy = ant->GetY() - m_Owner->GetY();
        const float distSq = dx * dx + dy * dy;

        if (distSq < closestDistSq)
        {
            closestDistSq = distSq;
            targetAnt = ant;
        }
    }

    if (targetAnt == nullptr)
    {
        return NodeState::Failure;
    }

    if (closestDistSq < 900.0f)
    {
        targetAnt->MarkForKill();
        m_Owner->Feed(20.0f);
        return NodeState::Success;
    }

    const float dist = std::sqrt(closestDistSq);
    const float dx = targetAnt->GetX() - m_Owner->GetX();
    const float dy = targetAnt->GetY() - m_Owner->GetY();

    m_Owner->SetPosition(m_Owner->GetX() + (dx / dist) * m_Speed * static_cast<float>(dt),
                         m_Owner->GetY() + (dy / dist) * m_Speed * static_cast<float>(dt));

    return NodeState::Running;
}
