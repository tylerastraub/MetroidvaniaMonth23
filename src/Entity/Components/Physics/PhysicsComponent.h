#pragma once

#include "vec2.h"

struct PhysicsComponent {
    strb::vec2f velocity = {0.f, 0.f};
    strb::vec2f acceleration = {0.f, 0.f};
    strb::vec2f airAcceleration = {0.f, 0.f};
    strb::vec2f maxVelocity = {0.f, 0.f};

    bool touchingGround = false;
    int offGroundCount = 0; // Number of frames the entity has been off the ground
    int coyoteTime = 4; // Number of frames entity can jump after leaving ground
    float jumpPower = 0.f;
    float frictionCoefficient = 10.f;
    float airFrictionCoefficient = 2.f;
    float gravity = 10.f;
};