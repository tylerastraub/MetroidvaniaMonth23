#pragma once

enum class InputEvent {
    NOVAL = -1,
    _MIN_ = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    JUMP,
    INTERACT,
    ATTACK,
    TOGGLE_DEBUG,
    _MAX_,
};