#include "core/spatialgrid.h"

#include <algorithm>
#include <cmath>

SpatialGrid::SpatialGrid(float width, float height, float cellSize) : m_CellSize(cellSize)
{
    m_Cols = static_cast<int>(std::ceil(width / cellSize));
    m_Rows = static_cast<int>(std::ceil(height / cellSize));
    m_Buckets.resize(m_Cols * m_Rows);
}

void SpatialGrid::Clear()
{
    for (auto& bucket : m_Buckets)
    {
        bucket.clear();
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

void SpatialGrid::Insert(GameObject* obj, float x, float y)
{
    const int index = GetBucketIndex(x, y);
    m_Buckets[index].push_back(obj);
}

void SpatialGrid::GetNearby(std::vector<GameObject*>& outList, float x, float y, float radius) const
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
                outList.push_back(obj);
            }
        }
    }
}
