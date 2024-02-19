#pragma once

#include "Audio.h"

#include <entt/entity/registry.hpp>

class HitSystem {
public:
    HitSystem() = default;
    ~HitSystem() = default;
    
    void update(entt::registry& ecs, float timescale);
    void checkForHitboxCollisions(entt::registry& ecs, float timescale, std::shared_ptr<Audio> audio);

private:

};