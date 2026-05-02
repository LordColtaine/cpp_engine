#include "core/spatialgrid.h"
#include <algorithm>
#include <cmath>

SpatialGrid::SpatialGrid(float width, float height, float cellSize) : m_CellSize(cellSize)
{
    m_Cols = static_cast<int>(std::ceil(width / cellSize));
    m_Rows = static_cast<int>(std::ceil(height / cellSize));

    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        m_Buckets[i].resize(m_Cols * m_Rows);
    }
}

void SpatialGrid::Clear()
{
    for (int i = 0; i < NUM_LAYERS; ++i)
    {
        for (auto& bucket : m_Buckets[i])
        {
            bucket.clear();
        }
    }
}

int SpatialGrid::GetBucketIndex(float x, float y) const
{
    int col = static_cast<int>(x / m_CellSize);
    int row = static_cast<int>(y / m_CellSize);
    col = std::max(0, std::min(col, m_Cols - 1));
    row = std::max(0, std::min(row, m_Rows - 1));
    return row * m_Cols + col;
}

void SpatialGrid::Insert(GameObject* obj, float x, float y, CollisionLayer layer)
{
    const int index = GetBucketIndex(x, y);
    m_Buckets[static_cast<int>(layer)][index].push_back(obj);
}
