#pragma once

#include <string_view>
#include <queue>
#include <functional>

#include "base.hpp"
#include "game_hook.hpp"


class SaveFileManager : public Mod {
public:
    bool disableSaveEncryption = false;
    bool dangerZone = false;
    bool decryptionPending = false;

    GameHook es3StreamCreateStreamHook;

    SaveFileManager();

    void uiUpdate() override;

    static void renameFile(std::string_view oldFilePath, std::string_view newFilePath);
    static System_Byte_array *loadRawBytes(std::string_view filePath, ES3Settings_o *settings);
    static void saveRawBytes(System_Byte_array *bytes, std::string_view filePath, ES3Settings_o *settings);

    static void loadIfNeeded();
};

extern ModInfo saveFileManagerInfo;
