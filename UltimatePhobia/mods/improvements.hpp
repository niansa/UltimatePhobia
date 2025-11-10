#pragma once

#include "base.hpp"
#include "game_hook.hpp"
#include "game_enums.hpp"
#include "il2cpp_api.hpp"

#include <queue>
#include <unordered_map>
#include <commoncpp/timer.hpp>

struct PainKillers_o;

class Improvements final : public Mod {
    bool autoGrabKeys = false, refillPills = false;

public:
    std::unordered_map<Il2CppObject *, common::Timer> painKillerTimers;

    GameHookPool hooks;

    void uiUpdate() override;
};

extern ModInfo improvementsInfo;
