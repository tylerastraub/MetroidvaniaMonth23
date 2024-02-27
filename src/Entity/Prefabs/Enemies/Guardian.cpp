#include "Guardian.h"
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
#include "AttackComponent.h"

namespace {
    class GuardianScript : public IScript {
    public:
        GuardianScript() = default;
        ~GuardianScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            entt::entity player = *ecs.view<PlayerComponent>().begin();
            auto& enemyComp = ecs.get<EnemyComponent>(owner);
            auto& dir = ecs.get<DirectionComponent>(owner);
            auto& physics = ecs.get<PhysicsComponent>(owner);
            auto& attack = ecs.get<AttackComponent>(owner);
            auto& hitbox = ecs.get<HitboxComponent>(owner);
            auto pCollision = ecs.get<CollisionComponent>(player).collisionRect;
            auto rCollision = ecs.get<CollisionComponent>(owner).collisionRect;
            const bool onLeftEdge = ecs.get<CollisionComponent>(owner).onLeftEdge;
            const bool onRightEdge = ecs.get<CollisionComponent>(owner).onRightEdge;
            auto playerPos = ecs.get<TransformComponent>(player).position;
            auto guardianPos = ecs.get<TransformComponent>(owner).position;

            int totalAttackTime = attack.groundAttackStartup + attack.groundAttackDuration + attack.groundAttackCooldown;
            if(attack.attackTimer < totalAttackTime) {
                enemyComp.currentActionTimer = 0;
                _currentActionDirection = 0.f;
                if(attack.attackTimer < attack.groundAttackStartup) {
                    // ???
                }
                else if(attack.attackTimer < attack.groundAttackStartup + attack.groundAttackDuration) {
                    hitbox.hitboxes.clear();
                    // first half of attack is stab, second half of attack is swing
                    if(attack.attackTimer < attack.groundAttackStartup + attack.groundAttackDuration / 2) {
                        Hitbox stabHitbox;
                        stabHitbox.bounds = {0.f, 0.f, 32.f, 32.f};
                        stabHitbox.offset.x = (dir.direction == Direction::EAST) ? 32.f : -32.f;
                        stabHitbox.offset.y = 0;
                        hitbox.hitboxes.push_back(stabHitbox);
                    }
                    else {
                        Hitbox swingHitbox;
                        swingHitbox.bounds = {0.f, 0.f, 32.f, 32.f};
                        int swingStartFrame = attack.groundAttackStartup + attack.groundAttackDuration / 2;
                        int swingDuration = attack.groundAttackStartup + attack.groundAttackDuration - swingStartFrame - 1;
                        float swingPercent = static_cast<float>(attack.attackTimer - swingStartFrame) / static_cast<float>(swingDuration);
                        float maxXOffset = 64.f;
                        float maxYOffset = -32.f;
                        if(dir.direction == Direction::EAST) {
                            swingHitbox.offset.x = 32.f - maxXOffset * swingPercent;
                        }
                        else {
                            swingHitbox.offset.x = -32.f + maxXOffset * swingPercent;
                        }
                        swingHitbox.offset.y = std::sin(swingPercent * M_PI) * maxYOffset;
                        hitbox.hitboxes.push_back(swingHitbox);
                    }
                }
                else {
                    if(hitbox.hitboxes.size()) hitbox.hitboxes.clear();
                    if(hitbox.hits.size()) hitbox.hits.clear();
                }
            }
            else if(std::abs(playerPos.x - guardianPos.x) < enemyComp.playerDistanceThreshold.x &&
               std::abs(playerPos.y - guardianPos.y) < enemyComp.playerDistanceThreshold.y) {
                if(std::abs(playerPos.x - guardianPos.x) <= ATTACK_RANGE) {
                    attack.attackTimer = 0;
                }
                else {
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
            }
            hitbox.hitboxes.push_back({{0.f, 0.f, 28.f, 28.f}, {2.f, 2.f}}); // body hitbox is always active
            
            // make it so there are pauses between short bursts of movement
            if(enemyComp.currentActionTimer > _currentActionLimit) {
                _currentActionDirection = 0.f;
            }
            if(enemyComp.currentActionTimer > _currentActionDuration) {
                enemyComp.currentActionTimer = 0;
                _currentActionLimit = RandomGen::getRandomInt(enemyComp.currentActionDurationMin, enemyComp.currentActionDurationMax);
                _currentActionDuration = RandomGen::getRandomInt(ACTION_DURATION_MIN, ACTION_DURATION_MAX);
                _currentActionDirection = static_cast<float>(RandomGen::getRandomInt(0, 1));
                if(_currentActionDirection == 0.f) _currentActionDirection = -1.f;
            }

            float coefficient = 0.f;
            if((_currentActionDirection == -1.f && !onLeftEdge) || (_currentActionDirection == 1.f && !onRightEdge)) {
                coefficient = _currentActionDirection;
            }
            physics.velocity.x += physics.acceleration.x * coefficient;

            enemyComp.currentActionTimer += timescale * 1000.f;
            ++attack.attackTimer;
        }

