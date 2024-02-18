#pragma once

#include "vec2.h"
#include "rect2.h"

#include <vector>

struct AttackComponent {
    int groundAttackStartup = 0; // number of ticks before hitbox comes out
    int groundAttackDuration = 0; // hitbox duration in ticks
    int groundAttackCooldown = 0; // number of ticks after hitbox disappears that entity can't attack again
    strb::vec2f groundAttackForce = {0.f, 0.f}; // applied to attacker
    strb::vec2f groundAttackKnockback = {0.f, 0.f}; // applied to entity hit
    std::vector<strb::rect2f> groundAttackHitboxes = {};

    int airAttackStartup = 0; // number of ticks before hitbox comes out
    int airAttackDuration = 0; // hitbox duration in ticks
    int airAttackCooldown = 0; // number of ticks after hitbox disappears that entity can't attack again
    strb::vec2f airAttackForce = {0.f, 0.f}; // applied to attacker
    float airAttackKnockback = 0.f; // applied to entity hit. direction is determined based on relative position
    std::vector<strb::rect2f> airAttackHitboxes = {};
    
    int attackTimer = 0; // current attack timer. gets reset if grounded state changes
};