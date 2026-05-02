#pragma once
#include "core/gameobject.h"
#include "logger/profiler.h"
#include <vector>

class SpatialGrid
{
public:
    SpatialGrid(float width, float height, float cellSize);
    void Clear();
    void Insert(GameObject* obj, float x, float y, CollisionLayer layer);

    template <typename T>
    void GetNearbyType(std::vector<T*>& outList, float x, float y, float radius, CollisionLayer layer) const
    {
        PROFILE_FUNCTION();

        const int startCol = std::max(0, static_cast<int>((x - radius) / m_CellSize));
        const int startRow = std::max(0, static_cast<int>((y - radius) / m_CellSize));
        const int endCol = std::min(m_Cols - 1, static_cast<int>((x + radius) / m_CellSize));
        const int endRow = std::min(m_Rows - 1, static_cast<int>((y + radius) / m_CellSize));

        for (int row = startRow; row <= endRow; ++row)
        {
            for (int col = startCol; col <= endCol; ++col)
            {
                const int index = row * m_Cols + col;
                for (GameObject* obj : m_Buckets[static_cast<int>(layer)][index])
                {
                    outList.push_back(static_cast<T*>(obj));
                }
            }
        }
    }

private:
    float m_CellSize;
    int m_Cols;
    int m_Rows;
    int GetBucketIndex(float x, float y) const;
    static constexpr int NUM_LAYERS = static_cast<int>(CollisionLayer::Count);
    std::vector<std::vector<GameObject*>> m_Buckets[NUM_LAYERS];
};
