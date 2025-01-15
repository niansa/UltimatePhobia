#include "save_file_manager.hpp"
#include "il2cpp_cppinterop.hpp"
#include "global_state.hpp"
#include "application.hpp"
#include "generated/il2cpp.hpp"

#include <filesystem>
#include <fstream>
#include <imgui.h>

using namespace Il2Cpp::CppInterop;



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
        Il2Cpp::System::IO::Stream::Close(stream);
        g.logger->info("Decrypting save file: Loading raw save file...");
        const auto bytes = Il2Cpp::ES3::LoadRawBytes("SaveFile.es3"_cs, settings);
        g.logger->info("Decrypting save file: Disabling encryption...");
        settings->fields.encryptionType = 0;
        g.logger->info("Decrypting save file: Saving loaded save file...");
        Il2Cpp::ES3::SaveRaw(bytes, "SaveFile.es3"_cs, settings);
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
          Il2Cpp::ES3Internal::ES3Stream::CreateStream_getPtr<System_IO_Stream_o *, ES3Settings_o *, int32_t>(),
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
            Il2Cpp::ES3::RenameFile("SaveFile.es3"_cs, "SaveFile-trashed.es3"_cs);
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

void SaveFileManager::loadIfNeeded() {
    g.logger->info("Checking if save file is decrypted...");
    const std::filesystem::path dataPath = ToCppString(Il2Cpp::UnityEngine::Application::get_persistentDataPath());
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
