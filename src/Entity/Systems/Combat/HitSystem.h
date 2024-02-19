#pragma once

#include <entt/entity/registry.hpp>

class HitSystem {
public:
    HitSystem() = default;
    ~HitSystem() = default;
    
    void update(entt::registry& ecs, float timescale);
    void checkForHitboxCollisions(entt::registry& ecs);

private:

};