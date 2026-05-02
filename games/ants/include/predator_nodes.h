#pragma once
#include "behaviour/behaviourtree.h"
#include <cmath>
#include <cstdlib>

// Forward declaration
class Predator;

// ==========================================
// CONDITION: Is the Predator Hungry?
// ==========================================
class Condition_IsHungry : public BTNode
{
public:
    Condition_IsHungry(Predator* owner) : m_Owner(owner) {}

    NodeState Tick(double dt) override;
    size_t GetMemorySize() const override { return sizeof(*this); }

private:
    Predator* m_Owner;
};

// ==========================================
// ACTION: Move to a Random Patrol Point
// ==========================================
class Action_Patrol : public BTNode
{
public:
    Action_Patrol(Predator* owner, float speed)
        : m_Owner(owner), m_Speed(speed), m_TargetX(0.0f), m_TargetY(0.0f), m_HasTarget(false)
    {
    }

    NodeState Tick(double dt) override;
    size_t GetMemorySize() const override { return sizeof(*this); }

private:
    Predator* m_Owner;
    float m_Speed;
    float m_TargetX;
    float m_TargetY;
    bool m_HasTarget;
};

class Action_HuntAnts : public BTNode
{
public:
    Action_HuntAnts(Predator* owner, float speed) : m_Owner(owner), m_Speed(speed) {}

    NodeState Tick(double dt) override;
    size_t GetMemorySize() const override { return sizeof(*this); }

private:
    Predator* m_Owner;
    float m_Speed;
};
