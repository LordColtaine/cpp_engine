#include "unitmanager.h"

#include "core/spatialgrid.h"
#include "core/world.h"
#include <algorithm>
#include <cmath>

// Steering Constants
constexpr float MAX_SPEED = 300.0f;
constexpr float MIN_SPEED = 150.0f;
constexpr float VISUAL_RANGE = 50.0f;
constexpr float SEPARATION_DIST = 15.0f;
constexpr float COHESION_WEIGHT = 0.005f;
constexpr float ALIGNMENT_WEIGHT = 0.05f;
constexpr float SEPARATION_WEIGHT = 0.1f;
constexpr float ATTACK_RANGE = 15.0f;
constexpr float CHASE_WEIGHT = 0.08f;
constexpr float RECOIL_FORCE = 150.0f;
constexpr float DPS = 50.0f;
constexpr float OBSTACLE_AVOID_WEIGHT = 50.0f;
constexpr float OBSTACLE_MARGIN = 40.0f;

UnitManager::UnitManager(World* world) : m_World(world)
{
    m_Bases.resize(MAX_BASES, {0, 0, 0, -1, 0, 0, 0, false});
    m_PhysicsPool.resize(MAX_UNITS, {0, 0, 0, 0, 0, 0, 0, false});
}

void UnitManager::SpawnUnit(float x, float y, int team)
{
    for (int i = 0; i < MAX_UNITS; ++i)
    {
        if (!m_PhysicsPool[i].active)
        {
            const float angle = GetRandomFloat(0, 360) * (PI / 180.0f);
            const float speed = GetRandomFloat(150, 300);

            m_PhysicsPool[i] = {x, y, cosf(angle) * speed, sinf(angle) * speed, 0.0f, 0.0f, team, true, false, 1.f};
            m_World->NewGameObject<Unit>(i, m_PhysicsPool.data());
            return;
        }
    }
}

void UnitManager::SpawnBase(float x, float y, float radius, int teamID, float startingHp)
{
    for (int i = 0; i < MAX_BASES; ++i)
    {
        if (!m_Bases[i].active)
        {
            m_Bases[i] = {x, y, radius, teamID, startingHp, 100.0f, 0.0f, true};
            return;
        }
    }
}

void UnitManager::SelectUnitsInBox(Rectangle box)
{
    for (int i = 0; i < MAX_UNITS; ++i)
    {
        if (!m_PhysicsPool[i].active)
        {
            continue;
        }

        if (CheckCollisionPointRec({m_PhysicsPool[i].x, m_PhysicsPool[i].y}, box))
        {
            m_PhysicsPool[i].selected = true;
        }
        else
        {
            m_PhysicsPool[i].selected = false;
        }
    }
}

void UnitManager::DrawEnvironment() const
{
    for (const auto& obs : m_Obstacles)
    {
        DrawCircle(static_cast<int>(obs.x), static_cast<int>(obs.y), obs.radius, DARKBLUE);
        DrawCircleLines(static_cast<int>(obs.x), static_cast<int>(obs.y), obs.radius, BLACK);
    }

    for (const auto& base : m_Bases)
    {
        if (!base.active)
        {
            continue;
        }

        Color baseColor = GRAY;
        if (base.teamID == 0)
            baseColor = DARKGREEN;
        if (base.teamID == 1)
            baseColor = MAROON;

        DrawCircle(static_cast<int>(base.x), static_cast<int>(base.y), base.radius, baseColor);
        DrawCircleLines(static_cast<int>(base.x), static_cast<int>(base.y), base.radius, WHITE);

        const char* hpText = TextFormat("%.0f", base.hp);
        int textWidth = MeasureText(hpText, 20);
        DrawText(hpText, static_cast<int>(base.x) - textWidth / 2, static_cast<int>(base.y) - 10, 20, WHITE);
    }
}

void UnitManager::ProcessCommands()
{
    for (const auto& cmd : m_PendingCommands)
    {
        if (cmd.type == CommandType::SET_RALLY)
        {
            m_RallyPoints[cmd.playerID] = {cmd.x, cmd.y};
            m_HasRallyPoint[cmd.playerID] = true;
        }
        else if (cmd.type == CommandType::CLEAR_RALLY)
        {
            m_HasRallyPoint[cmd.playerID] = false;
        }
        else if (cmd.type == CommandType::SELECT_BOX)
        {
            // Selection should be replicated to maintain same state for all clients.
            Rectangle box = {cmd.x, cmd.y, cmd.width, cmd.height};
            for (int i = 0; i < MAX_UNITS; ++i)
            {
                // Only evaluate units that belong to the player who issued the command
                if (!m_PhysicsPool[i].active || m_PhysicsPool[i].teamID != cmd.playerID)
                {
                    continue;
                }

                if (cmd.width > 5 && cmd.height > 5)
                {
                    m_PhysicsPool[i].selected = CheckCollisionPointRec({m_PhysicsPool[i].x, m_PhysicsPool[i].y}, box);
                }
                else
                {
                    m_PhysicsPool[i].selected = false;
                }
            }
        }
    }

    m_PendingCommands.clear();
}

