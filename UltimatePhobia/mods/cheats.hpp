#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "game_enums.hpp"

#include <queue>

class Cheats final : public Mod {
    bool infiniteStamina = false, pauseGhost = false, invincibility = false,
         autoGrabKeys = false, autoUnlockDoors = false,
         keepItemsAfterDeath = false, allowGrabWhenDead = false;

    void setGhostAIChangeStateHook();

public:
    std::queue<GhostState> ghostStateQueue;

    GameHookPool hooks;

    void uiUpdate() override;

    static void sendRPC(Photon_Pun_PhotonView_o *view,
                        std::string_view methodName, int32_t target);
};

extern ModInfo cheatsInfo;
