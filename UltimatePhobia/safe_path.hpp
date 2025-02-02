#pragma once

#include <filesystem>


namespace SafePath {
bool init();
const std::filesystem::path& get();
}