void UnitManager::UpdateBases(float dt)
{
    const float SPAWN_RATE = 0.5f;
    for (auto& base : m_Bases)
    {
        if (!base.active || base.teamID == -1)
        {
            continue;
        }

        base.spawnTimer += dt;
        if (base.spawnTimer >= SPAWN_RATE)
        {
            base.spawnTimer = 0.0f;
            SpawnUnit(base.x + base.radius + 5.0f, base.y, base.teamID);
        }
    }
}

void UnitManager::UnitUpdate(float dt)
{
    for (auto& p : m_PhysicsPool)
    {
        if (!p.active)
            continue;

        p.vx += p.ax;
        p.vy += p.ay;

        float speed = std::sqrt(p.vx * p.vx + p.vy * p.vy);
        if (speed > MAX_SPEED)
        {
            p.vx = (p.vx / speed) * MAX_SPEED;
            p.vy = (p.vy / speed) * MAX_SPEED;
        }
        else if (speed < MIN_SPEED && speed > 0)
        {
            p.vx = (p.vx / speed) * MIN_SPEED;
            p.vy = (p.vy / speed) * MIN_SPEED;
        }

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // Map Boundary Bounce
        if (p.x < 0 && p.vx < 0)
        {
            p.x = 0;
            p.vx *= -1.0f;
        }
        else if (p.x > MAP_WIDTH && p.vx > 0)
        {
            p.x = MAP_WIDTH;
            p.vx *= -1.0f;
        }

        if (p.y < 0 && p.vy < 0)
        {
            p.y = 0;
            p.vy *= -1.0f;
        }
        else if (p.y > MAP_HEIGHT && p.vy > 0)
        {
            p.y = MAP_HEIGHT;
            p.vy *= -1.0f;
        }

        // Obstacle Hard Collision
        for (const auto& obs : m_Obstacles)
        {
            const float dx = p.x - obs.x;
            const float dy = p.y - obs.y;
            const float distSq = dx * dx + dy * dy;

            if (distSq < obs.radius * obs.radius)
            {
                float dist = std::sqrt(distSq);
                if (dist == 0)
                    dist = 0.1f;

                const float overlap = obs.radius - dist;
                p.x += (dx / dist) * overlap;
                p.y += (dy / dist) * overlap;
            }
        }

        // Base Sacrifice / Capture
        for (auto& base : m_Bases)
        {
            if (!base.active)
                continue;

            const float dx = p.x - base.x;
            const float dy = p.y - base.y;
            const float distSq = dx * dx + dy * dy;

            if (distSq < (base.radius * base.radius))
            {
                if (p.teamID != base.teamID)
                {
                    // Enemy base: Attack and die
                    p.hp = 0;
                    p.active = false;
                    base.hp -= 1.0f;

                    if (base.hp <= 0.0f)
                    {
                        base.teamID = p.teamID;
                        base.hp = base.maxHp * 0.25f;
                    }
                }
                else if (base.hp < base.maxHp)
                {
                    // Friendly base: Heal and die
                    p.hp = 0;
                    p.active = false;
                    base.hp += 1.0f;
                }
            }
        }

        // Death Check
        if (p.hp <= 0.0f)
        {
            p.active = false;
        }

        p.ax = 0;
        p.ay = 0;
    }
}

void UnitManager::Update(float dt)
{
    ProcessCommands();
    ApplyBoidLogic(dt);
    UpdateBases(dt);
    UnitUpdate(dt);
}

