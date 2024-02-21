#pragma once

struct RocklingComponent {
    int msSinceHitPlayer = 2000;
    int playerHitCooldown = 2000; // how long until rockling starts chasing player again
};