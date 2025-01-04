#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "game_enums.hpp"

#include <optional>
#include <queue>


class Cheats final : public Mod {
    bool infiniteStamina = false,
         pauseGhost = false,
         invincibility = false,
         autoGrabKey = false,
         autoUnlockDoors = false,
         keepItemsAfterDeath = false;

    void setGhostAIChangeStateHook();

public:
    std::queue<GhostState> ghostStateQueue;

    std::optional<GameHook> playerStamina$$StartDrainingHook,
                            ghost$$UpdateHook,
                            player$$StartKillingPlayerHook,
                            ghostAI$$ChangeStateHook,
                            key$$StartHook,
                            door$$UpdateHook,
                            inventoryManager$$RemoveItemsFromInventoryHook;

    void uiUpdate() override;

    static void sendRPC(Photon_Pun_PhotonView_o *view, std::string_view methodName, int32_t target);
};

extern ModInfo cheatsInfo;
