#pragma once

#include "vec2.h"
#include "SpritesheetPropertiesComponent.h"

#include <entt/entity/registry.hpp>

namespace prefab {
    class AirDiver {
    public:
        AirDiver() = default;
        ~AirDiver() = default;

        static entt::entity create(entt::registry& ecs);
        static entt::entity create(entt::registry& ecs, strb::vec2f pos);

    private:
        static SpritesheetPropertiesComponent createSpritesheetPropertiesComponent();

    };
}