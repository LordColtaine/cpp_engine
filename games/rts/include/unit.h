#pragma once
#include "behaviour/btagent.h"

struct UnitPhysics
{
    float x, y;
    float vx, vy;
    float ax, ay;
    int teamID;
    bool active;
    bool selected;
    float hp;
};

class Unit : public BTAgent
{
public:
    Unit(int physicsIndex, UnitPhysics* pool);

    void Update(double dt) override;

    void Draw() const override;

    size_t GetMemorySize() const override { return sizeof(*this); }
    float GetX() const override { return m_X; }
    float GetY() const override { return m_Y; }
    size_t GetInstanceTypeID() const override { return GetTypeID<Unit>(); }
    int GetPhysicsIndex() const { return m_PhysicsIndex; }

protected:
    void ConstructBrain() override {}

private:
    int m_PhysicsIndex;
    UnitPhysics* m_Pool;
    float m_X, m_Y;
};
