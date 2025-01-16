#pragma once

#include <filesystem>


namespace SafePath {
void init();
const std::filesystem::path& get();
}
