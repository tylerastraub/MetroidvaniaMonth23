#include "Player.h"
#include "SpritesheetRegistry.h"
// Components
#include "TransformComponent.h"
#include "PlayerComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "InputComponent.h"
#include "CollisionComponent.h"
#include "ScriptComponent.h"
#include "PowerupComponent.h"
#include "CrouchComponent.h"
#include "AttackComponent.h"
#include "DirectionComponent.h"
#include "HitboxComponent.h"
#include "HurtboxComponent.h"
#include "HitstopComponent.h"
#include "AnimationComponent.h"

namespace {
    class PlayerScript : public IScript {
    public:
        PlayerScript() = default;
        ~PlayerScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            auto& input = ecs.get<InputComponent>(owner);
            auto& physics = ecs.get<PhysicsComponent>(owner);
            auto& render = ecs.get<RenderComponent>(owner);
            auto& collision = ecs.get<CollisionComponent>(owner);
            auto& transform = ecs.get<TransformComponent>(owner);
            auto& crouchComp = ecs.get<CrouchComponent>(owner);
            auto& attack = ecs.get<AttackComponent>(owner);
            auto& dir = ecs.get<DirectionComponent>(owner);
            auto& hitbox = ecs.get<HitboxComponent>(owner);
            auto& hurtbox = ecs.get<HurtboxComponent>(owner);
            auto& hitstop = ecs.get<HitstopComponent>(owner);
            auto& state = ecs.get<StateComponent>(owner);
            auto powerup = ecs.get<PowerupComponent>(owner);

            int totalAttackDuration = 0;
            if(physics.touchingGround) totalAttackDuration = attack.groundAttackStartup + attack.groundAttackDuration;
            else totalAttackDuration = attack.airAttackStartup + attack.airAttackDuration;
            bool canAct = (physics.offWallCount > physics.wallJumpTime || physics.wallSliding) &&
                          hurtbox.hitstunCount >= hurtbox.hitstunTime;
                         
            if(physics.touchingGround != physics.touchingGroundLastTick ||
               physics.wallSliding ||
               hurtbox.hitstunCount < hurtbox.hitstunTime
            ) {
                attack.attackTimer = 1000;
            }

            // ==================== SET ALLOWED INPUTS ====================
            input.allowedInputs = {};
            if(canAct) {
                if(!(physics.touchingGround && hitbox.doubleHitTimer < ON_HIT_FORCE_DURATION)) {
                    if(powerup.moveLeft) {
                        input.allowedInputs.push_back(InputEvent::LEFT);
                    }
                    if(powerup.moveRight) {
                        input.allowedInputs.push_back(InputEvent::RIGHT);
                    }
                }
                if((physics.offGroundCount < physics.coyoteTime && powerup.jump) ||
                   physics.wallSliding && powerup.walljump) {
                    input.allowedInputs.push_back(InputEvent::JUMP);
                }
                if(physics.touchingGround) {
                    if(powerup.crouch) input.allowedInputs.push_back(InputEvent::DOWN);
                    if(powerup.groundAttack && attack.attackTimer > totalAttackDuration + attack.groundAttackCooldown) {
                        input.allowedInputs.push_back(InputEvent::ATTACK);
                    }
                }
                else {
                    if(powerup.airAttack && attack.attackTimer > totalAttackDuration + attack.airAttackCooldown && !physics.wallSliding) {
                        input.allowedInputs.push_back(InputEvent::ATTACK);
                    }
                }
            }