void UnitManager::ApplyBoidLogic(float dt)
{
    thread_local std::vector<Unit*> neighbors;

    // Local constants for steering
    const float EDGE_MARGIN = 150.0f;
    const float TURN_FACTOR = 15.0f;
    const float ATTACK_RANGE = 15.0f;
    const float CHASE_WEIGHT = 0.08f;
    const float RECOIL_FORCE = 150.0f;
    const float DPS = 50.0f;

    for (int i = 0; i < MAX_UNITS; ++i)
    {
        UnitPhysics& boid = m_PhysicsPool[i];
        if (!boid.active)
            continue;

        // Obstacle Avoidance (Soft Dodge)
        for (const auto& obs : m_Obstacles)
        {
            const float dx = boid.x - obs.x;
            const float dy = boid.y - obs.y;
            const float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < obs.radius + OBSTACLE_MARGIN && dist > 0.1f)
            {
                boid.ax += (dx / dist) * OBSTACLE_AVOID_WEIGHT;
                boid.ay += (dy / dist) * OBSTACLE_AVOID_WEIGHT;
            }
        }

        //  Wall Avoidance (Soft Dodge)
        if (boid.x < EDGE_MARGIN)
            boid.ax += TURN_FACTOR;
        else if (boid.x > MAP_WIDTH - EDGE_MARGIN)
            boid.ax -= TURN_FACTOR;

        if (boid.y < EDGE_MARGIN)
            boid.ay += TURN_FACTOR;
        else if (boid.y > MAP_HEIGHT - EDGE_MARGIN)
            boid.ay -= TURN_FACTOR;

        // Rally Point Pull (Only for Selected Units)
        if (boid.selected && m_HasRallyPoint[boid.teamID])
        {
            const float dx = m_RallyPoints[boid.teamID].x - boid.x;
            const float dy = m_RallyPoints[boid.teamID].y - boid.y;

            const float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > 0)
            {
                boid.ax += (dx / dist) * MAX_SPEED * RALLY_WEIGHT;
                boid.ay += (dy / dist) * MAX_SPEED * RALLY_WEIGHT;
            }
        }

        // Spatial Grid Query for Neighbors & Combat
        neighbors.clear();
        m_World->GetSpatialGrid()->GetNearbyType<Unit>(neighbors, boid.x, boid.y, VISUAL_RANGE, CollisionLayer::Layer0);

        float avg_vx = 0, avg_vy = 0;
        float avg_x = 0, avg_y = 0;
        float sep_x = 0, sep_y = 0;
        float enemy_x = 0, enemy_y = 0;

        int count = 0;
        int enemy_count = 0;

        for (Unit* nUnit : neighbors)
        {
            int nIdx = nUnit->GetPhysicsIndex();
            if (nIdx == i)
                continue;

            UnitPhysics& other = m_PhysicsPool[nIdx];
            if (!other.active)
                continue;

            const float dx = boid.x - other.x;
            const float dy = boid.y - other.y;
            const float dist = std::sqrt(dx * dx + dy * dy);

            // COMBAT LOGIC
            if (boid.teamID != other.teamID)
            {
                if (dist < ATTACK_RANGE)
                {
                    // Deal Damage
                    other.hp -= DPS * dt;

                    // Recoil
                    if (dist > 0.1f)
                    {
                        boid.ax += (dx / dist) * RECOIL_FORCE;
                        boid.ay += (dy / dist) * RECOIL_FORCE;
                    }
                }
                else
                {
                    // Chase
                    enemy_x += other.x;
                    enemy_y += other.y;
                    enemy_count++;
                }

                continue;
            }

            // FRIENDLY FLOCKING LOGIC
            if (dist < SEPARATION_DIST)
            {
                sep_x += (boid.x - other.x);
                sep_y += (boid.y - other.y);
            }

            avg_vx += other.vx;
            avg_vy += other.vy;
            avg_x += other.x;
            avg_y += other.y;
            count++;
        }

        //  Apply Forces
        if (enemy_count > 0)
        {
            // Abandon flock to attack
            enemy_x /= enemy_count;
            enemy_y /= enemy_count;
            boid.ax += (enemy_x - boid.x) * CHASE_WEIGHT;
            boid.ay += (enemy_y - boid.y) * CHASE_WEIGHT;
        }
        else if (count > 0)
        {
            // Normal flocking
            avg_vx /= count;
            avg_vy /= count;
            avg_x /= count;
            avg_y /= count;

            boid.ax += (avg_vx - boid.vx) * ALIGNMENT_WEIGHT;
            boid.ay += (avg_vy - boid.vy) * ALIGNMENT_WEIGHT;
            boid.ax += (avg_x - boid.x) * COHESION_WEIGHT;
            boid.ay += (avg_y - boid.y) * COHESION_WEIGHT;
        }

        // Separation always applies to friends
        if (count > 0)
        {
            boid.ax += sep_x * SEPARATION_WEIGHT;
            boid.ay += sep_y * SEPARATION_WEIGHT;
        }
    }
}
