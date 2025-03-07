#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "game_enums.hpp"

#include <queue>
#include <map>
#include <commoncpp/timer.hpp>

struct PainKillers_o;

class Improvements final : public Mod {
    bool autoGrabKeys = false, refillPills = false;

public:
    std::map<PainKillers_o *, common::Timer> painKillerTimers;

    GameHookPool hooks;

    void uiUpdate() override;
};

extern ModInfo improvementsInfo;
