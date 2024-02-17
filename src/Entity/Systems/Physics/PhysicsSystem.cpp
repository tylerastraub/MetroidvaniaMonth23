#include "PhysicsSystem.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "CollisionComponent.h"
#include "CrouchComponent.h"

#include <iostream>
#include <algorithm>

void PhysicsSystem::updateX(entt::registry& ecs, float timescale) {
    auto entities = ecs.view<PhysicsComponent, TransformComponent>();
    for(auto ent : entities) {
        auto& physics = ecs.get<PhysicsComponent>(ent);
        auto& transform = ecs.get<TransformComponent>(ent);

        if(physics.wallSliding && ecs.all_of<CollisionComponent>(ent)) {
            auto collision = ecs.get<CollisionComponent>(ent);
            if(collision.collidingLeft) physics.velocity.x = physics.acceleration.x * -1.f;
            else if(collision.collidingRight) physics.velocity.x = physics.acceleration.x;
        }
        transform.lastPosition = transform.position;
        if(physics.velocity.x != 0.f) {
            float maxVelocity = physics.maxVelocity.x;
            if(ecs.all_of<CrouchComponent>(ent)) {
                auto crouchComp = ecs.get<CrouchComponent>(ent);
                if(crouchComp.crouching) maxVelocity *= crouchComp.crouchSpeedModifier;
            }
            if(physics.velocity.x > maxVelocity) physics.velocity.x = maxVelocity;
            else if(physics.velocity.x < maxVelocity * -1.f) physics.velocity.x = maxVelocity * -1.f;
            transform.position.x += physics.velocity.x * timescale;
            float friction = (physics.touchingGround) ? physics.frictionCoefficient : physics.airFrictionCoefficient;
            if(physics.offWallCount >= physics.wallJumpTime) moveToZero(physics.velocity.x, friction);
        }
    }
}

void PhysicsSystem::updateY(entt::registry& ecs, float timescale) {
    auto entities = ecs.view<PhysicsComponent, TransformComponent>();
    for(auto ent : entities) {
        auto& physics = ecs.get<PhysicsComponent>(ent);
        auto& transform = ecs.get<TransformComponent>(ent);

        if(physics.wallSliding) {
            physics.velocity.y = physics.wallSlideVelocity;
        }
        else {
            physics.velocity.y += physics.gravity;
        }
        transform.lastPosition = transform.position;
        if(physics.velocity.y != 0.f) {
            if(physics.velocity.y > physics.maxVelocity.y) physics.velocity.y = physics.maxVelocity.y;
            else if(physics.velocity.y < physics.maxVelocity.y * -1.f) physics.velocity.y = physics.maxVelocity.y * -1.f;
            transform.position.y += physics.velocity.y * timescale;
            float friction = (physics.touchingGround) ? physics.frictionCoefficient : physics.airFrictionCoefficient;
        }
        if(physics.touchingGround) {
            physics.offGroundCount = 0;
        }
        else {
            physics.offGroundCount++;
        }
        if(physics.wallSliding) {
            physics.offWallCount = 0;
        }
        else {
            physics.offWallCount++;
        }
    }
}

void PhysicsSystem::moveToZero(float &value, float amount) {
    if(value != 0.f) {
        if(value > 0.f) {
            value = (value > amount) ? value - amount : 0.f;
        }
        else {
            value = (std::abs(value) > amount) ? value + amount : 0.f;
        }
    }
}