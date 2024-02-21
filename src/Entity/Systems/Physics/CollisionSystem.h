#pragma once

#include "Level.h"
#include "Audio.h"

#include <memory>
#include <entt/entity/registry.hpp>

class CollisionSystem {
public:
    CollisionSystem() = default;
    ~CollisionSystem() = default;

    void updateLevelCollisionsOnXAxis(entt::registry& ecs, Level level);
    void updateLevelCollisionsOnYAxis(entt::registry& ecs, Level level);

    void checkForCrouchCollision(entt::registry& ecs, Level level);
    std::vector<entt::entity> checkForPlayerAndTriggerCollisions(entt::registry& ecs);
    void checkIfOnEdge(entt::registry& ecs, entt::entity entity, Level level);

private:

};