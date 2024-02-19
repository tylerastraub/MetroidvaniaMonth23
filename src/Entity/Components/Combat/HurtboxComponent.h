#pragma once

#include "rect2.h"
#include "vec2.h"

#include <entt/entity/entity.hpp>

struct HurtboxComponent {
    strb::rect2f bounds = {0.f, 0.f, 0.f, 0.f};
    strb::vec2f offset = {0.f, 0.f};

    int invulnTime = 100;
    int invulnCount = 100;
};