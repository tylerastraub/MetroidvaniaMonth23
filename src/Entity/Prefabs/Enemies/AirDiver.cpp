#include "AirDiver.h"
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

namespace {
    class AirDiverScript : public IScript {
    public:
        AirDiverScript() = default;
        ~AirDiverScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            entt::entity player = *ecs.view<PlayerComponent>().begin();
            auto& enemyComp = ecs.get<EnemyComponent>(owner);
            auto& dir = ecs.get<DirectionComponent>(owner);
            auto& physics = ecs.get<PhysicsComponent>(owner);
            auto pCollision = ecs.get<CollisionComponent>(player).collisionRect;
            auto rCollision = ecs.get<CollisionComponent>(owner).collisionRect;
            auto playerPos = ecs.get<TransformComponent>(player).position;
            auto airDiverPos = ecs.get<TransformComponent>(owner).position;

            if(_attackTimer < ATTACK_STARTUP) {
                _currentActionDirection = {0.f, 0.f};
            }
            else if(_attackTimer < ATTACK_STARTUP + ATTACK_DURATION) {
                if(!_attackStarted) {
                    _attackStarted = true;
                    _targetPos = playerPos;
                    _currentActionDirection.x = (_targetPos.x < airDiverPos.x) ? -1.f : 1.f;
                    _currentActionDirection.y = (_targetPos.y < airDiverPos.y) ? -1.f : 1.f;
                    float vecLength = std::hypot(_targetPos.x - airDiverPos.x, _targetPos.y - airDiverPos.y);
                    _moveMod = {
                        (_targetPos.x - airDiverPos.x) / vecLength,
                        (_targetPos.y - airDiverPos.y) / vecLength
                    };
                    physics.ignoreFriction = true;
                }
                physics.velocity.x += physics.acceleration.x * _moveMod.x;
                physics.velocity.y += physics.acceleration.y * _moveMod.y;
            }
            else if(_attackTimer < ATTACK_STARTUP + ATTACK_DURATION + ATTACK_COOLDOWN) {
                physics.ignoreFriction = false;
                _attackStarted = false;
                _currentActionDirection = {0.f, 0.f};
                // todo: get distance from ground, have AirDiver move up until they are high enough off the ground
            }
            else {
                if(std::abs(playerPos.x - airDiverPos.x) < enemyComp.playerDistanceThreshold.x &&
                   std::abs(playerPos.y - airDiverPos.y) < enemyComp.playerDistanceThreshold.y) {
                    _attackTimer = 0;
                }
                else {
                    // make it so there are pauses between short bursts of movement
                    if(enemyComp.currentActionTimer > _currentActionLimit) {
                        _currentActionDirection = {0.f, 0.f};
                    }
                    if(enemyComp.currentActionTimer > _currentActionDuration) {
                        enemyComp.currentActionTimer = 0;
                        _currentActionLimit = RandomGen::getRandomInt(enemyComp.currentActionDurationMin, enemyComp.currentActionDurationMax);
                        _currentActionDuration = RandomGen::getRandomInt(ACTION_DURATION_MIN, ACTION_DURATION_MAX);
                        _currentActionDirection.x = static_cast<float>(RandomGen::getRandomInt(0, 1));
                        if(_currentActionDirection.x == 0.f) _currentActionDirection.x = -1.f;
                    }

                    float coefficient = 0.f;
                    if((_currentActionDirection.x == -1.f) || (_currentActionDirection.x == 1.f)) {
                        coefficient = _currentActionDirection.x;
                    }
                    physics.velocity.x += physics.acceleration.x * coefficient;
                }
            }

            if(_attackTimer > ATTACK_STARTUP + ATTACK_DURATION + ATTACK_COOLDOWN) enemyComp.currentActionTimer += timescale * 1000.f;
            _attackTimer += timescale * 1000.f;
        }

    private:
        strb::vec2f _currentActionDirection = {0.f, 0.f};
        int _currentActionLimit = 0;
        int _currentActionDuration = 0;
        int _attackTimer = 0;
        bool _attackStarted = false;

        strb::vec2f _targetPos = {0.f, 0.f};
        strb::vec2f _moveMod = {0.f, 0.f};

        // todo: increase attack cooldown, tweak friction more, make attack range shorter
        const int ACTION_DURATION_MIN = 2000;
        const int ACTION_DURATION_MAX = 5000;
        const int ATTACK_DURATION = 700;
        const int ATTACK_STARTUP = 500;
        const int ATTACK_COOLDOWN = 1800;

    };

    
    class AirDiverOnHitScript : public IScript {
    public:
        AirDiverOnHitScript() = default;
        ~AirDiverOnHitScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {

        }

    private:

    };


    class AirDiverOnHurtScript : public IScript {
    public:
        AirDiverOnHurtScript() = default;
        ~AirDiverOnHurtScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {

        }

    private:

    };
}

namespace prefab {
    entt::entity AirDiver::create(entt::registry& ecs) {
        return create(ecs, {0.f, 0.f});
    }

    entt::entity AirDiver::create(entt::registry& ecs, strb::vec2f pos) {
        entt::entity airDiver = ecs.create();

        ecs.emplace<TransformComponent>(airDiver, pos, pos);

        PhysicsComponent physics;
        physics.maxVelocity = {140.f, 140.f};
        physics.frictionCoefficient = 8.f;
        physics.airFrictionCoefficient = 8.f;
        physics.acceleration = {10.f, 10.f};
        physics.airAcceleration = {2.f, 0.f};
        physics.gravity = 0.f;
        ecs.emplace<PhysicsComponent>(airDiver, physics);

        ecs.emplace<DirectionComponent>(airDiver, DirectionComponent{Direction::EAST});

        ecs.emplace<RenderComponent>(airDiver, RenderComponent{{0, 0, 24, 24}});

        ecs.emplace<CollisionComponent>(airDiver, CollisionComponent{{pos.x, pos.y , 24.f, 24.f}, {0, 0}});

        ecs.emplace<ScriptComponent>(airDiver, ScriptComponent{std::make_shared<AirDiverScript>()});

        HitboxComponent hitboxComp;
        hitboxComp.hitboxes.push_back({{0.f, 0.f, 20.f, 20.f}, {2.f, 2.f}});
        hitboxComp.damage = 1;
        hitboxComp.onHitScript = std::make_shared<AirDiverOnHitScript>();
        hitboxComp.hitstun = 400;
        hitboxComp.knockback = {200.f, -200.f};
        ecs.emplace<HitboxComponent>(airDiver, hitboxComp);

        HurtboxComponent hurtboxComp;
        hurtboxComp.bounds = {0, 0, 24, 24};
        hurtboxComp.offset = {0.f, 0.f};
        hurtboxComp.onHurtScript = std::make_shared<AirDiverOnHurtScript>();
        ecs.emplace<HurtboxComponent>(airDiver, hurtboxComp);

        ecs.emplace<HitstopComponent>(airDiver, HitstopComponent{150});

        ecs.emplace<EnemyComponent>(airDiver, EnemyComponent{{80.f, 80.f}, 100, 200});

        return airDiver;
    }

    SpritesheetPropertiesComponent AirDiver::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;

        return propsComp;
    }
}