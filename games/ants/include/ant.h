#pragma once
#include "core/gameobject.h"
#include "pheromonegrid.h"
#include "raylib.h"

enum class AntState
{
    Wandering,
    FoundFood,
    ReturningToNest,
    Patrol
};

class Ant : public GameObject
{
public:
    Ant(float startX, float startY, PheromoneGrid* grid, Color color, float speed);

    virtual void Update(double dt) override;
    virtual void Draw() const override;
    virtual size_t GetMemorySize() const override;
    virtual float GetX() const override { return m_X; }
    virtual float GetY() const override { return m_Y; }
    virtual bool HasSpatialCollision() const override { return false; }

protected:
    float m_X, m_Y;
    float m_Dx, m_Dy;
    float m_Speed;
    AntState m_CurrentState;
    Color m_Color;
    PheromoneGrid* m_Grid;
    float m_HarvestTimer;
    float m_FoodScentStrength;

    float m_SensorDistance;
    float m_SensorAngle;

    float m_CarriedFood = 0.0f;
    float m_MaxCarryCapacity = 10.0f;

    void NormalizeDirection();
    float Sense(float sensorX, float sensorY, bool lookingForFood);
    float SenseRally(float sensorX, float sensorY);
    void HandleWanderingState(double dt);
    void HandleReturningState(double dt);
    void MoveAndBounce(double dt);
};

class WorkerAnt : public Ant
{
public:
    WorkerAnt(float startX, float startY, PheromoneGrid* grid) : Ant(startX, startY, grid, BLACK, 80.0f) {}

    size_t GetMemorySize() const override { return sizeof(*this); }
    size_t GetInstanceTypeID() const override { return GetTypeID<WorkerAnt>(); }
};

class SoldierAnt : public Ant
{
public:
    SoldierAnt(float startX, float startY, PheromoneGrid* grid) : Ant(startX, startY, grid, DARKBLUE, 40.0f)
    {
        m_CurrentState = AntState::Patrol;
    }

    size_t GetMemorySize() const override { return sizeof(*this); }
    void Update(double dt) override;
    void Draw() const override;
    size_t GetInstanceTypeID() const override { return GetTypeID<SoldierAnt>(); }
};
