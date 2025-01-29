#pragma once

#include "ffi.hpp"

#include <string>
#include <filesystem>
#include <windows.h>


class DLLFFI final : public FFI {
    std::string modPathStr;
    HMODULE chandle;

public:
    DLLFFI(const std::filesystem::path& modPath);
    ~DLLFFI();

    void simpleCall(const char *name);
};
