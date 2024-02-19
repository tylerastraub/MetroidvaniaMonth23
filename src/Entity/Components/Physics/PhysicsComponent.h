#pragma once

#include "vec2.h"

struct PhysicsComponent {
    strb::vec2f velocity = {0.f, 0.f};
    strb::vec2f acceleration = {0.f, 0.f};
    strb::vec2f airAcceleration = {0.f, 0.f};
    strb::vec2f maxVelocity = {0.f, 0.f};

    float frictionCoefficient = 10.f;
    float airFrictionCoefficient = 2.f;
    bool ignoreFriction = false;
    float gravity = 10.f;

    bool touchingGround = false;
    bool touchingGroundLastTick = false;

    // Walljumping stuff
    bool wallSliding = false; // only gets set true if walljumping is enabled
    bool wallJumping = false;
    float wallSlideVelocity = 100.f;
    int offWallCount = 8;
    int wallJumpTime = 8;
    int wallSlideTime = 5; // how many ticks the player has to hold away from the wall before the come off

    // Regular jumping stuff
    bool jumping = false;
    int offGroundCount = 0; // Number of ticks the entity has been off the ground
    int coyoteTime = 4; // Number of ticks entity can jump after leaving ground
    int jumpTime = 10; // Maximum number of ticks entity can jump for
    int shortJumpTime = 6; // Minimum number of ticks entity can jump for
    float jumpPower = 0.f;
};