            // ==================== HANDLE X INPUTS ====================
            if(input.inputTime[InputEvent::LEFT] > 0 &&
               input.inputTime[InputEvent::RIGHT] == 0 &&
               isValidInput(input.allowedInputs, InputEvent::LEFT)) {
                if(physics.velocity.y > 0 && powerup.walljump) {
                    if(collision.collidingLeft) {
                        physics.wallSliding = true;
                        physics.offWallCount = 0;
                    }
                    else if(physics.wallSliding && input.inputTime[InputEvent::LEFT] > physics.wallSlideTime) {
                        physics.wallSliding = false;
                        physics.offWallCount = physics.wallJumpTime + 1;
                    }
                }
                if(physics.offWallCount > physics.wallJumpTime) {
                    dir.direction = Direction::WEST;
                    physics.velocity.x -= (physics.touchingGround) ? physics.acceleration.x : physics.airAcceleration.x;
                }
            }
            else if(input.inputTime[InputEvent::RIGHT] > 0 &&
               input.inputTime[InputEvent::LEFT] == 0 &&
               isValidInput(input.allowedInputs, InputEvent::RIGHT)) {
                if(physics.velocity.y > 0 && powerup.walljump) {
                    if(collision.collidingRight) {
                        physics.wallSliding = true;
                        physics.offWallCount = 0;
                    }
                    else if(physics.wallSliding && input.inputTime[InputEvent::RIGHT] > physics.wallSlideTime) {
                        physics.wallSliding = false;
                        physics.offWallCount = physics.wallJumpTime + 1;
                    }
                }
                if(physics.offWallCount > physics.wallJumpTime) {
                    dir.direction = Direction::EAST;
                    physics.velocity.x += (physics.touchingGround) ? physics.acceleration.x : physics.airAcceleration.x;
                }
            }
            else if(!collision.collidingLeft && !collision.collidingRight) {
                physics.wallSliding = false;
            }
            // ==================== HANDLE Y INPUTS ====================
            if(input.inputTime[InputEvent::JUMP] > 0) {
                if(input.inputTime[InputEvent::JUMP] == 1 &&
                   isValidInput(input.allowedInputs, InputEvent::JUMP)) {
                    if(physics.wallSliding) {
                        physics.wallSliding = false;
                        physics.wallJumping = true;
                        float coefficient = (collision.collidingLeft) ? 1.5f : -1.5f;
                        dir.direction = (collision.collidingLeft) ? Direction::EAST : Direction::WEST;
                        physics.velocity.x = physics.maxVelocity.x * coefficient;
                        physics.velocity.y = physics.jumpPower * -0.8f;
                    }
                    else {
                        physics.velocity.y = physics.jumpPower * -1.f;
                    }
                    physics.touchingGround = false;
                    physics.jumping = true;
                    physics.offGroundCount = physics.coyoteTime;
                }
                else if(physics.jumping && physics.offGroundCount < physics.jumpTime) {
                    if(physics.wallJumping) physics.velocity.y = physics.jumpPower * -0.8f;
                    else physics.velocity.y = physics.jumpPower * -1.f;
                }
            }
            else if(input.inputTime[InputEvent::JUMP] == 0 &&
                    physics.velocity.y < 0 &&
                    physics.jumping) {
                if(physics.wallJumping && physics.offWallCount > physics.wallJumpTime - 1) {
                    physics.wallJumping = false;
                    physics.jumping = false;
                    if(attack.attackTimer > totalAttackDuration) {
                        physics.velocity.y = physics.gravity * -3.f; // give a 3 tick buffer before falling down to make it look smoother
                    }
                    physics.offGroundCount = physics.jumpTime;
                }
                else if(!physics.wallJumping && physics.offGroundCount > physics.shortJumpTime) {
                    physics.jumping = false;
                    if(attack.attackTimer > totalAttackDuration && !attack.airAttacked) {
                        physics.velocity.y = physics.gravity * -3.f; // see above comment
                    }
                    physics.offGroundCount = physics.jumpTime;
                }
            }
            // ==================== HANDLE CROUCHING ====================
            if(input.inputTime[InputEvent::DOWN] > 0 &&
               isValidInput(input.allowedInputs, InputEvent::DOWN)) {
                if(!crouchComp.crouching) {
                    crouchComp.crouching = true;
                    collision.collisionRect.h = crouchComp.crouchingHeight;
                    collision.collisionRectOffset.y += crouchComp.standingHeight - crouchComp.crouchingHeight;
                    collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;
                    hurtbox.offset.y = 18.f;
                }
            }
            else if(crouchComp.canUncrouch && crouchComp.crouching) {
                crouchComp.crouching = false;
                collision.collisionRect.h = crouchComp.standingHeight;
                collision.collisionRectOffset.y -= crouchComp.standingHeight - crouchComp.crouchingHeight;
                collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;
                hurtbox.offset.y = 13.f;
            }

