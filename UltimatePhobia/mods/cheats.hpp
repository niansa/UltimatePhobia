#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "game_enums.hpp"

#include <optional>
#include <queue>


class Cheats final : public Mod {
    bool infiniteStamina = false,
         pauseGhost = false,
         invincibility = false;

    void setGhostAIChangeStateHook();

public:
    std::queue<GhostState> ghostStateQueue;

    std::optional<GameHook> playerStaminaStartDrainingHook,
                            ghostUpdateHook,
                            playerStartKillingPlayerHook,
                            ghostAIChangeStateHook;

    void uiUpdate() override;
};

extern ModInfo cheatsInfo;
