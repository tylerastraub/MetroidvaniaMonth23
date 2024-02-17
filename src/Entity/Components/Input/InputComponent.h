#pragma once

#include "InputEvent.h"

#include <vector>
#include <unordered_map>

struct InputComponent {
    std::vector<InputEvent> allowedInputs;

    std::unordered_map<InputEvent, int> inputTime; // how many ticks an input has been pressed for
    std::unordered_map<InputEvent, int> lastTickInputTime; // the inputTime's state last tick. used for checking if input was just released
};