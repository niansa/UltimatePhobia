#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "il2cpp.h"

class GlobalInstanceManager final : public Mod {
public:
    Player_o *player = nullptr;
    GhostAI_o *ghost = nullptr;
    GameController_o *gameController = nullptr;

    GameHook player$$AwakeHook, ghostAI$$AwakeHook, gameController$$AwakeHook;

    GlobalInstanceManager();
};

extern ModInfo globalInstanceManagerInfo;
