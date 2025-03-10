#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "il2cpp.h"

class GlobalInstanceManager final : public Mod {
public:
    Player_o *playerPtr = nullptr;
    GhostAI_o *ghostPtr = nullptr;
    GameController_o *gameControllerPtr = nullptr;

    GameHook player$$AwakeHook, ghostAI$$AwakeHook, gameController$$AwakeHook;

    GlobalInstanceManager();

    Player_o *getPlayer();
    GhostAI_o *getGhostAI();
    GameController_o *getGameController();
};

extern ModInfo globalInstanceManagerInfo;
