#include "goldberg_emu_manager.hpp"
#include "command_handler.hpp"
#include "console.hpp"

#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <imgui.h>

namespace {
class GoldbergEmuCommandHandler final : public CommandHandler {
private:
    GoldbergEmuManager *mod_;

public:
    explicit GoldbergEmuCommandHandler(GoldbergEmuManager *mod) : mod_(mod) {}

    void updateApp(CLI::App& app) override {
        CLI::App *cmd = app.add_subcommand("goldberg", "Manage Goldberg Steam emulator configuration");
        cmd->callback([this]() { mod_->printStatus(); });

        CLI::App *cmdName = cmd->add_subcommand("name", "Set nick name");
        auto nameVal = std::make_shared<std::string>();
        cmdName->add_option("account_name", *nameVal, "New nick name")->required();
        cmdName->callback([this, nameVal]() { mod_->setAccountName(*nameVal); });

        CLI::App *cmdLang = cmd->add_subcommand("language", "Set game language");
        auto langVal = std::make_shared<std::string>();
        cmdLang->add_option("lang", *langVal, "Language code (e.g. english)")->required();
        cmdLang->callback([this, langVal]() { mod_->setLanguage(*langVal); });
    }
};
} // namespace

std::filesystem::path GoldbergEmuManager::getSettingsPath() {
    return std::filesystem::path(getenv("USERPROFILE")) / "AppData" / "Roaming" / "Goldberg SteamEmu Saves" / "settings";
}

std::string GoldbergEmuManager::readFile(std::string_view name) {
    std::string fres;
    std::getline(std::ifstream(settingsPath / name), fres);
    return fres;
}

void GoldbergEmuManager::writeFile(std::string_view name, std::string_view data) { std::ofstream(settingsPath / name) << data << '\n'; }

void GoldbergEmuManager::copyStrToBuf(std::string_view str, char *buf, size_t buf_len) {
    if (buf == nullptr || buf_len == 0)
        return;

    const size_t copy_len = std::min(str.size(), buf_len - 1);
    std::memcpy(buf, str.data(), copy_len);
    buf[copy_len] = '\0';
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

std::unique_ptr<CommandHandler> GoldbergEmuManager::getCommandHandler() { return std::make_unique<GoldbergEmuCommandHandler>(this); }

void GoldbergEmuManager::setAccountName(std::string_view name) {
    copyStrToBuf(name, accountName.data(), accountName.size());
    writeFile("account_name.txt", {accountName.data(), strlen(accountName.data())});
    console::println(std::format("Account name updated to '{}'. Restart game to apply.", accountName.data()));
}

void GoldbergEmuManager::setLanguage(std::string_view lang) {
    copyStrToBuf(lang, language.data(), language.size());
    writeFile("language.txt", {language.data(), strlen(language.data())});
    console::println(std::format("Language updated to '{}'. Restart game to apply.", language.data()));
}

void GoldbergEmuManager::printStatus() const {
    console::println(std::format("Account Name: {}", accountName.data()));
    console::println(std::format("Language: {}", language.data()));
}

ModInfo goldbergEmuManagerInfo{"Goldberg Emu Manager", false, []() { return std::make_unique<GoldbergEmuManager>(); },
                               []() {
                                   if (std::filesystem::exists(GoldbergEmuManager::getSettingsPath()))
                                       goldbergEmuManagerInfo.load();
                               }};
