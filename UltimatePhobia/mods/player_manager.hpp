#pragma once

#include "base.hpp"
#include "game_hook.hpp"

#include <vector>


struct Player_o;


class PlayerManager : public Mod {
public:
    std::vector<Player_o *> trackedPlayers;

    GameHook player$$UpdateHook,
             player$$OnDisableHook;

    PlayerManager();

    void uiUpdate();
};

extern ModInfo playerManagerInfo;
