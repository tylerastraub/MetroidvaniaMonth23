#pragma once

#include "vec2.h"
#include "rect2.h"

struct CollisionComponent {
    strb::rect2f collisionRect = {0.f, 0.f, 0.f, 0.f};
    strb::vec2f collisionRectOffset = {0.f, 0.f}; // the collision rect's offset from the entity's transform position

    bool collidingLeft = false;
    bool collidingRight = false;
    bool collidingUp = false;
    bool collidingDown = false;
    
    bool onPlatform = false;

    bool onLeftEdge = false;
    bool onRightEdge = false;
};