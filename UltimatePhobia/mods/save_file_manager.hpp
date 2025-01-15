#pragma once

#include <string_view>
#include <queue>
#include <functional>

#include "base.hpp"
#include "game_hook.hpp"


class SaveFileManager final : public Mod {
public:
    bool disableSaveEncryption = false;
    bool dangerZone = false;
    bool decryptionPending = false;

    GameHook es3Stream$$CreateStreamHook;

    SaveFileManager();

    void uiUpdate() override;

    static void loadIfNeeded();
};

extern ModInfo saveFileManagerInfo;
