#pragma once

#include "base.hpp"
#include "game_hook.hpp"

#include <map>


struct Player_o;
struct UnityEngine_GameObject_o;


class PlayerManager final : public Mod {
public:
    std::map<Player_o *, UnityEngine_GameObject_o *> trackedPlayers;

    GameHook player$$UpdateHook,
             player$$OnDisableHook;

    PlayerManager();
    ~PlayerManager();

    Player_o *getLocalPlayer() const;
};

extern ModInfo playerManagerInfo;
