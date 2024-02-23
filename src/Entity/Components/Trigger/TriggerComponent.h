#pragma once

#include "ScriptComponent.h"

enum class TriggerType {
    NOVAL = -1,
    DIALOGUE,
    PREFAB_SPAWN,
    LEVEL_LOAD,
    CAMERA_TARGET,
};

struct TriggerComponent {
    TriggerType type = TriggerType::NOVAL;
    std::shared_ptr<IScript> onTriggerScript = nullptr;
    bool triggerOnce = true;
    bool entityMustBeGrounded = false;
};