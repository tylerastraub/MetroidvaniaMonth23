#pragma once

#include "vec2.h"
#include "SpritesheetPropertiesComponent.h"

#include <entt/entity/registry.hpp>

namespace prefab {
    class Player {
    public:
        Player() = default;
        ~Player() = default;

        static entt::entity create(entt::registry& ecs);
        static entt::entity create(entt::registry& ecs, strb::vec2f pos);

    private:
        static SpritesheetPropertiesComponent createSpritesheetPropertiesComponent();

        static const int RUNNING_NUM_OF_FRAMES = 8;
        static const int RUNNING_MS_BETWEEN_FRAMES = 100;
        static const int FALLING_NUM_OF_FRAMES = 2;
        static const int FALLING_MS_BETWEEN_FRAMES = 100;

    };
}