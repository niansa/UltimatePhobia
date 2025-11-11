#pragma once

#include <optional>

#include "base.hpp"
#include "game_hook.hpp"

struct UnityEngine_GameObject_o;

class Fixes final : public Mod {
    GameHook photonNetwork$$ConnectToBestCloudServerHook;

    void *fixMark = reinterpret_cast<void *>(1u);

    bool isSceneFixed();
    void markSceneFixed();
    void sceneFix();
    void playerFix();

public:
    Fixes();

    void uiUpdate() override;
};

extern ModInfo fixesInfo;
