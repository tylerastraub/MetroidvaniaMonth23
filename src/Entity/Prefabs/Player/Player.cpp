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
            auto powerup = ecs.get<PowerupComponent>(owner);

            // set allowed inputs
            input.allowedInputs = {InputEvent::LEFT, InputEvent::RIGHT};
            if(physics.offGroundCount < physics.coyoteTime || physics.wallSliding) {
                input.allowedInputs.push_back(InputEvent::JUMP);
            }
            if(physics.touchingGround && powerup.crouch) {
                input.allowedInputs.push_back(InputEvent::DOWN);
            }

            // ==================== X INPUTS ====================
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
                    physics.velocity.x += (physics.touchingGround) ? physics.acceleration.x : physics.airAcceleration.x;
                }
            }
            else if(!collision.collidingLeft && !collision.collidingRight) {
                physics.wallSliding = false;
            }
            // ==================== Y INPUTS ====================
            if(input.inputTime[InputEvent::JUMP] > 0) {
                if(input.inputTime[InputEvent::JUMP] == 1 &&
                   isValidInput(input.allowedInputs, InputEvent::JUMP)) {
                    if(physics.wallSliding) {
                        physics.wallSliding = false;
                        physics.wallJumping = true;
                        float coefficient = (collision.collidingLeft) ? 2.f : -2.f;
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
                    physics.velocity.y = physics.gravity * -3.f; // give a 3 tick buffer before falling down to make it look smoother
                    physics.offGroundCount = physics.jumpTime;
                }
                else if(!physics.wallJumping && physics.offGroundCount > physics.shortJumpTime) {
                    physics.jumping = false;
                    physics.velocity.y = physics.gravity * -3.f; // see above comment
                    physics.offGroundCount = physics.jumpTime;
                }
            }
            // ==================== CROUCHING ====================
            if(input.inputTime[InputEvent::DOWN] > 0 &&
               isValidInput(input.allowedInputs, InputEvent::DOWN)) {
                if(!crouchComp.crouching) {
                    crouchComp.crouching = true;
                    collision.collisionRect.h = crouchComp.crouchingHeight;
                    collision.collisionRectOffset.y += crouchComp.standingHeight - crouchComp.crouchingHeight;
                    collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;
                    render.renderQuad.h = crouchComp.crouchingHeight; // todo: delete
                    render.renderQuadOffset.y += crouchComp.standingHeight - crouchComp.crouchingHeight; // todo: delete
                }
            }
            else if(crouchComp.canUncrouch && crouchComp.crouching) {
                crouchComp.crouching = false;
                collision.collisionRect.h = crouchComp.standingHeight;
                collision.collisionRectOffset.y -= crouchComp.standingHeight - crouchComp.crouchingHeight;
                collision.collisionRect.y = transform.position.y + collision.collisionRectOffset.y;
                render.renderQuad.h = crouchComp.standingHeight; // todo: delete
                render.renderQuadOffset.y -= crouchComp.standingHeight - crouchComp.crouchingHeight; // todo: delete
            }
        }

    private:
        bool isValidInput(std::vector<InputEvent> allowedInputs, InputEvent input) {
            return std::find(allowedInputs.begin(), allowedInputs.end(), input) != allowedInputs.end();
        }

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

        ecs.emplace<RenderComponent>(player, RenderComponent{{0, 0, 24, 32}});

        ecs.emplace<CollisionComponent>(player, CollisionComponent{{pos.x, pos.y , 24.f, 32.f}, {0, 0}});
        
        ecs.emplace<InputComponent>(player, InputComponent{{InputEvent::LEFT, InputEvent::RIGHT, InputEvent::JUMP}});

        ecs.emplace<ScriptComponent>(player, ScriptComponent{std::make_shared<PlayerScript>()});

        ecs.emplace<PowerupComponent>(player, PowerupComponent{});

        ecs.emplace<CrouchComponent>(player, CrouchComponent{32.f, 16.f});

        return player;
    }

    SpritesheetPropertiesComponent Player::createSpritesheetPropertiesComponent() {
        SpritesheetPropertiesComponent propsComp;

        return propsComp;
    }
}