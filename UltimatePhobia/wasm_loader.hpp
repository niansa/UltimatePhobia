#pragma once

#include "mods/base.hpp"

#include <string_view>
#include <filesystem>


namespace WASMLoader {
ModInfo *createModInfo(const std::filesystem::path& base, std::string_view identifier);
}
