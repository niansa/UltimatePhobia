#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "il2cpp.h"

#include <optional>


class GlobalInstanceManager final : public Mod {
public:
    Player_o *player;
    GhostAI_o *ghost;

    std::optional<GameHook> playerStartHook,
                            ghostAIStartHook;

    GlobalInstanceManager();
};

extern ModInfo globalInstanceManagerInfo;