            // ==================== HANDLE ATTACKING ====================
            if(input.inputTime[InputEvent::ATTACK] == 1 &&
               isValidInput(input.allowedInputs, InputEvent::ATTACK)) {
                attack.attackTimer = 0;
                float coefficient = (dir.direction == Direction::EAST) ? 1.f : -1.f;
                strb::vec2f force = (physics.touchingGround) ? strb::vec2f{0.f, 0.f} : attack.airAttackForce;
                physics.velocity.x += force.x * coefficient;
                physics.velocity.y = force.y;
                if(!physics.touchingGround) attack.airAttacked = true;
            }
            if(physics.touchingGround) {
                attack.airAttacked = false;
                if(attack.attackTimer < attack.groundAttackStartup) {
                    // ???
                }
                else if(attack.attackTimer < attack.groundAttackStartup + attack.groundAttackDuration) {
                    hitbox.hitboxes.clear();
                    Hitbox groundAttackHitbox;
                    groundAttackHitbox.bounds = {0.f, 0.f, 32.f, 40.f};
                    if(dir.direction == Direction::EAST) {
                        groundAttackHitbox.offset = {24.f, -8.f};
                    }
                    else if(dir.direction == Direction::WEST) {
                        groundAttackHitbox.offset = {-32.f, -8.f};
                    }
                    hitbox.hitboxes.push_back(groundAttackHitbox);
                    hitbox.selfKnockback = attack.groundAttackForce;
                    hitstop.hitstopDurationOnAttack = 0;
                }
                else if(attack.attackTimer >= attack.groundAttackStartup + attack.groundAttackDuration) {
                    if(hitbox.hitboxes.size()) hitbox.hitboxes.clear();
                    if(hitbox.hits.size()) hitbox.hits.clear();
                }
            }
            else {
                if(attack.attackTimer < attack.airAttackStartup) {
                    // ???
                }
                else if(attack.attackTimer < attack.airAttackStartup + attack.airAttackDuration) {
                    hitbox.hitboxes.clear();
                    Hitbox airAttackHitbox;
                    airAttackHitbox.bounds = {0.f, 0.f, 64.f, 64.f};
                    airAttackHitbox.offset = {-20.f, -16.f};
                    hitbox.hitboxes.push_back(airAttackHitbox);
                    hitbox.selfKnockback = attack.airAttackForce;
                    hitstop.hitstopDurationOnAttack = 100;
                }
                else if(attack.attackTimer >= attack.airAttackStartup + attack.airAttackDuration) {
                    if(hitbox.hitboxes.size()) hitbox.hitboxes.clear();
                    if(hitbox.hits.size()) hitbox.hits.clear();
                }
            }

            // ==================== STATE SETTING ====================
            if(hurtbox.hitstunCount < hurtbox.hitstunTime) {
                state.state = EntityState::HURT;
            }
            else if(physics.offGroundCount >= physics.coyoteTime) {
                if(attack.attackTimer < totalAttackDuration) state.state = EntityState::ATTACKING_AIR;
                else if(physics.wallSliding) state.state = EntityState::WALLSLIDING;
                else if(physics.velocity.y < 0) {
                    if(physics.wallJumping) state.state = EntityState::WALLJUMPING;
                    else state.state = EntityState::JUMPING;
                }
                else state.state = EntityState::FALLING;
            }
            else {
                if(attack.attackTimer < totalAttackDuration) state.state = EntityState::ATTACKING_GROUND;
                else if(physics.velocity.x != 0.f && !collision.collidingLeft && !collision.collidingRight) {
                    if(crouchComp.crouching) state.state = EntityState::CROUCH_WALKING;
                    else state.state = EntityState::RUNNING;
                }
                else if(crouchComp.crouching) state.state = EntityState::CROUCHING;
                else state.state = EntityState::IDLE;
            }

