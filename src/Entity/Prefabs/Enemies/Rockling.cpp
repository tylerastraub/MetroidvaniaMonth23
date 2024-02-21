#include "Rockling.h"
#include "SpritesheetRegistry.h"
#include "RandomGen.h"
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
#include "PlayerComponent.h"
#include "EnemyComponent.h"
#include "RocklingComponent.h"

namespace {
    class RocklingScript : public IScript {
    public:
        RocklingScript() = default;
        ~RocklingScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            entt::entity player = *ecs.view<PlayerComponent>().begin();
            auto& rocklingComp = ecs.get<RocklingComponent>(owner);
            auto& enemyComp = ecs.get<EnemyComponent>(owner);
            auto& dir = ecs.get<DirectionComponent>(owner);
            auto& physics = ecs.get<PhysicsComponent>(owner);
            auto pCollision = ecs.get<CollisionComponent>(player).collisionRect;
            auto rCollision = ecs.get<CollisionComponent>(owner).collisionRect;
            const bool onLeftEdge = ecs.get<CollisionComponent>(owner).onLeftEdge;
            const bool onRightEdge = ecs.get<CollisionComponent>(owner).onRightEdge;
            auto playerPos = ecs.get<TransformComponent>(player).position;
            auto rocklingPos = ecs.get<TransformComponent>(owner).position;

            if(std::abs(playerPos.x - rocklingPos.x) < enemyComp.playerDistanceThreshold.x &&
               std::abs(playerPos.y - rocklingPos.y) < enemyComp.playerDistanceThreshold.y &&
               rocklingComp.msSinceHitPlayer > rocklingComp.playerHitCooldown) {
                enemyComp.currentActionTimer = 0;
                if(rCollision.x + rCollision.w / 2 < pCollision.x + pCollision.w / 2) {
                    dir.direction = Direction::EAST;
                    _currentActionDirection = 1.f;
                }
                else {
                    dir.direction = Direction::WEST;
                    _currentActionDirection = -1.f;
                }
            }

            if(enemyComp.currentActionTimer > _currentActionLimit) {
                enemyComp.currentActionTimer = 0;
                _currentActionLimit = RandomGen::getRandomInt(enemyComp.currentActionDurationMin, enemyComp.currentActionDurationMax);
                // alternate between moving and stopping
                if(_currentActionDirection == 0.f) {
                    _currentActionDirection = static_cast<float>(RandomGen::getRandomInt(0, 1));
                    if(_currentActionDirection == 0.f) _currentActionDirection = -1.f;
                }
                else {
                    _currentActionDirection = 0.f;
                }
            }

            float coefficient = 0.f;
            if((_currentActionDirection == -1.f && !onLeftEdge) || (_currentActionDirection == 1.f && !onRightEdge)) {
                coefficient = _currentActionDirection;
            }
            physics.velocity.x += physics.acceleration.x * coefficient; // only do this is collision.onEdge == false

            enemyComp.currentActionTimer += timescale * 1000.f;
            rocklingComp.msSinceHitPlayer += timescale * 1000.f;
        }

    private:
        float _currentActionDirection = 0.f;
        int _currentActionLimit = 0;

    };

    
    class RocklingOnHitScript : public IScript {
    public:
        RocklingOnHitScript() = default;
        ~RocklingOnHitScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            std::cout << "ROCKLING KILL!!!!!" << std::endl;
            auto& rocklingComp = ecs.get<RocklingComponent>(owner);
            rocklingComp.msSinceHitPlayer = 0;
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
        physics.maxVelocity = {30.f, 240.f};
        physics.frictionCoefficient = 5.f;
        physics.airFrictionCoefficient = 1.f;
        physics.acceleration = {10.f, 0.f};
        physics.airAcceleration = {2.f, 0.f};
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
        hitboxComp.hitstun = 400;
        hitboxComp.knockback = {200.f, -200.f};
        ecs.emplace<HitboxComponent>(rockling, hitboxComp);

        HurtboxComponent hurtboxComp;
        hurtboxComp.bounds = {0, 0, 16, 16};
        hurtboxComp.offset = {0.f, 0.f};
        hurtboxComp.onHurtScript = std::make_shared<RocklingOnHurtScript>();
        ecs.emplace<HurtboxComponent>(rockling, hurtboxComp);

        ecs.emplace<HitstopComponent>(rockling, HitstopComponent{150});

        ecs.emplace<EnemyComponent>(rockling, EnemyComponent{{112.f, 20.f}, 2000, 4000});

        ecs.emplace<RocklingComponent>(rockling, RocklingComponent{});

        return rockling;
    }

    SpritesheetPropertiesComponent Rockling::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;

        return propsComp;
    }
}