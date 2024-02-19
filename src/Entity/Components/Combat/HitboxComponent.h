#pragma once

#include "rect2.h"
#include "vec2.h"

#include <vector>

struct Hitbox {
    strb::rect2f bounds = {0.f, 0.f, 0.f, 0.f};
    strb::vec2f offset = {0.f, 0.f};
};

struct HitboxComponent {
    std::vector<Hitbox> hitboxes = {};
};