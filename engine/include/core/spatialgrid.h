#pragma once
#include <vector>

class GameObject;

class SpatialGrid
{
public:
    SpatialGrid(float width, float height, float cellSize);
    void Clear();
    void Insert(GameObject* obj, float x, float y);
    void GetNearby(std::vector<GameObject*>& outList, float x, float y, float radius) const;

private:
    float m_CellSize;
    int m_Cols;
    int m_Rows;

    std::vector<std::vector<GameObject*>> m_Buckets;
    int GetBucketIndex(float x, float y) const;
};
