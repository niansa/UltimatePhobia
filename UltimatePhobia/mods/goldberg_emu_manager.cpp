#include "goldberg_emu_manager.hpp"

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <imgui.h>



std::filesystem::path GoldbergEmuManager::getSettingsPath() {
    return std::filesystem::path(getenv("USERPROFILE"))/"AppData"/"Roaming"/"Goldberg SteamEmu Saves"/"settings";
}


std::string GoldbergEmuManager::readFile(std::string_view name) {
    std::string fres;
    std::getline(std::ifstream(settingsPath/name), fres);
    return fres;
}

void GoldbergEmuManager::writeFile(std::string_view name, std::string_view data) {
    std::ofstream(settingsPath/name) << data << '\n';
}

void GoldbergEmuManager::copyStrToBuf(std::string_view str, char *buf, size_t buf_len) {
    --buf_len; // Account for null terminator
    if (str.size() > buf_len)
        str = str.substr(0, buf_len);
    memcpy(buf, str.data(), str.size());
    buf[buf_len] = '\0';
}


GoldbergEmuManager::GoldbergEmuManager() {
    settingsPath = getSettingsPath();
    copyStrToBuf(readFile("account_name.txt"), accountName.data(), accountName.size());
    copyStrToBuf(readFile("language.txt"), language.data(), language.size());

    if (!std::filesystem::exists(settingsPath))
        goldbergEmuManagerInfo.unload();
}

void GoldbergEmuManager::uiUpdate() {
    using namespace ImGui;
    Begin("Goldberg Emu Manager");
    if (InputText("Account Name", accountName.data(), accountName.size()))
        writeFile("account_name.txt", {accountName.data(), accountName.size()});
    if (InputText("System Language", language.data(), language.size()))
        writeFile("language.txt", {language.data(), language.size()});
    TextUnformatted("Restart to apply changes");
    End();
}


ModInfo goldbergEmuManagerInfo {
    "Goldberg Emu Manager",
    [] () {return std::make_unique<GoldbergEmuManager>();},
    [] () {
        if (std::filesystem::exists(GoldbergEmuManager::getSettingsPath()))
            goldbergEmuManagerInfo.load();
    }
};
