#pragma once

#include "vec2.h"

struct EnemyComponent {
    strb::vec2f playerDistanceThreshold = {10000.f, 10000.f}; // how close player needs to be for enemy to detect them

    int currentActionDurationMin = 0;
    int currentActionDurationMax = 0;
    int currentActionTimer = 0;
};