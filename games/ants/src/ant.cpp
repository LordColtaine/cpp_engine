#include "ant.h"

#include "core/spatialgrid.h"
#include "core/world.h"
#include "food.h"
#include <cmath>
#include <cstdlib>

// ==========================================
// CONFIGURATION CONSTANTS
// ==========================================
namespace
{
    // Ant Physical Traits
    constexpr int WORKER_ANT_SIZE = 2;
    constexpr int SOLDIER_ANT_SIZE = 4;
    constexpr float PATROL_RADIUS = 400.0f;
    constexpr float DEFAULT_SENSOR_DISTANCE = 15.0f;
    constexpr float DEFAULT_SENSOR_ANGLE = 0.785f; // 45 degrees in radians

    // Scent & Payload
    constexpr float MAX_SCENT_PAYLOAD = 255.0f;
    constexpr float MIN_SCENT_PAYLOAD = 2.0f;
    constexpr float HOME_SCENT_DROP_RATE = 50.0f;
    constexpr float PAYLOAD_DRAIN_RATE = 15.0f;

    // Harvesting
    constexpr float HARVEST_RATE = 20.f;
    constexpr float HARVEST_TIME_FULL = 1.0f;
    constexpr float HARVEST_TIME_PARTIAL = 0.25f;

    // Movement & Steering
    constexpr float BASE_TURN_SPEED = 4.0f;
    constexpr float RALLY_TURN_MULTIPLIER = 1.5f;
    constexpr float WALL_TURN_MULTIPLIER = 2.0f;
    constexpr float WOBBLE_STRENGTH = 0.2f;
    constexpr float WORLD_BOUNDARY_MARGIN = 5.0f;

    // Math & Sensors
    constexpr float OBSTACLE_PENALTY = -9999.0f;
    constexpr float WALL_AVOID_THRESHOLD = -1000.0f;
    constexpr float DEG_TO_RAD = 3.14159f / 180.0f;
    constexpr float COS_45 = 0.707106f;
    constexpr float SIN_45 = 0.707106f;

    // Random Chances (out of 100)
    constexpr int SOLDIER_WANDER_CHANCE = 10;
    constexpr int WANDER_TWITCH_CHANCE = 5;
    constexpr int RETURN_TWITCH_CHANCE = 2;

    // Generates a random float between - 1.0f and 1.0f
    inline float GetRandomSymmetric() { return (rand() % 100 / 50.0f) - 1.0f; }
} // namespace

// ==========================================
// ANT IMPLEMENTATION
// ==========================================

Ant::Ant(float startX, float startY, PheromoneGrid* grid, Color color, float speed)
    : m_X(startX), m_Y(startY), m_Speed(speed), m_CurrentState(AntState::Wandering), m_Color(color), m_Grid(grid),
      m_HarvestTimer(0.0f), m_SensorDistance(DEFAULT_SENSOR_DISTANCE), m_SensorAngle(DEFAULT_SENSOR_ANGLE),
      m_FoodScentStrength(0.0f)
{
    m_Dx = GetRandomSymmetric();
    m_Dy = GetRandomSymmetric();
    NormalizeDirection();
}

