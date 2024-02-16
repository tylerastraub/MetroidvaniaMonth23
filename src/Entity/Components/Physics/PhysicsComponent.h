#pragma once

#include "vec2.h"

struct PhysicsComponent {
    strb::vec2f velocity = {0.f, 0.f};
    strb::vec2f acceleration = {0.f, 0.f};
    strb::vec2f airAcceleration = {0.f, 0.f};
    strb::vec2f maxVelocity = {0.f, 0.f};

    bool touchingGround = false;
    bool onPlatform = false;

    bool jumping = false;
    int offGroundCount = 0; // Number of ticks the entity has been off the ground
    int coyoteTime = 4; // Number of ticks entity can jump after leaving ground
    int jumpTime = 10; // Maximum number of ticks entity can jump for
    int shortJumpTime = 6; // Minimum number of ticks entity can jump for
    float jumpPower = 0.f;

    float frictionCoefficient = 10.f;
    float airFrictionCoefficient = 2.f;
    float gravity = 10.f;
};