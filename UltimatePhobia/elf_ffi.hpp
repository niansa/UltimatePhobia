#pragma once

#include "ffi.hpp"

#include <string>
#include <filesystem>
#include <optional>
#include <elfld/elfload.hpp>

class ELFFFI final : public FFI {
    std::string modPathStr;
    std::optional<ElfLoader> loader;

public:
    ELFFFI(const std::filesystem::path& modPath);
    ~ELFFFI();

    void simpleCall(const char *name);
};
