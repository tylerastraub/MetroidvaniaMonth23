#include "Rockling.h"
#include "SpritesheetRegistry.h"
// Components
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "CollisionComponent.h"
#include "ScriptComponent.h"
#include "AttackComponent.h"
#include "DirectionComponent.h"
#include "HitboxComponent.h"
#include "HurtboxComponent.h"
#include "HitstopComponent.h"

namespace {
    class RocklingScript : public IScript {
    public:
        RocklingScript() = default;
        ~RocklingScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {

        }

    private:

    };

    
    class RocklingOnHitScript : public IScript {
    public:
        RocklingOnHitScript() = default;
        ~RocklingOnHitScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            std::cout << "ROCKLING KILL!!!!!" << std::endl;
        }

    private:

    };


    class RocklingOnHurtScript : public IScript {
    public:
        RocklingOnHurtScript() = default;
        ~RocklingOnHurtScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            std::cout << "ROCKLING HURTS!!!!!" << std::endl;
        }

    private:

    };
}

namespace prefab {
    entt::entity Rockling::create(entt::registry& ecs) {
        return create(ecs, {0.f, 0.f});
    }

    entt::entity Rockling::create(entt::registry& ecs, strb::vec2f pos) {
        entt::entity rockling = ecs.create();

        ecs.emplace<TransformComponent>(rockling, pos, pos);

        PhysicsComponent physics;
        physics.maxVelocity = {60.f, 240.f};
        physics.frictionCoefficient = 20.f;
        physics.airFrictionCoefficient = 20.f;
        physics.acceleration = {60.f, 0.f};
        physics.airAcceleration = {60.f, 0.f};
        physics.gravity = 12.5f;
        ecs.emplace<PhysicsComponent>(rockling, physics);

        ecs.emplace<DirectionComponent>(rockling, DirectionComponent{Direction::EAST});

        ecs.emplace<RenderComponent>(rockling, RenderComponent{{0, 0, 16, 16}});

        ecs.emplace<CollisionComponent>(rockling, CollisionComponent{{pos.x, pos.y , 16.f, 16.f}, {0, 0}});

        ecs.emplace<ScriptComponent>(rockling, ScriptComponent{std::make_shared<RocklingScript>()});

        HitboxComponent hitboxComp;
        hitboxComp.hitboxes.push_back({{0.f, 0.f, 16, 16}, {0.f, 0.f}});
        hitboxComp.damage = 1;
        hitboxComp.onHitScript = std::make_shared<RocklingOnHitScript>();
        ecs.emplace<HitboxComponent>(rockling, hitboxComp);

        HurtboxComponent hurtboxComp;
        hurtboxComp.bounds = {0, 0, 16, 16};
        hurtboxComp.offset = {0.f, 0.f};
        hurtboxComp.onHurtScript = std::make_shared<RocklingOnHurtScript>();
        ecs.emplace<HurtboxComponent>(rockling, hurtboxComp);

        ecs.emplace<HitstopComponent>(rockling, HitstopComponent{0});

        return rockling;
    }

    SpritesheetPropertiesComponent Rockling::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;

        return propsComp;
    }
}