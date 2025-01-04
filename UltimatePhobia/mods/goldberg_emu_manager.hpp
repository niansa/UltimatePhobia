#pragma once

#include "base.hpp"

#include <string>
#include <string_view>
#include <array>
#include <filesystem>


class GoldbergEmuManager : public Mod {
    std::filesystem::path settingsPath;

    std::string readFile(std::string_view);
    void writeFile(std::string_view, std::string_view);
    void copyStrToBuf(std::string_view, char *, size_t);

    std::array<char, 32> accountName;
    std::array<char, 16> language;

public:
    static std::filesystem::path getSettingsPath();

    GoldbergEmuManager();

    void uiUpdate() override;
};

extern ModInfo goldbergEmuManagerInfo;
