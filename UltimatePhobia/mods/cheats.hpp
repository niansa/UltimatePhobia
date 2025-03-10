#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "game_enums.hpp"

#include <queue>

class Cheats final : public Mod {
    bool infiniteStamina = false, pauseGhost = false, invincibility = false, autoUnlockDoors = false, keepItemsAfterDeath = false, allowGrabWhenDead = false;

    void setGhostAIChangeStateHook();

public:
    std::queue<GhostState> ghostStateQueue;

    GameHookPool hooks;

    void uiUpdate() override;
};

extern ModInfo cheatsInfo;
