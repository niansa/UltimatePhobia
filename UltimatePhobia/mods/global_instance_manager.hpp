#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "il2cpp.h"


class GlobalInstanceManager final : public Mod {
public:
    Player_o *player;
    GhostAI_o *ghost;
    GameController_o *gameController;

    GameHook player$$AwakeHook,
             ghostAI$$AwakeHook,
             gameController$$AwakeHook;

    GlobalInstanceManager();
};

extern ModInfo globalInstanceManagerInfo;
