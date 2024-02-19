#pragma once

#include "rect2.h"
#include "vec2.h"
#include "ScriptComponent.h"

#include <entt/entity/entity.hpp>

struct HurtboxComponent {
    strb::rect2f bounds = {0.f, 0.f, 0.f, 0.f};
    strb::vec2f offset = {0.f, 0.f};

    std::shared_ptr<IScript> onHurtScript = nullptr;

    int invulnTime = 100;
    int invulnCount = 100;

    int hitstunTime = 0;
    int hitstunCount = 0;
};