void Ant::Update(double dt)
{
    switch (m_CurrentState)
    {
    case AntState::Wandering:
    {
        if (m_Grid != nullptr)
        {
            HandleWanderingState(dt);
            MoveAndBounce(dt);
        }
        break;
    }

    case AntState::ReturningToNest:
    {
        if (m_Grid != nullptr)
        {
            HandleReturningState(dt);
            MoveAndBounce(dt);
        }
        break;
    }

    case AntState::FoundFood:
    {
        if (m_Grid != nullptr)
        {
            thread_local std::vector<Food*> nearbyFood;
            nearbyFood.clear();
            GetWorld()->GetSpatialGrid()->GetNearbyType<Food>(nearbyFood, m_X, m_Y, m_SensorDistance);

            bool isTouchingFood = false;
            Food* closestFood = nullptr;
            float closestDistSq = 999999.0f;

            for (Food* const foodObj : nearbyFood)
            {
                const float dx = m_X - foodObj->GetX();
                const float dy = m_Y - foodObj->GetY();
                const float sqDist = dx * dx + dy * dy;
                const float foodRadius = foodObj->GetRadius();

                // Calculate movement of ant so small food particles aren't missed.
                const float stride = m_Speed * static_cast<float>(dt);
                const float hitRadius = foodRadius + stride;

                if (sqDist <= (hitRadius * hitRadius))
                {
                    const float spaceLeft = m_MaxCarryCapacity - m_CarriedFood;
                    const float requestedBite = std::min(HARVEST_RATE * static_cast<float>(dt), spaceLeft);
                    const float actualBite = foodObj->Harvest(requestedBite);

                    if (actualBite > 0.0f)
                    {
                        m_CarriedFood += actualBite;
                        isTouchingFood = true;

                        if (m_CarriedFood >= (m_MaxCarryCapacity - 0.01f))
                        {
                            m_CarriedFood = m_MaxCarryCapacity;
                            m_CurrentState = AntState::ReturningToNest;
                            m_FoodScentStrength = MAX_SCENT_PAYLOAD;
                            m_Color = BLUE;
                        }
                    }
                    break;
                }

                // Keep track of closest.
                if (sqDist < closestDistSq)
                {
                    closestDistSq = sqDist;
                    closestFood = foodObj;
                }
            }

            if (!isTouchingFood)
            {
                if (closestFood != nullptr)
                {
                    const float dx = m_X - closestFood->GetX();
                    const float dy = m_Y - closestFood->GetY();
                    const float dist = std::sqrt(closestDistSq);

                    m_X -= (dx / dist) * m_Speed * static_cast<float>(dt);
                    m_Y -= (dy / dist) * m_Speed * static_cast<float>(dt);
                }
                else
                {
                    if (m_CarriedFood > 0.0f)
                    {
                        m_CurrentState = AntState::ReturningToNest;
                        m_FoodScentStrength = MAX_SCENT_PAYLOAD;
                        m_Color = BLUE;
                    }
                    else
                    {
                        m_CurrentState = AntState::Wandering;
                        m_Color = BLACK;
                    }
                }
            }
        }
    }
    break;
    }
}

void Ant::Draw() const
{
    DrawRectangle(static_cast<int>(m_X), static_cast<int>(m_Y), WORKER_ANT_SIZE, WORKER_ANT_SIZE, m_Color);
}

size_t Ant::GetMemorySize() const { return sizeof(*this); }

void Ant::NormalizeDirection()
{
    const float length = std::sqrt(m_Dx * m_Dx + m_Dy * m_Dy);
    if (length > 0)
    {
        m_Dx /= length;
        m_Dy /= length;
    }
}

float Ant::Sense(float sensorX, float sensorY, bool lookingForFood)
{
    if (m_Grid->IsObstacle(sensorX, sensorY))
    {
        return OBSTACLE_PENALTY;
    }

    if (lookingForFood)
    {
        return m_Grid->GetFoodPheromone(sensorX, sensorY);
    }
    else
    {
        return m_Grid->GetHomePheromone(sensorX, sensorY);
    }
}

float Ant::SenseRally(float sensorX, float sensorY)
{
    if (m_Grid->IsObstacle(sensorX, sensorY))
        return OBSTACLE_PENALTY;
    return m_Grid->GetRallyPheromone(sensorX, sensorY);
}

// ==========================================
// SOLDIER ANT IMPLEMENTATION
// ==========================================

void SoldierAnt::Update(double dt)
{
    if (m_Grid == nullptr)
    {
        return;
    }

    const float dxToNest = m_Grid->GetNestX() - m_X;
    const float dyToNest = m_Grid->GetNestY() - m_Y;
    const float distSq = dxToNest * dxToNest + dyToNest * dyToNest;

    if (distSq > PATROL_RADIUS * PATROL_RADIUS)
    {
        const float angleToNest = std::atan2(dyToNest, dxToNest);
        m_Dx = std::cos(angleToNest);
        m_Dy = std::sin(angleToNest);
    }
    else if (rand() % 100 < SOLDIER_WANDER_CHANCE)
    {
        m_Dx += GetRandomSymmetric();
        m_Dy += GetRandomSymmetric();
        NormalizeDirection();
    }

    MoveAndBounce(dt);
}

void SoldierAnt::Draw() const
{
    DrawRectangle(static_cast<int>(m_X) - 1, static_cast<int>(m_Y) - 1, SOLDIER_ANT_SIZE, SOLDIER_ANT_SIZE, m_Color);
}

