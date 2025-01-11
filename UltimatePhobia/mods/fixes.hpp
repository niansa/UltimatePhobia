#pragma once

#include <optional>

#include "base.hpp"
#include "game_hook.hpp"


struct UnityEngine_GameObject_o;


class Fixes final : public Mod {
    UnityEngine_GameObject_o *fixMark = reinterpret_cast<UnityEngine_GameObject_o *>(1u);

    bool isSceneFixed();
    void markSceneFixed();
    void sceneFix();

    void updateVelocity();

public:
    GameHook characterController$$get_velocityHook;

    float distanceWalked = 0.0f;
    float velocity = 0.0f;

    UnityEngine_Vector3_Fields lastPlayerWorldPos = {0.0f, 0.0f, 0.0f};

    Fixes();

    void uiUpdate() override;
};

extern ModInfo fixesInfo;
