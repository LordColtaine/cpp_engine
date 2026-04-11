#pragma once
#include "core/gameobject.h"
#include "raylib.h"

class PheromoneGrid;

class Nest : public GameObject
{
public:
    Nest(float x, float y, float radius, PheromoneGrid* grid);

    void Update(double dt) override;
    void Draw() const override;

    size_t GetMemorySize() const override { return sizeof(*this); }
    float GetX() const override { return m_X; }
    float GetY() const override { return m_Y; }
    size_t GetInstanceTypeID() const override { return GetTypeID<Nest>(); }
    bool HasSpatialCollision() const override { return false; }

    void DepositFood(float amount);
    float ConsumeFood(float requestedAmount);
    bool CheckCollision(float antX, float antY) const;

private:
    float m_X, m_Y;
    float m_Radius;
    float m_StoredFood;

    PheromoneGrid* m_Grid;
    const float m_FoodCostPerAnt = 50.0f;
};
