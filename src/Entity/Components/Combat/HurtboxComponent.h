#pragma once

#include "rect2.h"
#include "vec2.h"

struct HurtboxComponent {
    strb::rect2f bounds = {0.f, 0.f, 0.f, 0.f};
    strb::vec2f offset = {0.f, 0.f};
};