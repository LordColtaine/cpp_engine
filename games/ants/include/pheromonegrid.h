#pragma once
#include "raylib.h"
#include <vector>

struct PheromoneCell
{
    float m_HomeScent = 0.0f;
    float m_FoodScent = 0.0f;
    float m_RallyScent = 0.0f;
    bool m_IsObstacle = false;
};

struct FoodSource
{
    float m_X;
    float m_Y;
    float m_Radius;
};

struct Nest
{
    float m_X;
    float m_Y;
    float m_Radius;
};

class PheromoneGrid
{
public:
    PheromoneGrid(int screenWidth, int screenHeight, int cellSize);
    PheromoneGrid(const PheromoneGrid&) = delete;
    PheromoneGrid& operator=(const PheromoneGrid&) = delete;
    void Cleanup();

    int GetWidth() const { return m_Cols * m_CellSize; }
    int GetHeight() const { return m_Rows * m_CellSize; }

    void PrepareUpdate();
    void UpdateChunk(double dt, int startY, int endY);
    void DrawDebug() const;

    void AddHomePheromone(float worldX, float worldY, float amount);
    void AddFoodPheromone(float worldX, float worldY, float amount);
    void AddRallyPheromone(float worldX, float worldY, float radius, float amount);

    // Scent reading functions
    float GetHomePheromone(float worldX, float worldY) const;
    float GetFoodPheromone(float worldX, float worldY) const;
    float GetRallyPheromone(float worldX, float worldY) const;

    // Food Management ---
    void SpawnFood(float x, float y, float radius);
    bool CheckFoodCollision(float antX, float antY) const;
    bool TryHarvestFood(float antX, float antY, float amount);

    // Nest info ---
    void SetNest(float x, float y, float radius);
    bool CheckNestCollision(float antX, float antY) const;

    float GetNestX() const { return m_Nest.m_X; }
    float GetNestY() const { return m_Nest.m_Y; }

    // Obstacle
    void SpawnObstacle(float x, float y, float radius);
    bool IsObstacle(float x, float y) const;

private:
    int m_CellSize;
    int m_Cols;
    int m_Rows;

    std::vector<PheromoneCell> m_Grid;
    std::vector<PheromoneCell> m_OldGrid;

    std::vector<FoodSource> m_FoodSources;

    Nest m_Nest;

    Texture2D m_GridTexture;
    Color* m_PixelData;
    int GetIndex(float worldX, float worldY) const;
};
