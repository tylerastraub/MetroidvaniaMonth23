#include "Player.h"
#include "SpritesheetRegistry.h"
// Components
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "InputComponent.h"
#include "CollisionComponent.h"
#include "ScriptComponent.h"

namespace {
    class PlayerScript : public IScript {
    public:
        PlayerScript() = default;
        ~PlayerScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            auto& input = ecs.get<InputComponent>(owner);
            auto physics = ecs.get<PhysicsComponent>(owner);

            input.allowedInputs = {InputEvent::LEFT, InputEvent::RIGHT};
            if(physics.offGroundCount < physics.coyoteTime) {
                input.allowedInputs.push_back(InputEvent::JUMP);
            }
        }

    private:

    };
}

namespace prefab {
    entt::entity Player::create(entt::registry& ecs) {
        return create(ecs, {0.f, 0.f});
    }

    entt::entity Player::create(entt::registry& ecs, strb::vec2f pos) {
        entt::entity player = ecs.create();

        ecs.emplace<TransformComponent>(player, pos, pos);

        PhysicsComponent physics;
        physics.maxVelocity = {70.f, 400.f};
        physics.frictionCoefficient = 15.f;
        physics.acceleration = {30.f, 0.f};
        physics.jumpPower = 220.f;
        physics.gravity = 8.f;
        ecs.emplace<PhysicsComponent>(player, physics);

        ecs.emplace<RenderComponent>(player, RenderComponent{{0, 0, 24, 32}});

        ecs.emplace<CollisionComponent>(player, CollisionComponent{{pos.x, pos.y , 24.f, 32.f}, {0, 0}});
        
        ecs.emplace<InputComponent>(player, InputComponent{{InputEvent::LEFT, InputEvent::RIGHT, InputEvent::JUMP}});

        ecs.emplace<ScriptComponent>(player, ScriptComponent{std::make_shared<PlayerScript>()});

        return player;
    }

    SpritesheetPropertiesComponent Player::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;

        return propsComp;
    }
}