#include "save_file_manager.hpp"
#include "game_types.hpp"
#include "global_state.hpp"
#include "application.hpp"
#include "generated/il2cpp.hpp"

#include <filesystem>
#include <fstream>
#include <imgui.h>



static System_IO_Stream_o *es3Stream$$CreateStreamFnc(System_IO_Stream_o* stream, ES3Settings_o* settings, int32_t fileMode, const MethodInfo* method) {
    if (!Application::isActive())
        return nullptr;
    const auto self = saveFileManagerInfo.get<SaveFileManager>();
    auto& hook = self->es3Stream$$CreateStreamHook;
    GameHookRelease GHR(hook);
    if (self->disableSaveEncryption)
        settings->fields.encryptionType = 0;
    if (self->decryptionPending) {
        g.logger->info("Decrypting save file: Closing existing stream...");
        Il2Cpp::Methods::System_IO_Stream__Close(stream);
        g.logger->info("Decrypting save file: Loading raw save file...");
        const auto bytes = SaveFileManager::loadRawBytes("SaveFile.es3", settings);
        g.logger->info("Decrypting save file: Disabling encryption...");
        settings->fields.encryptionType = 0;
        g.logger->info("Decrypting save file: Saving loaded save file...");
        SaveFileManager::saveRawBytes(bytes, "SaveFile.es3", settings);
        g.logger->info("Decrypting save file: Final exit.");
        Application::exit(0);
        self->disableSaveEncryption = true;
        self->decryptionPending = false;
        return nullptr;
    }
    return hook.getFunction<decltype(es3Stream$$CreateStreamFnc)>()(stream, settings, fileMode, method);
}


SaveFileManager::SaveFileManager()
    : es3Stream$$CreateStreamHook(
          Il2Cpp::Methods::ES3Internal_ES3Stream__CreateStream_getPtr<System_IO_Stream_o *, ES3Settings_o *, int32_t>(),
          reinterpret_cast<void*>(es3Stream$$CreateStreamFnc)
          ) {}

void SaveFileManager::uiUpdate() {
    using namespace ImGui;
    Begin("Save File Manager");
    SeparatorText("Danger Zone");
    if (dangerZone) {
        PushStyleColor(ImGuiCol_Text, IM_COL32(0x00, 0xff, 0x00, 0xff));
        if (Button("Leave danger zone"))
            dangerZone = false;
        PopStyleColor();
        BeginDisabled(decryptionPending);
        Checkbox("Disable encryption", &disableSaveEncryption);
        EndDisabled();
        if (Button("Delete save file"))
            SaveFileManager::renameFile("SaveFile.es3", "SaveFile-trashed.es3");
        SameLine();
        if (!decryptionPending) {
            BeginDisabled(disableSaveEncryption);
            if (Button("Decrypt save file & Exit"))
                decryptionPending = true;
            EndDisabled();
        } else {
            BeginDisabled();
            Button("Decryption pending...");
            EndDisabled();
        }
        Separator();
    } else {
        PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0x00, 0x00, 0xff));
        if (Button("Enter danger zone"))
            dangerZone = true;
        PopStyleColor();
    }
    End();
}

void SaveFileManager::renameFile(std::string_view oldFilePath, std::string_view newFilePath) {
    Il2Cpp::Methods::ES3__RenameFile(GameTypes::createCsString(oldFilePath), GameTypes::createCsString(newFilePath), nullptr);
}

System_Byte_array *SaveFileManager::loadRawBytes(std::string_view filePath, ES3Settings_o *settings) {
    return Il2Cpp::Methods::ES3__LoadRawBytes(GameTypes::createCsString(filePath), settings, nullptr);
}

void SaveFileManager::saveRawBytes(System_Byte_array *bytes, std::string_view filePath, ES3Settings_o *settings) {
    Il2Cpp::Methods::ES3__SaveRaw(bytes, GameTypes::createCsString(filePath), settings, nullptr);
}

void SaveFileManager::loadIfNeeded() {
    g.logger->info("Checking if save file is decrypted...");
    const std::filesystem::path dataPath = GameTypes::toCppString(Il2Cpp::Methods::UnityEngine_Application__get_persistentDataPath());
    const auto saveFilePath = dataPath/"SaveFile.es3";
    const bool isJson = std::ifstream(saveFilePath, std::ios::binary).get() == '{';
    if (isJson) {
        g.logger->info("Save file is decrypted, handling this condition...");
        saveFileManagerInfo.load();
        saveFileManagerInfo.get<SaveFileManager>()->disableSaveEncryption = true;
    } else {
        g.logger->info("Save file is encrypted. Nothing to be done here.");
    }
}


ModInfo saveFileManagerInfo {
    "Save File Manager",
    false,
    [] () {return std::make_unique<SaveFileManager>();},
    [] () {SaveFileManager::loadIfNeeded();}
};
