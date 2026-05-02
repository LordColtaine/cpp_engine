#pragma once
#include "behaviour/btagent.h"
#include "logger/profiler.h"
#include "predator_nodes.h"
#include "raylib.h"

class Predator : public BTAgent
{
public:
    Predator(float startX, float startY, PheromoneGrid* grid)
        : m_X(startX), m_Y(startY), m_Grid(grid), m_Energy(40.0f), m_MaxEnergy(100.0f)
    {
    }

    void Update(double dt) override
    {
        PROFILE_FUNCTION();
        BTAgent::Update(dt);
        m_Energy -= 2.0f * static_cast<float>(dt);
        if (m_Energy <= 0.0f)
        {
            LOG_INFO("The Predator has starved to death.");
            MarkForKill();
            return;
        }
        m_Grid->AddFearRadius(m_X, m_Y, 150.0f, 255.f * static_cast<float>(dt));
    }

    void Feed(float amount)
    {
        m_Energy += amount;
        if (m_Energy > m_MaxEnergy)
            m_Energy = m_MaxEnergy;
    }

    void Draw() const override
    {
        float hungerPct = m_Energy / m_MaxEnergy;
        Color bodyColor = {static_cast<unsigned char>(255 - (hungerPct * 100)), 0, 255, 255};

        DrawRectangle(static_cast<int>(m_X) - 6, static_cast<int>(m_Y) - 6, 12, 12, bodyColor);
        DrawCircleLines(static_cast<int>(m_X), static_cast<int>(m_Y), 30.0f, RED);
        DrawCircleLines(static_cast<int>(m_X), static_cast<int>(m_Y), 150.0f, {255, 0, 0, 50});
    }

    size_t GetMemorySize() const override { return sizeof(*this); }
    float GetX() const override { return m_X; }
    float GetY() const override { return m_Y; }
    size_t GetInstanceTypeID() const override { return GetTypeID<Predator>(); }

    void SetPosition(float newX, float newY)
    {
        m_X = newX;
        m_Y = newY;
    }

    float GetEnergy() const { return m_Energy; }
    float GetMaxEnergy() const { return m_MaxEnergy; }

protected:
    void ConstructBrain() override
    {
        Selector* rootSelector = m_Brain->CreateNode<Selector>();

        Sequence* huntSequence = m_Brain->CreateNode<Sequence>();
        Condition_IsHungry* checkHunger = m_Brain->CreateNode<Condition_IsHungry>(this);
        Action_HuntAnts* huntAction = m_Brain->CreateNode<Action_HuntAnts>(this, 100.0f);
        huntSequence->AddChild(checkHunger);
        huntSequence->AddChild(huntAction);

        Sequence* patrolSequence = m_Brain->CreateNode<Sequence>();
        Action_Patrol* patrolAction = m_Brain->CreateNode<Action_Patrol>(this, 60.0f);
        patrolSequence->AddChild(patrolAction);

        rootSelector->AddChild(huntSequence);
        rootSelector->AddChild(patrolSequence);

        m_Brain->SetRoot(rootSelector);
    }

private:
    float m_X;
    float m_Y;
    PheromoneGrid* m_Grid;
    float m_Energy;
    float m_MaxEnergy;
};
