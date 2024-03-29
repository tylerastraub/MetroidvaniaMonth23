#pragma once

#include "rect2.h"
#include "vec2.h"
#include "ScriptComponent.h"

#include <vector>
#include <entt/entity/entity.hpp>

struct Hitbox {
    strb::rect2f bounds = {0.f, 0.f, 0.f, 0.f};
    strb::vec2f offset = {0.f, 0.f};
};

struct HitboxComponent {
    std::vector<Hitbox> hitboxes = {};
    int damage = 0;
    int hitstun = 0;
    strb::vec2f selfKnockback = {0.f, 0.f};
    strb::vec2f knockback = {0.f, 0.f};

    std::shared_ptr<IScript> onHitScript = nullptr;

    std::vector<entt::entity> hits = {};
    int doubleHitTimer = 0;
    int doubleHitTimerLimit = 0; // how long until a hitbox can double hit
};