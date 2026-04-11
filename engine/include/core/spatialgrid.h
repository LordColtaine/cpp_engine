#pragma once
#include <vector>

#include "core/gameobject.h"

class SpatialGrid
{
public:
    SpatialGrid(float width, float height, float cellSize);
    void Clear();
    void Insert(GameObject* obj, float x, float y);
    void GetNearby(std::vector<GameObject*>& outList, float x, float y, float radius) const;

    template <typename T> void GetNearbyType(std::vector<T*>& outList, float x, float y, float radius) const
    {
        const int startCol = std::max(0, static_cast<int>((x - radius) / m_CellSize));
        const int startRow = std::max(0, static_cast<int>((y - radius) / m_CellSize));
        const int endCol = std::min(m_Cols - 1, static_cast<int>((x + radius) / m_CellSize));
        const int endRow = std::min(m_Rows - 1, static_cast<int>((y + radius) / m_CellSize));

        for (int row = startRow; row <= endRow; ++row)
        {
            for (int col = startCol; col <= endCol; ++col)
            {
                const int index = row * m_Cols + col;
                for (GameObject* obj : m_Buckets[index])
                {
                    if (obj->IsA<T>())
                    {
                        outList.push_back(static_cast<T*>(obj));
                    }
                }
            }
        }
    }

private:
    float m_CellSize;
    int m_Cols;
    int m_Rows;

    std::vector<std::vector<GameObject*>> m_Buckets;
    int GetBucketIndex(float x, float y) const;
};
