#pragma once

#include "unit.h"

#include "raylib.h"
#include <random>
#include <vector>

class World;
class SpatialGrid;

struct BaseData
{
    float x, y;
    float radius;
    int teamID;
    float hp;
    float maxHp;
    float spawnTimer;
    bool active;
};

struct ObstacleData
{
    float x, y;
    float radius;
};

enum class CommandType
{
    SET_RALLY,
    CLEAR_RALLY,
    START_GAME,
    CLIENT_READY,
    SELECT_BOX
};

struct Command
{
    CommandType type;
    int playerID;
    float x, y;
    uint32_t turnNumber;
    float width, height;
};

class UnitManager
{
public:
    static constexpr int MAX_UNITS = 10000;
    static constexpr float MAP_WIDTH = 5000.0f;
    static constexpr float MAP_HEIGHT = 5000.0f;
    static constexpr float GRID_CELL_SIZE = 50.0f;
    static constexpr float RALLY_WEIGHT = 0.02f;
    static constexpr int MAX_BASES = 20;

    UnitManager(World* world);
    void SetRandomSeed(unsigned int seed) { m_RNG.seed(seed); }

    float GetRandomFloat(float min, float max)
    {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(m_RNG);
    }

    int GetRandomInt(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(m_RNG);
    }

    void QueueCommand(const Command& cmd) { m_PendingCommands.push_back(cmd); }
    void SpawnUnit(float x, float y, int team);
    void SpawnBase(float x, float y, float radius, int teamID, float startingHp);
    void SpawnObstacle(float x, float y, float radius) { m_Obstacles.push_back({x, y, radius}); }

    void Update(float dt);

    void SelectUnitsInBox(Rectangle box);
    void DrawEnvironment() const;

private:
    void ApplyBoidLogic(float dt);
    void ProcessCommands();
    void UpdateBases(float dt);
    void UnitUpdate(float dt);

    std::vector<BaseData> m_Bases;
    std::vector<ObstacleData> m_Obstacles;
    std::vector<UnitPhysics> m_PhysicsPool;
    std::vector<Command> m_PendingCommands;

    // for deterministic RNG on multiplayer
    std::mt19937 m_RNG;

    World* m_World;
    Vector2 m_RallyPoints[2] = {{0, 0}, {0, 0}};
    bool m_HasRallyPoint[2] = {false, false};
};
