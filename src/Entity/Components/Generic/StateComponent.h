#pragma once

enum class EntityState {
    NOVAL = -1,
    IDLE,
    RUNNING,
    JUMPING,
    FALLING,
    HURT,
    ATTACKING_GROUND,
    ATTACKING_AIR,
    WALLSLIDING,
    WALLJUMPING,
    CROUCHING,
    CROUCH_WALKING,
};

struct StateComponent {
    EntityState state = EntityState::NOVAL;
};