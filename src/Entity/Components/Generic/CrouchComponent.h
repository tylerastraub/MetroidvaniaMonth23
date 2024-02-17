#pragma once

struct CrouchComponent {
    float standingHeight = 32.f;
    float crouchingHeight = 16.f;
    float crouchSpeedModifier = 0.5f;

    bool crouching = false;
    bool canUncrouch = true;
};