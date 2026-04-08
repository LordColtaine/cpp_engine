#include "pheromonegrid.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

namespace
{
    constexpr float HOME_EVAP_RATE = 0.1f;
    constexpr float FOOD_EVAP_RATE = 0.02f;
    constexpr float RALLY_EVAP_RATE = 0.2f;

    constexpr float SELF_BLUR_FACTOR = 0.6f;
    constexpr float ADJ_BLUR_FACTOR = 0.1f;

    constexpr float VISUAL_BOOST_FOOD = 2.0f;
    constexpr float VISUAL_BOOST_HOME = 1.5f;
    constexpr float VISUAL_BOOST_RALLY = 1.5f;

    constexpr float MAX_SCENT_VALUE = 255.f;
    constexpr float RALLY_RED_RATIO = 0.66f;
    constexpr float MIN_ALPHA = 0.005f;

    constexpr float FOOD_MIN_RAD = 2.f;
} // namespace

PheromoneGrid::PheromoneGrid(int screenWidth, int screenHeight, int cellSize) : m_CellSize(cellSize)
{
    m_Cols = screenWidth / m_CellSize;
    m_Rows = screenHeight / m_CellSize;
    m_Grid.resize(m_Cols * m_Rows);
    m_OldGrid.resize(m_Cols * m_Rows);

    m_PixelData = new Color[m_Cols * m_Rows];
    for (int i = 0; i < m_Cols * m_Rows; ++i)
    {
        m_PixelData[i] = BLANK; // BLANK is Raylib's 100% transparent color
    }

    Image img = GenImageColor(m_Cols, m_Rows, BLANK);
    m_GridTexture = LoadTextureFromImage(img);
    UnloadImage(img);
}

void PheromoneGrid::Cleanup()
{
    delete[] m_PixelData;
    UnloadTexture(m_GridTexture);
}

int PheromoneGrid::GetIndex(float worldX, float worldY) const
{
    int col = static_cast<int>(worldX) / m_CellSize;
    int row = static_cast<int>(worldY) / m_CellSize;

    col = std::clamp(col, 0, m_Cols - 1);
    row = std::clamp(row, 0, m_Rows - 1);

    return row * m_Cols + col;
}

void PheromoneGrid::PrepareUpdate() { m_OldGrid = m_Grid; }

