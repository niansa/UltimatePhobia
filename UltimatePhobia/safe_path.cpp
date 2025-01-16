#include "safe_path.hpp"

#include <string_view>
#include <array>
#include <filesystem>
#include <windows.h>



namespace SafePath {
namespace {
std::filesystem::path value = std::filesystem::current_path();
}

void init() {
    std::array<char, 32767> envSafePath;

    if (const auto len = GetEnvironmentVariable("UP_SAFE_PATH", envSafePath.data(), envSafePath.size())) {
        value = std::string_view{envSafePath.data(), len};
        SetEnvironmentVariable("UP_SAFE_PATH", nullptr);
    }
}

const std::filesystem::path& get() {
    return value;
}
}
