#pragma once

#include <optional>

#include "base.hpp"
#include "game_hook.hpp"


struct UnityEngine_GameObject_o;


class Fixes final : public Mod {
    GameHook photonNetwork$$ConnectToBestCloudServerHook;

    UnityEngine_GameObject_o *fixMark = reinterpret_cast<UnityEngine_GameObject_o *>(1u);

    bool isSceneFixed();
    void markSceneFixed();
    void sceneFix();

public:
    Fixes();

    void uiUpdate() override;
};

extern ModInfo fixesInfo;