void PheromoneGrid::UpdateChunk(double dt, int startY, int endY)
{
    const float homeEvap = 1.0f - (HOME_EVAP_RATE * static_cast<float>(dt));
    const float foodEvap = 1.0f - (FOOD_EVAP_RATE * static_cast<float>(dt));
    const float rallyEvap = 1.0f - (RALLY_EVAP_RATE * static_cast<float>(dt));
    const float flatDecay = 2.0f * static_cast<float>(dt);

    startY = std::max(1, startY);
    endY = std::min(m_Rows - 1, endY);

    for (int y = startY; y < endY; ++y)
    {
        for (int x = 1; x < m_Cols - 1; ++x)
        {
            const int i = y * m_Cols + x;
            if (m_OldGrid[i].m_IsObstacle)
            {
                m_Grid[i].m_HomeScent = 0.0f;
                m_Grid[i].m_FoodScent = 0.0f;
                m_Grid[i].m_RallyScent = 0.0f;
                m_PixelData[i] = DARKGRAY;
                continue;
            }

            // Applying a "blur" effect so that the scent looks like it spreads around.
            const float blurredHome =
                m_OldGrid[i].m_HomeScent * SELF_BLUR_FACTOR + m_OldGrid[i - 1].m_HomeScent * ADJ_BLUR_FACTOR +
                m_OldGrid[i + 1].m_HomeScent * ADJ_BLUR_FACTOR + m_OldGrid[i - m_Cols].m_HomeScent * ADJ_BLUR_FACTOR +
                m_OldGrid[i + m_Cols].m_HomeScent * ADJ_BLUR_FACTOR;

            const float blurredFood =
                m_OldGrid[i].m_FoodScent * SELF_BLUR_FACTOR + m_OldGrid[i - 1].m_FoodScent * ADJ_BLUR_FACTOR +
                m_OldGrid[i + 1].m_FoodScent * ADJ_BLUR_FACTOR + m_OldGrid[i - m_Cols].m_FoodScent * ADJ_BLUR_FACTOR +
                m_OldGrid[i + m_Cols].m_FoodScent * ADJ_BLUR_FACTOR;

            const float blurredRally =
                m_OldGrid[i].m_RallyScent * SELF_BLUR_FACTOR + m_OldGrid[i - 1].m_RallyScent * ADJ_BLUR_FACTOR +
                m_OldGrid[i + 1].m_RallyScent * ADJ_BLUR_FACTOR + m_OldGrid[i - m_Cols].m_RallyScent * ADJ_BLUR_FACTOR +
                m_OldGrid[i + m_Cols].m_RallyScent * ADJ_BLUR_FACTOR;

            m_Grid[i].m_HomeScent = (blurredHome * homeEvap) - flatDecay;
            m_Grid[i].m_FoodScent = (blurredFood * foodEvap) - flatDecay;
            m_Grid[i].m_RallyScent = (blurredRally * rallyEvap) - flatDecay;

            if (m_Grid[i].m_HomeScent < 0.01f)
                m_Grid[i].m_HomeScent = 0.0f;
            if (m_Grid[i].m_FoodScent < 0.01f)
                m_Grid[i].m_FoodScent = 0.0f;
            if (m_Grid[i].m_RallyScent < 0.01f)
                m_Grid[i].m_RallyScent = 0.0f;

            const float food = std::min(1.0f, (m_Grid[i].m_FoodScent * VISUAL_BOOST_FOOD) / MAX_SCENT_VALUE);
            const float home = std::min(1.0f, (m_Grid[i].m_HomeScent * VISUAL_BOOST_HOME) / MAX_SCENT_VALUE);
            const float rally = std::min(1.0f, (m_Grid[i].m_RallyScent * VISUAL_BOOST_RALLY) / MAX_SCENT_VALUE);

            // Food pushes pure Red. Rally (Purple) pushes Red and Blue. Home pushes pure Blue.
            const float r = std::min(1.0f, food + (rally * RALLY_RED_RATIO));
            const float g = 0.0f;
            const float b = std::min(1.0f, home + rally);

            // alpha is equal to whichever scent is the strongest
            const float a = std::max(food, std::max(home, rally));

            if (a > MIN_ALPHA)
            {
                m_PixelData[i] = {
                    static_cast<unsigned char>(r * MAX_SCENT_VALUE), static_cast<unsigned char>(g * MAX_SCENT_VALUE),
                    static_cast<unsigned char>(b * MAX_SCENT_VALUE), static_cast<unsigned char>(a * MAX_SCENT_VALUE)};
            }
            else
            {
                m_PixelData[i] = BLANK;
            }
        }
    }
}

void PheromoneGrid::AddHomePheromone(float worldX, float worldY, float amount)
{
    const int index = GetIndex(worldX, worldY);
    m_Grid[index].m_HomeScent += amount;

    // Add to nest scent so it gets stronger with more ants.
    if (m_Grid[index].m_HomeScent > MAX_SCENT_VALUE)
    {
        m_Grid[index].m_HomeScent = MAX_SCENT_VALUE;
    }
}

void PheromoneGrid::AddFoodPheromone(float worldX, float worldY, float amount)
{
    const int index = GetIndex(worldX, worldY);
    // Override instead of adding to prevent stronger scent near nest.
    m_Grid[index].m_FoodScent = std::max(m_Grid[index].m_FoodScent, amount);
}

float PheromoneGrid::GetHomePheromone(float worldX, float worldY) const
{
    return m_Grid[GetIndex(worldX, worldY)].m_HomeScent;
}

float PheromoneGrid::GetFoodPheromone(float worldX, float worldY) const
{
    return m_Grid[GetIndex(worldX, worldY)].m_FoodScent;
}

void PheromoneGrid::SpawnFood(float x, float y, float radius) { m_FoodSources.push_back({x, y, radius}); }

bool PheromoneGrid::CheckFoodCollision(float antX, float antY) const
{
    for (const auto& food : m_FoodSources)
    {
        const float dx = antX - food.m_X;
        const float dy = antY - food.m_Y;
        const float distanceSquared = dx * dx + dy * dy;

        if (distanceSquared <= (food.m_Radius * food.m_Radius))
        {
            return true;
        }
    }

    return false;
}

