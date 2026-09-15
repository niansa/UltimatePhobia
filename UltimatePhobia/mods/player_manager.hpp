#pragma once

#include "mods/base.hpp"
#include "game_hook.hpp"

#include <map>
#include <mutex>
#include <string>

struct Player_o;

// Stripped down since SluggerManager handles coordinates and depth calculation
struct PlayerTextData {
    std::string text;
};

class PlayerManager final : public Mod {
public:
    std::map<Player_o *, PlayerTextData> trackedPlayers;
    mutable std::mutex m_playerMutex;

    GameHook player$$UpdateHook;
    GameHook player$$OnDisableHook;

    PlayerManager();
    ~PlayerManager() override;

    Player_o *getLocalPlayer() const;
};

extern ModInfo playerManagerInfo;