void Ant::HandleWanderingState(double dt)
{
    // Front Sensor Coordinates
    const float frontX = m_X + m_Dx * m_SensorDistance;
    const float frontY = m_Y + m_Dy * m_SensorDistance;

    // Left Sensor (Rotate forward vector by -45 degrees)
    const float leftDx = m_Dx * COS_45 - m_Dy * (-SIN_45);
    const float leftDy = m_Dx * (-SIN_45) + m_Dy * COS_45;
    const float leftX = m_X + leftDx * m_SensorDistance;
    const float leftY = m_Y + leftDy * m_SensorDistance;

    // Right Sensor (Rotate forward vector by +45 degrees)
    const float rightDx = m_Dx * COS_45 - m_Dy * SIN_45;
    const float rightDy = m_Dx * SIN_45 + m_Dy * COS_45;
    const float rightX = m_X + rightDx * m_SensorDistance;
    const float rightY = m_Y + rightDy * m_SensorDistance;

    const float turnSpeed = BASE_TURN_SPEED * static_cast<float>(dt);
    float currentAngle = std::atan2(m_Dy, m_Dx);

    // Look for God Scent
    const float rallyForward = SenseRally(frontX, frontY);
    const float rallyLeft = SenseRally(leftX, leftY);
    const float rallyRight = SenseRally(rightX, rightY);

    if (rallyForward > 0.0f || rallyLeft > 0.0f || rallyRight > 0.0f)
    {
        if (rallyForward < rallyLeft || rallyForward < rallyRight)
        {
            if (rallyLeft > rallyRight)
            {
                currentAngle -= turnSpeed * RALLY_TURN_MULTIPLIER;
            }
            else
            {
                currentAngle += turnSpeed * RALLY_TURN_MULTIPLIER;
            }
        }
    }
    else
    {
        // Look for Food
        const float foodForward = Sense(frontX, frontY, true);
        const float foodLeft = Sense(leftX, leftY, true);
        const float foodRight = Sense(rightX, rightY, true);

        if (foodForward > 0.0f || foodLeft > 0.0f || foodRight > 0.0f)
        {
            if (foodForward < foodLeft || foodForward < foodRight)
            {
                if (foodLeft > foodRight)
                {
                    currentAngle -= turnSpeed;
                }
                else
                {
                    currentAngle += turnSpeed;
                }
            }
        }
        else
        {
            // Look for Home Scent
            const float homeForward = Sense(frontX, frontY, false);
            const float homeLeft = Sense(leftX, leftY, false);
            const float homeRight = Sense(rightX, rightY, false);

            if (homeForward > homeLeft && homeForward > homeRight)
            {
                if (rand() % 2 == 0)
                {
                    currentAngle += turnSpeed;
                }
                else
                {
                    currentAngle -= turnSpeed;
                }
            }
            else if (homeLeft > homeRight)
            {
                currentAngle += turnSpeed;
            }
            else if (homeRight > homeLeft)
            {
                currentAngle -= turnSpeed;
            }
        }
    }

    const float randomWobble = GetRandomSymmetric() * WOBBLE_STRENGTH;
    currentAngle += randomWobble;

    // Apply steering
    m_Dx = std::cos(currentAngle);
    m_Dy = std::sin(currentAngle);

    // --- DROP HOME SCENT & CHECK FOOD COLLISION ---
    m_Grid->AddHomePheromone(m_X, m_Y, HOME_SCENT_DROP_RATE * static_cast<float>(dt));

    thread_local std::vector<Food*> nearbyFood;
    nearbyFood.clear();
    GetWorld()->GetSpatialGrid()->GetNearbyType<Food>(nearbyFood, m_X, m_Y, m_SensorDistance);

    for (Food* const foodObj : nearbyFood)
    {
        const float dx = m_X - foodObj->GetX();
        const float dy = m_Y - foodObj->GetY();

        const float distToFoodSq = dx * dx + dy * dy;
        const float foodRadSq = foodObj->GetRadius() * foodObj->GetRadius();

        if (distToFoodSq <= foodRadSq)
        {
            m_CurrentState = AntState::FoundFood;
            m_Color = ORANGE;
            m_HarvestTimer = 0.0f;

            const float dxToNest = m_Grid->GetNestX() - m_X;
            const float dyToNest = m_Grid->GetNestY() - m_Y;
            const float angleToNest = std::atan2(dyToNest, dxToNest);
            m_Dx = std::cos(angleToNest);
            m_Dy = std::sin(angleToNest);

            break;
        }
    }

    // Twitching and Movement
    if (rand() % 100 < WANDER_TWITCH_CHANCE)
    {
        m_Dx += GetRandomSymmetric();
        m_Dy += GetRandomSymmetric();
        NormalizeDirection();
    }
}