bool PheromoneGrid::TryHarvestFood(float antX, float antY, float amount)
{
    for (auto it = m_FoodSources.begin(); it != m_FoodSources.end(); ++it)
    {
        const float dx = antX - it->m_X;
        const float dy = antY - it->m_Y;

        if ((dx * dx + dy * dy) <= (it->m_Radius * it->m_Radius))
        {
            it->m_Radius -= amount;
            if (it->m_Radius <= MIN_ALPHA)
            {
                m_FoodSources.erase(it);
            }

            return true;
        }
    }

    return false;
}

void PheromoneGrid::DrawDebug() const
{
    UpdateTexture(m_GridTexture, m_PixelData);
    DrawTextureEx(m_GridTexture, {0.0f, 0.0f}, 0.0f, static_cast<float>(m_CellSize), WHITE);
    DrawCircle(static_cast<int>(m_Nest.m_X), static_cast<int>(m_Nest.m_Y), m_Nest.m_Radius, DARKBROWN);
    for (const auto& food : m_FoodSources)
    {
        DrawCircle(static_cast<int>(food.m_X), static_cast<int>(food.m_Y), food.m_Radius, GREEN);
    }
}

void PheromoneGrid::SetNest(float x, float y, float radius) { m_Nest = {x, y, radius}; }

bool PheromoneGrid::CheckNestCollision(float antX, float antY) const
{
    const float dx = antX - m_Nest.m_X;
    const float dy = antY - m_Nest.m_Y;
    const float distanceSquared = dx * dx + dy * dy;

    return distanceSquared <= (m_Nest.m_Radius * m_Nest.m_Radius);
}

void PheromoneGrid::SpawnObstacle(float x, float y, float radius)
{
    const int gridX = static_cast<int>(x) / m_CellSize;
    const int gridY = static_cast<int>(y) / m_CellSize;
    const int gridRadius = static_cast<int>(radius) / m_CellSize;

    for (int dy = -gridRadius; dy <= gridRadius; ++dy)
    {
        for (int dx = -gridRadius; dx <= gridRadius; ++dx)
        {
            if (dx * dx + dy * dy <= gridRadius * gridRadius)
            {
                int cx = gridX + dx;
                int cy = gridY + dy;

                if (cx >= 0 && cx < m_Cols && cy >= 0 && cy < m_Rows)
                {
                    m_Grid[cy * m_Cols + cx].m_IsObstacle = true;
                    m_Grid[cy * m_Cols + cx].m_HomeScent = 0.0f;
                    m_Grid[cy * m_Cols + cx].m_FoodScent = 0.0f;
                }
            }
        }
    }
}

bool PheromoneGrid::IsObstacle(float x, float y) const
{
    const int gridX = static_cast<int>(x) / m_CellSize;
    const int gridY = static_cast<int>(y) / m_CellSize;

    if (gridX >= 0 && gridX < m_Cols && gridY >= 0 && gridY < m_Rows)
    {
        return m_Grid[gridY * m_Cols + gridX].m_IsObstacle;
    }

    return true;
}

void PheromoneGrid::AddRallyPheromone(float worldX, float worldY, float radius, float amount)
{
    const int gridX = static_cast<int>(worldX) / m_CellSize;
    const int gridY = static_cast<int>(worldY) / m_CellSize;
    const int gridRadius = static_cast<int>(radius) / m_CellSize;

    for (int dy = -gridRadius; dy <= gridRadius; ++dy)
    {
        for (int dx = -gridRadius; dx <= gridRadius; ++dx)
        {
            if (dx * dx + dy * dy <= gridRadius * gridRadius)
            {
                const int cx = gridX + dx;
                const int cy = gridY + dy;

                if (cx >= 0 && cx < m_Cols && cy >= 0 && cy < m_Rows)
                {
                    const int index = cy * m_Cols + cx;
                    m_Grid[index].m_RallyScent += amount;
                    if (m_Grid[index].m_RallyScent > MAX_SCENT_VALUE)
                    {
                        m_Grid[index].m_RallyScent = MAX_SCENT_VALUE;
                    }
                }
            }
        }
    }
}

float PheromoneGrid::GetRallyPheromone(float worldX, float worldY) const
{
    return m_Grid[GetIndex(worldX, worldY)].m_RallyScent;
}