            // ==================== TIMER AND COMPONENT UPDATES ====================
            if(physics.offWallCount < physics.wallJumpTime ||
               hurtbox.invulnCount < ON_HURT_KNOCKBACK_DURATION ||
               hitbox.doubleHitTimer < ON_HIT_FORCE_DURATION) {
                physics.ignoreFriction = true;
            }
            else {
                physics.ignoreFriction = false;
            }
            if(hitstop.hitstopCount >= hitstop.hitstopCountLimit) ++attack.attackTimer;
            float coefficient = (dir.direction == Direction::EAST) ? -1.f : 1.f;
            if(hitbox.doubleHitTimer < ON_HIT_FORCE_DURATION && physics.touchingGround) physics.velocity.x += (ON_HIT_FORCE_DURATION / (hitbox.doubleHitTimer + 1) * coefficient);
        }

    private:
        bool isValidInput(std::vector<InputEvent> allowedInputs, InputEvent input) {
            return std::find(allowedInputs.begin(), allowedInputs.end(), input) != allowedInputs.end();
        }
        
        const int ON_HIT_FORCE_DURATION = 60; // on hitting something, how many ticks do we get knocked back for?
        const int ON_HURT_KNOCKBACK_DURATION = 500; // when hit, how many ms do we fly away without slowing down?

    };

    
    class PlayerOnHitScript : public IScript {
    public:
        PlayerOnHitScript() = default;
        ~PlayerOnHitScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            std::cout << "hit something!" << std::endl;
        }

    private:

    };


    class PlayerOnHurtScript : public IScript {
    public:
        PlayerOnHurtScript() = default;
        ~PlayerOnHurtScript() = default;

        void update(entt::registry& ecs, entt::entity owner, float timescale, std::shared_ptr<Audio> audio) override {
            std::cout << "ow me player me hurt!" << std::endl;
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

        ecs.emplace<PlayerComponent>(player, PlayerComponent{});

        ecs.emplace<TransformComponent>(player, pos, pos);

        PhysicsComponent physics;
        physics.maxVelocity = {120.f, 240.f};
        physics.frictionCoefficient = 40.f;
        physics.airFrictionCoefficient = 20.f;
        physics.acceleration = {80.f, 0.f};
        physics.airAcceleration = {40.f, 0.f};
        physics.jumpPower = 250.f;
        physics.gravity = 12.5f;
        physics.jumpTime = 11;
        physics.shortJumpTime = 7;
        ecs.emplace<PhysicsComponent>(player, physics);

        ecs.emplace<DirectionComponent>(player, DirectionComponent{Direction::EAST});

        ecs.emplace<RenderComponent>(player, RenderComponent{{0, 0, 24, 32}});

        ecs.emplace<CollisionComponent>(player, CollisionComponent{{pos.x, pos.y , 16.f, 20.f}, {4, 12}});
        
        ecs.emplace<InputComponent>(player, InputComponent{{InputEvent::LEFT, InputEvent::RIGHT, InputEvent::JUMP}});

        ecs.emplace<ScriptComponent>(player, ScriptComponent{std::make_shared<PlayerScript>()});

        ecs.emplace<PowerupComponent>(player, PowerupComponent{});

        ecs.emplace<CrouchComponent>(player, CrouchComponent{20.f, 16.f});

        AttackComponent attack;
        attack.groundAttackStartup = 3;
        attack.groundAttackDuration = 4;
        attack.groundAttackCooldown = 4;
        attack.groundAttackForce = {120.f, 0.f};
        attack.groundAttackKnockback = {0.f, 0.f};
        attack.airAttackStartup = 4;
        attack.airAttackDuration = 6;
        attack.airAttackCooldown = 20;
        attack.airAttackForce = {0.f, -150.f};
        attack.airAttackKnockback = 0.f;
        ecs.emplace<AttackComponent>(player, attack);

        HitboxComponent hitboxComp;
        hitboxComp.damage = 1;
        hitboxComp.onHitScript = std::make_shared<PlayerOnHitScript>();
        hitboxComp.doubleHitTimerLimit = 1000;
        hitboxComp.doubleHitTimer = 1000;
        ecs.emplace<HitboxComponent>(player, hitboxComp);

        HurtboxComponent hurtboxComp;
        hurtboxComp.bounds = {0, 0, 12, 12};
        hurtboxComp.offset = {6.f, 13.f};
        hurtboxComp.onHurtScript = std::make_shared<PlayerOnHurtScript>();
        hurtboxComp.invulnTime = 2000;
        hurtboxComp.invulnCount = 2000;
        ecs.emplace<HurtboxComponent>(player, hurtboxComp);

        ecs.emplace<HitstopComponent>(player, HitstopComponent{});

        ecs.emplace<StateComponent>(player, StateComponent{EntityState::IDLE});

        ecs.emplace<AnimationComponent>(player, AnimationComponent{});

        ecs.emplace<SpritesheetPropertiesComponent>(player, createSpritesheetPropertiesComponent());

        return player;
    }

    SpritesheetPropertiesComponent Player::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;
        propsComp.spritesheet = SpritesheetRegistry::getSpritesheet(SpritesheetID::PLAYER);

        SpritesheetProperties idleEast = {
            0,                         // xTileIndex
            1,                         // yTileIndex
            false,                     // isAnimated
            false,                     // isLooped
            1,                         // numOfFrames
            1,                         // msBetweenFrames
            SDL_FLIP_NONE,             // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::IDLE, Direction::EAST, idleEast);

        SpritesheetProperties idleWest = {
            0,                         // xTileIndex
            1,                         // yTileIndex
            false,                     // isAnimated
            false,                     // isLooped
            1,                         // numOfFrames
            1,                         // msBetweenFrames
            SDL_FLIP_HORIZONTAL,       // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::IDLE, Direction::WEST, idleWest);

        SpritesheetProperties runningEast = {
            0,                         // xTileIndex
            2,                         // yTileIndex
            true,                      // isAnimated
            true,                      // isLooped
            RUNNING_NUM_OF_FRAMES,     // numOfFrames
            RUNNING_MS_BETWEEN_FRAMES, // msBetweenFrames
            SDL_FLIP_NONE,             // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::RUNNING, Direction::EAST, runningEast);

        SpritesheetProperties runningWest = {
            0,                         // xTileIndex
            2,                         // yTileIndex
            true,                      // isAnimated
            true,                      // isLooped
            RUNNING_NUM_OF_FRAMES,     // numOfFrames
            RUNNING_MS_BETWEEN_FRAMES, // msBetweenFrames
            SDL_FLIP_HORIZONTAL,       // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::RUNNING, Direction::WEST, runningWest);
        
        SpritesheetProperties jumpingEast = {
            0,                         // xTileIndex
            3,                         // yTileIndex
            false,                     // isAnimated
            false,                     // isLooped
            1,                         // numOfFrames
            1,                         // msBetweenFrames
            SDL_FLIP_NONE,             // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::JUMPING, Direction::EAST, jumpingEast);

        SpritesheetProperties jumpingWest = {
            0,                         // xTileIndex
            3,                         // yTileIndex
            false,                     // isAnimated
            false,                     // isLooped
            1,                         // numOfFrames
            1,                         // msBetweenFrames
            SDL_FLIP_HORIZONTAL,       // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::JUMPING, Direction::WEST, jumpingWest);
        
        SpritesheetProperties fallingEast = {
            0,                         // xTileIndex
            4,                         // yTileIndex
            true,                      // isAnimated
            false,                     // isLooped
            FALLING_NUM_OF_FRAMES,     // numOfFrames
            FALLING_MS_BETWEEN_FRAMES, // msBetweenFrames
            SDL_FLIP_NONE,             // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::FALLING, Direction::EAST, fallingEast);

        SpritesheetProperties fallingWest = {
            0,                         // xTileIndex
            4,                         // yTileIndex
            true,                      // isAnimated
            false,                     // isLooped
            FALLING_NUM_OF_FRAMES,     // numOfFrames
            FALLING_MS_BETWEEN_FRAMES, // msBetweenFrames
            SDL_FLIP_HORIZONTAL,       // flip
            0.0,                       // angle
            {-1.f, -1.f}               // center
        };
        propsComp.addSpritesheetProperties(EntityState::FALLING, Direction::WEST, fallingWest);

        return propsComp;
    }
}