    private:
        float _currentActionDirection = 0.f;
        int _currentActionLimit = 0;
        int _currentActionDuration = 0;
        const int ACTION_DURATION_MIN = 2000;
        const int ACTION_DURATION_MAX = 5000;
        const int ATTACK_RANGE = 56;

    };

    
    class GuardianOnHitScript : public IScript {
    public:
        GuardianOnHitScript() = default;
        ~GuardianOnHitScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {

        }

    private:

    };


    class GuardianOnHurtScript : public IScript {
    public:
        GuardianOnHurtScript() = default;
        ~GuardianOnHurtScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {

        }

    private:

    };
}

namespace prefab {
    entt::entity Guardian::create(entt::registry& ecs) {
        return create(ecs, {0.f, 0.f});
    }

    entt::entity Guardian::create(entt::registry& ecs, strb::vec2f pos) {
        entt::entity guardian = ecs.create();

        ecs.emplace<TransformComponent>(guardian, pos, pos);

        PhysicsComponent physics;
        physics.maxVelocity = {20.f, 240.f};
        physics.frictionCoefficient = 5.f;
        physics.airFrictionCoefficient = 1.f;
        physics.acceleration = {10.f, 0.f};
        physics.airAcceleration = {2.f, 0.f};
        physics.gravity = 12.5f;
        ecs.emplace<PhysicsComponent>(guardian, physics);

        ecs.emplace<DirectionComponent>(guardian, DirectionComponent{Direction::EAST});

        ecs.emplace<RenderComponent>(guardian, RenderComponent{{0, 0, 32, 32}});

        ecs.emplace<CollisionComponent>(guardian, CollisionComponent{{pos.x, pos.y , 32.f, 32.f}, {0, 0}});

        ecs.emplace<ScriptComponent>(guardian, ScriptComponent{std::make_shared<GuardianScript>()});

        HitboxComponent hitboxComp;
        hitboxComp.hitboxes.push_back({{0.f, 0.f, 28.f, 28.f}, {2.f, 2.f}});
        hitboxComp.damage = 1;
        hitboxComp.onHitScript = std::make_shared<GuardianOnHitScript>();
        hitboxComp.hitstun = 500;
        hitboxComp.knockback = {300.f, -300.f};
        ecs.emplace<HitboxComponent>(guardian, hitboxComp);

        HurtboxComponent hurtboxComp;
        hurtboxComp.bounds = {0, 0, 32, 32};
        hurtboxComp.offset = {0.f, 0.f};
        hurtboxComp.onHurtScript = std::make_shared<GuardianOnHurtScript>();
        ecs.emplace<HurtboxComponent>(guardian, hurtboxComp);

        ecs.emplace<HitstopComponent>(guardian, HitstopComponent{150});

        ecs.emplace<EnemyComponent>(guardian, EnemyComponent{{112.f, 20.f}, 500, 1000});

        AttackComponent attack;
        attack.groundAttackStartup = 25;
        attack.groundAttackDuration = 50;
        attack.groundAttackCooldown = 50;
        ecs.emplace<AttackComponent>(guardian, attack);

        return guardian;
    }

    SpritesheetPropertiesComponent Guardian::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;

        return propsComp;
    }
}