void Ant::HandleReturningState(double dt)
{
    // Front Sensor Coordinates
    const float frontX = m_X + m_Dx * m_SensorDistance;
    const float frontY = m_Y + m_Dy * m_SensorDistance;

    // Left Sensor (Rotate forward vector by -45 degrees)
    const float leftDx = m_Dx * COS_45 - m_Dy * (-SIN_45);
    const float leftDy = m_Dx * (-SIN_45) + m_Dy * COS_45;
    const float leftX = m_X + leftDx * m_SensorDistance;
    const float leftY = m_Y + leftDy * m_SensorDistance;

    // Right Sensor (Rotate forward vector by +45 degrees)
    const float rightDx = m_Dx * COS_45 - m_Dy * SIN_45;
    const float rightDy = m_Dx * SIN_45 + m_Dy * COS_45;
    const float rightX = m_X + rightDx * m_SensorDistance;
    const float rightY = m_Y + rightDy * m_SensorDistance;

    const float frontWall = Sense(frontX, frontY, false);
    const float leftWall = Sense(leftX, leftY, false);
    const float rightWall = Sense(rightX, rightY, false);

    float currentAngle = std::atan2(m_Dy, m_Dx);
    const float dxToNest = m_Grid->GetNestX() - m_X;
    const float dyToNest = m_Grid->GetNestY() - m_Y;
    const float targetAngle = std::atan2(dyToNest, dxToNest);

    const float turnSpeed = BASE_TURN_SPEED * static_cast<float>(dt);

    // Try to avoid walls.
    if (frontWall < WALL_AVOID_THRESHOLD || leftWall < WALL_AVOID_THRESHOLD || rightWall < WALL_AVOID_THRESHOLD)
    {
        if (leftWall > rightWall)
        {
            currentAngle -= turnSpeed * WALL_TURN_MULTIPLIER;
        }
        else if (rightWall > leftWall)
        {
            currentAngle += turnSpeed * WALL_TURN_MULTIPLIER;
        }
        else
        {
            currentAngle += turnSpeed * WALL_TURN_MULTIPLIER;
        }
    }
    else
    {
        float angleDiff = targetAngle - currentAngle;
        while (angleDiff > M_PI)
        {
            angleDiff -= 2.0f * M_PI;
        }
        while (angleDiff < -M_PI)
        {
            angleDiff += 2.0f * M_PI;
        }

        if (angleDiff > turnSpeed)
        {
            currentAngle += turnSpeed;
        }
        else if (angleDiff < -turnSpeed)
        {
            currentAngle -= turnSpeed;
        }
        else
        {
            currentAngle = targetAngle;
        }
    }

    m_Dx = std::cos(currentAngle);
    m_Dy = std::sin(currentAngle);

    // Food pheromone degrades every time it is dropped.
    m_Grid->AddFoodPheromone(m_X, m_Y, m_FoodScentStrength);
    m_FoodScentStrength -= PAYLOAD_DRAIN_RATE * static_cast<float>(dt);

    if (m_FoodScentStrength < MIN_SCENT_PAYLOAD)
    {
        m_FoodScentStrength = MIN_SCENT_PAYLOAD;
    }

    if (m_Grid->CheckNestCollision(m_X, m_Y))
    {
        m_CurrentState = AntState::Wandering;
        m_Color = BLACK;
        m_Dx *= -1.0f;
        m_Dy *= -1.0f;
        m_CarriedFood = 0.0f;
        NormalizeDirection();
    }

    if (rand() % 100 < RETURN_TWITCH_CHANCE)
    {
        m_Dx += GetRandomSymmetric();
        m_Dy += GetRandomSymmetric();
        NormalizeDirection();
    }
}

void Ant::MoveAndBounce(double dt)
{
    const float nextX = m_X + m_Dx * m_Speed * dt;
    const float nextY = m_Y + m_Dy * m_Speed * dt;

    if (m_Grid->IsObstacle(nextX, nextY))
    {
        m_Dx *= -1.0f;
        m_Dy *= -1.0f;

        m_Dx += GetRandomSymmetric();
        m_Dy += GetRandomSymmetric();
        NormalizeDirection();
    }
    else
    {
        m_X = nextX;
        m_Y = nextY;
    }

    const int worldWidth = m_Grid->GetWidth();
    const int worldHeight = m_Grid->GetHeight();

    if (m_X < WORLD_BOUNDARY_MARGIN)
    {
        m_X = WORLD_BOUNDARY_MARGIN;
        m_Dx *= -1.0f;
    }
    if (m_X > worldWidth - WORLD_BOUNDARY_MARGIN)
    {
        m_X = worldWidth - WORLD_BOUNDARY_MARGIN;
        m_Dx *= -1.0f;
    }
    if (m_Y < WORLD_BOUNDARY_MARGIN)
    {
        m_Y = WORLD_BOUNDARY_MARGIN;
        m_Dy *= -1.0f;
    }
    if (m_Y > worldHeight - WORLD_BOUNDARY_MARGIN)
    {
        m_Y = worldHeight - WORLD_BOUNDARY_MARGIN;
        m_Dy *= -1.0f;
    }
}
